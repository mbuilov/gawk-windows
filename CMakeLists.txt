#
# CMakeLists.txt --- CMake input file for gawk
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

cmake_minimum_required (VERSION 2.6)
project (gawk C)

include(cmake/configure.cmake)

set (EXTRA_LIBS "")

if (${HAVE_MPFR})
  set (EXTRA_LIBS ${EXTRA_LIBS} mpfr gmp)
endif ()
if (${DYNAMIC})
  set (EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_DL_LIBS} )
endif ()

include_directories(${CMAKE_SOURCE_DIR})

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  # awk.h changed
  # pc/gawkmisc.pc:605 execvp raus
  set (GAWK_SOURCES ${GAWK_SOURCES} regex.c pc/getid.c)
  add_definitions(-D HAVE_WINT_T)
  add_definitions(-D HAVE_ISWUPPER)
  add_definitions(-D HAVE_SETENV)
  add_definitions(-D MB_CUR_MAX)
  add_definitions(-D STDC_HEADERS)
endif()

set (GAWK_SOURCES ${GAWK_SOURCES}
  array.c
  builtin.c 
  cint_array.c 
  command.c
  debug.c
  dfa.c
  eval.c 
  ext.c 
  field.c 
  floatcomp.c
  gawkapi.c 
  gawkmisc.c 
  int_array.c
  io.c 
  main.c 
  mpfr.c 
  msg.c 
  node.c 
  profile.c 
  random.c 
  re.c 
  replace.c 
  str_array.c 
  symbol.c 
  version.c 
)

add_executable (gawk ${GAWK_SOURCES} ${BISON_awkgram_OUTPUTS})
target_link_libraries (gawk m ${EXTRA_LIBS})
install(PROGRAMS ${CMAKE_BINARY_DIR}/gawk DESTINATION bin)

if (CMAKE_HOST_UNIX)
  # Beware: before building the extension, -DGAWK gets undefined.
  add_subdirectory(extension)

  if(NOT ${CMAKE_CROSSCOMPILING} STREQUAL "TRUE")
    enable_testing()
    add_subdirectory(test)
    if (LATEX_COMPILER)
      add_subdirectory(doc)
    endif()
  endif()

  include(InstallRequiredSystemLibraries)
  set(CPACK_PACKAGING_INSTALL_PREFIX /usr)
  include(cmake/package.cmake)
endif()
