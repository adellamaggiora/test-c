#include <stdatomic.h>
#include <stdbool.h>
#include <time.h>
#include "mercury_reader.h"
#include "worker.h"

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

    if(count > 0) {
        params->avg = (float)(total / count);
    }
    
    
    return NULL;
}