#include <math.h>
#include <pthread.h>
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

typedef struct
{
    const DeadTimeTestParams *params;
    double measured_value;
    DeadTimeTestResult *result;
} DeadTimeWorkerParams;

static void *test_dead_time_worker(void *args)
{
    DeadTimeWorkerParams *worker = args;
    DeadTimeTestResult *result = worker->result;
    double measured_value = worker->measured_value;

    result->measured_dead_time_us = measured_value;
    result->reference_dead_time_us =
        worker->params->reference_dead_time_us;

    if (!isfinite(measured_value))
    {
        result->status = DEAD_TIME_VALUE_NOT_FINITE;
        return NULL;
    }

    if (measured_value <= 0.0)
    {
        result->status = DEAD_TIME_VALUE_NOT_POSITIVE;
        return NULL;
    }

    result->status = DEAD_TIME_VALUE_VALID;
    result->deviation_percent = calculate_deviation_percent(
        measured_value,
        worker->params->reference_dead_time_us);
    result->test_passed =
        result->deviation_percent <
        worker->params->max_deviation_percent;

    return NULL;
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

    pthread_t *workers = calloc(total_tubes, sizeof(pthread_t));
    DeadTimeWorkerParams *worker_params = calloc(
        total_tubes,
        sizeof(DeadTimeWorkerParams));
    bool *worker_started = calloc(total_tubes, sizeof(bool));

    if (results.test_results == NULL || workers == NULL ||
        worker_params == NULL || worker_started == NULL)
    {
        free(results.test_results);
        free(workers);
        free(worker_params);
        free(worker_started);
        results.test_results = NULL;
        results.error = DEAD_TIME_TEST_MALLOC_ERROR;
        return results;
    }

    results.count = total_tubes;

    for (size_t i = 0; i < total_tubes; i++)
    {
        results.test_results[i].tube_index = i;
        worker_params[i] = (DeadTimeWorkerParams){
            .params = params,
            .measured_value = measured_dead_times_us[i],
            .result = &results.test_results[i]};

        if (pthread_create(
                &workers[i],
                NULL,
                test_dead_time_worker,
                &worker_params[i]) == 0)
        {
            worker_started[i] = true;
        }
        else
        {
            results.test_results[i].status = DEAD_TIME_THREAD_ERROR;
        }
    }

    for (size_t i = 0; i < total_tubes; i++)
    {
        if (worker_started[i])
        {
            pthread_join(workers[i], NULL);
        }
    }

    free(workers);
    free(worker_params);
    free(worker_started);

    return results;
}

DeadTimeTestResults test_dead_time(
    const DeadTimeTestParams *params,
    double measured_dead_time_us,
    size_t tube_index)
{
    DeadTimeTestResults results = test_dead_times(
        params,
        &measured_dead_time_us,
        1);

    if (results.test_results != NULL && results.count == 1)
    {
        results.test_results[0].tube_index = tube_index;
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
        case DEAD_TIME_THREAD_ERROR:
            return "thread_error";
    }

    return "unknown";
}
