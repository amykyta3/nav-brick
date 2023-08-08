#ifndef USB_UART_H
#define USB_UART_H

#include "utils/uart.h"

void usb_uart_init(void);

extern uart_dev_t usb_uart_dev;

#endif
