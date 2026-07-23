# C Best Practices and Info

- le struct si mettono nel file .h

- non è necessario creare una cartella models contente le struct: si mettono direttamente nel modulo

- ```char *[]``` è equivalente a ```char **```

- ```int``` numero intero con segno ```size_t``` intero senza segno

- ```exit(EXIT_FAILURE)``` termina l'intero programma da qualunque funzione venga chiamato, ```return EXIT_FAILURE``` termina il main
e restituisce l'errore al sistema operativo

- aggiunta di librerie esterne: #include "tomlc17.h" serve solo a rendere visibili dichiarazioni, tipi e funzioni. Non compila automaticamente l’implementazione. Occorre quindi compilare la libreria

- il compilatore trova la libreria tomlc17 senza importare il percorso esatto perchè il Makefile imposta questa opzione nel compilatore ```INCLUDES = -I lib/tomlc17``` che va a cercare gli header in ```lib/tomlc17/```

- ```*config = (Config){0};``` azzera tutti i campi della struct config (passata come parametro)

- Le include guard servono ad evitare che un certo modulo (un file header) venga incluso più volte nello stesso file compilato:
    ```c
        #ifndef <MODULE_NAME>_H
        #define <MODULE_NAME>_H

        /* dichiarazioni */

        #endif
    ```

- per accedere ai campi di una struct si usa ```->``` quando si ha un puntatore a una struct, mentre si usa il ```.``` quando nella variabile c'è la struct stessa. ```config_ptr->operator_name``` equivale a ```(*config_ptr).operator_name;```