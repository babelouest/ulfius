SHELL := /bin/bash # Use bash syntax
CC=gcc
ROOT = $(shell pwd)
CONF_FOLDER = $(ROOT)/conf
GEN_FOLDER = $(ROOT)/generated

ULFIUS_INCLUDE=../../include
ULFIUS_LOCATION=../../src
CERT_LOCATION=../cert

KEY_FILE=server.key
CERT_FILE=server.crt

CFLAGS+=-Wall -I$(ULFIUS_INCLUDE) $(ADDITIONALFLAGS) $(CPPFLAGS)
LIBS=-lc -lulfius -lorcania -lyder -lpthread $(LYDER) -L$(ULFIUS_LOCATION)

clean:
	rm -f ulfius_ws_echo_server ulfius_ws_echo_client ulfius_ws_echo_server.log ulfius_ws_echo_client.log $(CONF_FOLDER)/$(KEY_FILE) $(CONF_FOLDER)/$(CERT_FILE)
	-docker rmi -f pypy:2 crossbario/autobahn-testsuite:latest ulfius/autobahn:base ulfius/autobahn:server ulfius/autobahn:client-http ulfius/autobahn:client-https
	-docker system prune -f

test: autobahn-server autobahn-client-http

all: test

$(ULFIUS_LOCATION)/libulfius.so:
	cd $(ULFIUS_LOCATION) && $(MAKE) debug $*

ulfius_ws_echo_server: ulfius_ws_echo_server.c $(ULFIUS_LOCATION)/libulfius.so
	$(CC) -o ulfius_ws_echo_server ulfius_ws_echo_server.c $(CFLAGS) $(LIBS)

ulfius_ws_echo_client: ulfius_ws_echo_client.c $(ULFIUS_LOCATION)/libulfius.so
	$(CC) -o ulfius_ws_echo_client ulfius_ws_echo_client.c $(CFLAGS) $(LIBS)

docker-autobahn:
	docker build -t ulfius/autobahn:base -f Dockerfile .

docker-autobahn-server: docker-autobahn
	docker build -t ulfius/autobahn:server -f Dockerfile.server .

docker-autobahn-client-http: docker-autobahn
	docker build -t ulfius/autobahn:client-http -f Dockerfile.client-http .

docker-autobahn-client-https: docker-autobahn
	docker build -t ulfius/autobahn:client-https -f Dockerfile.client-https .

autobahn-server: docker-autobahn-server ulfius_ws_echo_server
	#rm -rf $(GEN_FOLDER)/autobahn/server
	#./ulfius_ws_echo_server $(CERT_LOCATION)/$(KEY_FILE) $(CERT_LOCATION)/$(CERT_FILE) & echo $$! > ulfius_ws_echo_server.pid
	#sleep 2
	#docker run -it --rm \
	#--network="host" \
	#  -v ${CONF_FOLDER}:/config \
	#  -v ${GEN_FOLDER}:/generated \
	#  -p 9001:9001 \
	#  --name fuzzingclient \
	#  ulfius/autobahn:server
	#kill `cat ulfius_ws_echo_server.pid` && rm -f ulfius_ws_echo_server.pid
	@echo
	@echo ===============================================================
	@echo = MANUAL RUN UNTIL I FIGURE OUT HOW TO RUN THIS AUTOMATICALLY =
	@echo ===============================================================
	@echo "To run Autobahn websocket server tests, execute the following commands"
	@echo "Remove ./generated/server directory if exists"
	@echo "In a console, run ulfius_ws_echo_server using https certificate in test/cert"
	@echo '$$ ./ulfius_ws_echo_server ../cert/server.key ../cert/server.crt'
	@echo "In another console, run docker image ulfius/autobahn:server to execute the tests"
	@echo '$$ docker run -it --rm --network="host" -v $${PWD}/conf:/config -v $${PWD}/generated:/generated -p 9001:9001 --name fuzzingclient ulfius/autobahn:server'

autobahn-client-http: docker-autobahn-client-http ulfius_ws_echo_client
	#rm -rf $(GEN_FOLDER)/autobahn/client-http
	#docker run -d --rm \
	#  -v ${CONF_FOLDER}:/config \
	#  -v ${GEN_FOLDER}:/generated \
	#  -p 9001:9001 \
	#  --name client_http \
	#  ulfius/autobahn:client-http
	#sleep 5
	#I=1 ; while [[ $$I -le 10 ]] ; do \
	#	echo "Run test case $$I"; \
	#	./ulfius_ws_echo_client $$I; \
	#	((I = I + 1)) ; \
	#done;
	#uwsc -s -q -i http://localhost:9001/updateReports?agent=ulfius
	#docker kill client_http
	@echo
	@echo ===============================================================
	@echo = MANUAL RUN UNTIL I FIGURE OUT HOW TO RUN THIS AUTOMATICALLY =
	@echo ===============================================================
	@echo "To run Autobahn websocket client HTTP tests, execute the following commands"
	@echo "Remove ./generated/client-http directory if exists"
	@echo "In a console, run docker image ulfius/autobahn:client-http to start the websocket server test"
	@echo '$$ docker run -it --rm -v $${PWD}/conf:/config -v $${PWD}/generated:/generated -p 9001:9001 --name client_http ulfius/autobahn:client-http'
	@echo "In another console, run ulfius_ws_echo_client with the test case number as argument (between 1 and 519)"
	@echo "To run all test cases run the following command"
	@echo '$$ for I in {1..519}; do echo Run test $$I && ./ulfius_ws_echo_client $$I; done'
	@echo "After executing all test cases, generate the report with the uwsc command:"
	@echo '$$ uwsc -s -q -i http://localhost:9001/updateReports?agent=ulfius'
	@echo "Then close the websocket server"
	@echo "docker kill client_http"

autobahn-client-https: docker-autobahn-client-https ulfius_ws_echo_client
	#rm -rf $(GEN_FOLDER)/autobahn/client-https
	#if [[ ! -f $(CONF_FOLDER)/$(KEY_FILE) ]]; then \
	#	cp $(CERT_LOCATION)/$(KEY_FILE) $(CONF_FOLDER); \
	#fi
	#if [[ ! -f $(CONF_FOLDER)/$(CERT_FILE) ]]; then \
	#	cp $(CERT_LOCATION)/$(CERT_FILE) $(CONF_FOLDER); \
	#fi
	#docker run -d --rm \
	#  -v ${CONF_FOLDER}:/config \
	#  -v ${GEN_FOLDER}:/generated \
	#  -p 9001:9001 \
	#  --name client_https \
	#  ulfius/autobahn:client-https
	#sleep 5
	#I=1 ; while [[ $$I -le 519 ]] ; do \
	#	echo "Run test case $$I"; \
	#	./ulfius_ws_echo_client $$I -https; \
	#	((I = I + 1)) ; \
	#done;
	#uwsc -s -q -i https://localhost:9001/updateReports?agent=ulfius
	#docker kill client_https
	@echo
	@echo ===============================================================
	@echo = MANUAL RUN UNTIL I FIGURE OUT HOW TO RUN THIS AUTOMATICALLY =
	@echo ===============================================================
	@echo "To run Autobahn websocket client HTTPS tests, execute the following commands"
	@echo "Remove ./generated/client-https directory if exists"
	@echo "Copy test/cert/server.* files to test/autobahn/conf/"
	@echo '$$ cp ../cert/server.* conf/'
	@echo "In a console, run docker image ulfius/autobahn:client-https to start the websocket server test"
	@echo '$$ docker run -it --rm -v $${PWD}/conf:/config -v $${PWD}/generated:/generated -p 9001:9001 --name client_https ulfius/autobahn:client-https'
	@echo "In another console, run ulfius_ws_echo_client with the test case number as argument (between 1 and 519) and the argument -https"
	@echo "To run all test cases run the following command"
	@echo '$$ for I in {1..519}; do echo Run test $$I && ./ulfius_ws_echo_client $$I -https; done'
	@echo "After executing all test cases, generate the report with the uwsc command:"
	@echo '$$ uwsc -s -q -i https://localhost:9001/updateReports?agent=ulfius'
	@echo "Then close the websocket server"
	@echo "docker kill client_https"
