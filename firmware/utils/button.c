#include <stdint.h>
#include <avr/interrupt.h>

#include "button.h"
#include "rtc.h"
#include "event_queue.h"
#include "../board.h"

typedef struct {
    // bit-masks that indicate a given button's state
    uint8_t debouncing; // is waiting on timer to finish debouncing
} state_t;

static state_t State;

static timer_t debounce_timer;


void button_init(void) {
    BUTTON_PORT.PINCONFIG = PORT_PULLUPEN_bm | PORT_INVEN_bm;
    BUTTON_PORT.PINCTRLUPD = BUTTON_MASK;

    // Set all buttons to interrupt on change
    BUTTON_PORT.PINCONFIG = PORT_ISC_BOTHEDGES_gc;
    BUTTON_PORT.PINCTRLSET = BUTTON_MASK;
}

static void on_button_down_event_wrapper(void){
    uint8_t flags;
    event_PopEventData(&flags, 1);
    onButtonDown(flags);
}

static void on_button_up_event_wrapper(void){
    uint8_t flags;
    event_PopEventData(&flags, 1);
    onButtonUp(flags);
}

static void debounce_cb(void *d) {
    // Debounce has completed. Figure out which events to call
    uint8_t button_down;
    uint8_t button_up;
    uint8_t in;

    in = BUTTON_PORT.IN;
    button_down = in & State.debouncing;
    button_up = ~in & State.debouncing;

    if(button_down){
        event_PushEvent(on_button_down_event_wrapper, &button_down, 1);
    }

    if(button_up){
        event_PushEvent(on_button_up_event_wrapper, &button_up, 1);
    }

    // Re-enable interrupts
    BUTTON_PORT.PINCONFIG = PORT_ISC_BOTHEDGES_gc;
    BUTTON_PORT.PINCTRLSET = State.debouncing;
    State.debouncing = 0;
}


ISR(BUTTON_PORT_INT_vect) {
    uint8_t flags;
    flags = BUTTON_PORT.INTFLAGS;

    // Immediately disable and clear interrupts for any buttons that fired
    BUTTON_PORT.PINCONFIG = PORT_ISC_gm;
    BUTTON_PORT.PINCTRLCLR = flags;
    BUTTON_PORT.INTFLAGS = flags;

    // Start debounce timer
    if(State.debouncing) {
        // debounce timer is already running. Cancel it
        timer_stop(&debounce_timer);
    }
    State.debouncing |= flags;
    struct timerctl tc;
    tc.interval = RTC_CNT_FREQ / 50; // 20ms
    tc.repeat = false;
    tc.callback = debounce_cb;
    timer_start(&debounce_timer, &tc);
}
