#
# Example program
#
# Makefile used to build the software
#
# Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
PREFIX=/usr/local
ULFIUS_LOCATION=../../src
ULFIUS_INCLUDE=../../include
EXAMPLE_INCLUDE=../include
CC=gcc
CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) -I$(EXAMPLE_INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(LIBYDER_LOCATION) -lc -lulfius -lorcania -L$(ULFIUS_LOCATION)

ifndef YDERFLAG
LIBS+= -lyder
endif

all: simple_example

clean:
	rm -f *.o simple_example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: simple_example

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug CURLFLAG=1 GNUTLSFLAG=1

simple_example.o: simple_example.c
	$(CC) $(CFLAGS) simple_example.c -DDEBUG -g -O0

simple_example: ../../src/libulfius.so simple_example.o
	$(CC) -o simple_example simple_example.o $(LIBS)

test: simple_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./simple_example

static: simple_example.o
	$(CC) -o simple_example simple_example.o $(PREFIX)/lib/liborcania.a $(PREFIX)/lib/libyder.a $(PREFIX)/lib/libulfius.a -ljansson -lmicrohttpd -lpthread -lgnutls
