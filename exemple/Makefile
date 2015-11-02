#
# Ulfius Framework
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
LIBS=-lc -lulfius -ljansson -L../src

all: example

clean:
	rm -f *.o example

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: example

ulfius.so:
	cd ../src && $(MAKE) debug

example.o: example.c ulfius.so
	$(CC) $(CFLAGS) example.c -DDEBUG -g -O0

example: example.o
	$(CC) -o example example.o $(LIBS)

test: example
	LD_LIBRARY_PATH=../src:${LD_LIBRARY_PATH} ./example
