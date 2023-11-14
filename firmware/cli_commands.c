
#include <stdio.h>
#include <string.h>

#include "cli_commands.h"
#include "display/display.h"
#include "slate.h"
#include "fram.h"


//==============================================================================
// Device-specific output functions
//==============================================================================
void cli_puts(char *str){
    fputs(str, stdout);
}

void cli_putc(char chr){
    putchar(chr);
}

void cli_print_prompt(void){
    putchar('>');
}

void cli_print_error(int error){
    printf("Returned with error code %x\n", error);
}

void cli_print_notfound(char *strcmd){
    fputs("Command not found\n", stdout);
}

//==============================================================================
// Custom Commands
//==============================================================================

int cmd_ArgList(uint8_t argc, char *argv[]){
    cli_puts("Argument List:\r\n");
    int i;
    for(i=0;i<argc;i++){
        cli_putc('[');
        cli_puts(argv[i]);
        cli_puts("]\r\n");
    }
    return 0;
}

static uint8_t lightness_values[8] = {
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255
};
int cmd_SetLightness(uint8_t argc, char *argv[]){
    if(argc != 3) return 1;

    int idx;
    int value;
    sscanf(argv[1], "%d", &idx);
    sscanf(argv[2], "%d", &value);

    if(idx >= 8) return 1;
    if(value > 255) return 1;

    if(idx < 0){
        for(int i=0; i<8; i++){
            lightness_values[i] = value;
        }
    } else {
        lightness_values[idx] = value;
    }

    if(argv[0][1] == 'p'){
        display_set_pwm_raw(lightness_values);
    } else {
        display_set_lightness(lightness_values);
    }
    return 0;
}

int cmd_DumpSlate(uint8_t argc, char *argv[]){
    printf("gps.altitude: %ld +/- %lu\n", Slate.gps.altitude, Slate.gps.altitude_accuracy);
    printf("gps.speed: %lu +/- %lu\n", Slate.gps.speed, Slate.gps.speed_accuracy);
    printf("gps.heading: %ld +/- %lu\n", Slate.gps.heading, Slate.gps.heading_accuracy);
    printf("gps.good_frame_count: %u\n", Slate.gps.good_frame_count);
    printf("gps.bad_frame_count: %u\n", Slate.gps.bad_frame_count);

    printf("light.vis: %u\n", Slate.light.vis);
    printf("light.ir: %u\n", Slate.light.ir);

    printf("alt.temperature: %ld\n", Slate.alt.temperature);
    printf("alt.pressure: %ld\n", Slate.alt.pressure);

    printf("sensor_poll_duration: %u\n", Slate.sensor_poll_duration);

    printf("current_alt: %.2f\n", Slate.current_altitude);
    printf("alt_trim: %.2f\n", Slate.altitude_trim);

    printf("gui_state.page: %d\n", Slate.nv.gui_state.page);
    printf("gui_state.altitude_mode: %d\n", Slate.nv.gui_state.altitude_mode);
    printf("gui_state.speed_mode: %d\n", Slate.nv.gui_state.speed_mode);
    printf("gui_state.pressure_mode: %d\n", Slate.nv.gui_state.pressure_mode);

    return 0;
}


int cmd_Debug(uint8_t argc, char *argv[]){
    return 0;
}
