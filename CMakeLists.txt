cmake_minimum_required (VERSION 2.6)
project (gawk C)

include(CheckIncludeFiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckStructHasMember)

MACRO(DefineFunctionIfAvailable func feature)
  check_function_exists("${func}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature})
  ENDIF ()
ENDMACRO(DefineFunctionIfAvailable)

MACRO(DefineHFileIfAvailable hfile feature)
  check_include_file("${hfile}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature})
  ENDIF ()
ENDMACRO(DefineHFileIfAvailable)

MACRO(DefineTypeIfAvailable type feature)
  check_type_size("${type}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature}=${${feature}})
  ENDIF ()
ENDMACRO(DefineTypeIfAvailable)

MACRO(DefineSymbolIfAvailable symbol hfile feature)
  check_symbol_exists("${symbol}" "${hfile}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature})
  ENDIF ()
ENDMACRO(DefineSymbolIfAvailable)

MACRO(DefineStructHasMemberIfAvailable struct member hfile feature)
  check_struct_has_member("${struct}" "${member}" "${hfile}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature})
  ENDIF ()
ENDMACRO(DefineStructHasMemberIfAvailable)

MACRO(DefineLibraryIfAvailable lib func location feature)
  check_library_exists("${lib}" "${func}" "${location}" "${feature}")
  IF (${feature})
    ADD_DEFINITIONS (-D ${feature})
  ENDIF ()
ENDMACRO(DefineLibraryIfAvailable)

file( WRITE config.h "/* empty file, all settings defined by CMake. */" )
# Configure a header file to pass some of the CMake settings
# to the source code
# http://www.cmake.org/cmake/help/v2.8.8/cmake.html#command:configure_file
# CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h IMMEDIATE )

FILE( READ  configure.ac CONFIG_AUTOMAKE )
STRING( REGEX MATCH "AC_INIT\\(\\[GNU Awk\\], ([0-9]+\\.[0-9]+\\.[0-9]+)" GAWK_AUTOMAKE_LINE_VERSION "${CONFIG_AUTOMAKE}") 
STRING( REGEX REPLACE ".*([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" GAWK_MAJOR_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 
STRING( REGEX REPLACE ".*[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" GAWK_MINOR_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 
STRING( REGEX REPLACE ".*[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GAWK_BUGFIX_VERSION "${GAWK_AUTOMAKE_LINE_VERSION}") 

add_definitions(-D GAWK)
set(GAWK_VERSION "${GAWK_MAJOR_VERSION}.${GAWK_MINOR_VERSION}.${GAWK_BUGFIX_VERSION}")
add_definitions(-D VERSION=\\"${GAWK_VERSION}\\")
add_definitions(-D PACKAGE=\\"gawk\\")
add_definitions(-D PACKAGE_STRING="GNU Awk ${GAWK_VERSION}")
add_definitions(-D PACKAGE_TARNAME=\\"gawk\\")
add_definitions(-D PACKAGE_URL=\\"http://www.gnu.org/software/gawk/\\")
add_definitions(-D PACKAGE_VERSION=\\"${GAWK_VERSION}\\")
add_definitions(-D DEFPATH=\\"${CMAKE_BINARY_DIR}/awk\\")
add_definitions(-D DEFLIBPATH=\\"${CMAKE_BINARY_DIR}/lib\\")
#add_definitions(-D EXTLIB_SUFFIX=\\"lib\\")
add_definitions(-D SHLIBEXT=\\".so\\")
DefineTypeIfAvailable("unsigned int" SIZEOF_UNSIGNED_INT)
DefineTypeIfAvailable("unsigned long" SIZEOF_UNSIGNED_LONG)
#/* Define to 1 if *printf supports %F format */
add_definitions(-D PRINTF_HAS_F_FORMAT)
#/* Define as the return type of signal handlers (`int' or `void'). */
add_definitions(-D RETSIGTYPE=void)
#add_definitions(-D PIPES_SIMULATED)
add_definitions(-D GETPGRP_VOID)
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
DefineFunctionIfAvailable(gettext HAVE_GETTEXT)
DefineFunctionIfAvailable(dcgettext HAVE_DCGETTEXT)
DefineSymbolIfAvailable("CODESET" "langinfo.h" HAVE_LANGINFO_CODESET)
DefineSymbolIfAvailable("LC_MESSAGES" "locale.h" HAVE_LC_MESSAGES)
DefineTypeIfAvailable("_Bool" HAVE__BOOL)
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
#DefineTypeIfAvailable("long long int"   HAVE_LONG_LONG_INT)
#add_definitions(-D HAVE_UNSIGNED_LONG_LONG_INT)
DefineTypeIfAvailable(intmax_t INTMAX_T)
DefineTypeIfAvailable(uintmax_t UINTMAX_T)

# These dont work, maybe CheckCSourceCompiles would be better.
add_definitions(-D TIME_T_IN_SYS_TYPES_H)
#DefineTypeIfAvailable("time_t" TIME_T_IN_SYS_TYPES_H)
DefineTypeIfAvailable("wctype_t" WCTYPE_T)
add_definitions(-D WINT_T)
#DefineTypeIfAvailable("wint_t" WINT_T)
add_definitions(-D HAVE_SOCKADDR_STORAGE)
#DefineTypeIfAvailable("struct sockaddr_storage" SOCKADDR_STORAGE)
add_definitions(-D HAVE_STRUCT_STAT_ST_BLKSIZE)
#DefineStructHasMemberIfAvailable("struct stat" st_blksize bits/stat.h HAVE_STRUCT_STAT_ST_BLKSIZE)
add_definitions(-D HAVE_ST_BLKSIZE)
#DefineStructHasMemberIfAvailable("struct stat" st_blksize bits/stat.h HAVE_ST_BLKSIZE)
add_definitions(-D HAVE_TM_ZONE)
#DefineStructHasMemberIfAvailable("struct tm" tm_zone time.h HAVE_TM_ZONE)
add_definitions(-D HAVE_STRUCT_TM_TM_ZONE)
#DefineStructHasMemberIfAvailable("struct tm" tm_zone time.h HAVE_STRUCT_TM_TM_ZONE)

DefineHFileIfAvailable(sys/time.h HAVE_SYS_TIME_H)
DefineFunctionIfAvailable(alarm HAVE_ALARM)
DefineFunctionIfAvailable(mktime HAVE_MKTIME)
DefineFunctionIfAvailable(getaddrinfo HAVE_GETADDRINFO)
DefineFunctionIfAvailable(atexit HAVE_ATEXIT)
DefineFunctionIfAvailable(btowc HAVE_BTOWC)
add_definitions(-D HAVE_FMOD)
#DefineFunctionIfAvailable(fmod HAVE_FMOD)
DefineFunctionIfAvailable(isinf HAVE_ISINF)
DefineFunctionIfAvailable(ismod HAVE_ISMOD)
DefineFunctionIfAvailable(getgrent HAVE_GETGRENT)
DefineFunctionIfAvailable(getgroups HAVE_GETGROUPS)
add_definitions(-D GETGROUPS_T=gid_t)
DefineTypeIfAvailable("pid_t"   PID_T)
DefineFunctionIfAvailable(grantpt HAVE_GRANTPT)
DefineFunctionIfAvailable(isascii HAVE_ISASCII)
DefineFunctionIfAvailable(iswctype HAVE_ISWCTYPE)
DefineFunctionIfAvailable(iswlower HAVE_ISWLOWER)
DefineFunctionIfAvailable(iswupper HAVE_ISUPPER)
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

set (EXTRA_LIBS "")

DefineLibraryIfAvailable(m    sin         "" HAVE_LIBM)
DefineLibraryIfAvailable(mpfr mpfr_add_si "" HAVE_MPFR)
DefineLibraryIfAvailable(c    socket      "" HAVE_SOCKETS)

if (${HAVE_MPFR})
  set (EXTRA_LIBS ${EXTRA_LIBS} mpfr gmp)
endif ()

include_directories(${CMAKE_SOURCE_DIR})

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  # awk.h changed
  # pc/gawkmisc.pc:605 execvp raus
  set (GAWK_SOURCES ${GAWK_SOURCES} regex.c pc/getid.c)
  add_definitions(-D HAVE_WINT_T)
  add_definitions(-D HAVE_ISWUPPER)
  add_definitions(-D HAVE_SETENV)
  add_definitions(-D MB_CUR_MAX)
  add_definitions(-D STDC_HEADERS)
endif()

set (GAWK_SOURCES ${GAWK_SOURCES}
  array.c
  awkgram.c
  builtin.c 
  cint_array.c 
  command.c
  debug.c
  dfa.c
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
  random.c 
  re.c 
  replace.c 
  str_array.c 
  symbol.c 
  version.c 
)

add_executable (gawk ${GAWK_SOURCES})
target_link_libraries (gawk m ${EXTRA_LIBS})

