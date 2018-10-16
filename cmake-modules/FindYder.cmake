#.rst:
# FindYder
# -----------
#
# Find Yder
#
# Find Yder headers and libraries.
#
# ::
#
#   YDER_FOUND          - True if Yder found.
#   YDER_INCLUDE_DIRS   - Where to find yder.h.
#   YDER_LIBRARIES      - List of libraries when using Yder.
#   YDER_VERSION_STRING - The version of Yder found.

#=============================================================================
# Copyright 2018 Nicolas Mora <mail@babelouest.org>
# Copyright 2018 Silvio Clecio <silvioprog@gmail.com>
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
pkg_check_modules(PC_YDER QUIET yder)

find_path(YDER_INCLUDE_DIR
        NAMES yder.h
        HINTS ${PC_YDER_INCLUDEDIR} ${PC_YDER_INCLUDE_DIRS})

find_library(YDER_LIBRARY
        NAMES yder libyder
        HINTS ${PC_YDER_LIBDIR} ${PC_YDER_LIBRARY_DIRS})

set(YDER_VERSION_STRING 0.0.0)
if (PC_YDER_VERSION)
    set(YDER_VERSION_STRING ${PC_YDER_VERSION})
elseif (YDER_INCLUDE_DIR AND EXISTS "${YDER_INCLUDE_DIR}/yder.h")
    set(regex_yder_version "^#define[ \t]+YDER_VERSION[ \t]+([^\"]+).*")
    file(STRINGS "${YDER_INCLUDE_DIR}/yder-cfg.h" yder_version REGEX "${regex_yder_version}")
    string(REGEX REPLACE "${regex_yder_version}" "\\1" YDER_VERSION_STRING "${yder_version}")
    unset(regex_yder_version)
    unset(yder_version)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Yder
        REQUIRED_VARS YDER_LIBRARY YDER_INCLUDE_DIR
        VERSION_VAR YDER_VERSION_STRING)

if (YDER_FOUND)
    set(YDER_LIBRARIES ${YDER_LIBRARY})
    set(YDER_INCLUDE_DIRS ${YDER_INCLUDE_DIR})
endif ()

mark_as_advanced(YDER_INCLUDE_DIR YDER_LIBRARY)
