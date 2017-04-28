#
# Orcania library
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

CC=gcc
CFLAGS=-Wall -D_REENTRANT -DDEBUG -g -O0
ULFIUS_LOCATION=../src
LIBS=-lc -lorcania -lyder -lulfius -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)

all: test

clean:
	rm -f *.o u_map core valgrind.txt

liborcania.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

u_map: u_map.c
	$(CC) $(CFLAGS) u_map.c -o u_map $(LIBS)

test_u_map: liborcania.so u_map
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./u_map

core: core.c
	$(CC) $(CFLAGS) core.c -o core $(LIBS)

test_core: liborcania.so core
	-LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./core

test: test_u_map test_core
