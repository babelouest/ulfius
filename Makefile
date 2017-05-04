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

debug:
	cd $(LIBORCANIA_LOCATION) && $(MAKE) debug $(ADD_JANSSONFLAG)
	cd $(LIBYDER_LOCATION) && $(MAKE) debug
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug $(ADD_JANSSONFLAG) $(ADD_CURLFLAG) $(ADD_WEBSOCKETFLAG)

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
	cd $(LIBORCANIA_LOCATION) && $(MAKE) $(ADD_JANSSONFLAG)
	cd $(LIBYDER_LOCATION) && $(MAKE)
	cd $(LIBULFIUS_LOCATION) && $(MAKE) $(ADD_JANSSONFLAG) $(ADD_CURLFLAG) $(ADD_WEBSOCKETFLAG)
