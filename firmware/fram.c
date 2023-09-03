#include <stdint.h>
#include "fram.h"
#include "board.h"

#define OP_WREN     0x06
#define OP_WRDI     0x04
#define OP_RDSR     0x05
#define OP_WRSR     0x01
#define OP_READ     0x03
#define OP_WRITE    0x02

void fram_init(void){
    // 20 MHz max
    FRAM_SPI.CTRLA = SPI_MASTER_bm | SPI_CLK2X_bm | SPI_PRESC_DIV4_gc;
    FRAM_SPI.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm;
    FRAM_SPI.INTCTRL = 0;
    FRAM_SPI.CTRLA |= SPI_ENABLE_bm;
}

static uint8_t transfer_byte(uint8_t d){
    FRAM_SPI.DATA = d;
    while((FRAM_SPI.INTFLAGS & SPI_IF_bm) == 0);
    return FRAM_SPI.DATA;
}

void fram_wren(void){
    PORTA.OUTCLR = P_FRAM_CSN;
    transfer_byte(OP_WREN);
    PORTA.OUTSET = P_FRAM_CSN;
}

void fram_read(uint16_t addr, void *buf, uint16_t size){
    uint8_t *u8_buf = buf;

    PORTA.OUTCLR = P_FRAM_CSN;

    transfer_byte(OP_READ);
    transfer_byte(addr >> 8);
    transfer_byte(addr & 0xFF);
    for(uint16_t i=0; i<size; i++){
        u8_buf[i] = transfer_byte(0);
    }
    PORTA.OUTSET = P_FRAM_CSN;
}

void fram_write(uint16_t addr, void *buf, uint16_t size){
    uint8_t *u8_buf = buf;

    fram_wren();

    PORTA.OUTCLR = P_FRAM_CSN;

    transfer_byte(OP_WRITE);
    transfer_byte(addr >> 8);
    transfer_byte(addr & 0xFF);
    for(uint16_t i=0; i<size; i++){
        transfer_byte(u8_buf[i]);
    }
    PORTA.OUTSET = P_FRAM_CSN;
}
