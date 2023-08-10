#include <stdint.h>

#include "display.h"

// 14-segment bit-code mapping
#define _A  (1<<13)
#define _B  (1<<14)
#define _C  (1<<0)
#define _D  (1<<2)
#define _E  (1<<7)
#define _F  (1<<8)
#define _G  (1<<10)
#define _H  (1<<11)
#define _J  (1<<3)
#define _K  (1<<4)
#define _L  (1<<5)
#define _M  (1<<6)
#define _N  (1<<12)
#define _P  (1<<9)
#define _DP (1<<1)


static const uint16_t ALPHAS[26] = {
    _A | _B | _C | _E | _F | _N | _J,   // A
    _A | _B | _C | _D | _J | _G | _L,   // B
    _A | _D | _E | _F,                  // C
    _A | _B | _C | _D | _G | _L,        // D
    _A | _D | _E | _F | _N | _J,        // E
    _A | _E | _F | _N | _J,             // F
    _A | _C | _D | _E | _F | _J,        // G
    _B | _C | _E | _F | _N | _J,        // H
    _A | _D | _G | _L,                  // I
    _B | _C | _D | _E,                  // J
    _E | _F | _N | _H | _K,             // K
    _D | _E | _F,                       // L
    _B | _C | _E | _F | _P | _H,        // M
    _B | _C | _E | _F | _P | _K,        // N
    _A | _B | _C | _D | _E | _F,        // O
    _A | _B | _E | _F | _N | _J,        // P
    _A | _B | _C | _D | _E | _F | _K,   // Q
    _A | _B | _E | _F | _N | _J | _K,   // R
    _A | _C | _D | _J | _P,             // S
    _A | _G | _L,                       // T
    _B | _C | _D | _E | _F,             // U
    _E | _F | _H | _M,                  // V
    _B | _C | _E | _F | _K | _M,        // W
    _P | _H | _K | _M,                  // X
    _P | _H | _L,                       // Y
    _A | _D | _H | _M                   // Z
};

static const uint16_t DIGITS[10] = {
    _A | _B | _C | _D | _E | _F,            // 0
    _B | _C | _H,                           // 1
    _A | _B | _D | _E | _N | _J,            // 2
    _A | _B | _C | _D | _J,                 // 3
    _B | _C | _F | _N | _J,                 // 4
    _A | _C | _D | _F | _N | _J,            // 5
    _A | _C | _D | _E | _F | _N | _J,       // 6
    _A | _H | _L,                           // 7
    _A | _B | _C | _D | _E | _F | _N | _J,  // 8
    _A | _B | _C | _F | _N | _J             // 9
};

static uint16_t code_from_ascii(char c){
    if(c >= '0' && c <= '9') return DIGITS[c - '0'];
    if(c == '.') return _DP;
    if(c == ' ') return 0;
    if(c >= 'A' && c <= 'Z') return ALPHAS[c - 'A'];
    if(c >= 'a' && c <= 'z') return ALPHAS[c - 'a'];
    if(c == '+') return (_G | _J | _L | _N);
    if(c == '-') return (_J | _N);
    if(c == '/') return (_H | _M);
    if(c == '\\') return (_P | _K);
    if(c == '_') return _D;
    if(c == '*') return (_F | _A | _B | _N | _J);

    // If invalid, light up everything except DP
    return (_A | _B | _C | _D | _E | _F | _G | _H | _J | _K | _L | _M | _N | _P);
}

void display_data_from_str(uint16_t *data, const char *str, text_align_t align){
    uint8_t sidx = 0;
    uint8_t didx = 0;

    // Convert string to 14-segment LED digit codes
    while(str[sidx]){
        if(didx == N_LED_DIGITS) break;

        if(str[sidx] == '.' && didx > 0) {
            // merge decimal point into prior digit code
            data[didx-1] |= _DP;
            sidx++;
            continue;
        }

        data[didx] = code_from_ascii(str[sidx]);
        didx++;
        sidx++;
    }

    if(didx == N_LED_DIGITS){
        // Alignment not relevant. Text fills the display.
        return;
    }

    switch (align) {
        case ALIGN_LEFT: {
            // Text is already left-aligned.
            // Just need to blank out the rest of the buffer
            while(didx < N_LED_DIGITS){
                data[didx] = 0;
                didx++;
            }
            break;
        }
        case ALIGN_RIGHT: {
            uint8_t didx2 = N_LED_DIGITS-1;

            // shift digits
            while(didx != 0) {
                didx--;
                data[didx2] = data[didx];
                didx2--;
            }

            // zero fill left side
            data[didx2] = 0;
            while(didx2 != 0){
                didx2--;
                data[didx2] = 0;
            }
            break;
        }
    }
}
