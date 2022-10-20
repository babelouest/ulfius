#.rst:
# FindGnuTLS
# -----------
#
# Find GnuTLS
#
# Find Yder headers and libraries.
#
# ::
#
#   GNUTLS_FOUND          - True if GnuTLS found.
#   GNUTLS_INCLUDE_DIRS   - Where to find gnutls/gnutls.h.
#   GNUTLS_LIBRARIES      - List of libraries when using GnuTLS.
#   GNUTLS_VERSION_STRING - The version of GnuTLS found.

#=============================================================================
# Copyright 2022 Nicolas Mora <mail@babelouest.org>
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
pkg_check_modules(PC_GNUTLS QUIET gnutls)

find_path(GNUTLS_INCLUDE_DIR
        NAMES gnutls/gnutls.h
        HINTS ${PC_GNUTLS_INCLUDEDIR} ${PC_GNUTLS_INCLUDE_DIRS})

find_library(GNUTLS_LIBRARY
        NAMES gnutls libgnutls
        HINTS ${PC_GNUTLS_LIBDIR} ${PC_GNUTLS_LIBRARY_DIRS})

set(GNUTLS_VERSION_STRING 0.0.0)
if (PC_GNUTLS_VERSION)
    set(GNUTLS_VERSION_STRING ${PC_GNUTLS_VERSION})
elseif (GNUTLS_INCLUDE_DIR AND EXISTS "${GNUTLS_INCLUDE_DIR}/gnutls/gnutls.h")
    set(regex_gnutls_version "^#define[ \t]+GNUTLS_VERSION[ \t]+([^\"]+).*")
    file(STRINGS "${GNUTLS_INCLUDE_DIR}/gnutls/gnutls.h" gnutls_version REGEX "${regex_gnutls_version}")
    string(REGEX REPLACE "${regex_gnutls_version}" "\\1" GNUTLS_VERSION_STRING "${gnutls_version}")
    unset(regex_gnutls_version)
    unset(gnutls_version)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GnuTLS
        REQUIRED_VARS GNUTLS_LIBRARY GNUTLS_INCLUDE_DIR
        VERSION_VAR GNUTLS_VERSION_STRING)

if (PC_GNUTLS_FOUND)
    set(GNUTLS_FOUND 1)
endif ()

if (GNUTLS_FOUND)
    set(GNUTLS_LIBRARIES ${GNUTLS_LIBRARY})
    set(GNUTLS_INCLUDE_DIRS ${GNUTLS_INCLUDE_DIR})
    if (NOT TARGET GnuTLS::GnuTLS)
        add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
        set_target_properties(GnuTLS::GnuTLS PROPERTIES
                IMPORTED_LOCATION "${GNUTLS_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${GNUTLS_INCLUDE_DIR}")
    endif ()
endif ()

mark_as_advanced(GNUTLS_INCLUDE_DIR GNUTLS_LIBRARY)
