#ifndef GUI_H
#define GUI_H

#include <stdint.h>

typedef struct {
    uint8_t page;
    uint8_t altitude_mode;
    uint8_t speed_mode;
    uint8_t pressure_mode;
} gui_state_t;

void gui_next_page(void);
void gui_next_page_mode(void);
void gui_refresh_display(void);

#endif
