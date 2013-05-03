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

if(WIN32)
  set(SHELL_PREFIX "C:\\MinGW\\msys\\1.0\\bin\\sh")
endif()

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am ALL_TESTS)
string(REGEX MATCHALL "[A-Z_]*_TESTS " ALL_TESTS "${ALL_TESTS}")
string(REGEX REPLACE "_TESTS "            ";" ALL_TESTS "${ALL_TESTS}")
foreach(testgroup ${ALL_TESTS} )
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am ONE_GROUP)
  string(REGEX MATCH "${testgroup}_TESTS = [a-z0-9_ \\\n\t]*" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE "${testgroup}_TESTS = "             "" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE "[\\\n\t]"                          "" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE " "                                ";" ONE_GROUP "${ONE_GROUP}")
  foreach(testcase ${ONE_GROUP} )
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

    # Check for test cases that cause a "hang" in test case execution.
    # Each of them has to be analysed until all of them disappear from this list.
    if(${testcase} STREQUAL getline2 OR ${testcase} STREQUAL litoct    OR
       ${testcase} STREQUAL nonl     OR ${testcase} STREQUAL poundbang OR
       ${testcase} STREQUAL beginfile1 OR ${testcase} STREQUAL manyfiles OR
       ${testcase} STREQUAL rsstart2 OR ${testcase} STREQUAL strftime OR
       ${testcase} STREQUAL readdir
      )
      add_test(${testcase} ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}_FAILS  ${options} )
    else()
      add_test(${testcase} ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}  ${options} )
    endif()
  endforeach(testcase)

endforeach(testgroup)
