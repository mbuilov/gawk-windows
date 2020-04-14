/* configh.in.  Generated from configure.ac by autoheader.  */
/* pc/config.h.  Generated automatically by pc/config.sed.  */

/* dynamic loading is possible */
#ifdef _WIN32
#define DYNAMIC 1
#endif

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#undef ENABLE_NLS

/* Define to the type of elements in the array set by `getgroups'. Usually
   this is either `int' or `gid_t'. */
#define GETGROUPS_T gid_t

/* Define to 1 if the `getpgrp' function requires zero arguments. */
#define GETPGRP_VOID 1

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#undef HAVE_ARPA_INET_H

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Define to 1 if you have the `btowc' function. */
#define HAVE_BTOWC 1

/* Define to 1 if you have the Mac OS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
#undef HAVE_CFLOCALECOPYCURRENT

/* Define to 1 if you have the Mac OS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
#undef HAVE_CFPREFERENCESCOPYAPPVALUE

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#undef HAVE_DCGETTEXT

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
#ifdef __MINGW32__
#define HAVE_DECL_TZNAME 1
#endif

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fmod' function. */
#define HAVE_FMOD 1

/* Define to 1 if you have the `fwrite_unlocked' function. */
#undef HAVE_FWRITE_UNLOCKED

/* Define to 1 if you have the `gai_strerror' function. */
#ifdef __MINGW32__
#define HAVE_GAI_STRERROR 1
#endif

/* have getaddrinfo */
#ifdef __MINGW32__
#define HAVE_GETADDRINFO 1
#endif

/* Define to 1 if you have the `getgrent' function. */
#undef HAVE_GETGRENT

/* Define to 1 if you have the `getgroups' function. */
#undef HAVE_GETGROUPS

/* Define if the GNU gettext() function is already present or preinstalled. */
#undef HAVE_GETTEXT

/* Define to 1 if you have the `grantpt' function. */
#undef HAVE_GRANTPT

/* Do we have history_list? */
#undef HAVE_HISTORY_LIST

/* Define if you have the iconv() function and it works. */
#undef HAVE_ICONV

/* Define to 1 if the system has the type `intmax_t'. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_INTMAX_T 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isascii' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_ISASCII 1
#endif

/* Define to 1 if you have the `isblank' function. */
#undef HAVE_ISBLANK

/* Define to 1 if you have the `iswctype' function. */
#ifdef __MINGW32__
#define HAVE_ISWCTYPE 1
#endif

/* Define to 1 if you have the `iswlower' function. */
#ifdef __MINGW32__
#define HAVE_ISWLOWER 1
#endif

/* Define to 1 if you have the `iswupper' function. */
#ifdef __MINGW32__
#define HAVE_ISWUPPER 1
#endif

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define HAVE_LANGINFO_CODESET 1
#endif

/* Define if your <locale.h> file defines LC_MESSAGES. */
#undef HAVE_LC_MESSAGES

/* Define to 1 if you have the <libintl.h> header file. */
#undef HAVE_LIBINTL_H

/* Define to 1 if you have a fully functional readline library. */
/* #undef HAVE_LIBREADLINE */

/* Define if you have the libsigsegv library. */
#undef HAVE_LIBSIGSEGV

/* Define to 1 if you have the <locale.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_LOCALE_H 1
#endif

/* Define to 1 if the system has the type 'long long int'. */
#undef HAVE_LONG_LONG_INT

/* Define to 1 if you have the `mbrlen' function. */
#ifdef __MINGW32__
#define HAVE_MBRLEN 1
#endif

/* Define to 1 if mbrtowc and mbstate_t are properly declared. */
#ifdef __MINGW32__
#define HAVE_MBRTOWC 1
#endif

/* Define to 1 if you have the <mcheck.h> header file. */
#undef HAVE_MCHECK_H

/* Define to 1 if you have the `memcmp' function. */
#define HAVE_MEMCMP 1

/* Define to 1 if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define to 1 if you have the `memcpy_ulong' function. */
#undef HAVE_MEMCPY_ULONG

/* Define to 1 if you have the `memmove' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifdef __DJGPP__
#define HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have the `memset_ulong' function. */
#undef HAVE_MEMSET_ULONG

/* Define to 1 if you have the `mkstemp' function. */
#ifdef __DJGPP__
#define HAVE_MKSTEMP 1
#endif

/* we have the mktime function */
#define HAVE_MKTIME 1

/* Define to 1 if you have fully functional mpfr and gmp libraries. */
/* #undef HAVE_MPFR */

/* Define to 1 if you have the <netdb.h> header file. */
#undef HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#undef HAVE_NETINET_IN_H

/* Define to 1 if you have the `posix_openpt' function. */
#undef HAVE_POSIX_OPENPT

/* Define to 1 if you have the `setenv' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_SETENV 1
#endif

/* Define to 1 if you have the `setlocale' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_SETLOCALE 1
#endif

/* Define to 1 if you have the `setsid' function. */
#undef HAVE_SETSID

/* Define to 1 if you have the `sigprocmask' function. */
#undef HAVE_SIGPROCMASK

/* Define to 1 if you have the `snprintf' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_SNPRINTF 1
#endif

/* newer systems define this type here */
#ifdef __MINGW32__
#define HAVE_SOCKADDR_STORAGE 1
#endif

/* we have sockets on this system */
#ifdef __MINGW32__
#define HAVE_SOCKETS 1
#endif

/* Define to 1 if stdbool.h conforms to C99. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stddef.h> header file. */
#ifdef __GNUC__
#define HAVE_STDDEF_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strcasecmp' function. */
#ifdef __DJGPP__
#define HAVE_STRCASECMP 1
#endif

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strcoll' function. */
#ifdef __DJGPP__
#define HAVE_STRCOLL 1
#endif

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strftime' function. */
#ifdef __MINGW32__
/* MinGW uses the replacement from missing_d, to support the %e specifier.  */
#define strftime rpl_strftime
#else
#define HAVE_STRFTIME 1
#endif

/* Define to 1 if cpp supports the ANSI # stringizing operator. */
#define HAVE_STRINGIZE 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1
#ifdef __EMX__
#define strncasecmp strnicmp
#endif

/* Define to 1 if you have the <stropts.h> header file. */
#undef HAVE_STROPTS_H

/* Define to 1 if you have the `strtod' function. */
#define HAVE_STRTOD 1

/* Define to 1 if you have the `strtoul' function. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_STRTOUL 1
#endif

/* Define to 1 if `gr_passwd' is a member of `struct group'. */
#undef HAVE_STRUCT_GROUP_GR_PASSWD

/* Define to 1 if `pw_passwd' is a member of `struct passwd'. */
#undef HAVE_STRUCT_PASSWD_PW_PASSWD

/* Define to 1 if `st_blksize' is a member of `struct stat'. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1
#endif

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#undef HAVE_STRUCT_TM_TM_ZONE

/* Define to 1 if you have the `system' function. */
#define HAVE_SYSTEM 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#ifdef __DJGPP__
#define HAVE_SYS_IOCTL_H 1
#endif

/* Define to 1 if you have the <sys/param.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/select.h> header file. */
#undef HAVE_SYS_SELECT_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#undef HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#ifdef __DJGPP__
#define HAVE_SYS_WAIT_H 1
#endif

/* Define to 1 if you have the <termios.h> header file. */
#undef HAVE_TERMIOS_H

/* Define to 1 if you have the `timegm' function. */
#undef HAVE_TIMEGM

/* Define to 1 if you have the `tmpfile' function. */
#ifdef __DJGPP__
#define HAVE_TMPFILE 1
#endif

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#undef HAVE_TM_ZONE

/* Define to 1 if you have the `towlower' function. */
#ifdef __MINGW32__
#define HAVE_TOWLOWER 1
#endif

/* Define to 1 if you have the `towupper' function. */
#ifdef __MINGW32__
#define HAVE_TOWUPPER 1
#endif

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
#define HAVE_TZNAME 1

/* Define to 1 if you have the `tzset' function. */
#define HAVE_TZSET 1

/* Define to 1 if the system has the type `uintmax_t'. */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define HAVE_UINTMAX_T 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if the system has the type `unsigned long long int'. */
#undef HAVE_UNSIGNED_LONG_LONG_INT

/* Define to 1 if you have the `usleep' function. */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define HAVE_USLEEP 1
#endif

/* Define to 1 if you have the `waitpid' function. */
#undef HAVE_WAITPID

/* Define to 1 if you have the <wchar.h> header file. */
#ifdef __MINGW32__
#define HAVE_WCHAR_H 1
#endif

/* Define to 1 if you have the `wcrtomb' function. */
#ifdef __MINGW32__
#define HAVE_WCRTOMB 1
#endif

/* Define to 1 if you have the `wcscoll' function. */
#ifdef __MINGW32__
#define HAVE_WCSCOLL 1
#endif

/* Define to 1 if you have the `wctype' function. */
#ifdef __MINGW32__
#define HAVE_WCTYPE 1
#endif

/* Define to 1 if you have the <wctype.h> header file. */
#ifdef __MINGW32__
#define HAVE_WCTYPE_H 1
#endif

/* systems should define this type here */
#ifdef __MINGW32__
#define HAVE_WCTYPE_T 1
#endif

/* systems should define this type here */
#ifdef __MINGW32__
#define HAVE_WINT_T 1
#endif

/* Define to 1 if the system has the type `_Bool'. */
#ifdef __DJGPP__
#define HAVE__BOOL 1
#endif

/* Define to 1 if you have the `__etoa_l' function. */
#undef HAVE___ETOA_L

/* disable lint checks */
#undef NO_LINT

/* Name of package */
#define PACKAGE "gawk"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "bug-gawk@gnu.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GNU Awk"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GNU Awk 5.1.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gawk"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://www.gnu.org/software/gawk/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "5.1.0"

/* Define to 1 if *printf supports %a format */
#define PRINTF_HAS_A_FORMAT 1

/* Define to 1 if *printf supports %F format */
#ifdef __DJGPP__
#define PRINTF_HAS_F_FORMAT 1
#endif

/* The size of `unsigned int', as computed by sizeof. */
#define SIZEOF_UNSIGNED_INT 4

/* The size of `unsigned long', as computed by sizeof. */
#define SIZEOF_UNSIGNED_LONG 4

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* enable built-in intdiv0 function */
#undef SUPPLY_INTDIV

/* some systems define this type here */
#undef TIME_T_IN_SYS_TYPES_H

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#undef TM_IN_SYS_TIME

/* Define to 1 if the character set is EBCDIC */
#undef USE_EBCDIC

/* This is required to compile Gnulib regex code.  */
#if defined(__DJGPP__) || defined(__MINGW32__)
#define _GNU_SOURCE 1
#endif
/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# undef _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# undef _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# undef _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif


/* Version number of package */
#define VERSION "5.1.0"

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#undef _FILE_OFFSET_BITS

/* Define for large files, on AIX-style hosts. */
#undef _LARGE_FILES

/* Define to 1 if on MINIX. */
#undef _MINIX

/* The _Noreturn keyword of C11.  */
#ifndef _Noreturn
# if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__) \
      || 0x5110 <= __SUNPRO_C)
#  define _Noreturn __attribute__ ((__noreturn__))
# elif defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif


/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#undef _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#undef _POSIX_SOURCE

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
# undef __CHAR_UNSIGNED__
#endif

/* Define to empty if `const' does not conform to ANSI C. */
#undef const

/* Define to `int' if <sys/types.h> doesn't define. */
#undef gid_t

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#ifdef __GNUC__
#define inline __inline__
#endif
#endif

/* Define to the widest signed integer type if <stdint.h> and <inttypes.h> do
   not define. */
#undef intmax_t

/* Define to `int' if <sys/types.h> does not define. */
#undef pid_t

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#undef restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
#undef size_t

/* type to use in place of socklen_t if not defined */
#undef socklen_t

/* Define to `int' if <sys/types.h> does not define. */
#undef ssize_t

/* Define to `int' if <sys/types.h> doesn't define. */
#undef uid_t

/* Define to the widest unsigned integer type if <stdint.h> and <inttypes.h>
   do not define. */
#undef uintmax_t

#include "custom.h"
#ifdef __DJGPP__
/* gcc no longer includes this by default */
# include <sys/version.h>

/* Library search path */
# if (__DJGPP__ > 2 || __DJGPP_MINOR__ >= 3)
#  define DEFPATH  ".;/dev/env/DJDIR/share/awk"
# else
#  define DEFPATH  ".;c:/lib/awk;c:/gnu/lib/awk"
# endif

/* Function prototype.  */
#include <stdbool.h>
extern bool is_valid_identifier(const char *name);
#endif

#ifndef __DJGPP__
#define HAVE_POPEN_H 1
#endif

#if defined(__EMX__)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif
