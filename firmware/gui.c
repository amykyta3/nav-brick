#include <stdio.h>
#include <string.h>
#include <math.h>

#include "slate.h"
#include "display/display.h"

#define ROUND_TO(n, x) (roundf((n) / (x)) * (x))

//------------------------------------------------------------------------------
// Page state defs
//------------------------------------------------------------------------------
typedef enum {
    ALTITUDE,
    SPEED,
    PRESSURE,
    _page_max
} page_e;

typedef enum {
    METERS,
    FEET,
    BANANAS,
    _alt_max
} altitude_mode_e;

typedef enum {
    KPH,
    MPH,
    _speed_max
} speed_mode_e;

typedef enum {
    KPA,
    _pressure_max
} pressure_mode_e;


#define STRBUF_SIZE 16


//------------------------------------------------------------------------------
void gui_next_page(void){
    Slate.nv.gui_state.page++;
    if(Slate.nv.gui_state.page >= _page_max) Slate.nv.gui_state.page = 0;
    gui_refresh_display();
}

void gui_next_page_mode(void){
    switch(Slate.nv.gui_state.page){
        case ALTITUDE: {
            Slate.nv.gui_state.altitude_mode++;
            if(Slate.nv.gui_state.altitude_mode >= _alt_max) Slate.nv.gui_state.altitude_mode = 0;
            break;
        }
        case SPEED: {
            Slate.nv.gui_state.speed_mode++;
            if(Slate.nv.gui_state.speed_mode >= _speed_max) Slate.nv.gui_state.speed_mode = 0;
            break;
        }
        case PRESSURE: {
            Slate.nv.gui_state.pressure_mode++;
            if(Slate.nv.gui_state.pressure_mode >= _pressure_max) Slate.nv.gui_state.pressure_mode = 0;
            break;
        }
        default: {
            Slate.nv.gui_state.page = 0;
            break;
        }
    }
    gui_refresh_display();
}

static void get_altitude_str(char *str){
    float alt = Slate.current_altitude;
    switch(Slate.nv.gui_state.altitude_mode){
        case METERS: {
            alt = ROUND_TO(alt, 0.2);
            snprintf(str, STRBUF_SIZE, "%.1f m", alt);
            break;
        }
        case FEET: {
            alt *= 3.28084;
            alt = ROUND_TO(alt, 0.5);
            snprintf(str, STRBUF_SIZE, "%.1f FT", alt);
            break;
        }
        case BANANAS: {
            alt *= 5.618;
            alt = ROUND_TO(alt, 0.5);
            snprintf(str, STRBUF_SIZE, "%.1f Bn", alt);
            break;
        }
    }

    if(Slate.gps.altitude_accuracy > (50U*1000U)){
        // No GPS signal.
        // Indicate with a dot
        strcat(str, ".");
    }

}


#define MIN_SPEED_ACCURACY_CMPS 750
static void get_speed_str(char *str){
    uint16_t speed;
    switch(Slate.nv.gui_state.speed_mode){
        case KPH: {
            if(Slate.gps.speed_accuracy < MIN_SPEED_ACCURACY_CMPS) {
                speed = roundf(Slate.gps.speed * 0.036);
                snprintf(str, STRBUF_SIZE, "%d kph", speed);
            } else {
                snprintf(str, STRBUF_SIZE, "---- kph");
            }
            break;
        }
        case MPH: {
            if(Slate.gps.speed_accuracy < MIN_SPEED_ACCURACY_CMPS) {
                speed = roundf(Slate.gps.speed * 0.022369363);
                snprintf(str, STRBUF_SIZE, "%d mph", speed);
            } else {
                snprintf(str, STRBUF_SIZE, "---- mph");
            }
            break;
        }
    }

}

static void get_pressure_str(char *str){
    snprintf(str, STRBUF_SIZE, "%.1f KPA", (Slate.alt.pressure * 0.001));
}

void gui_refresh_display(void){
    char str[STRBUF_SIZE];

    switch(Slate.nv.gui_state.page){
        case ALTITUDE: {
            get_altitude_str(str);
            break;
        }
        case SPEED: {
            get_speed_str(str);
            break;
        }
        case PRESSURE: {
            get_pressure_str(str);
            break;
        }
    }
    display_update_str(str, ALIGN_RIGHT);
}
