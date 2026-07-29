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

- il tipo ```atomic_bool``` è usato per supportare le operazioni ```atomic_load()``` e ```atomic_store()```. Questo operazioni sono atomiche, ovvero indivisibili rispetto agli altri thread: nessun thread può osservare una scrittura parziale

- dichiarare una variabile o una funzione con ```static``` significa che quella variabile è visibile solo nel file .c in cui è dichiarata (internal linkage)

- Principali tipi

| Tipo                 | Descrizione                        | Dimensione tipica  | Esempio                             |
| -------------------- | ---------------------------------- | -----------------: | ----------------------------------- |
| `char`               | carattere / intero piccolo         |             1 byte | `char c = 'A';`                     |
| `signed char`        | intero con segno                   |             1 byte | `signed char x = -10;`              |
| `unsigned char`      | intero senza segno                 |             1 byte | `unsigned char x = 255;`            |
| `short`              | intero corto                       |             2 byte | `short x = -1000;`                  |
| `unsigned short`     | intero corto senza segno           |             2 byte | `unsigned short x = 1000;`          |
| `int`                | intero standard                    |             4 byte | `int x = -42;`                      |
| `unsigned int`       | intero senza segno                 |             4 byte | `unsigned int x = 42U;`             |
| `long`               | intero lungo                       |         4 o 8 byte | `long x = 100000L;`                 |
| `unsigned long`      | intero lungo senza segno           |         4 o 8 byte | `unsigned long x = 100000UL;`       |
| `long long`          | intero molto lungo                 |             8 byte | `long long x = 100000LL;`           |
| `unsigned long long` | intero molto lungo senza segno     |             8 byte | `unsigned long long x = 100000ULL;` |
| `float`              | virgola mobile, precisione singola |             4 byte | `float x = 3.14f;`                  |
| `double`             | virgola mobile, precisione doppia  |             8 byte | `double x = 3.14;`                  |
| `long double`        | virgola mobile estesa              |    8, 12 o 16 byte | `long double x = 3.14L;`            |
| `_Bool` / `bool`     | valore booleano                    |      1 byte tipico | `bool ok = true;`                   |
| `void`               | assenza di valore                  |                  — | `void funzione(void);`              |

- un puntatore di tipo `void` può puntare a qualunque tipo di dato, rendendolo versatile