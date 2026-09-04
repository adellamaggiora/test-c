#ifndef GAMMA_H
#define GAMMA_H

// include va dopo la header guard he deve racchiudere tuttom il contenuto del file
// perchè così si evita che le dichiarazioni dell'header vengano processate più volte
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"

typedef enum
{
    GAMMA_TEST_OK = 0,
    GAMMA_TEST_INVALID_ARGUMENT,
    GAMMA_TEST_MALLOC_ERROR,
    GAMMA_TEST_REF_AVG_ERROR
} GammaDoseRateTestError;


typedef struct GammaDoseRateWorkerParams
{
    float avg;
    atomic_bool *acquisition_running;
} GammaDoseRateWorkerParams;


typedef struct GammaDoseRateTestResult {
    size_t tube_index;
    float avg;
    bool thread_started;
    bool ref_tube;
    bool test_passed;
} GammaDoseRateTestResult;

typedef struct GammaDoseRateTestResults {
    size_t count;
    GammaDoseRateTestResult *test_results;
} GammaDoseRateTestResults;

GammaDoseRateTestResults test_gamma_tubes(
    const GammaTestParams *params,
    size_t total_tubes,
    size_t ref_tube_index);

GammaDoseRateTestResults test_gamma_tube(
    const GammaTestParams *params,
    size_t tube_index,
    size_t ref_tube_index);

#endif
