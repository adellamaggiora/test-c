CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wpedantic -std=gnu11
INCLUDES = -Ilib/tomlc17 -Isrc/modules

SOURCES = src/main.c \
          src/modules/config.c \
          lib/tomlc17/tomlc17.c

build/main: $(SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o build/main

clean:
	rm -rf build