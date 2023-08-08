
#include <stdio.h>
#include "cli_commands.h"

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
    cli_puts("\r\n>");
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

int cmd_Hello(uint8_t argc, char *argv[]){
    cli_puts("Hello World\r\n");
    return(0);
}

//------------------------------------------------------------------------------
int cmd_ArgList(uint8_t argc, char *argv[]){
    cli_puts("Argument List:\r\n");
    int i;
    for(i=0;i<argc;i++){
        cli_putc('[');
        cli_puts(argv[i]);
        cli_puts("]\r\n");
    }
    return(0);
}
