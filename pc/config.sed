# -config.sed-----------------------------------------------------------
# Configuration script for pc/config.h
# ----------------------------------------------------------------------

# Copyright (C) 2011, 2016-2019 the Free Software Foundation, Inc.

# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.

# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

# ----------------------------------------------------------------------

/configh\.in/a\
/* pc/config.h.  Generated automatically by pc/config.sed.  */

/^#undef DYNAMIC$/c\
#ifdef _WIN32\
#define DYNAMIC 1\
#endif
s/^#undef GETPGRP_VOID *$/#define GETPGRP_VOID 1/
s/^#undef GETGROUPS_T *$/#define GETGROUPS_T gid_t/
/^#undef GETPGRP_VOID$/c\
#ifdef __DJGPP__\
#define GETPGRP_VOID 1\
#endif
s/^#undef HAVE_ALARM *$/#define HAVE_ALARM 1/
s/^#undef HAVE_ATEXIT *$/#define HAVE_ATEXIT 1/
/^#undef HAVE_BTOWC *$/c\
#define HAVE_BTOWC 1
/^#undef HAVE_DECL_TZNAME *$/c\
#ifdef __MINGW32__\
#define HAVE_DECL_TZNAME 1\
#endif
s/^#undef HAVE_FCNTL_H *$/#define HAVE_FCNTL_H 1/
s/^#undef HAVE_FMOD *$/#define HAVE_FMOD 1/
/^#undef HAVE_GAI_STRERROR *$/c\
#ifdef __MINGW32__\
#define HAVE_GAI_STRERROR 1\
#endif
/^#undef HAVE_GETADDRINFO *$/c\
#ifdef __MINGW32__\
#define HAVE_GETADDRINFO 1\
#endif
/^#undef HAVE_INTMAX_T *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_INTMAX_T 1\
#endif
/^#undef HAVE_INTTYPES_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_INTTYPES_H 1\
#endif
/^#undef HAVE_INTTYPES_H_WITH_UINTMAX *$/c\
#ifdef __MINGW32__\
#define HAVE_INTTYPES_H_WITH_UINTMAX 1\
#endif
/^#undef HAVE_ISASCII *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_ISASCII 1\
#endif
/^#undef HAVE_ISWCTYPE *$/c\
#ifdef __MINGW32__\
#define HAVE_ISWCTYPE 1\
#endif
/^#undef HAVE_ISWLOWER *$/c\
#ifdef __MINGW32__\
#define HAVE_ISWLOWER 1\
#endif
/^#undef HAVE_ISWUPPER *$/c\
#ifdef __MINGW32__\
#define HAVE_ISWUPPER 1\
#endif
/^#undef HAVE_LANGINFO_CODESET *$/c\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define HAVE_LANGINFO_CODESET 1\
#endif
s/^#undef HAVE_LIBM *$/#define HAVE_LIBM 1/
/^#undef HAVE_LIBREADLINE *$/c\
/* #undef HAVE_LIBREADLINE */
s/^#undef HAVE_LIMITS_H *$/#define HAVE_LIMITS_H 1/
/^#undef HAVE_LOCALE_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_LOCALE_H 1\
#endif
/^#undef HAVE_LONG_LONG *$/c\
#ifdef __MINGW32__\
#define HAVE_LONG_LONG 1\
#endif
/^#undef HAVE_MBRLEN *$/c\
#ifdef __MINGW32__\
#define HAVE_MBRLEN 1\
#endif
/^#undef HAVE_MBRTOWC *$/c\
#ifdef __MINGW32__\
#define HAVE_MBRTOWC 1\
#endif
s/^#undef HAVE_MEMCMP *$/#define HAVE_MEMCMP 1/
s/^#undef HAVE_MEMCPY *$/#define HAVE_MEMCPY 1/
/^#undef HAVE_MEMMOVE *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_MEMMOVE 1\
#endif
/^#undef HAVE_MEMORY_H *$/c\
#ifdef __DJGPP__\
#define HAVE_MEMORY_H 1\
#endif
s/^#undef HAVE_MEMSET *$/#define HAVE_MEMSET 1/
/^#undef HAVE_MKSTEMP *$/c\
#ifdef __DJGPP__\
#define HAVE_MKSTEMP 1\
#endif
s/^#undef HAVE_MKTIME *$/#define HAVE_MKTIME 1/
/^#undef HAVE_MPFR *$/c\
/* #undef HAVE_MPFR */
/^#undef HAVE_SETENV *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_SETENV 1\
#endif
/^#undef HAVE_SETLOCALE *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_SETLOCALE 1\
#endif
/^#undef HAVE_SNPRINTF *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_SNPRINTF 1\
#endif
/^#undef HAVE_SOCKADDR_STORAGE *$/c\
#ifdef __MINGW32__\
#define HAVE_SOCKADDR_STORAGE 1\
#endif
/^#undef HAVE_SOCKETS *$/c\
#ifdef __MINGW32__\
#define HAVE_SOCKETS 1\
#endif
s/^#undef HAVE_STDARG_H *$/#define HAVE_STDARG_H 1/
/^#undef HAVE_STDBOOL_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_STDBOOL_H 1\
#endif
/^#undef HAVE_STDDEF_H *$/c\
#ifdef __GNUC__\
#define HAVE_STDDEF_H 1\
#endif
/^#undef HAVE_STDINT_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_STDINT_H 1\
#endif
/^#undef HAVE_STDLIB_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_STDLIB_H 1\
#endif
/^#undef HAVE_STRCASECMP *$/c\
#ifdef __DJGPP__\
#define HAVE_STRCASECMP 1\
#endif
/^#undef HAVE_STRCOLL *$/c\
#ifdef __DJGPP__\
#define HAVE_STRCOLL 1\
#endif
s/^#undef HAVE_STRCHR *$/#define HAVE_STRCHR 1/
s/^#undef HAVE_STRERROR *$/#define HAVE_STRERROR 1/
/^#undef HAVE_STRFTIME *$/c\
#ifdef __MINGW32__\
/* MinGW uses the replacement from missing_d, to support the %e specifier.  */\
#define strftime rpl_strftime\
#else\
#define HAVE_STRFTIME 1\
#endif
s/^#undef HAVE_STRINGIZE *$/#define HAVE_STRINGIZE 1/
s/^#undef HAVE_STRINGS_H *$/#define HAVE_STRINGS_H 1/
s/^#undef HAVE_STRING_H *$/#define HAVE_STRING_H 1/
/^#undef HAVE_STRNCASECMP *$/c\
#define HAVE_STRNCASECMP 1\
#ifdef __EMX__\
#define strncasecmp strnicmp\
#endif
s/^#undef HAVE_STRTOD *$/#define HAVE_STRTOD 1/
/^#undef HAVE_STRTOUL *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_STRTOUL 1\
#endif
/^#undef HAVE_STRUCT_STAT_ST_BLKSIZE *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1\
#endif
/^#undef HAVE_SYS_IOCTL_H *$/c\
#ifdef __DJGPP__\
#define HAVE_SYS_IOCTL_H 1\
#endif
s/^#undef HAVE_SYSTEM *$/#define HAVE_SYSTEM 1/
/^#undef HAVE_SYS_PARAM_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_SYS_PARAM_H 1\
#endif
/^#undef HAVE_SYS_STAT_H *$/c\
#if defined(__MINGW32__) || defined(__DJGPP__)\
#define HAVE_SYS_STAT_H 1\
#endif
/^#undef HAVE_SYS_TIME_H *$/c\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define HAVE_SYS_TIME_H 1\
#endif
s/^#undef HAVE_SYS_TYPES_H *$/#define HAVE_SYS_TYPES_H 1/
/^#undef HAVE_SYS_WAIT_H *$/c\
#ifdef __DJGPP__\
#define HAVE_SYS_WAIT_H 1\
#endif
/^#undef HAVE_TMPFILE *$/c\
#ifdef __DJGPP__\
#define HAVE_TMPFILE 1\
#endif
/^#undef HAVE_TOWLOWER *$/c\
#ifdef __MINGW32__\
#define HAVE_TOWLOWER 1\
#endif
/^#undef HAVE_TOWUPPER *$/c\
#ifdef __MINGW32__\
#define HAVE_TOWUPPER 1\
#endif
s/^#undef HAVE_TZNAME *$/#define HAVE_TZNAME 1/
s/^#undef HAVE_TZSET *$/#define HAVE_TZSET 1/
/^#undef HAVE_UINTMAX_T *$/c\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define HAVE_UINTMAX_T 1\
#endif
/^#undef HAVE_UNISTD_H *$/c\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define HAVE_UNISTD_H 1\
#endif
s/^#undef HAVE_UNSIGNED_LONG_LONG *$/#define HAVE_UNSIGNED_LONG_LONG 1/
/^#undef HAVE_USLEEP *$/c\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define HAVE_USLEEP 1\
#endif
s/^#undef HAVE_VPRINTF *$/#define HAVE_VPRINTF 1/
/^#undef HAVE_WCHAR_H *$/c\
#ifdef __MINGW32__\
#define HAVE_WCHAR_H 1\
#endif
/^#undef HAVE_WCRTOMB *$/c\
#ifdef __MINGW32__\
#define HAVE_WCRTOMB 1\
#endif
/^#undef HAVE_WCSCOLL *$/c\
#ifdef __MINGW32__\
#define HAVE_WCSCOLL 1\
#endif
/^#undef HAVE_WCTYPE *$/c\
#ifdef __MINGW32__\
#define HAVE_WCTYPE 1\
#endif
/^#undef HAVE_WCTYPE_H *$/c\
#ifdef __MINGW32__\
#define HAVE_WCTYPE_H 1\
#endif
/^#undef HAVE_WCTYPE_T *$/c\
#ifdef __MINGW32__\
#define HAVE_WCTYPE_T 1\
#endif
/^#undef HAVE_WINT_T *$/c\
#ifdef __MINGW32__\
#define HAVE_WINT_T 1\
#endif
/^#undef HAVE__BOOL *$/c\
#ifdef __DJGPP__\
#define HAVE__BOOL 1\
#endif
/^#undef PRINTF_HAS_A_FORMAT *$/c\
#define PRINTF_HAS_A_FORMAT 1
/^#undef PRINTF_HAS_F_FORMAT *$/c\
#ifdef __DJGPP__\
#define PRINTF_HAS_F_FORMAT 1\
#endif
s/^#undef PROTOTYPES *$/#define PROTOTYPES 1/
s/^#undef RETSIGTYPE *$/#define RETSIGTYPE void/
/^#.*RETSIGTYPE /a\
\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#include <limits.h>\
#endif
s/^#undef SIZEOF_UNSIGNED_INT *$/#define SIZEOF_UNSIGNED_INT 4/
s/^#undef SIZEOF_UNSIGNED_LONG *$/#define SIZEOF_UNSIGNED_LONG 4/
s/^#undef STDC_HEADERS *$/#define STDC_HEADERS 1/
s/^#undef TIME_WITH_SYS_TIME *$/#define TIME_WITH_SYS_TIME 1/
/^#undef inline *$/c\
#ifdef __GNUC__\
#define inline __inline__\
#endif

/^\/\* Enable extensions on AIX 3, Interix.  \*\//i\
/* This is required to compile Gnulib regex code.  */\
#if defined(__DJGPP__) || defined(__MINGW32__)\
#define _GNU_SOURCE 1\
#endif

s|^#undef PACKAGE_URL *$|#define PACKAGE_URL "http://www.gnu.org/software/gawk/"|

$a\
#ifdef __DJGPP__\
/* gcc no longer includes this by default */\
# include <sys/version.h>\
\
/* Library search path */\
# if (__DJGPP__ > 2 || __DJGPP_MINOR__ >= 3)\
#  define DEFPATH  ".;/dev/env/DJDIR/share/awk"\
# else\
#  define DEFPATH  ".;c:/lib/awk;c:/gnu/lib/awk"\
# endif\
\
/* Function prototype.  */\
#include <stdbool.h>\
extern bool is_valid_identifier(const char *name);\
#endif\
\
#ifndef __DJGPP__\
#define HAVE_POPEN_H 1\
#endif\
\
#if defined(__EMX__)\
#define strcasecmp stricmp\
#define strncasecmp strnicmp\
#endif
