#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "mercury_reader.h"
#include "tube_test/gamma.h"
#include "tube_test/starting_voltage.h"

static void print_starting_voltage_results(
    const char *tube_type,
    const StartingVoltageTestResults *results)
{
    printf("Starting voltage test (%s tubes)\n", tube_type);

    if (results->error != STARTING_VOLTAGE_TEST_OK)
    {
        printf("  test error: %d\n", results->error);
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const StartingVoltageTestResult *tube =
            &results->test_results[i];

        printf("  tube %zu: status=%s",
               tube->tube_index,
               starting_voltage_status_string(tube->status));

        if (tube->status == STARTING_VOLTAGE_FOUND)
        {
            printf(", voltage=%d V, events=%u, deviation=%.2f%%, passed=%s",
                   tube->starting_voltage,
                   tube->detected_events,
                   tube->deviation_percent,
                   tube->test_passed ? "yes" : "no");
        }

        printf("\n");
    }

    printf("  safe shutdown: %s\n",
           results->safe_shutdown_ok ? "yes" : "no");
}


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

    GammaDoseRateTestResults gamma_dose_rate_test_results = 
        test_gamma_tubes(config.small_tube.gamma_test_params.acquisition_time_sec, 6, 1);

    free(gamma_dose_rate_test_results.test_results);

    const size_t total_tubes = 6;

    if (mercury_reader_init(total_tubes) != MERCURY_READER_OK)
    {
        fprintf(stderr, "Error: Mercury reader initialization failed\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    StartingVoltageTestResults large_tube_results =
        test_starting_voltage_tubes(
            &config.large_tube.starting_voltage_test_params,
            total_tubes);

    print_starting_voltage_results("large", &large_tube_results);
    free_starting_voltage_test_results(&large_tube_results);

    StartingVoltageTestResults small_tube_results =
        test_starting_voltage_tubes(
            &config.small_tube.starting_voltage_test_params,
            total_tubes);

    print_starting_voltage_results("small", &small_tube_results);
    free_starting_voltage_test_results(&small_tube_results);

    mercury_reader_close();

    free_config(&config);

    return 0;
}
