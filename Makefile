CC=gcc #compiler
LIBFLAGS=-lm -pthread #math library (math.h) and POSIX thread (pthread.h)
STD=c11 #c standard
OFLAG=-O2 #optimization flag

newton:src/newton.c
	$(CC) $(OFLAG) -o $@ $< $(LIBFLAGS)

clean: 
	rm newton
