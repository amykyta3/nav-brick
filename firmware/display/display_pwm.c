#include <stdint.h>
#include <avr/io.h>
#include <avr/cpufunc.h>

#include "../utils/fixedpt.h"

/*
* PWM digit mapping (left to right) is as follows:
*   Char0 = PD0 = TCA0.WO0:LCMP0
*   Char1 = PD3 = TCA0.WO3:HCMP0
*   Char2 = PD1 = TCA0.WO1:LCMP1
*   Char3 = PD4 = TCA0.WO4:HCMP1
*   Char4 = PD2 = TCA0.WO2:LCMP2
*   Char5 = PD5 = TCA0.WO5:HCMP2
*   Char6 = PF1 = TCD0.WOB
*   Char7 = PF0 = TCD0.WOA
*/

static void tca0_init(void){
   // Use TCA0 in split mode to maximize output channels
   TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;

   // PWM freq = CLK_PER / (DIV * PER)
   // 366.2 Hz = 24 MHz  / (256 * 256)
   TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV256_gc;

   // Enable waveform outputs
   TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm
                    | TCA_SPLIT_HCMP1EN_bm
                    | TCA_SPLIT_HCMP2EN_bm
                    | TCA_SPLIT_LCMP0EN_bm
                    | TCA_SPLIT_LCMP1EN_bm
                    | TCA_SPLIT_LCMP2EN_bm
                    ;

    // Set all PWM outputs to 0
    TCA0.SPLIT.CTRLC = 0;

    // Maximum period
    TCA0.SPLIT.LPER = 0xFF;
    TCA0.SPLIT.HPER = 0xFF;

    // PWM duty of 0
    TCA0.SPLIT.LCMP0 = 0;
    TCA0.SPLIT.LCMP1 = 0;
    TCA0.SPLIT.LCMP2 = 0;
    TCA0.SPLIT.HCMP0 = 0;
    TCA0.SPLIT.HCMP1 = 0;
    TCA0.SPLIT.HCMP2 = 0;

    // Offset PWM phases by 180 degrees so that digit pairs do not overlap
    TCA0.SPLIT.LCNT = 0;
    TCA0.SPLIT.HCNT = 128;

    // Enable TCA
    TCA0.SPLIT.CTRLA |= TCA_SPLIT_ENABLE_bm;
}

static void tcd0_init(void){
    // PWM freq = CLK_PER / (SYNCPRESC * CNTPRESC * (CMPACLR + CMPBCLR + 2))
    // 366.2 Hz = 24 MHz  / (8         * 32       * (128     + 128     + 2))
    TCD0.CTRLA = TCD_CLKSEL_CLKPER_gc
               | TCD_CNTPRES_DIV32_gc
               | TCD_SYNCPRES_DIV8_gc
               ;
    TCD0.CTRLB = TCD_WGMODE_TWORAMP_gc;
    TCD0.CTRLC = 0;

    ccp_write_io((void*) &TCD0.FAULTCTRL, TCD_CMPAEN_bm | TCD_CMPBEN_bm);

    // CMPxCLR dictates each ramp's period
    TCD0.CMPACLR = 127;
    TCD0.CMPBCLR = 127;

    // CMPxSET controls the duty cycle, but it is inverted
    TCD0.CMPASET = 127;
    TCD0.CMPBSET = 127;

    // Enable the counter
    TCD0.CTRLA |= TCD_ENABLE_bm;
}

static uint8_t pwm_from_lightness(uint8_t L){
    uint8_t pwm;
    uint8_t L_2;
    uint8_t L_3;
    // Convert lightness to PWM value
    // Y = 0.5*L^2 + 0.5*L^3 (where Y and L are from 0.0 to 1.0)
    // ... is a surprisingly close approximation to the CIELAB lightness formula

    // Interpret L as Q0.8 fixed point
    L_2 = mpy_Q8(L, L);
    L_3 = mpy_Q8(L_2, L);
    L_2 >>= 1;
    L_3 >>= 1;

    pwm = L_2 + L_3;

    // PWM is from 0-127. Scale down
    pwm = pwm >> 1;

    // Clamp to min brightness if nonzero
    if(pwm == 0 && L != 0) pwm = 1;

    return(pwm);
}

void display_pwm_init(void){
    tca0_init();
    tcd0_init();
}

void display_set_lightness(uint8_t *values){
    TCA0.SPLIT.LCMP0 = pwm_from_lightness(values[0]);
    TCA0.SPLIT.HCMP0 = pwm_from_lightness(values[1]);
    TCA0.SPLIT.LCMP1 = pwm_from_lightness(values[2]);
    TCA0.SPLIT.HCMP1 = pwm_from_lightness(values[3]);
    TCA0.SPLIT.LCMP2 = pwm_from_lightness(values[4]);
    TCA0.SPLIT.HCMP2 = pwm_from_lightness(values[5]);

    TCD0.CMPBSETL = 127 - pwm_from_lightness(values[6]);
    TCD0.CMPASETL = 127 - pwm_from_lightness(values[7]);
    TCD0.CTRLE = TCD_SYNCEOC_bm;
}

void display_set_lightness_all(uint8_t value) {
    uint8_t values[8];
    for(uint8_t i=0; i<8; i++) values[i] = value;
    display_set_lightness(values);
}

void display_set_pwm_raw(uint8_t *values){
    TCA0.SPLIT.LCMP0 = (values[0] >> 1);
    TCA0.SPLIT.HCMP0 = (values[1] >> 1);
    TCA0.SPLIT.LCMP1 = (values[2] >> 1);
    TCA0.SPLIT.HCMP1 = (values[3] >> 1);
    TCA0.SPLIT.LCMP2 = (values[4] >> 1);
    TCA0.SPLIT.HCMP2 = (values[5] >> 1);

    TCD0.CMPBSETL = 127 - (values[6] >> 1);
    TCD0.CMPASETL = 127 - (values[7] >> 1);
    TCD0.CTRLE = TCD_SYNCEOC_bm;
}
