#include "compiled_config.h"

/*
 * Acceptance limits are compiled into the executable so the operator cannot
 * change the PASS/FAIL criteria by editing config.toml.
 */
const CompiledAcceptanceConfig compiled_acceptance_config = {
    .small_tube = {
        .gamma_max_deviation_percent = 5,
        .starting_voltage_max_deviation_percent = 10,
        .dead_time_max_deviation_percent = 5.0},
    .large_tube = {
        .gamma_max_deviation_percent = 5,
        .starting_voltage_max_deviation_percent = 10,
        .dead_time_max_deviation_percent = 5.0}};
