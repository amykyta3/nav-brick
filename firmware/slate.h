#ifndef SLATE_H
#define SLATE_H

#include <stdint.h>

typedef struct {
    struct {
        int32_t altitude_mm;
        uint32_t altitude_accuracy_mm;
        uint32_t speed_cmps;
        uint32_t speed_accuracy_cmps;
        int32_t heading_deg_1em5;
        uint32_t heading_accuracy_deg_1em5;
    } gps;
} slate_t;

extern slate_t Slate;

void slate_init(void);

#endif
