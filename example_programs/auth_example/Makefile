#
# Example program
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
CFLAGS=-c -Wall -I$(ULFIUS_INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS)
LIBS=-lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

all: auth_client auth_server

clean:
	rm -f *.o auth_client auth_server

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: auth_client auth_server

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

auth_client.o: auth_client.c libulfius.so
	$(CC) $(CFLAGS) auth_client.c -DDEBUG -g -O0

auth_client: auth_client.o
	$(CC) -o auth_client auth_client.o $(LIBS)

test_auth_client: auth_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./auth_client

auth_server.o: auth_server.c libulfius.so
	$(CC) $(CFLAGS) auth_server.c -DDEBUG -g -O0

auth_server: auth_server.o
	$(CC) -o auth_server auth_server.o $(LIBS)

test_auth_server: auth_server
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./auth_server
