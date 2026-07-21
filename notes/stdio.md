- printf() stampa sempre su stdout:

    ```c
    printf("Ciao %d\n", x);
    ```

- fprintf() stampa su uno stream scelto:
    ```c
    fprintf(stdout, "Ciao\n");
    fprintf(stderr, "Errore\n");
    fprintf(file, "Nel file\n");
    ```