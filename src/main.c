#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"


int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Error: expected exactly one argument: <config_file>\n");
        return EXIT_FAILURE;
    }

    char *file_path = argv[1];
    Config config;

    if (read_config(file_path, &config) != 0)
    {
        return EXIT_FAILURE;
    }


    free_config(&config);

    return 0;
}