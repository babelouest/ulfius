#
# Example program
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
LIBS=-lc -lulfius -ljansson -L$(ULFIUS_LOCATION)

all: simple_example

clean:
	rm -f *.o simple_example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: simple_example

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

simple_example.o: simple_example.c
	$(CC) $(CFLAGS) simple_example.c -DDEBUG -g -O0

simple_example: libulfius.so simple_example.o
	$(CC) -o simple_example simple_example.o $(LIBS)

test: simple_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./simple_example
