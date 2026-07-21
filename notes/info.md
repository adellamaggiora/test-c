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