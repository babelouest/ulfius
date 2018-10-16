#
# Example program
#
# Makefile used to build the software
#
# Copyright 2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
CC=gcc
ULFIUS_LOCATION=../../src
ULFIUS_INCLUDE=../../include
CFLAGS=-c -Wall -I$(ULFIUS_INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS)
LIBS=-L$(LIBYDER_LOCATION) -lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

all: multiple_callbacks_example

clean:
	rm -f *.o multiple_callbacks_example valgrind.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: multiple_callbacks_example

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

multiple_callbacks_example.o: multiple_callbacks_example.c
	$(CC) $(CFLAGS) multiple_callbacks_example.c -DDEBUG -g -O0

multiple_callbacks_example: ../../src/libulfius.so multiple_callbacks_example.o
	$(CC) -o multiple_callbacks_example multiple_callbacks_example.o $(LIBS)

test: multiple_callbacks_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./multiple_callbacks_example
