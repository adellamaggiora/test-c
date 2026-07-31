#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include "config.h"
#include "mercury_reader.h"


static atomic_bool acquisition_running;


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



    acquisition_running = true;



    free_config(&config);

    

    
    
    return 0;
}