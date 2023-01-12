
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "platform.h"

#include "display.h"

#define SPI_USE spi1

uint8_t spi_buffer[256];
static unsigned char lookup[16] = {
	0x0,
	0x8,
	0x4,
	0xc,
	0x2,
	0xa,
	0x6,
	0xe,
	0x1,
	0x9,
	0x5,
	0xd,
	0x3,
	0xb,
	0x7,
	0xf,
};
uint8_t reverse(uint8_t n) {
	// Reverse the top and bottom nibble then swap them.
	return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}

__inline void __not_in_flash_func(spi_write)(const uint8_t* src, size_t len) {
	for(uint8_t i = 0; i < len; i++) {
		spi_buffer[i] = reverse(src[i]);
	}
	spi_write_blocking(SPI_USE, spi_buffer, len);
}

__inline void __not_in_flash_func(spi_write_byte)(uint8_t src) {
	uint8_t lsb_byte = reverse(src);
	spi_write_blocking(SPI_USE, (const uint8_t*)&lsb_byte, 1);
	sleep_us(2);
}

void set_dimming_level(uint8_t lvl) {
	uint8_t buf[2] = { 0xE4, lvl };
	gpio_put(VFD_CS_PIN, 0);
	// for (uint8_t i = 0; i < 2; i++) {
	// 	spi_write_byte(buf[i]);
	// }
	spi_write(buf, 2);
	gpio_put(VFD_CS_PIN, 1);
}

void set_timing(void) {
	uint8_t buf[2] = { 0xE0, 0x07 };
	gpio_put(VFD_CS_PIN, 0);
	// for (uint8_t i = 0; i < 2; i++) {
	// 	spi_write_byte(buf[i]);
	// }
	spi_write(buf, 2);
	gpio_put(VFD_CS_PIN, 1);
}

void set_display_onoff(uint8_t on) {
	uint8_t buf[1];
	gpio_put(VFD_CS_PIN, 0);
	if(on > 0) {
		buf[0] = 0xE8 | 0x00;
	} else {
		buf[0] = 0xE8 | 0x20;
	}
	// spi_write_byte(buf[0]);
	spi_write(buf, 1);
	gpio_put(VFD_CS_PIN, 1);
}

void set_display_sleep(uint8_t slp) {
	uint8_t buf[1];
	gpio_put(VFD_CS_PIN, 0);
	if(slp > 0) {
		buf[0] = 0xEC | 0x01;
	} else {
		buf[0] = 0xEC | 0x00;
	}
	// spi_write_byte(buf[0]);
	spi_write(buf, 1);
	gpio_put(VFD_CS_PIN, 1);
}

static void set_DRAM_map(void) {
	uint8_t buf[9] = { 0x20, 0, 1, 2, 3, 4, 5, 6, 7 };
	gpio_put(VFD_CS_PIN, 0);
	// for (uint8_t i = 0; i < 9; i++) {
	// 	spi_write_byte(buf[i]);
	// }
	spi_write(buf, 9);
	gpio_put(VFD_CS_PIN, 1);
}

void set_CGRAM(uint8_t* in, uint8_t start, uint8_t len) {
	uint8_t buf[41] = { 0x40 + start };
	if(start >= 8) {
		return;
	}
	if(start + len > 8) {
		len = 8 - start;
		if(len == 0) {
			return;
		}
	}
	for(uint8_t i = 0; i < len * 5; i++) {
		buf[i + 1] = in[i];
	}
	gpio_put(VFD_CS_PIN, 0);
	// for (uint8_t i = 0; i < 1 + len * 5; i++) {
	// 	spi_write_byte(buf[i]);
	// }
	spi_write(buf, 1 + len * 5);
	gpio_put(VFD_CS_PIN, 1);
}

void set_CGRAM_all(uint8_t* in) {
	set_CGRAM(in, 0, 8);
}

static void test_all_on(void) {
	uint8_t buf[1] = { 0xE9 };
	gpio_put(VFD_CS_PIN, 0);
	spi_write(buf, 1);
	gpio_put(VFD_CS_PIN, 1);
}
static void test_cgram_on(void) {
	uint8_t buf[40];
	for(uint8_t i = 0; i < 40; i++) {
		buf[i] = i;
	}
	set_CGRAM_all(buf);
}

void display_init(void) {
	uint8_t _a = 0x00;
	printf("SPI baud rate = %dHz\n", spi_init(SPI_USE, 400000));
	spi_set_format(SPI_USE, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
	gpio_set_function(VFD_SDA_PIN, GPIO_FUNC_SPI);
	gpio_set_function(VFD_CLK_PIN, GPIO_FUNC_SPI);

	gpio_init(VFD_CS_PIN);
	gpio_set_dir(VFD_CS_PIN, GPIO_OUT);
	gpio_init(VFD_RST_PIN);
	gpio_set_dir(VFD_RST_PIN, GPIO_OUT);
	gpio_init(VFD_HV_PIN);
	gpio_set_dir(VFD_HV_PIN, GPIO_OUT);

	gpio_put(VFD_RST_PIN, 0);
	gpio_put(VFD_CS_PIN, 1);
	gpio_put(VFD_HV_PIN, 1);
	spi_write(&_a, 1);
	sleep_ms(10);
	gpio_put(VFD_RST_PIN, 1);
	sleep_ms(5);
	// DCRAM Map = CGRAM->display
	// 配置映射，将CGRAM映射到显示，之后可以直接控制CGRAM实现控制显示内容

	set_timing();
	sleep_us(4);
	set_dimming_level(180);
	sleep_us(4);
	// test_all_on();
	// sleep_us(4);
	// test_cgram_on();
	// sleep_us(4);
	set_DRAM_map();
	sleep_us(4);
	set_display_onoff(1);
	sleep_us(4);
}
