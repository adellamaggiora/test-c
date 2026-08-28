#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include "config.h"
#include "tube_test/dead_time.h"
#include "tube_test/gamma.h"
#include "tube_test/starting_voltage.h"

typedef struct
{
    const Config *config;
    const GammaDoseRateTestResults *small_gamma;
    const GammaDoseRateTestResults *large_gamma;
    const StartingVoltageTestResults *small_starting_voltage;
    const StartingVoltageTestResults *large_starting_voltage;
    const DeadTimeTestResults *small_dead_time;
    const DeadTimeTestResults *large_dead_time;
} TestReportData;

int write_test_report(
    const TestReportData *data,
    char *generated_path,
    size_t generated_path_size);

#endif
