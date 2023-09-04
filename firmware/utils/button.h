#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

void button_init(void);

void onButtonDown(uint8_t flags);
void onButtonUp(uint8_t flags);
#endif
