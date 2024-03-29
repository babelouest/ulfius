#
# Public domain, no copyright. Use at your own risk.
#

ULFIUS_INCLUDE=../include
ULFIUS_LOCATION=../src
ULFIUS_EXAMPLE_CALLBACK_COMPRESS=../example_callbacks/http_compression
ULFIUS_EXAMPLE_CALLBACK_FILE=../example_callbacks/static_compressed_inmemory_website
ULFIUS_LIBRARY=$(ULFIUS_LOCATION)/libulfius.so
ULFIUS_SCRUTINIZE=$(ULFIUS_INCLUDE)/ulfius.h $(ULFIUS_INCLUDE)/u_private.h $(ULFIUS_INCLUDE)/yuarel.h $(ULFIUS_LOCATION)/ulfius.c $(ULFIUS_LOCATION)/u_map.c $(ULFIUS_LOCATION)/u_request.c $(ULFIUS_LOCATION)/u_response.c $(ULFIUS_LOCATION)/u_send_request.c $(ULFIUS_LOCATION)/u_websocket.c $(ULFIUS_LOCATION)/yuarel.c
CC=gcc
CFLAGS+=-Wall -Werror -Wextra -D_REENTRANT -I$(ULFIUS_INCLUDE) -DDEBUG -g -O0 $(CPPFLAGS)
LDFLAGS=-lc -L$(ULFIUS_LOCATION) -lulfius $(shell pkg-config --libs liborcania) $(shell pkg-config --libs libyder) $(shell pkg-config --libs check) $(shell pkg-config --libs gnutls) $(shell pkg-config --libs libcurl) $(shell pkg-config --libs jansson) -lz -lpthread
# Use this LDFLAGS below if you don't have/need gnutls
#LDFLAGS=-lc -lorcania -lyder -lulfius -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)
# Use this LDFLAGS below if you use yder logs
#LDFLAGS=-lc -lorcania -lyder -lulfius -lgnutls -lcheck -lpthread -lm -lrt -lsubunit -L$(ULFIUS_LOCATION)
VALGRIND_COMMAND=valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all
TARGET=u_map core framework websocket example_callbacks
VERBOSE=0
MEMCHECK=0
CERT=cert

all: test

clean:
	rm -f *.o $(TARGET) valgrind-*.txt *.log $(CERT)/*.crt $(CERT)/*.key

$(CERT)/server.key:
	./$(CERT)/create-cert.sh

$(ULFIUS_LIBRARY): $(ULFIUS_SCRUTINIZE)
	cd $(ULFIUS_LOCATION) && $(MAKE) debug

%: %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

http_compression_callback.o: $(ULFIUS_EXAMPLE_CALLBACK_COMPRESS)/http_compression_callback.c $(ULFIUS_EXAMPLE_CALLBACK_COMPRESS)/http_compression_callback.h
	$(CC) -c $(CFLAGS) -Wconversion $(ULFIUS_EXAMPLE_CALLBACK_COMPRESS)/http_compression_callback.c

static_compressed_inmemory_website_callback.o: $(ULFIUS_EXAMPLE_CALLBACK_FILE)/static_compressed_inmemory_website_callback.c $(ULFIUS_EXAMPLE_CALLBACK_FILE)/static_compressed_inmemory_website_callback.h
	$(CC) -c $(CFLAGS) -Wconversion $(ULFIUS_EXAMPLE_CALLBACK_FILE)/static_compressed_inmemory_website_callback.c

example_callbacks: example_callbacks.c http_compression_callback.o static_compressed_inmemory_website_callback.o
	$(CC) -I$(ULFIUS_EXAMPLE_CALLBACK_COMPRESS) -I$(ULFIUS_EXAMPLE_CALLBACK_FILE) example_callbacks.c http_compression_callback.o static_compressed_inmemory_website_callback.o -o example_callbacks $(LDFLAGS) $(CFLAGS)

test: $(ULFIUS_LIBRARY) $(CERT)/server.key $(TARGET) test_u_map test_core test_framework test_websocket test_example_callbacks

test_%: % $(ULFIUS_LIBRARY) $(CERT)/server.key
	@if [ "$(VERBOSE)" = "0" ] && [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./run_test.sh ./$^; \
	elif [ "$(MEMCHECK)" = "0" ]; then \
		LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} ./$^ ; \
	else \
		LD_LIBRARY_PATH=$(ULFIUS_LOCATION):${LD_LIBRARY_PATH} $(VALGRIND_COMMAND) ./$^ 2>valgrind-$@.txt; \
	fi

check: test
