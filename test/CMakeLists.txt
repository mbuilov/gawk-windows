#
# test/CMakeLists.txt --- CMake input file for gawk
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

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Maketests BASIC_TESTS)
string(REGEX REPLACE "^Gt-dummy:\n"   "" BASIC_TESTS "${BASIC_TESTS}")
string(REGEX MATCHALL "[a-zA-Z0-9_]+:\n" BASIC_TESTS "${BASIC_TESTS}")
string(REGEX REPLACE ":\n"            "" BASIC_TESTS "${BASIC_TESTS}")
foreach(testcase ${BASIC_TESTS} )
  set(options "")
  if(${testcase} STREQUAL lintold)
    set(options "--lint-old")
  endif()
  if(${testcase} STREQUAL defref   OR ${testcase} STREQUAL fmtspcl  OR
     ${testcase} STREQUAL lintwarn OR ${testcase} STREQUAL noeffect OR
     ${testcase} STREQUAL nofmtch  OR ${testcase} STREQUAL shadow   OR
     ${testcase} STREQUAL uninit2  OR ${testcase} STREQUAL uninit3  OR
     ${testcase} STREQUAL uninit4  OR ${testcase} STREQUAL uninit5  OR
     ${testcase} STREQUAL uninitialized)
    set(options "--lint")
  endif()

  add_test(${testcase} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}  ${options} )
endforeach(testcase)

