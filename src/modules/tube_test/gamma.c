#include <stdatomic.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mercury_reader.h"
#include "tube_test/gamma.h"

void *get_gamma_dose_rate_avg(void *args)
{

    GammaDoseRateWorkerParams *params = args;

    float total = 0.0;
    unsigned int count = 0;

    struct timespec next;
    // legge il valore attuale di CLOCK_MONOTONIC  e lo scrive in next
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (atomic_load(params->is_running))
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

int test_gamma_tubes(unsigned int acquisition_time_sec, size_t total_threads)
{

    atomic_bool acquisition_running;
    atomic_init(&acquisition_running, true);

    if (total_threads == 0)
    {
        return EXIT_FAILURE;
    }
    

    pthread_t *workers = malloc(total_threads * sizeof(pthread_t));
    GammaDoseRateWorkerParams *workers_params = malloc(total_threads * sizeof(GammaDoseRateWorkerParams));
    size_t total_workers = 0;

    if (workers == NULL || workers_params == NULL)
    {
        free(workers);
        free(workers_params);
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < total_threads; i++)
    {
        workers_params[total_workers].avg = 0.0f;
        workers_params[total_workers].is_running = &acquisition_running;

        int thread_created = pthread_create(
            &workers[total_workers],
            NULL,
            get_gamma_dose_rate_avg,
            &workers_params[total_workers]);

        if (thread_created == 0)
        {
            total_workers++;
        }
    }

    sleep(acquisition_time_sec);
    atomic_store(&acquisition_running, false);

    // attende che termini il thread
    for (size_t i = 0; i < total_workers; i++)
    {
        pthread_join(workers[i], NULL);
    }

    free(workers);
    free(workers_params);

    return EXIT_SUCCESS;
}