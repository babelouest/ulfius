#
# Ulfius library
#
# Makefile used to build the tests
#
# Copyright 2017 Nicolas Mora <mail@babelouest.org>
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

ULFIUS_INCLUDE=../include
ULFIUS_LOCATION=../src
ULFIUS_LIBRARY=$(ULFIUS_LOCATION)/libulfius.so
CC=gcc
CFLAGS+=-Wall -D_REENTRANT -I$(ULFIUS_INCLUDE) -DDEBUG -g -O0 $(CPPFLAGS)
LIBS=-lc -lorcania -lulfius -lgnutls -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)
# Use this LIBS below if you don't have/need gnutls
#LIBS=-lc -lorcania -lyder -lulfius -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)
# Use this LIBS below if you use yder logs
#LIBS=-lc -lorcania -lyder -lulfius -lgnutls -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)
VALGRIND_COMMAND=valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all

all: test

clean:
	rm -f *.o u_map core framework websocket valgrind-*.txt

$(ULFIUS_LIBRARY):
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

u_map: u_map.c
	$(CC) $(CFLAGS) u_map.c -o u_map $(LIBS)

test_u_map: $(ULFIUS_LIBRARY) u_map
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./u_map

core: core.c
	$(CC) $(CFLAGS) core.c -o core $(LIBS)

test_core: $(ULFIUS_LIBRARY) core
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./core

framework: framework.c
	$(CC) $(CFLAGS) framework.c -o framework $(LIBS)

test_framework: $(ULFIUS_LIBRARY) framework
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./framework

websocket: websocket.c
	$(CC) $(CFLAGS) websocket.c -o websocket $(LIBS)

test_websocket: $(ULFIUS_LIBRARY) websocket
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./websocket

test: test_u_map test_core test_framework test_websocket

check: test

memcheck: $(ULFIUS_LIBRARY) u_map core framework websocket
	-CK_FORK=no LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./u_map 2>valgrind-u_map.txt
	-CK_FORK=no LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./core 2>valgrind-core.txt
	# test framework seems not to work properly when CK_FORK=no is used, so using it old school with forks then
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./framework 2>valgrind-framework.txt
	-CK_FORK=no LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./websocket 2>valgrind-websocket.txt
