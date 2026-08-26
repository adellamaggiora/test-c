#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "mercury_reader.h"
#include "tube_test/starting_voltage.h"

static bool parameters_are_valid(
    const StartingVoltageTestParams *params,
    size_t total_tubes,
    StartingVoltageTestError *error)
{
    if (params == NULL || total_tubes == 0 ||
        params->voltage_steps <= 0 ||
        params->min_detected_events <= 0 ||
        params->step_acquisition_time_sec <= 0 ||
        params->hv_increment_per_step <= 0 ||
        params->reference_voltage <= 0 ||
        params->max_deviation_percent < 0 ||
        params->voltage_settling_time_ms < 0 ||
        params->absolute_max_voltage <= 0 ||
        params->min_voltage > params->max_voltage ||
        params->start_voltage < params->min_voltage ||
        params->start_voltage > params->max_voltage)
    {
        *error = STARTING_VOLTAGE_TEST_INVALID_ARGUMENT;
        return false;
    }

    long long last_voltage =
        (long long)params->start_voltage +
        (long long)(params->voltage_steps - 1) *
            params->hv_increment_per_step;

    if (last_voltage > params->absolute_max_voltage ||
        last_voltage > INT_MAX)
    {
        *error = STARTING_VOLTAGE_TEST_UNSAFE_VOLTAGE_RANGE;
        return false;
    }

    return true;
}

static void sleep_milliseconds(int milliseconds)
{
    struct timespec duration = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (long)(milliseconds % 1000) * 1000000L};

    while (nanosleep(&duration, &duration) != 0 && errno == EINTR)
    {
    }
}

static void add_one_second(struct timespec *time)
{
    time->tv_sec++;
}

static bool tube_is_pending(const StartingVoltageTestResult *result)
{
    return result->status == STARTING_VOLTAGE_PENDING;
}

static bool any_tube_is_pending(
    const StartingVoltageTestResults *results)
{
    for (size_t i = 0; i < results->count; i++)
    {
        if (tube_is_pending(&results->test_results[i]))
        {
            return true;
        }
    }

    return false;
}

static float calculate_deviation_percent(
    int measured_voltage,
    int reference_voltage)
{
    return fabsf(
        ((float)measured_voltage - (float)reference_voltage) /
        (float)reference_voltage) * 100.0f;
}

static bool disable_all_tubes(size_t total_tubes)
{
    bool shutdown_ok = true;

    for (size_t i = 0; i < total_tubes; i++)
    {
        if (set_hv(i, false, 0) != MERCURY_READER_OK)
        {
            shutdown_ok = false;
        }
    }

    return shutdown_ok;
}

StartingVoltageTestResults test_starting_voltage_tubes(
    const StartingVoltageTestParams *params,
    size_t total_tubes)
{
    StartingVoltageTestResults results = {
        .error = STARTING_VOLTAGE_TEST_OK,
        .safe_shutdown_ok = false,
        .count = 0,
        .test_results = NULL};

    if (!parameters_are_valid(params, total_tubes, &results.error))
    {
        return results;
    }

    results.test_results = calloc(
        total_tubes,
        sizeof(StartingVoltageTestResult));

    unsigned int *step_events = calloc(
        total_tubes,
        sizeof(unsigned int));

    if (results.test_results == NULL || step_events == NULL)
    {
        free(results.test_results);
        free(step_events);
        results.test_results = NULL;
        results.error = STARTING_VOLTAGE_TEST_MALLOC_ERROR;
        return results;
    }

    results.count = total_tubes;

    for (size_t i = 0; i < total_tubes; i++)
    {
        results.test_results[i].tube_index = i;
        results.test_results[i].status = STARTING_VOLTAGE_PENDING;
    }

    for (int step = 0;
         step < params->voltage_steps && any_tube_is_pending(&results);
         step++)
    {
        int voltage = params->start_voltage +
            step * params->hv_increment_per_step;

        for (size_t i = 0; i < total_tubes; i++)
        {
            step_events[i] = 0;

            if (tube_is_pending(&results.test_results[i]) &&
                set_hv(i, true, voltage) != MERCURY_READER_OK)
            {
                results.test_results[i].status =
                    STARTING_VOLTAGE_SET_HV_ERROR;
            }
        }

        sleep_milliseconds(params->voltage_settling_time_ms);

        struct timespec next_sample;
        clock_gettime(CLOCK_MONOTONIC, &next_sample);

        for (int second = 0;
             second < params->step_acquisition_time_sec;
             second++)
        {
            add_one_second(&next_sample);
            clock_nanosleep(
                CLOCK_MONOTONIC,
                TIMER_ABSTIME,
                &next_sample,
                NULL);

            for (size_t i = 0; i < total_tubes; i++)
            {
                if (!tube_is_pending(&results.test_results[i]))
                {
                    continue;
                }

                GammaCountsResult measurement =
                    get_instant_measurement_gm(i);

                if (measurement.error != MERCURY_READER_OK)
                {
                    results.test_results[i].status =
                        STARTING_VOLTAGE_MEASUREMENT_ERROR;
                    continue;
                }

                if (measurement.is_new)
                {
                    if (UINT_MAX - step_events[i] < measurement.events)
                    {
                        step_events[i] = UINT_MAX;
                    }
                    else
                    {
                        step_events[i] += measurement.events;
                    }
                }
            }
        }

        for (size_t i = 0; i < total_tubes; i++)
        {
            StartingVoltageTestResult *tube_result =
                &results.test_results[i];

            if (!tube_is_pending(tube_result) ||
                step_events[i] <
                    (unsigned int)params->min_detected_events)
            {
                continue;
            }

            tube_result->status = STARTING_VOLTAGE_FOUND;
            tube_result->starting_voltage = voltage;
            tube_result->detection_step = (size_t)step;
            tube_result->detected_events = step_events[i];
            tube_result->detected_on_first_step = (step == 0);
            tube_result->deviation_percent =
                calculate_deviation_percent(
                    voltage,
                    params->reference_voltage);
            tube_result->test_passed =
                tube_result->deviation_percent <
                (float)params->max_deviation_percent;
        }
    }

    for (size_t i = 0; i < total_tubes; i++)
    {
        if (tube_is_pending(&results.test_results[i]))
        {
            results.test_results[i].status =
                STARTING_VOLTAGE_NOT_FOUND;
        }
    }

    results.safe_shutdown_ok = disable_all_tubes(total_tubes);
    free(step_events);

    return results;
}

void free_starting_voltage_test_results(
    StartingVoltageTestResults *results)
{
    if (results == NULL)
    {
        return;
    }

    free(results->test_results);
    *results = (StartingVoltageTestResults){0};
}

const char *starting_voltage_status_string(StartingVoltageStatus status)
{
    switch (status)
    {
        case STARTING_VOLTAGE_PENDING:
            return "pending";
        case STARTING_VOLTAGE_FOUND:
            return "found";
        case STARTING_VOLTAGE_NOT_FOUND:
            return "not_found";
        case STARTING_VOLTAGE_SET_HV_ERROR:
            return "set_hv_error";
        case STARTING_VOLTAGE_MEASUREMENT_ERROR:
            return "measurement_error";
    }

    return "unknown";
}
