#!/usr/bin/make -f 


CC=gcc
CFLAGS=-g -O0 -Wall --pedantic-errors

objs=lfo


lfo: src/main.o
	$(CC) $(CCFLAGS) src/main.o -o lfo


.PHONY: clean copy

copy:
	cp -r data data2

clean:
	rm -f src/*.o
	rm -r data2
