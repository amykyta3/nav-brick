#include <math.h>
#include "slate.h"

#define GPS_TRIM_SLOWDOWN   128

float get_px_altitude(void){
    // Barometric formula (https://en.wikipedia.org/wiki/Barometric_formula)
    // Assuming T_b = 15C
    float x;

    x = Slate.pressure;
    x /= 101325.0;
    x = 1.0 - powf(x, 1.0 / 5.225787741);
    x *= 44330.77;
    return x;
}

void update_altitude(void){
    float altitude;

    // Start with altitude derived from pressure sensor
    altitude = get_px_altitude();

    // Trim it with current correction factor
    altitude += Slate.altitude_trim;

    Slate.current_altitude = altitude;

    // Update corretion factor if needed
    if(Slate.gps.altitude_accuracy < (50U*1000U)){
        // GPS altitude is accurate enough to consider
        float error;
        error = (0.001 * Slate.gps.altitude) - altitude;

        if(fabsf(error) > (0.001 * Slate.gps.altitude_accuracy)) {
            // Error is greater than GPS accuracy.
            // Do a correction
            Slate.altitude_trim += (error / GPS_TRIM_SLOWDOWN);
        }
    }
}