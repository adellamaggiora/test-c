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

    GammaDoseRateTestResult gamma_result = {
        .tube_index = 0,
        .avg = 10.2f,
        .thread_started = true,
        .ref_tube = false,
        .test_passed = true};
    GammaDoseRateTestResults gamma = {
        .count = 1,
        .test_results = &gamma_result};

    StartingVoltageTestResult starting_voltage_result = {
        .tube_index = 0,
        .status = STARTING_VOLTAGE_FOUND,
        .starting_voltage = 330,
        .detected_events = 7,
        .deviation_percent = 2.0f,
        .test_passed = true};
    StartingVoltageTestResults starting_voltage = {
        .error = STARTING_VOLTAGE_TEST_OK,
        .safe_shutdown_ok = true,
        .count = 1,
        .test_results = &starting_voltage_result};

    DeadTimeTestResult dead_time_result = {
        .tube_index = 0,
        .status = DEAD_TIME_VALUE_VALID,
        .measured_dead_time_us = 190.0,
        .reference_dead_time_us = 190.0,
        .deviation_percent = 0.0,
        .test_passed = true};
    DeadTimeTestResults dead_time = {
        .error = DEAD_TIME_TEST_OK,
        .count = 1,
        .test_results = &dead_time_result};

    TestReportData data = {
        .config = &config,
        .tube_code = "GM001",
        .tube_profile = "small",
        .tube_index = 0,
        .gamma = &gamma,
        .starting_voltage = &starting_voltage,
        .dead_time = &dead_time};

    char report_path[4096];
    assert(write_test_report(
               &data,
               report_path,
               sizeof(report_path)) == 0);
    assert(strcmp(report_path, "build/test_reports/GM001.txt") == 0);

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
    assert(strstr(content, "TUBE TEST REPORT") != NULL);
    assert(strstr(content, "Tube code: GM001") != NULL);
    assert(strstr(content, "Operator: test operator") != NULL);
    assert(strstr(content, "Overall result: PASS") != NULL);
    assert(strstr(content, "[x] Gamma: PASS") != NULL);

    data.tube_code = "../invalid";
    assert(write_test_report(
               &data,
               report_path,
               sizeof(report_path)) != 0);

    assert(unlink(report_path) == 0);
    rmdir("build/test_reports");

    puts("report test: ok");
    return 0;
}
