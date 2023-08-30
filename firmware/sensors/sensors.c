#include <stddef.h>

#include "../board.h"
#include "sensors.h"
#include "../utils/rtc.h"
#include "../utils/event_queue.h"
#include "../slate.h"

#define POLL_HZ 10
#define PX_AVERAGE 8

// Shared I2C device for all sensors
i2c_dev_t sensors_i2c_dev;

// Polling loop timer object
static timer_t sensor_timer;

static void polling_event(void){
    static int32_t px_buffer[PX_AVERAGE];
    static uint8_t px_idx;

    uint16_t start;
    start = RTC.CNT;
    als_get_sample(&Slate.light.vis, &Slate.light.ir);

    px_sensor_get_sample(&Slate.temperature, &px_buffer[px_idx++]);
    if(px_idx >= PX_AVERAGE) {
        px_idx = 0;
    }

    // moving average of px
    int32_t px_sum;
    px_sum = 0;
    for(uint8_t i=0; i<PX_AVERAGE; i++) px_sum += px_buffer[i];
    Slate.pressure = px_sum / PX_AVERAGE;

    Slate.poll_duration = RTC.CNT - start;
}

static void timer_callback(void* d){
    // Timer callback is called from within an ISR.
    // re-schedule it into the event loop to allow for the ISR to exit and
    // unblock other ISRs
    event_PushEvent(polling_event, NULL, 0);
}

void sensors_init(void){
    // Baud rate ~= 300 KHz
    i2c_init(&sensors_i2c_dev, &SENSOR_I2C, 40);

    // TODO: Init mag sensor
    als_init();
    px_sensor_init();

    // Start sensor polling loop
    struct timerctl timer_settings;
    timer_settings.interval = RTC_CNT_FREQ / POLL_HZ;
    timer_settings.repeat = true;
    timer_settings.callback = timer_callback;
    timer_settings.callback_data = NULL;
    timer_start(&sensor_timer, &timer_settings);
}
