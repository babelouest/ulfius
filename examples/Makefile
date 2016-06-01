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

SIMPLE_EXAMPLE_LOCATION=./simple_example
SHEEP_COUNTER_LOCATION=./sheep_counter
PROXY_EXAMPLE_LOCATION=./proxy_example
REQUEST_EXAMPLE_LOCATION=./request_example
TEST_U_MAP_LOCATION=./test_u_map
INJECTION_EXAMPLE_LOCATION=./injection_example
AUTH_EXAMPLE_LOCATION=./auth_example
STREAM_EXAMPLE_LOCATION=./stream_example

all: debug

debug:
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(INJECTION_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(AUTH_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(STREAM_EXAMPLE_LOCATION) && $(MAKE) debug
	cd $(TEST_U_MAP_LOCATION) && $(MAKE) debug

clean:
	cd $(SIMPLE_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(SHEEP_COUNTER_LOCATION) && $(MAKE) clean
	cd $(PROXY_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(REQUEST_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(INJECTION_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(AUTH_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(STREAM_EXAMPLE_LOCATION) && $(MAKE) clean
	cd $(TEST_U_MAP_LOCATION) && $(MAKE) clean
