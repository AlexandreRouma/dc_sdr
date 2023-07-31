#include "tpl0401.h"

namespace TPL0401 {
    Driver::Driver(int sdaGPIO, int sclGPIO, Address addr) :
        SDA_GPIO(sdaGPIO),
        SCL_GPIO(sclGPIO)
    {
        // Configure GPIOs

        // Init i2c
    }

    void set(uint8_t value) {
        // TODO
    }
}