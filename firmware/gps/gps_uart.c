#include <stdint.h>

#include "../board.h"
#include "../sys.h"
#include "../utils/uart.h"

uart_dev_t gps_uart_dev;
static uint8_t gps_uart_rxbuf[256];
static uint8_t gps_uart_txbuf[128];

void gps_uart_init(void){
    uart_init(
        &gps_uart_dev, &GPS_USART,
        gps_uart_rxbuf, sizeof(gps_uart_rxbuf),
        gps_uart_txbuf, sizeof(gps_uart_txbuf),
        UART_BAUD(9600)
    );
}
