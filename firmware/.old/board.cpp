#include "board.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

namespace board {
    void init() {
        fpga::init();
        seven_segment::init();
    }
}

namespace fpga {
    void init() {
        // Wait 100ms to make sure the FPGA is properly powered up
        sleep_ms(100);

        // Init FPGA control pins
        gpio_init(FPGA_CDONE);
        gpio_init(FPGA_RST);
        gpio_init(FPGA_CS);
        gpio_set_dir(FPGA_CDONE, GPIO_IN);
        gpio_set_dir(FPGA_RST, GPIO_OUT);
        gpio_set_dir(FPGA_CS, GPIO_OUT);
        gpio_put(FPGA_RST, 1);
        gpio_put(FPGA_CS, 1);
        
        // Init SPI controller
        gpio_set_function(FPGA_MISO, GPIO_FUNC_SPI);
        gpio_set_function(FPGA_MOSI, GPIO_FUNC_SPI);
        gpio_set_function(FPGA_CLK, GPIO_FUNC_SPI);
        spi_init(spi1, FPGA_UPLOAD_BAUDRATE);
    }

    void reset() {
        gpio_put(FPGA_RST, 0);
        sleep_us(1);
        gpio_put(FPGA_RST, 1);
        sleep_ms(2);
    }

    void select(bool select) {
        gpio_put(FPGA_CS, !select);
    }

    void configure(uint8_t* bitstream, int len) {
        const uint8_t dummy_byte = 0;

        // Reset FPGA into slave mode
        select(true);
        reset();
        
        // Send 8 clock cycle with SS high for intialization
        select(false);
        spi_write_blocking(spi1, &dummy_byte, 1);
        select(true);

        // Write bitstream
        spi_write_blocking(spi1, bitstream, len);

        // Send clock cycles until CDONE goes high
        while (!gpio_get(FPGA_CDONE)) {
            spi_write_blocking(spi1, &dummy_byte, 1);
        }

        // Add 64 clock cycle
        const uint8_t dummy_64[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
        spi_write_blocking(spi1, dummy_64, 8);
    }
}

namespace seven_segment {
    void init() {
        gpio_init(SEG_A_EN);
        gpio_init(SEG_B_EN);
        gpio_set_dir(SEG_A_EN, GPIO_OUT);
        gpio_set_dir(SEG_B_EN, GPIO_OUT);
        enable(false, false);
    }

    void enable(bool enableSegA, bool enableSegB) {
        gpio_put(SEG_A_EN, enableSegA);
        gpio_put(SEG_B_EN, enableSegB);
    }
}