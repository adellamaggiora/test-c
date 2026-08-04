#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "config.h"
#include "mercury_reader.h"
#include "worker.h"

static atomic_bool acquisition_running;

static pthread_t worker_1;
static pthread_t worker_2;
static pthread_t worker_3;

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Error: expected exactly one argument: <config_file>\n");
        return EXIT_FAILURE;
    }

    char *file_path = argv[1];
    Config config;

    if (read_config(file_path, &config) != 0)
    {
        return EXIT_FAILURE;
    }

    // acquisition gamma dose rate
    atomic_init(&acquisition_running, true);

    GammaDoseRateWorkerParams w1_params = {
        .avg = 0.0f,
        .is_running = &acquisition_running};

    GammaDoseRateWorkerParams w2_params = {
        .avg = 0.0f,
        .is_running = &acquisition_running};

    GammaDoseRateWorkerParams w3_params = {
        .avg = 0.0f,
        .is_running = &acquisition_running};

    pthread_create(&worker_1, NULL, get_gamma_dose_rate_avg, &w1_params);
    pthread_create(&worker_2, NULL, get_gamma_dose_rate_avg, &w2_params);
    pthread_create(&worker_3, NULL, get_gamma_dose_rate_avg, &w3_params);

    sleep(config.small_tube.gamma_test_params.acquisition_time_sec);
    atomic_store(&acquisition_running, false);
    // attende che termini il thread
    pthread_join(worker_1, NULL);
    printf("avg: %f \n", w1_params.avg);
    printf("avg: %f \n", w2_params.avg);
    printf("avg: %f \n", w3_params.avg);

    free_config(&config);

    return 0;
}