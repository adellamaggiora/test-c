#ifndef STARTING_VOLTAGE_H
#define STARTING_VOLTAGE_H

#include <stdbool.h>
#include <stddef.h>
#include "config.h"

typedef enum
{
    STARTING_VOLTAGE_PENDING = 0,
    STARTING_VOLTAGE_FOUND,
    STARTING_VOLTAGE_NOT_FOUND,
    STARTING_VOLTAGE_THREAD_ERROR,
    STARTING_VOLTAGE_SET_HV_ERROR,
    STARTING_VOLTAGE_MEASUREMENT_ERROR
} StartingVoltageStatus;

typedef enum
{
    STARTING_VOLTAGE_TEST_OK = 0,
    STARTING_VOLTAGE_TEST_INVALID_ARGUMENT,
    STARTING_VOLTAGE_TEST_UNSAFE_VOLTAGE_RANGE,
    STARTING_VOLTAGE_TEST_MALLOC_ERROR
} StartingVoltageTestError;

typedef struct
{
    size_t tube_index;
    StartingVoltageStatus status;
    int starting_voltage;
    size_t detection_step;
    unsigned int detected_events;
    float deviation_percent;
    bool detected_on_first_step;
    bool test_passed;
} StartingVoltageTestResult;

typedef struct
{
    StartingVoltageTestError error;
    bool safe_shutdown_ok;
    size_t count;
    StartingVoltageTestResult *test_results;
} StartingVoltageTestResults;

StartingVoltageTestResults test_starting_voltage_tubes(
    const StartingVoltageTestParams *params,
    size_t total_tubes);

StartingVoltageTestResults test_starting_voltage_tube(
    const StartingVoltageTestParams *params,
    size_t tube_index);

void free_starting_voltage_test_results(
    StartingVoltageTestResults *results);

const char *starting_voltage_status_string(StartingVoltageStatus status);

#endif
