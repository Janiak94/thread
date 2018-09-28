CC=gcc
LIBFLAGS=-lm
STD=c11

main:src/main.c
	$(CC) -o $@ $< $(LIBFLAGS)
