#include "gps_uart.h"
#include "ubx.h"

void gps_init(void) {
    gps_uart_init();
    ubx_init();
}

void gps_reinit(void) {
    ubx_init();
}

void gps_poll_uart(void){
    if(uart_rdcount(&gps_uart_dev)) {
        char c;
        c = uart_getc(&gps_uart_dev);
        ubx_process_char(c);
    }
}
