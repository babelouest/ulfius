#.rst:
# FindUlfius
# -----------
#
# Find Ulfius
#
# Find Ulfius headers and libraries.
#
# ::
#
#   ULFIUS_FOUND          - True if Ulfius found.
#   ULFIUS_INCLUDE_DIRS   - Where to find ulfius.h.
#   ULFIUS_LIBRARIES      - List of libraries when using Ulfius.
#   ULFIUS_VERSION_STRING - The version of Ulfius found.

#=============================================================================
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation;
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU GENERAL PUBLIC LICENSE for more details.
#
# You should have received a copy of the GNU General Public
# License along with this library.	If not, see <http://www.gnu.org/licenses/>.
#=============================================================================

find_package(PkgConfig QUIET)
pkg_check_modules(PC_ULFIUS QUIET ulfius)

find_path(ULFIUS_INCLUDE_DIR
        NAMES ulfius.h
        HINTS ${PC_ULFIUS_INCLUDEDIR} ${PC_ULFIUS_INCLUDE_DIRS})

find_library(ULFIUS_LIBRARY
        NAMES ulfius libulfius
        HINTS ${PC_ULFIUS_LIBDIR} ${PC_ULFIUS_LIBRARY_DIRS})

set(ULFIUS_VERSION_STRING 0.0.0)
if (PC_ULFIUS_VERSION)
    set(ULFIUS_VERSION_STRING ${PC_ULFIUS_VERSION})
elseif (ULFIUS_INCLUDE_DIR AND EXISTS "${ULFIUS_INCLUDE_DIR}/ulfius.h")
    set(regex_ulfius_version "^#define[ \t]+ULFIUS_VERSION[ \t]+([^\"]+).*")
    file(STRINGS "${ULFIUS_INCLUDE_DIR}/ulfius-cfg.h" ulfius_version REGEX "${regex_ulfius_version}")
    string(REGEX REPLACE "${regex_ulfius_version}" "\\1" ULFIUS_VERSION_STRING "${ulfius_version}")
    unset(regex_ulfius_version)
    unset(ulfius_version)

    set(regex_ulfius_disable_curl "^#define[ \t]+U_DISABLE_CURL.*")
    file(STRINGS "${ULFIUS_INCLUDE_DIR}/ulfius-cfg.h" ulfius_disable_curl REGEX "${regex_ulfius_disable_curl}")
    if (NOT "${ulfius_disable_curl}" STREQUAL "")
      set(WITH_CURL OFF)
    else ()
      set(WITH_CURL ON)
    endif()
    unset(regex_ulfius_disable_curl)
    unset(ulfius_disable_curl)

    set(regex_ulfius_disable_jansson "^#define[ \t]+U_DISABLE_JANSSON.*")
    file(STRINGS "${ULFIUS_INCLUDE_DIR}/ulfius-cfg.h" ulfius_disable_jansson REGEX "${regex_ulfius_disable_jansson}")
    if (NOT "${ulfius_disable_jansson}" STREQUAL "")
      set(WITH_JANSSON OFF)
    else ()
      set(WITH_JANSSON ON)
    endif()
    unset(regex_ulfius_disable_jansson)
    unset(ulfius_disable_jansson)

    set(regex_ulfius_disable_websocket "^#define[ \t]+U_DISABLE_WEBSOCKET.*")
    file(STRINGS "${ULFIUS_INCLUDE_DIR}/ulfius-cfg.h" ulfius_disable_websocket REGEX "${regex_ulfius_disable_websocket}")
    if (NOT "${ulfius_disable_websocket}" STREQUAL "")
      set(WITH_WEBSOCKET OFF)
    else ()
      set(WITH_WEBSOCKET ON)
    endif()
    unset(regex_ulfius_disable_websocket)
    unset(ulfius_disable_websocket)

endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ulfius
        REQUIRED_VARS ULFIUS_LIBRARY ULFIUS_INCLUDE_DIR
        VERSION_VAR ULFIUS_VERSION_STRING)

if (ULFIUS_FOUND)
    set(ULFIUS_LIBRARIES ${ULFIUS_LIBRARY})
    set(ULFIUS_INCLUDE_DIRS ${ULFIUS_INCLUDE_DIR})
endif ()

mark_as_advanced(ULFIUS_INCLUDE_DIR ULFIUS_LIBRARY)
