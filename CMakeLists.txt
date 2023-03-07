cmake_minimum_required(VERSION 3.16)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(VFD_Clock C CXX ASM)

pico_sdk_init()

set(CMAKE_BUILD_TYPE "Release")

add_definitions(-DG_LOG_ENABLED=1 -DEVT_LOG_ENABLED=0)
add_subdirectory(scheduler)

add_executable(VFD_Clock
	main.c
	display.c
	assets.c
	ui.c
	btn.c
	stopwatch.c
	pomodoro.c
)

# Link with the pico stdlib
# hardware_clocks hardware_i2c hardware_pio hardware_interp hardware_rtc hardware_uart hardware_irq hardware_resets
# pico_unique_id
# tinyusb_host tinyusb_device tinyusb_board
target_link_libraries(VFD_Clock
	scheduler
	pico_stdlib
	pico_sync
	hardware_adc
	hardware_sync
	hardware_spi
	hardware_clocks
	hardware_xosc
	hardware_pio
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(VFD_Clock)

# Enable USB serial
pico_enable_stdio_usb(VFD_Clock 1)
pico_enable_stdio_uart(VFD_Clock 0)
