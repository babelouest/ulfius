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

set(PROJECT_DESCRIPTION "Web Framework to build REST APIs, Webservices or any HTTP endpoint in C language. Can stream large amount of data, integrate JSON data with Jansson, and create websocket services")
set(PROJECT_HOMEPAGE_URL "https://github.com/babelouest/ulfius/")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/ulfius/issues")
set(LIBRARY_VERSION_MAJOR "2")
set(LIBRARY_VERSION_MINOR "4")
set(LIBRARY_VERSION_PATCH "0")

set(PROJECT_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(LIBRARY_SOVERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")
set(YDER_VERSION_DOWNLOAD "1.4.2")
set(ORCANIA_VERSION_DOWNLOAD "1.2.6")

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
set(UWSC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools/uwsc)

include_directories(${INC_DIR})

set(LIB_SRC
    ${INC_DIR}/ulfius.h
    ${INC_DIR}/u_private.h
    ${INC_DIR}/yuarel.h
    ${SRC_DIR}/u_map.c
    ${SRC_DIR}/u_request.c
    ${SRC_DIR}/u_response.c
    ${SRC_DIR}/u_send_request.c
    ${SRC_DIR}/u_websocket.c
    ${SRC_DIR}/yuarel.c
    ${SRC_DIR}/ulfius.c)

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
    include_directories(${MHD_INCLUDE_DIRS})
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
        include_directories(${CURL_INCLUDE_DIRS})
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
        include_directories(${JANSSON_INCLUDE_DIRS})
        set(LIBS ${LIBS} ${JANSSON_LIBRARIES})
    endif ()
else ()
    add_definitions(-DU_DISABLE_JANSSON)
endif ()

# static library

option(BUILD_STATIC "Build static library." OFF)

if (BUILD_STATIC)
    add_library(ulfius_static STATIC ${LIB_SRC})
    target_compile_definitions(ulfius_static PUBLIC -DO_STATIC_LIBRARY)
    set_target_properties(ulfius_static PROPERTIES
                    OUTPUT_NAME ulfius)
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

# build uwsc

option(BUILD_UWSC "Build uwsc application." ON)

if (WITH_WEBSOCKET AND BUILD_UWSC)
    add_executable(uwsc ${UWSC_DIR}/uwsc.c ${INC_DIR}/ulfius.h ${INC_DIR}/u_private.h)
    add_dependencies(uwsc ulfius)
    target_link_libraries(uwsc ulfius)
    install(TARGETS uwsc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

# dependencies

set(Orcania_FIND_QUIETLY ON) # force to find Orcania quietly
include(FindOrcania)
find_package(Orcania 1.1 QUIET) # try to find Orcania
if (NOT ORCANIA_FOUND)
    include(DownloadProject)
    download_project(PROJ orcania # ... otherwise, download archive
                      URL "https://github.com/babelouest/orcania/archive/v${ORCANIA_VERSION_DOWNLOAD}.tar.gz"
                      QUIET)
    add_subdirectory(${orcania_SOURCE_DIR} ${orcania_BINARY_DIR})
    include_directories(${orcania_SOURCE_DIR}/include)
    include_directories(${orcania_SOURCE_DIR}/include)
    add_dependencies(ulfius orcania)
    add_dependencies(ulfius orcania)
    set(ORCANIA_LIBRARIES orcania orcania)
else()
    set(LIBS ${LIBS} ${ORCANIA_LIBRARIES})
    include_directories(${ORCANIA_INCLUDE_DIRS})
endif ()
target_link_libraries(ulfius ${LIBS} ${ORCANIA_LIBRARIES})

set(SEARCH_ORCANIA OFF CACHE BOOL "Force to false") # Avoid to search and download orcania during yder search and download
set(Yder_FIND_QUIETLY ON) # force to find Yder quietly
include(FindYder)
find_package(Yder 1.1 QUIET) # try to find Yder
if (NOT YDER_FOUND)
    include(DownloadProject)
    download_project(PROJ yder # ... otherwise, download archive
                      URL "https://github.com/babelouest/yder/archive/v${YDER_VERSION_DOWNLOAD}.tar.gz"
                      QUIET)
    add_subdirectory(${yder_SOURCE_DIR} ${yder_BINARY_DIR})
    include_directories(${yder_SOURCE_DIR}/include)
    include_directories(${orcania_SOURCE_DIR}/include)
    add_dependencies(ulfius yder)
    add_dependencies(ulfius orcania)
    set(YDER_LIBRARIES yder orcania)
else()
    set(LIBS ${LIBS} ${YDER_LIBRARIES})
    include_directories(${YDER_INCLUDE_DIRS})
endif ()
target_link_libraries(ulfius ${LIBS} ${YDER_LIBRARIES})

set(PKGCONF_REQ "")
set(PKGCONF_REQ_PRIVATE "liborcania, libyder")
if (WITH_CURL)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libcurl")
endif ()
if (WITH_JANSSON)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, jansson")
endif ()
if (WITH_WEBSOCKET)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, gnutls > 3.5.0, libmicrohttpd > 0.9.53")
else ()
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libmicrohttpd > 0.9.51")
endif ()

# tests

option(BUILD_ULFIUS_TESTING "Build the testing tree." OFF) # because we do not use include(CTest)

if (BUILD_ULFIUS_TESTING)
    include(FindCheck)
    find_package(Check REQUIRED)
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
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

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
    install(DIRECTORY example_callbacks/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}/example_callbacks/ COMPONENT runtime)
    install(DIRECTORY example_programs/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}/example_programs/ COMPONENT runtime)
    install(FILES README.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
    install(FILES INSTALL.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
    install(FILES API.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
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
        "lib${CMAKE_PROJECT_NAME}-dev_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
else ()
set(PACKAGE_FILE_NAME
        "lib${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
endif ()
set(PACKAGE_IGNORED_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/;/.git/;.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_PACKAGE_NAME "libulfius")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Web Framework for C programs")
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_PACKAGE_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

option(BUILD_RPM "Build a RPM for your system" OFF)
if (BUILD_RPM)
  set(CPACK_GENERATOR "TGZ;DEB;RPM")
  set(CPACK_RPM_PACKAGE_LICENSE "LGPL")
  set(CPACK_RPM_PACKAGE_URL "http://babelouest.github.io/ulfius/")
endif ()

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "mail@babelouest.org")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://babelouest.github.io/ulfius/")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.4), liborcania (>= 1.2), libyder (>= 1.3)")
if (WITH_CURL)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libcurl3-gnutls (>= 7.16.2)")
endif ()
if (WITH_JANSSON)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjansson4 (>= 2.1)")
endif ()
if (WITH_WEBSOCKET)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgnutls30 (>= 3.5.0), libmicrohttpd12 (>= 0.9.53)")
else ()
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmicrohttpd12 (>= 0.9.51)")
endif ()
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist_u COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
