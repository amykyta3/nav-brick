
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "rtc.h"

//------------------------------------------------------------------------------

// 1-minute period
#define RTC_PER_VALUE   ((RTC_CLK_FREQ * 60UL) >> RTC_PRESCALE_SHIFT)

#if(RTC_CALENDAR_ENABLE)
//==============================================================================
// Calendar Variables
//==============================================================================
static uint16_t Cal_year;
static uint8_t  Cal_month;
static uint8_t  Cal_day;
static uint8_t  Cal_dayofweek;
static uint8_t  Cal_hour;
static uint8_t  Cal_minute;

static uint8_t DST_observed;
static uint8_t DST_on;

static calendar_alarm_t *Alarm_first;

// Clock correction variables
static calendar_time_t prev_set_time;
static int32_t correction_interval;
static int32_t correction_interval_counter;
static int32_t total_correction_minutes;

//==============================================================================
// Calendar Functions
//==============================================================================

/**
* \brief Calculates and fills in the \c day_of_week member based on the date.
* \param T #calendar_time_t object
**/
static void calc_DOW(calendar_time_t *T){
    const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint16_t y;
    y = T->year - (T->month < 3);
    T->dayofweek = (y + y/4 - y/100 + y/400 + t[T->month-1] + T->day) % 7;
}

//------------------------------------------------------------------------------
void calendar_set_time(calendar_time_t *T){
    uint8_t intctrl;

    if(T->dayofweek == UNKNOWN_DOW){
        calc_DOW(T);
    }

    // Stop RTC
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;

    // Wait for busy to clear
    while(RTC.STATUS & RTC_CNTBUSY_bm);

    // Update
    RTC.CNT = (uint16_t)T->second * RTC_CNT_FREQ;
    Cal_year = T->year;
    Cal_month = T->month;
    Cal_day = T->day;
    Cal_dayofweek = T->dayofweek;
    Cal_hour = T->hour;
    Cal_minute = T->minute;

    // Restart RTC
    RTC.INTCTRL = intctrl | RTC_OVF_bm;

    // Update reference time
    prev_set_time = *T;

    correction_interval_counter = 0;
    total_correction_minutes = 0;
}

//------------------------------------------------------------------------------
void calendar_set_DST(uint8_t observed, uint8_t enabled){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;

    DST_observed = observed;
    DST_on = enabled;

    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
void calendar_get_time(calendar_time_t *T){

    if((RTC.INTCTRL & RTC_OVF_bm) == 0){
        // Interrupt is not enabled, therefore time was never set since powerup
        T->second = 0;
        T->year = 0;
        T->month = 0;
        T->day = 0;
        T->dayofweek = 0;
        T->hour = 0;
        T->minute = 0;
    }else{
        // Copy state
        T->second = RTC.CNT / RTC_CNT_FREQ;
        T->year = Cal_year;
        T->month = Cal_month;
        T->day = Cal_day;
        T->dayofweek = Cal_dayofweek;
        T->hour = Cal_hour;
        T->minute = Cal_minute;
    }
}

//------------------------------------------------------------------------------
void calendar_get_last_set_timestamp(calendar_time_t *T){
    *T = prev_set_time;
}

//------------------------------------------------------------------------------
void calendar_set_correction_interval(int32_t interval){
    correction_interval = interval;
    correction_interval_counter = 0;
    total_correction_minutes = 0;
}

//------------------------------------------------------------------------------
int32_t calendar_get_total_correction(void){
    return(total_correction_minutes);
}

//------------------------------------------------------------------------------
void calendar_add_alarm(calendar_alarm_t *A){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;

    // Insert alarm to head of list
    A->next = Alarm_first;
    Alarm_first = A;

    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
void calendar_remove_alarm(calendar_alarm_t *A){
    uint8_t intctrl;
    intctrl = RTC.INTCTRL;
    RTC.INTCTRL = 0;

    if(A && Alarm_first){

        // Check edge case: if it is the first one in the list
        if(A == Alarm_first){
            Alarm_first = Alarm_first->next;
        }else{
            calendar_alarm_t *alarm_prev = Alarm_first;
            calendar_alarm_t *alarm = Alarm_first->next;

            // Find alarm and remove from list
            while(1){
                if(alarm == A){
                    alarm_prev->next = alarm->next;
                    break;
                }

                // goto next
                if(alarm){
                    alarm_prev = alarm;
                    alarm = alarm->next;
                }else{
                    break;
                }
            }
        }
    }

    RTC.INTCTRL = intctrl;
}

//------------------------------------------------------------------------------
static uint8_t days_in_current_month(){
    const uint8_t n[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t days;

    days = n[Cal_month-1];
    if(Cal_month == 2){
        // February. Check if leap year
        if((Cal_year % 4) == 0){
            if((Cal_year % 100) != 0){
                days++;
            }else if((Cal_year % 400) == 0){
                days++;
            }
        }
    }
    return(days);
}
//------------------------------------------------------------------------------
static bool inc_day(void){
    if(Cal_dayofweek == 6){
        Cal_dayofweek = 0;
    } else {
        Cal_dayofweek++;
    }

    if(Cal_day == days_in_current_month(Cal_month, Cal_year)){
        // signal to increment month
        Cal_day = 1;
        return(true);
    } else {
        Cal_day++;
    }

    return(false);
}

//------------------------------------------------------------------------------
static void adjust_for_dst(void){
    // Function is only called if DST is observed, and just rolled over to a new hour

    if(DST_on == 0){
        // DST starts on the second Sunday in March,
        //  moving forward from 2:00 a.m. to 3:00 a.m.
        if((Cal_month == 3) // March
            && (Cal_dayofweek == 0) // Sunday
            && (Cal_day >= 8) && (Cal_day <= 14) // Second Sunday
            && (Cal_hour == 2) // 2:00 AM
        ){
            Cal_hour = 3;
            DST_on = 1;
        }
    }else{
        // DST ends on the first Sunday in November,
        //  moving back from 2:00 a.m. to 1:00 a.m
        if((Cal_month == 11) // November
            && (Cal_dayofweek == 0) // Sunday
            && (Cal_day >= 1) && (Cal_day <= 7) // First Sunday
            && (Cal_hour == 2) // 2:00 AM
        ){
            Cal_hour = 1;
            DST_on = 0;
        }
    }
}

//------------------------------------------------------------------------------
static void check_alarms(void){
    calendar_alarm_t *alarm = Alarm_first;

    while(alarm){
        // Check if the alarm should fire

        if(((0x01 << Cal_dayofweek) & alarm->dayofweek_mask)
            && (Cal_hour == alarm->hour)
            && (Cal_minute == alarm->minute)
        ){
            // Trigger callback
            if(alarm->callback){
                alarm->callback(alarm->callback_data);
            }
        }

        alarm = alarm->next;
    }
}

//------------------------------------------------------------------------------
static void minute_tick_isr(void){
    // Increment Time
    if(Cal_minute == 59){
        Cal_minute = 0;
        if(Cal_hour == 23){
            Cal_hour = 0;
            if(inc_day()){
                // inc month
                Cal_day = 1;
                if(Cal_month == 12){
                    Cal_month = 1;
                    Cal_year++;
                }else{
                    Cal_month++;
                }
            }
        }else{
            Cal_hour++;

            // If USA Daylight savings is observed, check if adjustment must be made
            if(DST_observed){
                adjust_for_dst();
            }
        }
    }else{
        Cal_minute++;
    }

    check_alarms();
}

static void rtc_ovf_isr(void) {
    // Increment clock correction counter
    if(correction_interval > 0){
        // Correcting for slow clock
        correction_interval_counter++;
        if(correction_interval == correction_interval_counter){
            correction_interval_counter = 0;
            total_correction_minutes++;

            // Clock is slow. Insert an extra minute to catch up
            minute_tick_isr();
        }
        minute_tick_isr();
    } else if(correction_interval < 0){
        // Correcting for fast clock
        correction_interval_counter--;
        if(correction_interval == correction_interval_counter){
            correction_interval_counter = 0;
            total_correction_minutes--;

            // Clock is fast. Remove a minute by not doing the ISR
        }else{
            minute_tick_isr();
        }
    } else {
        // No correction enabled.
        minute_tick_isr();
    }
}

#endif // RTC_CALENDAR_ENABLE

//==============================================================================
// Event Timer Variables
//==============================================================================
#if(RTC_TIMER_ENABLE)

/**
 * Head of linked list of all active timers
 **/
static timer_t *Timer_first;

/**
 * Tick reference.
 * This is the CNT value that the first timer's ticks_remaining value is in
 * reference to.
 **/
static uint16_t Timer_tickref;

//==============================================================================
// Event Timer Functions
//==============================================================================

/**
* \brief Updates timer lists based on new_cnt value
*
* Time elapsed is ambiguous if new_cnt == Timer_tickref.
* To clear this up, assumption will be based on is_isr:
*   is_isr == true: Time elapsed is RTC_PER_VALUE
*   is_isr == false: Time elapsed is 0
*
* \warning Must call this from within an atomic block!
*
* \param new_cnt Latest RTC.CNT value
* \param is_isr Set to 1 if calling from COMP ISR.
**/
static void update_tickref(uint16_t new_cnt, uint8_t is_isr){

    // In the super rare (impossible?) chance that cnt is caught
    // before it gets reset, set to 0
    if(new_cnt == RTC_PER_VALUE) new_cnt = 0;

    if(Timer_first){
        uint16_t ticks_elapsed;

        // Calculate time elapsed since last update
        if(new_cnt == Timer_tickref){
            if(is_isr){
                ticks_elapsed = RTC_PER_VALUE;
            }else{
                ticks_elapsed = 0;
            }
        }else if(new_cnt > Timer_tickref){
            ticks_elapsed = new_cnt - Timer_tickref;
        }else{
            // wrapped
            ticks_elapsed = RTC_PER_VALUE - Timer_tickref;
            ticks_elapsed += new_cnt;
        }

        // Update first timer
        if(ticks_elapsed > Timer_first->ticks_remaining){
            // something went wrong... Zero out the timer
            Timer_first->ticks_remaining = 0;
        }else{
            Timer_first->ticks_remaining -= ticks_elapsed;
        }
    }
    Timer_tickref = new_cnt;
}

//------------------------------------------------------------------------------
/**
* \brief Insert timer into linked list
* \warning Must call this from within an atomic block!
* \param t Timer to insert
**/
static void insert_timer(timer_t *t){
    timer_t *t_prev = NULL;
    timer_t *t_current = Timer_first;

    t->next = NULL;

    while(1){
        if(t_current == NULL){
            // Got to end of list. Append
            if(t_prev){
                t_prev->next = t;
            }else{
                // List was empty
                Timer_first = t;
            }
            return;
        }

        // See if t should go before or after t_current
        if(t->ticks_remaining < t_current->ticks_remaining){
            // Insert before
            if(t_prev){
                t_prev->next = t;
            }else{
                Timer_first = t;
            }
            t->next = t_current;

            // update
            t_current->ticks_remaining -= t->ticks_remaining;
            return;
        }else{
            // Seek to next
            t->ticks_remaining -= t_current->ticks_remaining;
            t_prev = t_current;
            t_current = t_current->next;
        }

    }
}

//------------------------------------------------------------------------------
/**
* \brief First timer in list may have changed. Update RTC.COMP
* \warning Must call this from within an atomic block!
**/
static void update_COMP(){
    if(Timer_first){
        // Update COMP based on first timer. Assume tickref was just updated
        if(Timer_first->ticks_remaining < RTC_PER_VALUE){
            uint16_t new_comp;
            // Next timer is due to expire soon!
            new_comp = Timer_tickref + Timer_first->ticks_remaining;
            if(new_comp >= RTC_PER_VALUE){
                new_comp -= RTC_PER_VALUE;
            }

            // Set comp to 1 before since interupt fires at COMP+1
            if(new_comp == 0){
                new_comp = RTC_PER_VALUE-1;
            }else{
                new_comp -= 1;
            }
            while(RTC.STATUS & RTC_CMPBUSY_bm);
            RTC.CMP = new_comp;

        }else{
            // Leave COMP as-is
        }

        // If disabled, clear flag and enable COMP interrupt
        if((RTC.INTCTRL & RTC_CMP_bm) == 0){
            RTC.INTFLAGS = RTC_CMP_bm;
            RTC.INTCTRL |= RTC_CMP_bm;
        }
    }else{
        // Disable COMP Interrupt
        RTC.INTCTRL &= ~(RTC_CMP_bm);
    }
}

//------------------------------------------------------------------------------
static void rtc_cmp_isr(void) {
    uint16_t current_CNT;
    timer_t *t = NULL;
    // Get CNT value
    current_CNT = RTC.CNT;

    // Check if timer expired
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        update_tickref(current_CNT,1);

        if(Timer_first){
            if(Timer_first->ticks_remaining == 0){
                // Timer expired

                // Save pointer to timer and remove from list
                t = Timer_first;
                Timer_first = Timer_first->next;

                // if repeat, reload ticks_remaining and insert_timer()
                if(t->ticks_reload){
                    t->ticks_remaining = t->ticks_reload;
                    insert_timer(t);
                }

            }
        }
    }

    // Call callback
    if(t){
        if(t->callback){
            t->callback(t->callback_data);
        }
    }

    // Check for any more expired timers. Repeat until all are cleared
    while(t){
        t = NULL;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            if(Timer_first){
                if(Timer_first->ticks_remaining == 0){
                    // Timer expired

                    // Save pointer to timer and remove from list
                    t = Timer_first;
                    Timer_first = Timer_first->next;

                    // if repeat, reload ticks_remaining and insert_timer()
                    if(t->ticks_reload){
                        t->ticks_remaining = t->ticks_reload;
                        insert_timer(t);
                    }
                }
            }
        }

        // Call callback
        if(t){
            if(t->callback){
                t->callback(t->callback_data);
            }
        }
    }

    update_COMP();
}

//------------------------------------------------------------------------------
void timer_start(timer_t *timerid, struct timerctl *settings){
    uint16_t current_CNT;
    if(settings){
        // creating new timer. Copy parameters
        timerid->ticks_remaining = settings->interval;
        timerid->callback = settings->callback;
        timerid->callback_data = settings->callback_data;
        if(settings->repeat){
            timerid->ticks_reload = timerid->ticks_remaining;
        } else {
            timerid->ticks_reload = 0;
        }
    }

    // Get CNT value
    current_CNT = RTC.CNT;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        update_tickref(current_CNT,0);
        insert_timer(timerid);
        if(timerid == Timer_first){
            update_COMP();
        }
    }
}

//------------------------------------------------------------------------------
void timer_stop(timer_t *timerid){
    uint16_t current_CNT;
    uint16_t ticks_sum = 0;

    // Get CNT value
    current_CNT = RTC.CNT;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        timer_t *t_prev = NULL;
        timer_t *t_current = Timer_first;

        update_tickref(current_CNT,0);

        // Remove timerid from list
        //  (Keep track of cumulative ticks_remaining)
        while(t_current){
            if(t_current == timerid){
                // found match
                if(t_prev){
                    t_prev->next = t_current->next;
                    // FIXME: if canceling a timer, all others need tobe updated too
                }else{
                    // t_current is Timer_first
                    Timer_first = Timer_first->next;
                }
                break;
            } else {
                // goto next
                ticks_sum += t_current->ticks_remaining;
                t_prev = t_current;
                t_current = t_current->next;
            }
        }

        update_COMP();
    }

    // Update timerid
    timerid->ticks_remaining += ticks_sum;
}
#endif // RTC_TIMER_ENABLE

//==============================================================================
// General RTC Functions
//==============================================================================
void rtc_init(void){
    RTC.CTRLA = RTC_PRESCALER_gc | RTC_RTCEN_bm;
    RTC.CLKSEL = RTC_CLKSEL_gc;

    while(RTC.STATUS & (RTC_PERBUSY_bm | RTC_CNTBUSY_bm));

    // Set period to 1 minute
    RTC.PER = RTC_PER_VALUE;
    RTC.CNT = 0x0000;

    RTC.INTFLAGS = RTC_CMP_bm | RTC_OVF_bm;
    RTC.INTCTRL = 0;

    #if(RTC_CALENDAR_ENABLE)
        Alarm_first = NULL;
        correction_interval = 0;
        correction_interval_counter = 0;
        total_correction_minutes = 0;

        // No need to initialize calendar variables. They are not used until actually set.
    #endif

    #if(RTC_TIMER_ENABLE)
        Timer_first = NULL;
    #endif
}

//------------------------------------------------------------------------------
void rtc_uninit(void){
    RTC.CTRLA = RTC_PRESCALER_gc;
    RTC.INTCTRL = 0;
}


ISR(RTC_CNT_vect) {
    #if(RTC_CALENDAR_ENABLE)
        if(RTC.INTFLAGS & RTC_OVF_bm){
            RTC.INTFLAGS = RTC_OVF_bm;
            rtc_ovf_isr();
        }
    #endif

    #if(RTC_TIMER_ENABLE)
        if(RTC.INTFLAGS & RTC_CMP_bm){
            RTC.INTFLAGS = RTC_CMP_bm;
            rtc_cmp_isr();
        }
    #endif
}
