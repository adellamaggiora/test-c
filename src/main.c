#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main() {


    FILE *file = fopen("config.txt", "r");

    if (file == NULL) {
        fprintf(stderr, "Errore apertura file\n");
        return EXIT_FAILURE;
    }


    



    return 0;



}