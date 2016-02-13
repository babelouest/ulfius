#
# Sheep counter program
#
# Makefile used to build the software
#
# Copyright 2014-2015 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
CC=gcc
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-lc -lulfius -lyder -lorcania -ljansson -L$(ULFIUS_LOCATION)

all: sheep_counter

clean:
	rm -f *.o sheep_counter

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: sheep_counter

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

sheep_counter.o: sheep_counter.c
	$(CC) $(CFLAGS) sheep_counter.c -DDEBUG -g -O0

sheep_counter: libulfius.so sheep_counter.o
	$(CC) -o sheep_counter sheep_counter.o $(LIBS)

test: sheep_counter
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./sheep_counter
