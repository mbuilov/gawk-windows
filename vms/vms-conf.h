#ifndef CONFIG_H
#define CONFIG_H
/*
 * config.h -- configuration definitions for gawk.
 *
 * For VMS (assumes V4.6 or later; tested on V7.3-1, V8.3.
 */

/* 
 * Copyright (C) 1991-1992, 1995-1996, 1999, 2001, 2002, 2003, 2005
 * the Free Software Foundation, Inc.
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

/* switch statements are enabled in awk programs */
#undef ALLOW_SWITCH

#if 0		/* no longer used */
/* Define to 1 if using alloca.c.  */
#define C_ALLOCA 1
#else
#define NO_ALLOCA	/* vms/vms_fwrite.c needs this */
/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
#define STACK_DIRECTION (-1)
#endif /*0*/

/* dynamic loading is possible */
#undef DYNAMIC

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#undef ENABLE_NLS

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or `gid_t'. */
#define GETGROUPS_T int

/* Define to 1 if the `getpgrp' function requires zero arguments. */
#define GETPGRP_VOID	1

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#undef HAVE_ARPA_INET_H

/* Define to 1 if you have the `btowc' function. */
#undef HAVE_BTOWC

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
#undef HAVE_CFLOCALECOPYCURRENT

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
#undef HAVE_CFPREFERENCESCOPYAPPVALUE

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#undef HAVE_DCGETTEXT

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
#undef HAVE_DECL_TZNAME

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#undef HAVE_DOPRNT

/* Define to 1 if you have the <fcntl.h> header file. */
#undef HAVE_FCNTL_H

/* Define to 1 if you have the `fmod' function. */
#define HAVE_FMOD 1

/* have getaddrinfo */
#undef HAVE_GETADDRINFO

/* Define to 1 if you have the `getgrent' function. */
#undef HAVE_GETGRENT

/* Define to 1 if you have the `getgroups' function. */
#undef HAVE_GETGROUPS

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Define to 1 if you have the `grantpt' function. */
#undef HAVE_GRANTPT

/* Define if you have the iconv() function. */
#undef HAVE_ICONV

/* Define if you have the 'intmax_t' type in <stdint.h> or <inttypes.h>. */
#undef HAVE_INTMAX_T

/* Define to 1 if you have the <inttypes.h> header file. */
#undef HAVE_INTTYPES_H

/* Define if <inttypes.h> exists, doesn't clash with <sys/types.h>, and
   declares uintmax_t. */
#undef HAVE_INTTYPES_H_WITH_UINTMAX

/* Define to 1 if you have the `isascii' function. */
#define HAVE_ISASCII 1

/* Define to 1 if you have the `iswctype' function. */
#undef HAVE_ISWCTYPE

/* Define to 1 if you have the `iswlower' function. */
#undef HAVE_ISWLOWER

/* Define to 1 if you have the `iswupper' function. */
#undef HAVE_ISWUPPER

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#undef HAVE_LANGINFO_CODESET

/* Define if your <locale.h> file defines LC_MESSAGES. */
#undef HAVE_LC_MESSAGES

/* Define to 1 if you have the <libintl.h> header file. */
#undef HAVE_LIBINTL_H

/* Define to 1 if you have the `m' library (-lm). */
#undef HAVE_LIBM

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <locale.h> header file. */
#undef HAVE_LOCALE_H

/* Define if you have the 'long long' type. */
#undef HAVE_LONG_LONG

/* Define to 1 if the system has the type `long long int'. */
#undef HAVE_LONG_LONG_INT

/* Define to 1 if you have the `mbrlen' function. */
#undef HAVE_MBRLEN

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
#undef HAVE_MBRTOWC

/* Define to 1 if you have the <mcheck.h> header file. */
#undef HAVE_MCHECK_H

/* Define to 1 if you have the `memcmp' function. */
#define HAVE_MEMCMP 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the `memcpy_ulong' function. */
#undef HAVE_MEMCPY_ULONG

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#undef HAVE_MEMORY_H

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `memset_ulong' function. */
#undef HAVE_MEMSET_ULONG

/* Define to 1 if you have the `mkstemp' function. */
#undef HAVE_MKSTEMP

/* we have the mktime function */
#define HAVE_MKTIME 1

/* Define to 1 if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* we have portals on /p on this system */
#undef HAVE_PORTALS

/* Define to 1 if you have the `setlocale' function. */
#undef HAVE_SETLOCALE

/* Define to 1 if you have the <signum.h> header file. */
#undef HAVE_SIGNUM_H

/* Define to 1 if you have the `snprintf' function. */
#undef HAVE_SNPRINTF

/* newer systems define this type here */
#undef HAVE_SOCKADDR_STORAGE

/* we have sockets on this system */
#undef HAVE_SOCKETS

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#undef HAVE_STDINT_H

/* Define if <stdint.h> exists, doesn't clash with <sys/types.h>, and declares
   uintmax_t. */
#undef HAVE_STDINT_H_WITH_UINTMAX

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#undef HAVE_STRFTIME		/* use the missing_d/strfime.c version */

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#ifdef VAXC
#undef HAVE_STRINGIZE
#else
#define HAVE_STRINGIZE 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#undef HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#undef HAVE_STRNCASECMP

/* Define to 1 if you have the <stropts.h> header file. */
#undef HAVE_STROPTS_H

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if `st_blksize' is member of `struct stat'. */
#undef HAVE_STRUCT_STAT_ST_BLKSIZE

/* Define to 1 if `tm_zone' is member of `struct tm'. */
#undef HAVE_STRUCT_TM_TM_ZONE

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
#undef HAVE_ST_BLKSIZE

/* Define to 1 if you have the `system' function. */
#define HAVE_SYSTEM 1

/* Define to 1 if you have the <sys/param.h> header file. */
#undef HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#undef HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#undef HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#undef HAVE_SYS_TYPES_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#undef HAVE_SYS_WAIT_H

/* Define to 1 if you have the <termios.h> header file. */
#undef HAVE_TERMIOS_H

/* Define to 1 if you have the `tmpfile' function. */
#define HAVE_TMPFILE 1
/* Force snprintf.c to use tmpfile() instead of mkstemp(). */
#ifdef HAVE_MKSTEMP
#undef HAVE_MKSTEMP
#endif

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#undef HAVE_TM_ZONE

/* Define to 1 if you have the `towlower' function. */
#undef HAVE_TOWLOWER

/* Define to 1 if you have the `towupper' function. */
#undef HAVE_TOWUPPER

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#define HAVE_TZNAME 1		/* (faked in vms/vms_misc.c) */

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1		/* (faked in vms/vms_misc.c) */

/* Define if you have the 'uintmax_t' type in <stdint.h> or <inttypes.h>. */
#undef HAVE_UINTMAX_T

/* Define to 1 if you have the <unistd.h> header file. */
#ifdef __DECC
#define HAVE_UNISTD_H 1
#else
#undef HAVE_UNISTD_H
#endif

/* Define if you have the 'unsigned long long' type. */
#undef HAVE_UNSIGNED_LONG_LONG

/* Define to 1 if the system has the type `unsigned long long int'. */
#undef HAVE_UNSIGNED_LONG_LONG_INT

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the <wchar.h> header file. */
#undef HAVE_WCHAR_H

/* Define to 1 if you have the `wcrtomb' function. */
#undef HAVE_WCRTOMB

/* Define to 1 if you have the `wcscoll' function. */
#undef HAVE_WCSCOLL

/* Define to 1 if you have the `wctype' function. */
#undef HAVE_WCTYPE

/* Define to 1 if you have the <wctype.h> header file. */
#undef HAVE_WCTYPE_H

/* systems should define this type here */
#undef HAVE_WCTYPE_T

/* systems should define this type here */
#undef HAVE_WINT_T

/* disable fatal errors on directories */
#undef NO_DIRECTORY_FATAL

/* disable lint checks */
#undef NO_LINT

/* Name of package */
#define PACKAGE "gawk"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-gawk@gnu.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GNU Awk"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GNU Awk 3.1.6"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gawk"

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.1.6"

/* Define to 1 if *printf supports %F format */
#undef PRINTF_HAS_F_FORMAT

/* Define to 1 if the C compiler supports function prototypes. */
#define PROTOTYPES 1

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of a `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of a `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* return type of sprintf */
#define SPRINTF_RET int

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* some systems define this type here */
#undef TIME_T_IN_SYS_TYPES_H

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#undef TIME_WITH_SYS_TIME

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#undef TM_IN_SYS_TIME

/* force use of our version of strftime */
#define USE_INCLUDED_STRFTIME 1

/* Version number of package */
#define VERSION "3.1.6"

/* Define to 1 if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#undef _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
# undef __CHAR_UNSIGNED__
#endif

/* Enable extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif

/* Define like PROTOTYPES; this can be used by system headers. */
#define __PROTOTYPES 1

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `int' if <sys/types.h> doesn't define. */
#undef gid_t

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#undef inline
#endif

/* Define to widest signed type if <inttypes.h> doesn't define. */
#define intmax_t long int

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define to equivalent of C99 restrict keyword, or to nothing if this is not
   supported. Do not define if restrict is supported directly. */
#define restrict
#if defined(__DECC) && (__DECC_VER >= 60400000)
#undef restrict
#endif

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* type to use in place of socklen_t if not defined */
#undef socklen_t

/* Define to `int' if <sys/types.h> does not define. */
#define ssize_t int

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define to unsigned long or unsigned long long if <stdint.h> and
   <inttypes.h> don't define. */
#define uintmax_t unsigned long

#if 0
#include "custom.h"	/* overrides for stuff autoconf can't deal with */
#else

/* Whether `time_t' is an unsigned type. */
#define TIME_T_UNSIGNED 1

/*******************************/
/* Gawk configuration options. */
/*******************************/

#define ALLOW_SWITCH 1

/*
 * DEFPATH
 *	VMS: "/AWK_LIBRARY" => "AWK_LIBRARY:"
 * The default search path for the -f option of gawk.  It is used
 * if the AWKPATH environment variable is undefined.
 *
 * Note: OK even if no AWK_LIBRARY logical name has been defined.
 */

#define DEFPATH	".,/AWK_LIBRARY"
#define ENVSEP	','

/*
 * Extended source file access.
 */
#define DEFAULT_FILETYPE ".awk"

/*
 * Pipe handling.
 */
#define PIPES_SIMULATED	1

/*
 * VAXCRTL is pre-ANSI and does some variations of numeric formatting
 * differently than gawk expects.
 */
#if defined(VAX) && !defined(__DECC)
/* '0' format modifier for %e,%f,%g gives wrong results in many cases */
#define VAXCRTL
/* %g format chooses %e format when should use %f */
#define GFMT_WORKAROUND	1
#endif

/*
 * VAX C
 *
 * As of V3.2, VAX C is not yet ANSI-compliant.  But it's close enough
 * for GAWK's purposes.  Comment this out for VAX C V2.4 and earlier.
 * YYDEBUG definition is needed for combination of VAX C V2.x and Bison.
 */
#if defined(VAXC) && !defined(__STDC__)
#define __STDC__	0
#define NO_TOKEN_PASTING
#define signed  /*empty*/
#define inline	/*empty*/
#ifndef __DECC	/* DEC C does not support #pragma builtins even in VAXC mode */
#define VAXC_BUILTINS
#endif
/* #define YYDEBUG 0 */
#define NO_MBSUPPORT  /* VAX C's preprocessor can't handle mbsupport.h */
#define RE_TOKEN_INIT_BUG     /* regcomp.c */
#endif

/*
 * DEC C
 *
 * Digital's ANSI complier.
 */
#ifdef __DECC
 /* DEC C implies DECC$SHR, which doesn't have the %g problem of VAXCRTL */
#undef GFMT_WORKAROUND
 /* DEC C V5.x introduces incompatibilities with prior porting efforts */
#define _DECC_V4_SOURCE
#define __SOCKET_TYPEDEFS
#if __VMS_VER >= 60200000
# undef __VMS_VER
# define __VMS_VER 60100000
#endif
#if __CRTL_VER >= 60200000
# if __CRTL_VER >= 70320000
#  define CRTL_VER_V732
#  define HAVE_SNPRINTF 1
# endif
# if __CRTL_VER >= 70301000
#  define CRTL_VER_V731
# endif
# undef __CRTL_VER
# define __CRTL_VER 60100000
#endif
#if __DECC_VER >= 60400000 && !defined(DEBUG)
/* disable "new feature in C99" diagnostics (for regex code);
   NEWC99 ought to suffice but doesn't (at least in V6.4) */
#pragma message disable (NEWC99,DESIGNATORUSE)
#endif
#endif	/* __DECC */

/*
 * GNU C
 *
 * Versions of GCC (actually GAS) earlier than 1.38 don't produce the
 * right code for ``extern const'' constructs, and other usages of
 * const might not be right either.  The old set of include files from
 * the gcc-vms distribution did not contain prototypes, and this could
 * provoke some const-related compiler warnings.  If you've got an old
 * version of gcc for VMS, define 'const' out of existance, and by all
 * means obtain the most recent version!
 *
 * Note: old versions of GCC should also avoid defining STDC_HEADERS,
 *       because most of the ANSI-C required header files are missing.
 */
#ifdef __GNUC__
/* #define const */
/* #undef STDC_HEADERS */
/* #undef HAVE_STDDEF_H */
#ifndef STDC_HEADERS
#define alloca __builtin_alloca
#define environ $$PsectAttributes_NOSHR$$environ	/* awful GAS kludge */
#endif
#undef  REGEX_MALLOC	/* use true alloca() in regex.c */
#endif

#define IN_CONFIG_H
#include "vms/redirect.h"
#undef  IN_CONFIG_H

#endif	/*"custom.h"*/

#endif	/*CONFIG_H*/
