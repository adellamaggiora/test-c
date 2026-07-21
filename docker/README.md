# Guida alla "Dockerizzazione"


## Creazione e avvio del container

```bash
docker build . -t <image-name>
````

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
gcc -g -Wall -Wextra -std=c11 -o <output-file> <input-file>.c
```

* `-g` -> include info utili per il debugging
* `-Wall` -> abilita molti warning utili
* `-Wextra` -> abilita warning aggiuntivi
* `-std=c11` -> usa la versione C11 del linguaggio C
* `-o <output-file>` -> indica il nome del file eseguibile prodotto
* `<input-file>.c` -> file sorgente C, contenente la funzione `main()`

Esempio:

```bash
gcc -g -Wall -Wextra -std=c11 -o charstr charstr.c
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