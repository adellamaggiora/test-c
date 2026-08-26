#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mercury_reader.h"

typedef struct
{
    bool hv_enabled;
    int voltage;
    int simulated_starting_voltage;
    uint32_t sample_number;
} SimulatedTube;

static SimulatedTube *simulated_tubes = NULL;
static size_t simulated_tube_count = 0;

MercuryReaderError mercury_reader_init(size_t total_tubes)
{
    if (total_tubes == 0)
    {
        return MERCURY_READER_INVALID_ARGUMENT;
    }

    mercury_reader_close();

    simulated_tubes = calloc(total_tubes, sizeof(SimulatedTube));
    if (simulated_tubes == NULL)
    {
        return MERCURY_READER_ALLOCATION_ERROR;
    }

    simulated_tube_count = total_tubes;

    for (size_t i = 0; i < total_tubes; i++)
    {
        /* Deterministic thresholds make development runs reproducible. */
        simulated_tubes[i].simulated_starting_voltage =
            320 + (int)(i % 5) * 10;
    }

    return MERCURY_READER_OK;
}

void mercury_reader_close(void)
{
    free(simulated_tubes);
    simulated_tubes = NULL;
    simulated_tube_count = 0;
}

MercuryReaderError set_hv(size_t tube_index, bool enabled, int voltage)
{
    if (simulated_tubes == NULL)
    {
        return MERCURY_READER_NOT_INITIALIZED;
    }

    if (tube_index >= simulated_tube_count || voltage < 0)
    {
        return MERCURY_READER_INVALID_ARGUMENT;
    }

    simulated_tubes[tube_index].hv_enabled = enabled;
    simulated_tubes[tube_index].voltage = enabled ? voltage : 0;
    simulated_tubes[tube_index].sample_number = 0;

    return MERCURY_READER_OK;
}

GammaCountsResult get_instant_measurement_gm(size_t tube_index)
{
    GammaCountsResult result = {
        .is_new = false,
        .events = 0,
        .error = MERCURY_READER_OK};

    if (simulated_tubes == NULL)
    {
        result.error = MERCURY_READER_NOT_INITIALIZED;
        return result;
    }

    if (tube_index >= simulated_tube_count)
    {
        result.error = MERCURY_READER_INVALID_ARGUMENT;
        return result;
    }

    SimulatedTube *tube = &simulated_tubes[tube_index];
    tube->sample_number++;
    result.is_new = true;

    if (tube->hv_enabled &&
        tube->voltage >= tube->simulated_starting_voltage)
    {
        result.events = 3U +
            (unsigned int)((tube->voltage -
                            tube->simulated_starting_voltage) / 10);
    }

    return result;
}

GammaDoseRateResult get_gamma_dose_rate(void) {
    int max = 100;
    int value = rand();
    float random = (float)(value % max);
    GammaDoseRateResult result;
    result.is_new = true;
    result.dose_rate = random;
    return result;
}


    
