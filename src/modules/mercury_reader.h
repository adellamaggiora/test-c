#ifndef MERCURY_READER_H
#define MERCURY_READER_H
#include <stdbool.h>

typedef struct
{
    bool is_new;
    float dose_rate;
} GammaDoseRateResult;


GammaDoseRateResult get_gamma_dose_rate(void);

#endif