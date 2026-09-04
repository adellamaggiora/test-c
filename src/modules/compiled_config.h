#ifndef COMPILED_CONFIG_H
#define COMPILED_CONFIG_H

typedef struct
{
    int gamma_max_deviation_percent;
    int starting_voltage_max_deviation_percent;
    double dead_time_max_deviation_percent;
} CompiledTubeAcceptanceConfig;

typedef struct
{
    CompiledTubeAcceptanceConfig small_tube;
    CompiledTubeAcceptanceConfig large_tube;
} CompiledAcceptanceConfig;

extern const CompiledAcceptanceConfig compiled_acceptance_config;

#endif
