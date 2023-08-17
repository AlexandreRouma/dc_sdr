#include <stdio.h>
#include "pico/stdlib.h"

#include "st77xx.h"
#include "tpl0401.h"
#include "r820t.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/adc.h>
#include <hardware/dma.h>

#include "dsp_thread.h"

#include "ws2812/ws2812.h"

#include "lcd.h"

#include "parrots.h"

ST77XX::Driver lcd;

#define BTN_L   8
#define BTN_CL  9
#define BTN_P   10
#define BTN_CR  11
#define BTN_R   12

int adcDMA0;
int adcDMA1;
dma_channel_config dmaConf0;
dma_channel_config dmaConf1;
uint16_t* buf0;
uint16_t* buf1;
int dmaBufferId = 0;

void dmaIRQ0() {
    // Re-arm DMA
    dma_channel_acknowledge_irq0(adcDMA0);
    dma_channel_set_write_addr(adcDMA0, buf0, false);

    // Send data to DSP
    if (sem_try_acquire(&bufferEmpty)) {
        buffer = buf0;
        bufferId = dmaBufferId++;
        sem_release(&bufferFull);
    }
}

void dmaIRQ1() {
    // Re-arm DMA
    dma_channel_acknowledge_irq1(adcDMA1);
    dma_channel_set_write_addr(adcDMA1, buf1, false);

    // Send data to DSP
    if (sem_try_acquire(&bufferEmpty)) {
        buffer = buf1;
        bufferId = dmaBufferId++;
        sem_release(&bufferFull);
    }
}

int counter = 0;
int test = 0;

void vsync_handler(uint gpio, uint32_t events) {
    

    if (test++ == 2) {
        test = 0;
        lcd.setScrollStartLine(139 - (counter++ % 134) + 44);
    }
}

int main() {
    stdio_init_all();

    // Init buttons
    gpio_init(BTN_L);
    gpio_init(BTN_CL);
    gpio_init(BTN_P);
    gpio_init(BTN_CR);
    gpio_init(BTN_R);
    gpio_set_dir(BTN_L, false);
    gpio_set_dir(BTN_CL, false);
    gpio_set_dir(BTN_P, false);
    gpio_set_dir(BTN_CR, false);
    gpio_set_dir(BTN_R, false);

    // Initialize LCD display
    lcd = ST77XX::Driver(3, 2, 1, 4, -1, 0, 5);
    lcd.reset(ST77XX::RESET_HARDWARE);
    lcd.setSleep(false);
    lcd.setPartial(false);
    lcd.setIdle(false);
    lcd.setColorMode(ST77XX::COLOR_MODE_16BIT);
    lcd.setMemoryAccess(false, false, false, ST77XX::LINE_SCAN_TOP_TO_BOTTOM, ST77XX::COL_SCAN_LEFT_TO_RIGHT, ST77XX::COLOR_RGB);
    lcd.defineVerticalScroll(44, 134, 142);
    lcd.setInverted(true);
    lcd.setTearingOutput(true);

    for (int i = 0; i < 240*240; i++) {
        uint8_t tmp = parrots_bmp[i*2+1];
        parrots_bmp[i*2+1] = parrots_bmp[i*2];
        parrots_bmp[i*2] = tmp;

    }

    lcd.blit(0, 0, 240, 240, parrots_bmp);
    lcd.setBlank(false);

    gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_RISE, true, &vsync_handler);

    // // Start DSP
    // sem_init(&bufferFull, 0, 1);
    // sem_init(&bufferEmpty, 1, 1);
    // dsp_init();
    // multicore_launch_core1(dsp_thread);

    // // Configure ADC
    // adc_init();
    // adc_gpio_init(26);
    // adc_set_round_robin(0);
    // adc_select_input(0);
    // adc_set_clkdiv(0); // Run at 1KS/s
    // adc_fifo_setup(true, true, 1, false, false);

    // // Allocate buffers
    // bufferSize = 512;
    // buf0 = (uint16_t*)malloc(bufferSize*sizeof(uint16_t));
    // buf1 = (uint16_t*)malloc(bufferSize*sizeof(uint16_t));

    // // Configure DMA
    // adcDMA0 = dma_claim_unused_channel(true);
    // adcDMA1 = dma_claim_unused_channel(true);

    // dmaConf0 = dma_channel_get_default_config(adcDMA0);
    // channel_config_set_transfer_data_size(&dmaConf0, DMA_SIZE_16);
    // channel_config_set_read_increment(&dmaConf0, false);
    // channel_config_set_write_increment(&dmaConf0, true);
    // channel_config_set_dreq(&dmaConf0, DREQ_ADC);
    // channel_config_set_irq_quiet(&dmaConf0, false);
    // channel_config_set_chain_to(&dmaConf0, adcDMA1);
    // dma_channel_configure(adcDMA0, &dmaConf0, buf0, &adc_hw->fifo, bufferSize, false);
    // dma_channel_set_irq0_enabled(adcDMA0, true);
    // irq_set_exclusive_handler(DMA_IRQ_0, dmaIRQ0);
    // irq_set_enabled(DMA_IRQ_0, true);

    // dmaConf1 = dma_channel_get_default_config(adcDMA1);
    // channel_config_set_transfer_data_size(&dmaConf1, DMA_SIZE_16);
    // channel_config_set_read_increment(&dmaConf1, false);
    // channel_config_set_write_increment(&dmaConf1, true);
    // channel_config_set_dreq(&dmaConf1, DREQ_ADC);
    // channel_config_set_irq_quiet(&dmaConf1, false);
    // channel_config_set_chain_to(&dmaConf1, adcDMA0);  
    // dma_channel_configure(adcDMA1, &dmaConf1, buf1, &adc_hw->fifo, bufferSize, false);
    // dma_channel_set_irq1_enabled(adcDMA1, true);
    // irq_set_exclusive_handler(DMA_IRQ_1, dmaIRQ1);
    // irq_set_enabled(DMA_IRQ_1, true);

    // // Start DMA
    // dma_channel_start(adcDMA0);
    
    // // Run ADC
    // adc_run(true);

    // Init tuner
    r820t_priv_t r820t = {
      16000000, // xtal_freq => 24MHz
      3000000, // Set at boot to airspy_m0_m4_conf_t conf0 -> r820t_if_freq
      100000000, /* Default Freq 100Mhz */
      {
        /* 05 */ 0x90, // LNA manual gain mode, init to 0
        /* 06 */ 0x80,
        /* 07 */ 0x60,
        /* 08 */ 0x80, // Image Gain Adjustment
        /* 09 */ 0x40, // Image Phase Adjustment
        /* 0A */ 0xA8, // Channel filter [0..3]: 0 = widest, f = narrowest - Optimal. Don't touch!
        /* 0B */ 0x0F, // High pass filter - Optimal. Don't touch!
        /* 0C */ 0x40, // VGA control by code, init at 0
        /* 0D */ 0x63, // LNA AGC settings: [0..3]: Lower threshold; [4..7]: High threshold
        /* 0E */ 0x75,
        /* 0F */ 0xE8, // Filter Widest, LDO_5V OFF, clk out ON,
        /* 10 */ 0x7C,
        /* 11 */ 0x42,
        /* 12 */ 0x06,
        /* 13 */ 0x00,
        /* 14 */ 0x0F,
        /* 15 */ 0x00,
        /* 16 */ 0xC0,
        /* 17 */ 0xA0,
        /* 18 */ 0x48,
        /* 19 */ 0xCC,
        /* 1A */ 0x60,
        /* 1B */ 0x00,
        /* 1C */ 0x54,
        /* 1D */ 0xAE,
        /* 1E */ 0x0A,
        /* 1F */ 0xC0
      },
      0 /* uint16_t padding */
    };

    uint8_t debug;

    printf("Init I2C\n");
    i2c_init(i2c1, 100000);
    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    
    printf("Init R820T\n");
    r820t_init(&r820t, 3000000);
    r820t_standby();


    // printf("Configure R820T\n");
    // r820t_set_freq(&r820t, 98500000);//(107900000 - (1500000 - 875000)));
    // r820t_set_lna_gain(&r820t, 15);
    // r820t_set_vga_gain(&r820t, 15);
    // r820t_set_mixer_gain(&r820t, 15);

    // debug = airspy_r820t_read_single(&r820t, 0x00);
    // printf("REG0: 0x%02X\n", debug);

    // while(1) {
    //     debug = airspy_r820t_read_single(&r820t, 0x00);
    //     printf("REG0: 0x%02X\n", debug);
    //     sleep_ms(1000);
    // }

    ws2812_main();

    // int counter = 0;
    // while(1) {
    //     lcd.setScrollStartLine(139 - (counter++ % 134) + 44);
    //     sleep_ms(50);
    // }
}