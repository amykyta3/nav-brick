#include <stdio.h>
#include <string.h>

#include "slate.h"
#include "display/display.h"

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
    switch(Slate.nv.gui_state.altitude_mode){
        case METERS: {
            snprintf(str, STRBUF_SIZE, "%.1f m", Slate.current_altitude);
            break;
        }
        case FEET: {
            snprintf(str, STRBUF_SIZE, "%.1f FT", (Slate.current_altitude * 3.28084));
            break;
        }
        case BANANAS: {
            snprintf(str, STRBUF_SIZE, "%.1f Bn", (Slate.current_altitude * 5.618));
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
    switch(Slate.nv.gui_state.speed_mode){
        case KPH: {
            if(Slate.gps.speed_accuracy < MIN_SPEED_ACCURACY_CMPS) {
                snprintf(str, STRBUF_SIZE, "%.1f KPH", (Slate.gps.speed * 0.036));
            } else {
                snprintf(str, STRBUF_SIZE, "---- KPH");
            }
            break;
        }
        case MPH: {
            if(Slate.gps.speed_accuracy < MIN_SPEED_ACCURACY_CMPS) {
                snprintf(str, STRBUF_SIZE, "%.1f MPH", (Slate.gps.speed * 0.022369363));
            } else {
                snprintf(str, STRBUF_SIZE, "---- MPH");
            }
            break;
        }
    }

}

static void get_pressure_str(char *str){
    snprintf(str, STRBUF_SIZE, "%.1f KPA", (Slate.pressure * 0.001));
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
