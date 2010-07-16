#ifndef CONFIG_H
#define CONFIG_H
/*
 * config.h -- configuration definitions for gawk.
 *
 * For VMS (assumes V4.6 or later; tested on V5.5-2)
 */

/* 
 * Copyright (C) 1991-1992, 1995-1996, 1999, 2001, 2002 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#if 0
/* Define if using alloca.c.  */
#define C_ALLOCA
#else
#define NO_ALLOCA	/* vms/vms_fwrite.c needs this */
#endif

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
#define GETGROUPS_T int

/* Define if the `getpgrp' function takes no argument.  */
#define GETPGRP_VOID	1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#undef HAVE_ALLOCA_H

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have a working `mmap' system call.  */
#undef HAVE_MMAP

/* Define if your struct stat has st_blksize.  */
#undef HAVE_ST_BLKSIZE

/* Define if you have the ANSI # stringizing operator in cpp. */
#ifdef VAXC
#undef HAVE_STRINGIZE
#else
#define HAVE_STRINGIZE 1
#endif

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define if your struct tm has tm_zone.  */
#undef HAVE_TM_ZONE

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1		/* (faked in vms/vms_misc.c) */

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

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
#define STACK_DIRECTION (-1)

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#undef TIME_WITH_SYS_TIME

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

#define REGEX_MALLOC 1 /* use malloc instead of alloca in regex.c */
#define SPRINTF_RET int /* return type of sprintf */
#define HAVE_MKTIME 1	/* have the mktime function */
#undef HAVE_SOCKETS	/* don't have sockets on this system by default */
#undef HAVE_PORTALS	/* don't have portals on /p on this system */
#undef DYNAMIC		/* don't allow dynamic addition of builtins */
#define STRTOD_NOT_C89 1 /* strtod doesn't have C89 semantics */
/* #undef ssize_t */  /* signed version of size_t */

/* Define if you have the __argz_count function.  */
#undef HAVE___ARGZ_COUNT

/* Define if you have the __argz_next function.  */
#undef HAVE___ARGZ_NEXT

/* Define if you have the __argz_stringify function.  */
#undef HAVE___ARGZ_STRINGIFY

/* Define if you have the alarm function.  */
#define HAVE_ALARM 1

/* Define if you have the dcgettext function.  */
#undef HAVE_DCGETTEXT

/* Define if you have the fmod function.  */
#define HAVE_FMOD 1

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getpagesize function.  */
#undef HAVE_GETPAGESIZE

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the munmap function.  */
#undef HAVE_MUNMAP

/* Define if you have the putenv function.  */
#undef HAVE_PUTENV

/* Define if you have the setenv function.  */
#undef HAVE_SETENV

/* Define if you have the setlocale function.  */
#undef HAVE_SETLOCALE

/* Define if you have the stpcpy function.  */
#undef HAVE_STPCPY

/* Define if you have the strcasecmp function.  */
#undef HAVE_STRCASECMP

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strdup function.  */
#undef HAVE_STRDUP

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#undef HAVE_STRFTIME		/* use the missing_d/strfime.c version */

/* Define if you have the strncasecmp function.  */
#undef HAVE_STRNCASECMP

/* Define if you have the strtod function.  */
#define HAVE_STRTOD 1

/* Define if you have the system function.  */
#define HAVE_SYSTEM 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1		/* (faked in vms/vms_misc.c) */

/* Define if you have the <argz.h> header file.  */
#undef HAVE_ARGZ_H

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <libintl.h> header file.  */
#undef HAVE_LIBINTL_H

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#undef HAVE_LOCALE_H

/* Define if you have the <malloc.h> header file.  */
#undef HAVE_MALLOC_H

/* Define if you have the <mcheck.h> header file.  */
#undef HAVE_MCHECK_H

/* Define if you have the <memory.h> header file.  */
#undef HAVE_MEMORY_H

/* Define if you have the <netdb.h> header file.  */
#undef HAVE_NETDB_H

/* Define if you have the <netinet/in.h> header file.  */
#undef HAVE_NETINET_IN_H

/* Define if you have the <nl_types.h> header file.  */
#undef HAVE_NL_TYPES_H

/* Define if you have the <signum.h> header file.  */
#undef HAVE_SIGNUM_H

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <strings.h> header file.  */
#undef HAVE_STRINGS_H

/* Define if you have the <sys/param.h> header file.  */
#undef HAVE_SYS_PARAM_H

/* Define if you have the <sys/socket.h> header file.  */
#undef HAVE_SYS_SOCKET_H

/* Define if you have the <sys/time.h> header file.  */
#undef HAVE_SYS_TIME_H

/* Define if you have the <unistd.h> header file.  */
#ifdef __DECC
#define HAVE_UNISTD_H 1
#else
#undef HAVE_UNISTD_H
#endif

/* Define if you have the i library (-li).  */
#undef HAVE_LIBI

/* Define if you have the intl library (-lintl).  */
#undef HAVE_LIBINTL

/* Define if you have the m library (-lm).  */
#undef HAVE_LIBM

/* Name of package */
#define PACKAGE "gawk"

/* Version number of package */
#define VERSION "3.1.1"

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define to make ftello visible on some hosts (e.g. HP-UX 10.20). */
#undef _LARGEFILE_SOURCE

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to make ftello visible on some hosts (e.g. glibc 2.1.3). */
#undef _XOPEN_SOURCE

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define to 1 if you have the stpcpy function. */
#undef HAVE_STPCPY

/* Define if your locale.h file contains LC_MESSAGES. */
#undef HAVE_LC_MESSAGES

/* Define to 1 if NLS is requested. */
#undef ENABLE_NLS

/* Define to 1 if you have gettext and don't want to use GNU gettext. */
#undef HAVE_GETTEXT

/* Define as 1 if you have catgets and don't want to use GNU gettext. */
#undef HAVE_CATGETS

#if 0
#include <custom.h>	/* overrides for stuff autoconf can't deal with */
#else

/*******************************/
/* Gawk configuration options. */
/*******************************/

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
 * %g format in VAXCRTL is broken (chooses %e format when should use %f).
 */
#define GFMT_WORKAROUND	1

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
#ifndef __DECC	/* DEC C does not support #pragma builtins even in VAXC mode */
#define VAXC_BUILTINS
#endif
/* #define YYDEBUG 0 */
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
# undef __CRTL_VER
# define __CRTL_VER 60100000
#endif
#endif

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
#ifndef STDC_HEADERS
#define alloca __builtin_alloca
#define environ $$PsectAttributes_NOSHR$$environ	/* awful GAS kludge */
#endif
#undef  REGEX_MALLOC	/* use true alloca() in regex.c */
#endif

#define IN_CONFIG_H
#include "vms/redirect.h"
#undef  IN_CONFIG_H

#endif	/*<custom.h>*/

#endif	/*CONFIG_H*/
