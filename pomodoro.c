
#include "platform.h"
#include "scheduler/uevent.h"
#include "ui.h"
#include "assets.h"

#define AUTO_POMODORO (0)
#define AUTO_BREAK (0)

#define MAX_ROUTINE (4)
#define WORK_MINUTE (25)
#define SHORT_BREAK_MINUTE (5)
#define LONG_BREAK_MINUTE (15)

// 0:clear 1:run 2:pause
static uint8_t running = 0;
//
static uint8_t routine = 0;
static uint8_t isbreak = 0;

static uint8_t minute = WORK_MINUTE;
static uint8_t second = 0;

static uint8_t colon_show = 1;

#define POMODORO_FONT H7_Sq_array
void pomodoro_redraw(void) {
	if(colon_show) {
		draw_bitmap_to(&colon_bmp, 3);
	} else {
		draw_clear_to(3);
	}
	draw_bitmap_to(POMODORO_FONT[minute / 10], 1);
	draw_bitmap_to(POMODORO_FONT[minute % 10], 2);
	draw_bitmap_to(POMODORO_FONT[second / 10], 4);
	draw_bitmap_to(POMODORO_FONT[second % 10], 5);
	draw_update();
}

static void timer_update(void) {
	second = 0;
	if(isbreak) {
		if(routine < MAX_ROUTINE) {
			minute = SHORT_BREAK_MINUTE;
		} else {
			minute = LONG_BREAK_MINUTE;
			routine = 0;
		}
	} else {
		minute = WORK_MINUTE;
	}
}

static void pomodoro_1hz(void) {
	if(running == 1) {
		colon_show = 1;
		if(second > 0) {
			second -= 1;
		} else if(minute > 0) {
			minute -= 1;
			second = 59;
		} else {
			if(isbreak) {
				// TODO: break ALARM
				if(AUTO_POMODORO > 0) {
					isbreak = 0;
					timer_update();
				} else {
					running = 2;
				}
			} else {
				// TODO: work ALARM
				if(AUTO_BREAK > 0) {
					isbreak = 1;
					timer_update();
				} else {
					running = 2;
				}
			}
		}
		pomodoro_redraw();
	}
}

static void start(void) {
	running = 1;
}

static void restart(void) {
	minute = WORK_MINUTE;
	second = 0;
	routine = 0;
	isbreak = 0;
	start();
}

static void continues(void) {
	if(minute == 0 && second == 0) {
		isbreak = !isbreak;
		if(!isbreak) {
			routine += 1;
		}
		timer_update();
		start();
	} else {
		start();
	}
}

static void pause(void) {
	running = 2;
}

static void clear(void) {
	minute = WORK_MINUTE;
	second = 0;
	running = 0;
}

void pomodoro_handler(uevt_t* evt) {
	static uint8_t count = 0;
	switch(evt->evt_id) {
		case UEVT_BTN_LEFT_DOWN:
			btn_beep();
			if(running == 0) {
				restart();
			} else if(running == 2) {
				continues();
			} else {
				pause();
			}
			break;
		case UEVT_BTN_RIGHT_DOWN:
			if(running == 2) {
				btn_beep();
				clear();
				pomodoro_redraw();
			} else if(running == 0) {
				btn_beep();
				// TODO: 切换模式
			}
			break;
		case UEVT_FSM_STATE_CHANGE:
			pomodoro_redraw();
			break;
		case UEVT_TIMER_4HZ:
			if(running == 1) {
				count += 1;
				if((count & 0x3) == 0) {
					pomodoro_1hz();
				}
				if((count & 0x3) == 2) {
					colon_show = !colon_show;
					pomodoro_redraw();
				}
			}
			break;
	}
}
