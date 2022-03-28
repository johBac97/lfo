#!/usr/bin/make -f 


CC=gcc
CFLAGS=-g -O0 -Wall --pedantic-errors

objs=lfo


lfo: src/main.o
	$(CC) $(CCFLAGS) src/main.o -o lfo


.PHONY: clean



clean:
	rm  src/*.o
