#.rst:
# FindCheck
# -----------
#
# Find Check
#
# Find Check headers and libraries.
#
# ::
#
#   CHECK_FOUND          - True if Check found.
#   CHECK_INCLUDE_DIRS   - Where to find check.h.
#   CHECK_LIBRARIES      - List of libraries when using Check.
#   CHECK_VERSION_STRING - The version of Check found.

#=============================================================================
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

# Sat Jan 20 23:33:47 -03 2018

find_package(PkgConfig QUIET)
pkg_check_modules(PC_CHECK QUIET check)

find_path(CHECK_INCLUDE_DIR
        NAMES check.h
        HINTS ${PC_CHECK_INCLUDEDIR} ${PC_CHECK_INCLUDE_DIRS})

find_library(CHECK_LIBRARY
        NAMES check libcheck
        HINTS ${PC_CHECK_LIBDIR} ${PC_CHECK_LIBRARY_DIRS})

if (PC_CHECK_VERSION)
    set(CHECK_VERSION_STRING ${PC_CHECK_VERSION})
elseif (CHECK_INCLUDE_DIR AND EXISTS "${CHECK_INCLUDE_DIR}/check.h")
    set(check_version_list MAJOR MINOR MICRO)
    foreach (v ${check_version_list})
        set(regex_check_version "^#define CHECK_${v}_VERSION +\\(?([0-9]+)\\)?$")
        file(STRINGS "${CHECK_INCLUDE_DIR}/check.h" check_version_${v} REGEX "${regex_check_version}")
        string(REGEX REPLACE "${regex_check_version}" "\\1" check_version_${v} "${check_version_${v}}")
        unset(regex_check_version)
    endforeach ()
    set(CHECK_VERSION_STRING "${check_version_MAJOR}.${check_version_MINOR}.${check_version_MICRO}")
    foreach (v check_version_list)
        unset(check_version_${v})
    endforeach ()
    unset(check_version_list)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Check
        REQUIRED_VARS CHECK_LIBRARY CHECK_INCLUDE_DIR
        VERSION_VAR CHECK_VERSION_STRING)

if (CHECK_FOUND)
    set(CHECK_LIBRARIES ${CHECK_LIBRARY})
    set(CHECK_INCLUDE_DIRS ${CHECK_INCLUDE_DIR})
endif ()

mark_as_advanced(CHECK_INCLUDE_DIR CHECK_LIBRARY)