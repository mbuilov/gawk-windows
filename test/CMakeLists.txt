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

# Find the names of the groups of tests in Makefile.am.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am ALL_GROUPS)
string(REGEX MATCHALL "[A-Z_]*_TESTS "                      ALL_GROUPS "${ALL_GROUPS}")
string(REGEX REPLACE "_TESTS "                          ";" ALL_GROUPS "${ALL_GROUPS}")
# For each group of test cases, search through Makefile.am and find the test cases.
foreach(testgroup ${ALL_GROUPS} )
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am ONE_GROUP)
  string(REGEX MATCH "${testgroup}_TESTS = [a-z0-9_ \\\n\t]*" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE "${testgroup}_TESTS = "             "" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE "[\\\n\t]"                          "" ONE_GROUP "${ONE_GROUP}")
  string(REGEX REPLACE " "                                ";" ONE_GROUP "${ONE_GROUP}")
  # Use each name of a test case to start a script that executes the test case.
  # Some test cases are special, treat them accordingly.
  foreach(testcase ${ONE_GROUP} )
    set(options "")
    set(file_suffix  "")
    if(${testcase} STREQUAL lintold)
      set(options "--lint-old")
    elseif(
       ${testcase} STREQUAL defref     OR ${testcase} STREQUAL fmtspcl       OR
       ${testcase} STREQUAL lintwarn   OR ${testcase} STREQUAL noeffect      OR
       ${testcase} STREQUAL nofmtch    OR ${testcase} STREQUAL shadow        OR
       ${testcase} STREQUAL uninit2    OR ${testcase} STREQUAL uninit3       OR
       ${testcase} STREQUAL uninit4    OR ${testcase} STREQUAL uninit5       OR
       ${testcase} STREQUAL uninitialized)
      set(options "--lint")
    # These are the test cases that fail.
    # Some of them may reveal genuine bugs.
    # Most of them fail because they need to be invoked with a special parameter.
    elseif(
       ${testcase} STREQUAL argarray   OR ${testcase} STREQUAL argtest       OR
       ${testcase} STREQUAL arynocls   OR ${testcase} STREQUAL awkpath       OR
       ${testcase} STREQUAL badargs    OR ${testcase} STREQUAL beginfile2    OR
       ${testcase} STREQUAL binmode1   OR ${testcase} STREQUAL charasbytes   OR
       ${testcase} STREQUAL colonwarn  OR ${testcase} STREQUAL compare       OR
       ${testcase} STREQUAL devfd      OR ${testcase} STREQUAL devfd1        OR
       ${testcase} STREQUAL devfd2     OR ${testcase} STREQUAL dumpvars      OR
       ${testcase} STREQUAL exit       OR ${testcase} STREQUAL exitval1      OR
       ${testcase} STREQUAL fflush     OR ${testcase} STREQUAL fmtspcl       OR
       ${testcase} STREQUAL fsspcoln   OR
       ${testcase} STREQUAL gsubtst3   OR ${testcase} STREQUAL incdupe       OR
       ${testcase} STREQUAL incdupe2   OR ${testcase} STREQUAL incdupe3      OR
       ${testcase} STREQUAL incdupe4   OR ${testcase} STREQUAL incdupe5      OR
       ${testcase} STREQUAL incdupe6   OR ${testcase} STREQUAL incdupe7      OR
       ${testcase} STREQUAL include2   OR ${testcase} STREQUAL inetdayt      OR
       ${testcase} STREQUAL inetdayu   OR ${testcase} STREQUAL inetecht      OR
       ${testcase} STREQUAL inetechu   OR ${testcase} STREQUAL inftest       OR
       ${testcase} STREQUAL jarebug    OR
       ${testcase} STREQUAL leaddig    OR ${testcase} STREQUAL localenl      OR
       ${testcase} STREQUAL mbfw1      OR ${testcase} STREQUAL mbprintf1     OR
       ${testcase} STREQUAL messages   OR ${testcase} STREQUAL mmap8k        OR
       ${testcase} STREQUAL next       OR ${testcase} STREQUAL nofile        OR
       ${testcase} STREQUAL nondec2    OR ${testcase} STREQUAL nors          OR
       ${testcase} STREQUAL pid        OR
       ${testcase} STREQUAL pipeio2    OR ${testcase} STREQUAL posix2008sub  OR
       ${testcase} STREQUAL printf0    OR ${testcase} STREQUAL printfbad2    OR
       ${testcase} STREQUAL profile1   OR ${testcase} STREQUAL profile2      OR
       ${testcase} STREQUAL profile3   OR
       ${testcase} STREQUAL redfilnm   OR ${testcase} STREQUAL regtest       OR
       ${testcase} STREQUAL rsnulbig   OR ${testcase} STREQUAL rsnulbig2     OR
       ${testcase} STREQUAL rsstart3   OR ${testcase} STREQUAL rtlen         OR
       ${testcase} STREQUAL rtlen01    OR ${testcase} STREQUAL rtlenmb       OR
       ${testcase} STREQUAL space      OR ${testcase} STREQUAL strftlng      OR
       ${testcase} STREQUAL symtab6    OR ${testcase} STREQUAL symtab8
      )
      set(file_suffix  "_FAILS")
    endif()

    if ("${file_suffix}" STREQUAL "")
      add_test("${testgroup}.${testcase}" ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}${file_suffix}  ${options} )
    else()
      message(STATUS "Skipping test case ${testgroup}.${testcase}")
    endif()
  endforeach(testcase)

endforeach(testgroup)
