#ifndef LTR_329ALS_LIGHT
#define LTR_329ALS_LIGHT

#include <stdint.h>

void als_init(void);

/*
* ch0: Peak sensitivity in visible
* ch1: Peak sensitivity in infrared
*/
void als_get_sample(uint16_t *ch0, uint16_t *ch1);

#endif
