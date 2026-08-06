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

static bool gamma_test_ok(float gamma_ref, float gamma_out, size_t max_deviation_percent)
{
    if (gamma_out == 0)
    {
        return false;
    }

    float deviation_percent = fabsf((gamma_out - gamma_ref) / gamma_out) * 100.0f;

    return deviation_percent < (float)max_deviation_percent;
}

GammaDoseRateTestResults test_gamma_tubes(size_t acquisition_time_sec, size_t total_tubes, size_t ref_tube_index)
{

    GammaDoseRateTestResults result = {
        .count = 0,
        .test_results = NULL};

    if (total_tubes == 0 || ref_tube_index >= total_tubes)
    {
        return result;
    }

    pthread_t *workers = malloc(total_tubes * sizeof(pthread_t));
    GammaDoseRateWorkerParams *workers_params = malloc(total_tubes * sizeof(GammaDoseRateWorkerParams));
    result.test_results = malloc(total_tubes * sizeof(GammaDoseRateTestResult));

    if (workers == NULL || workers_params == NULL || result.test_results == NULL)
    {
        free(workers);
        free(workers_params);
        free(result.test_results);
        result.test_results = NULL;

        perror("malloc");
        return result;
    }

    result.count = total_tubes;
    // start acquisition
    atomic_bool acquisition_running;
    atomic_init(&acquisition_running, true);

    for (size_t i = 0; i < total_tubes; i++)
    {
        // worker params
        workers_params[i].avg = 0.0f;
        workers_params[i].acquisition_running = &acquisition_running;
        // test results
        result.test_results[i].avg = 0.0f;
        result.test_results[i].ref_tube = (bool)(i == ref_tube_index);
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
            fprintf(stderr,
                    "pthread_create tube %zu: %s\n",
                    i,
                    strerror(error));
        }
    }

    sleep(acquisition_time_sec);
    atomic_store(&acquisition_running, false);

    // terminazione dei thread: avg viene valorizzata
    for (size_t i = 0; i < total_tubes; i++)
    {
        if (result.test_results[i].thread_started == true)
        {
            pthread_join(workers[i], NULL);
            result.test_results[i].avg = workers_params[i].avg;
        }
    }

    // controllo media del tubo di riferimento @todo

    free(workers);
    free(workers_params);

    return result;
}
