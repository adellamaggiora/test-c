#include <stdatomic.h>
#include <stdbool.h>
#include <time.h>


int get_gamma_dose_rate_avg(atomic_bool *acquisition_running)
{

    float total = 0;
    unsigned int count = 0;

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    // now = getTime()

    while (acquisition_running == true)
    {
        /* code */
    }
}