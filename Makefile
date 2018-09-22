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
UWSC_LOCATION=./uwsc
TESTS_LOCATION=./test

ifeq (($(JANSSONFLAG)),"")
ADD_JANSSONFLAG="JANSSONFLAG=-DU_DISABLE_JANSSON"
endif

ifeq (($(CURLFLAG)),"")
ADD_CURLFLAG="CURLFLAG=-DU_DISABLE_CURL"
endif

ifeq (($(WEBSOCKETFLAG)),"")
ADD_WEBSOCKETFLAG="WEBSOCKETFLAG=-DU_DISABLE_WEBSOCKET"
endif

all: libulfius.so
	cd $(EXAMPLES_LOCATION) && $(MAKE) debug

debug:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug $(ADD_JANSSONFLAG) $(ADD_CURLFLAG) $(ADD_WEBSOCKETFLAG)
	cd $(EXAMPLES_LOCATION) && $(MAKE) debug

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

libulfius.so:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) $(ADD_JANSSONFLAG) $(ADD_CURLFLAG) $(ADD_WEBSOCKETFLAG)
