CFLAGS ?= -O2 -march=native -g -std=c11 -Wall -W -pedantic
CXXFLAGS ?= -O2 -march=native -g -std=c++11 -Wall -W -pedantic
AFL_CC ?= afl-gcc

BENCHMARK_SRCS = \
	benchmark.cpp \
	benchmark-packmsg.cpp \
	benchmark-msgpack.cpp \
	benchmark-printf.cpp

BENCHMARK_HDRS = \
	benchmark-packmsg.h \
	benchmark-msgpack.h \
	benchmark-printf.h

all: example benchmark

example: example.c packmsg.h Makefile
	$(CC) -o $@ $< $(CFLAGS)

benchmark: $(BENCHMARK_SRCS) $(BENCHMARK_HDRS) packmsg.h Makefile
	$(CXX) -o $@ $(BENCHMARK_SRCS) $(CXXFLAGS) -lbenchmark -lmsgpackc

test: test.c packmsg.h Makefile
	$(CC) -o $@ $< $(CFLAGS) `pkg-config --cflags --libs check`

decode: decode.c packmsg.h Makefile
	$(AFL_CC) -o $@ $< $(CFLAGS)

check: test
	./test

fuzz: decode check
	afl-fuzz -i fuzz-in -o fuzz-out -- ./decode @@

clean:
	rm -f example benchmark decode test fuzz-in/testcase-*

.PHONY: clean check fuzz
