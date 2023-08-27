#include "gps_uart.h"
#include "../slate.h"

typedef struct {
    uint8_t data[64];
    union {
        uint8_t len;
        uint8_t pos;
    };
} ubx_frame_t;


//------------------------------------------------------------------------------
// UBX frame manipulation
//------------------------------------------------------------------------------
static void push_U1(ubx_frame_t *f, uint8_t v){
    f->data[f->len++] = v;
}

static void push_U2(ubx_frame_t *f, uint16_t v){
    f->data[f->len++] = v & 0xFF;
    v = v >> 8;
    f->data[f->len++] = v & 0xFF;
}

static void push_U4(ubx_frame_t *f, uint32_t v){
    f->data[f->len++] = v & 0xFF;
    for(uint8_t i=1; i<4; i++){
        v = v >> 8;
        f->data[f->len++] = v & 0xFF;
    }
}

static uint8_t pop_U1(ubx_frame_t *f){
    return f->data[f->pos++];
}

static uint16_t pop_U2(ubx_frame_t *f){
    uint16_t d;
    d = f->data[f->pos++];
    d |= ((uint16_t)f->data[f->pos++]) << 8;
    return d;
}

static uint32_t pop_U4(ubx_frame_t *f){
    uint32_t d = 0;
    for(uint8_t i=0; i<4; i++){
        d |= ((uint32_t)f->data[f->pos++]) << (i*8);
    }
    return d;
}

#define pop_I1(f) ((int8_t)pop_U1(f))
#define pop_I2(f) ((int16_t)pop_U2(f))
#define pop_I4(f) ((int32_t)pop_U4(f))

static uint16_t calc_checksum(ubx_frame_t *f){
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    for(uint8_t i=0; i<f->len; i++){
        ck_a += f->data[i];
        ck_b += ck_a;
    }

    uint16_t cksum;
    cksum = ck_b;
    cksum <<= 8;
    cksum += ck_a;
    return cksum;
}

//------------------------------------------------------------------------------
// Send UBX messages
//------------------------------------------------------------------------------
static void send_frame(ubx_frame_t *f){
    // Calculate and append checksum
    push_U2(f, calc_checksum(f));

    // Send preamble
    uart_putc(&gps_uart_dev, 0xb5);
    uart_putc(&gps_uart_dev, 0x62);

    // Send payload
    uart_write(&gps_uart_dev, f->data, f->len);
}

//------------------------------------------------------------------------------
// Receive UBX messages
//------------------------------------------------------------------------------
// Parser states
#define PS_PRE1      0
#define PS_PRE2      1
#define PS_CLASS     2
#define PS_ID        3
#define PS_LENGTH1   4
#define PS_LENGTH2   5
#define PS_PAYLOAD   6
#define PS_CKA       7
#define PS_CKB       8

static void process_frame(ubx_frame_t *f);

void ubx_process_char(uint8_t c){
    static uint8_t state = PS_PRE1;
    static ubx_frame_t f;

    static union {
        uint16_t bytes_remaining;
        uint16_t cksum;
    } v;

    switch (state){
        case PS_PRE1: {
            if(c == 0xb5) state = PS_PRE2;
            else state = PS_PRE1;
            break;
        }

        case PS_PRE2: {
            f.len = 0;
            if(c == 0x62) state = PS_CLASS;
            else state = PS_PRE1;
            break;
        }

        case PS_CLASS: {
            f.data[f.len++] = c;
            state = PS_ID;
            break;
        }

        case PS_ID: {
            f.data[f.len++] = c;
            state = PS_LENGTH1;
            break;
        }

        case PS_LENGTH1: {
            f.data[f.len++] = c;
            v.bytes_remaining = c;
            state = PS_LENGTH2;
            break;
        }

        case PS_LENGTH2: {
            f.data[f.len++] = c;
            v.bytes_remaining |= ((uint16_t)c) << 8;
            if(v.bytes_remaining == 0) state = PS_CKA;
            else state = PS_PAYLOAD;
            break;
        }

        case PS_PAYLOAD: {
            f.data[f.len++] = c;
            v.bytes_remaining--;
            if(v.bytes_remaining == 0) state = PS_CKA;
            break;
        }

        case PS_CKA: {
            v.cksum = c;
            state = PS_CKB;
            break;
        }

        case PS_CKB: {
            v.cksum |= ((uint16_t)c) << 8;
            if(v.cksum == calc_checksum(&f)){
                // Frame is good!
                // reset position and process it
                f.pos = 0;
                process_frame(&f);
            }
            state = PS_PRE1;
            break;
        }
    }
}

static void process_POSLLH_frame(ubx_frame_t *f);
static void process_VELNED_frame(ubx_frame_t *f);

static void process_frame(ubx_frame_t *f){
    uint16_t class_id; // {class, id}

    class_id = pop_U1(f);
    class_id <<= 8;
    class_id |= pop_U1(f);

    // Discard length
    pop_U2(f);

    switch(class_id){
        case 0x0102: {
            process_POSLLH_frame(f);
            break;
        }
        case 0x0112: {
            process_VELNED_frame(f);
            break;
        }
    }
}

static void process_POSLLH_frame(ubx_frame_t *f){
    int32_t hMSL;
    uint32_t vAcc;
    pop_U4(f); // iTOW
    pop_I4(f); // lon
    pop_I4(f); // lat
    pop_I4(f); // height
    hMSL = pop_I4(f);
    pop_U4(f); // hAcc
    vAcc = pop_U4(f);

    // Save to slate
    Slate.gps.altitude_mm = hMSL;
    Slate.gps.altitude_accuracy_mm = vAcc;
}

static void process_VELNED_frame(ubx_frame_t *f){
    uint32_t speed;
    int32_t heading;
    uint32_t sAcc;
    uint32_t cAcc;

    pop_U4(f); // iTOW
    pop_I4(f); // velN
    pop_I4(f); // velE
    pop_I4(f); // velD
    speed = pop_U4(f);
    pop_U4(f); // gSpeed
    heading = pop_I4(f);
    sAcc = pop_U4(f);
    cAcc = pop_U4(f);

    // Save to slate
    Slate.gps.speed_cmps = speed;
    Slate.gps.speed_accuracy_cmps = sAcc;
    Slate.gps.heading_deg_1em5 = heading;
    Slate.gps.heading_accuracy_deg_1em5 = cAcc;
}

//------------------------------------------------------------------------------
#define CFG_MSGOUT_NMEA_ID_GGA_UART1 0x209100bb
#define CFG_MSGOUT_NMEA_ID_GLL_UART1 0x209100ca
#define CFG_MSGOUT_NMEA_ID_GSA_UART1 0x209100c0
#define CFG_MSGOUT_NMEA_ID_GSV_UART1 0x209100c5
#define CFG_MSGOUT_NMEA_ID_RMC_UART1 0x209100ac
#define CFG_MSGOUT_NMEA_ID_VTG_UART1 0x209100b1

#define CFG_MSGOUT_UBX_NAV_POSLLH_UART1 0x2091002a
#define CFG_MSGOUT_UBX_NAV_VELNED_UART1 0x20910043

void ubx_init(void){
    ubx_frame_t f;

    // Disable all NMEA messages
    f.len = 0;
    push_U1(&f, 0x06); // class
    push_U1(&f, 0x8A); // id
    push_U2(&f, 34); // length
    push_U1(&f, 0); // valset.version
    push_U1(&f, 1); // valset.layers: RAM
    push_U1(&f, 0); // valset.reserved
    push_U1(&f, 0); // valset.reserved
    push_U4(&f, CFG_MSGOUT_NMEA_ID_GGA_UART1);
    push_U1(&f, 0);
    push_U4(&f, CFG_MSGOUT_NMEA_ID_GLL_UART1);
    push_U1(&f, 0);
    push_U4(&f, CFG_MSGOUT_NMEA_ID_GSA_UART1);
    push_U1(&f, 0);
    push_U4(&f, CFG_MSGOUT_NMEA_ID_GSV_UART1);
    push_U1(&f, 0);
    push_U4(&f, CFG_MSGOUT_NMEA_ID_RMC_UART1);
    push_U1(&f, 0);
    push_U4(&f, CFG_MSGOUT_NMEA_ID_VTG_UART1);
    push_U1(&f, 0);
    send_frame(&f);

    // Enable UBX messages I care about
    f.len = 0;
    push_U1(&f, 0x06); // class
    push_U1(&f, 0x8A); // id
    push_U2(&f, 14); // length
    push_U1(&f, 0); // valset.version
    push_U1(&f, 1); // valset.layers: RAM
    push_U1(&f, 0); // valset.reserved
    push_U1(&f, 0); // valset.reserved
    push_U4(&f, CFG_MSGOUT_UBX_NAV_POSLLH_UART1);
    push_U1(&f, 1);
    push_U4(&f, CFG_MSGOUT_UBX_NAV_VELNED_UART1);
    push_U1(&f, 1);
    send_frame(&f);
}
