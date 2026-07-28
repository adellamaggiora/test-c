#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "mercury_reader.h"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Error: expected exactly one argument: <config_file>\n");
        return EXIT_FAILURE;
    }

    char *file_path = argv[1];
    Config config;

    printf("%d", 1);

    if (read_config(file_path, &config) != 0) {
        return EXIT_FAILURE;
    }


    for (int i = 0; i < 10; i++)
    {
        int dose_rate = get_gamma_dose_rate();
        printf("%d \n", dose_rate);
    }

    

    free_config(&config);

    

    
    
    return 0;
}