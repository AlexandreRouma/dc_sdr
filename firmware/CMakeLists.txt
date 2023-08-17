cmake_minimum_required(VERSION 3.13)
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(firmware)
pico_sdk_init()

file(GLOB_RECURSE SRC "src/*.cpp")
file(GLOB_RECURSE LIB "lib/*.cpp")


add_executable(${PROJECT_NAME} ${SRC} ${LIB})

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/lib/ws2812/ws2812.pio OUTPUT_DIR lib/ws2812)

target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_spi hardware_i2c hardware_pwm hardware_adc hardware_dma pico_multicore hardware_pio)

target_include_directories(${PROJECT_NAME} PRIVATE "lib/")

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})