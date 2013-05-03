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
    set(suffix  "")
    if(${testcase} STREQUAL lintold)
      set(options "--lint-old")
      add_test(${testcase} ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}  ${options} )
    endif()
    if(${testcase} STREQUAL defref     OR ${testcase} STREQUAL fmtspcl       OR
       ${testcase} STREQUAL lintwarn   OR ${testcase} STREQUAL noeffect      OR
       ${testcase} STREQUAL nofmtch    OR ${testcase} STREQUAL shadow        OR
       ${testcase} STREQUAL uninit2    OR ${testcase} STREQUAL uninit3       OR
       ${testcase} STREQUAL uninit4    OR ${testcase} STREQUAL uninit5       OR
       ${testcase} STREQUAL uninitialized)
      set(options "--lint")
    # Check for test cases that cause a "hang" in test case execution.
    # Each of them has to be analysed until all of them disappear from this list.
    elseif(
       ${testcase} STREQUAL getline2   OR ${testcase} STREQUAL litoct        OR
       ${testcase} STREQUAL nonl       OR ${testcase} STREQUAL poundbang     OR
       ${testcase} STREQUAL beginfile1 OR ${testcase} STREQUAL manyfiles     OR
       ${testcase} STREQUAL rsstart2   OR ${testcase} STREQUAL strftime      OR
       ${testcase} STREQUAL readdir
      )
      set(suffix  "_HANGS")
    # These are the test cases that fail.
    # Some of them may reveal genuine bugs.
    # Most of them need fail because they need to be invoked with a special parameter.
    elseif(
       ${testcase} STREQUAL argarray   OR ${testcase} STREQUAL argtest       OR
       ${testcase} STREQUAL arynocls   OR ${testcase} STREQUAL _awkpath      OR
       ${testcase} STREQUAL badargs    OR ${testcase} STREQUAL _beginfile2   OR
       ${testcase} STREQUAL binmode1   OR ${testcase} STREQUAL _charasbytes  OR
       ${testcase} STREQUAL colonwarn  OR ${testcase} STREQUAL _compare      OR
       ${testcase} STREQUAL devfd      OR ${testcase} STREQUAL _devfd1       OR
       ${testcase} STREQUAL devfd2     OR ${testcase} STREQUAL _dumpvars     OR
       ${testcase} STREQUAL exit       OR ${testcase} STREQUAL _exitval1     OR
       ${testcase} STREQUAL fflush     OR ${testcase} STREQUAL _fmtspcl      OR
       ${testcase} STREQUAL fsspcoln   OR ${testcase} STREQUAL _fts          OR
       ${testcase} STREQUAL gsubtst3   OR ${testcase} STREQUAL _incdupe      OR
       ${testcase} STREQUAL incdupe2   OR ${testcase} STREQUAL _incdupe3     OR
       ${testcase} STREQUAL incdupe4   OR ${testcase} STREQUAL _incdupe5     OR
       ${testcase} STREQUAL incdupe6   OR ${testcase} STREQUAL _incdupe7     OR
       ${testcase} STREQUAL include2   OR ${testcase} STREQUAL _inetdayt     OR
       ${testcase} STREQUAL inetdayu   OR ${testcase} STREQUAL _inetecht     OR
       ${testcase} STREQUAL inetechu   OR ${testcase} STREQUAL _inftest      OR
       ${testcase} STREQUAL inplace1   OR ${testcase} STREQUAL _inplace2     OR
       ${testcase} STREQUAL inplace3   OR ${testcase} STREQUAL _jarebug      OR
       ${testcase} STREQUAL leaddig    OR ${testcase} STREQUAL _localenl     OR
       ${testcase} STREQUAL mbfw1      OR ${testcase} STREQUAL _mbprintf1    OR
       ${testcase} STREQUAL messages   OR ${testcase} STREQUAL _mmap8k       OR
       ${testcase} STREQUAL mpfrbigint OR ${testcase} STREQUAL _mpfrexprange OR
       ${testcase} STREQUAL mpfrnr     OR ${testcase} STREQUAL _mpfrrnd      OR
       ${testcase} STREQUAL next       OR ${testcase} STREQUAL _nofile       OR
       ${testcase} STREQUAL nondec2    OR ${testcase} STREQUAL _nors         OR
       ${testcase} STREQUAL ordchr2    OR ${testcase} STREQUAL _pid          OR
       ${testcase} STREQUAL pipeio2    OR ${testcase} STREQUAL _posix2008sub OR
       ${testcase} STREQUAL printf0    OR ${testcase} STREQUAL _printfbad2   OR
       ${testcase} STREQUAL profile1   OR ${testcase} STREQUAL _profile2     OR
       ${testcase} STREQUAL profile3   OR ${testcase} STREQUAL _readfile     OR
       ${testcase} STREQUAL redfilnm   OR ${testcase} STREQUAL _regtest      OR
       ${testcase} STREQUAL rsnulbig   OR ${testcase} STREQUAL _rsnulbig2    OR
       ${testcase} STREQUAL rsstart3   OR ${testcase} STREQUAL _rtlen        OR
       ${testcase} STREQUAL rtlen01    OR ${testcase} STREQUAL _rtlenmb      OR
       ${testcase} STREQUAL space      OR ${testcase} STREQUAL _strftlng     OR
       ${testcase} STREQUAL symtab6    OR ${testcase} STREQUAL _symtab8      OR
       ${testcase} STREQUAL testext
      )
      set(suffix  "_FAILS")
    endif()

    add_test(${testcase} ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk ${testcase}${suffix}  ${options} )
  endforeach(testcase)

endforeach(testgroup)
