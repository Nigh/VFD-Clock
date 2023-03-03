
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/sync.h"

#include "scheduler/uevent.h"
#include "scheduler/scheduler.h"

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812/ws2812.pio.h"

#include "platform.h"
#include "display.h"
#include "assets.h"

#include "pico/sync.h"
critical_section_t scheduler_lock;
static __inline void CRITICAL_REGION_INIT(void) {
	critical_section_init(&scheduler_lock);
}
static __inline void CRITICAL_REGION_ENTER(void) {
	critical_section_enter_blocking(&scheduler_lock);
}
static __inline void CRITICAL_REGION_EXIT(void) {
	critical_section_exit(&scheduler_lock);
}

bool timer_64hz_callback(struct repeating_timer* t) {
	uevt_bc_e(UEVT_TIMER_64HZ);
	return true;
}
bool timer_100hz_callback(struct repeating_timer* t) {
	uevt_bc_e(UEVT_TIMER_100HZ);
	return true;
}


void temperature_routine(void) {
	static uint16_t tick = 0;
	static float _t = 0;
	if(tick > 8) {
		tick = 0;
		int32_t t_adc = adc_read();
		_t = 27.0 - ((t_adc - 876) / 2.136f);
		uevt_bc(UEVT_ADC_TEMPERATURE_RESULT, &_t);
	}
	tick += 1;
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}
static inline void put_pixel(uint32_t pixel_grb) {
	pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
void led_blink_routine(void) {
	static uint8_t _tick = 0;
	_tick += 1;
	if(_tick == 55) {
		put_pixel(urgb_u32(87 / 10, 209 / 8, 180 / 18));
	}
	if(_tick == 60) {
		put_pixel(0);
		_tick = 0;
	}
}

uint8_t beep_delay = 0;
uint8_t beep_length = 0;
void btn_beep(void) {
	beep_delay = 1;
	beep_length = 10;
}

static void buzzer_routine(void) {
	if(beep_delay > 0) {
		beep_delay -= 1;
		if(beep_delay == 0) {
			if(beep_length<=0) {
				beep_length == 1;
			}
			gpio_put(BUZZER_PIN, 0);
		}
	}
	if(beep_length > 0) {
		beep_length -= 1;
		if(beep_length == 0) {
			gpio_put(BUZZER_PIN, 1);
		}
	}
}

void vfd_routine(void) {
	static uint8_t startBin = 0;
	uint8_t bb = startBin++;
	uint8_t bitmap[40];
	uint8_t* p = bitmap;
	for(uint8_t j = 0; j < 8; j++) {
		for(uint8_t i = 0; i < 5; i++) {
			*p++ = bb++;
		}
		bb += 2;
	}
	set_CGRAM_all(bitmap);
}

#include "hardware/xosc.h"
extern void btn_callback(uint gpio, uint32_t events);
void sys_init(void) {
	xosc_init();
	stdio_init_all();

	adc_init();
	adc_set_temp_sensor_enabled(true);
	adc_select_input(4);

	PIO pio = pio0;
	uint offset = pio_add_program(pio, &ws2812_program);
	ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, false);
	put_pixel(urgb_u32(10, 0, 0));
	put_pixel(0);
	display_init();

	gpio_init(BUZZER_PIN);
	gpio_set_dir(BUZZER_PIN, GPIO_OUT);
	gpio_put(BUZZER_PIN, 1);

	gpio_set_pulls(BTN_L_PIN, true, false);
	gpio_set_pulls(BTN_R_PIN, true, false);
	gpio_set_irq_enabled_with_callback(BTN_L_PIN, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
	gpio_set_irq_enabled_with_callback(BTN_R_PIN, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

	static struct repeating_timer timer;
	add_repeating_timer_ms(10, timer_100hz_callback, NULL, &timer);
}

typedef enum {
	FSM_NULL,
	FSM_BOOT,
	FSM_SLEEP,
	FSM_STOPWATCH,
	FSM_TIMER,
	FSM_POMODORO
} gstate;
gstate global_state = FSM_NULL;
gstate next_state = FSM_BOOT;

void boot_handler(uevt_t* evt) {
	static uint16_t start_timer = 0;
	static uint8_t boot_frame = 0;
	switch(evt->evt_id) {
		case UEVT_FSM_STATE_CHANGE:
			sys_init();
			start_timer = 0;
			boot_frame = 0;
			break;
		case UEVT_TIMER_100HZ:
			start_timer += 1;
			if((start_timer == 5) || (start_timer == 22)) {
				gpio_put(BUZZER_PIN, 0);
			}
			if((start_timer == 15) || start_timer == 33) {
				gpio_put(BUZZER_PIN, 1);
			}
			if((start_timer & 0x7) == 0) {
				if(boot_frame <= 3) {
					if(start_timer >= 33) {
						draw_bitmap_to_all(boot_array[boot_frame]);
						draw_update();
						boot_frame += 1;
					}
				} else if(boot_frame <= 5) {
					if(start_timer >= 108) {
						draw_bitmap_to_all(boot_array[boot_frame]);
						draw_update();
						boot_frame += 1;
					}
				} else {
					if(boot_frame <= 6) {
						draw_buffer_clear();
						draw_update();
					}
					boot_frame += 1;
					if(boot_frame > 9) {
						next_state = FSM_STOPWATCH;
					}
				}
			}
			break;
	}
}

extern void stopwatch_handler(uevt_t* evt);
// extern void timer_handler(uevt_t* evt);
// extern void pomodoro_handler(uevt_t* evt);
void fsm_handler(uevt_t* evt) {
	switch(global_state) {
		case FSM_BOOT:
			boot_handler(evt);
			break;
		case FSM_SLEEP:
			break;
		case FSM_STOPWATCH:
			stopwatch_handler(evt);
			break;
		case FSM_TIMER:
			break;
		case FSM_POMODORO:
			break;
	}
	if(next_state != global_state) {
		global_state = next_state;
		uevt_bc_e(UEVT_FSM_STATE_CHANGE);
	}
}

void main_handler(uevt_t* evt) {
	static uint8_t flag = 0;
	switch(evt->evt_id) {
		case UEVT_TIMER_4HZ:
			temperature_routine();
			break;
		case UEVT_TIMER_100HZ:
			led_blink_routine();
			buzzer_routine();
			// vfd_routine();
			flag += 1;
			if(flag == 25) {
				flag = 0;
				uevt_bc_e(UEVT_TIMER_4HZ);
			}
			break;
		case UEVT_ADC_TEMPERATURE_RESULT:
			printf("Temperature is %0.2f\n", *((float*)(evt->content)));
			break;
	}
}
#define case_evt_log(XXX) \
	case XXX:LOG_RAW("EVT:[" #XXX "]\n");break 

void log_handler(uevt_t* evt) {
	switch(evt->evt_id) {
		case_evt_log(UEVT_BTN_LEFT_DOWN);
		case_evt_log(UEVT_BTN_LEFT_RELEASE);
		case_evt_log(UEVT_BTN_LEFT_LONG);
		case_evt_log(UEVT_BTN_LEFT_REPEAT);
		case_evt_log(UEVT_BTN_RIGHT_DOWN);
		case_evt_log(UEVT_BTN_RIGHT_RELEASE);
		case_evt_log(UEVT_BTN_RIGHT_LONG);
		case_evt_log(UEVT_BTN_RIGHT_REPEAT);
		case_evt_log(UEVT_BTN_BOTH_DOWN);
		case_evt_log(UEVT_BTN_BOTH_RELEASE);
		case_evt_log(UEVT_BTN_BOTH_LONG);
		case_evt_log(UEVT_BTN_BOTH_REPEAT);
	}
}
extern void btn_handler(uevt_t* evt);
int main() {
	CRITICAL_REGION_INIT();
	app_sched_init();
	user_event_init();
	user_event_handler_regist(log_handler);
	user_event_handler_regist(btn_handler);
	user_event_handler_regist(main_handler);
	user_event_handler_regist(fsm_handler);
	uevt_bc_e(UEVT_FSM_NULL);

	while(true) {
		app_sched_execute();
		__wfi();
	}
}
