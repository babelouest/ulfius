#
# injection_example program
#
# Makefile used to build the software
#
# Copyright 2016-2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
CC=gcc
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-lc -lulfius -lyder -ljansson -L$(ULFIUS_LOCATION)

all: injection_example

clean:
	rm -f *.o injection_example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: injection_example

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

injection_example.o: injection_example.c
	$(CC) $(CFLAGS) injection_example.c -DDEBUG -g -O0

injection_example: libulfius.so injection_example.o
	$(CC) -o injection_example injection_example.o $(LIBS)

test: injection_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./injection_example
