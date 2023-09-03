#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define N_LED_DIGITS  8

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT
} text_align_t;

/*
* Initialize the display
*/
void display_init(void);

/*
* Update display with specified string.
*/
void display_update_str(char *str, text_align_t align);

/*
* Update per-digit lightness.
*
* 'values' is an array of 8 integers, 0-255 that denote the display lightness.
*/
void display_set_lightness(uint8_t *values);
void display_set_lightness_all(uint8_t value);
void display_set_pwm_raw(uint8_t *values);
void display_set_pwm_raw_all(uint8_t value);

#endif
