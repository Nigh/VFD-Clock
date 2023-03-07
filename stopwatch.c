
#include "platform.h"
#include "scheduler/uevent.h"
#include "ui.h"
#include "assets.h"

static uint16_t minute = 0;
static uint8_t sec = 0;
static uint8_t msec = 0;

static bool colon_show = true;

// 0:clear 1:running 2:pause
static uint8_t running = 0;

#define STOPWATCH_FONT H7_Sq_array
#define MSEC_FONT H5_bottom_array
void stopwatch_redraw(void) {
	if(colon_show) {
		draw_bitmap_to(&colon_bmp, 3);
	} else {
		draw_clear_to(3);
	}
	draw_bitmap_to(STOPWATCH_FONT[minute / 100], 0);
	draw_bitmap_to(STOPWATCH_FONT[(minute / 10) % 10], 1);
	draw_bitmap_to(STOPWATCH_FONT[minute % 10], 2);
	draw_bitmap_to(STOPWATCH_FONT[sec / 10], 4);
	draw_bitmap_to(STOPWATCH_FONT[sec % 10], 5);
	draw_bitmap_to(MSEC_FONT[msec / 10], 6);
	draw_bitmap_to(MSEC_FONT[msec % 10], 7);
	draw_update();
}

static void pause(void) {
	running = 2;
	colon_show = true;
	stopwatch_redraw();
}

static void clear(void) {
	running = 0;
	colon_show = true;
	minute = 0;
	sec = 0;
	msec = 0;
	stopwatch_redraw();
}

static void start(void) {
	running = 1;
	colon_show = false;
	stopwatch_redraw();
}

void stopwatch_handler(uevt_t* evt) {
	switch(evt->evt_id) {
		case UEVT_BTN_LEFT_DOWN:
			btn_beep();
			if(running != 1) {
				start();
			} else {
				pause();
			}
			break;
		case UEVT_BTN_RIGHT_DOWN:
			if(running == 2) {
				btn_beep();
				clear();
			} else if(running == 0) {
				btn_beep();
				// TODO: 切换模式
			}
			break;
		case UEVT_FSM_STATE_CHANGE:
			colon_show = true;
			stopwatch_redraw();
			break;
		case UEVT_TIMER_100HZ:
			if(running == 1) {
				msec += 1;
				if(msec >= 100) {
					colon_show = true;
					msec = 0;
					sec += 1;
					if(sec >= 60) {
						sec = 0;
						minute += 1;
						if(minute >= 99) {
							pause();
						}
					}
				}
				if((msec == 25) || (msec == 75)) {
					colon_show = false;
				}
				if((msec == 0) || (msec == 50)) {
					colon_show = true;
				}
				stopwatch_redraw();
			}
			break;
	}
}
