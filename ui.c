
#include "ui.h"

#include "display.h"


static uint8_t display_buffer[40];

static void draw_bitmap_to(sBITMAP* bp, uint8_t pos) {
	if(pos>7) {
		return;
	}
	uint8_t w = bp->w;
	uint8_t* pb_s = bp->map;
	uint8_t* pb_t = &display_buffer[5*pos];
	if(w > 5) {
		w = 5;
	}
	for (uint8_t i = 0; i < w; i++) {
		*pb_t++ = *pb_s++;
	}
}

void draw_buffer_clear(void) {
	for (uint8_t i = 0; i < 40; i++) {
		display_buffer[i] = 0;
	}
}
void draw_bitmap_to_all(sBITMAP* bp) {
	for (uint8_t i = 0; i < 8; i++) {
		draw_bitmap_to(bp, i);
	}
}
void draw_update(void) {
	set_CGRAM_all(display_buffer);
}

uint8_t test_array[8] = {0,1,2,3,4,5,6,7};
void ui_test(void) {
	for (uint8_t i = 0; i < 8; i++) {
		draw_bitmap_to(H6_array[test_array[i]], i);
		test_array[i]+=1;
		if(test_array[i] >= 10) {
			test_array[i] = 0;
		}
	}
	set_CGRAM_all(display_buffer);
}

