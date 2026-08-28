#ifndef DEAD_TIME_H
#define DEAD_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include "config.h"

typedef enum
{
    DEAD_TIME_VALUE_VALID = 0,
    DEAD_TIME_VALUE_NOT_FINITE,
    DEAD_TIME_VALUE_NOT_POSITIVE,
    DEAD_TIME_THREAD_ERROR
} DeadTimeStatus;

typedef enum
{
    DEAD_TIME_TEST_OK = 0,
    DEAD_TIME_TEST_INVALID_ARGUMENT,
    DEAD_TIME_TEST_MALLOC_ERROR
} DeadTimeTestError;

typedef struct
{
    size_t tube_index;
    DeadTimeStatus status;
    double measured_dead_time_us;
    double reference_dead_time_us;
    double deviation_percent;
    bool test_passed;
} DeadTimeTestResult;

typedef struct
{
    DeadTimeTestError error;
    size_t count;
    DeadTimeTestResult *test_results;
} DeadTimeTestResults;

DeadTimeTestResults test_dead_times(
    const DeadTimeTestParams *params,
    const double *measured_dead_times_us,
    size_t total_tubes);

void free_dead_time_test_results(DeadTimeTestResults *results);

const char *dead_time_status_string(DeadTimeStatus status);

#endif
