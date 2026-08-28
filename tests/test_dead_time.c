#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "tube_test/dead_time.h"

static void test_evaluates_all_tubes(void)
{
    DeadTimeTestParams params = {
        .reference_dead_time_us = 200.0,
        .max_deviation_percent = 5.0};

    const double measured_values[] = {
        200.0,
        195.0,
        210.0,
        -1.0,
        NAN};

    DeadTimeTestResults results = test_dead_times(
        &params,
        measured_values,
        sizeof(measured_values) / sizeof(measured_values[0]));

    assert(results.error == DEAD_TIME_TEST_OK);
    assert(results.count == 5);

    assert(results.test_results[0].status == DEAD_TIME_VALUE_VALID);
    assert(results.test_results[0].test_passed);
    assert(results.test_results[0].deviation_percent == 0.0);

    assert(results.test_results[1].status == DEAD_TIME_VALUE_VALID);
    assert(results.test_results[1].test_passed);
    assert(fabs(results.test_results[1].deviation_percent - 2.5) < 0.0001);

    /* The specification uses a strict '<': exactly 5% does not pass. */
    assert(results.test_results[2].status == DEAD_TIME_VALUE_VALID);
    assert(!results.test_results[2].test_passed);
    assert(fabs(results.test_results[2].deviation_percent - 5.0) < 0.0001);

    assert(results.test_results[3].status ==
           DEAD_TIME_VALUE_NOT_POSITIVE);
    assert(!results.test_results[3].test_passed);

    assert(results.test_results[4].status ==
           DEAD_TIME_VALUE_NOT_FINITE);
    assert(!results.test_results[4].test_passed);

    free_dead_time_test_results(&results);
}

static void test_rejects_invalid_configuration(void)
{
    const double measured_value = 200.0;
    DeadTimeTestParams params = {
        .reference_dead_time_us = 0.0,
        .max_deviation_percent = 5.0};

    DeadTimeTestResults results =
        test_dead_times(&params, &measured_value, 1);

    assert(results.error == DEAD_TIME_TEST_INVALID_ARGUMENT);
    assert(results.count == 0);
    assert(results.test_results == NULL);
}

int main(void)
{
    test_evaluates_all_tubes();
    test_rejects_invalid_configuration();

    puts("dead time test: ok");
    return 0;
}
