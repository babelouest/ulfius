#.rst:
# FindOrcania
# -----------
#
# Find Orcania
#
# Find Orcania headers and libraries.
#
# ::
#
#   ORCANIA_FOUND          - True if Orcania found.
#   ORCANIA_INCLUDE_DIRS   - Where to find orcania.h.
#   ORCANIA_LIBRARIES      - List of libraries when using Orcania.
#   ORCANIA_VERSION_STRING - The version of Orcania found.

#=============================================================================
# Copyright 2018 Silvio Clecio <silvioprog@gmail.com>
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
pkg_check_modules(PC_ORCANIA QUIET liborcania)

find_path(ORCANIA_INCLUDE_DIR
        NAMES orcania.h
        HINTS ${PC_ORCANIA_INCLUDEDIR} ${PC_ORCANIA_INCLUDE_DIRS})

find_library(ORCANIA_LIBRARY
        NAMES orcania liborcania
        HINTS ${PC_ORCANIA_LIBDIR} ${PC_ORCANIA_LIBRARY_DIRS})

set(ORCANIA_VERSION_STRING 0.0.0)
if (PC_ORCANIA_VERSION)
    set(ORCANIA_VERSION_STRING ${PC_ORCANIA_VERSION})
elseif (ORCANIA_INCLUDE_DIR AND EXISTS "${ORCANIA_INCLUDE_DIR}/orcania.h")
    set(regex_orcania_version "^#define[ \t]+ORCANIA_VERSION[ \t]+([^\"]+).*")
    file(STRINGS "${ORCANIA_INCLUDE_DIR}/orcania.h" orcania_version REGEX "${regex_orcania_version}")
    string(REGEX REPLACE "${regex_orcania_version}" "\\1" ORCANIA_VERSION_STRING "${orcania_version}")
    unset(regex_orcania_version)
    unset(orcania_version)
    if (NOT ORCANIA_VERSION_STRING)
       set(regex_orcania_version "^#define[ \t]+ORCANIA_VERSION[ \t]+([^\"]+).*")
        file(STRINGS "${ORCANIA_INCLUDE_DIR}/orcania-cfg.h" orcania_version REGEX "${regex_orcania_version}")
        string(REGEX REPLACE "${regex_orcania_version}" "\\1" ORCANIA_VERSION_STRING "${orcania_version}")
        unset(regex_orcania_version)
        unset(orcania_version)
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Orcania
        REQUIRED_VARS ORCANIA_LIBRARY ORCANIA_INCLUDE_DIR
        VERSION_VAR ORCANIA_VERSION_STRING)

if (ORCANIA_FOUND)
    set(ORCANIA_LIBRARIES ${ORCANIA_LIBRARY})
    set(ORCANIA_INCLUDE_DIRS ${ORCANIA_INCLUDE_DIR})
endif ()

mark_as_advanced(ORCANIA_INCLUDE_DIR ORCANIA_LIBRARY)
