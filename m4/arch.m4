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
  CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE -D_XOPEN_SOURCE=600"
  if test "x$GCC" != "xyes"
  then
    dnl If the user is using the "cc" or "c89" compiler frontends, then
    dnl give up. These do not accept standard XL C -qfoobar options, and
    dnl instead use a devil's-spawn option syntax involving parentheses.
    dnl (For example, the below CFLAGS addendum becomes
    dnl "-W c,langlvl(stdc99,libext)". Good luck quoting that.)
    if echo " $CC " | $EGREP ' (/bin/)?(cc|c89) ' >/dev/null
    then
:      AC_MSG_ERROR([invalid-cc
GNU Awk does not support the "cc" nor "c89" compiler frontends on z/OS.
Please set CC to "c99" or one of the "xlc" frontends.])
    fi
    dnl This enables C99, and on z/OS 1.11, the setenv() prototype.
:    CFLAGS="$CFLAGS -qlanglvl=stdc99:libext"
    dnl This is needed so that xlc considers a missing header file to be an
    dnl error and not a warning. (Yes, the latter is in fact the default
    dnl behavior on z/OS.)
:    CPPFLAGS="$CPPFLAGS -qhaltonmsg=CCN3296"
  fi
  ac_cv_zos_uss=yes
else
  ac_cv_zos_uss=no
fi
])dnl
AC_MSG_RESULT([${ac_cv_zos_uss}])
])dnl
