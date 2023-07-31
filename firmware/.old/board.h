#pragma once
#include <stdint.h>

// Config
#define FPGA_UPLOAD_BAUDRATE    16000000

// FPGA
#define FPGA_CDONE  14
#define FPGA_RST    13
#define FPGA_MISO   12
#define FPGA_MOSI   11
#define FPGA_CLK    10
#define FPGA_CS     9

// Buttons
#define BTN_0       17
#define BTN_1       16

// Switches and Buttons
#define SW_0        18
#define SW_1        19
#define SW_2        20
#define SW_3        21
#define SW_4        22
#define SW_5        26
#define SW_6        27
#define SW_7        28

// 7 Segment
#define SEG_A_EN    8
#define SEG_B_EN    15

namespace board {
    void init();
}

namespace fpga {
    void init();
    void reset();
    void select(bool select);
    void configure(uint8_t* bitstream, int len);
}

namespace seven_segment {
    void init();
    void enable(bool enableSegA, bool enableSegB);
}