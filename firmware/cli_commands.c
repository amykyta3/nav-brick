
#include <stdio.h>
#include <string.h>

#include "cli_commands.h"
#include "display/display.h"
#include "sensors/ms5611_pressure.h"
#include "sensors/ltr-329als_light.h"
#include "slate.h"
#include "calculations.h"


//==============================================================================
// Device-specific output functions
//==============================================================================
#include "usb_uart.h"

#define cli_put_x8(num) uart_put_x8(&usb_uart_dev, num)
#define cli_put_x16(num) uart_put_x16(&usb_uart_dev, num)
#define cli_put_x32(num) uart_put_x32(&usb_uart_dev, num)
#define cli_put_d8(num) uart_put_d8(&usb_uart_dev, num)
#define cli_put_d16(num) uart_put_d16(&usb_uart_dev, num)
#define cli_put_d32(num) uart_put_d32(&usb_uart_dev, num)
#define cli_put_sd8(num) uart_put_sd8(&usb_uart_dev, num)
#define cli_put_sd16(num) uart_put_sd16(&usb_uart_dev, num)
#define cli_put_sd32(num) uart_put_sd32(&usb_uart_dev, num)

void cli_puts(char *str){
    uart_puts(&usb_uart_dev, str);
}

void cli_putc(char chr){
    uart_putc(&usb_uart_dev, chr);
}

void cli_print_prompt(void){
    cli_puts(">");
}

void cli_print_error(int error){
    cli_puts("Returned with error code ");
    cli_put_x32(error);
    cli_puts("\r\n");
}

void cli_print_notfound(char *strcmd){
    cli_puts("Command not found\r\n");
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

int cmd_DisplayLeft(uint8_t argc, char *argv[]){
    if(argc != 2) return 1;
    display_update_str(argv[1], ALIGN_LEFT);
    return 0;
}

int cmd_DisplayRight(uint8_t argc, char *argv[]){
    if(argc != 2) return 1;
    display_update_str(argv[1], ALIGN_RIGHT);
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
    cli_puts("gps.altitude: ");
    cli_put_sd32(Slate.gps.altitude);
    cli_puts(" +/- ");
    cli_put_d32(Slate.gps.altitude_accuracy);

    cli_puts("\r\ngps.speed: ");
    cli_put_d32(Slate.gps.speed);
    cli_puts(" +/- ");
    cli_put_d32(Slate.gps.speed_accuracy);

    cli_puts("\r\ngps.heading: ");
    cli_put_sd32(Slate.gps.heading);
    cli_puts(" +/- ");
    cli_put_d32(Slate.gps.heading_accuracy);

    cli_puts("\r\nlight.vis: ");
    cli_put_d16(Slate.light.vis);
    cli_puts("\r\nlight.ir: ");
    cli_put_d16(Slate.light.ir);

    cli_puts("\r\ntemperature: ");
    cli_put_d32(Slate.temperature);

    cli_puts("\r\npressure: ");
    cli_put_d32(Slate.pressure);

    cli_puts("\r\npoll_duration: ");
    cli_put_d16(Slate.poll_duration);

    cli_puts("\r\n");

    return 0;
}

int cmd_Altitude(uint8_t argc, char *argv[]){
    double alt;

    alt = get_px_altitude();
    alt *= 10;

    cli_puts("altitude: ");
    cli_put_sd32(alt);
    cli_puts(" mm\r\n");

    return 0;
}

int cmd_RTC_State(uint8_t argc, char *argv[]){
    cli_puts("CNT: ");
    cli_put_d16(RTC.CNT);
    cli_puts(", CMP: ");
    cli_put_d16(RTC.CMP);
    cli_puts("\r\n");
    return 0;
}
