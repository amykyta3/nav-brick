#ifndef SENSORS_H
#define SENSORS_H

#include "../utils/i2c.h"
#include "ltr-329als_light.h"
#include "ms5611_pressure.h"

extern i2c_dev_t sensors_i2c_dev;

void sensors_init(void);

#endif
