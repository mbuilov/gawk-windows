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
