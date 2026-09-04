#include <errno.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
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

static float calculate_deviation_percent(
    int measured_voltage,
    int reference_voltage)
{
    return fabsf(
        ((float)measured_voltage - (float)reference_voltage) /
        (float)reference_voltage) * 100.0f;
}

typedef struct
{
    const StartingVoltageTestParams *params;
    size_t tube_index;
    StartingVoltageTestResult *result;
} StartingVoltageWorkerParams;

static void *test_starting_voltage_worker(void *args)
{
    StartingVoltageWorkerParams *worker = args;
    const StartingVoltageTestParams *params = worker->params;
    StartingVoltageTestResult *result = worker->result;

    for (int step = 0; step < params->voltage_steps; step++)
    {
        int voltage = params->start_voltage +
            step * params->hv_increment_per_step;

        if (set_hv(worker->tube_index, true, voltage) !=
            MERCURY_READER_OK)
        {
            result->status = STARTING_VOLTAGE_SET_HV_ERROR;
            break;
        }

        sleep_milliseconds(params->voltage_settling_time_ms);

        unsigned int step_events = 0;
        struct timespec next_sample;
        clock_gettime(CLOCK_MONOTONIC, &next_sample);

        for (int second = 0;
             second < params->step_acquisition_time_sec;
             second++)
        {
            add_one_second(&next_sample);

            while (clock_nanosleep(
                       CLOCK_MONOTONIC,
                       TIMER_ABSTIME,
                       &next_sample,
                       NULL) == EINTR)
            {
            }

            GammaCountsResult measurement =
                get_instant_measurement_gm(worker->tube_index);

            if (measurement.error != MERCURY_READER_OK)
            {
                result->status =
                    STARTING_VOLTAGE_MEASUREMENT_ERROR;
                break;
            }

            if (measurement.is_new)
            {
                if (UINT_MAX - step_events < measurement.events)
                {
                    step_events = UINT_MAX;
                }
                else
                {
                    step_events += measurement.events;
                }
            }
        }

        if (result->status == STARTING_VOLTAGE_MEASUREMENT_ERROR)
        {
            break;
        }

        if (step_events >= (unsigned int)params->min_detected_events)
        {
            result->status = STARTING_VOLTAGE_FOUND;
            result->starting_voltage = voltage;
            result->detection_step = (size_t)step;
            result->detected_events = step_events;
            result->detected_on_first_step = (step == 0);
            result->deviation_percent =
                calculate_deviation_percent(
                    voltage,
                    params->reference_voltage);
            result->test_passed =
                result->deviation_percent <
                (float)params->max_deviation_percent;
            break;
        }
    }

    if (result->status == STARTING_VOLTAGE_PENDING)
    {
        result->status = STARTING_VOLTAGE_NOT_FOUND;
    }

    set_hv(worker->tube_index, false, 0);
    return NULL;
}

static bool disable_tested_tubes(
    const size_t *tube_indices,
    size_t tube_count)
{
    bool shutdown_ok = true;

    for (size_t i = 0; i < tube_count; i++)
    {
        size_t tube_index = tube_indices == NULL ? i : tube_indices[i];
        if (set_hv(tube_index, false, 0) != MERCURY_READER_OK)
        {
            shutdown_ok = false;
        }
    }

    return shutdown_ok;
}

static StartingVoltageTestResults test_starting_voltage_indices(
    const StartingVoltageTestParams *params,
    const size_t *tube_indices,
    size_t tube_count)
{
    StartingVoltageTestResults results = {
        .error = STARTING_VOLTAGE_TEST_OK,
        .safe_shutdown_ok = false,
        .count = 0,
        .test_results = NULL};

    if (!parameters_are_valid(params, tube_count, &results.error))
    {
        return results;
    }

    results.test_results = calloc(
        tube_count,
        sizeof(StartingVoltageTestResult));

    pthread_t *workers = calloc(tube_count, sizeof(pthread_t));
    StartingVoltageWorkerParams *worker_params = calloc(
        tube_count,
        sizeof(StartingVoltageWorkerParams));
    bool *worker_started = calloc(tube_count, sizeof(bool));

    if (results.test_results == NULL || workers == NULL ||
        worker_params == NULL || worker_started == NULL)
    {
        free(results.test_results);
        free(workers);
        free(worker_params);
        free(worker_started);
        results.test_results = NULL;
        results.error = STARTING_VOLTAGE_TEST_MALLOC_ERROR;
        return results;
    }

    results.count = tube_count;

    for (size_t i = 0; i < tube_count; i++)
    {
        size_t tube_index = tube_indices == NULL ? i : tube_indices[i];
        results.test_results[i].tube_index = tube_index;
        results.test_results[i].status = STARTING_VOLTAGE_PENDING;

        worker_params[i] = (StartingVoltageWorkerParams){
            .params = params,
            .tube_index = tube_index,
            .result = &results.test_results[i]};

        if (pthread_create(
                &workers[i],
                NULL,
                test_starting_voltage_worker,
                &worker_params[i]) == 0)
        {
            worker_started[i] = true;
        }
        else
        {
            results.test_results[i].status =
                STARTING_VOLTAGE_THREAD_ERROR;
        }
    }

    for (size_t i = 0; i < tube_count; i++)
    {
        if (worker_started[i])
        {
            pthread_join(workers[i], NULL);
        }
    }

    results.safe_shutdown_ok = disable_tested_tubes(
        tube_indices,
        tube_count);
    free(workers);
    free(worker_params);
    free(worker_started);

    return results;
}

StartingVoltageTestResults test_starting_voltage_tubes(
    const StartingVoltageTestParams *params,
    size_t total_tubes)
{
    return test_starting_voltage_indices(params, NULL, total_tubes);
}

StartingVoltageTestResults test_starting_voltage_tube(
    const StartingVoltageTestParams *params,
    size_t tube_index)
{
    return test_starting_voltage_indices(params, &tube_index, 1);
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
        case STARTING_VOLTAGE_THREAD_ERROR:
            return "thread_error";
        case STARTING_VOLTAGE_SET_HV_ERROR:
            return "set_hv_error";
        case STARTING_VOLTAGE_MEASUREMENT_ERROR:
            return "measurement_error";
    }

    return "unknown";
}
