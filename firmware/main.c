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

int main(void) {
    sys_init();
    rtc_init();

    sei(); // Enable interrupts

    event_init();
    display_init();
    sensors_init();

    usb_uart_init();
    gps_init();

    event_StartHandler(); // Does not return
}

void onIdle(void){
    if(uart_rdcount(&usb_uart_dev) > 0){
        char c;
        c = uart_getc(&usb_uart_dev);
        cli_process_char(c);
    }
}
