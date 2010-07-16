/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*
 * acconfig.h -- configuration definitions for gawk.
 */

/*
 * Copyright (C) 1995-2005 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#define GETGROUPS_T gid_t

/* Define if the `getpgrp' function takes no argument.  */
#define GETPGRP_VOID 1

/* Define to 1 if you have the `atexit' function. */
# define HAVE_ATEXIT 1

/* Define to 1 if you have the `btowc' function. */
#ifdef _WIN32
#define HAVE_BTOWC 1
#endif

#ifdef __MINGW32__
/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
#define HAVE_DECL_TZNAME 1

/* Define if you have the 'intmax_t' type in <stdint.h> or <inttypes.h>. */
#define HAVE_INTMAX_T 1

/* Define if you have the 'uintmax_t' type in <stdint.h> or <inttypes.h>. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define if <inttypes.h> exists, doesn't clash with <sys/types.h>, and
   declares uintmax_t. */
#define HAVE_INTTYPES_H_WITH_UINTMAX 1
#endif

#ifdef __MINGW32__
/* Define to 1 if you have the `isascii' function. */
#define HAVE_ISASCII 1

/* Define to 1 if you have the `iswctype' function. */
#define HAVE_ISWCTYPE 1

/* Define to 1 if you have the `iswlower' function. */
#define HAVE_ISWLOWER 1

/* Define to 1 if you have the `iswupper' function. */
#define HAVE_ISWUPPER 1

/* Define if you have the 'long long' type. */
#define HAVE_LONG_LONG 1

/* Define to 1 if you have the `mbrlen' function. */
#define HAVE_MBRLEN 1

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
#define HAVE_MBRTOWC 1

/* Define to 1 if you have the `towlower' function. */
#define HAVE_TOWLOWER 1

/* Define to 1 if you have the `towupper' function. */
#define HAVE_TOWUPPER 1

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have the `wcrtomb' function. */
#define HAVE_WCRTOMB 1

/* Define to 1 if you have the `wcscoll' function. */
#define HAVE_WCSCOLL 1

/* Define to 1 if you have the `wctype' function. */
#define HAVE_WCTYPE 1

/* Define to 1 if you have the <wctype.h> header file. */
#define HAVE_WCTYPE_H 1

/* systems should define this type here */
#define HAVE_WCTYPE_T 1

/* systems should define this type here */
#define HAVE_WINT_T 1
#endif

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
/* #undef HAVE_ALLOCA_H */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have a working `mmap' system call.  */
/* #undef HAVE_MMAP */

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if you have the ANSI # stringizing operator in cpp. */
#define HAVE_STRINGIZE 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
/* #undef HAVE_SYS_WAIT_H */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
#if defined (_MSC_VER)
#define inline
#endif

#ifdef __GNUC__
#define inline __inline__
/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1
#endif

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

#define REGEX_MALLOC 1 /* use malloc instead of alloca in regex.c */
#define SPRINTF_RET int /* return type of sprintf */
#define HAVE_MKTIME    /* we have the mktime function */
/* #undef HAVE_SOCKETS */ /* we have sockets on this system */
/* #undef HAVE_PORTALS */ /* we have portals on /p on this system */
/* #undef DYNAMIC */  /* allow dynamic addition of builtins */
/* #undef STRTOD_NOT_C89 */ /* strtod doesn't have C89 semantics */

/* Define if you have the __argz_count function.  */
#define HAVE___ARGZ_COUNT 1

/* Define if you have the __argz_next function.  */
#define HAVE___ARGZ_NEXT 1

/* Define if you have the __argz_stringify function.  */
#define HAVE___ARGZ_STRINGIFY 1

/* Define if you have the alarm function.  */
#define HAVE_ALARM 1

/* Define if you have the dcgettext function.  */
/* #undef HAVE_DCGETTEXT */

/* Define if you have the fmod function.  */
#define HAVE_FMOD 1

/* Define if you have the getcwd function.  */
/* #undef HAVE_GETCWD */

/* Define if you have the getpagesize function.  */
/* #undef HAVE_GETPAGESIZE */

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

#ifdef __MINGW32__
/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1
#endif

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the munmap function.  */
/* #undef HAVE_MUNMAP */

#ifdef __MINGW32__
/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* FIXME!! */

/* we have sockets on this system */
#undef HAVE_SOCKETS

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H
#endif

/* Define if you have the setenv function.  */
/* #define HAVE_SETENV */

/* Define if you have the stpcpy function.  */
/* #undef HAVE_STPCPY */

/* Define if you have the strcasecmp function.  */
/* #undef HAVE_STRCASECMP */

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strtod function.  */
#define HAVE_STRTOD 1

#ifdef __MINGW32__

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1
#endif

/* Define if you have the system function.  */
#define HAVE_SYSTEM 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the <argz.h> header file.  */
/* #undef HAVE_ARGZ_H */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <libintl.h> header file.  */
/* #undef HAVE_LIBINTL_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <netdb.h> header file.  */
/* #undef HAVE_NETDB_H */

/* Define if you have the <netinet/in.h> header file.  */
/* #undef HAVE_NETINET_IN_H */

/* Define if you have the <nl_types.h> header file.  */
/* #undef HAVE_NL_TYPES_H */

/* Define if you have the <signum.h> header file.  */
/* #undef HAVE_SIGNUM_H */

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

#ifdef __MINGW32__
/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define if you have the 'unsigned long long' type. */
#define HAVE_UNSIGNED_LONG_LONG 1
#endif

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/socket.h> header file.  */
/* #undef HAVE_SYS_SOCKET_H */

/* Define if you have the <sys/time.h> header file.  */
#if defined(DJGPP)
# define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file.  */
#if defined(DJGPP) || defined(__MINGW32__)
# define HAVE_UNISTD_H 1
#endif

/* Define if you have the i library (-li).  */
/* #undef HAVE_LIBI */

/* Define if you have the intl library (-lintl).  */
/* #undef HAVE_LIBINTL */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Name of package */
#define PACKAGE "gawk"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-gawk@gnu.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GNU Awk"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GNU Awk 3.1.5a"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gawk"

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.1.5a"

/* Version number of package */
#define VERSION "3.1.5a"

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to make ftello visible on some hosts (e.g. HP-UX 10.20). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to make ftello visible on some hosts (e.g. glibc 2.1.3). */
/* #undef _XOPEN_SOURCE */

/* Define to 1 if *printf supports %F format */
/* #undef PRINTF_HAS_F_FORMAT */

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define to 1 if you have the stpcpy function. */
/* #undef HAVE_STPCPY */

/* Define if your locale.h file contains LC_MESSAGES. */
/* #undef HAVE_LC_MESSAGES */

/* Define to 1 if NLS is requested. */
/* #undef ENABLE_NLS */

/* Define to 1 if you have gettext and don't want to use GNU gettext. */
/* #undef HAVE_GETTEXT */

/* Define as 1 if you have catgets and don't want to use GNU gettext. */
/* #undef HAVE_CATGETS */

/* The size of `unsigned int' & `unsigned long', as computed by sizeof. */
#if defined(DJGPP) || defined(_MSC_VER) || defined(__MINGW32__)
# include <limits.h>
#endif

#if UINT_MAX == 65536
# define SIZEOF_UNSIGNED_INT 2
#endif

#if UINT_MAX == 4294967295U
# define SIZEOF_UNSIGNED_INT 4
#endif

#if ULONG_MAX == 4294967295UL
# define SIZEOF_UNSIGNED_LONG 4
#endif

/* Library search path */
#if defined(__DJGPP__) && (__DJGPP__ > 2 || __DJGPP_MINOR__ >= 3)
# define DEFPATH  ".;/dev/env/DJDIR/share/awk"
#else
# define DEFPATH  ".;c:/lib/awk;c:/gnu/lib/awk"
#endif

#if defined (_MSC_VER)
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#undef HAVE_UNISTD_H
#undef HAVE_SYS_PARAM_H
#undef HAVE_RANDOM
/* msc strftime is incomplete, use supplied version */
#undef HAVE_STRFTIME
/* #define HAVE_TM_ZONE */
#define altzone timezone
#endif

#define HAVE_POPEN_H 1

#if defined(_MSC_VER) && defined(MSDOS)
#define system(s) os_system(s)
#endif

#ifdef _MSC_VER
#define ssize_t long int /* DJGPP has ssize_t */
#define intmax_t long
#define uintmax_t unsigned long
#endif

#if defined (_MSC_VER) || defined(__EMX__)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#if defined(DJGPP)
# define HAVE_LIMITS_H 1
# undef HAVE_POPEN_H
# undef HAVE_ALLOCA
# define HAVE_MKSTEMP 1
#define intmax_t long long
#define uintmax_t unsigned long long
#define restrict /* nothing */
#endif

#if defined(__WIN32__) && defined(__CRTRSXNT__)
#include <crtrsxnt.h>
#endif

/* For vcWin32 */
#if defined(WIN32) && defined(_MSC_VER)
#define alloca _alloca
#define system(s) os_system(s)
/* VC++ dosen't supprt restrict keyword */
#define restrict
/* VC++ have tmpfile */
#define HAVE_TMPFILE
/* sys/time.h is not exist in VC++? */
#undef TIME_WITH_SYS_TIME
#endif

#if defined(__MINGW32__)
#undef HAVE_SYS_PARAM_H
#endif


/* #define NO_LINT 1 */
