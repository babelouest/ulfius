#
# Ulfius Framework
#
# Makefile used to build all programs
#
# Copyright 2014-2017 Nicolas Mora <mail@babelouest.org>
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
#

LIBULFIUS_LOCATION=./src
EXAMPLES_LOCATION=./example_programs
UWSC_LOCATION=./tools/uwsc
TESTS_LOCATION=./test

all:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) $*
	cd $(UWSC_LOCATION) && $(MAKE) $*

debug:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) debug $*
	cd $(UWSC_LOCATION) && $(MAKE) debug $*

clean:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) clean
	cd $(EXAMPLES_LOCATION) && $(MAKE) clean
	cd $(UWSC_LOCATION) && $(MAKE) clean
	cd $(TESTS_LOCATION) && $(MAKE) clean
	rm -rf doc/API.md doc/html

examples:
	cd $(EXAMPLES_LOCATION) && $(MAKE) $*

install:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) install
	cd $(UWSC_LOCATION) && $(MAKE) install

uninstall:
	cd $(LIBULFIUS_LOCATION) && $(MAKE) uninstall

check:
	cd $(TESTS_LOCATION) && $(MAKE)

doxygen:
	echo "# Ulfius API Documentation" > doc/API.md
	echo "" >> doc/API.md
	tail -n +$(shell grep -n "## Header file" API.md | cut -f1 -d:) API.md >> doc/API.md
	doxygen doc/doxygen.cfg
