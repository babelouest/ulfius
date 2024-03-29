#
# Ulfius library
#
# CMake file used to build example programs
#
# Copyright 2018-2022 Nicolas Mora <mail@babelouest.org>
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
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# library info

set(PROJECT_DESCRIPTION "HTTP Framework for REST API in C, using JSON or not, with websockets or not, with streaming data or not")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/ulfius/issues")

include(GNUInstallDirs)
include(CheckSymbolExists)

# cmake modules

set(U_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
list(APPEND CMAKE_MODULE_PATH "${U_CMAKE_MODULE_PATH}")

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

include(FindZLIB)
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
	set(LIBS ${LIBS} ${ZLIB_LIBRARIES})
	include_directories(${ZLIB_INCLUDE_DIRS})
endif ()

find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

include(FindUlfius)
set(ULFIUS_MIN_VERSION "2.6")
find_package(Ulfius ${ULFIUS_MIN_VERSION} REQUIRED)
set(LIBS ${LIBS} ${ULFIUS_LIBRARIES} "-lorcania -ljansson")
include_directories(${ULFIUS_INCLUDE_DIRS})

include(FindYder)
find_package(Yder)
if (YDER_FOUND)
    set(LIBS ${LIBS} "-lyder")
    include_directories(${YDER_INCLUDE_DIRS})
endif ()

# examples

set(STATIC_CALLBACK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../example_callbacks/static_compressed_inmemory_website/)
include_directories(${STATIC_CALLBACK_DIR})

set(HTTP_COMRESSION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../example_callbacks/http_compression/)
include_directories(${HTTP_COMRESSION_DIR})

add_executable(simple_example ${CMAKE_CURRENT_SOURCE_DIR}/simple_example/simple_example.c)
set_target_properties(simple_example PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
target_link_libraries(simple_example ${LIBS})

add_executable(multiple_callbacks_example ${CMAKE_CURRENT_SOURCE_DIR}/multiple_callbacks_example/multiple_callbacks_example.c)
set_target_properties(multiple_callbacks_example PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
target_link_libraries(multiple_callbacks_example ${LIBS})

add_executable(stream_example ${CMAKE_CURRENT_SOURCE_DIR}/stream_example/stream_example.c)
set_target_properties(stream_example PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
target_link_libraries(stream_example ${LIBS})

add_executable(test_u_map ${CMAKE_CURRENT_SOURCE_DIR}/test_u_map/test_u_map.c)
set_target_properties(test_u_map PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
target_link_libraries(test_u_map ${LIBS})

if (WITH_CURL)
  add_executable(stream_client ${CMAKE_CURRENT_SOURCE_DIR}/stream_example/stream_client.c)
  set_target_properties(stream_client PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(stream_client ${LIBS})
endif ()

if (WITH_JANSSON)
  add_executable(injection_example ${CMAKE_CURRENT_SOURCE_DIR}/injection_example/injection_example.c)
  set_target_properties(injection_example PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(injection_example ${LIBS})

  add_executable(sheep_counter ${CMAKE_CURRENT_SOURCE_DIR}/sheep_counter/sheep_counter.c ${STATIC_CALLBACK_DIR}/static_compressed_inmemory_website_callback.c ${HTTP_COMRESSION_DIR}/http_compression_callback.c)
  set_target_properties(sheep_counter PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(sheep_counter ${LIBS})
endif ()

if (WITH_CURL AND WITH_JANSSON)
  add_executable(proxy_example ${CMAKE_CURRENT_SOURCE_DIR}/proxy_example/proxy.c)
  set_target_properties(proxy_example PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(proxy_example ${LIBS})

  add_executable(request_client ${CMAKE_CURRENT_SOURCE_DIR}/request_example/client.c)
  set_target_properties(request_client PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(request_client ${LIBS})

  add_executable(request_server ${CMAKE_CURRENT_SOURCE_DIR}/request_example/server.c)
  set_target_properties(request_server PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(request_server ${LIBS})

  add_executable(request_mail ${CMAKE_CURRENT_SOURCE_DIR}/request_example/mail.c)
  set_target_properties(request_mail PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(request_mail ${LIBS})
endif ()

if (WITH_WEBSOCKET)
  add_executable(websocket_server ${CMAKE_CURRENT_SOURCE_DIR}/websocket_example/websocket_server.c ${STATIC_CALLBACK_DIR}/static_compressed_inmemory_website_callback.c)
  set_target_properties(websocket_server PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  set_target_properties(websocket_server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/websocket_example/")
  target_link_libraries(websocket_server ${LIBS})
  add_executable(websocket_client ${CMAKE_CURRENT_SOURCE_DIR}/websocket_example/websocket_client.c)
  set_target_properties(websocket_client PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  set_target_properties(websocket_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/websocket_example/")
  target_link_libraries(websocket_client ${LIBS})

  add_executable(auth_server ${CMAKE_CURRENT_SOURCE_DIR}/auth_example/auth_server.c)
  set_target_properties(auth_server PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
  target_link_libraries(auth_server ${LIBS} gnutls)

  if (WITH_CURL)
    add_executable(auth_client ${CMAKE_CURRENT_SOURCE_DIR}/auth_example/auth_client.c)
    set_target_properties(auth_client PROPERTIES COMPILE_OPTIONS "-Wextra;-Wconversion")
    target_link_libraries(auth_client ${LIBS})
  endif ()
endif ()

message(STATUS "Websocket support: ${WITH_WEBSOCKET}")
message(STATUS "Outgoing requests support: ${WITH_CURL}")
message(STATUS "Jansson library support: ${WITH_JANSSON}")
message(STATUS "Yder library support: ${WITH_YDER}")
