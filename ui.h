
#ifndef _UI_H_
#define _UI_H_

#include "assets.h"

extern void ui_test(void);
extern void draw_bitmap_to(sBITMAP* bp, uint8_t pos);
extern void draw_clear_to(uint8_t pos);
extern void draw_buffer_clear(void);
extern void draw_bitmap_to_all(sBITMAP* bp);
extern void draw_update(void);

#endif
