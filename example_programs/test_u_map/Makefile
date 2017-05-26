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
CC=gcc
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-lc -lorcania -lulfius -lyder -L$(ULFIUS_LOCATION)

all: test_u_map

clean:
	rm -f *.o test_u_map

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: test_u_map

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

test_u_map.o: test_u_map.c
	$(CC) $(CFLAGS) test_u_map.c -DDEBUG -g -O0

test_u_map: libulfius.so test_u_map.o
	$(CC) -o test_u_map test_u_map.o $(LIBS)

test: test_u_map
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./test_u_map
