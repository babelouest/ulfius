#
# Example program
#
# Makefile used to build the software
#
# Copyright 2016 Nicolas Mora <mail@babelouest.org>
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
CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) -I$(EXAMPLE_INCLUDE) -D_REENTRANT $(ADDITIONALFLAGS)
LIBS=-lc -lorcania -lulfius -L$(ULFIUS_LOCATION)

ifndef YDERFLAG
LIBS+= -lyder
endif

all: stream_example stream_client

clean:
	rm -f *.o stream_example stream_client

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: stream_example stream_client

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET

stream_example.o: stream_example.c
	$(CC) $(CFLAGS) stream_example.c -DDEBUG -g -O0

stream_example: ../../src/libulfius.so stream_example.o
	$(CC) -o stream_example stream_example.o $(LIBS)

stream_client.o: stream_client.c
	$(CC) $(CFLAGS) stream_client.c -DDEBUG -g -O0

stream_client: ../../src/libulfius.so stream_client.o
	$(CC) -o stream_client stream_client.o $(LIBS)

test_example: stream_example
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./stream_example

test_client: stream_client
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./stream_client
