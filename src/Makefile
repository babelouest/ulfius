#
# Ulfius Framework
#
# Makefile used to build the software
#
# Copyright 2015-2018 Nicolas Mora <mail@babelouest.org>
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

ULFIUS_INCLUDE=../include
CONFIG_TEMPLATE=$(ULFIUS_INCLUDE)/ulfius-cfg.h.in
CONFIG_FILE=$(ULFIUS_INCLUDE)/ulfius-cfg.h
DESTDIR=/usr/local
CC=gcc
CFLAGS+=-c -pedantic -std=gnu99 -fPIC -Wall -D_REENTRANT -I$(ULFIUS_INCLUDE) $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(DESTDIR)/lib -lc -lmicrohttpd -lyder -lorcania -lpthread $(LDFLAGS)
OUTPUT=libulfius.so
VERSION=2.5.0

ifndef JANSSONFLAG
DISABLE_JANSSON=0
LJANSSON=-ljansson
else
DISABLE_JANSSON=1
endif

ifndef CURLFLAG
DISABLE_CURL=0
LCURL=-lcurl
else
DISABLE_CURL=1
endif

ifndef WEBSOCKETFLAG
DISABLE_WEBSOCKET=0
LWEBSOCKET=-lgnutls
else
DISABLE_WEBSOCKET=1
endif

all: release

$(CONFIG_FILE):
	@cp $(CONFIG_TEMPLATE) $(CONFIG_FILE)
	@sed -i -e 's/$${PROJECT_VERSION}/$(VERSION)/g' $(CONFIG_FILE)        
	@if [ "$(DISABLE_JANSSON)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_JANSSON/\#define U_DISABLE_JANSSON/g' $(CONFIG_FILE); \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_JANSSON/\/* #undef U_DISABLE_JANSSON *\//g' $(CONFIG_FILE); \
	fi
	@if [ "$(DISABLE_CURL)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_CURL/\#define U_DISABLE_CURL/g' $(CONFIG_FILE); \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_CURL/\/* #undef U_DISABLE_CURL *\//g' $(CONFIG_FILE); \
	fi
	@pkg-config --atleast-version=0.9.53 libmicrohttpd; \
	if [ $$? -ne 0 ] || [ "$(DISABLE_WEBSOCKET)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_WEBSOCKET/\#define U_DISABLE_WEBSOCKET/g' $(CONFIG_FILE); \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_WEBSOCKET/\/* #undef U_DISABLE_WEBSOCKET *\//g' $(CONFIG_FILE); \
	fi
	@echo Config file $(CONFIG_FILE) generated

%.o: %.c $(CONFIG_FILE) $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h $(ULFIUS_INCLUDE)/yuarel.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $<

target: ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o yuarel.o

libulfius.so: ulfius.c u_map.c u_request.c u_response.c u_send_request.c u_websocket.c yuarel.c
	@$(MAKE) --no-print-directory target
	$(CC) -shared -fPIC -Wl,-soname,$(OUTPUT) -o $(OUTPUT).$(VERSION) ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o yuarel.o $(LIBS) $(LJANSSON) $(LCURL) $(LWEBSOCKET)
	ln -sf $(OUTPUT).$(VERSION) $(OUTPUT)

libulfius.a: ulfius.c u_map.c u_request.c u_response.c u_send_request.c u_websocket.c yuarel.c
	@$(MAKE) --no-print-directory target
	ar rcs libulfius.a ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o yuarel.o

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).* $(CONFIG_FILE)

install: all
	install $(OUTPUT).$(VERSION) $(DESTDIR)/lib
	install -m644 $(ULFIUS_INCLUDE)/ulfius.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include
	-ldconfig

static-install: static
	install libulfius.a $(DESTDIR)/lib
	install -m644 $(ULFIUS_INCLUDE)/ulfius.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include

uninstall:
	rm -f $(DESTDIR)/lib/$(OUTPUT) $(DESTDIR)/lib/libulfius.a
	rm -f $(DESTDIR)/lib/$(OUTPUT).*
	rm -f $(DESTDIR)/include/ulfius.h
	rm -f $(DESTDIR)/include/ulfius-cfg.h

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libulfius.so

release: ADDITIONALFLAGS=-O3

release: libulfius.so

static: ADDITIONALFLAGS=-O3

static: libulfius.a

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libulfius.a
