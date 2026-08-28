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

static void write_gamma_results(
    FILE *file,
    const char *tube_type,
    const GammaDoseRateTestResults *results)
{
    fprintf(file, "\nGamma test (%s tubes)\n", tube_type);

    if (results == NULL || results->test_results == NULL)
    {
        fprintf(file, "  test unavailable\n");
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const GammaDoseRateTestResult *tube = &results->test_results[i];
        fprintf(
            file,
            "  tube %zu: average=%.2f, reference=%s, "
            "thread_started=%s, passed=%s\n",
            i,
            tube->avg,
            tube->ref_tube ? "yes" : "no",
            tube->thread_started ? "yes" : "no",
            tube->test_passed ? "yes" : "no");
    }
}

static void write_starting_voltage_results(
    FILE *file,
    const char *tube_type,
    const StartingVoltageTestResults *results)
{
    fprintf(file, "\nStarting voltage test (%s tubes)\n", tube_type);

    if (results == NULL || results->error != STARTING_VOLTAGE_TEST_OK)
    {
        fprintf(file, "  test error: %d\n",
                results == NULL ? -1 : (int)results->error);
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const StartingVoltageTestResult *tube = &results->test_results[i];
        fprintf(
            file,
            "  tube %zu: status=%s, voltage=%d V, events=%u, "
            "deviation=%.2f%%, passed=%s\n",
            tube->tube_index,
            starting_voltage_status_string(tube->status),
            tube->starting_voltage,
            tube->detected_events,
            tube->deviation_percent,
            tube->test_passed ? "yes" : "no");
    }

    fprintf(file, "  safe shutdown: %s\n",
            results->safe_shutdown_ok ? "yes" : "no");
}

static void write_dead_time_results(
    FILE *file,
    const char *tube_type,
    const DeadTimeTestResults *results)
{
    fprintf(file, "\nDead time test (%s tubes)\n", tube_type);

    if (results == NULL || results->error != DEAD_TIME_TEST_OK)
    {
        fprintf(file, "  test error: %d\n",
                results == NULL ? -1 : (int)results->error);
        return;
    }

    for (size_t i = 0; i < results->count; i++)
    {
        const DeadTimeTestResult *tube = &results->test_results[i];
        fprintf(
            file,
            "  tube %zu: status=%s, measured=%.2f us, "
            "reference=%.2f us, deviation=%.2f%%, passed=%s\n",
            tube->tube_index,
            dead_time_status_string(tube->status),
            tube->measured_dead_time_us,
            tube->reference_dead_time_us,
            tube->deviation_percent,
            tube->test_passed ? "yes" : "no");
    }
}

static FILE *open_unique_report(
    const char *directory,
    char *generated_path,
    size_t generated_path_size,
    const struct tm *local_time)
{
    char timestamp[32];
    if (strftime(
            timestamp,
            sizeof(timestamp),
            "%Y%m%d_%H%M%S",
            local_time) == 0)
    {
        return NULL;
    }

    for (unsigned int suffix = 0; suffix < 1000; suffix++)
    {
        int written;
        if (suffix == 0)
        {
            written = snprintf(
                generated_path,
                generated_path_size,
                "%s/test_report_%s.txt",
                directory,
                timestamp);
        }
        else
        {
            written = snprintf(
                generated_path,
                generated_path_size,
                "%s/test_report_%s_%u.txt",
                directory,
                timestamp,
                suffix);
        }

        if (written < 0 || (size_t)written >= generated_path_size)
        {
            return NULL;
        }

        int descriptor = open(
            generated_path,
            O_WRONLY | O_CREAT | O_EXCL,
            0644);

        if (descriptor >= 0)
        {
            FILE *file = fdopen(descriptor, "w");
            if (file == NULL)
            {
                close(descriptor);
            }
            return file;
        }

        if (errno != EEXIST)
        {
            return NULL;
        }
    }

    return NULL;
}

int write_test_report(
    const TestReportData *data,
    char *generated_path,
    size_t generated_path_size)
{
    if (data == NULL || data->config == NULL ||
        generated_path == NULL || generated_path_size == 0 ||
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

    FILE *file = open_unique_report(
        data->config->report_folder_path,
        generated_path,
        generated_path_size,
        &local_time);

    if (file == NULL)
    {
        return -1;
    }

    char generated_at[64];
    strftime(
        generated_at,
        sizeof(generated_at),
        "%Y-%m-%d %H:%M:%S",
        &local_time);

    fprintf(file, "Tube test report\n");
    fprintf(file, "Generated at: %s\n", generated_at);
    fprintf(file, "Operator: %s\n", data->config->operator_name);
    fprintf(file, "Execution mode: %s\n", data->config->execution_mode);

    write_gamma_results(file, "small", data->small_gamma);
    write_gamma_results(file, "large", data->large_gamma);
    write_starting_voltage_results(
        file,
        "small",
        data->small_starting_voltage);
    write_starting_voltage_results(
        file,
        "large",
        data->large_starting_voltage);
    write_dead_time_results(file, "small", data->small_dead_time);
    write_dead_time_results(file, "large", data->large_dead_time);

    if (fclose(file) != 0)
    {
        return -1;
    }

    return 0;
}
