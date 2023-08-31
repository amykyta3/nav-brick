#include <stdio.h>
#include <stdint.h>

#include "mock_avr/avr_includes/avr/io.h"
#include "mock_avr/mock_rtc.h"

#include "../../firmware/utils/rtc.h"

#define SENSOR_INTERVAL  (1024/10)
#define DISPLAY_INTERVAL 1024

#define DELAY 10

#define N_ITERATIONS (1024*51*2+1000)

void sensor_cb(void *d);
void display_cb(void *d);
int s_ticks;
int d_ticks;

timer_t s_timer;
timer_t d_timer;

int main(void){
    mock_rtc_init();
    rtc_init();

    printf("PER = %d\n", RTC.PER);

    s_ticks = 0;
    d_ticks = 0;

    for(int i=0; i<DELAY; i++) mock_rtc_do_tick();

    struct timerctl timer_settings;
    timer_settings.interval = SENSOR_INTERVAL;
    timer_settings.repeat = true;
    timer_settings.callback = sensor_cb;
    timer_start(&s_timer, &timer_settings);

mock_rtc_do_tick();

    timer_settings.interval = DISPLAY_INTERVAL;
    timer_settings.repeat = true;
    timer_settings.callback = display_cb;
    timer_start(&d_timer, &timer_settings);

    printf("First CMP: %d\n", RTC.CMP);

    for(int i=0; i<N_ITERATIONS; i++){
        mock_rtc_do_tick();
    }

    if(s_ticks != (N_ITERATIONS/SENSOR_INTERVAL)) {
        printf("Not enough s_ticks. Expected %d, Got %d\n", N_ITERATIONS/SENSOR_INTERVAL, s_ticks);
    }
    if(d_ticks != (N_ITERATIONS/DISPLAY_INTERVAL)) {
        printf("Not enough d_ticks. Expected %d, Got %d\n", N_ITERATIONS/DISPLAY_INTERVAL, d_ticks);
    }

    return 0;
}



void sensor_cb(void *d){
    static uint64_t expected_time = SENSOR_INTERVAL + DELAY;
    //printf("S\n");

    if(current_time != expected_time){
        printf("Sensor: expected time %ld != %ld\n", expected_time, current_time);
    }

    expected_time += SENSOR_INTERVAL;
    s_ticks++;
}

void display_cb(void *d){
    static uint64_t expected_time = DISPLAY_INTERVAL + DELAY + 1;

    printf("CMP:%d CNT:%d s-TR:%d\n", RTC.CMP, RTC.CNT, s_timer.ticks_remaining);

    if(current_time != expected_time){
        printf("Display: expected time %ld != %ld\n", expected_time, current_time);
    }

    expected_time += DISPLAY_INTERVAL;
    d_ticks++;
}
