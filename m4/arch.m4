dnl
dnl arch.m4 --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995, 1996, 1998, 1999, 2000, 2003, 2004 the Free Software Foundation, Inc.
dnl
dnl This file is part of GAWK, the GNU implementation of the
dnl AWK Progamming Language.
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

dnl Check for AIX and add _XOPEN_SOURCE_EXTENDED
AC_DEFUN([GAWK_AC_AIX_TWEAK], [
AC_MSG_CHECKING([for AIX compilation hacks])
AC_CACHE_VAL(gawk_cv_aix_hack, [
if test -d /lpp
then
	CFLAGS="$CFLAGS -D_XOPEN_SOURCE_EXTENDED=1 -DGAWK_AIX=1"
	gawk_cv_aix_hack=yes
else
	gawk_cv_aix_hack=no
fi
])dnl
AC_MSG_RESULT([${gawk_cv_aix_hack}])
])dnl

dnl Check for Alpha Linux systems
AC_DEFUN([GAWK_AC_LINUX_ALPHA], [
AC_MSG_CHECKING([for Linux/Alpha compilation hacks])
AC_CACHE_VAL(gawk_cv_linux_alpha_hack, [
if test "Linux" = "`uname`" && test "alpha" = "`uname -m`"
then
	# this isn't necessarily always true,
	# the vendor's compiler is also often found
	if test "$GCC" = yes
	then
		CFLAGS="$CFLAGS -mieee"
		gawk_cv_linux_alpha_hack=yes
	else
		gawk_cv_linux_alpha_hack=no
	fi
else
	gawk_cv_linux_alpha_hack=no
fi
])dnl
AC_MSG_RESULT([${gawk_cv_linux_alpha_hack}])
])dnl

dnl Check for z/OS Unix Systems Services
AC_DEFUN([AC_ZOS_USS], [
AC_MSG_CHECKING([for z/OS USS compilation])
AC_CACHE_VAL(ac_cv_zos_uss, [
if test "OS/390" = "`uname`"
then
  ac_cv_zos_uss=yes
else
  ac_cv_zos_uss=no
fi
])dnl
AC_MSG_RESULT([${ac_cv_zos_uss}])
if test "x$ac_cv_zos_uss" = "xyes"
then
  dnl These feature test macros are needed on z/OS.
  CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE -D_XOPEN_SOURCE=600"
  dnl If _C89_OPTIONS is set, then assume the user is building with the c89
  dnl compiler frontend, and knows what they are doing. c89 (as well as cc)
  dnl pre-dates xlc, and accepts an option syntax that involves parentheses
  dnl and cannot be handled in the usual way in C(PP)FLAGS. However, c89
  dnl will also accept options via the aforementioned environment variable,
  dnl which gives the user one way around the problem. (If you're wondering
  dnl about cc, it is meant for programs written in "Common Usage C"
  dnl [a.k.a. K&R C] as opposed to "Standard C" [a.k.a. ANSI], and does not
  dnl build gawk correctly.)
  if test -n "$_C89_OPTIONS"
  then
    AC_MSG_NOTICE([_C89_OPTIONS = $_C89_OPTIONS])
  elif test "x$GCC" != "xyes"
  then
    if echo " $CC " | $EGREP ' (/bin/)?cc ' >/dev/null
    then
      AC_MSG_ERROR([cc-invalid
The z/OS "cc" compiler does not build GNU Awk correctly.

If the "xlc" or "c89" compiler is available, please set CC accordingly
and reconfigure. ("xlc" is the recommended compiler on z/OS.)])
    fi
    if echo " $CC " | $EGREP ' (/bin/)?c89 ' >/dev/null
    then
      AC_MSG_ERROR([c89-setup-required
To build GNU Awk using "c89", please set

    _C89_OPTIONS="-W c,langlvl(stdc99,libext),haltonmsg(CCN3296)"

in your environment, and reconfigure. (The above flags cannot be specified
in CFLAGS/CPPFLAGS, due to the parentheses.)])
    fi
    dnl This enables C99, and on z/OS 1.11, the setenv() prototype.
    CFLAGS="$CFLAGS -qlanglvl=stdc99:libext"
    dnl This is needed so that xlc considers a missing header file to be an
    dnl error and not a warning. (Yes, the latter is in fact the default
    dnl behavior on z/OS.)
    CPPFLAGS="$CPPFLAGS -qhaltonmsg=CCN3296"
  fi
fi
])dnl
