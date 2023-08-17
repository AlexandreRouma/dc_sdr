/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include <math.h>
#include "../colormap.h"

#define BTN_L   8
#define BTN_CL  9
#define BTN_P   10
#define BTN_CR  11
#define BTN_R   12

#define IS_RGBW false
#define NUM_PIXELS 1

// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 22

struct RGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct HSV
{
	double H;
	double S;
	double V;
};

struct RGB HSVToRGB(struct HSV hsv) {
	double r = 0, g = 0, b = 0;

	if (hsv.S == 0)
	{
		r = hsv.V;
		g = hsv.V;
		b = hsv.V;
	}
	else
	{
		int i;
		double f, p, q, t;

		if (hsv.H == 360)
			hsv.H = 0;
		else
			hsv.H = hsv.H / 60;

		i = (int)trunc(hsv.H);
		f = hsv.H - i;

		p = hsv.V * (1.0 - hsv.S);
		q = hsv.V * (1.0 - (hsv.S * f));
		t = hsv.V * (1.0 - (hsv.S * (1.0 - f)));

		switch (i)
		{
		case 0:
			r = hsv.V;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = hsv.V;
			b = p;
			break;

		case 2:
			r = p;
			g = hsv.V;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = hsv.V;
			break;

		case 4:
			r = t;
			g = p;
			b = hsv.V;
			break;

		default:
			r = hsv.V;
			g = p;
			b = q;
			break;
		}

	}

	struct RGB rgb;
	rgb.R = r * 255;
	rgb.G = g * 255;
	rgb.B = b * 255;

	return rgb;
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

double dmod(double val, double mod) {
	while (val > mod) {
		val -= mod;
	}
	while (val < mod) {
		val += mod;
	}
	return val;
}

int ws2812_main() {
    //set_sys_clock_48();
    printf("WS2812 Smoke Test, using pin %d", WS2812_PIN);

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    int t = 0;
    double hue = 0.0;
	double value = 0.15;
    while (1) {
        HSV hsv = { hue, 1.0, value };
        RGB rgb = HSVToRGB(hsv);
        put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
        put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
		put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
        put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
		put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
        put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
		put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
        put_pixel(urgb_u32(rgb.R, rgb.G, rgb.B));
        hue += 1.0;
        if (hue > 360.0) {
            hue = 0.0;
        }

		if (gpio_get(BTN_L)) {
			value -= 0.01;
		}

		if (gpio_get(BTN_R)) {
			value += 0.01;
		}

		if (gpio_get(BTN_P)) {
			value = 0.15;
		}

		if (value > 1.0) {value = 1.0; };
		if (value < 0.0) {value = 0.0; };

        sleep_ms(10);
    }
}