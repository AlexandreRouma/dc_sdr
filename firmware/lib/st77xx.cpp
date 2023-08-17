#include "st77xx.h"
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include "pico/stdlib.h"

namespace ST77XX {
    Driver::Driver(int mosiGPIO, int clkGPIO, int dcGPIO, int rstGPIO, int csGPIO, int blGPIO, int teGPIO) :
        SPI(((mosiGPIO >> 3) & 1) ? spi1 : spi0),
        DC_GPIO(dcGPIO),
        RST_GPIO(rstGPIO),
        BL_GPIO(blGPIO),
        TE_GPIO(teGPIO)
    {
        // Configure GPIOs
        gpio_set_function(mosiGPIO, GPIO_FUNC_SPI);
        gpio_set_function(clkGPIO, GPIO_FUNC_SPI);

        gpio_init(DC_GPIO);
        gpio_set_dir(DC_GPIO, true);
        gpio_put(DC_GPIO, DC_COMMAND);
        
        gpio_init(RST_GPIO);
        gpio_set_dir(RST_GPIO, true);
        gpio_put(RST_GPIO, 0);

        gpio_init(BL_GPIO); // TODO: PWM
        gpio_set_dir(BL_GPIO, true);
        gpio_put(BL_GPIO, 1);

        gpio_init(TE_GPIO);
        gpio_set_dir(TE_GPIO, false);

        //TODO: Finish the rest of the GPIOs

        //Configure SPI bus
        spi_init(SPI, 62500000);
        spi_set_format(SPI, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);

        // Initialize the display
        // TODO
    }

    Driver& Driver::operator=(const Driver&& b) {
        SPI = b.SPI;
        BL_GPIO = b.BL_GPIO;
        DC_GPIO = b.DC_GPIO;
        RST_GPIO = b.RST_GPIO;
        TE_GPIO = b.TE_GPIO;
        colorMode = b.colorMode;
        return *this;
    }

    void Driver::reset(ResetType type) {
        if (type == RESET_HARDWARE) {
            // Strobe reset line
            gpio_put(RST_GPIO, 0);
            sleep_us(20);
            gpio_put(RST_GPIO, 1);
        }
        else {
            // Issue software reset command
            sendCommand(CMD_SWRESET);
        }

        // Wait for reset to finish
        sleep_ms(120);
    }

    void Driver::setSleep(bool sleep) {
        // Issue command
        sendCommand(sleep ? CMD_SLPIN : CMD_SLPOUT);

        // Wait for command to complete
        sleep_ms(sleep ? 5 : 120);
    }

    void Driver::setPartial(bool partial) {
        // Issue command
        sendCommand(partial ? CMD_PLTON : CMD_NORON);
    }

    void Driver::setInverted(bool inverted) {
        // Issue command
        sendCommand(inverted ? CMD_INVON : CMD_INVOFF);
    }

    void Driver::setGammaCurve(GammaCurve curve) {
        // Issue command
        sendCommand(CMD_GAMSET, (uint8_t*)&curve, 1);
    }

    void Driver::setBlank(bool blank) {
        // Issue command
        sendCommand(blank ? CMD_DISPOFF : CMD_DISPON);
    }

    void Driver::blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* bitmap) {
        // Update start address
        setColumnAddress(x, x + w - 1);
        setRowAddress(y, y + h - 1);

        // Write bitmap data
        sendCommand(CMD_RAMWR, bitmap, w*h*2/*TODO*/);

        // Send NOP to finish the command (TODO: Check if actually needed)
        sendCommand(CMD_NOP);
    }

    void Driver::setPartialArea(uint16_t startRow, uint16_t endRow) {
        // Encode parameters
        uint8_t args[4]  = {
            startRow >> 8,
            startRow & 0xFF,
            endRow >> 8,
            endRow & 0xFF
        };

        // Issue command
        sendCommand(CMD_PTLAR, args, sizeof(args));
    }

    void Driver::defineVerticalScroll(uint16_t topFixedArea, uint16_t verticalScrollArea, uint16_t bottomFixedArea) {
        // Encode parameters
        uint8_t args[6]  = {
            topFixedArea >> 8,
            topFixedArea & 0xFF,
            verticalScrollArea >> 8,
            verticalScrollArea & 0xFF,
            bottomFixedArea >> 8,
            bottomFixedArea & 0xFF
        };

        // Issue command
        sendCommand(CMD_VSCRDEF, args, sizeof(args));
    }

    void Driver::setTearingOutput(bool enabled) {
        // Issue command
        sendCommand(enabled ? CMD_TEON : CMD_TEOFF);
    }

    void Driver::setMemoryAccess(bool flipX, bool flipY, bool swapXY, LineScanOrder lineScanOrder, ColumnScanOrder columnScanOrder, ColorOrder colorOrder) {
        // Encode parameters
        uint8_t reg = 0;
        if (columnScanOrder == COL_SCAN_RIGHT_TO_LEFT) { reg |= (1 << 2); }
        if (colorOrder == COLOR_BGR) { reg |= (1 << 3); }
        if (lineScanOrder == LINE_SCAN_BOTTOM_TO_TOP) { reg |= (1 << 4); }
        if (swapXY) { reg |= (1 << 5); }
        if (flipX) { reg |= (1 << 6); }
        if (flipY) { reg |= (1 << 7); }

        // Issue command
        sendCommand(CMD_MADCTL, &reg, 1);
    }

    void Driver::setScrollStartLine(uint16_t line) {
        // Encode parameter
        uint8_t args[6]  = {
            line >> 8,
            line & 0xFF
        };

        // Issue command
        sendCommand(CMD_VSCSAD, args, sizeof(args));
    }

    void Driver::setColorMode(ColorMode mode) {
        // Encode parameter
        uint8_t param = ((uint8_t)mode << 4) | (uint8_t)mode;

        // Issue command
        sendCommand(CMD_COLMOD, &param, 1);
    }

    void Driver::fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
        // Update start address
        setColumnAddress(x, x + w - 1);
        setRowAddress(y, y + h - 1);

        // Enter command mode
        gpio_put(DC_GPIO, DC_COMMAND);

        // Send command byte
        uint8_t data = CMD_RAMWR;
        spi_write_blocking(SPI, &data, 1);

        // Enter data mode
        gpio_put(DC_GPIO, DC_DATA);

        // Send all pixels
        int pcount = w*h;
        for (int i = 0; i < pcount; i++) {
            spi_write_blocking(SPI, (uint8_t*)&color, 2);
        }

        // Send NOP to finish the command (TODO: Check if actually needed)
        sendCommand(CMD_NOP);
    }

    void Driver::setColumnAddress(uint16_t start, uint16_t end) {
        // Encode parameters
        uint8_t args[4]  = {
            start >> 8,
            start & 0xFF,
            end >> 8,
            end & 0xFF
        };

        // Issue command
        sendCommand(CMD_CASET, args, sizeof(args));
    }

    void Driver::setRowAddress(uint16_t start, uint16_t end) {
        // Encode parameters
        uint8_t args[4]  = {
            start >> 8,
            start & 0xFF,
            end >> 8,
            end & 0xFF
        };

        // Issue command
        sendCommand(CMD_RASET, args, sizeof(args));
    }

    void Driver::setIdle(bool idle) {
        sendCommand(idle ? CMD_IDMON : CMD_IDMOFF);
    }

    void Driver::sendCommand(Command cmd, const uint8_t* data, int len) {
        // Enter command mode
        gpio_put(DC_GPIO, DC_COMMAND);

        // Send command byte
        spi_write_blocking(SPI, (uint8_t*)&cmd, 1);

        // If data is given, transmit it as well
        if (len) {
            // Enter data mode
            gpio_put(DC_GPIO, DC_DATA);

            // Send bytes
            spi_write_blocking(SPI, data, len);
        }

        //sleep_ms(120);
    }
}