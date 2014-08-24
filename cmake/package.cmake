#
# cmake/package --- CMake input file for gawk
#
# Copyright (C) 2013-2014
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

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is GNU Awk ${GAWK_VERSION}")
set(CPACK_PACKAGE_VENDOR "GNU Project - Free Software Foundation (FSF)")
SET(CPACK_PACKAGE_NAME "gawk")
SET(CPACK_PACKAGE_VERSION "${GAWK_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR "${GAWK_MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${GAWK_MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${GAWK_BUGFIX_VERSION}")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_RESOURCE_FILE_README  "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_CONTACT "bug-gawk@gnu.org")

IF (WIN32)
  SET(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_INSTALL_ROOT "C:")
  set(CPACK_NSIS_MENU_LINKS "http://www.gnu.org/software/gawk" "GNU Awk")
  set(CPACK_NSIS_MUI_ICON    "${CMAKE_SOURCE_DIR}/cmake/auk.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/cmake/auk.ico")
  set(CPACK_NSIS_MODIFY_PATH true)
  set(CPACK_NSIS_CONTACT "bug-gawk@gnu.org")
  set(CPACK_NSIS_DISPLAY_NAME "GNU Awk")
  include(CPack)
ELSE()
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_PACKAGING_INSTALL_PREFIX /usr)
  if     (CPACK_GENERATOR STREQUAL "TGZ")
    include(CPack)
  elseif (CPACK_GENERATOR STREQUAL "RPM")
    include(CPack)
  elseif (CPACK_GENERATOR STREQUAL "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "interpreters")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.gnu.org/software/gawk")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12)")
    include(CPack)
  else()
    # No package format selected. Don't create the "package" target.
  endif()
  if     (CPACK_GENERATOR)
    message(STATUS "CPACK_GENERATOR set to ${CPACK_GENERATOR}")
  endif()
ENDIF() 

