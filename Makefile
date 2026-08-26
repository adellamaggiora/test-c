CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wpedantic -pthread -std=gnu11
INCLUDES = -Ilib/tomlc17 -Isrc/modules
LDLIBS = -lm

SOURCES = src/main.c \
	src/modules/config.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/gamma.c \
	src/modules/tube_test/starting_voltage.c \
    lib/tomlc17/tomlc17.c

TEST_SOURCES = tests/test_starting_voltage.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/starting_voltage.c

build/main: $(SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o build/main $(LDLIBS)

build/test_starting_voltage: $(TEST_SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SOURCES) -o build/test_starting_voltage $(LDLIBS)

test: build/test_starting_voltage
	./build/test_starting_voltage

clean:
	rm -rf build
