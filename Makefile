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
EXAMPLES_LOCATION=./example_programs
UWSC_LOCATION=./tools/uwsc
TESTS_LOCATION=./test

all:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) $*
	cd $(EXAMPLES_LOCATION) && $(MAKE) $*
	cd $(UWSC_LOCATION) && $(MAKE) $*

debug:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug $*
	cd $(EXAMPLES_LOCATION) && $(MAKE) debug $*
	cd $(UWSC_LOCATION) && $(MAKE) debug $*

clean:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) clean
	cd $(EXAMPLES_LOCATION) && $(MAKE) clean
	cd $(UWSC_LOCATION) && $(MAKE) clean
	cd $(TESTS_LOCATION) && $(MAKE) clean

install:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) install
	cd $(UWSC_LOCATION) && $(MAKE) install

uninstall:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) uninstall
