// Interface with the MS5611-01BA pressure sensor

#include <stddef.h>
#include <stdint.h>

#include "../sys.h"
#include <util/delay.h>

#include "sensors.h"
#include "ms5611_pressure.h"

#define CMD_RESET               0x1E
#define CMD_CONV_D1_OSR_256     0x40
#define CMD_CONV_D1_OSR_512     0x42
#define CMD_CONV_D1_OSR_1024    0x44
#define CMD_CONV_D1_OSR_2048    0x46
#define CMD_CONV_D1_OSR_4096    0x48
#define CMD_CONV_D2_OSR_256     0x50
#define CMD_CONV_D2_OSR_512     0x52
#define CMD_CONV_D2_OSR_1024    0x54
#define CMD_CONV_D2_OSR_2048    0x56
#define CMD_CONV_D2_OSR_4096    0x58
#define CMD_ADC_READ            0x00
#define CMD_PROM_READ_START     0xA0


static i2c_package_t pkg;

static struct {
    uint16_t C[6];
} prom;

#define C1 (prom.C[0])
#define C2 (prom.C[1])
#define C3 (prom.C[2])
#define C4 (prom.C[3])
#define C5 (prom.C[4])
#define C6 (prom.C[5])

void px_sensor_init(void){
    pkg.slave_addr = 0x77; // 1110111
    px_sensor_reset();
    px_sensor_read_prom();
}

// This routine is blocking
void px_sensor_reset(void){
    pkg.addr_len = 1;
    pkg.addr[0] = CMD_RESET;
    pkg.data_len = 0;
    pkg.read = false;

    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

    // I2C transaction is done, but need to still wait ~2.8ms for reload time
    // TODO: Do I need this? Or will the device stall I2C implicitly?
    _delay_ms(3);
}

// This routine is blocking
void px_sensor_read_prom(void){
    uint8_t d[2];
    pkg.addr_len = 1;
    pkg.data_len = 2;
    pkg.data = d;
    pkg.read = true;

    for(int i=0; i<6; i++){
        pkg.addr[0] = CMD_PROM_READ_START + (i << 1);
        i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
        while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

        // I2C reads data into buffer MSB first.
        prom.C[i] = (d[0] << 8) | d[1];
    }
}

// This routine is blocking
void px_sensor_get_raw_data(uint32_t *raw_temp, uint32_t *raw_pressure){
    uint8_t d[3];

    // Send command to start temperature conversion
    pkg.addr_len = 1;
    pkg.addr[0] = CMD_CONV_D2_OSR_4096;
    pkg.data_len = 0;
    pkg.read = false;
    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

    // Wait for conversion
    // TODO: probe this on a scope... will the device hold I2C until done?
    _delay_ms(10);

    // Read result
    pkg.addr_len = 0;
    pkg.data_len = 3;
    pkg.data = d;
    pkg.read = true;
    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

    *raw_temp = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];

    // Send command to start pressure conversion
    pkg.addr_len = 1;
    pkg.addr[0] = CMD_CONV_D1_OSR_4096;
    pkg.data_len = 0;
    pkg.read = false;
    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

    // Wait for conversion
    _delay_ms(10);

    // Read result
    pkg.addr_len = 0;
    pkg.data_len = 3;
    pkg.data = d;
    pkg.read = true;
    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY) _delay_us(100);

    *raw_pressure = ((uint32_t)d[0] << 16) | ((uint32_t)d[1] << 8) | d[2];
}

void px_sensor_get_sample(int32_t *temperature, int32_t *pressure){
    // temperature is in deg C * 100
    // pressure is in mbar * 100
    uint32_t raw_temp;
    uint32_t raw_pres;
    px_sensor_get_raw_data(&raw_temp, &raw_pres);

    // Calculate temperature
    int32_t dT = raw_temp - (C5 << 8);
    int64_t TEMP64 = 2000 + dT * C6;
    *temperature = TEMP64 >> 23;

    // Second-order temperature compensation
    int64_t OFF2;
    int64_t SENS2;
    if(*temperature < 2000){
        int32_t T2;
        int64_t dT_sq = dT * dT;
        T2 = dT_sq >> 31;
        int64_t fivetempsq;
        fivetempsq = (*temperature - 2000);
        fivetempsq = fivetempsq * fivetempsq * 5;

        OFF2 = fivetempsq >> 1;
        SENS2 = fivetempsq >> 2;

        *temperature -= T2;
    } else {
        OFF2 = 0;
        SENS2 = 0;
    }

    // Calculate pressure
    int64_t OFF = ((uint64_t)C2 << 16) + (((uint64_t)C4 * dT) >> 7) - OFF2;
    int64_t SENS = ((uint64_t)C1 << 15) + (((uint64_t)C3 * dT) >> 8) - SENS2;
    int64_t P64 = ((raw_pres * SENS) >> 21) - OFF;
    *pressure = P64 >> 15;
}
