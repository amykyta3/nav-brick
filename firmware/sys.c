#include <avr/io.h>
#include <avr/cpufunc.h>
#include "board.h"

FUSES = {
    .WDTCFG = WDT_WINDOW_OFF_gc | WDT_PERIOD_OFF_gc,
    .BODCFG = BOD_ACTIVE_DIS_gc | BOD_SLEEP_DIS_gc,
    .OSCCFG = CLKSEL_OSCHF_gc,
    .SYSCFG0 = CRCSRC_NOCRC_gc | RSTPINCFG_GPIO_gc,
    .SYSCFG1 = SUT_64MS_gc,
    .CODESIZE = 0,
    .BOOTSIZE = 0
};

static void io_init(void){
    // Initialize i/o to safe defaults

    // Port A
    PORTA.OUT = P_USB_UART_TX | P_FRAM_CSN;
    PORTA.PIN2CTRL = PORT_PULLUPEN_bm; // I2C SDA
    PORTA.PIN3CTRL = PORT_PULLUPEN_bm; // I2C SCL
    PORTA.DIR = P_USB_UART_TX | P_FRAM_MOSI | P_FRAM_SCK | P_FRAM_CSN;

    // Port C
    PORTC.OUT = P_LED_CSN;
    PORTC.DIR = P_LED_CSN | P_LED_MOSI | P_LED_SCK;

    // Port D
    PORTD.OUT = 0; // All PWMs off
    PORTD.DIR = P_PWM0 | P_PWM2 | P_PWM4 | P_PWM1 | P_PWM3 | P_PWM5;
    PORTD.PIN6CTRL = PORT_PULLUPEN_bm; // BUTTON1
    PORTD.PIN7CTRL = PORT_PULLUPEN_bm; // BUTTON2

    // Port F
    PORTF.OUT = P_GPS_TX;
    PORTF.DIR = P_PWM7 | P_PWM6 | P_GPS_TX;

    PORTMUX.SPIROUTEA = PORTMUX_SPI0_DEFAULT_gc | PORTMUX_SPI1_DEFAULT_gc;
    PORTMUX.USARTROUTEA = PORTMUX_USART0_DEFAULT_gc | PORTMUX_USART2_ALT1_gc;
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTD_gc;
    PORTMUX.TCDROUTEA = PORTMUX_TCD0_ALT2_gc;
}

static void clk_init(void){
    // Set OSCHF to 24 MHz
    ccp_write_io((void*) &CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc);

    // Select OSCHF for CLK_MAIN
    ccp_write_io((void*)&CLKCTRL.MCLKCTRLA, 0);

    // Disable peripheral clock division for CLK_PER
    ccp_write_io((void*)&CLKCTRL.MCLKCTRLB, 0);

    // Force internal 32k osc on
    ccp_write_io((void*)&CLKCTRL.OSC32KCTRLA, CLKCTRL_RUNSTDBY_bm);
}

void sys_init(void){
    io_init();
    clk_init();
}
