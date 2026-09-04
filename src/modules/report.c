#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "report.h"

#define MAX_TUBE_CODE_LENGTH 64

static int create_directory_tree(const char *directory)
{
    if (directory == NULL || directory[0] == '\0')
    {
        return -1;
    }

    char path[PATH_MAX];
    int written = snprintf(path, sizeof(path), "%s", directory);

    if (written < 0 || (size_t)written >= sizeof(path))
    {
        return -1;
    }

    size_t length = strlen(path);
    while (length > 1 && path[length - 1] == '/')
    {
        path[--length] = '\0';
    }

    for (char *separator = path + 1; *separator != '\0'; separator++)
    {
        if (*separator != '/')
        {
            continue;
        }

        *separator = '\0';
        if (mkdir(path, 0755) != 0 && errno != EEXIST)
        {
            return -1;
        }
        *separator = '/';
    }

    if (mkdir(path, 0755) != 0 && errno != EEXIST)
    {
        return -1;
    }

    return 0;
}

int tube_code_is_valid(const char *tube_code)
{
    if (tube_code == NULL)
    {
        return 0;
    }

    size_t length = strlen(tube_code);
    if (length == 0 || length > MAX_TUBE_CODE_LENGTH ||
        strcmp(tube_code, ".") == 0 || strcmp(tube_code, "..") == 0)
    {
        return 0;
    }

    for (size_t i = 0; i < length; i++)
    {
        unsigned char character = (unsigned char)tube_code[i];
        if (!isalnum(character))
        {
            return 0;
        }
    }

    return 1;
}

static const GammaDoseRateTestResult *find_gamma_result(
    const GammaDoseRateTestResults *results,
    size_t tube_index)
{
    if (results == NULL || results->test_results == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        if (results->test_results[i].tube_index == tube_index)
        {
            return &results->test_results[i];
        }
    }

    return NULL;
}

static const StartingVoltageTestResult *find_starting_voltage_result(
    const StartingVoltageTestResults *results,
    size_t tube_index)
{
    if (results == NULL || results->error != STARTING_VOLTAGE_TEST_OK ||
        results->test_results == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        if (results->test_results[i].tube_index == tube_index)
        {
            return &results->test_results[i];
        }
    }

    return NULL;
}

static const DeadTimeTestResult *find_dead_time_result(
    const DeadTimeTestResults *results,
    size_t tube_index)
{
    if (results == NULL || results->error != DEAD_TIME_TEST_OK ||
        results->test_results == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        if (results->test_results[i].tube_index == tube_index)
        {
            return &results->test_results[i];
        }
    }

    return NULL;
}

static const char *test_outcome(bool passed)
{
    return passed ? "PASS" : "FAIL";
}

static void write_gamma_result(
    FILE *file,
    const GammaDoseRateTestResult *result)
{
    fprintf(file, "\nGamma test\n");
    fprintf(file, "----------\n");
    fprintf(file, "Average dose rate: %.2f\n", result->avg);
    fprintf(file, "Reference tube: %s\n", result->ref_tube ? "yes" : "no");
    fprintf(file, "Acquisition started: %s\n",
            result->thread_started ? "yes" : "no");
    fprintf(file, "Result: %s\n", test_outcome(result->test_passed));
}

static void write_starting_voltage_result(
    FILE *file,
    const StartingVoltageTestResult *result,
    bool safe_shutdown_ok)
{
    fprintf(file, "\nStarting voltage test\n");
    fprintf(file, "---------------------\n");
    fprintf(file, "Status: %s\n",
            starting_voltage_status_string(result->status));

    if (result->status == STARTING_VOLTAGE_FOUND)
    {
        fprintf(file, "Starting voltage: %d V\n", result->starting_voltage);
        fprintf(file, "Detected events: %u\n", result->detected_events);
        fprintf(file, "Deviation: %.2f%%\n", result->deviation_percent);
    }

    fprintf(file, "Safe shutdown: %s\n", safe_shutdown_ok ? "yes" : "no");
    fprintf(file, "Result: %s\n", test_outcome(result->test_passed));
}

static void write_dead_time_result(
    FILE *file,
    const DeadTimeTestResult *result)
{
    fprintf(file, "\nDead time test\n");
    fprintf(file, "--------------\n");
    fprintf(file, "Status: %s\n", dead_time_status_string(result->status));
    fprintf(file, "Measured dead time: %.2f us\n",
            result->measured_dead_time_us);
    fprintf(file, "Reference dead time: %.2f us\n",
            result->reference_dead_time_us);
    fprintf(file, "Deviation: %.2f%%\n", result->deviation_percent);
    fprintf(file, "Result: %s\n", test_outcome(result->test_passed));
}

static FILE *open_report(
    const char *directory,
    const char *tube_code,
    char *generated_path,
    size_t generated_path_size)
{
    int written = snprintf(
        generated_path,
        generated_path_size,
        "%s/%s.txt",
        directory,
        tube_code);

    if (written < 0 || (size_t)written >= generated_path_size)
    {
        return NULL;
    }

    int descriptor = open(
        generated_path,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644);

    if (descriptor < 0)
    {
        return NULL;
    }

    FILE *file = fdopen(descriptor, "w");
    if (file == NULL)
    {
        close(descriptor);
    }

    return file;
}

int write_test_report(
    const TestReportData *data,
    char *generated_path,
    size_t generated_path_size)
{
    if (data == NULL || data->config == NULL ||
        data->config->operator_name == NULL ||
        data->config->report_folder_path == NULL ||
        data->config->execution_mode == NULL ||
        data->tube_profile == NULL || !tube_code_is_valid(data->tube_code) ||
        generated_path == NULL || generated_path_size == 0)
    {
        return -1;
    }

    const GammaDoseRateTestResult *gamma =
        find_gamma_result(data->gamma, data->tube_index);
    const StartingVoltageTestResult *starting_voltage =
        find_starting_voltage_result(
            data->starting_voltage,
            data->tube_index);
    const DeadTimeTestResult *dead_time =
        find_dead_time_result(data->dead_time, data->tube_index);

    if (gamma == NULL || starting_voltage == NULL || dead_time == NULL ||
        create_directory_tree(data->config->report_folder_path) != 0)
    {
        return -1;
    }

    time_t now = time(NULL);
    struct tm local_time;
    if (now == (time_t)-1 || localtime_r(&now, &local_time) == NULL)
    {
        return -1;
    }

    FILE *file = open_report(
        data->config->report_folder_path,
        data->tube_code,
        generated_path,
        generated_path_size);

    if (file == NULL)
    {
        return -1;
    }

    char generated_at[64];
    if (strftime(
            generated_at,
            sizeof(generated_at),
            "%Y-%m-%d %H:%M:%S",
            &local_time) == 0)
    {
        fclose(file);
        return -1;
    }

    bool overall_passed = gamma->test_passed &&
        starting_voltage->test_passed && dead_time->test_passed;

    fprintf(file, "TUBE TEST REPORT\n");
    fprintf(file, "================\n");
    fprintf(file, "Tube code: %s\n", data->tube_code);
    fprintf(file, "Tube profile: %s\n", data->tube_profile);
    fprintf(file, "Generated at: %s\n", generated_at);
    fprintf(file, "Operator: %s\n", data->config->operator_name);
    fprintf(file, "Execution mode: %s\n", data->config->execution_mode);
    fprintf(file, "Overall result: %s\n", test_outcome(overall_passed));

    fprintf(file, "\nTest summary\n");
    fprintf(file, "------------\n");
    fprintf(file, "[x] Gamma: %s\n", test_outcome(gamma->test_passed));
    fprintf(file, "[x] Starting voltage: %s\n",
            test_outcome(starting_voltage->test_passed));
    fprintf(file, "[x] Dead time: %s\n",
            test_outcome(dead_time->test_passed));

    write_gamma_result(file, gamma);
    write_starting_voltage_result(
        file,
        starting_voltage,
        data->starting_voltage->safe_shutdown_ok);
    write_dead_time_result(file, dead_time);

    if (fclose(file) != 0)
    {
        return -1;
    }

    return 0;
}
