#include <string.h>

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
#include "calculations.h"
#include "utils/string_ext.h"
#include "utils/event_queue.h"

static void update_display_callback(void *d);

int main(void) {
    sys_init();
    rtc_init();

    sei(); // Enable interrupts

    event_init();
    display_init();
    sensors_init();

    usb_uart_init();
    gps_init();

    // Start display update timer
    timer_t timer;
    struct timerctl timer_settings;
    timer_settings.interval = RTC_CNT_FREQ / 2;
    timer_settings.repeat = true;
    timer_settings.callback = update_display_callback;
    timer_settings.callback_data = NULL;
    timer_start(&timer, &timer_settings);

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


static void update_display(void);

static void update_display_callback(void *d){
    event_PushEvent(update_display, NULL, 0);
}

static void update_display(void){
    float alt = get_px_altitude();
    alt *= 10;

    char str[16];


    snprint_sd16(str, sizeof(str), alt);

    uint8_t end = strlen(str);

    // Insert a decimal point
    str[end] = str[end-1];
    str[end-1] = '.';
    end++;

    // Append " M"
    str[end++] = ' ';
    str[end++] = 'M';
    str[end] = 0;

    display_update_str(str, ALIGN_RIGHT);
}
