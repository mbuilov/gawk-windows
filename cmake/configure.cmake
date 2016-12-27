#
# cmake/configure --- CMake input file for gawk
#
# Copyright (C) 2013-2014
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

option (USE_CONFIG_H          "Generate a file config.h for inclusion into C source code" ON)
if (USE_CONFIG_H)
  file( WRITE config.h "/* all settings defined by CMake. */\n\n" )
  ADD_DEFINITIONS (-D HAVE_CONFIG_H)
  # Configure a header file to pass some of the CMake settings
  # to the source code
  # http://www.cmake.org/cmake/help/v2.8.8/cmake.html#command:configure_file
  # CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h IMMEDIATE )
else()
  file( WRITE config.h "/* empty file, all settings defined by CMake. */" )
endif()

include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructHasMember)
INCLUDE(CheckCSourceCompiles)
include(CheckPrototypeDefinition)

MACRO(DefineConfigH feature)
#  message(STATUS feature=${feature}=${${feature}})
  if (${feature})
    if (${USE_CONFIG_H} STREQUAL ON)
      FILE( APPEND config.h "#define ${feature} ${${feature}}\n")
    else()
      #ADD_DEFINITIONS (-D ${feature})
      ADD_DEFINITIONS (-D${feature}=${${feature}})
    endif ()
  endif ()
ENDMACRO(DefineConfigH)

MACRO(DefineConfigHValue feature value)
  set(${feature} ${value})
  DefineConfigH(${feature})
ENDMACRO(DefineConfigHValue)

MACRO(DefineFunctionIfAvailable func feature)
  check_function_exists("${func}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineFunctionIfAvailable)

MACRO(DefineHFileIfAvailable hfile feature)
  check_include_file("${hfile}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineHFileIfAvailable)

MACRO(DefineTypeIfAvailable type feature)
  check_type_size("${type}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineTypeIfAvailable)

MACRO(DefineSymbolIfAvailable symbol hfile feature)
  check_symbol_exists("${symbol}" "${hfile}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineSymbolIfAvailable)

MACRO(DefineStructHasMemberIfAvailable struct member hfile feature)
  check_struct_has_member("${struct}" "${member}" "${hfile}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineStructHasMemberIfAvailable)

MACRO(DefineLibraryIfAvailable lib func location feature)
  check_library_exists("${lib}" "${func}" "${location}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineLibraryIfAvailable)

MACRO(DefineIfSourceCompiles source feature)
  check_c_source_compiles( "${source}" "${feature}")
  DefineConfigH(${feature})
ENDMACRO(DefineIfSourceCompiles)

FILE( READ  ${CMAKE_SOURCE_DIR}/configure.ac CONFIG_AUTOMAKE )
STRING( REGEX MATCH "AC_INIT\\(\\[GNU Awk\\], ([0-9]+\\.[0-9]+\\.[0-9]+)" GAWK_AUTOMAKE_LINE_VERSION "${CONFIG_AUTOMAKE}") 
STRING( REGEX REPLACE ".*([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" GAWK_MAJOR_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 
STRING( REGEX REPLACE ".*[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" GAWK_MINOR_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 
STRING( REGEX REPLACE ".*[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GAWK_BUGFIX_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 

# The definition of the symbol GAWK cannot be passed in config.h
# because the extensions will fail to build.
add_definitions(-DGAWK)
add_definitions(-DSTDC_HEADERS)
# Tell the C compiler to accept C99.
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "--std=gnu99 ${CMAKE_C_FLAGS}")
  endif ()
else ()
  set (CMAKE_C_STANDARD 99)
endif ()
DefineConfigHValue(_GL_ATTRIBUTE_PURE "__attribute__ ((__pure__))")
DefineConfigHValue(GAWK_VERSION "${GAWK_MAJOR_VERSION}.${GAWK_MINOR_VERSION}.${GAWK_BUGFIX_VERSION}")
DefineConfigHValue(VERSION \\"${GAWK_VERSION}\\")
DefineConfigHValue(PACKAGE \\"gawk\\")
DefineConfigHValue(PACKAGE_STRING \\"GNU Awk ${GAWK_VERSION}\\")
DefineConfigHValue(PACKAGE_TARNAME \\"gawk\\")
DefineConfigHValue(PACKAGE_URL \\"http://www.gnu.org/software/gawk/\\")
DefineConfigHValue(PACKAGE_VERSION \\"${GAWK_VERSION}\\")
DefineConfigHValue(DEFPATH \\"${CMAKE_BINARY_DIR}/awk\\")
DefineConfigHValue(DEFLIBPATH \\"${CMAKE_BINARY_DIR}/lib\\")
if (CMAKE_DL_LIBS)
  message(STATUS "Found CMAKE_DL_LIBS:${CMAKE_DL_LIBS}")
else()
  message(STATUS "Found no CMAKE_DL_LIBS")
endif()
if (CMAKE_SHARED_LIBRARY_SUFFIX)
  DefineConfigHValue(DYNAMIC 1)
  STRING( REGEX REPLACE "^(\\.)([a-zA-Z0-9])" "\\2" SHLIBEXT "${CMAKE_SHARED_LIBRARY_SUFFIX}") 
  DefineConfigHValue(SHLIBEXT \\"${SHLIBEXT}\\")
  message(STATUS "Found SHLIBEXT: ${SHLIBEXT}")
else()
  message(STATUS "Found no SHLIBEXT")
endif()
DefineTypeIfAvailable("unsigned int" SIZEOF_UNSIGNED_INT)
DefineTypeIfAvailable("unsigned long" SIZEOF_UNSIGNED_LONG)
#/* Define to 1 if *printf supports %F format */
add_definitions(-D PRINTF_HAS_F_FORMAT)
#/* Define as the return type of signal handlers (`int' or `void'). */
add_definitions(-D RETSIGTYPE=void)
#add_definitions(-D PIPES_SIMULATED)
check_prototype_definition(getpgrp "pid_t getpgrp(void)" "NULL" "unistd.h" GETPGRP_VOID)
DefineConfigH(GETPGRP_VOID)
#add_definitions(-D YYPARSE_PARAM)

DefineFunctionIfAvailable(snprintf HAVE_SNPRINTF)
DefineFunctionIfAvailable(vprintf HAVE_VPRINTF)
DefineHFileIfAvailable(sys/types.h HAVE_SYS_TYPES_H)
DefineHFileIfAvailable(sys/stat.h HAVE_SYS_STAT_H)
DefineHFileIfAvailable(string.h HAVE_STRING_H)
DefineHFileIfAvailable(memory.h HAVE_MEMORY_H)
DefineHFileIfAvailable(strings.h HAVE_STRINGS_H)
DefineHFileIfAvailable(stdint.h HAVE_STDINT_H)
DefineHFileIfAvailable(inttypes.h HAVE_INTTYPES_H)
DefineHFileIfAvailable(stdlib.h HAVE_STDLIB_H)
DefineHFileIfAvailable(unistd.h HAVE_UNISTD_H)
FIND_PATH(INTL_INCLUDE_DIR libintl.h PATHS /usr/include /usr/local/include)
FIND_LIBRARY(INTL_LIBRARIES intl c PATHS /usr/lib/ /usr/local/lib)
DefineSymbolIfAvailable("CODESET" "langinfo.h" HAVE_LANGINFO_CODESET)
DefineSymbolIfAvailable("LC_MESSAGES" "locale.h" HAVE_LC_MESSAGES)
DefineTypeIfAvailable("_Bool" HAVE__BOOL)
if (${HAVE_GETTEXT} AND  ${HAVE_DCGETTEXT} AND ${HAVE_LANGINFO_CODESET} AND ${HAVE_LC_MESSAGES})
  add_definitions(-D LOCALEDIR=\\"/usr/share/locale\\")
  add_definitions(-D ENABLE_NLS)
  ADD_SUBDIRECTORY( ${CMAKE_SOURCE_DIR}/po )
endif()
DefineHFileIfAvailable(stdbool.h HAVE_STDBOOL_H)
DefineHFileIfAvailable(sys/wait.h HAVE_SYS_WAIT_H)
DefineHFileIfAvailable(arpa/inet.h HAVE_ARPA_INET_H)
DefineHFileIfAvailable(fcntl.h HAVE_FCNTL_H)
DefineHFileIfAvailable(limits.h HAVE_LIMITS_H)
DefineHFileIfAvailable(locale.h HAVE_LOCALE_H)
DefineHFileIfAvailable(libintl.h HAVE_LIBINTL_H)
DefineHFileIfAvailable(mcheck.h HAVE_MCHECK_H)
DefineHFileIfAvailable(netdb.h HAVE_NETDB_H)
DefineHFileIfAvailable(netinet/in.h HAVE_NETINET_IN_H)
DefineHFileIfAvailable(stdarg.h HAVE_STDARG_H)
DefineHFileIfAvailable(stddef.h HAVE_STDDEF_H)
DefineHFileIfAvailable(sys/ioctl.h HAVE_SYS_IOCTL_H)
DefineHFileIfAvailable(sys/param.h HAVE_SYS_PARAM_H)
DefineHFileIfAvailable(sys/socket.h HAVE_SYS_SOCKET_H)
DefineHFileIfAvailable(sys/termios.h HAVE_TERMIOS_H)
DefineHFileIfAvailable(stropts.h HAVE_STROPTS_H)
DefineHFileIfAvailable(wchar.h HAVE_WCHAR_H)
DefineHFileIfAvailable(wctype.h HAVE_WCTYPE_H)
DefineTypeIfAvailable("long long int" HAVE_LONG_LONG_INT)
DefineTypeIfAvailable("unsigned long long int" HAVE_UNSIGNED_LONG_LONG_INT)
DefineTypeIfAvailable(intmax_t INTMAX_T)
DefineTypeIfAvailable(uintmax_t UINTMAX_T)
DefineTypeIfAvailable("time_t" TIME_T_IN_SYS_TYPES_H)
SET(CMAKE_EXTRA_INCLUDE_FILES wctype.h)
DefineTypeIfAvailable("wctype_t" HAVE_WCTYPE_T)
DefineTypeIfAvailable("wint_t" HAVE_WINT_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)

DefineStructHasMemberIfAvailable("struct sockaddr_storage" ss_family sys/socket.h HAVE_SOCKADDR_STORAGE)
DefineStructHasMemberIfAvailable("struct stat" st_blksize sys/stat.h HAVE_STRUCT_STAT_ST_BLKSIZE)
DefineStructHasMemberIfAvailable("struct stat" st_blksize sys/stat.h HAVE_ST_BLKSIZE)
DefineStructHasMemberIfAvailable("struct tm" tm_zone time.h HAVE_TM_ZONE)
DefineStructHasMemberIfAvailable("struct tm" tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)

DefineHFileIfAvailable(sys/time.h HAVE_SYS_TIME_H)
DefineFunctionIfAvailable(alarm HAVE_ALARM)
DefineFunctionIfAvailable(tzname HAVE_DECL_TZNAME)
DefineFunctionIfAvailable(mktime HAVE_MKTIME)
DefineFunctionIfAvailable(getaddrinfo HAVE_GETADDRINFO)
DefineFunctionIfAvailable(atexit HAVE_ATEXIT)
DefineFunctionIfAvailable(btowc HAVE_BTOWC)
DefineFunctionIfAvailable(fmod HAVE_FMOD)
DefineFunctionIfAvailable(isinf HAVE_ISINF)
DefineFunctionIfAvailable(ismod HAVE_ISMOD)
DefineFunctionIfAvailable(getgrent HAVE_GETGRENT)
DefineSymbolIfAvailable("getgroups" "unistd.h" HAVE_GETGROUPS)
if (${HAVE_GETGROUPS})
  check_prototype_definition(getgroups "int getgroups(int size, gid_t list[])" "NULL" "unistd.h" GETGROUPS_T)
  if (${GETGROUPS_T})
    DefineConfigHValue(GETGROUPS_T gid_t)
  else()
    DefineConfigHValue(GETGROUPS_T int)
  endif()
endif()

DefineTypeIfAvailable("pid_t"   PID_T)
DefineTypeIfAvailable("intmax_t"   HAVE_INTMAX_T)
DefineFunctionIfAvailable(grantpt HAVE_GRANTPT)
DefineFunctionIfAvailable(isascii HAVE_ISASCII)
DefineFunctionIfAvailable(iswctype HAVE_ISWCTYPE)
DefineFunctionIfAvailable(iswlower HAVE_ISWLOWER)
DefineFunctionIfAvailable(iswupper HAVE_ISWUPPER)
DefineFunctionIfAvailable(mbrlen HAVE_MBRLEN)
DefineFunctionIfAvailable(memcmp HAVE_MEMCMP)
DefineFunctionIfAvailable(memcpy HAVE_MEMCPY)
DefineFunctionIfAvailable(memmove HAVE_MEMMOVE)
DefineFunctionIfAvailable(memset HAVE_MEMSET)
DefineFunctionIfAvailable(mkstemp HAVE_MKSTEMP)
DefineFunctionIfAvailable(posix_openpt HAVE_POSIX_OPENPT)
DefineFunctionIfAvailable(setenv HAVE_SETENV)
DefineFunctionIfAvailable(setlocale HAVE_SETLOCALE)
DefineFunctionIfAvailable(setsid HAVE_SETSID)
DefineFunctionIfAvailable(strchr HAVE_STRCHR)
DefineFunctionIfAvailable(strerror HAVE_STRERROR)
DefineFunctionIfAvailable(strftime HAVE_STRFTIME)
DefineFunctionIfAvailable(strncasecmp HAVE_STRNCASECMP)
DefineFunctionIfAvailable(strcoll HAVE_STRCOLL)
DefineFunctionIfAvailable(strtod HAVE_STRTOD)
DefineFunctionIfAvailable(strtoul HAVE_STRTOUL)
DefineFunctionIfAvailable(system HAVE_SYSTEM)
DefineFunctionIfAvailable(tmpfile HAVE_TMPFILE)
DefineFunctionIfAvailable(towlower HAVE_TOWLOWER)
DefineFunctionIfAvailable(towupper HAVE_TOWUPPER)
DefineFunctionIfAvailable(tzset HAVE_TZSET)
DefineFunctionIfAvailable(usleep HAVE_USLEEP)
DefineFunctionIfAvailable(wcrtomb HAVE_WCRTOMB)
DefineFunctionIfAvailable(wcscoll HAVE_WCSCOLL)
DefineFunctionIfAvailable(wctype HAVE_WCTYPE)
DefineFunctionIfAvailable(mbrtowc HAVE_MBRTOWC)

add_definitions(-D HAVE_STRINGIZE)
add_definitions(-D _Noreturn=)

find_package(BISON QUIET)
# If there is a bison installed on this platform,
if (${BISON_FOUND} STREQUAL "TRUE")
  # then let bison generate awkgram.c.
  BISON_TARGET(awkgram awkgram.y ${CMAKE_SOURCE_DIR}/awkgram.c)
else()
  # otherwise use the existing awkgram.c.
  set(BISON_awkgram_OUTPUTS ${CMAKE_SOURCE_DIR}/awkgram.c)
endif()

find_package(Gettext REQUIRED)
if (GETTEXT_FOUND STREQUAL "TRUE")
  include_directories(${GETTEXT_INCLUDE_DIR})
  DefineFunctionIfAvailable(gettext HAVE_GETTEXT)
  DefineFunctionIfAvailable(dcgettext HAVE_DCGETTEXT)
else ()
  message( FATAL_ERROR "Gettext not found" )
endif()

find_package(LATEX)
include(GNUInstallDirs)
include(GetPrerequisites)

# For some unknown reason the defines for the extensions
# are written into config.h only if they are implemented
# here and not in extension/CMakeLists.txt.
DefineLibraryIfAvailable(m    sin         "" HAVE_LIBM)
DefineLibraryIfAvailable(mpfr mpfr_add_si "" HAVE_MPFR)
DefineLibraryIfAvailable(c    socket      "" HAVE_SOCKETS)
DefineLibraryIfAvailable(readline readline    "" HAVE_LIBREADLINE)
DefineFunctionIfAvailable(fnmatch HAVE_FNMATCH)
DefineHFileIfAvailable(fnmatch.h HAVE_FNMATCH_H)
DefineHFileIfAvailable(dirent.h HAVE_DIRENT_H)
DefineFunctionIfAvailable(dirfd HAVE_DIRFD)
DefineFunctionIfAvailable(getdtablesize HAVE_GETDTABLESIZE)
DefineFunctionIfAvailable(select HAVE_SELECT)
DefineFunctionIfAvailable(gettimeofday HAVE_GETTIMEOFDAY)
DefineHFileIfAvailable(sys/select.h HAVE_SYS_SELECT_H)
DefineFunctionIfAvailable(nanosleep HAVE_NANOSLEEP)
DefineHFileIfAvailable(time.h HAVE_TIME_H)
DefineFunctionIfAvailable(GetSystemTimeAsFileTime HAVE_GETSYSTEMTIMEASFILETIME)

