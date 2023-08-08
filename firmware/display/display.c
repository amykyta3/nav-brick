#include "../board.h"
#include "display.h"

void display_data_from_str(uint16_t *data, const char *str, text_align_t align);
void display_spi_init(void);
void display_spi_update(void *data);
void display_pwm_init(void);


void display_init(void){
    display_spi_init();
    display_pwm_init();

    // Clear display
    display_update_str("", ALIGN_LEFT);
}

void display_update_str(char *str, text_align_t align){
    uint16_t data[N_LED_DIGITS];
    display_data_from_str(data, str, align);
    display_spi_update(data);
}
