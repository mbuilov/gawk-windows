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

add_library (filefuncs MODULE filefuncs.c stack.c gawkfts.c)
target_link_libraries (filefuncs)
set_target_properties(filefuncs PROPERTIES PREFIX "")

DefineFunctionIfAvailable(fnmatch HAVE_FNMATCH)
DefineHFileIfAvailable(fnmatch.h HAVE_FNMATCH_H)
if (${HAVE_FNMATCH} AND ${HAVE_FNMATCH_H})
  add_library (fnmatch MODULE fnmatch.c)
  target_link_libraries (fnmatch)
  set_target_properties(fnmatch PROPERTIES PREFIX "")
else()
  message(WARNING "extension fnmatch cannot be built because function fnmatch or fnmatch.h is missing")
endif()

add_library (fork MODULE fork.c)
target_link_libraries (fork)
set_target_properties(fork PROPERTIES PREFIX "")

add_library (inplace MODULE inplace.c)
target_link_libraries (inplace)
set_target_properties(inplace PROPERTIES PREFIX "")

add_library (ordchr MODULE ordchr.c)
target_link_libraries (ordchr)
set_target_properties(ordchr PROPERTIES PREFIX "")

DefineHFileIfAvailable(dirent.h HAVE_DIRENT_H)
if (${HAVE_DIRENT_H})
  add_library (readdir MODULE readdir.c)
  target_link_libraries (readdir)
set_target_properties(readdir PROPERTIES PREFIX "")
else()
  message(WARNING "extension readdir cannot be built because function readdir is missing")
endif()

add_library (readfile MODULE readfile.c)
target_link_libraries (readfile)
set_target_properties(readfile PROPERTIES PREFIX "")

add_library (revoutput MODULE revoutput.c)
target_link_libraries (revoutput)
set_target_properties(revoutput PROPERTIES PREFIX "")

DefineFunctionIfAvailable(getdtablesize HAVE_GETDTABLESIZE)
if (${HAVE_GETDTABLESIZE})
  add_library (revtwoway MODULE revtwoway.c)
  target_link_libraries (revtwoway)
  set_target_properties(revtwoway PROPERTIES PREFIX "")
else()
  message(WARNING "extension revtwoway cannot be built because function getdtablesize is missing")
endif()

add_library (rwarray MODULE rwarray.c)
target_link_libraries (rwarray)
set_target_properties(rwarray PROPERTIES PREFIX "")

DefineFunctionIfAvailable(select HAVE_SELECT)
DefineFunctionIfAvailable(gettimeofday HAVE_GETTIMEOFDAY)
DefineHFileIfAvailable(sys/select.h HAVE_SYS_SELECT_H)
DefineFunctionIfAvailable(nanosleep HAVE_NANOSLEEP)
DefineHFileIfAvailable(time.h HAVE_TIME_H)
DefineFunctionIfAvailable(GetSystemTimeAsFileTime HAVE_GETSYSTEMTIMEASFILETIME)
add_library (time MODULE time.c)
target_link_libraries (time)
set_target_properties(time PROPERTIES PREFIX "")

add_library (testext MODULE testext.c)
target_link_libraries (testext)
set_target_properties(testext PROPERTIES PREFIX "")

