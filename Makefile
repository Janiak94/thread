CC=gcc #compiler
LIBFLAGS=-lm -pthread #math library (math.h) and POSIX thread (pthread.h)
STD=c11 #c standard
OFLAG=-O2 -ffast-math #optimization flag

.PHONY: all performance

all: newton

newton:newton.c
	$(CC) $(OFLAG) -I. -o $@ $< $(LIBFLAGS)



.PHONY: gcov gprof valgrind

performance: gcov gprof valgrind perf

gcov:newton.c
	$(CC) -I. -ftest-coverage -fprofile-arcs -o newton_gcov $< $(LIBFLAGS)

gprof:newton.c
	$(CC) $(OFLAG) -pg -o $@ $< $(LIBFLAGS)

valgrind:newton.c
	$(CC) -g -o $@ $< $(LIBFLAGS)

perf:newton.c
	$(CC) $(OFLAG) -o $@ $< $(LIBFLAGS)

clean: 
	rm newton
