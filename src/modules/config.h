typedef struct {
    int acquisition_time_sec;
    int max_sensitivity_deviation_percent;
} GammaTestParams;

typedef struct {
    int voltage_steps;
    int min_detected_events;
    int step_acquisition_time_sec;
    int min_voltage;
    int max_voltage;
    int hv_increment_per_step;
} StartingVoltageTestParams;

typedef struct {
    char *operator_name;
    char *report_folder_path;
    char *execution_mode;
    GammaTestParams gamma_test_params;
    StartingVoltageTestParams starting_voltage_test_params;
} Config;



Config read_config(const char *path, Config *config);