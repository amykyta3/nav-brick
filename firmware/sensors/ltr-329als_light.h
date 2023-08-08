#ifndef LTR_329ALS_LIGHT
#define LTR_329ALS_LIGHT

#include <stdint.h>

void als_init(void);
void als_get_sample(uint16_t *ch0, uint16_t *ch1);

#endif
