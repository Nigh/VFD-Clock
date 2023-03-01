
#include "platform.h"
#include "display.h"
#include "scheduler/uevent.h"

void stopwatch_handler(uevt_t* evt) {
	switch(evt->evt_id) {
		case UEVT_FSM_STATE_CHANGE:
			// stopwatch_redraw();
			break;
		case UEVT_TIMER_4HZ:
			ui_test();
			break;
		case UEVT_TIMER_64HZ:
			break;
	}
}
