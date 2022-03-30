#!/usr/bin/make -f 


CC= gcc
CFLAGS= -O2 -Wall -Wextra --pedantic-errors

objs = lfo time_exec


all: $(objs)

lfo: src/main.o
	$(CC) $(CCFLAGS) src/main.o -o lfo

time_exec: src/time_exec.o
	$(CC) $(CCFLAGS) src/time_exec.o -o time_exec

.PHONY: clean copy install

install: all
	mkdir -p bin
	cp lfo bin/

copy:
	cp -r data data2

clean:
	rm -f src/*.o
	rm -fr data2
