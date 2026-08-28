#ifndef CONFIG_H
#define CONFIG_H


typedef struct GammaTestParams
{
    int acquisition_time_sec;
    int max_deviation_percent;
    int avg_dose_rate_ref;
} GammaTestParams;

typedef struct StartingVoltageTestParams
{
    int voltage_steps;
    int min_detected_events;
    int step_acquisition_time_sec;
    int start_voltage;
    int min_voltage;
    int max_voltage;
    int hv_increment_per_step;
    int reference_voltage;
    int max_deviation_percent;
    int voltage_settling_time_ms;
    int absolute_max_voltage;
} StartingVoltageTestParams;

typedef struct DeadTimeTestParams
{
    double reference_dead_time_us;
    double max_deviation_percent;
} DeadTimeTestParams;

typedef struct TubeConfig
{
    GammaTestParams gamma_test_params;
    StartingVoltageTestParams starting_voltage_test_params;
    DeadTimeTestParams dead_time_test_params;
} TubeConfig;

typedef struct Config
{
    char *operator_name;
    char *report_folder_path;
    char *execution_mode;
    TubeConfig small_tube;
    TubeConfig large_tube;
} Config;


int read_config(const char *path, Config *config);

void free_config(Config *config);


#endif
