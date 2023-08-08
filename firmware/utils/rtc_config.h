
#ifndef RTC_CONFIG_H
#define RTC_CONFIG_H

#define RTC_CLKSEL_gc    RTC_CLKSEL_OSC1K_gc
    // RTC_CLKSEL_OSC32K_gc
    // RTC_CLKSEL_OSC1K_gc
    // RTC_CLKSEL_XOSC32K_gc

#define RTC_PRESCALER_gc       RTC_PRESCALER_DIV1_gc
    // All powers-of-two between:
    // RTC_PRESCALER_DIV1_gc
    // RTC_PRESCALER_DIV32768_gc

#define RTC_CALENDAR_ENABLE 0
#define RTC_TIMER_ENABLE    1

#endif
