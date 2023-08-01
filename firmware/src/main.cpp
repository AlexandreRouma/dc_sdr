#include <stdio.h>
#include "pico/stdlib.h"

#include "st77xx.h"
#include "tpl0401.h"

#include "parrots.h"

#include <hardware/gpio.h>

#include <hardware/i2c.h>

ST77XX::Driver lcd;
void vsync_callback(uint gpio, uint32_t events);

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan() {
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
    return;
}

int main() {
    // // // Initialize LCD display
    // lcd = ST77XX::Driver(3, 2, 1, 4, -1, 0, 5);

    // lcd.reset(ST77XX::RESET_HARDWARE);
    // lcd.setSleep(false);
    // lcd.setPartial(false);
    // lcd.setIdle(false);
    // lcd.setColorMode(ST77XX::COLOR_MODE_16BIT);
    // lcd.setMemoryAccess(false, false, false, ST77XX::LINE_SCAN_TOP_TO_BOTTOM, ST77XX::COL_SCAN_LEFT_TO_RIGHT, ST77XX::COLOR_RGB);

    // // Swap endianess
    // for (int i = 0; i < 240*240; i++) {
    //     uint8_t* a = &parrots_bmp[i*2];
    //     uint8_t* b = &parrots_bmp[i*2 + 1];
    //     uint8_t tmp = *a;
    //     *a = *b;
    //     *b = tmp;
    // }

    // lcd.blit(0, 0, 240, 240, parrots_bmp);

    // // Unblank
    // lcd.setInverted(true);
    // lcd.setBlank(false);
    

    // // int testPin = 1;
    // // gpio_init(testPin);
    // // gpio_set_dir(testPin, true);
    // // while (true) {
    // //     gpio_put(testPin, 1);
    // //     sleep_ms(50);
    // //     gpio_put(testPin, 0);
    // //     sleep_ms(50);
    // // }

    // // while (true) {
    // //     lcd.sendCommand((ST77XX::Command)0x69);
    // // }

    // lcd.defineVerticalScroll(44, 134, 142);

    // //gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_RISE, true, vsync_callback);
    // //lcd.setTearingOutput(true);

    // int counter = 0;
    // while(1) {
    //     lcd.setScrollStartLine(139 - (counter++ % 134) + 44);
    //     sleep_ms(50);
    // }

    gpio_init(13);
    gpio_set_dir(13, true);
    gpio_put(13, 0);
    sleep_ms(500);
    gpio_put(13, 1);

    stdio_init_all();

    gpio_set_function(6, GPIO_FUNC_I2C);
    gpio_set_function(7, GPIO_FUNC_I2C);
    i2c_init(i2c1, 100000);

    while (true) {
        scan();
        sleep_ms(2000);
    }
    


    while(1);
}


void vsync_callback(uint gpio, uint32_t events) {
    
}
