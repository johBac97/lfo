#!/usr/bin/make -f 


CC=gcc
CCFLAGS=-g -O0

objs=lfo


lfo: src/main.o
	$(CC) $(CCFLAGS) src/main.o -o lfo


.PHONY: clean



clean:
	rm  src/*.o
