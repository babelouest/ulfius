#
# Ulfius Framework
#
# Makefile used to build the software
#
# Copyright 2015-2020 Nicolas Mora <mail@babelouest.org>
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
PROJECT_NAME=ulfius
PROJECT_DESCRIPTION=Web Framework to build REST APIs, Webservices or any HTTP endpoint in C language. Can stream large amount of data, integrate JSON data with Jansson, and create websocket services
PROJECT_BUGREPORT_PATH=https://github.com/babelouest/ulfius/issues
PKGCONF_REQ=
PKGCONF_REQ_PRIVATE=
ULFIUS_INCLUDE=../include
CONFIG_TEMPLATE=$(ULFIUS_INCLUDE)/ulfius-cfg.h.in
CONFIG_FILE=$(ULFIUS_INCLUDE)/ulfius-cfg.h
PKGCONFIG_FILE=../libulfius.pc
PKGCONFIG_TEMPLATE=../libulfius.pc.in
DESTDIR=/usr/local
CC=gcc
CFLAGS+=-c -pedantic -std=gnu99 -fPIC -Wall -Werror -Wextra -D_REENTRANT -I$(ULFIUS_INCLUDE) $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-L$(DESTDIR)/lib -lc -lmicrohttpd -lorcania -lpthread $(LDFLAGS)
SONAME = -soname
ifeq ($(shell uname -s),Darwin)
	SONAME = -install_name
endif
OBJECTS=ulfius.o u_map.o u_request.o u_response.o u_send_request.o u_websocket.o yuarel.o
OUTPUT=libulfius.so
VERSION_MAJOR=2
VERSION_MINOR=6
VERSION_PATCH=9

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

ifndef GNUTLSFLAG
DISABLE_GNUTLS=0
LGNUTLS=-lgnutls
else
DISABLE_GNUTLS=1
WEBSOCKETFLAG=1
endif

ifndef WEBSOCKETFLAG
DISABLE_WEBSOCKET=0
else
DISABLE_WEBSOCKET=1
endif

ifndef YDERFLAG
DISABLE_YDER=0
LYDER=-lyder
else
DISABLE_YDER=1
endif

ifndef FREERTOSFLAG
WITH_FREERTOS=0
else
WITH_FREERTOS=1
endif

ifndef LWIPFLAG
WITH_LWIP=0
else
CFLAGS+= $(shell pkg-config --cflags lwip)
LIBS+= $(shell pkg-config --libs lwip)
WITH_LWIP=1
endif

ifeq ($(WITH_FREERTOS)$(WITH_LWIP),11)
$(error FREERTOSFLAG and LWIPFLAG can't be enabled together)
endif

.PHONY: all clean

all: release

$(CONFIG_FILE):
	@cp $(CONFIG_TEMPLATE) $(CONFIG_FILE)
	@echo Config file $(CONFIG_FILE) generated
	@sed -i -e 's/$${PROJECT_VERSION}/$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_MAJOR}/$(VERSION_MAJOR)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_MINOR}/$(VERSION_MINOR)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_PATCH}/$(VERSION_PATCH)/g' $(CONFIG_FILE)        
	@sed -i -e 's/$${PROJECT_VERSION_NUMBER}/$(shell printf '%02d' $(VERSION_MAJOR))$(shell printf '%02d' $(VERSION_MINOR))$(shell printf '%02d' $(VERSION_PATCH))/g' $(CONFIG_FILE)        
	@if [ "$(DISABLE_JANSSON)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_JANSSON/\#define U_DISABLE_JANSSON/g' $(CONFIG_FILE); \
		echo "JANSSON SUPPORT    DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_JANSSON/\/* #undef U_DISABLE_JANSSON *\//g' $(CONFIG_FILE); \
		echo "JANSSON SUPPORT    ENABLED"; \
	fi
	@if [ "$(DISABLE_CURL)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_CURL/\#define U_DISABLE_CURL/g' $(CONFIG_FILE); \
		echo "CURL SUPPORT       DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_CURL/\/* #undef U_DISABLE_CURL *\//g' $(CONFIG_FILE); \
		echo "CURL SUPPORT       ENABLED"; \
	fi
	@if [ "$(DISABLE_GNUTLS)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_GNUTLS/\#define U_DISABLE_GNUTLS/g' $(CONFIG_FILE); \
		echo "GNUTLS SUPPORT     DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_GNUTLS/\/* #undef U_DISABLE_GNUTLS *\//g' $(CONFIG_FILE); \
		echo "GNUTLS SUPPORT     ENABLED"; \
	fi
	@pkg-config --atleast-version=0.9.53 libmicrohttpd; \
	if [ $$? -ne 0 ] || [ "$(DISABLE_WEBSOCKET)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_WEBSOCKET/\#define U_DISABLE_WEBSOCKET/g' $(CONFIG_FILE); \
		echo "WEBSOCKET SUPPORT  DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_WEBSOCKET/\/* #undef U_DISABLE_WEBSOCKET *\//g' $(CONFIG_FILE); \
		echo "WEBSOCKET SUPPORT  ENABLED"; \
	fi
	@if [ "$(DISABLE_YDER)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_DISABLE_YDER/\#define U_DISABLE_YDER/g' $(CONFIG_FILE); \
		echo "YDER SUPPORT       DISABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_DISABLE_YDER/\/* #undef U_DISABLE_YDER *\//g' $(CONFIG_FILE); \
		echo "YDER SUPPORT       ENABLED"; \
	fi
	@if [ "$(WITH_FREERTOS)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_WITH_FREERTOS/\#define U_WITH_FREERTOS/g' $(CONFIG_FILE); \
		echo "FREERTOS SUPPORT   ENABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_WITH_FREERTOS/\/* #undef U_WITH_FREERTOS *\//g' $(CONFIG_FILE); \
		echo "FREERTOS SUPPORT   DISABLED"; \
	fi
	@if [ "$(WITH_LWIP)" = "1" ]; then \
		sed -i -e 's/\#cmakedefine U_WITH_LWIP/\#define U_WITH_LWIP/g' $(CONFIG_FILE); \
		echo "LWIP SUPPORT       ENABLED"; \
	else \
		sed -i -e 's/\#cmakedefine U_WITH_LWIP/\/* #undef U_WITH_LWIP *\//g' $(CONFIG_FILE); \
		echo "LWIP SUPPORT       DISABLED"; \
	fi

$(PKGCONFIG_FILE):
	@cp $(PKGCONFIG_TEMPLATE) $(PKGCONFIG_FILE)
	@echo Pkgconfig file $(PKGCONFIG_FILE) generated
	@sed -i -e 's#@CMAKE_INSTALL_PREFIX@#$(DESTDIR)#g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@CMAKE_INSTALL_LIBDIR@/lib/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@CMAKE_INSTALL_INCLUDEDIR@/include/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PROJECT_NAME@/$(PROJECT_NAME)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PROJECT_DESCRIPTION@/$(PROJECT_DESCRIPTION)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's|@PROJECT_BUGREPORT_PATH@|$(PROJECT_BUGREPORT_PATH)|g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@LIBRARY_VERSION@/$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PKGCONF_REQ@/$(PKGCONF_REQ)/g' $(PKGCONFIG_FILE)
	@sed -i -e 's/@PKGCONF_REQ_PRIVATE@/$(PKGCONF_REQ_PRIVATE)/g' $(PKGCONFIG_FILE)

target: $(OBJECTS)

%.o: %.c $(CONFIG_FILE) $(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h $(ULFIUS_INCLUDE)/yuarel.h
	$(CC) $(CFLAGS) $<

libulfius.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wl,$(SONAME),$(OUTPUT) -o $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OBJECTS) $(LIBS) $(LYDER) $(LJANSSON) $(LCURL) $(LGNUTLS)
	ln -sf $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(OUTPUT)

libulfius.a: $(OBJECTS)
	ar rcs libulfius.a $(OBJECTS)

clean:
	rm -f *.o *.so *.a $(OUTPUT) $(OUTPUT).* $(CONFIG_FILE) $(PKGCONFIG_FILE)

install: all $(PKGCONFIG_FILE)
	install $(OUTPUT).$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH) $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/lib/pkgconfig/
	install -m644 $(PKGCONFIG_FILE) $(DESTDIR)/lib/pkgconfig/
	install -m644 $(ULFIUS_INCLUDE)/ulfius.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include
	-ldconfig $(DESTDIR)/lib

static-install: static
	install libulfius.a $(DESTDIR)/lib
	install -m644 $(PKGCONFIG_FILE) $(DESTDIR)/lib/pkgconfig
	install -m644 $(ULFIUS_INCLUDE)/ulfius.h $(DESTDIR)/include
	install -m644 $(CONFIG_FILE) $(DESTDIR)/include

uninstall:
	rm -f $(DESTDIR)/lib/$(OUTPUT) $(DESTDIR)/lib/libulfius.a
	rm -f $(DESTDIR)/lib/$(OUTPUT).*
	rm -f $(DESTDIR)/include/ulfius.h
	rm -f $(DESTDIR)/include/$(CONFIG_FILE)
	rm -f $(DESTDIR)/lib/pkgconfig/$(PKGCONFIG_FILE)

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: libulfius.so

release: ADDITIONALFLAGS=-O3

release: libulfius.so

static: ADDITIONALFLAGS=-O3

static: libulfius.a

static-debug: ADDITIONALFLAGS=-DDEBUG -g -O0

static-debug: libulfius.a

