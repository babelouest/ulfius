#
# Websocket example program
#
# Makefile used to build the software
#
# Copyright 2017-2020 Nicolas Mora <mail@babelouest.org>
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
CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) -I$(EXAMPLE_INCLUDE) -I$(STATIC_FILE_LOCATION) -I$(STATIC_COMPRESSED_INMEMORY_WEBSITE) $(ADDITIONALFLAGS) $(CPPFLAGS)
STATIC_FILE_LOCATION=../../example_callbacks/static_file
STATIC_COMPRESSED_INMEMORY_WEBSITE=../../example_callbacks/static_compressed_inmemory_website
LIBS=-lc -lz -lpthread -ljansson -lulfius -lorcania -L$(ULFIUS_LOCATION)
LIBULFIUS=../../src/libulfius.so
#SECUREFLAG=-https test.key test.pem

ifndef YDERFLAG
LIBS+= -lyder
endif

all: websocket_server websocket_client

clean:
	rm -f *.o websocket_server websocket_client valgrind_server.txt valgrind_client.txt

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: websocket_server websocket_client

$(LIBULFIUS): $(ULFIUS_LOCATION)/ulfius.c $(ULFIUS_LOCATION)/u_map.c $(ULFIUS_LOCATION)/u_request.c $(ULFIUS_LOCATION)/u_response.c $(ULFIUS_LOCATION)/u_send_request.c $(ULFIUS_LOCATION)/u_websocket.c $(ULFIUS_LOCATION)/yuarel.c $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

static_file_callback.o: $(STATIC_FILE_LOCATION)/static_file_callback.c
	$(CC) $(CFLAGS) $(STATIC_FILE_LOCATION)/static_file_callback.c

static_compressed_inmemory_website_callback.o: $(STATIC_COMPRESSED_INMEMORY_WEBSITE)/static_compressed_inmemory_website_callback.c
	$(CC) $(CFLAGS) $(STATIC_COMPRESSED_INMEMORY_WEBSITE)/static_compressed_inmemory_website_callback.c

websocket_server: $(LIBULFIUS) static_compressed_inmemory_website_callback.o websocket_server.c
	$(CC) $(CFLAGS) websocket_server.c
	$(CC) -o websocket_server websocket_server.o static_compressed_inmemory_website_callback.o $(LIBS)

websocket_client: $(LIBULFIUS) websocket_client.c
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
