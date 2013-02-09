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

add_library (filefuncs SHARED filefuncs.c stack.c gawkfts.c)
target_link_libraries (filefuncs)

DefineFunctionIfAvailable(fnmatch HAVE_FNMATCH_H)
add_library (fnmatch SHARED fnmatch.c)
target_link_libraries (fnmatch)

add_library (fork SHARED fork.c)
target_link_libraries (fork)

add_library (inplace SHARED inplace.c)
target_link_libraries (inplace)

add_library (ordchr SHARED ordchr.c)
target_link_libraries (ordchr)

DefineHFileIfAvailable(dirent.h HAVE_DIRENT_H)
add_library (readdir SHARED readdir.c)
target_link_libraries (readdir)

add_library (readfile SHARED readfile.c)
target_link_libraries (readfile)

add_library (revoutput SHARED revoutput.c)
target_link_libraries (revoutput)

DefineFunctionIfAvailable(getdtablesize HAVE_GETDTABLESIZE)
add_library (revtwoway SHARED revtwoway.c)
target_link_libraries (revtwoway)

add_library (rwarray SHARED rwarray.c)
target_link_libraries (rwarray)

add_library (time SHARED time.c)
target_link_libraries (time)

add_library (testext SHARED testext.c)
target_link_libraries (testext)

