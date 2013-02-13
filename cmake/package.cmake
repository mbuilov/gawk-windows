#
# cmake/package --- CMake input file for gawk
#
# Copyright (C) 2013
# the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

## process this file with CMake to produce Makefile

SET(CPACK_GENERATOR "TGZ")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is GNU Awk ${GAWK_VERSION}")
SET(CPACK_PACKAGE_NAME "gawk")
SET(CPACK_PACKAGE_VERSION "${GAWK_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${GAWK_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${GAWK_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${GAWK_BUGFIX_VERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README")
SET(CPACK_PACKAGING_INSTALL_PREFIX /usr)

INCLUDE(CPack)
