#include "dsp_thread.h"
#include "dsp/fft.h"
#include "lcd.h"
#include <string.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include "colormap.h"

semaphore_t bufferFull;
semaphore_t bufferEmpty;
uint16_t* buffer;
int bufferSize;
int bufferId;

int lastBufferId;
complex_t* inBuf;
complex_t* fftOut;
float* fftAmplitude;
uint16_t* wfPixels;

int line = 0;

void dsp_init() {
    inBuf = (complex_t*)malloc(2048*sizeof(complex_t));
    fftOut = (complex_t*)malloc(2048*sizeof(complex_t));
    fftAmplitude = (float*)malloc(2048*sizeof(float));
    wfPixels = (uint16_t*)malloc(240*sizeof(uint16_t));
    memset(inBuf, 0, 2048*sizeof(complex_t));
    memset(fftOut, 0, 2048*sizeof(complex_t));
}

void dsp_thread() {
    while (true) {
        // Acquire buffer
        sem_acquire_blocking(&bufferFull);

        // Convert to complex
        for (int i = 0; i < bufferSize; i++) {
            int16_t sign = (int16_t)buffer[i] - 2048;
            inBuf[i].re = (float)sign * (1.0f/2048.0f);
            inBuf[i].im = 0.0f;
            //if (i & 1) { inBuf[i].re = -inBuf[i].re; }
        }
        
        // Releadse buffer
        sem_release(&bufferEmpty);

        // Do FFT
        fft(inBuf, fftOut, bufferSize);

        // Compute power
        for (int i = 0; i < bufferSize; i++) {
            fftAmplitude[i] = 20.0f*log10f(fftOut[i].amplitude()*(1.0f/256.0f));
        }

        // Convert to pixels
        for (int i = 0; i < 240; i++) {
            float val = (fftAmplitude[i] + 70.0f) * (1.0f/70.0f);
            if (val > 1.0f) { val = 1.0f; }
            else if (val < 0.0f) { val = 0.0f; }

            wfPixels[i] = colormap[(int)(val * 65535.0f)];
        }

        // Write to LCD
        lcd.setScrollStartLine(239-line);
        lcd.blit(0, 239-line, 240, 1, (uint8_t*)wfPixels);

        // Update drawing vars
        line++;
        if (line >= 240) { line = 0; }
    }
}