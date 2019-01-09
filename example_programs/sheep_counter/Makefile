#
# Sheep counter program
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
CC=gcc
ULFIUS_LOCATION=../../src
ULFIUS_INCLUDE=../../include
EXAMPLE_INCLUDE=../include
CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) -I$(EXAMPLE_INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-lc -lulfius -lorcania -ljansson -L$(ULFIUS_LOCATION)

ifndef YDERFLAG
LIBS+= -lyder
endif

all: sheep_counter

clean:
	rm -f *.o sheep_counter

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: sheep_counter

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET GNUTLSFLAG=-DU_DISABLE_GNUTLS

sheep_counter.o: sheep_counter.c
	$(CC) $(CFLAGS) sheep_counter.c -DDEBUG -g -O0

sheep_counter: ../../src/libulfius.so sheep_counter.o
	$(CC) -o sheep_counter sheep_counter.o $(LIBS)

test: sheep_counter
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./sheep_counter
