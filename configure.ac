dnl
dnl configure.ac --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995-2020 the Free Software Foundation, Inc.
dnl
dnl This file is part of GAWK, the GNU implementation of the
dnl AWK Programming Language.
dnl
dnl GAWK is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl GAWK is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
dnl

dnl Process this file with autoconf to produce a configure script.

AC_INIT([GNU Awk],[5.1.0],[bug-gawk@gnu.org],[gawk])

# This is a hack. Different versions of install on different systems
# are just too different. Chuck it and use install-sh.
#
# If the user supplies $INSTALL, figure they know what they're doing.
#
# With Autoconf 2.5x, this needs to come very early on, but *after*
# the INIT macro. Sigh.

if test "$INSTALL" = ""
then
	INSTALL="$srcdir/install-sh -c"
	export INSTALL
fi

AC_PREREQ([2.69])
AM_INIT_AUTOMAKE([1.15 dist-xz dist-lzip])

AC_CONFIG_MACRO_DIR([m4])

AC_ARG_ENABLE([lint],
	[AS_HELP_STRING([--disable-lint],[do not compile in gawk lint checking])],
	if test "$enableval" = no
	then
		AC_DEFINE(NO_LINT, 1, [disable lint checks])
	fi
)
AC_ARG_ENABLE([builtin-intdiv0],
	[AS_HELP_STRING([--enable-builtin-intdiv0],[enable built-in intdiv0 function])],
	if test "$enableval" = yes
	then
		AC_DEFINE(SUPPLY_INTDIV, 1, [enable built-in intdiv0 function])
		sed '/^@set PATCHLEVEL/a\
@set INTDIV' < "$srcdir"/doc/gawktexi.in > foo
		cp foo "$srcdir"/doc/gawktexi.in
		rm foo
	fi
)

SKIP_MPFR=no
AC_ARG_ENABLE([mpfr],
	[AS_HELP_STRING([--disable-mpfr],[do not check for MPFR])],
	if test "$enableval" = no
	then
		SKIP_MPFR=yes
	fi
)

EXTENSIONDIR=
AC_ARG_ENABLE([versioned-extension-dir],
	[AS_HELP_STRING([--enable-versioned-extension-dir], [use a versioned directory for extensions])],
	if test "$enableval" = yes
	then
		MAJOR=`awk '/define gawk_api_major_version/ { print $3 }' < $srcdir/gawkapi.h`
		MINOR=`awk '/define gawk_api_minor_version/ { print $3 }' < $srcdir/gawkapi.h`

		# note leading slash on the value
		export EXTENSIONDIR=/ext-$MAJOR.$MINOR
	fi
)

# set default shared library location
AC_SUBST([pkgextensiondir], ['${pkglibdir}'${EXTENSIONDIR}])

GAWK_CANONICAL_HOST
AC_USE_SYSTEM_EXTENSIONS

dnl checks for programs
AC_PROG_EGREP
AC_PROG_YACC
AC_PROG_LN_S
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_RANLIB

AC_OBJEXT
AC_EXEEXT

AC_PROG_INSTALL
AC_PROG_MAKE_SET

# This is mainly for my use during testing and development.
# Yes, it's a bit of a hack.
AC_MSG_CHECKING([for special development options])
if test -f $srcdir/.developing
then
	# add other debug flags as appropriate, save GAWKDEBUG for emergencies
	CFLAGS="$CFLAGS -DARRAYDEBUG -DYYDEBUG -DLOCALEDEBUG -DMEMDEBUG"

	# turn on compiler warnings if we're doing development
	# enable debugging using macros also
	if test "$GCC" = yes
	then
		CFLAGS="$CFLAGS -Wall -fno-builtin -g3 -ggdb3"
	fi
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	CFLAGS="$CFLAGS -DNDEBUG"	# turn off assertions
fi

AC_SUBST(CFLAGS)

# shared library suffix for dynamic loading:
AC_SUBST(acl_shlibext)

dnl checks for systems
AC_ZOS_USS
AC_SEARCH_LIBS([strerror],[cposix])
AC_SYS_LARGEFILE

AC_MSG_CHECKING([if we are using EBCDIC])
AC_EGREP_CPP([gnu_gawk_in_ebcdic],
[#if 'a' == 0x81
gnu_gawk_in_ebcdic
#endif],
[AC_DEFINE(USE_EBCDIC, 1, Define to 1 if the character set is EBCDIC)
 use_ebcdic=yes],
[use_ebcdic=no])
AC_MSG_RESULT([$use_ebcdic])
if test "$use_ebcdic" = yes
then
	ZOS_FAIL='; echo Expect $@ to fail on z/OS.'
fi
AC_SUBST(ZOS_FAIL)

dnl Set the programming language for checks. Fortunately,
dnl this only needs to be set once, since everything is in C.
AC_LANG([C])

dnl initialize GNU gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
AM_LANGINFO_CODESET
gt_LC_MESSAGES

dnl checks for header files
AC_CHECK_HEADERS(arpa/inet.h fcntl.h locale.h libintl.h mcheck.h \
	netdb.h netinet/in.h stddef.h string.h \
	sys/ioctl.h sys/param.h sys/select.h sys/socket.h sys/time.h unistd.h \
	termios.h stropts.h wchar.h wctype.h)
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_SYS_WAIT
AC_HEADER_TIME

if test "$ac_cv_header_string_h" = yes
then
	AC_CHECK_HEADERS(memory.h)
else
	AC_CHECK_HEADERS(strings.h)
fi

dnl Check cross compiling
AM_CONDITIONAL([TEST_CROSS_COMPILE], [test "$build_alias" != "$host_alias"])

dnl checks for typedefs
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_GETGROUPS
AC_TYPE_LONG_LONG_INT
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_INTMAX_T
AC_TYPE_UINTMAX_T
AC_CHECK_TYPE(ssize_t, int)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
dnl see if time_t is defined in <sys/types.h>
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>]], [[
	time_t foo;
	foo = 0;
]])],[AC_DEFINE(TIME_T_IN_SYS_TYPES_H, 1,
		  some systems define this type here)],[])
dnl check for wctype_t in <wctype.h>
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <wctype.h>]], [[
	wctype_t foo;
	foo = 0;
]])],[AC_DEFINE(HAVE_WCTYPE_T, 1, systems should define this type here)],[])
dnl check for wint_t in <wctype.h>
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <wctype.h>]], [[
	wint_t foo;
	foo = 0;
]])],[AC_DEFINE(HAVE_WINT_T, 1, systems should define this type here)],[])
dnl check for sockaddr_storage
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/socket.h>]], [[
	struct sockaddr_storage foo;
]])],[AC_DEFINE(HAVE_SOCKADDR_STORAGE, 1, newer systems define this type here)],[])

dnl Borrwed from rsync, thanks to to Jim Meyering.

dnl Check for socklen_t: historically on BSD it is an int, and in
dnl POSIX 1g it is a type of its own, but some platforms use different
dnl types for the argument to getsockopt, getpeername, etc.  So we
dnl have to test to find something that will work.

dnl This is no good, because passing the wrong pointer on C compilers is
dnl likely to only generate a warning, not an error.

AC_DEFUN([TYPE_SOCKLEN_T],
[
   AC_CHECK_TYPE([socklen_t], ,[
      AC_MSG_CHECKING([for socklen_t equivalent])
      AC_CACHE_VAL([rsync_cv_socklen_t_equiv],
      [
         # Systems have either "struct sockaddr *" or
         # "void *" as the second argument to getpeername
         rsync_cv_socklen_t_equiv=
         for arg2 in "struct sockaddr" void
	 do
            for t in int size_t unsigned long "unsigned long"
	    do
               AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/socket.h>

                  int getpeername (int, $arg2 *, $t *);
               ]], [[
                  $t len;
                  getpeername(0,0,&len);
               ]])],[
                  rsync_cv_socklen_t_equiv="$t"
                  break
               ],[])
            done
         done

         if test "$rsync_cv_socklen_t_equiv" = ""
	 then
dnl		Some systems get this. Default to int.  -- ADR
dnl            AC_MSG_ERROR([Cannot find a type to use in place of socklen_t])
		rsync_cv_socklen_t_equiv=int
         fi
      ])
      AC_MSG_RESULT($rsync_cv_socklen_t_equiv)
      AC_DEFINE_UNQUOTED(socklen_t, $rsync_cv_socklen_t_equiv,
			[type to use in place of socklen_t if not defined])],
      [#include <sys/types.h>
#include <sys/socket.h>])
])
TYPE_SOCKLEN_T

dnl Check for C11 _Noreturn
GAWK_AC_NORETURN

dnl checks for functions
AC_FUNC_MKTIME
case "$ac_cv_func_working_mktime" in
yes)	AC_DEFINE(HAVE_MKTIME, 1, [we have the mktime function])
	;;
esac

AC_CHECK_FUNC(getaddrinfo, [AC_DEFINE(HAVE_GETADDRINFO, 1, [have getaddrinfo])],
	      [AC_CHECK_LIB(socket, getaddrinfo,
			    [AC_DEFINE(HAVE_GETADDRINFO, 1,
				       [have getaddrinfo])])])

AC_SEARCH_LIBS(fmod, m)
AC_SEARCH_LIBS(isinf, m)
AC_SEARCH_LIBS(ismod, m)
dnl Don't look for libsigsegv on OSF/1, gives us severe headaches
case $host_os in
osf1)	: ;;
*)
	gl_LIBSIGSEGV
	;;
esac

# Need the check for mkstemp and tmpfile for missing_d/snprintf.c.
AC_CHECK_FUNCS(__etoa_l atexit btowc fmod gai_strerror \
	getgrent getgroups grantpt \
	fwrite_unlocked \
	isascii isblank iswctype iswlower iswupper mbrlen \
	memcmp memcpy memcpy_ulong memmove memset \
	memset_ulong mkstemp posix_openpt setenv setlocale setsid sigprocmask \
	snprintf strchr \
	strerror strftime strcasecmp strncasecmp strcoll strtod strtoul \
	system timegm tmpfile towlower towupper tzset usleep waitpid wcrtomb \
	wcscoll wctype)
dnl this check is for both mbrtowc and the mbstate_t type, which is good
AC_FUNC_MBRTOWC

dnl check for dynamic linking
dnl This is known to be very primitive
AC_ARG_ENABLE([extensions],
	[AS_HELP_STRING([--disable-extensions], [disable dynamic extensions (default is detect)])])
if test "$enable_extensions" != "no"
then
	extensions_supported=no

	dnl On MirBSD (and probably other systems), don't even try.
	case $host_os in
	mirbsd* | openedition*)	# OS/390 z/OS POSIX layer
		;;
	*)
	AC_CHECK_HEADER(dlfcn.h,
		[
		# Check this separately. Some systems have dlopen
		# in libc. Notably freebsd and cygwin.
		# HP-NSK has it in zrldsrl
		AC_SEARCH_LIBS(dlopen, dl zrldsrl, gawk_have_dlopen=yes, gawk_have_dlopen=no)
		# Only do DYNAMIC if we have the lib. z/OS (some versions) have
		# the header but not the lib, apparently
		if test "$gawk_have_dlopen" = yes
		then
			extensions_supported=yes
			AC_DEFINE([DYNAMIC], 1, [dynamic loading is possible])
		fi
	])
		;;
	esac

	if test "$enable_extensions$extensions_supported" = "yesno"
	then
		AC_MSG_ERROR([extension support requested, but unavailable])
	fi
	enable_extensions=$extensions_supported
fi
AM_CONDITIONAL([ENABLE_EXTENSIONS], [test "$enable_extensions" = "yes"])

dnl check for how to use getpgrp
dnl have to hardwire it for VMS POSIX. Sigh.
dnl ditto for BeOS, OS/2, and MS-DOS.
case $host_os in
vms*|beos*|os2*|msdos)
	AC_DEFINE(GETPGRP_VOID, 1,
	  [Define to 1 if the getpgrp function requires zero arguments.])
	;;
*)	AC_FUNC_GETPGRP
	;;
esac

dnl check for printf %F format
AC_MSG_CHECKING([for printf %F format])
AC_RUN_IFELSE([
AC_LANG_SOURCE([
#include <stdio.h>
#include <string.h>

int main()
{
	char buf[[100]];

	sprintf(buf, "%F", 123.45);

	if (strcmp(buf, "123.450000") == 0)
		return 0;
	else
		return 1;
}
])],
	has_f_format=yes,
	has_f_format=no,
	has_f_format=no  dnl Cross-compiling, assuming the worst.
)
if test "$has_f_format" = yes
then
	AC_DEFINE(PRINTF_HAS_F_FORMAT, 1, [Define to 1 if *printf supports %F format])
fi
AC_MSG_RESULT($has_f_format)

dnl check for printf %a format
AC_MSG_CHECKING([for printf %a format])
AC_RUN_IFELSE([
AC_LANG_SOURCE([
#include <stdio.h>
#include <string.h>

int main()
{
	char buf[[100]];

	sprintf(buf, "%a", 8.0);

	if (strncmp(buf, "0x", 2) == 0)
		return 0;
	else
		return 1;
}
])],
	has_a_format=yes,
	has_a_format=no,
	has_a_format=no  dnl Cross-compiling, assuming the worst.
)
if test "$has_a_format" = yes
then
	AC_DEFINE(PRINTF_HAS_A_FORMAT, 1, [Define to 1 if *printf supports %a format])
fi
AC_MSG_RESULT($has_a_format)

dnl check for sockets
GAWK_AC_LIB_SOCKETS

dnl check for readline support
GAWK_CHECK_READLINE

dnl check for mpfr support
case `uname -m` in
*'Power Macintosh'*)
	: ;;
*)
	case $SKIP_MPFR in
	no)	GNUPG_CHECK_MPFR
		;;
	esac
	;;
esac

dnl checks for structure members
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_CHECK_MEMBERS([struct passwd.pw_passwd],,,[
#include <sys/types.h>
#include <pwd.h>
])
AC_CHECK_MEMBERS([struct group.gr_passwd],,,[
#include <sys/types.h>
#include <grp.h>
])
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

dnl checks for compiler characteristics
AC_C_CHAR_UNSIGNED
AC_C_CONST
AC_C_RESTRICT
AC_C_INLINE
AC_C_STRINGIZE

AC_CONFIG_HEADERS([config.h:configh.in])
AH_BOTTOM([#include "custom.h"])

dnl Crude but small hack to make plug-ins work on Mac OS X
dnl We should really use the libtool value for shrext_cmds, but that
dnl is not available here, since we do not use libtool at the top level.
case $acl_shlibext in
dylib)	GAWKLIBEXT=so ;;	# MacOS uses .dylib for shared libraries, but libtool uses .so for modules
*) GAWKLIBEXT=$acl_shlibext ;;
esac
AC_SUBST(GAWKLIBEXT)

AC_CONFIG_FILES(Makefile
	support/Makefile
	awklib/Makefile
	doc/Makefile
	extras/Makefile
	po/Makefile.in
	test/Makefile)
if test "$enable_extensions" = "yes"
then
	AC_CONFIG_SUBDIRS(extension)
fi
AC_OUTPUT
if test "$GCC" = yes &&
   test -f $srcdir/.developing &&
   grep -i debug $srcdir/.developing > /dev/null
then
	for i in . support extension
	do
		sed '/-O2/s///' $i/Makefile > foo
		mv foo $i/Makefile
	done
fi
