#
# Ulfius Framework
#
# Makefile used to build the software
#
# Copyright 2015-2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation;
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this library.	If not, see <http://www.gnu.org/licenses/>.
#

LIBORCANIA_LOCATION=../lib/orcania/src
LIBYDER_LOCATION=../lib/yder/src
ULFIUS_INCLUDE=../include/
PREFIX=/usr/local
CC=gcc
CFLAGS=-c -pedantic -std=gnu99 -fPIC -Wall -D_REENTRANT -I$(PREFIX)/include -I$(ULFIUS_INCLUDE) -I$(LIBORCANIA_LOCATION) -I$(LIBYDER_LOCATION) $(ADDITIONALFLAGS) $(JANSSONFLAG) $(CURLFLAG) $(WEBSOCKETFLAG)
LIBS=-L$(PREFIX)/lib -L$(LIBORCANIA_LOCATION) -L$(LIBYDER_LOCATION) -lc -lmicrohttpd -lyder -lorcania -lpthread
OUTPUT=libulfius.so
VERSION=2.2.4

ifndef JANSSONFLAG
LJANSSON=-ljansson
endif

ifndef CURLFLAG
LCURL=-lcurl
endif

ifndef WEBSOCKETFLAG
LWEBSOCKET=-lgnutls
endif

all: release

libulfius.so: ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o
	$(CC) -shared -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o $(LIBS) $(LJANSSON) $(LCURL) $(LWEBSOCKET)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

libulfius.a: ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o
	ar rcs libulfius.a ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o

ulfius.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h ulfius.c
	$(CC) $(CFLAGS) ulfius.c

u_map.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h u_map.c
	$(CC) $(CFLAGS) u_map.c

u_request.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h u_request.c
	$(CC) $(CFLAGS) u_request.c

u_response.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h u_response.c
	$(CC) $(CFLAGS) u_response.c

u_websocket.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h u_websocket.c
	$(CC) $(CFLAGS) u_websocket.c

u_send_request.o: $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h u_send_request.c
	$(CC) $(CFLAGS) u_send_request.c

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).*

install: all
	cp $(OUTPUT).$(VERSION) $(PREFIX)/lib
	cp ulfius.h $(PREFIX)/include
	-ldconfig

static-install: static
	cp libulfius.a $(PREFIX)/lib
	cp $(ULFIUS_INCLUDE)/ulfius.h $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/$(OUTPUT) libulfius.a
	rm -f $(PREFIX)/lib/$(OUTPUT).*
	rm -f $(PREFIX)/include/ulfius.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libulfius.so

release: ADDITIONALFLAGS=-O3

release: libulfius.so

static: ADDITIONALFLAGS=-O3

static: libulfius.a

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libulfius.a
