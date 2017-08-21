dnl
dnl arch.m4 --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995, 1996, 1998, 1999, 2000, 2003, 2004, 2015
dnl the Free Software Foundation, Inc.
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
  dnl Identify the compiler.
  ac_zos_uss_cc_id=unknown
  echo " $CC " | $EGREP [' (/bin/)?c89[ |_]'] >/dev/null && ac_zos_uss_cc_id=c89
  echo " $CC " | $EGREP [' (/bin/)?c99[ |_]'] >/dev/null && ac_zos_uss_cc_id=xlc
  echo " $CC " | $EGREP [' (/bin/)?cc[ |_]']  >/dev/null && ac_zos_uss_cc_id=cc
  echo " $CC " | $EGREP [' (/bin/)?xlc[ |_]'] >/dev/null && ac_zos_uss_cc_id=xlc
  echo " $CC " | $EGREP [' (/bin/)?xlC[ |_]'] >/dev/null && ac_zos_uss_cc_id=xlc++
  echo " $CC " | $EGREP [' (/bin/)?xlc\+\+[ |_]'] >/dev/null && ac_zos_uss_cc_id=xlc++
  test "x$GCC" = "xyes" && ac_zos_uss_cc_id=gcc
  dnl These feature test macros are needed on z/OS.
  CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE -D_OPEN_SYS_UNLOCKED_EXT"
  test "$ac_zos_uss_cc_id" != xlc++ && CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=600"
  case "$ac_zos_uss_cc_id" in
  c89)
    dnl If the user has set CC=c89 and _C89_OPTIONS, then don't get in
    dnl their way.  c89 (as well as cc) pre-dates xlc, and accepts an
    dnl option syntax that involves parentheses and cannot be handled
    dnl in the usual way in C(PP)FLAGS.  However, c89 will also accept
    dnl options via the aforementioned environment variable, which
    dnl gives the user one way around the problem.  (If you're
    dnl wondering about cc, it is meant for programs written in
    dnl "Common Usage C" [a.k.a. K&R C] as opposed to "Standard C"
    dnl [a.k.a. ANSI], and does not build gawk correctly.)
    if test -n "$_C89_OPTIONS"
    then
      AC_MSG_NOTICE([CC = $CC])
      AC_MSG_NOTICE([_C89_OPTIONS = $_C89_OPTIONS])
    else
      AC_MSG_ERROR([c89-setup-required
To build GNU Awk using "c89", please set

    _C89_OPTIONS="-W c,langlvl(stdc99,libext),haltonmsg(CCN3296)"

in your environment, and reconfigure. (The above flags cannot be specified
in CFLAGS/CPPFLAGS, due to the parentheses.)])
    fi
    ;;
  gcc)
    dnl GCC has not yet been ported to z/OS as of this writing
    ;;
  cc)
    AC_MSG_ERROR([cc-invalid
The z/OS "cc" compiler does not build GNU Awk correctly.

If the "xlc" or "c89" compiler is available, please set CC accordingly
and reconfigure. ("xlc" is the recommended compiler on z/OS.)])
    ;;
  xlc*)
    dnl This enables C99, and on z/OS 1.11, the setenv() prototype.
    CFLAGS="$CFLAGS -qlanglvl=stdc99:libext"
    dnl Use a compiler wrapper script to address some annoyances.
    cat >zos-cc <<EOF
#!/bin/sh
#
# This wrapper script addresses two annoying peculiarities of the IBM
# xlc/c99 compiler on z/OS:
#
# 1. Missing header files are considered warnings by default rather
#    than fatal errors;
#
# 2. Include directories specified with -I are searched _after_ the
#    system include directories (for #include<> directives) rather
#    than before.
#
# This script allows the code and the build system to assume standard
# compiler behavior.
#

PS4='zos-cc: '
REAL_CC="$CC"

set -x
\$REAL_CC -qhaltonmsg=CCN3296 -qnosearch "\$[]@" -qsearch=/usr/include
EOF
    chmod +x zos-cc
    AC_MSG_NOTICE([wrapping $CC with zos-cc to obtain standard behavior])
    CC="`pwd`/zos-cc"
    ;;
  *)
    AC_MSG_WARN([unrecognized compiler environment])
    ;;
  esac
fi # ac_cv_zos_uss = yes
])dnl
