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

all: websocket_server websocket_client

clean:
	rm -f websocket_server websocket_client valgrind_server.txt valgrind_client.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: websocket_server websocket_client

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL

static_file_callback.o: $(STATIC_FILE_LOCATION)/static_file_callback.c
	$(CC) $(CFLAGS) $(STATIC_FILE_LOCATION)/static_file_callback.c

websocket_server: libulfius.so static_file_callback.o websocket_server.c
	$(CC) $(CFLAGS) websocket_server.c
	$(CC) -o websocket_server websocket_server.o static_file_callback.o $(LIBS)

websocket_client: libulfius.so websocket_client.c
	$(CC) -o websocket_client websocket_client.c $(LIBS)

test_server: websocket_server websocket_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_server

test_client: websocket_client websocket_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_client

memcheck_server: websocket_server websocket_client
	LD_LIBRARY_PATH=../../src valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./websocket_server 2>valgrind_server.txt

memcheck_client: websocket_server websocket_client
	LD_LIBRARY_PATH=../../src valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./websocket_client 2>valgrind_client.txt
