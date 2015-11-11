#
# Example program
#
# Makefile used to build all programs
#
# Copyright 2014-2015 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU GENERAL PUBLIC LICENSE for more details.
#

LIBULFIUS_LOCATION=./src
SIMPLE_EXAMPLE_LOCATION=./examples/simple_example
SHEEP_COUNTER_LOCATION=./examples/sheep_counter
PROXY_EXAMPLE_LOCATION=./examples/proxy_example
REQUEST_EXAMPLE_LOCATION=./examples/request_example

all: libulfius.so simple_example sheep_counter proxy_example request_example

debug:
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) debug

clean:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) clean
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(SHEEP_COUNTER_LOCATION) && $(MAKE) clean
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) clean

libulfius.so:
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE)

simple_example:
	cd $(LIBULFIUS_LOCATION) && $(MAKE)

sheep_counter:
	cd $(SHEEP_COUNTER_LOCATION) && $(MAKE)

proxy_example:
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE)

request_example:
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE)
