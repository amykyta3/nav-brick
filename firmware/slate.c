#include "slate.h"

slate_t Slate;

void slate_init(void){
    // Initialize GPS accuracies to be "bad"
    Slate.gps.altitude_accuracy = 3750000128;
    Slate.gps.speed_accuracy = 2000;
    Slate.gps.heading_accuracy = 18000000;
}
