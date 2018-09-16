#
# uwsc - Ulfius Websocket client
#
# Makefile used to build the software
#
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
CC=gcc
ULFIUS_INCLUDE=../include
ULFIUS_LOCATION=../src

CFLAGS=-c -Wall -I$(ULFIUS_INCLUDE) $(ADDITIONALFLAGS)
LIBS=-lc -lulfius -lyder -lorcania -L$(ULFIUS_LOCATION)

COMMAND=./uwsc --output-log-file=out.log http://localhost:9275/websocket/echo

all: uwsc

clean:
	rm -f *.o uwsc valgrind.txt out.log

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: uwsc

uwsc: uwsc.c
	$(CC) $(CFLAGS) uwsc.c
	$(CC) -o uwsc uwsc.o $(LIBS)

test: uwsc
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(COMMAND)

memcheck: uwsc
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all  $(COMMAND) 2>valgrind.txt
