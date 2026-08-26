#include <assert.h>
#include <stdio.h>
#include "mercury_reader.h"
#include "tube_test/starting_voltage.h"

static StartingVoltageTestParams valid_params(void)
{
    StartingVoltageTestParams params = {
        .voltage_steps = 5,
        .min_detected_events = 3,
        .step_acquisition_time_sec = 1,
        .start_voltage = 300,
        .min_voltage = 280,
        .max_voltage = 320,
        .hv_increment_per_step = 10,
        .reference_voltage = 330,
        .max_deviation_percent = 5,
        .voltage_settling_time_ms = 0,
        .absolute_max_voltage = 400};

    return params;
}

static void test_detects_first_passing_step(void)
{
    const size_t total_tubes = 3;
    StartingVoltageTestParams params = valid_params();

    StartingVoltageTestResults results =
        test_starting_voltage_tubes(&params, total_tubes);

    assert(results.error == STARTING_VOLTAGE_TEST_OK);
    assert(results.count == total_tubes);
    assert(results.safe_shutdown_ok);

    for (size_t i = 0; i < total_tubes; i++)
    {
        assert(results.test_results[i].status ==
               STARTING_VOLTAGE_FOUND);
        assert(results.test_results[i].starting_voltage ==
               320 + (int)i * 10);
        assert(results.test_results[i].test_passed);
    }

    free_starting_voltage_test_results(&results);
}

static void test_reports_not_found(void)
{
    StartingVoltageTestParams params = valid_params();
    params.voltage_steps = 1;

    StartingVoltageTestResults results =
        test_starting_voltage_tubes(&params, 1);

    assert(results.error == STARTING_VOLTAGE_TEST_OK);
    assert(results.safe_shutdown_ok);
    assert(results.test_results[0].status ==
           STARTING_VOLTAGE_NOT_FOUND);
    assert(!results.test_results[0].test_passed);

    free_starting_voltage_test_results(&results);
}

static void test_rejects_unsafe_voltage_range(void)
{
    StartingVoltageTestParams params = valid_params();
    params.voltage_steps = 20;

    StartingVoltageTestResults results =
        test_starting_voltage_tubes(&params, 1);

    assert(results.error ==
           STARTING_VOLTAGE_TEST_UNSAFE_VOLTAGE_RANGE);
    assert(results.count == 0);
    assert(results.test_results == NULL);
}

static void test_marks_detection_on_first_step(void)
{
    StartingVoltageTestParams params = valid_params();
    params.start_voltage = 320;
    params.reference_voltage = 320;

    StartingVoltageTestResults results =
        test_starting_voltage_tubes(&params, 1);

    assert(results.error == STARTING_VOLTAGE_TEST_OK);
    assert(results.test_results[0].status ==
           STARTING_VOLTAGE_FOUND);
    assert(results.test_results[0].detected_on_first_step);
    assert(results.test_results[0].starting_voltage == 320);

    free_starting_voltage_test_results(&results);
}

int main(void)
{
    assert(mercury_reader_init(3) == MERCURY_READER_OK);

    test_detects_first_passing_step();
    test_reports_not_found();
    test_rejects_unsafe_voltage_range();
    test_marks_detection_on_first_step();

    mercury_reader_close();

    puts("starting voltage test: ok");
    return 0;
}
