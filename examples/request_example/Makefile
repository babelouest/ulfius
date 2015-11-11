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
LIBS=-lc -lulfius -ljansson -L$(ULFIUS_LOCATION)

all: send_requests

clean:
	rm -f *.o send_requests

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: send_requests

libulfius.so: $(ULFIUS_LOCATION)/libulfius.so
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

send_requests.o: send_requests.c libulfius.so
	$(CC) $(CFLAGS) send_requests.c -DDEBUG -g -O0

send_requests: send_requests.o
	$(CC) -o send_requests send_requests.o $(LIBS)

test: send_requests
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./send_requests
