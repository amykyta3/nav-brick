#ifndef SLATE_H
#define SLATE_H

#include <stdint.h>
#include <stdbool.h>

#include "gui.h"

#define NV_SLATE_VERSION    0

typedef struct {
    gui_state_t gui_state;
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
        uint16_t good_frame_count;
        uint16_t bad_preamble_count;
        uint16_t bad_frame_count;
        uint16_t unhandled_frame_count;
    } gps;

    //--------------------------------------------------------------------------
    // Sensors
    //--------------------------------------------------------------------------
    struct {
        uint16_t vis; // unitless?
        uint16_t ir; // unitless?
    } light;

    struct {
        int32_t temperature; // C * 100
        int32_t pressure; // mbar * 100
    } alt;

    uint16_t sensor_poll_duration;

    //--------------------------------------------------------------------------
    float current_altitude; // meters
    float altitude_trim; // meters
} slate_t;

extern slate_t Slate;

//------------------------------------------------------------------------------
void slate_init(void);
void slate_save_nv(void);



typedef struct {
    uint8_t version;
    uint16_t seq_num;
    nonvolatile_slate_t data;
} entry_payload_t;

typedef struct {
    entry_payload_t payload;
    uint32_t crc;
} fram_entry_t;

void slate_write_nv(fram_entry_t *entry, uint16_t addr);
bool slate_read_nv(fram_entry_t *entry, uint16_t addr);

#endif
