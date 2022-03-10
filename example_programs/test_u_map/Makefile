#
# Example program
#
# Makefile used to build the software
#
# Copyright 2014-2022 Nicolas Mora <mail@babelouest.org>
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
LIBS=-lc -lorcania -lulfius -L$(ULFIUS_LOCATION)

ifndef YDERFLAG
LIBS+= -lyder
endif

all: test_u_map

clean:
	rm -f *.o test_u_map

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: test_u_map

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=1 CURLFLAG=1 GNUTLSFLAG=1

test_u_map.o: test_u_map.c
	$(CC) $(CFLAGS) test_u_map.c -DDEBUG -g -O0

test_u_map: ../../src/libulfius.so test_u_map.o
	$(CC) -o test_u_map test_u_map.o $(LIBS)

test: test_u_map
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./test_u_map
