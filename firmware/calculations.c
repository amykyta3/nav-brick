#include <math.h>
#include "slate.h"

double get_px_altitude(void){
    // Barometric formula (https://en.wikipedia.org/wiki/Barometric_formula)
    // Assuming T_b = 15C
    double x;

    x = Slate.pressure;
    x /= 101325.0;
    x = 1.0 - pow(x, 1.0 / 5.225787741);
    x *= 44330.77;
    return x;
}
