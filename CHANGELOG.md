# Ulfius Changelog

## 2.5.0

- Add struct _u_endpoint.check_utf8 to check all request parameters and values to be valid utf8 strings
- Add client certificate authentication for webservice and send request library (issue #83)
- Fix build config file bug when using -jxx option to Makefile (issue #84)
- Allow to disable Yder logging library, to use Ulfius in embedded systems like FreeRTOS
  where console, syslog or journald are not available, and file logging is overkill
- Add support for FreeRTOS and LWIP, Thanks to Dirk Uhlemann
- Add support for SameSite attribute in response cookies (issue #88)

## 2.4.4

- CMake scripts improvements

## 2.4.3

- Add config file ulfius-cfg.h dynamically built with the options
- Adapt examples with new ulfius-cfg.h file

## 2.4.2

- Fix #79 where yuarel should be hidden from the outside world

## 2.4.1

- Fix #78 where gnutls is not required if websocket is disabled

## 2.4.0

- Fix Websocket fragmented messages
- Fix CMake script that installed Orcania twice
- Fix cppcheck warnings
- Add timeout for http connections
- Allow to use MHD_RESPMEM_MUST_COPY for different memory manager, fix #63
- Add websocket client framework
- Add uwsc - Ulfius WebSocket Client - A simple command-line websocket client program
- Add Travis CI
- Add RPM in CMake script package

## 2.3.8

- Fix CMake build when /usr/local is not present in default build path

## 2.3.7

- Improve documentation with summary
- Yet another websocket fix, this one was binary messages not properly handled
- At the same time, improve websocket_example to handle incoming binary messages

## 2.3.6

- Fix websocket bug that did not close a websocket properly after wrongly closed connections
- Add last example_callbacks versions
- Improve documentation on ulfius_get_json_body_request and ulfius_get_json_body_response

## 2.3.5

- Fix websocket bug that kept some connections open after being unproperly closed by the client

## 2.3.4

- Fix Makefile soname

## 2.3.3

- Add Debian hardening patch on Makefile

## 2.3.2

- Fix websocket_example that worked for Firefox onky due to minor bugs in websocket management and misunderstanding the RFC
- Update oauth2_bearer/glewlwyd_resource to handle client tokens

## 2.3.1

- Sync version number on all places it's located

## 2.3

- Add CMake installation script
- Various bugfixes

## 2.2.3

- Fix PTHREAD_MUTEX_RECURSIVE_NP bug

## 2.2.2

- Fix bug in websockets

## 2.2.1

- Added error informations inside ulfius_get_json_body_request function (#33)
- code cleaning
- small bugfixes

## 2.2

- Add large file upload support (#31)
- fix `upload_data_size` and memory consumption
- Make ldconfig harmless if not root (#26)
