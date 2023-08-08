#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <avr/interrupt.h>

#include "display.h"
#include "../board.h"


// Each LED update is 8 characters, 16-bits each
#define LEN_BYTES (N_LED_DIGITS*2)
static uint8_t current_buf[LEN_BYTES];
static uint8_t current_idx = 0;

void display_spi_init(void){
    LED_SPI.CTRLA = SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESC_DIV4_gc;
    LED_SPI.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm | SPI_BUFEN_bm;
    LED_SPI.INTCTRL = 0;
    LED_SPI.CTRLA |= SPI_ENABLE_bm;
}

void display_spi_update(void *data){
    // Disable the SPI interrupt in case it is still in the middle of a prior
    // operation.
    // This effectively disrupts any prior transfers
    LED_SPI.INTCTRL = 0;

    // Copy buffer since lifetime of 'data' is not known
    memcpy(current_buf, data, LEN_BYTES);

    // Deassert CSN to start new transfer
    PORTC.OUTCLR = P_LED_CSN;

    // Display data is shifted backwards.
    //  - 16-bit values are loaded MSB-first
    //  - characters are loaded rightmost first
    // Since AVR is little-endian, this results in data to be loaded in bytewise
    // reverse order.

    // Transmit first byte
    LED_SPI.DATA = current_buf[LEN_BYTES-1];
    current_idx = LEN_BYTES - 2;

    // Interrupt when ready for another byte
    LED_SPI.INTCTRL = SPI_DREIE_bm;
}

ISR(LED_SPI_INT_vect){
    if(LED_SPI.INTFLAGS & SPI_DREIF_bm){
        // Tx buffer is ready for more data
        if(current_idx == 0){
            // Last byte just moved to the Tx shift register
            // Disable DRE interrupt, enable TX complete interrupt
            LED_SPI.INTFLAGS = SPI_TXCIF_bm; // clear prior flag
            LED_SPI.INTCTRL = SPI_TXCIE_bm;
        } else {
            // Load next byte
            current_idx--;
            LED_SPI.DATA = current_buf[current_idx];
        }
    } else if (LED_SPI.INTFLAGS & SPI_TXCIF_bm){
        LED_SPI.INTCTRL = 0;
        PORTC.OUTSET = P_LED_CSN;
    }
}
