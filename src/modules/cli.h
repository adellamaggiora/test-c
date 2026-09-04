#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdio.h>
#include "config.h"

int run_cli(
    FILE *input,
    FILE *output,
    FILE *error_output,
    const Config *config,
    size_t maximum_tubes,
    size_t gamma_reference_tube_index);

#endif
