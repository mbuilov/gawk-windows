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
if (${HAVE_LIBREADLINE})
  set (EXTRA_LIBS ${EXTRA_LIBS} readline)
endif ()
if (${DYNAMIC})
  set (EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_DL_LIBS} )
endif ()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/support)

if(WIN32 OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  # This is enough to build with MinGW in a native Windows environment
  # and also with a cross-compiler on OpenSuSE 12.2.
  # On Ubuntu 12.04 patches to gawk's source code are needed:
  #  - insert #include <windows.h> at the top of awk.h
  #  - remove function execvp from pc/gawkmisc.pc
  DefineConfigHValue(HAVE_SETENV 1)
  DefineConfigHValue(HAVE_USLEEP 1)
  DefineConfigHValue(STDC_HEADERS 1)
  DefineConfigHValue(HAVE_STRINGIZE 1)
  include_directories(${CMAKE_SOURCE_DIR}/missing_d)
  DefineConfigHValue(HAVE_MKSTEMP 1)
  set (EXTRA_LIBS ${EXTRA_LIBS} ws2_32)
  # TODO: Eli Zaretskii remined me that the generated
  # settings in config.h should be the same as those in
  # pc/config.h. With these settings and DYNAMIC=1
  # it looks like functions in dynamic libs (extensions) can
  # be invoked on Windows.
  DefineConfigHValue(HAVE_GETSYSTEMTIMEASFILETIME 1)
  set (GAWK_SOURCES ${GAWK_SOURCES} regex.c pc/getid.c pc/gawkmisc.pc pc/popen.c)
  include_directories(${CMAKE_SOURCE_DIR}/pc)
endif()

add_subdirectory(support)

set (GAWK_SOURCES ${GAWK_SOURCES}
  array.c
  builtin.c 
  cint_array.c 
  command.c
  debug.c
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
  re.c 
  replace.c 
  str_array.c 
  symbol.c 
  version.c 
)

add_executable (gawk ${GAWK_SOURCES} ${BISON_awkgram_OUTPUTS})
target_link_libraries (gawk m support ${EXTRA_LIBS})
install(PROGRAMS ${CMAKE_BINARY_DIR}/gawk${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)

# Beware: before building the extension, -DGAWK gets undefined.
add_subdirectory(extension)
enable_testing()
add_subdirectory(test)
add_subdirectory(doc)
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGING_INSTALL_PREFIX /usr)
include(cmake/package.cmake)
