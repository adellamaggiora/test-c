#ifndef WORKER_H
#define WORKER_H

// include va dopo la header guard he deve racchiudere tuttom il contenuto del file
// perchè così si evita che le dichiarazioni dell'header vengano processate più volte
#include <stdatomic.h>
#include <stdbool.h>

typedef struct GammaDoseRateWorkerParams
{
    float avg;
    atomic_bool *acquisition_running;
} GammaDoseRateWorkerParams;


typedef struct GammaDoseRateTestResult {
    float avg;
    bool thread_started;
    bool ref_tube;
    bool test_passed;
} GammaDoseRateTestResult;

typedef struct GammaDoseRateTestResults {
    size_t count;
    GammaDoseRateTestResult *test_results;
} GammaDoseRateTestResults;

GammaDoseRateTestResults test_gamma_tubes(size_t acquisition_time_sec, size_t total_tubes, size_t ref_tube_index);

#endif