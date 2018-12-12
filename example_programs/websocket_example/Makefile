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
ULFIUS_LOCATION=../../src
ULFIUS_INCLUDE=../../include
EXAMPLE_INCLUDE=../include
CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) -I$(EXAMPLE_INCLUDE) -I$(STATIC_FILE_LOCATION) $(ADDITIONALFLAGS)
STATIC_FILE_LOCATION=../../example_callbacks/static_file
LIBS=-lc -lulfius -lorcania -L$(ULFIUS_LOCATION)
#SECUREFLAG=-https test.key test.pem

ifndef YDERFLAG
LIBS+= -lyder
endif

all: websocket_server websocket_client

clean:
	rm -f *.o websocket_server websocket_client valgrind_server.txt valgrind_client.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: websocket_server websocket_client

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON

static_file_callback.o: $(STATIC_FILE_LOCATION)/static_file_callback.c
	$(CC) $(CFLAGS) $(STATIC_FILE_LOCATION)/static_file_callback.c

websocket_server: ../../src/libulfius.so static_file_callback.o websocket_server.c
	$(CC) $(CFLAGS) websocket_server.c
	$(CC) -o websocket_server websocket_server.o static_file_callback.o $(LIBS)

websocket_client: ../../src/libulfius.so websocket_client.c
	$(CC) $(CFLAGS) websocket_client.c 
	$(CC) -o websocket_client websocket_client.o $(LIBS)

test_server: websocket_server
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_server $(SECUREFLAG)

test_client: websocket_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket_client $(SECUREFLAG)

memcheck_server: websocket_server
	LD_LIBRARY_PATH=../../src valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./websocket_server $(SECUREFLAG) 2>valgrind_server.txt

memcheck_client: websocket_client
	LD_LIBRARY_PATH=../../src valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all ./websocket_client $(SECUREFLAG) 2>valgrind_client.txt
