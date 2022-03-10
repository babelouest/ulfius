#
# uwsc - Ulfius Websocket client
#
# Makefile used to build the software
#
# Copyright 2018-2022 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU GENERAL PUBLIC LICENSE
# License as published by the Free Software Foundation;
# version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
CC=gcc
ULFIUS_INCLUDE=../../include
ULFIUS_LOCATION=../../src
DESTDIR=/usr/local

ifndef YDERFLAG
LYDER=-lyder
endif

CFLAGS+=-c -Wall -I$(ULFIUS_INCLUDE) $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-lc -lulfius -lorcania $(LYDER) -L$(ULFIUS_LOCATION)

REMOTE_URL=http://localhost:9275/websocket/echo
COMMAND=./uwsc --output-log-file out.log $(REMOTE_URL)

all: uwsc

clean:
	rm -f *.o uwsc valgrind.txt out.log

debug: ADDITIONALFLAGS=-DDEBUG -g -O0

debug: uwsc

../../src/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug JANSSONFLAG=-DU_DISABLE_JANSSON CURLFLAG=-DU_DISABLE_CURL

uwsc: ../../src/libulfius.so uwsc.c
	$(CC) $(CFLAGS) uwsc.c
	$(CC) -o uwsc uwsc.o $(LIBS)

test: uwsc
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(COMMAND)

memcheck: uwsc
	LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all $(COMMAND) 2>valgrind.txt

install: uwsc
	mkdir -p $(DESTDIR)/bin $(DESTDIR)/man/man1/
	install -m755 uwsc $(DESTDIR)/bin
	install -m644 uwsc.1 $(DESTDIR)/man/man1/

uninstall:
	rm -f $(DESTDIR)/bin/uwsc

manpage:
	help2man ./uwsc -s 1 -n "Command-line websocket client" > uwsc.1
