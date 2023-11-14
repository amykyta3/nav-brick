#include <stddef.h>

#include "../board.h"
#include "sensors.h"
#include "../utils/rtc.h"
#include "../utils/event_queue.h"
#include "../slate.h"

#define POLL_HZ 16
#define PX_AVERAGE 16
#define LIGHT_AVERAGE 16

// Shared I2C device for all sensors
i2c_dev_t sensors_i2c_dev;

// Polling loop timer object
static timer_t sensor_timer;

static void sample_light(void){
    static uint16_t buffer[LIGHT_AVERAGE];
    static uint8_t idx;
    static bool buffer_ok = false;

    als_get_sample(&buffer[idx], &Slate.light.ir);

    if(buffer_ok){
        // moving average
        uint32_t sum;
        sum = 0;
        for(uint8_t i=0; i<LIGHT_AVERAGE; i++) sum += buffer[i];
        Slate.light.vis = sum / LIGHT_AVERAGE;
    } else {
        Slate.light.vis = buffer[idx];
    }

    idx++;
    if(idx >= LIGHT_AVERAGE) {
        idx = 0;
        buffer_ok = true;
    }
}

static void sample_px(void){
    static int32_t buffer[PX_AVERAGE];
    static uint8_t idx;
    static bool buffer_ok = false;

    px_sensor_get_sample(&Slate.alt.temperature, &buffer[idx]);

    if(buffer_ok){
        // moving average
        int32_t sum;
        sum = 0;
        for(uint8_t i=0; i<PX_AVERAGE; i++) sum += buffer[i];
        Slate.alt.pressure = sum / PX_AVERAGE;
    } else {
        Slate.alt.pressure = buffer[idx];
    }

    idx++;
    if(idx >= PX_AVERAGE) {
        idx = 0;
        buffer_ok = true;
    }
}


static void polling_event(void){
    uint16_t start;
    start = RTC.CNT;

    sample_light();
    sample_px();

    Slate.sensor_poll_duration = RTC.CNT - start;
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
