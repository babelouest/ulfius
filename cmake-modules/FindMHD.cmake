#.rst:
# FindJansson
# -----------
#
# Find libmicrohttpd
#
# Find libmicrohttpd headers and libraries.
#
# ::
#
#   MHD_FOUND          - True if libmicrohttpd found.
#   MHD_INCLUDE_DIRS   - Where to find microhttpd.h.
#   MHD_LIBRARIES      - List of libraries when using libmicrohttpd.
#   MHD_VERSION_STRING - The version of libmicrohttpd found.

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
pkg_check_modules(PC_MHD QUIET libmicrohttpd)

find_path(MHD_INCLUDE_DIR
        NAMES microhttpd.h
        HINTS ${PC_MHD_INCLUDEDIR} ${PC_MHD_INCLUDE_DIRS})

find_library(MHD_LIBRARY
        NAMES libmicrohttpd microhttpd
        HINTS ${PC_MHD_LIBDIR} ${PC_MHD_LIBRARY_DIRS})

if (PC_MHD_VERSION)
    set(MHD_VERSION_STRING ${PC_MHD_VERSION})
elseif (MHD_INCLUDE_DIR AND EXISTS "${MHD_INCLUDE_DIR}/microhttpd.h")
    set(regex_mhd_version "^#define[ \t]+MHD_VERSION[ \t]+([^\"]+).*")
    file(STRINGS "${MHD_INCLUDE_DIR}/microhttpd.h" mhd_version REGEX "${regex_mhd_version}")
    string(REGEX REPLACE "${regex_mhd_version}" "\\1" MHD_VERSION_NUM "${mhd_version}")
    unset(regex_mhd_version)
    unset(mhd_version)
    # parse MHD_VERSION from numerical format 0x12345678 to string format "12.34.56.78" so the version value can be compared to the one returned by pkg-config
    string(SUBSTRING ${MHD_VERSION_NUM} 2 2 MHD_VERSION_STRING_MAJOR)
    string(SUBSTRING ${MHD_VERSION_NUM} 4 2 MHD_VERSION_STRING_MINOR)
    string(SUBSTRING ${MHD_VERSION_NUM} 6 2 MHD_VERSION_STRING_REVISION)
    string(SUBSTRING ${MHD_VERSION_NUM} 8 2 MHD_VERSION_STRING_PATCH)
    set(MHD_VERSION_STRING "${MHD_VERSION_STRING_MAJOR}.${MHD_VERSION_STRING_MINOR}.${MHD_VERSION_STRING_REVISION}.${MHD_VERSION_STRING_PATCH}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mhd
        REQUIRED_VARS MHD_LIBRARY MHD_INCLUDE_DIR
        VERSION_VAR MHD_VERSION_STRING)

if (MHD_FOUND)
    set(MHD_LIBRARIES ${MHD_LIBRARY})
    set(MHD_INCLUDE_DIRS ${MHD_INCLUDE_DIR})
endif ()

mark_as_advanced(MHD_INCLUDE_DIR MHD_LIBRARY)
