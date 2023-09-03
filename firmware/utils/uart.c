#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "uart.h"

// mapping of USART io devs --> uart_dev_t objects
static uart_dev_t *devs[3];

void uart_init(uart_dev_t *dev, USART_t *io, void *rxbuf, size_t rxbufsize, void *txbuf, size_t txbufsize, uint16_t bauddiv_q16){

    if(io == &USART0) {
        devs[0] = dev;
    } else if (io == &USART1){
        devs[1] = dev;
    } else if (io == &USART2){
        devs[2] = dev;
    }

    dev->io = io;
    fifo_init(&dev->rxfifo, rxbuf, rxbufsize);
    fifo_init(&dev->txfifo, txbuf, txbufsize);

    // global interrupts must be disabled during the initialization
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        io->BAUD = bauddiv_q16;
        io->CTRLC = USART_CHSIZE_8BIT_gc;
        io->CTRLB = USART_RXEN_bm | USART_TXEN_bm;
        io->CTRLA = USART_RXCIE_bm;
    }
}

void uart_read(uart_dev_t *dev, void *buf, size_t size){
    size_t rdcount;
    uint8_t* u8buf = (uint8_t*)buf;

    while(size > 0){
        // Get number of bytes that can be read.
        rdcount = fifo_rdcount(&dev->rxfifo);
        if(rdcount > size){
            rdcount = size;
        }

        if(rdcount != 0){
            if(u8buf){
                fifo_read(&dev->rxfifo, u8buf, rdcount);
                u8buf += rdcount;
            }else{
                fifo_read(&dev->rxfifo, NULL, rdcount);
            }
            size -= rdcount;
        }
    }
}

void uart_write(uart_dev_t *dev, void *buf, size_t size){
    size_t wrcount;
    uint8_t* u8buf = (uint8_t*)buf;

    while(size > 0){
        // Get number of bytes that can be written.
        wrcount = fifo_wrcount(&dev->txfifo);
        if(wrcount > size){
            wrcount = size;
        }

        if(wrcount != 0){
            fifo_write(&dev->txfifo, u8buf, wrcount);
            u8buf += wrcount;
            size -= wrcount;

            // Enable TX interrupt.
            // If TX is idle, then the interrupt should occur immediately.
            dev->io->CTRLA = USART_RXCIE_bm | USART_DREIE_bm;
        }
    }
}

static void rxc_isr(uart_dev_t *dev){
    uint8_t c;
    c = dev->io->RXDATAL;
    fifo_write(&dev->rxfifo, &c, 1);
}

static void dre_isr(uart_dev_t *dev){
    uint8_t c;
    if(fifo_read(&dev->txfifo, &c, 1) == 0){
        dev->io->TXDATAL = c;
    }else{
        // disable tx interrupt
        dev->io->CTRLA = USART_RXCIE_bm;
    }
}

ISR(USART0_RXC_vect) {rxc_isr(devs[0]);}
ISR(USART1_RXC_vect) {rxc_isr(devs[1]);}
ISR(USART2_RXC_vect) {rxc_isr(devs[2]);}
ISR(USART0_DRE_vect) {dre_isr(devs[0]);}
ISR(USART1_DRE_vect) {dre_isr(devs[1]);}
ISR(USART2_DRE_vect) {dre_isr(devs[2]);}


size_t uart_rdcount(uart_dev_t *dev){
    return(fifo_rdcount(&dev->rxfifo));
}

void uart_rdflush(uart_dev_t *dev){
    fifo_clear(&dev->rxfifo);
}

char uart_getc(uart_dev_t *dev){
    char c;
    while(fifo_rdcount(&dev->rxfifo) == 0);
    fifo_read(&dev->rxfifo, &c, 1);
    return(c);
}

void uart_putc(uart_dev_t *dev, const char c){
    uart_write(dev, (uint8_t*) &c, 1);
}

void uart_puts(uart_dev_t *dev, const char *s){
    uart_write(dev, (uint8_t*)s, strlen(s));
}
