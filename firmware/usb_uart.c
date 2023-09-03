#include <stdio.h>
#include <stdint.h>

#include "board.h"
#include "sys.h"
#include "utils/uart.h"

uart_dev_t usb_uart_dev;
static uint8_t usb_uart_rxbuf[256];
static uint8_t usb_uart_txbuf[128];

static int putchar_printf(char var, FILE *stream);
static FILE usb_stdout = FDEV_SETUP_STREAM(putchar_printf, NULL, _FDEV_SETUP_WRITE);

void usb_uart_init(void){
    uart_init(
        &usb_uart_dev, &USB_USART,
        usb_uart_rxbuf, sizeof(usb_uart_rxbuf),
        usb_uart_txbuf, sizeof(usb_uart_txbuf),
        UART_BAUD(9600)
    );

    stdout = &usb_stdout;
}


static int putchar_printf(char c, FILE *stream) {
    if (c == '\n') uart_putc(&usb_uart_dev, '\r');
    uart_putc(&usb_uart_dev, c);
    return 0;
}
