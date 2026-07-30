#include <stdatomic.h>
#include <stdbool.h>
#include <time.h>
#include "mercury_reader.h"


float get_gamma_dose_rate_avg(atomic_bool *acquisition_running)
{
    double total = 0.0;
    unsigned int count = 0;

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (atomic_load(acquisition_running))
    {
        float value = get_gamma_dose_rate();

        total += value;
        count++;

        next.tv_sec += 1;
        clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,
            &next,
            NULL
        );
    }

    return count > 0 ? (float)(total / count) : 0.0f;
}



/*

    do_measurement

    time1 = now();

    start_measurement();

    time2 = now();

    diff = time2 - time1();

    remaining = 1000 - diff

    sleep(remaining)


\*/