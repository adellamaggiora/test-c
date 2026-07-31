#include <stdio.h>
#include <stdlib.h>
#include "mercury_reader.h"


GammaDoseRateResult get_gamma_dose_rate(void) {
    int max = 100;
    int value = rand();
    float random = (float)(value % max);
    GammaDoseRateResult result;
    result.is_new = true;
    result.dose_rate = random;
    return result;
}


    