#
# Example program
#
# Makefile used to build the software
#
# Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
PREFIX=/usr/local
LIBYDER_LOCATION=../../lib/yder/src
CC=gcc
CFLAGS=-c -Wall -I$(LIBYDER_LOCATION) -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-L$(LIBYDER_LOCATION) -lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

all: simple_example

clean:
	rm -f *.o simple_example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: simple_example

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

simple_example.o: simple_example.c
	$(CC) $(CFLAGS) simple_example.c -DDEBUG -g -O0

simple_example: libulfius.so simple_example.o
	$(CC) -o simple_example simple_example.o $(LIBS)

test: simple_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./simple_example

static: simple_example.o
	$(CC) -o simple_example simple_example.o $(PREFIX)/lib/liborcania.a $(PREFIX)/lib/libyder.a $(PREFIX)/lib/libulfius.a -ljansson -lmicrohttpd -lpthread -lgnutls
