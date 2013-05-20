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
  foreach(testcase ${ONE_GROUP} )
    add_test("${testgroup}.${testcase}" ${SHELL_PREFIX} ${CMAKE_SOURCE_DIR}/cmake/basictest ${CMAKE_BINARY_DIR}/gawk${CMAKE_EXECUTABLE_SUFFIX} ${testcase})
  endforeach(testcase)
endforeach(testgroup)

# Create an empty configuration file for customizing test execution.
set(CTestCustom ${CMAKE_BINARY_DIR}/CTestCustom.cmake)
file(WRITE  ${CTestCustom} "# DO NOT EDIT, THIS FILE WILL BE OVERWRITTEN\n" )
# Test case SHLIB.filefuncs needs a file named gawkapi.o in source directory.
file(APPEND ${CTestCustom} "file(COPY ${CMAKE_SOURCE_DIR}/README DESTINATION  ${CMAKE_SOURCE_DIR}/gawkapi.o)\n")
# Exclude test cases from execution that make no sense on a certain platform.
file(APPEND ${CTestCustom} "set(CTEST_CUSTOM_TESTS_IGNORE\n")
if(WIN32)
  file(APPEND ${CTestCustom} "  BASIC.exitval2\n")
  file(APPEND ${CTestCustom} "  BASIC.hsprint\n")
  file(APPEND ${CTestCustom} "  BASIC.rstest4\n")
  file(APPEND ${CTestCustom} "  BASIC.rstest5\n")
  file(APPEND ${CTestCustom} "  UNIX.getlnhd\n")
  file(APPEND ${CTestCustom} "  UNIX.pid\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.beginfile1\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.beginfile2\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.clos1way\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.devfd\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.devfd1\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.devfd2\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.getlndir\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.posix\n")
  file(APPEND ${CTestCustom} "  GAWK_EXT.pty1\n")
  file(APPEND ${CTestCustom} "  INET.inetdayu\n")
  file(APPEND ${CTestCustom} "  INET.inetdayt\n")
  file(APPEND ${CTestCustom} "  INET.inetechu\n")
  file(APPEND ${CTestCustom} "  INET.inetecht\n")
  file(APPEND ${CTestCustom} "  MACHINE.double2\n")
  file(APPEND ${CTestCustom} "  LOCALE_CHARSET.fmttest\n")
  file(APPEND ${CTestCustom} "  LOCALE_CHARSET.lc_num1\n")
  file(APPEND ${CTestCustom} "  LOCALE_CHARSET.mbfw1\n")
  file(APPEND ${CTestCustom} "  SHLIB.filefuncs\n")
  file(APPEND ${CTestCustom} "  SHLIB.fnmatch\n")
  file(APPEND ${CTestCustom} "  SHLIB.fork\n")
  file(APPEND ${CTestCustom} "  SHLIB.fork2\n")
  file(APPEND ${CTestCustom} "  SHLIB.fts\n")
  file(APPEND ${CTestCustom} "  SHLIB.functab4\n")
  file(APPEND ${CTestCustom} "  SHLIB.readdir\n")
  file(APPEND ${CTestCustom} "  SHLIB.revtwoway\n")
  file(APPEND ${CTestCustom} "  SHLIB.rwarray\n")
endif()
file(APPEND ${CTestCustom} ")\n")

