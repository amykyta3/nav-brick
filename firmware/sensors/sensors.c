#include "../board.h"
#include "sensors.h"

// Shared I2C device for all sensors
i2c_dev_t sensors_i2c_dev;

void sensors_init(void){
    // Baud rate ~= 300 KHz
    i2c_init(&sensors_i2c_dev, &SENSOR_I2C, 40);

    // TODO: Init all other sensors
    als_init();
    px_sensor_init();
}
