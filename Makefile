#
# Ulfius Framework
#
# Makefile used to build all programs
#
# Copyright 2014-2017 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

LIBULFIUS_LOCATION=./src
LIBORCANIA_LOCATION=lib/orcania/src
LIBYDER_LOCATION=lib/yder/src
EXAMPLES_LOCATION=./example_programs

all: libulfius.so

debug:
	cd $(LIBORCANIA_LOCATION) && $(MAKE) debug
	cd $(LIBYDER_LOCATION) && $(MAKE) debug
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug

clean:
	cd $(LIBORCANIA_LOCATION) && $(MAKE) clean
	cd $(LIBYDER_LOCATION) && $(MAKE) clean
	cd $(LIBULFIUS_LOCATION) && $(MAKE) clean
	cd $(EXAMPLES_LOCATION) && $(MAKE) clean

install:
	cd $(LIBORCANIA_LOCATION) && $(MAKE) install
	cd $(LIBYDER_LOCATION) && $(MAKE) install
	cd $(LIBULFIUS_LOCATION) && $(MAKE) install

uninstall:
	cd $(LIBORCANIA_LOCATION) && $(MAKE) uninstall
	cd $(LIBYDER_LOCATION) && $(MAKE) uninstall
	cd $(LIBULFIUS_LOCATION) && $(MAKE) uninstall

libulfius.so:
	cd $(LIBORCANIA_LOCATION) && $(MAKE)
	cd $(LIBYDER_LOCATION) && $(MAKE)
	cd $(LIBULFIUS_LOCATION) && $(MAKE)
