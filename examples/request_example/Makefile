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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
CC=gcc
CFLAGS=-c -Wall -D_REENTRANT $(ADDITIONALFLAGS)
ULFIUS_LOCATION=../../src
LIBS=-lc -lulfius -ljansson -lyder -lorcania -L$(ULFIUS_LOCATION)

all: client server mail

clean:
	rm -f *.o client server mail

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: client server mail

libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

client.o: client.c libulfius.so
	$(CC) $(CFLAGS) client.c -DDEBUG -g -O0

client: client.o
	$(CC) -o client client.o $(LIBS)

server.o: server.c libulfius.so
	$(CC) $(CFLAGS) server.c -DDEBUG -g -O0

server: server.o
	$(CC) -o server server.o $(LIBS)

mail.o: mail.c libulfius.so
	$(CC) $(CFLAGS) mail.c -DDEBUG -g -O0

mail: mail.o
	$(CC) -o mail mail.o $(LIBS)
