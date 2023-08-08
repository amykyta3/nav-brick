#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>

#include "fifo.h"

typedef struct {
    USART_t *io;
    FIFO_t rxfifo;
    FIFO_t txfifo;
} uart_dev_t;


void uart_init(uart_dev_t *dev, USART_t *io, void *rxbuf, size_t rxbufsize, void *txbuf, size_t txbufsize, uint16_t bauddiv_q16);


/**
* \brief Get the number of bytes available to be read
* \return Number of bytes
**/
size_t uart_rdcount(uart_dev_t *dev);

/**
* \brief Discards any data received
**/
void uart_rdflush(uart_dev_t *dev);

/**
* \brief Reads the next character received from the UART
* \details If a character is not immediately available, function will block until it receives one.
* \return The next available character
**/
char uart_getc(uart_dev_t *dev);

/**
* \brief Read data from the UART. Blocks until all data has been received.
* \param buf Destination buffer of the data to be read. A \c NULL pointer discards the data.
* \param size Number of bytes to be read.
**/
void uart_read(uart_dev_t *dev, void *buf, size_t size);


/**
* \brief Transmit data over UART
* \param data Pointer to the data to be transmitted
* \param len number of bytes to send
**/
void uart_write(uart_dev_t *dev, void *data, size_t len);

/**
* \brief Writes a character to the UART
* \param c character to be written
**/
void uart_putc(uart_dev_t *dev, const char c);

/**
* \brief Writes a character string to the UART
* \param s Pointer to the Null-terminated string to be sent
**/
void uart_puts(uart_dev_t *dev, const char *s);

void uart_put_x8(uart_dev_t *dev, uint8_t num);
void uart_put_x16(uart_dev_t *dev, uint16_t num);
void uart_put_x32(uart_dev_t *dev, uint32_t num);
void uart_put_d8(uart_dev_t *dev, uint8_t num);
void uart_put_d16(uart_dev_t *dev, uint16_t num);
void uart_put_d32(uart_dev_t *dev, uint32_t num);
void uart_put_sd8(uart_dev_t *dev, int8_t num);
void uart_put_sd16(uart_dev_t *dev, int16_t num);
void uart_put_sd32(uart_dev_t *dev, int32_t num);

#define UART_BAUD(bitrate) ((64UL * CLK_PER_FREQ) / (16UL * (bitrate)))

#endif
