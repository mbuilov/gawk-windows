dnl
dnl configure.ac --- autoconf input file for gawk
dnl
dnl Copyright (C) 2012 the Free Software Foundation, Inc.
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

AC_INIT([GNU Awk Bundled Extensions], 4.0.70, bug-gawk@gnu.org, gawk-extensions)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

INSTALL="$ac_aux_dir/install-sh -c"
export INSTALL

AM_INIT_AUTOMAKE([-Wall -Werror])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

AM_PROG_AR
AC_SYS_LARGEFILE
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
dnl AC_PROG_INSTALL

AC_SUBST([pkgextensiondir], ['${libdir}/gawk'])

if test "$GCC" = yes
then
	CFLAGS="$CFLAGS -Wall -Wextra"
fi

AC_MSG_CHECKING([for special development options])
if test -f $srcdir/.developing
then
	if test "$GCC" = yes
	then
		CFLAGS="$CFLAGS -Wall -fno-builtin -g3 -gdwarf-2"
	fi
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi

AC_CHECK_HEADERS(dirent.h fnmatch.h time.h sys/time.h sys/select.h)

AC_CHECK_FUNCS(fdopendir fnmatch gettimeofday \
		getdtablesize nanosleep select GetSystemTimeAsFileTime)

dnl checks for compiler characteristics
AC_C_INLINE

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
