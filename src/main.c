#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "mercury_reader.h"
#include "report.h"
#include "tube_test/dead_time.h"
#include "tube_test/gamma.h"
#include "tube_test/starting_voltage.h"

static void print_gamma_results(
    const char *tube_type,
    const GammaDoseRateTestResults *results)
{
    printf("Gamma test (%s tubes)\n", tube_type);

    if (results->test_results == NULL)
    {
        printf("  test unavailable\n");
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const GammaDoseRateTestResult *tube =
            &results->test_results[i];

        printf("  tube %zu: average=%.2f, reference=%s, "
               "thread_started=%s, passed=%s\n",
               i,
               tube->avg,
               tube->ref_tube ? "yes" : "no",
               tube->thread_started ? "yes" : "no",
               tube->test_passed ? "yes" : "no");
    }
}

static void print_dead_time_results(
    const char *tube_type,
    const DeadTimeTestResults *results)
{
    printf("Dead time test (%s tubes)\n", tube_type);

    if (results->error != DEAD_TIME_TEST_OK)
    {
        printf("  test error: %d\n", results->error);
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const DeadTimeTestResult *tube = &results->test_results[i];

        printf("  tube %zu: status=%s, measured=%.2f us, "
               "reference=%.2f us, deviation=%.2f%%, passed=%s\n",
               tube->tube_index,
               dead_time_status_string(tube->status),
               tube->measured_dead_time_us,
               tube->reference_dead_time_us,
               tube->deviation_percent,
               tube->test_passed ? "yes" : "no");
    }
}

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

    const char *file_path = argv[1];
    Config config;

    if (read_config(file_path, &config) != 0)
    {
        return EXIT_FAILURE;
    }

    const size_t total_tubes = 6;
    const size_t gamma_reference_tube_index = 1;
    const double measured_dead_times_us[] = {
        190.0,
        187.0,
        193.0,
        185.0,
        195.0,
        200.0};

    if (mercury_reader_init(total_tubes) != MERCURY_READER_OK)
    {
        fprintf(stderr, "Error: Mercury reader initialization failed\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    GammaDoseRateTestResults small_gamma_results =
        test_gamma_tubes(
            &config.small_tube.gamma_test_params,
            total_tubes,
            gamma_reference_tube_index);

    print_gamma_results("small", &small_gamma_results);

    GammaDoseRateTestResults large_gamma_results =
        test_gamma_tubes(
            &config.large_tube.gamma_test_params,
            total_tubes,
            gamma_reference_tube_index);

    print_gamma_results("large", &large_gamma_results);

    StartingVoltageTestResults large_tube_results =
        test_starting_voltage_tubes(
            &config.large_tube.starting_voltage_test_params,
            total_tubes);

    print_starting_voltage_results("large", &large_tube_results);

    StartingVoltageTestResults small_tube_results =
        test_starting_voltage_tubes(
            &config.small_tube.starting_voltage_test_params,
            total_tubes);

    print_starting_voltage_results("small", &small_tube_results);

    DeadTimeTestResults large_dead_time_results =
        test_dead_times(
            &config.large_tube.dead_time_test_params,
            measured_dead_times_us,
            total_tubes);

    print_dead_time_results("large", &large_dead_time_results);

    DeadTimeTestResults small_dead_time_results =
        test_dead_times(
            &config.small_tube.dead_time_test_params,
            measured_dead_times_us,
            total_tubes);

    print_dead_time_results("small", &small_dead_time_results);

    TestReportData report_data = {
        .config = &config,
        .small_gamma = &small_gamma_results,
        .large_gamma = &large_gamma_results,
        .small_starting_voltage = &small_tube_results,
        .large_starting_voltage = &large_tube_results,
        .small_dead_time = &small_dead_time_results,
        .large_dead_time = &large_dead_time_results};

    char report_path[PATH_MAX];
    int exit_status = EXIT_SUCCESS;

    if (write_test_report(
            &report_data,
            report_path,
            sizeof(report_path)) != 0)
    {
        fprintf(stderr, "Error: could not write the test report\n");
        exit_status = EXIT_FAILURE;
    }
    else
    {
        printf("Report written to: %s\n", report_path);
    }

    free(small_gamma_results.test_results);
    free(large_gamma_results.test_results);
    free_starting_voltage_test_results(&large_tube_results);
    free_starting_voltage_test_results(&small_tube_results);
    free_dead_time_test_results(&large_dead_time_results);
    free_dead_time_test_results(&small_dead_time_results);

    mercury_reader_close();

    free_config(&config);

    return exit_status;
}
