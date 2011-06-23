dnl
dnl configure.ac --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995-2011 the Free Software Foundation, Inc.
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

AC_INIT([GNU Awk], 4.0.0, bug-gawk@gnu.org, gawk)

# This is a hack. Different versions of install on different systems
# are just too different. Chuck it and use install-sh.
#
# If the user supplies $INSTALL, figure they know what they're doing.
#
# With Autoconf 2.5x, this needs to come very early on, but *after*
# the INIT macro. Sigh.

if test "x$INSTALL" = "x"
then
	INSTALL="$srcdir/install-sh -c"
	export INSTALL
fi

AC_PREREQ(2.68)
AM_INIT_AUTOMAKE([1.11 dist-bzip2 dist-xz])

AC_CONFIG_MACRO_DIR([m4])

dnl Additional argument stuff
AC_ARG_WITH(whiny-user-strftime, [  --with-whiny-user-strftime	Force use of included version of strftime for deficient systems],
	if test "$withval" = yes
	then
		AC_DEFINE(USE_INCLUDED_STRFTIME, 1,
			  [force use of our version of strftime])
	fi
)
AC_ARG_ENABLE([lint], [  --disable-lint	Disable gawk lint checking],
	if test "$enableval" = no
	then
		AC_DEFINE(NO_LINT, 1, [disable lint checks])
	fi
)

AC_USE_SYSTEM_EXTENSIONS

dnl checks for programs
AC_PROG_EGREP
AC_PROG_YACC
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CPP

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
	CFLAGS="$CFLAGS -DARRAYDEBUG -DYYDEBUG"
	if grep dbug $srcdir/.developing
	then
		CFLAGS="$CFLAGS -DDBUG"
		LIBS="$LIBS dbug/libdbug.a"
	fi
	# turn on compiler warnings if we're doing development
	# enable debugging using macros also
	if test "$GCC" = yes
	then
		CFLAGS="$CFLAGS -Wall -fno-builtin -g3 -gdwarf-2"
	fi
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi

AC_SUBST(CFLAGS)

dnl checks for systems
AC_ZOS_USS
AC_ISC_POSIX
AC_SYS_LARGEFILE
GAWK_AC_AIX_TWEAK
GAWK_AC_LINUX_ALPHA

if test "$ISC" = 1	# will be set by test for ISC
then
dnl	need -D_SYSV3 for ISC
	CFLAGS="$CFLAGS -D_SYSV3"
fi

dnl Set the programming language for checks. Fortunately,
dnl this only needs to be set once, since everything is in C.
AC_LANG([C])

dnl check for C compiler for automake. Needed for dfa.h.
AM_C_PROTOTYPES

dnl Cygwin doesn't like to get libs with full paths
dnl since that overrides linking against DLLs.
case `(uname) 2> /dev/null` in
*CYGWIN*)
	with_libiconv_prefix=no
	with_libintl_prefix=no
	;;
*)
	;;
esac

dnl initialize GNU gettext
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_LANGINFO_CODESET
gt_LC_MESSAGES

dnl checks for header files
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(arpa/inet.h fcntl.h limits.h locale.h libintl.h mcheck.h \
	netdb.h netinet/in.h stdarg.h stddef.h string.h \
	sys/param.h sys/socket.h sys/time.h unistd.h \
	termios.h stropts.h wchar.h wctype.h)

if test "$ac_cv_header_string_h" = yes
then
	AC_CHECK_HEADERS(memory.h)
else
	AC_CHECK_HEADERS(strings.h)
fi

dnl checks for typedefs
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_SIZE_T
AC_TYPE_GETGROUPS
gl_AC_TYPE_LONG_LONG
gl_AC_TYPE_UNSIGNED_LONG_LONG
gl_AC_TYPE_INTMAX_T
gl_AC_TYPE_UINTMAX_T
AC_CHECK_TYPE(ssize_t, int)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long)
dnl see if time_t is defined in <sys/types.h>
AC_TRY_COMPILE([#include <sys/types.h>],[
	time_t foo;
	foo = 0;
],
	AC_DEFINE(TIME_T_IN_SYS_TYPES_H, 1,
		  [some systems define this type here]))
dnl check for wctype_t in <wctype.h>
AC_TRY_COMPILE([#include <wctype.h>],[
	wctype_t foo;
	foo = 0;
],
	AC_DEFINE(HAVE_WCTYPE_T, 1, [systems should define this type here]))
dnl check for wint_t in <wctype.h>
AC_TRY_COMPILE([#include <wctype.h>],[
	wint_t foo;
	foo = 0;
],
	AC_DEFINE(HAVE_WINT_T, 1, [systems should define this type here]))
dnl check for sockaddr_storage
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],[
	struct sockaddr_storage foo;
],
	AC_DEFINE(HAVE_SOCKADDR_STORAGE, 1, [newer systems define this type here]))

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
         for arg2 in "struct sockaddr" void; do
            for t in int size_t unsigned long "unsigned long"; do
               AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>

                  int getpeername (int, $arg2 *, $t *);
               ],[
                  $t len;
                  getpeername(0,0,&len);
               ],[
                  rsync_cv_socklen_t_equiv="$t"
                  break
               ])
            done
         done

         if test "x$rsync_cv_socklen_t_equiv" = x; then
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

dnl checks for functions
AC_FUNC_VPRINTF
AC_FUNC_MKTIME
case "$ac_cv_func_working_mktime" in
yes)	AC_DEFINE(HAVE_MKTIME, 1, [we have the mktime function])
	;;
esac

AC_CHECK_FUNC(getaddrinfo, [AC_DEFINE(HAVE_GETADDRINFO, 1, [have getaddrinfo])],
	      [AC_CHECK_LIB(socket, getaddrinfo,
			    [AC_DEFINE(HAVE_GETADDRINFO, 1,
				       [have getaddrinfo])])])

AC_CHECK_LIB(m, fmod)
AC_CHECK_LIB(m, isinf)
AC_CHECK_LIB(m, ismod)
dnl Don't look for libsigsegv on OSF/1, gives us severe headaches
case `uname` in
OSF1)	: ;;
*)
	gl_LIBSIGSEGV
	;;
esac

# Need the check for mkstemp and tmpfile for missing_d/snprintf.c.
AC_CHECK_FUNCS(atexit btowc fmod getgrent getgroups grantpt \
	isascii iswctype iswlower iswupper mbrlen \
	memcmp memcpy memcpy_ulong memmove memset \
	memset_ulong mkstemp setenv setlocale snprintf strchr \
	strerror strftime strncasecmp strcoll strtod strtoul \
	system tmpfile towlower towupper tzset usleep wcrtomb \
	wcscoll wcscoll wctype)
dnl this check is for both mbrtowc and the mbstate_t type, which is good
AC_FUNC_MBRTOWC

dnl check for dynamic linking
dnl This is known to be very primitive
AC_CHECK_HEADER(dlfcn.h,
	[AC_DEFINE([DYNAMIC], 1, [dynamic loading is possible])
	if test "$GCC" = yes
	then
		# Add others here as appropriate,
		# one day use GNU libtool.
		# 3/2010: Used to have cygwin here but removed since
		# we get complaints that -export-dynamic doesn't work.
		if uname | $EGREP -i 'linux|freebsd' > /dev/null
		then
			LDFLAGS="$LDFLAGS -export-dynamic"
		fi
	fi

	# Check this separately. Some systems have dlopen
	# in libc. Notably freebsd and cygwin.
	# HP-NSK has it in zrldsrl
	AC_SEARCH_LIBS(dlopen, dl zrldsrl)
])

dnl check for how to use getpgrp
dnl have to hardwire it for VMS POSIX. Sigh.
dnl ditto for BeOS, OS/2, and MS-DOS.
case `(uname) 2> /dev/null` in
*VMS*|*BeOS*|*OS/2*|*MS-DOS*)
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
if test "$has_f_format" = yes; then
	AC_DEFINE(PRINTF_HAS_F_FORMAT, 1, [Define to 1 if *printf supports %F format])
fi
AC_MSG_RESULT($has_f_format)

dnl check for sockets
GAWK_AC_LIB_SOCKETS

dnl check for readline support
GNUPG_CHECK_READLINE

dnl checks for structure members
AC_STRUCT_ST_BLKSIZE
AC_HEADER_TIME
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

AC_CONFIG_FILES(Makefile
	awklib/Makefile
	doc/Makefile
	po/Makefile.in
	test/Makefile)
AC_OUTPUT
