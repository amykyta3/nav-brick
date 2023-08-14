#ifndef GPS_UART_H
#define GPS_UART_H

#include "../utils/uart.h"

void gps_uart_init(void);

extern uart_dev_t gps_uart_dev;

#endif
