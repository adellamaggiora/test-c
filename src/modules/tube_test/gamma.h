#ifndef WORKER_H
#define WORKER_H

// include va dopo la header guard he deve racchiudere tuttom il contenuto del file
// perchè così si evita che le dichiarazioni dell'header vengano processate più volte
#include <stdatomic.h>

typedef struct GammaDoseRateWorkerParams {
    float avg;
    // puntatore perchè condivisa con il main
    atomic_bool *is_running;
} GammaDoseRateWorkerParams;


void *get_gamma_dose_rate_avg(void *args);

void test_gamma_tubes(int acquisition_time_sec, size_t total_threads);


#endif