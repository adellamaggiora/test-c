# Guida alla "Dockerizzazione"


## Creazione e avvio del container

```bash
docker build . -t <image-name>
```

Creazione immagine Docker per avere sotto al cofano un ambiente di sviluppo con tutti gli strumenti necessari per compilazione e debugging.

```bash
docker run -it --rm -v ${PWD}:/workspace -p 5000:5000 <image-name>
```

Avvio del container in modo interattivo, montando la cartella corrente dentro `/workspace`.

La parte:

```bash
-p 5000:5000
```

serve per usare strumenti via browser, ad esempio `gdbgui`.


## Compilazione

```bash
gcc -g -Wall -Wextra -std=c11 -pthread -o <output-file> <input-file>.c
```

* `-g` -> include info utili per il debugging
* `-Wall` -> abilita molti warning utili
* `-Wextra` -> abilita warning aggiuntivi
* `-std=c11` -> usa la versione C11 del linguaggio C
* `-pthread` -> è l'API POSIX per creare e gestire thread in C
* `-o <output-file>` -> indica il nome del file eseguibile prodotto
* `<input-file>.c` -> file sorgente C, contenente la funzione `main()`

Esempio:

```bash
gcc -g -Wall -Wextra -std=c11 -o charstr charstr.c
```

Per compilare questo progetto dalla cartella `/workspace`:

```bash
make clean
make
```

L'eseguibile viene creato in `build/main`.


## Utilizzo del software

Avviare la CLI passando il percorso della configurazione modificabile:

```bash
./build/main config.toml
```

All'avvio il software richiede, nell'ordine:

1. il numero di tubi da testare, da 1 a 6;
2. un codice alfanumerico univoco per ogni tubo;
3. il tipo di tubo, `Small` oppure `Large`.

Il menu permette quindi di eseguire separatamente i test `Gamma`,
`Starting voltage` e `Dead time`, oppure di eseguirli tutti con `Run all`.
Il simbolo `[x]` accanto a un test indica che il test e' stato completato;
l'esito viene mostrato separatamente come `PASS` oppure `FAIL`.

La voce `Print report` si attiva dopo il completamento di tutti e tre i test.
Genera un file `.txt` per ogni tubo nella cartella indicata da
`general.report_folder_path`. Il nome del file corrisponde al codice del tubo,
per esempio:

```text
report/ABC123.txt
report/XYZ456.txt
```

I parametri operativi presenti in `config.toml` vengono letti ogni volta che il
programma viene avviato. Possono quindi essere modificati senza ricompilare il
software.

Le soglie `max_deviation_percent`, che determinano l'esito `PASS` o `FAIL`,
sono invece definite in `src/modules/compiled_config.c`. Dopo una loro modifica
e' necessario ricompilare:

```bash
make clean
make
```

## Debugging

Per compilare un programma in modo adatto al debugging:

```bash
gcc -g -Wall -Wextra -std=gnu11 -o <output-file> <input-file>.c
```

Poi si può avviare il debugger da terminale:

```bash
gdb ./charstr
```

Oppure dal browser con `gdbgui`:

```bash
gdbgui -r --host 0.0.0.0 --port 5000 ./charstr
```

Dal browser aprire:

```text
http://localhost:5000
```
