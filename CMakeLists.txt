#
# Ulfius library
#
# CMake file used to build all programs
#
# Copyright 2018 Silvio Clecio <silvioprog@gmail.com>
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

project(ulfius C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# library info

set(PROJECT_DESCRIPTION "Logging library for C applications")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/ulfius/issues")

set(LIBRARY_VERSION_MAJOR "1")
set(LIBRARY_VERSION_MINOR "1")
set(LIBRARY_VERSION_PATCH "1")
set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(LIBRARY_SOVERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")

# cmake modules

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

include(GNUInstallDirs)
include(CheckSymbolExists)

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

# directories and source

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${INC_DIR})

set(LIB_SRC
        ${INC_DIR}/ulfius.h
        ${INC_DIR}/u_private.h
        ${SRC_DIR}/u_map.c
        ${SRC_DIR}/u_request.c
        ${SRC_DIR}/u_response.c
        ${SRC_DIR}/u_send_request.c
        ${SRC_DIR}/u_websocket.c
        ${SRC_DIR}/ulfius.c)

include(FindOrcania)
set(ORCANIA_MIN_VERSION "1.1.1")
find_package(Orcania ${ORCANIA_MIN_VERSION} REQUIRED)
set(LIBS ${ORCANIA_LIBRARIES})

include(FindYder)
set(YDER_MIN_VERSION "1.1.1")
find_package(Yder ${YDER_MIN_VERSION} REQUIRED)
set(LIBS ${LIBS} ${YDER_LIBRARIES})

include(FindMHD)
set(MHD_MIN_VERSION 0.9.53)
find_package(MHD ${MHD_MIN_VERSION} REQUIRED)
if (MHD_FOUND)
		set(LIBS ${LIBS} ${MHD_LIBRARIES})
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

# websocket support

option(INSTALL_WEBSOCKET "Websocket support" ON)

if (WIN32)
	set(INSTALL_WEBSOCKET OFF)
endif ()

if (!INSTALL_WEBSOCKET)
	add_definitions(-DU_DISABLE_WEBSOCKET)
endif ()

# static library

option(BUILD_STATIC "Build static library." OFF)

if (BUILD_STATIC)
	add_library(ulfius_static STATIC ${LIB_SRC})
	target_compile_definitions(ulfius_static PUBLIC -DO_STATIC_LIBRARY)
	set_target_properties(ulfius_static PROPERTIES
					OUTPUT_NAME libulfius)
endif ()

# shared library

add_library(ulfius SHARED ${LIB_SRC})
set_target_properties(ulfius PROPERTIES
        COMPILE_OPTIONS -Wextra
        PUBLIC_HEADER ${INC_DIR}/ulfius.h
        VERSION "${LIBRARY_VERSION}"
        SOVERSION "${LIBRARY_SOVERSION}")
if (WIN32)
    set_target_properties(ulfius PROPERTIES SUFFIX "-${LIBRARY_VERSION_MAJOR}.dll")
endif ()
target_link_libraries(ulfius ${LIBS})

# tests

option(BUILD_TESTING "Build the testing tree." OFF) # because we don not use include(CTest)

if (BUILD_TESTING)
    include(FindCheck)
    find_package(Check)
    if (CHECK_FOUND)
        if (NOT WIN32)
            include(FindSubunit)
            find_package(Subunit REQUIRED)
        endif ()

        enable_testing()

        set(CMAKE_CTEST_COMMAND ctest -V)

        set(TST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
        set(LIBS ulfius ${LIBS} ${CHECK_LIBRARIES})
        if (NOT WIN32)
            find_package(Threads REQUIRED)
            set(LIBS ${LIBS} ${SUBUNIT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} m rt)
        endif ()

        set(TESTS core u_map framework)

        configure_file(
                "${CMAKE_MODULE_PATH}/CTestCustom.cmake.in"
                "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
                @ONLY)

        foreach (t ${TESTS})
            add_executable(${t} EXCLUDE_FROM_ALL ${TST_DIR}/${t}.c)
            target_include_directories(${t} PUBLIC ${TST_DIR})
            target_link_libraries(${t} PUBLIC ${LIBS})
            add_test(NAME ${t}
                    WORKING_DIRECTORY ${TST_DIR}
                    COMMAND ${t})
        endforeach ()
    endif ()
endif ()

# install target

option(INSTALL_HEADER "Install the header files" ON) # Install ulfius.h or not

configure_file(libulfius.pc.in libulfius.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libulfius.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

SET (TARGETS ulfius)
if (BUILD_STATIC)
	SET (TARGETS ${TARGETS} ulfius_static)
endif ()

if (INSTALL_HEADER)
install(TARGETS ${TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else ()
install(TARGETS ${TARGETS}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif ()

# uninstall target

if (NOT TARGET uninstall)
    configure_file(
            "${CMAKE_MODULE_PATH}/CMakeUninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()

# packaging

set(CPACK_PACKAGE_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

if (INSTALL_HEADER)
set(PACKAGE_FILE_NAME
        "lib${CMAKE_PROJECT_NAME}-dev${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
else ()
set(PACKAGE_FILE_NAME
        "lib${CMAKE_PROJECT_NAME}${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
endif ()
set(PACKAGE_IGNORED_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/;/.git/;.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_GENERATOR "TGZ;DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "mail@babelouest.org")
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist
        COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

option(WITH_EXAMPLES "Build Ulfius Examples" ON)

if (WITH_EXAMPLES)

	# examples

	set(EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/example_programs)
	set(STATIC_CALLBACK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/example_callbacks/static_file/)
	set(LIBS ${LIBS} ulfius)
  
	add_executable(simple_example ${EXAMPLE_DIR}/simple_example/simple_example.c)
	target_link_libraries(simple_example ${LIBS})
  add_dependencies(simple_example ulfius)

	add_executable(auth_client ${EXAMPLE_DIR}/auth_example/auth_client.c)
	target_link_libraries(auth_client ${LIBS})
  add_dependencies(auth_client ulfius)

	add_executable(auth_server ${EXAMPLE_DIR}/auth_example/auth_server.c)
	target_link_libraries(auth_server ${LIBS})
  add_dependencies(auth_server ulfius)

	add_executable(injection_example ${EXAMPLE_DIR}/injection_example/injection_example.c)
	target_link_libraries(injection_example ${LIBS})
  add_dependencies(injection_example ulfius)

	add_executable(multiple_callbacks_example ${EXAMPLE_DIR}/multiple_callbacks_example/multiple_callbacks_example.c)
	target_link_libraries(multiple_callbacks_example ${LIBS})
  add_dependencies(multiple_callbacks_example ulfius)

	add_executable(proxy_example ${EXAMPLE_DIR}/proxy_example/proxy.c)
	target_link_libraries(proxy_example ${LIBS})
  add_dependencies(proxy_example ulfius)

	add_executable(request_client ${EXAMPLE_DIR}/request_example/client.c)
	target_link_libraries(request_client ${LIBS})
  add_dependencies(request_client ulfius)

	add_executable(request_server ${EXAMPLE_DIR}/request_example/server.c)
	target_link_libraries(request_server ${LIBS})
  add_dependencies(request_server ulfius)

	add_executable(request_mail ${EXAMPLE_DIR}/request_example/mail.c)
	target_link_libraries(request_mail ${LIBS})
  add_dependencies(request_mail ulfius)

	add_executable(sheep_counter ${EXAMPLE_DIR}/sheep_counter/sheep_counter.c)
	target_link_libraries(sheep_counter ${LIBS})
  add_dependencies(sheep_counter ulfius)

	add_executable(stream_client ${EXAMPLE_DIR}/stream_example/stream_client.c)
	target_link_libraries(stream_client ${LIBS})
  add_dependencies(stream_client ulfius)

	add_executable(stream_example ${EXAMPLE_DIR}/stream_example/stream_example.c)
	target_link_libraries(stream_example ${LIBS})
  add_dependencies(stream_example ulfius)

	add_executable(test_u_map ${EXAMPLE_DIR}/test_u_map/test_u_map.c)
	target_link_libraries(test_u_map ${LIBS})
  add_dependencies(test_u_map ulfius)

	if (INSTALL_WEBSOCKET)
		add_executable(websocket_example ${EXAMPLE_DIR}/websocket_example/websocket_example.c ${STATIC_CALLBACK_DIR}/static_file_callback.c)
		target_link_libraries(websocket_example ${LIBS})
    add_dependencies(websocket_example ulfius)
	endif ()

endif ()
