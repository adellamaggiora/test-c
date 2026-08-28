#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "report.h"

int main(void)
{
    Config config = {
        .operator_name = "test operator",
        .report_folder_path = "build/test_reports",
        .execution_mode = "simulated"};

    GammaDoseRateTestResults gamma = {0};
    StartingVoltageTestResults starting_voltage = {0};
    DeadTimeTestResults dead_time = {0};

    TestReportData data = {
        .config = &config,
        .small_gamma = &gamma,
        .large_gamma = &gamma,
        .small_starting_voltage = &starting_voltage,
        .large_starting_voltage = &starting_voltage,
        .small_dead_time = &dead_time,
        .large_dead_time = &dead_time};

    char report_path[4096];
    assert(write_test_report(
               &data,
               report_path,
               sizeof(report_path)) == 0);

    FILE *report = fopen(report_path, "r");
    assert(report != NULL);

    char content[4096];
    size_t bytes_read = fread(
        content,
        1,
        sizeof(content) - 1,
        report);
    content[bytes_read] = '\0';

    assert(fclose(report) == 0);
    assert(strstr(content, "Tube test report") != NULL);
    assert(strstr(content, "Operator: test operator") != NULL);

    assert(unlink(report_path) == 0);
    rmdir("build/test_reports");

    puts("report test: ok");
    return 0;
}
