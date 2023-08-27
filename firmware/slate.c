#include "slate.h"

slate_t Slate;

void slate_init(void){
    // Initialize GPS accuracies to be "bad"
    Slate.gps.altitude_accuracy_mm = 3750000128;
    Slate.gps.speed_accuracy_cmps = 2000;
    Slate.gps.heading_accuracy_deg_1em5 = 18000000;
}
