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
LIBS=-lc -lulfius -lyder -L$(ULFIUS_LOCATION)

all: auth_client auth_server

clean:
	rm -f *.o auth_client auth_server

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: auth_client auth_server

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

auth_client.o: auth_client.c libulfius.so
	$(CC) $(CFLAGS) auth_client.c -DDEBUG -g -O0

auth_client: auth_client.o
	$(CC) -o auth_client auth_client.o $(LIBS)

auth_server.o: auth_server.c libulfius.so
	$(CC) $(CFLAGS) auth_server.c -DDEBUG -g -O0

auth_server: auth_server.o
	$(CC) -o auth_server auth_server.o $(LIBS)
