#
# Ulfius library
#
# CMake file used to build all programs
#
# Copyright 2018      Silvio Clecio <silvioprog@gmail.com>
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

project(ulfius C)

set(CMAKE_C_STANDARD 99)
if (NOT MSVC)
    set(CMAKE_C_FLAGS "-Wall -Werror ${CMAKE_C_FLAGS}")
endif()

# library info

set(ULFIUS_LIBS )

set(PROJECT_DESCRIPTION "Web Framework to build REST APIs, Webservices or any HTTP endpoint in C language. Can stream large amount of data, integrate JSON data with Jansson, and create websocket services")
set(PROJECT_HOMEPAGE_URL "https://github.com/babelouest/ulfius/")
set(PROJECT_BUGREPORT_PATH "https://github.com/babelouest/ulfius/issues")
set(LIBRARY_VERSION_MAJOR "2")
set(LIBRARY_VERSION_MINOR "7")
set(LIBRARY_VERSION_PATCH "14")

set(PROJECT_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(PROJECT_VERSION_MAJOR ${LIBRARY_VERSION_MAJOR})
set(PROJECT_VERSION_MINOR ${LIBRARY_VERSION_MINOR})
set(PROJECT_VERSION_PATCH ${LIBRARY_VERSION_PATCH})

if (${LIBRARY_VERSION_MAJOR} VERSION_LESS 10)
    set (LIBRARY_VERSION_MAJOR_PAD "0${LIBRARY_VERSION_MAJOR}")
else ()
    set (LIBRARY_VERSION_MAJOR_PAD "${LIBRARY_VERSION_MAJOR}")
endif ()
if (${LIBRARY_VERSION_MINOR} VERSION_LESS 10)
    set (LIBRARY_VERSION_MINOR_PAD "0${LIBRARY_VERSION_MINOR}")
else ()
    set (LIBRARY_VERSION_MINOR_PAD "${LIBRARY_VERSION_MINOR}")
endif ()
if (${LIBRARY_VERSION_PATCH} VERSION_LESS 10)
    set (LIBRARY_VERSION_PATCH_PAD "0${LIBRARY_VERSION_PATCH}")
else ()
    set (LIBRARY_VERSION_PATCH_PAD "${LIBRARY_VERSION_PATCH}")
endif ()
set(PROJECT_VERSION_NUMBER "${LIBRARY_VERSION_MAJOR_PAD}${LIBRARY_VERSION_MINOR_PAD}${LIBRARY_VERSION_PATCH_PAD}")

set(LIBRARY_VERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}.${LIBRARY_VERSION_PATCH}")
set(LIBRARY_SOVERSION "${LIBRARY_VERSION_MAJOR}.${LIBRARY_VERSION_MINOR}")
set(ORCANIA_VERSION_REQUIRED "2.3.3")
set(YDER_VERSION_REQUIRED "1.4.20")
set(JANSSON_VERSION_REQUIRED "2.1")

# cmake modules

set(U_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
list(APPEND CMAKE_MODULE_PATH "${U_CMAKE_MODULE_PATH}")

include(GNUInstallDirs)
include(CheckSymbolExists)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)

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

# pthread libraries
find_package (Threads)
list(APPEND ULFIUS_LIBS ${CMAKE_THREAD_LIBS_INIT})

# GNU TLS support
option(WITH_GNUTLS "GNU TLS support" ON)

if (WITH_GNUTLS)
    find_package(GnuTLS REQUIRED)
    list(APPEND ULFIUS_LIBS GnuTLS::GnuTLS)
endif ()

# websocket support

# current websocket implementation depends on GNU TLS, and is not supported on Windows
cmake_dependent_option(WITH_WEBSOCKET "Websocket support" ON "NOT WIN32;WITH_GNUTLS" OFF)
option(WITH_WEBSOCKET_MESSAGE_LIST "Websocket messages list support" ON)

if (WITH_WEBSOCKET)
    set(MHD_MIN_VERSION 0.9.53)
else ()
    set(MHD_MIN_VERSION 0.9.51)
endif ()

if (WITH_GNUTLS)
    set(U_DISABLE_GNUTLS OFF)
else ()
    set(U_DISABLE_GNUTLS ON)
endif ()

find_package(MHD ${MHD_MIN_VERSION} REQUIRED)
list(APPEND ULFIUS_LIBS MHD::MHD)
include_directories(${MHD_INCLUDE_DIRS})
if (MHD_VERSION_STRING VERSION_LESS "0.9.53")
    set(WITH_WEBSOCKET OFF)
endif ()

if (WITH_WEBSOCKET)
    set(U_DISABLE_WEBSOCKET OFF)
    find_package(ZLIB REQUIRED)
    list(APPEND ULFIUS_LIBS ZLIB::ZLIB)
else ()
    set(U_DISABLE_WEBSOCKET ON)
endif ()

if (WITH_WEBSOCKET_MESSAGE_LIST)
    set(U_DISABLE_WS_MESSAGE_LIST OFF)
else ()
    set(U_DISABLE_WS_MESSAGE_LIST ON)
endif ()

option(WITH_CURL "Use Curl library" ON)

if (WITH_CURL)
    find_package(CURL REQUIRED)
    # FIXME: CMake 3.12.4 creates CURL::libcurl
    if (NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl INTERFACE IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
                INTERFACE_LINK_LIBRARIES "${CURL_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
    endif ()
    list(APPEND ULFIUS_LIBS CURL::libcurl)
    set(U_DISABLE_CURL OFF)
else ()
    set(U_DISABLE_CURL ON)
endif ()

option(WITH_JANSSON "Use jansson library" ON)

if (WITH_JANSSON)
    set(JANSSON_MIN_VERSION 2.4)
    find_package(Jansson ${JANSSON_MIN_VERSION} REQUIRED)
    list(APPEND ULFIUS_LIBS Jansson::Jansson)
    set(U_DISABLE_JANSSON OFF)
else ()
    set(U_DISABLE_JANSSON ON)
endif ()

option(BUILD_SHARED "Build shared library." ON)
option(BUILD_STATIC "Build static library." OFF)

if (NOT BUILD_STATIC AND NOT BUILD_SHARED)
    message(FATAL_ERROR "BUILD_SHARED and BUILD_STATIC cannot be both disabled")
endif ()

# TO MY FUTURE SELF
# The following 2 blocks are put BEFORE searching for Orcania and Yder by design
# Otherwise it will lead to cmake errors
# DON'T MOVE IT BELOW PLEASE!

# static library

if (BUILD_STATIC)
    add_library(ulfius_static STATIC ${LIB_SRC})
    add_library(Ulfius::Ulfius-static ALIAS ulfius_static)
    target_include_directories(ulfius_static
        PUBLIC "$<BUILD_INTERFACE:${INC_DIR}>"
        PUBLIC "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
    target_link_libraries(ulfius_static PUBLIC ${ULFIUS_LIBS})
    target_compile_definitions(ulfius_static PUBLIC O_STATIC_LIBRARY)
    set_target_properties(ulfius_static PROPERTIES
            PUBLIC_HEADER "${INC_DIR}/ulfius.h;${PROJECT_BINARY_DIR}/ulfius-cfg.h"
            OUTPUT_NAME ulfius
            EXPORT_NAME Ulfius-static)
    if (MSVC)
        set_target_properties(ulfius_static PROPERTIES
                OUTPUT_NAME ulfius-static)
    endif ()
    if (NOT MSVC)
        target_compile_options(ulfius_static PRIVATE -Wextra -Wconversion)
    endif ()
    set(ulfius_lib ulfius_static)
endif ()

# shared library

if (BUILD_SHARED)
    add_library(ulfius SHARED ${LIB_SRC})
    add_library(Ulfius::Ulfius ALIAS ulfius)
    target_include_directories(ulfius
        PUBLIC "$<BUILD_INTERFACE:${INC_DIR}>"
        PUBLIC "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
    target_link_libraries(ulfius PUBLIC ${ULFIUS_LIBS})
    set_target_properties(ulfius PROPERTIES
            PUBLIC_HEADER "${INC_DIR}/ulfius.h;${PROJECT_BINARY_DIR}/ulfius-cfg.h"
            VERSION "${LIBRARY_VERSION}"
            SOVERSION "${LIBRARY_SOVERSION}"
            WINDOWS_EXPORT_ALL_SYMBOLS TRUE
            EXPORT_NAME Ulfius)
    if (WIN32)
        set_target_properties(ulfius PROPERTIES SUFFIX "-${LIBRARY_VERSION_MAJOR}.dll")
    endif ()
    if (NOT MSVC)
        target_compile_options(ulfius PRIVATE -Wextra -Wconversion)
    endif()
    set(ulfius_lib ulfius)
endif ()

# documentation

option(BUILD_ULFIUS_DOCUMENTATION "Build the documentation." OFF)
if (BUILD_ULFIUS_DOCUMENTATION)
    find_package(Doxygen REQUIRED)
    if (DOXYGEN_FOUND)
        set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen.cfg)
        set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)

        configure_file(${doxyfile_in} ${doxyfile} @ONLY)

        add_custom_target(doc
                          COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_in}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                          COMMENT "Generating documentation with Doxygen"
                          VERBATIM)

    else ()
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif ()
endif ()

# dependencies

find_package(Orcania ${ORCANIA_VERSION_REQUIRED} REQUIRED)
if ("${ORCANIA_VERSION_STRING}" VERSION_GREATER_EQUAL "${ORCANIA_VERSION_REQUIRED}")
    message(STATUS "Orcania found: ${ORCANIA_VERSION_STRING}")
else ()
    message( FATAL_ERROR "Orcania version required: ${ORCANIA_VERSION_REQUIRED} - version installed: ${ORCANIA_VERSION_STRING}")
endif ()

if (BUILD_SHARED)
    target_link_libraries(ulfius PUBLIC $<TARGET_NAME:Orcania::Orcania>)
endif ()
if (BUILD_STATIC)
    if(TARGET Orcania::Orcania-static)
        target_link_libraries(ulfius_static PUBLIC $<TARGET_NAME:Orcania::Orcania-static>)
    else()
        target_link_libraries(ulfius_static PUBLIC $<TARGET_NAME:Orcania::Orcania>)
    endif()
endif ()

find_package(Yder ${YDER_VERSION_REQUIRED} REQUIRED)
if ("${YDER_VERSION_STRING}" VERSION_GREATER_EQUAL "${YDER_VERSION_REQUIRED}")
    message(STATUS "Yder found: ${YDER_VERSION_STRING}")
else ()
    message( FATAL_ERROR "Yder version required: ${YDER_VERSION_REQUIRED} - version installed: ${YDER_VERSION_STRING}")
endif ()

if (BUILD_SHARED)
    target_link_libraries(ulfius PUBLIC $<TARGET_NAME:Yder::Yder>)
endif ()
if (BUILD_STATIC)
    if(TARGET Yder::Yder-static)
        target_link_libraries(ulfius_static PUBLIC $<TARGET_NAME:Yder::Yder-static>)
    else()
        target_link_libraries(ulfius_static PUBLIC $<TARGET_NAME:Yder::Yder>)
    endif()
endif ()

# build uwsc

cmake_dependent_option(BUILD_UWSC "Build uwsc application." ON "WITH_WEBSOCKET" OFF)

if (BUILD_UWSC)
    add_executable(uwsc ${UWSC_DIR}/uwsc.c ${INC_DIR}/ulfius.h ${INC_DIR}/u_private.h ${PROJECT_BINARY_DIR}/ulfius-cfg.h)
    set_target_properties(uwsc PROPERTIES SKIP_BUILD_RPATH TRUE)
    add_dependencies(uwsc ulfius)
    target_link_libraries(uwsc PRIVATE "${ulfius_lib}")
    target_compile_options(uwsc PRIVATE -Wextra -Wconversion)
    install(TARGETS uwsc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    INSTALL(FILES ${UWSC_DIR}/uwsc.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 COMPONENT runtime)
endif ()

set(PKGCONF_REQ "")
set(PKGCONF_REQ_PRIVATE "liborcania, libyder")
if (WITH_CURL)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libcurl")
endif ()
if (WITH_JANSSON)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, jansson")
endif ()
if (WITH_GNUTLS)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, gnutls >= 3.5.0")
endif ()
if (WITH_WEBSOCKET)
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libmicrohttpd >= 0.9.53")
else ()
  set (PKGCONF_REQ_PRIVATE "${PKGCONF_REQ_PRIVATE}, libmicrohttpd >= 0.9.51")
endif ()

# build ulfius-cfg.h file
configure_file(${INC_DIR}/ulfius-cfg.h.in ${PROJECT_BINARY_DIR}/ulfius-cfg.h)
set (CMAKE_EXTRA_INCLUDE_FILES ${PROJECT_BINARY_DIR})

# tests

option(BUILD_ULFIUS_TESTING "Build the testing tree." OFF) # because we do not use include(CTest)

if (BUILD_ULFIUS_TESTING)
    find_package(Check REQUIRED)
    if (CHECK_FOUND)
        if (NOT WIN32 AND NOT APPLE)
            include(FindSubunit)
            find_package(Subunit REQUIRED)
        endif ()

        enable_testing()

        set(CMAKE_CTEST_COMMAND ctest -V)

        set(TST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
        set(EXAMPLE_CALLBACK_COMPRESSION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/example_callbacks/http_compression/)
        set(EXAMPLE_CALLBACK_STATIC_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/example_callbacks/static_compressed_inmemory_website/)
        set(TEST_LIBS Ulfius::Ulfius Check::Check)
        if (NOT WIN32)
            find_package(Threads REQUIRED)
            list(APPEND TEST_LIBS ${CMAKE_THREAD_LIBS_INIT} m)
        endif ()
        if (NOT APPLE AND NOT WIN32)
            list(APPEND TEST_LIBS rt)
        endif ()
        if (NOT WIN32 AND NOT APPLE)
            list(APPEND TEST_LIBS Subunit::Subunit)
        endif ()
        
        set(TESTS core u_map framework example_callbacks)
        if (WITH_WEBSOCKET)
            set(TESTS ${TESTS} websocket)
        endif ()

        configure_file(
                "${U_CMAKE_MODULE_PATH}/CTestCustom.cmake.in"
                "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
                @ONLY)
    
        foreach (t ${TESTS})
            add_executable(${t} EXCLUDE_FROM_ALL ${TST_DIR}/${t}.c
                                                 ${EXAMPLE_CALLBACK_COMPRESSION_DIR}/http_compression_callback.c
                                                 ${EXAMPLE_CALLBACK_COMPRESSION_DIR}/http_compression_callback.h
                                                 ${EXAMPLE_CALLBACK_STATIC_FILE_DIR}/static_compressed_inmemory_website_callback.c
                                                 ${EXAMPLE_CALLBACK_STATIC_FILE_DIR}/static_compressed_inmemory_website_callback.h )
            target_include_directories(${t} PRIVATE ${TST_DIR} ${EXAMPLE_CALLBACK_COMPRESSION_DIR} ${EXAMPLE_CALLBACK_STATIC_FILE_DIR})
            target_link_libraries(${t} PRIVATE ${TEST_LIBS})
            add_test(NAME ${t}
                     WORKING_DIRECTORY ${TST_DIR}
                     COMMAND ${t})
        endforeach ()

    endif ()
endif ()

# install target

option(INSTALL_HEADER "Install the header files" ON) # Install ulfius.h or not

if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(PKGCONFIG_TARGET_INCLUDES "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  set(PKGCONFIG_TARGET_INCLUDES "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()

if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(PKGCONFIG_TARGET_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
  set(PKGCONFIG_TARGET_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()

configure_file(libulfius.pc.in libulfius.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libulfius.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

set(TARGETS )
if (BUILD_SHARED)
    list(APPEND TARGETS ulfius)
endif ()
if (BUILD_STATIC)
    list(APPEND TARGETS ulfius_static)
endif ()

if (INSTALL_HEADER)
	install(TARGETS ${TARGETS} EXPORT UlfiusExports
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY example_callbacks/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}/example_callbacks/ COMPONENT runtime)
    install(DIRECTORY example_programs/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}/example_programs/
            COMPONENT runtime
			PATTERN example_programs/.gitignore EXCLUDE)
    install(FILES README.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
    install(FILES INSTALL.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
    install(FILES API.md
            DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT runtime)
else ()
    install(TARGETS ${TARGETS} EXPORT UlfiusExports
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif ()

if (INSTALL_HEADER)
    set(ULFIUS_INSTALL_CMAKEDIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake/Ulfius")
    if (WIN32 AND NOT MINGW)
        set(ULFIUS_INSTALL_CMAKEDIR_DEFAULT "cmake")
    endif ()
    set(ULFIUS_INSTALL_CMAKEDIR ${ULFIUS_INSTALL_CMAKEDIR_DEFAULT} CACHE STRING "Location where to install the cmake config files")

    install(EXPORT UlfiusExports DESTINATION "${ULFIUS_INSTALL_CMAKEDIR}"
        NAMESPACE "Ulfius::"
        FILE "UlfiusTargets.cmake")

    configure_package_config_file(cmake-modules/UlfiusConfig.cmake.in UlfiusConfig.cmake
        INSTALL_DESTINATION "${ULFIUS_INSTALL_CMAKEDIR}")
    write_basic_package_version_file(UlfiusConfigVersion.cmake
        COMPATIBILITY AnyNewerVersion)

    install(FILES
                cmake-modules/FindGnuTLS.cmake
                cmake-modules/FindJansson.cmake
                cmake-modules/FindMHD.cmake
                "${PROJECT_BINARY_DIR}/UlfiusConfig.cmake"
                "${PROJECT_BINARY_DIR}/UlfiusConfigVersion.cmake"
            DESTINATION "${ULFIUS_INSTALL_CMAKEDIR}")
endif ()

# uninstall target

if (NOT TARGET uninstall)
    configure_file(
            "${U_CMAKE_MODULE_PATH}/CMakeUninstall.cmake.in"
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
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.4), liborcania (>= ${ORCANIA_VERSION_REQUIRED}), libyder (>= ${YDER_VERSION_REQUIRED})")

if (INSTALL_HEADER)
  if (WITH_CURL)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libcurl4-gnutls-dev (>= 7.16.2) | libcurl4-nss-dev (>= 7.16.2) | libcurl4-openssl-dev (>= 7.16.2)")
  endif ()
  if (WITH_JANSSON)
      if (NOT ${RELEASE_CODENAME} STREQUAL "stretch" AND NOT ${RELEASE_CODENAME} STREQUAL "bionic")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjansson-dev (>= ${JANSSON_VERSION_REQUIRED})")
      endif ()
  endif ()
  if (WITH_GNUTLS)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgnutls28-dev|libgnutls-dev (>= 3.5.0)")
  endif ()
  if (WITH_WEBSOCKET)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmicrohttpd-dev (>= 0.9.53)")
  else ()
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmicrohttpd-dev (>= 0.9.51)")
  endif ()
else ()
  if (WITH_CURL)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libcurl3-gnutls (>= 7.16.2) | libcurl3-nss (>= 7.16.2) | libcurl4 (>= 7.16.2)")
  endif ()
  if (WITH_JANSSON)
      if (NOT ${RELEASE_CODENAME} STREQUAL "stretch" AND NOT ${RELEASE_CODENAME} STREQUAL "bionic")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjansson4 (>= ${JANSSON_VERSION_REQUIRED})")
      endif ()
  endif ()
  if (WITH_GNUTLS)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgnutls30 (>= 3.5.0)")
  endif ()
  if (WITH_WEBSOCKET)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmicrohttpd12 (>= 0.9.53)")
  else ()
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmicrohttpd12 (>= 0.9.51)")
  endif ()
endif ()

set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_FILE_NAME})
set(CPACK_SOURCE_IGNORE_FILES ${PACKAGE_IGNORED_FILES})

include(CPack)

add_custom_target(dist_u COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

message(STATUS "GNU TLS support:                ${WITH_GNUTLS}")
message(STATUS "Websocket support:              ${WITH_WEBSOCKET}")
message(STATUS "Websocket message list support: ${WITH_WEBSOCKET_MESSAGE_LIST}")
message(STATUS "Outgoing requests support:      ${WITH_CURL}")
message(STATUS "Jansson library support:        ${WITH_JANSSON}")
message(STATUS "Yder support:                   ${WITH_YDER}")
message(STATUS "Build uwsc application:         ${BUILD_UWSC}")
message(STATUS "Build shared library:           ${BUILD_SHARED}")
message(STATUS "Build static library:           ${BUILD_STATIC}")
message(STATUS "Build testing tree:             ${BUILD_ULFIUS_TESTING}")
message(STATUS "Install the header files:       ${INSTALL_HEADER}")
message(STATUS "Build RPM package:              ${BUILD_RPM}")
message(STATUS "Build documentation:            ${BUILD_ULFIUS_DOCUMENTATION}")
