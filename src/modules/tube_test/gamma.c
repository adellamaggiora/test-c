#include <stdatomic.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mercury_reader.h"
#include "tube_test/gamma.h"

static void *get_tube_gamma_dose_rate_avg(void *args)
{

    GammaDoseRateWorkerParams *params = args;

    float total = 0.0;
    unsigned int count = 0;

    struct timespec next;
    // legge il valore attuale di CLOCK_MONOTONIC  e lo scrive in next
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (atomic_load(params->acquisition_running))
    {
        GammaDoseRateResult value = get_gamma_dose_rate();

        if (value.is_new == true)
        {
            total += value.dose_rate;
            count++;
        }

        next.tv_sec += 1;

        // Attende fino alla prossima scadenza assoluta,
        // impostata a un secondo dopo la precedente.
        clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,
            &next,
            NULL);
    }

    if (count > 0)
    {
        params->avg = (float)(total / count);
    }

    return NULL;
}

static bool gamma_test_ok(
    float gamma_ref,
    float gamma_out,
    int max_deviation_percent)
{
    if (gamma_ref <= 0.0f || gamma_out <= 0.0f)
    {
        return false;
    }

    float deviation_percent =
        fabsf((gamma_out - gamma_ref) / gamma_ref) * 100.0f;

    return deviation_percent < (float)max_deviation_percent;
}

static GammaDoseRateTestResults test_gamma_indices(
    const GammaTestParams *params,
    const size_t *tube_indices,
    size_t tube_count,
    size_t ref_tube_index)
{

    GammaDoseRateTestResults result = {
        .count = 0,
        .test_results = NULL};

    if (params == NULL || params->acquisition_time_sec <= 0 ||
        params->max_deviation_percent < 0 ||
        params->avg_dose_rate_ref <= 0 || tube_count == 0)
    {
        return result;
    }

    size_t ref_result_index = tube_count;
    for (size_t i = 0; i < tube_count; i++)
    {
        size_t tube_index = tube_indices == NULL ? i : tube_indices[i];
        if (tube_index == ref_tube_index)
        {
            ref_result_index = i;
            break;
        }
    }

    if (ref_result_index == tube_count)
    {
        return result;
    }

    pthread_t *workers = malloc(tube_count * sizeof(pthread_t));
    GammaDoseRateWorkerParams *workers_params = malloc(
        tube_count * sizeof(GammaDoseRateWorkerParams));
    result.test_results = malloc(
        tube_count * sizeof(GammaDoseRateTestResult));

    if (workers == NULL || workers_params == NULL || result.test_results == NULL)
    {
        free(workers);
        free(workers_params);
        free(result.test_results);
        result.test_results = NULL;

        perror("malloc");
        return result;
    }

    result.count = tube_count;
    // start acquisition
    atomic_bool acquisition_running;
    atomic_init(&acquisition_running, true);

    for (size_t i = 0; i < tube_count; i++)
    {
        size_t tube_index = tube_indices == NULL ? i : tube_indices[i];
        // worker params
        workers_params[i].avg = 0.0f;
        workers_params[i].acquisition_running = &acquisition_running;
        // test results
        result.test_results[i].tube_index = tube_index;
        result.test_results[i].avg = 0.0f;
        result.test_results[i].ref_tube =
            (bool)(tube_index == ref_tube_index);
        result.test_results[i].thread_started = false;
        result.test_results[i].test_passed = false;

        int error = pthread_create(
            &workers[i],
            NULL,
            get_tube_gamma_dose_rate_avg,
            &workers_params[i]);

        if (error == 0)
        {
            result.test_results[i].thread_started = true;
        }
        else
        {
            fprintf(
                stderr,
                "pthread_create tube %zu: %s\n",
                tube_index,
                strerror(error));
        }
    }

    sleep((unsigned int)params->acquisition_time_sec);
    atomic_store(&acquisition_running, false);

    // terminazione dei thread: avg viene valorizzata
    for (size_t i = 0; i < tube_count; i++)
    {
        if (result.test_results[i].thread_started == true)
        {
            pthread_join(workers[i], NULL);
            result.test_results[i].avg = workers_params[i].avg;
        }
    }

    // la media del tubo di riferimento
    float ref_avg = result.test_results[ref_result_index].avg;
    if (ref_avg == 0.0f)
    {
        fprintf(stderr, "tube ref average not calculated\n");
    }

    for (size_t i = 0; i < tube_count; i++)
    {
        if (!result.test_results[i].thread_started)
        {
            continue;
        }

        float reference = result.test_results[i].ref_tube
            ? (float)params->avg_dose_rate_ref
            : ref_avg;

        result.test_results[i].test_passed = gamma_test_ok(
            reference,
            result.test_results[i].avg,
            params->max_deviation_percent);
    }

    free(workers);
    free(workers_params);

    return result;
}

GammaDoseRateTestResults test_gamma_tubes(
    const GammaTestParams *params,
    size_t total_tubes,
    size_t ref_tube_index)
{
    if (ref_tube_index >= total_tubes)
    {
        return (GammaDoseRateTestResults){0};
    }

    return test_gamma_indices(params, NULL, total_tubes, ref_tube_index);
}

GammaDoseRateTestResults test_gamma_tube(
    const GammaTestParams *params,
    size_t tube_index,
    size_t ref_tube_index)
{
    size_t tube_indices[2] = {tube_index, ref_tube_index};
    size_t tube_count = tube_index == ref_tube_index ? 1 : 2;

    return test_gamma_indices(
        params,
        tube_indices,
        tube_count,
        ref_tube_index);
}
