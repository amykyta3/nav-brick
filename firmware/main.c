#include <string.h>
#include <stdio.h>
#include <math.h>

#include <avr/interrupt.h>

#include "sys.h"
#include "display/display.h"
#include "usb_uart.h"
#include "utils/event_queue.h"
#include "utils/cli.h"
#include "utils/rtc.h"
#include "sensors/sensors.h"
#include "board.h"
#include "gps/gps.h"
#include "altitude.h"
#include "slate.h"
#include "fram.h"
#include "utils/button.h"
#include "utils/event_queue.h"
#include "gui.h"

static void update_display(void);
static void update_brightness(void);

static void timer_cb_to_event(void *d){
    event_PushEvent(d, NULL, 0);
}

int main(void) {
    sys_init();
    rtc_init();

    sei(); // Enable interrupts

    usb_uart_init();
    display_init();
    fram_init();
    sensors_init();
    slate_init();
    event_init();
    gps_init();
    button_init();

    struct timerctl timer_settings;
    timer_t content_timer;
    timer_settings.interval = RTC_CNT_FREQ / 2;
    timer_settings.repeat = true;
    timer_settings.callback = timer_cb_to_event;
    timer_settings.callback_data = update_display;
    timer_start(&content_timer, &timer_settings);

    timer_t brightness_timer;
    timer_settings.interval = RTC_CNT_FREQ / 16;
    timer_settings.repeat = true;
    timer_settings.callback = timer_cb_to_event;
    timer_settings.callback_data = update_brightness;
    timer_start(&brightness_timer, &timer_settings);

    event_StartHandler(); // Does not return
}

void onIdle(void){
    if(uart_rdcount(&usb_uart_dev) > 0){
        char c;
        c = uart_getc(&usb_uart_dev);
        cli_process_char(c);
    }
    gps_poll_uart();
}

static void update_display(void){
    update_altitude();
    gui_refresh_display();
    slate_save_nv();
}

static void update_brightness(void){
    float brightness;
    brightness = sqrtf(Slate.light.vis) * 6.75;
    if(brightness > 255){
        brightness = 255;
    } else if(brightness < 4) {
        brightness = 4;
    }
    display_set_pwm_raw_all(brightness);
}

void onButtonDown(uint8_t flags){
    if(flags & P_BUTTON1) {
        gui_next_page();
    }

    if(flags & P_BUTTON2) {
        gui_next_page_mode();
    }
}

void onButtonUp(uint8_t flags){
}
