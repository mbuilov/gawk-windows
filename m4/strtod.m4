dnl
dnl strtod.m4 --- autoconf input file for gawk
dnl
dnl Copyright (C) 2001, 2002, 2004 the Free Software Foundation, Inc.
dnl
dnl This file is part of GAWK, the GNU implementation of the
dnl AWK Progamming Language.
dnl
dnl GAWK is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
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

dnl Check for strtod with C89 semantics

AC_DEFUN([GAWK_AC_FUNC_STRTOD_C89], [
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_FUNCS(strtod)
AC_CACHE_CHECK([for strtod with C89 semantics], gawk_ac_cv_func_strtod_c89,
[AC_TRY_RUN(
[/* Test program from Arnold Robbins (arnold@skeeve.com) */
#if HAVE_STDLIB_H
#include <stdlib.h>
#else
extern double strtod();
#endif

int
main ()
{
#if ! HAVE_STRTOD
  exit(1);
#else
  double d;
  char *str = "0x345a";

  d = strtod(str, 0);
  if (d == 0)
     exit (0);
  else
     exit (1);
#endif
}],
gawk_ac_cv_func_strtod_c89=yes, gawk_ac_cv_func_strtod_c89=no,
gawk_ac_cv_func_strtod_c89=no)])
if test $gawk_ac_cv_func_strtod_c89 = no; then
  AC_DEFINE(STRTOD_NOT_C89, 1, [strtod doesn't have C89 semantics])
fi
])# GAWK_FUNC_STRTOD_C89
