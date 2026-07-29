#include <time.h>
#include <stdio.h>
#include "utils.h"

void timer_ms(long ms)
{
    struct timespec t = {
        .tv_sec = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000L
    };

    nanosleep(&t, NULL);
}