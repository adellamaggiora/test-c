# C Best Practices and Info

- le struct si mettono nel file .h

-   le include guard servono a evitare che lo stesso .h venga incluso più volte
    ```c
        #ifndef CONFIG_H
        #define CONFIG_H

        // contenuto di config.h

        #endif
    ```
- non è necessario creare una cartella models contente le struct: si mettono direttamente nel modulo

- ```char *[]``` è equivalente a ```char **```

- ```int``` numero intero con segno ```size_t``` intero senza segno

- ```exit(EXIT_FAILURE)``` termina l'intero programma da qualunque funzione venga chiamato, ```return EXIT_FAILURE``` termina il main
e restituisce l'errore al sistema operativo

- aggiunta di librerie esterne: #include "tomlc17.h" serve solo a rendere visibili dichiarazioni, tipi e funzioni. Non compila automaticamente l’implementazione. Occorre quindi compilare la libreria

- il compilatore trova la libreria tomlc17 senza importare il percorso esatto perchè il Makefile imposta questa opzione nel compilatore ```INCLUDES = -I lib/tomlc17``` che va a cercare gli header in ```lib/tomlc17/```
