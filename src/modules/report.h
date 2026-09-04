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
    const char *tube_code;
    const char *tube_profile;
    size_t tube_index;
    const GammaDoseRateTestResults *gamma;
    const StartingVoltageTestResults *starting_voltage;
    const DeadTimeTestResults *dead_time;
} TestReportData;

/*
 * Tube codes are also used as file names. Keeping the accepted character set
 * small makes it impossible for user input to escape the report directory.
 */
int tube_code_is_valid(const char *tube_code);

int write_test_report(
    const TestReportData *data,
    char *generated_path,
    size_t generated_path_size);

#endif
