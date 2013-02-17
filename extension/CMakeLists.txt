#
# extension/CMakeLists.txt --- CMake input file for gawk
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

remove_definitions(-DGAWK)

MACRO(BuildExtension name sources)
  add_library (${name} MODULE ${sources})
  target_link_libraries(${name})
  set_target_properties(${name} PROPERTIES PREFIX "")
  install(PROGRAMS ${CMAKE_BINARY_DIR}/extension/${name}${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION lib)
ENDMACRO(BuildExtension)

BuildExtension(filefuncs filefuncs.c stack.c gawkfts.c)

if (${HAVE_FNMATCH} AND ${HAVE_FNMATCH_H})
  BuildExtension(fnmatch fnmatch.c)
else()
  message(WARNING "extension fnmatch cannot be built because function fnmatch or fnmatch.h is missing")
endif()

BuildExtension(fork fork.c)

BuildExtension(inplace inplace.c)

BuildExtension(ordchr ordchr.c)

if (${HAVE_DIRENT_H})
  BuildExtension(readdir readdir.c)
else()
  message(WARNING "extension readdir cannot be built because function readdir is missing")
endif()

BuildExtension(readfile readfile.c)

BuildExtension(revoutput revoutput.c)

if (${HAVE_GETDTABLESIZE})
  BuildExtension(revtwoway revtwoway.c)
else()
  message(WARNING "extension revtwoway cannot be built because function getdtablesize is missing")
endif()

BuildExtension(rwarray rwarray.c)

BuildExtension(time time.c)

BuildExtension(testext testext.c)

