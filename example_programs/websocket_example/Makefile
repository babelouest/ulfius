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
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL

static_file_callback.o: $(STATIC_FILE_LOCATION)/static_file_callback.c
	$(CC) $(CFLAGS) $(STATIC_FILE_LOCATION)/static_file_callback.c

websocket_example.o: websocket_example.c
	$(CC) $(CFLAGS) websocket_example.c

websocket_example: libulfius.so static_file_callback.o websocket_example.o
	$(CC) -o websocket_example websocket_example.o static_file_callback.o $(LIBS)

websocket_client: libulfius.so websocket_client.c
	$(CC) -o websocket_client websocket_client.c $(LIBS)

test: websocket_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_example

test_client: websocket_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_client

memcheck: websocket_example
	LD_LIBRARY_PATH=../../src valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./websocket_example 2>valgrind.txt
