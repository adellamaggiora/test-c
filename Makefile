CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wpedantic -pthread -std=gnu11
INCLUDES = -Ilib/tomlc17 -Isrc/modules
LDLIBS = -lm

TARGET = build/main
SOURCES = src/main.c \
	src/modules/cli.c \
	src/modules/compiled_config.c \
	src/modules/config.c \
	src/modules/report.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/gamma.c \
	src/modules/tube_test/starting_voltage.c \
	src/modules/tube_test/dead_time.c \
	lib/tomlc17/tomlc17.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -rf build
