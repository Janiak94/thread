CC=gcc #compiler
LIBFLAGS=-lm -pthread #math library (math.h) and POSIX thread (pthread.h)
STD=c11 #c standard
OFLAG=-O2 #optimization flag

newton:src/newton.c
	$(CC) $(OFLAG) -I. -o $@ $< $(LIBFLAGS)

test:test.c
	$(CC) $(OFLAG) -o $@ $< $(LIBFLAGS)

.PHONY: gcov

gcov: src/newton.c
	$(CC) $(OFLAG) -I. -ftest-coverage -fprofile-arcs -o newton_gcov $< $(LIBFLAGS)

clean: 
	rm newton
