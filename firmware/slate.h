#ifndef SLATE_H
#define SLATE_H

#include <stdint.h>
#include <stdbool.h>

#define NV_SLATE_VERSION    3

typedef struct {
    float prev_session_altitude;
} nonvolatile_slate_t;

typedef struct {
    nonvolatile_slate_t nv;
    bool prev_session_valid;
    //--------------------------------------------------------------------------
    // GPS
    //--------------------------------------------------------------------------
    struct {
        int32_t altitude; // mm
        uint32_t altitude_accuracy; // mm
        uint32_t speed; // cm/sec
        uint32_t speed_accuracy; // cm/sec
        int32_t heading; // Degrees * 100,000
        uint32_t heading_accuracy; // Degrees * 100,000
    } gps;

    //--------------------------------------------------------------------------
    // Sensors
    //--------------------------------------------------------------------------
    struct {
        uint16_t vis; // unitless?
        uint16_t ir; // unitless?
    } light;

    int32_t temperature; // C * 100
    int32_t pressure; // mbar * 100

    uint16_t sensor_poll_duration;

    //--------------------------------------------------------------------------
    float current_altitude; // meters
    float altitude_trim; // meters
} slate_t;

extern slate_t Slate;

void slate_init(void);
void slate_save_nv(void);

#endif
