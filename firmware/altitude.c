#include <math.h>
#include "slate.h"

#define GPS_TRIM_SLOWDOWN   128

float get_px_altitude(void){
    // Barometric formula (https://en.wikipedia.org/wiki/Barometric_formula)
    // Assuming T_b = 15C
    float x;

    x = Slate.alt.pressure;
    x /= 101325.0;
    x = 1.0 - powf(x, 1.0 / 5.225787741);
    x *= 44330.77;
    return x;
}

void update_altitude(void){
    static uint8_t startup_grace_period = 10;

    float altitude;

    // Start with altitude derived from pressure sensor
    altitude = get_px_altitude();

    if(startup_grace_period != 0) {
        // For the first few iterations, wait for things to stabilize and assume
        // the altitude is constant.
        startup_grace_period--;

        if(Slate.prev_session_valid) {
            // Assume prior session altitude is the same as current.
            // Shortcut trim to force the altitude to be the same
            Slate.altitude_trim = Slate.nv.prev_session_altitude - altitude;

            // Just in case something gets corrupted...
            if(isnan(Slate.altitude_trim)) Slate.altitude_trim = 0;
        }
    }

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
