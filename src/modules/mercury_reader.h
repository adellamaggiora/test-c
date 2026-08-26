#ifndef MERCURY_READER_H
#define MERCURY_READER_H
#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    bool is_new;
    float dose_rate;
} GammaDoseRateResult;

typedef enum
{
    MERCURY_READER_OK = 0,
    MERCURY_READER_INVALID_ARGUMENT,
    MERCURY_READER_NOT_INITIALIZED,
    MERCURY_READER_ALLOCATION_ERROR
} MercuryReaderError;

typedef struct
{
    bool is_new;
    unsigned int events;
    MercuryReaderError error;
} GammaCountsResult;

GammaDoseRateResult get_gamma_dose_rate(void);

/*
 * Temporary simulated backend. The same interface can be kept when the
 * native Mercury protocol is connected.
 */
MercuryReaderError mercury_reader_init(size_t total_tubes);
void mercury_reader_close(void);
MercuryReaderError set_hv(size_t tube_index, bool enabled, int voltage);
GammaCountsResult get_instant_measurement_gm(size_t tube_index);

#endif
