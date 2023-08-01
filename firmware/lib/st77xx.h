#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <hardware/spi.h>

#define ST7789_RESET_HW_US  10000
#define ST7789_RESET_SW_US  10000

namespace ST77XX {
    // Note: This driver doesn't implement reading so the associated commands are not listed for conciseness
    enum Command : uint8_t {
        CMD_NOP         = 0x00,
        CMD_SWRESET     = 0x01,
        CMD_SLPIN       = 0x10,
        CMD_SLPOUT      = 0x11,
        CMD_PLTON       = 0x12,
        CMD_NORON       = 0x13,
        CMD_INVOFF      = 0x20,
        CMD_INVON       = 0x21,
        CMD_GAMSET      = 0x26,
        CMD_DISPOFF     = 0x28,
        CMD_DISPON      = 0x29,
        CMD_CASET       = 0x2A,
        CMD_RASET       = 0x2B,
        CMD_RAMWR       = 0x2C,
        CMD_PTLAR       = 0x30,
        CMD_VSCRDEF     = 0x33,
        CMD_TEOFF       = 0x34,
        CMD_TEON        = 0x35,
        CMD_MADCTL      = 0x36,
        CMD_VSCSAD      = 0x37,
        CMD_IDMOFF      = 0x38,
        CMD_IDMON       = 0x39,
        CMD_COLMOD      = 0x3A,
        CMD_RAMWRC      = 0x3C,
        CMD_TESCAN      = 0x44,
        CMD_WRDISBV     = 0x51,
        CMD_WRCTRLD     = 0x53,
        CMD_WRCACE      = 0x55,
        CMD_WRCABCMB    = 0x5E
    };

    enum DCState : bool {
        DC_COMMAND  = 0,
        DC_DATA     = 1
    };

    enum ResetType {
        RESET_HARDWARE,
        RESET_SOFTWARE
    };

    enum GammaCurve : uint8_t {
        GAMMA_CURVE_1   = (1 << 0),
        GAMMA_CURVE_2   = (1 << 1),
        GAMMA_CURVE_3   = (1 << 2),
        GAMMA_CURVE_4   = (1 << 3)
    };

    enum LineScanOrder : bool {
        LINE_SCAN_TOP_TO_BOTTOM  = 0,
        LINE_SCAN_BOTTOM_TO_TOP  = 1
    };

    enum ColumnScanOrder : bool {
        COL_SCAN_LEFT_TO_RIGHT  = 0,
        COL_SCAN_RIGHT_TO_LEFT  = 1
    };

    enum ColorOrder : bool {
        COLOR_RGB   = 0,
        COLOR_BGR   = 1
    };

    enum ColorMode : uint8_t {
        COLOR_MODE_12BIT  = 0b011,
        COLOR_MODE_16BIT  = 0b101,
        COLOR_MODE_18BIT  = 0b110
    };

    class Driver {
    public:
        // TODO: This is shit
        Driver() {}
        Driver(int mosiGPIO, int clkGPIO, int dcGPIO, int rstGPIO, int csGPIO = -1, int blGPIO = -1, int teGPIO = -1);
        //Driver(Driver& b) = delete;
        Driver(Driver&& b);
        ~Driver() {}

        Driver& operator=(const Driver&& b);

        /**
         * Perform a software or hardware reset. Takes 120ms.
         * @param type Reset type, either `RESET_HARDWARE` or `RESET_SOFTWARE´.
        */
        void reset(ResetType type);

        /**
         * Enable or disable sleep mode. Takes 5ms to enter sleep mode and 120ms to exit.
         * @param sleep `true` to enter sleep mode, `false` to exit.
        */
        void setSleep(bool sleep);

        /**
         * Enable or disable partial display mode.
         * @param partial `true` to enable partial mode, `false` to disable.
        */
        void setPartial(bool partial);

        /**
         * Enable or disable color inversion.
         * @param inverted `true` for inverted colors, `false` for normal colors.
        */
        void setInverted(bool inverted);

        /**
         * Select which gamma correction curve to use.
         * @param curve Gamma curve to use.
        */
        void setGammaCurve(GammaCurve curve);

        /**
         * Enable or disable display blanking. Switches from displaying the framebuffer to displaying black.
         * @param blank `true` to blank, `false` to unblank.
        */
        void setBlank(bool blank);

        /**
         * Blit a bitmap into VRAM. In 12bit color mode, please ensure that an even number of pixels gets transfered.
         * @param x X coordinate of the top left of the write window.
         * @param y Y coordinate of the top left of the write window.
         * @param w Width of the write window.
         * @param h Height of the write window.
         * @param bitmap Bitmap data to blit in the write window.
        */
        void blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* bitmap);

        /**
         * Define the partial display mode window.
         * @param startRow First row of displayed data.
         * @param endRow Last row of displayed data.
        */
        void setPartialArea(uint16_t startRow, uint16_t endRow);

        /**
         * TODO
        */
        void defineVerticalScroll(uint16_t topFixedArea, uint16_t verticalScrollArea, uint16_t bottomFixedArea);

        /**
         * Enable or disable the tearing output.
         * @param enabled `true` to enable, `false` to disable.
        */
        void setTearingOutput(bool enabled);

        /**
         * TODO
        */
        void setMemoryAccess(bool flipX, bool flipY, bool swapXY, LineScanOrder lineScanOrder, ColumnScanOrder columnScanOrder, ColorOrder colorOrder);

        /**
         * Select the line to draw right after the Top Fixed Area.
         * @param line Line to be drawn after the TFA.
        */
        void setScrollStartLine(uint16_t line);

        /**
         * TODO
        */
        void setIdle(bool idle);

        /**
         * TODO
        */
        void setColorMode(ColorMode mode);

        void fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

    //protected:
        void sendCommand(Command cmd, const uint8_t* data = NULL, int len = 0);

        void setColumnAddress(uint16_t start, uint16_t end);
        void setRowAddress(uint16_t start, uint16_t end);

        spi_inst_t* SPI;
        int BL_GPIO;
        int DC_GPIO;
        int RST_GPIO;
        int TE_GPIO;

        ColorMode colorMode;
    };
}

