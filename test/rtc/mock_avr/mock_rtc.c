#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "avr_includes/avr/io.h"


RTC_t RTC;
uint64_t current_time;

void isr_RTC_CNT_vect(void);


void mock_rtc_init(void){
    RTC.CMP = 0;
    RTC.CNT = 0;
    RTC.PER = 0;
    RTC.INTFLAGS = 0;
    RTC.INTCTRL = 0;
}

void mock_rtc_do_tick(void){
    bool cmp_event;

    // Mark the passage of time for this event
    current_time++;

    // ISR happens at end of count
    if(RTC.CNT == RTC.CMP){
        cmp_event = true;
    } else {
        cmp_event = false;
    }

    // Advance CNT
    RTC.CNT++;
    if(RTC.CNT == RTC.PER){
        // Overflow condition
        RTC.CNT = 0;
    }

    if(cmp_event){
        // CMP match condition

        // Set CMP flag
        RTC.INTFLAGS |= RTC_CMP_bm;

        if(RTC.INTCTRL & RTC_CMP_bm){
            //printf("Calling isr @ CNT=%d, time=%ld\n", RTC.CNT, current_time);
            // Interrupt is enabled
            // Call ISR
            isr_RTC_CNT_vect();

            // Assume IFG was cleared
            RTC.INTFLAGS = 0;
            //printf("New CMP = %d\n", RTC.CMP);
        } else {
            //printf("Skipping isr @ CNT=%d, time=%ld\n", RTC.CNT, current_time);
        }
    }
}
