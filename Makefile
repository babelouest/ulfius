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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

LIBULFIUS_LOCATION=./src
SIMPLE_EXAMPLE_LOCATION=./examples/simple_example
SHEEP_COUNTER_LOCATION=./examples/sheep_counter
PROXY_EXAMPLE_LOCATION=./examples/proxy_example
REQUEST_EXAMPLE_LOCATION=./examples/request_example
TEST_U_MAP_LOCATION=./examples/test_u_map
INJECTION_EXAMPLE_LOCATION=./examples/injection_example
AUTH_EXAMPLE_LOCATION=./examples/auth_example
STREAM_EXAMPLE_LOCATION=./examples/stream_example

all: libulfius.so simple_example sheep_counter proxy_example request_example injection_example auth_example test_u_map

debug:
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(INJECTION_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(AUTH_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(STREAM_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(TEST_U_MAP_LOCATION) && $(MAKE) debug

clean:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) clean
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(SHEEP_COUNTER_LOCATION) && $(MAKE) clean
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(INJECTION_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(AUTH_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(STREAM_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(TEST_U_MAP_LOCATION) && $(MAKE) clean

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

injection_example:
	cd $(INJECTION_EXAMPLE_LOCATION) && $(MAKE)

stream_example:
	cd $(STREAM_EXAMPLE_LOCATION) && $(MAKE)

auth_example:
	cd $(AUTH_EXAMPLE_LOCATION) && $(MAKE)

test_u_map:
	cd $(TEST_U_MAP_LOCATION) && $(MAKE)
