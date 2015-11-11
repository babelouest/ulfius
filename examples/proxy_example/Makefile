#
# Example program
#
# Makefile used to build the software
#
# Copyright 2015 Nicolas Mora <mail@babelouest.org>
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

all: proxy

clean:
	rm -f *.o proxy

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: proxy

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

proxy.o: proxy.c
	$(CC) $(CFLAGS) proxy.c -DDEBUG -g -O0

proxy: libulfius.so proxy.o
	$(CC) -o proxy proxy.o $(LIBS)

test: proxy
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./proxy
