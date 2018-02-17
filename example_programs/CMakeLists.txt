#
# Ulfius library
#
# CMake file used to build example programs
#
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the MIT License
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

cmake_minimum_required(VERSION 3.5)

project(ulfius_examples C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# library info

set(PROJECT_DESCRIPTION "HTTP Framework for REST API in C, using JSON or not, with websockets or not, with streaming data or not")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/ulfius/issues")

include(GNUInstallDirs)
include(CheckSymbolExists)

# cmake modules

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

# check if _GNU_SOURCE is available

if (NOT _GNU_SOURCE)
    check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

    if (NOT _GNU_SOURCE)
        unset(_GNU_SOURCE CACHE)
        check_symbol_exists(_GNU_SOURCE "features.h" _GNU_SOURCE)
    endif ()
endif ()

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif ()

include(FindOrcania)
set(ORCANIA_MIN_VERSION "1.1.1")
find_package(Orcania ${ORCANIA_MIN_VERSION} REQUIRED)
set(LIBS ${ORCANIA_LIBRARIES})

include(FindYder)
set(YDER_MIN_VERSION "1.1.1")
find_package(Yder ${YDER_MIN_VERSION} REQUIRED)
set(LIBS ${LIBS} ${YDER_LIBRARIES})

include(FindUlfius)
set(ULFIUS_MIN_VERSION "2.2")
find_package(Ulfius ${ULFIUS_MIN_VERSION} REQUIRED)
set(LIBS ${LIBS} ${ULFIUS_LIBRARIES})

# websocket support

option(WITH_WEBSOCKET "Websocket support" ON)

if (WIN32)
	set(WITH_WEBSOCKET OFF)
endif ()

if (NOT WITH_WEBSOCKET)
	add_definitions(-DU_DISABLE_WEBSOCKET)
  set(MHD_MIN_VERSION 0.9.51)
else ()
  set(MHD_MIN_VERSION 0.9.53)
	include(FindGnuTLS)
	find_package(GnuTLS REQUIRED)
	if (GNUTLS_FOUND)
			set(LIBS ${LIBS} ${GNUTLS_LIBRARIES})
	endif ()
endif ()

include(FindMHD)
find_package(MHD ${MHD_MIN_VERSION} REQUIRED)
if (MHD_FOUND)
		set(LIBS ${LIBS} ${MHD_LIBRARIES})
    if (MHD_VERSION_STRING VERSION_LESS "0.9.53")
      set(WITH_WEBSOCKET OFF)
    endif ()
endif ()

option(WITH_CURL "Use Curl library" ON)

if (WITH_CURL)
	include(FindCURL)
	find_package(CURL REQUIRED)
	if (CURL_FOUND)
			set(LIBS ${LIBS} ${CURL_LIBRARIES})
	endif ()
else ()
    add_definitions(-DU_DISABLE_CURL)
endif ()

option(WITH_JANSSON "Use jansson library" ON)

if (WITH_JANSSON)
    include(FindJansson)
		set(JANSSON_MIN_VERSION 2.4)
    find_package(Jansson ${JANSSON_MIN_VERSION} REQUIRED)
    if (JANSSON_FOUND)
        set(LIBS ${LIBS} ${JANSSON_LIBRARIES})
    endif ()
else ()
    add_definitions(-DU_DISABLE_JANSSON)
endif ()

# examples

set(STATIC_CALLBACK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../example_callbacks/static_file/)

add_executable(simple_example ${CMAKE_CURRENT_SOURCE_DIR}/simple_example/simple_example.c)
target_link_libraries(simple_example ${LIBS})

add_executable(auth_client ${CMAKE_CURRENT_SOURCE_DIR}/auth_example/auth_client.c)
target_link_libraries(auth_client ${LIBS})

add_executable(auth_server ${CMAKE_CURRENT_SOURCE_DIR}/auth_example/auth_server.c)
target_link_libraries(auth_server ${LIBS})

add_executable(multiple_callbacks_example ${CMAKE_CURRENT_SOURCE_DIR}/multiple_callbacks_example/multiple_callbacks_example.c)
target_link_libraries(multiple_callbacks_example ${LIBS})

add_executable(stream_client ${CMAKE_CURRENT_SOURCE_DIR}/stream_example/stream_client.c)
target_link_libraries(stream_client ${LIBS})

add_executable(stream_example ${CMAKE_CURRENT_SOURCE_DIR}/stream_example/stream_example.c)
target_link_libraries(stream_example ${LIBS})

add_executable(test_u_map ${CMAKE_CURRENT_SOURCE_DIR}/test_u_map/test_u_map.c)
target_link_libraries(test_u_map ${LIBS})

if (WITH_JANSSON)
  add_executable(injection_example ${CMAKE_CURRENT_SOURCE_DIR}/injection_example/injection_example.c)
  target_link_libraries(injection_example ${LIBS})

  add_executable(sheep_counter ${CMAKE_CURRENT_SOURCE_DIR}/sheep_counter/sheep_counter.c)
  target_link_libraries(sheep_counter ${LIBS})
endif ()

if (WITH_CURL AND WITH_JANSSON)
  add_executable(proxy_example ${CMAKE_CURRENT_SOURCE_DIR}/proxy_example/proxy.c)
  target_link_libraries(proxy_example ${LIBS})

  add_executable(request_client ${CMAKE_CURRENT_SOURCE_DIR}/request_example/client.c)
  target_link_libraries(request_client ${LIBS})

  add_executable(request_server ${CMAKE_CURRENT_SOURCE_DIR}/request_example/server.c)
  target_link_libraries(request_server ${LIBS})

  add_executable(request_mail ${CMAKE_CURRENT_SOURCE_DIR}/request_example/mail.c)
  target_link_libraries(request_mail ${LIBS})
endif ()

if (WITH_WEBSOCKET)
  add_executable(websocket_example ${CMAKE_CURRENT_SOURCE_DIR}/websocket_example/websocket_example.c ${STATIC_CALLBACK_DIR}/static_file_callback.c)
  target_link_libraries(websocket_example ${LIBS})
endif ()
