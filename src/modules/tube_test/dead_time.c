#include <math.h>
#include <stdlib.h>
#include "tube_test/dead_time.h"

static bool parameters_are_valid(
    const DeadTimeTestParams *params,
    const double *measured_dead_times_us,
    size_t total_tubes)
{
    return params != NULL &&
        measured_dead_times_us != NULL &&
        total_tubes > 0 &&
        isfinite(params->reference_dead_time_us) &&
        params->reference_dead_time_us > 0.0 &&
        isfinite(params->max_deviation_percent) &&
        params->max_deviation_percent > 0.0;
}

static double calculate_deviation_percent(
    double measured_dead_time_us,
    double reference_dead_time_us)
{
    return fabs(
        (measured_dead_time_us - reference_dead_time_us) /
        reference_dead_time_us) * 100.0;
}

DeadTimeTestResults test_dead_times(
    const DeadTimeTestParams *params,
    const double *measured_dead_times_us,
    size_t total_tubes)
{
    DeadTimeTestResults results = {
        .error = DEAD_TIME_TEST_OK,
        .count = 0,
        .test_results = NULL};

    if (!parameters_are_valid(
            params,
            measured_dead_times_us,
            total_tubes))
    {
        results.error = DEAD_TIME_TEST_INVALID_ARGUMENT;
        return results;
    }

    results.test_results = calloc(
        total_tubes,
        sizeof(DeadTimeTestResult));

    if (results.test_results == NULL)
    {
        results.error = DEAD_TIME_TEST_MALLOC_ERROR;
        return results;
    }

    results.count = total_tubes;

    for (size_t i = 0; i < total_tubes; i++)
    {
        DeadTimeTestResult *tube_result = &results.test_results[i];
        double measured_value = measured_dead_times_us[i];

        tube_result->tube_index = i;
        tube_result->measured_dead_time_us = measured_value;
        tube_result->reference_dead_time_us =
            params->reference_dead_time_us;

        if (!isfinite(measured_value))
        {
            tube_result->status = DEAD_TIME_VALUE_NOT_FINITE;
            continue;
        }

        if (measured_value <= 0.0)
        {
            tube_result->status = DEAD_TIME_VALUE_NOT_POSITIVE;
            continue;
        }

        tube_result->status = DEAD_TIME_VALUE_VALID;
        tube_result->deviation_percent =
            calculate_deviation_percent(
                measured_value,
                params->reference_dead_time_us);
        tube_result->test_passed =
            tube_result->deviation_percent <
            params->max_deviation_percent;
    }

    return results;
}

void free_dead_time_test_results(DeadTimeTestResults *results)
{
    if (results == NULL)
    {
        return;
    }

    free(results->test_results);
    *results = (DeadTimeTestResults){0};
}

const char *dead_time_status_string(DeadTimeStatus status)
{
    switch (status)
    {
        case DEAD_TIME_VALUE_VALID:
            return "valid";
        case DEAD_TIME_VALUE_NOT_FINITE:
            return "not_finite";
        case DEAD_TIME_VALUE_NOT_POSITIVE:
            return "not_positive";
    }

    return "unknown";
}
