#ifndef CONFIG_H
#define CONFIG_H


typedef struct 
{
    int acquisition_time_sec;
    int max_sensitivity_deviation_percent;
    int avg_dose_rate_ref;
} GammaTestParams;

typedef struct 
{
    int voltage_steps;
    int min_detected_events;
    int step_acquisition_time_sec;
    int min_voltage;
    int max_voltage;
    int hv_increment_per_step;
} StartingVoltageTestParams;

typedef struct 
{
    GammaTestParams gamma_test_params;
    StartingVoltageTestParams starting_voltage_test_params;
} TubeConfig;

typedef struct 
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