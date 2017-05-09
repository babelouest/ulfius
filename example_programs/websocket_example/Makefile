#
# Websocket example program
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
CFLAGS=-c -Wall -I$(ULFIUS_LOCATION) -I$(STATIC_FILE_LOCATION) $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
STATIC_FILE_LOCATION=../../example_callbacks/static_file
LIBS=-lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

all: websocket_example

clean:
	rm -f *.o websocket_example valgrind.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: websocket_example

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug CURLFLAG=-DU_DISABLE_CURL

static_file_callback.o: $(STATIC_FILE_LOCATION)/static_file_callback.c
	$(CC) $(CFLAGS) $(STATIC_FILE_LOCATION)/static_file_callback.c -DDEBUG -g -O0

websocket_example.o: websocket_example.c
	$(CC) $(CFLAGS) websocket_example.c -DDEBUG -g -O0

websocket_example: libulfius.so static_file_callback.o websocket_example.o
	$(CC) -o websocket_example websocket_example.o static_file_callback.o $(LIBS)

test: websocket_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_example
