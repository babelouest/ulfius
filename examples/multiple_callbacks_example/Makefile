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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

LIBYDER_LOCATION=../../lib/yder/src
CC=gcc
CFLAGS=-c -Wall -I$(LIBYDER_LOCATION) -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-L$(LIBYDER_LOCATION) -lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

all: multiple_callbacks_example

clean:
	rm -f *.o multiple_callbacks_example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: multiple_callbacks_example

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

multiple_callbacks_example.o: multiple_callbacks_example.c
	$(CC) $(CFLAGS) multiple_callbacks_example.c -DDEBUG -g -O0

multiple_callbacks_example: libulfius.so multiple_callbacks_example.o
	$(CC) -o multiple_callbacks_example multiple_callbacks_example.o $(LIBS)

test: multiple_callbacks_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./multiple_callbacks_example
