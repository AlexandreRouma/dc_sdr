#pragma once
#include <stdint.h>

namespace TPL0401 {
    enum Address : uint8_t {
        ADDR_A  = 0x00, // TODO: Address here
        ADDR_B  = 0x01
    };

    class Driver {
    public:
        Driver(int sdaGPIO, int sclGPIO, Address addr);

        void set(uint8_t value);

    private:
        const int SDA_GPIO;
        const int SCL_GPIO;
    };
}