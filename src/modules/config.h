typedef enum {
    CONFIG_OK,
    CONFIG_FILE_ERROR,
    CONFIG_PARSE_ERROR
} ConfigResult;

typedef struct {
    int param1;
    int param2;
} Config;



ConfigResult read_config(const char *path, Config *config);