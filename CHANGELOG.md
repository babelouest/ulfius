# Ulfius Changelog

## 2.6.7

- Check header property case insensitive in websocket client
- Add libcurl option `CURLOPT_NOPROGRESS` in `ulfius_send_http_streaming_request`
- Add `ulfius_start_framework_with_mhd_options` for expert mode

## 2.6.6

- Update doc generation
- Fix jansson memoy management bug

## 2.6.5

- Fix build on MinGW-w64
- Allow `NULL` values on `struct _u_map`
- Add function `ulfius_send_smtp_rich_email` to send e-mails with a specified content-type
- Fix and improve `ulfius_add_endpoint_list`
- Add doxygen documentation
- Add `follow_redirect` in `struct _u_request`
- Fix certificate check #154

## 2.6.4

- Add precision for chunked response, got the inspiration from #132
- Update access token for oauth2 bearer validation callback function, add precision concerning libjwt, fix #133
- Update callback_check_glewlwyd_access_token to the up-to-date version
- Various small fixes

## 2.6.3

- Fix warning appeared with gcc 9.1, fixes #128
- Make `instance->mhd_response_copy_data` useless if MHD>=0.9.61
- Fix `MHD_start_daemon` flag to reuse `MHD_USE_THREAD_PER_CONNECTION` by default, fix #131, thanks laf0rge!

## 2.6.2

- Clean build process
- Fix memory leak in `ulfius_set_string_body_request` and `ulfius_set_string_body_response`
- Call callback function websocket_onclose_callback on all times, even if the websocket hasn't properly worked, so the calling program can avoid memory leak and broken resources, fix #126

## 2.6.1

- Fix package dependencies in cmake script
- Fix core test to skip websocket tests if webscket is disabled
- Disable ipv6 capabilities if libmicrohttpd is older than 0.9.52
- Small bugfixes

## 2.6.0

- Add `struct _u_request->callback_position` to know the position of the current callback in the callback list
- Use `MHD_USE_AUTO` instead of `MHD_USE_THREAD_PER_CONNECTION` if `libmicrohttpd` is newer then 0.9.52
- Add `network_type` in `struct _u_instance` and `struct _u_request` to specify IPV4, IPV6 or both networks
- Add `check_server_certificate_flag`, `check_proxy_certificate`, `check_proxy_certificate_flag` and `ca_path` in `struct _u_request` to add more precision and control on SSL verification in `u_send_request`
- Add functions `ulfius_set_string_body_request`, `ulfius_set_binary_body_request`, `ulfius_set_empty_body_request`
- Add `url_path` in `struct _u_request` to store the url path only, without query parameters
- Add `ulfius_url_decode` and `ulfius_url_encode`
- Clean code, add more tests
- Install pkgconfig file when using Makefile
- Fix #121 where websockets messages of 126 or 127 bytes long made errors
- Use `gnutls_rnd()` instead of `rand()`
- Fix sneaky bug where endpoint injection inside a endpoint callback can lead to wrong callback calls, or even worse, crashes

## 2.5.5

- Fix #121 where websockets messages of 126 or 127 bytes long made errors
- Fix sneaky bug where endpoint injection inside a endpoint callback can lead to wrong callback calls, or even worse, crashes

## 2.5.3

- Add flag to build Ulfius with GnuTLS support but without Websockets
- Improve Travis CI script
- Fix CMake build process that didn't obviously linked Ulfius with pthreads
- Add command to run tests with valgrind

## 2.5.2

- Fix utf8 check on NULL value

## 2.5.1

- Fix uwsc crash on some systems

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
