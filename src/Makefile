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

LIBORCANIA_LOCATION=../lib/orcania
LIBYDER_LOCATION=../lib/yder/src
PREFIX=/usr/local
CC=gcc
CFLAGS=-c -fPIC -Wall -D_REENTRANT -I$(PREFIX)/include -I$(LIBORCANIA_LOCATION) -I$(LIBYDER_LOCATION) $(ADDITIONALFLAGS) $(JANSSONFLAG) $(CURLFLAG)
LIBS=-L$(PREFIX)/lib -L$(LIBORCANIA_LOCATION) -L$(LIBYDER_LOCATION) -lc -lmicrohttpd -lyder -lorcania
OUTPUT=libulfius.so
VERSION=1.9

ifneq (($(JANSSONFLAG)),"")
LJANSSON=-ljansson
endif

ifneq (($(CURLFLAG)),"")
LCURL=-lcurl
endif

all: release

libulfius.so: ulfius.o u_map.o u_request.o u_response.o u_send_request.o
	$(CC) -shared -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) ulfius.o u_map.o u_request.o u_response.o u_send_request.o $(LIBS) $(LJANSSON) $(LCURL)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

ulfius.o: ulfius.h ulfius.c
	$(CC) $(CFLAGS) $(FLAGS_YDER) ulfius.c

u_map.o: ulfius.h u_map.c
	$(CC) $(CFLAGS) $(FLAGS_YDER) u_map.c

u_request.o: ulfius.h u_request.c
	$(CC) $(CFLAGS) $(FLAGS_YDER) u_request.c

u_response.o: ulfius.h u_response.c
	$(CC) $(CFLAGS) $(FLAGS_YDER) u_response.c

u_send_request.o: ulfius.h u_send_request.c
	$(CC) $(CFLAGS) $(FLAGS_YDER) u_send_request.c

clean:
	rm -f *.o *.so $(OUTPUT) $(OUTPUT).*

install: all
	cp $(OUTPUT).$(VERSION) $(PREFIX)/lib
	cp ulfius.h $(PREFIX)/include
	ldconfig -r $(PREFIX)

uninstall:
	rm -f $(PREFIX)/lib/$(OUTPUT)
	rm -f $(PREFIX)/lib/$(OUTPUT).*
	rm -f $(PREFIX)/include/ulfius.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libulfius.so

release: ADDITIONALFLAGS=-O3

release: libulfius.so
