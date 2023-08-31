#ifndef MOCK_RTC_H
#define MOCK_RTC_H

void mock_rtc_init(void);
void mock_rtc_do_tick(void);

extern uint64_t current_time;

#endif
