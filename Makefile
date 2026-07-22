CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wpedantic -std=gnu11
INCLUDES = -I lib/tomlc17

build/main: src/main.c lib/tomlc17/tomlc17.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) src/main.c lib/tomlc17/tomlc17.c -o build/main

clean:
	rm -rf build