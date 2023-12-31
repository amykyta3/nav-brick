#ifndef FRAM_H
#define FRAM_H

#include <stdint.h>

#define FRAM_SIZE 0x800

void fram_init(void);
void fram_wren(void);
void fram_read(uint16_t addr, void *buf, uint16_t size);
void fram_write(uint16_t addr, void *buf, uint16_t size);

#endif
