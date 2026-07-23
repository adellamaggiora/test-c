#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tomlc17.h>
#include "config.h"


static int read_int(
    toml_datum_t root,
    const char *path,
    int *destination
) {
    toml_datum_t value = toml_seek(root, path);

    if (value.type != TOML_INT64) {
        fprintf(stderr, "Configuration error: '%s' must be an integer\n", path);
        return -1;
    }

    if (value.u.int64 < INT_MIN || value.u.int64 > INT_MAX) {
        fprintf(stderr, "Configuration error: '%s' is out of range\n", path);
        return -1;
    }

    *destination = (int)value.u.int64;
    return 0;
}

static int read_string(
    toml_datum_t root,
    const char *path,
    char **destination
) {
    toml_datum_t value = toml_seek(root, path);

    if (value.type != TOML_STRING) {
        fprintf(stderr, "Configuration error: '%s' must be a string\n", path);
        return -1;
    }

    size_t length = strlen(value.u.s);

    *destination = malloc(length + 1);

    if (*destination == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return -1;
    }

    memcpy(*destination, value.u.s, length + 1);
    return 0;
}

int read_config(const char *file_path, Config *config)
{
    // azzera tutti i campi della struct config (passata come parametro)
    *config = (Config){0};

    toml_result_t parsed = toml_parse_file_ex(file_path);

    if (!parsed.ok) {
        fprintf(stderr, "Configuration error: %s\n", parsed.errmsg);
        toml_free(parsed);
        return -1;
    }

    toml_datum_t root = parsed.toptab;

    int failed =
        read_string(root, "general.operator_name",
                    &config->operator_name) ||

        read_string(root, "general.report_folder_path",
                    &config->report_folder_path) ||

        read_string(root, "general.execution_mode",
                    &config->execution_mode) ||

        read_int(root, "small_tube.test_gamma.acquisition_time_sec",
                 &config->small_tube.gamma_test_params.acquisition_time_sec) ||

        read_int(root, "small_tube.test_gamma.max_sensitivity_deviation_percent",
                 &config->small_tube.gamma_test_params.max_sensitivity_deviation_percent) ||

        read_int(root, "large_tube.test_gamma.acquisition_time_sec",
                 &config->large_tube.gamma_test_params.acquisition_time_sec) ||

        read_int(root, "large_tube.test_gamma.max_sensitivity_deviation_percent",
                 &config->large_tube.gamma_test_params.max_sensitivity_deviation_percent) ||

        read_int(root, "small_tube.test_starting_voltage.voltage_steps",
                 &config->small_tube.starting_voltage_test_params.voltage_steps) ||

        read_int(root, "small_tube.test_starting_voltage.min_detected_events",
                 &config->small_tube.starting_voltage_test_params.min_detected_events) ||

        read_int(root, "small_tube.test_starting_voltage.step_acquisition_time_sec",
                 &config->small_tube.starting_voltage_test_params.step_acquisition_time_sec) ||

        read_int(root, "small_tube.test_starting_voltage.min_voltage",
                 &config->small_tube.starting_voltage_test_params.min_voltage) ||

        read_int(root, "small_tube.test_starting_voltage.max_voltage",
                 &config->small_tube.starting_voltage_test_params.max_voltage) ||

        read_int(root, "small_tube.test_starting_voltage.hv_increment_per_step",
                 &config->small_tube.starting_voltage_test_params.hv_increment_per_step) ||

        read_int(root, "large_tube.test_starting_voltage.voltage_steps",
                 &config->large_tube.starting_voltage_test_params.voltage_steps) ||

        read_int(root, "large_tube.test_starting_voltage.min_detected_events",
                 &config->large_tube.starting_voltage_test_params.min_detected_events) ||

        read_int(root, "large_tube.test_starting_voltage.step_acquisition_time_sec",
                 &config->large_tube.starting_voltage_test_params.step_acquisition_time_sec) ||

        read_int(root, "large_tube.test_starting_voltage.min_voltage",
                 &config->large_tube.starting_voltage_test_params.min_voltage) ||

        read_int(root, "large_tube.test_starting_voltage.max_voltage",
                 &config->large_tube.starting_voltage_test_params.max_voltage) ||

        read_int(root, "large_tube.test_starting_voltage.hv_increment_per_step",
                 &config->large_tube.starting_voltage_test_params.hv_increment_per_step);

    toml_free(parsed);

    if (failed) {
        free_config(config);
        return -1;
    }

    return 0;
}

void free_config(Config *config)
{
    free(config->operator_name);
    free(config->report_folder_path);
    free(config->execution_mode);

    *config = (Config){0};
}