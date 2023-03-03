
#include "platform.h"
#include "hardware/gpio.h"
#include "scheduler/uevent.h"


#define BUTTON_NUMBER (2)
#define BTN_LEFT_IDX (0)
#define BTN_RIGHT_IDX (1)

// 等待所有按键弹起，在此之前不产生按键事件
static bool button_wait_release = false;

static uint8_t button_current_valid = 0x00;
static uint8_t button_old_down = 0x00;
static uint8_t button_current_down = 0x00;
static uint16_t button_press_time[BUTTON_NUMBER] = { 0 };
static uint8_t button_repeat_count[BUTTON_NUMBER] = { 0 };
static uint8_t button_release_count[BUTTON_NUMBER] = { 0 };

void btn_wait_release(void) {
	button_wait_release = true;
}

#define BTN_BIT(x) (0x01 << x)
volatile uint8_t btn_int_valid = 0xFF;

static bool btn_l_pin_get(void) {
	return gpio_get(BTN_L_PIN);
}
static bool btn_r_pin_get(void) {
	return gpio_get(BTN_R_PIN);
}
static bool (*const is_button_pin_released[BUTTON_NUMBER])(void) = {
	btn_l_pin_get,
	btn_r_pin_get
};

void btn_callback(uint gpio, uint32_t events) {
	switch(gpio) {
		case BTN_L_PIN:
			if(btn_int_valid & BTN_BIT(BTN_LEFT_IDX)) {
				btn_int_valid &= 0xFF ^ BTN_BIT(BTN_LEFT_IDX);
				button_current_down |= BTN_BIT(BTN_LEFT_IDX);
				button_release_count[BTN_LEFT_IDX] = 0;
				uevt_bc_e(UEVT_BTN_LEFT_DOWN);
			}
			break;
		case BTN_R_PIN:
			if(btn_int_valid & BTN_BIT(BTN_RIGHT_IDX)) {
				btn_int_valid &= 0xFF ^ BTN_BIT(BTN_RIGHT_IDX);
				button_current_down |= BTN_BIT(BTN_RIGHT_IDX);
				button_release_count[BTN_RIGHT_IDX] = 0;
				uevt_bc_e(UEVT_BTN_RIGHT_DOWN);
			}
			break;
	}
}

void btn_handler(uevt_t* evt) {
	switch(evt->evt_id) {
		case UEVT_TIMER_100HZ:
			button_old_down = button_current_down;
			for (uint8_t i = 0; i < BUTTON_NUMBER; i++) {
				if(is_button_pin_released[i]()) {
					button_release_count[i] += 1;
					if(button_release_count[i] >= 5) {
						button_current_down &= ~(0x01 << i);
						btn_int_valid |= BTN_BIT(i);
					}
				} else {
					button_release_count[i] = 0;
				}
			}
			if(((button_old_down & BTN_BIT(BTN_LEFT_IDX)) > 0)
			&& ((button_current_down & BTN_BIT(BTN_LEFT_IDX)) == 0)) {
				uevt_bc_e(UEVT_BTN_LEFT_RELEASE);
			}
			if(((button_old_down & BTN_BIT(BTN_RIGHT_IDX)) > 0)
			&& ((button_current_down & BTN_BIT(BTN_RIGHT_IDX)) == 0)) {
				uevt_bc_e(UEVT_BTN_RIGHT_RELEASE);
			}
			break;
		case UEVT_BTN_LEFT_DOWN:
			break;
		case UEVT_BTN_RIGHT_DOWN:
			break;
	}
}
