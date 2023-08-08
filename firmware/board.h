#ifndef BOARD_H
#define BOARD_H

#include <avr/io.h>

// Port A
#define P_USB_UART_RX   PIN0_bm
#define P_USB_UART_TX   PIN1_bm
#define P_I2C_SDA       PIN2_bm
#define P_I2C_SCL       PIN3_bm
#define P_FRAM_MOSI     PIN4_bm
#define P_FRAM_MISO     PIN5_bm
#define P_FRAM_SCK      PIN6_bm
#define P_FRAM_CSN      PIN7_bm

// Port C
#define P_LED_MOSI      PIN0_bm
#define P_LED_MISO      PIN1_bm
#define P_LED_SCK       PIN2_bm
#define P_LED_CSN       PIN3_bm

// Port D
#define P_PWM0          PIN0_bm
#define P_PWM2          PIN1_bm
#define P_PWM4          PIN2_bm
#define P_PWM1          PIN3_bm
#define P_PWM3          PIN4_bm
#define P_PWM5          PIN5_bm
#define P_BUTTON1       PIN6_bm
#define P_BUTTON2       PIN7_bm

// Port F
#define P_PWM7          PIN0_bm
#define P_PWM6          PIN1_bm
#define P_GPS_RSTN      PIN3_bm
#define P_GPS_TX        PIN4_bm
#define P_GPS_RX        PIN5_bm
#define P_PF_UNUSED     (PIN2_bm | PIN6_bm)

// Devices
#define USB_USART   USART0
#define SENSOR_I2C  TWI0
#define FRAM_SPI    SPI0

#define LED_SPI             SPI1
#define LED_SPI_INT_vect    SPI1_INT_vect

#define GPS_USART   USART2

#endif
