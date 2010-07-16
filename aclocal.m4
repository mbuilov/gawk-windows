dnl
dnl aclocal.m4 --- autoconf input file for gawk
dnl 
dnl Copyright (C) 1995, 96 the Free Software Foundation, Inc.
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
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
dnl

dnl gawk-specific macros for autoconf. one day hopefully part of autoconf

AC_DEFUN(GAWK_AC_C_STRINGIZE, [
AC_REQUIRE([AC_PROG_CPP])
AC_MSG_CHECKING([for ANSI stringizing capability])
AC_CACHE_VAL(gawk_cv_c_stringize, 
AC_EGREP_CPP([#teststring],[
#define x(y) #y

char *s = x(teststring);


dnl To: Keith Bostic <bostic@bsdi.com>
dnl Cc: bug-gnu-utils@prep.ai.mit.edu
dnl Subject: Re: autoconf-2.7
dnl From: Jim Meyering <meyering@asic.sc.ti.com>
dnl Date: 15 Oct 1996 11:57:12 -0500
dnl 
dnl | The assumption that, if a simple program can't be compiled and
dnl | run, that the user is doing cross-compilation, is causing me
dnl | serious grief.  The problem is that Solaris ships a cc command
dnl | that just fails, if you haven't bought their compiler.
dnl | 
dnl | What the user is eventually told is that it's not possible to
dnl | run test programs when cross-compiling, which doesn't point them
dnl | at the right problem.
dnl | 
dnl | Maybe it's just me, but I don't know too many normal users that
dnl | do cross-compilation.  I'd like to see a more stringent test to
dnl | decide if we're doing cross-compilation.  (Maybe with a message
dnl | to use gcc!?!?  ;-})
dnl 
dnl Now I put this line in configure.in files:
dnl 
dnl AM_SANITY_CHECK_CC
dnl 
dnl Here's the macro that goes in aclocal.m4 -- it should be in the
dnl next official release of automake.

AC_DEFUN(AM_SANITY_CHECK_CC,
[dnl Derived from macros from Bruno Haible and from Cygnus.
AC_MSG_CHECKING([whether the compiler ($CC $CFLAGS $LDFLAGS) actually works])
AC_LANG_SAVE
  AC_LANG_C
  AC_TRY_RUN([main() { exit(0); }],
             am_cv_prog_cc_works=yes, am_cv_prog_cc_works=no,
             dnl When crosscompiling, just try linking.
             AC_TRY_LINK([], [], am_cv_prog_cc_works=yes,
                         am_cv_prog_cc_works=no))
AC_LANG_RESTORE
case "$am_cv_prog_cc_works" in
  *no) AC_MSG_ERROR([Installation or configuration problem: C compiler cannot cr
eate executables.]) ;;
  *yes) ;;
esac
AC_MSG_RESULT(yes)
])dnl
], gawk_cv_c_stringize=no, gawk_cv_c_stringize=yes))
if test "${gawk_cv_c_stringize}" = yes
then
	AC_DEFINE(HAVE_STRINGIZE)
fi
AC_MSG_RESULT([${gawk_cv_c_stringize}])
])dnl


dnl To: Keith Bostic <bostic@bsdi.com>
dnl Cc: bug-gnu-utils@prep.ai.mit.edu
dnl Subject: Re: autoconf-2.7
dnl From: Jim Meyering <meyering@asic.sc.ti.com>
dnl Date: 15 Oct 1996 11:57:12 -0500
dnl 
dnl | The assumption that, if a simple program can't be compiled and
dnl | run, that the user is doing cross-compilation, is causing me
dnl | serious grief.  The problem is that Solaris ships a cc command
dnl | that just fails, if you haven't bought their compiler.
dnl | 
dnl | What the user is eventually told is that it's not possible to
dnl | run test programs when cross-compiling, which doesn't point them
dnl | at the right problem.
dnl | 
dnl | Maybe it's just me, but I don't know too many normal users that
dnl | do cross-compilation.  I'd like to see a more stringent test to
dnl | decide if we're doing cross-compilation.  (Maybe with a message
dnl | to use gcc!?!?  ;-})
dnl 
dnl Now I put this line in configure.in files:
dnl 
dnl AM_SANITY_CHECK_CC
dnl 
dnl Here's the macro that goes in aclocal.m4 -- it should be in the
dnl next official release of automake.

AC_DEFUN(AM_SANITY_CHECK_CC,
[dnl Derived from macros from Bruno Haible and from Cygnus.
AC_MSG_CHECKING([whether the compiler ($CC $CFLAGS $LDFLAGS) actually works])
AC_LANG_SAVE
  AC_LANG_C
  AC_TRY_RUN([main() { exit(0); }],
             am_cv_prog_cc_works=yes, am_cv_prog_cc_works=no,
             dnl When crosscompiling, just try linking.
             AC_TRY_LINK([], [], am_cv_prog_cc_works=yes,
                         am_cv_prog_cc_works=no))
AC_LANG_RESTORE
case "$am_cv_prog_cc_works" in
  *no) AC_MSG_ERROR([Installation or configuration problem: C compiler cannot cr
eate executables.]) ;;
  *yes) ;;
esac
AC_MSG_RESULT(yes)
])dnl
