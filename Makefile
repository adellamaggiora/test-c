CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -Wpedantic -pthread -std=gnu11
INCLUDES = -Ilib/tomlc17 -Isrc/modules
LDLIBS = -lm

SOURCES = src/main.c \
	src/modules/config.c \
	src/modules/report.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/gamma.c \
	src/modules/tube_test/starting_voltage.c \
	src/modules/tube_test/dead_time.c \
    lib/tomlc17/tomlc17.c

TEST_SOURCES = tests/test_starting_voltage.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/starting_voltage.c

DEAD_TIME_TEST_SOURCES = tests/test_dead_time.c \
	src/modules/tube_test/dead_time.c

REPORT_TEST_SOURCES = tests/test_report.c \
	src/modules/report.c \
	src/modules/mercury_reader.c \
	src/modules/tube_test/starting_voltage.c \
	src/modules/tube_test/dead_time.c

build/main: $(SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o build/main $(LDLIBS)

build/test_starting_voltage: $(TEST_SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SOURCES) -o build/test_starting_voltage $(LDLIBS)

build/test_dead_time: $(DEAD_TIME_TEST_SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(DEAD_TIME_TEST_SOURCES) -o build/test_dead_time $(LDLIBS)

build/test_report: $(REPORT_TEST_SOURCES)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(REPORT_TEST_SOURCES) -o build/test_report $(LDLIBS)

test: build/test_starting_voltage build/test_dead_time build/test_report
	./build/test_starting_voltage
	./build/test_dead_time
	./build/test_report

clean:
	rm -rf build
