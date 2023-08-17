#pragma once
#include "complex.h"

inline void fft(const complex_t* in, complex_t* out, int N, int s = 1) {
    if (N == 1) {
        *out = *in;
    }
    else {
        int halfN = N/2;
        float invN = 1.0f/(float)N;
        int twoS = s*2;

        fft(in, out, halfN, twoS);
        fft(&in[s], &out[halfN], halfN, twoS);

        for (int k = 0; k < halfN; k++) {
            float phase = -2.0f*(float)k*FL_M_PI*invN;
            complex_t p = out[k];
            complex_t q = complex_t{ cosf(phase), sinf(phase) } * out[halfN+k];
            out[k] = p+q;
            out[halfN+k] = p-q;
        }
    }
}
