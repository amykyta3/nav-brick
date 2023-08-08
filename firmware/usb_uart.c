#include <stdint.h>

#include "board.h"
#include "sys.h"
#include "utils/uart.h"

uart_dev_t usb_uart_dev;
static uint8_t usb_uart_rxbuf[256];
static uint8_t usb_uart_txbuf[128];

void usb_uart_init(void){
    uart_init(
        &usb_uart_dev, &USB_USART,
        usb_uart_rxbuf, sizeof(usb_uart_rxbuf),
        usb_uart_txbuf, sizeof(usb_uart_txbuf),
        UART_BAUD(9600)
    );
}
