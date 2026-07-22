#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tomlc17.h"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "Error: expected exactly one argument: <config_file>\n");
        return EXIT_FAILURE;
    }

    toml_result_t parsed = toml_parse_file_ex(argv[1]);

    if (!parsed.ok)
    {
        error(parsed.errmsg, 0);
    }

    // Extract values
    toml_datum_t host = toml_seek(parsed.toptab, "server.host");

    FILE *file = fopen(argv[1], "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error: failed to open the configuration file\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Store the content of the file
    char myString[100];

    // Read the content and print it
    while (fgets(myString, 100, file))
    {
        printf("%s", myString);
    }

    fclose(file);

    return 0;
}