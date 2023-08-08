#ifndef MS5611_PRESSURE_H
#define MS5611_PRESSURE_H

#include <stdint.h>

void px_sensor_init(void);
void px_sensor_reset(void);
void px_sensor_read_prom(void);
void px_sensor_get_raw_data(uint32_t *raw_temp, uint32_t *raw_pressure);
void px_sensor_get_sample(int32_t *temperature, int32_t *pressure);

#endif
