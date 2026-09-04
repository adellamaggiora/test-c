#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "config.h"
#include "mercury_reader.h"

#define TOTAL_TUBES 6
#define GAMMA_REFERENCE_TUBE_INDEX 1

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(
            stderr,
            "Usage: %s <config_file>\n",
            argv[0]);
        return EXIT_FAILURE;
    }

    const char *file_path = argv[1];
    Config config;

    if (read_config(file_path, &config) != 0)
    {
        return EXIT_FAILURE;
    }

    if (mercury_reader_init(TOTAL_TUBES) != MERCURY_READER_OK)
    {
        fprintf(stderr, "Error: Mercury reader initialization failed.\n");
        free_config(&config);
        return EXIT_FAILURE;
    }

    int cli_result = run_cli(
        stdin,
        stdout,
        stderr,
        &config,
        TOTAL_TUBES,
        GAMMA_REFERENCE_TUBE_INDEX);

    mercury_reader_close();
    free_config(&config);

    return cli_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
