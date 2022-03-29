#!/usr/bin/make -f 


CC=gcc
CFLAGS=-g -O2 -Wall 

objs = lfo time_exec


all: $(objs)

lfo: src/main.o
	$(CC) $(CCFLAGS) src/main.o -o lfo

time_exec: src/time_exec.o
	$(CC) $(CCFLAGS) src/time_exec.o -o time_exec

.PHONY: clean copy

copy:
	cp -r data data2

clean:
	rm -f src/*.o
	rm -fr data2
