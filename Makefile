CC=gcc #compiler
LIBFLAGS=-lm #math library (math.h)
STD=c11 #c standard
OFLAG=-O2 #optimization flag

main:src/main.c
	$(CC) $(OFLAG) -o $@ $< $(LIBFLAGS)

ver1: src/functions.c
	$(CC) -o $@ $< $(LIBFLAGS)

ver2: src/functions_reduced.c
	$(CC) -o $@ $< $(LIBFLAGS)
