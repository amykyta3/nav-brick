
#include <stdint.h>
#include <stddef.h>

#include "../sys.h"
#include <util/delay.h>

#include "sensors.h"

//------------------------------------------------------------------------------
#define SADDR   0x29

// Register addresses
#define ALS_CONTR       0x80
#define ALS_MEAS_RATE   0x85
#define PART_ID         0x86
#define MANUFAC_ID      0x87
#define ALS_DATA_CH1_0  0x88
#define ALS_DATA_CH1_1  0x89
#define ALS_DATA_CH0_0  0x8A
#define ALS_DATA_CH0_1  0x8B
#define ALS_STATUS      0x8C

// ALS_CONTR register
#define ALS_GAIN_bm     0x1C
#define ALS_GAIN_bp     2
#define ALS_GAIN_1X_gc  (0x0 << ALS_GAIN_bp)
#define ALS_GAIN_2X_gc  (0x1 << ALS_GAIN_bp)
#define ALS_GAIN_4X_gc  (0x2 << ALS_GAIN_bp)
#define ALS_GAIN_8X_gc  (0x3 << ALS_GAIN_bp)
#define ALS_GAIN_48X_gc (0x6 << ALS_GAIN_bp)
#define ALS_GAIN_96X_gc (0x7 << ALS_GAIN_bp)
#define SW_RESET_bm     0x2
#define SW_RESET_bp     1
#define ALS_MODE_bm     0x1
#define ALS_MODE_bp     0
#define ALS_MODE_STANDBY_gc (0 << ALS_MODE_bp)
#define ALS_MODE_ACTIVE_gc  (1 << ALS_MODE_bp)

// ALS_MEAS_RATE Register
#define INTEG_TIME_bm       0x38
#define INTEG_TIME_bp       3
#define INTEG_TIME_100MS_gc (0 << INTEG_TIME_bp)
#define INTEG_TIME_50MS_gc  (1 << INTEG_TIME_bp)
#define INTEG_TIME_200MS_gc (2 << INTEG_TIME_bp)
#define INTEG_TIME_400MS_gc (3 << INTEG_TIME_bp)
#define INTEG_TIME_150MS_gc (4 << INTEG_TIME_bp)
#define INTEG_TIME_250MS_gc (5 << INTEG_TIME_bp)
#define INTEG_TIME_300MS_gc (6 << INTEG_TIME_bp)
#define INTEG_TIME_350MS_gc (7 << INTEG_TIME_bp)
#define MEAS_RATE_bm        0x07
#define MEAS_RATE_bp        0
#define MEAS_RATE_50MS_gc   (0 << MEAS_RATE_bp)
#define MEAS_RATE_100MS_gc  (1 << MEAS_RATE_bp)
#define MEAS_RATE_200MS_gc  (2 << MEAS_RATE_bp)
#define MEAS_RATE_500MS_gc  (3 << MEAS_RATE_bp)
#define MEAS_RATE_1000MS_gc (4 << MEAS_RATE_bp)
#define MEAS_RATE_2000MS_gc (5 << MEAS_RATE_bp)

//------------------------------------------------------------------------------
static i2c_package_t pkg;

static void write_reg(uint8_t addr, uint8_t data){
    pkg.addr_len = 1;
    pkg.addr[0] = addr;
    pkg.data_len = 1;
    pkg.data = &data;
    pkg.read = false;

    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY);
}

static uint8_t read_reg(uint8_t addr){
    uint8_t data;
    pkg.addr_len = 1;
    pkg.addr[0] = addr;
    pkg.data_len = 1;
    pkg.data = &data;
    pkg.read = true;

    i2c_transfer_start(&sensors_i2c_dev, &pkg, NULL);
    while(i2c_transfer_status(&sensors_i2c_dev) == I2C_BUSY);
    return data;
}


void als_init(void){
    pkg.slave_addr = SADDR;

    write_reg(ALS_CONTR, SW_RESET_bm);
    write_reg(ALS_MEAS_RATE, MEAS_RATE_200MS_gc | INTEG_TIME_200MS_gc);
    write_reg(ALS_CONTR, ALS_GAIN_1X_gc | ALS_MODE_ACTIVE_gc);
    _delay_ms(10);
}

void als_get_sample(uint16_t *ch0, uint16_t *ch1){
    *ch1 = read_reg(ALS_DATA_CH1_0);
    *ch1 |= read_reg(ALS_DATA_CH1_1) << 8;
    *ch0 = read_reg(ALS_DATA_CH0_0);
    *ch0 |= read_reg(ALS_DATA_CH0_1) << 8;
}
