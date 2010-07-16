dnl
dnl aclocal.m4 --- autoconf input file for gawk
dnl 
dnl Copyright (C) 1995, 1996, 1998, 1999 the Free Software Foundation, Inc.
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
], gawk_cv_c_stringize=no, gawk_cv_c_stringize=yes))
if test "${gawk_cv_c_stringize}" = yes
then
	AC_DEFINE(HAVE_STRINGIZE)
fi
AC_MSG_RESULT([${gawk_cv_c_stringize}])
])dnl


dnl By default, many hosts won't let programs access large files;
dnl one must use special compiler options to get large-file access to work.
dnl For more details about this brain damage please see:
dnl http://www.sas.com/standards/large.file/x_open.20Mar96.html

dnl Written by Paul Eggert <eggert@twinsun.com>.

dnl Internal subroutine of GAWK_AC_SYS_LARGEFILE.
dnl GAWK_AC_SYS_LARGEFILE_FLAGS(FLAGSNAME)
AC_DEFUN(GAWK_AC_SYS_LARGEFILE_FLAGS,
  [AC_CACHE_CHECK([for $1 value to request large file support],
     gawk_cv_sys_largefile_$1,
     [gawk_cv_sys_largefile_$1=`($GETCONF LFS_$1) 2>/dev/null` || {
	gawk_cv_sys_largefile_$1=no
	ifelse($1, CFLAGS,
	  [case "$host_os" in
	   # IRIX 6.2 and later require cc -n32.
changequote(, )dnl
	   irix6.[2-9]* | irix6.1[0-9]* | irix[7-9].* | irix[1-9][0-9]*)
changequote([, ])dnl
	     if test "$GCC" != yes; then
	       gawk_cv_sys_largefile_CFLAGS=-n32
	     fi
	     gawk_save_CC="$CC"
	     CC="$CC $gawk_cv_sys_largefile_CFLAGS"
	     AC_TRY_LINK(, , , gawk_cv_sys_largefile_CFLAGS=no)
	     CC="$gawk_save_CC"
	   esac])
      }])])

dnl Internal subroutine of GAWK_AC_SYS_LARGEFILE.
dnl GAWK_AC_SYS_LARGEFILE_SPACE_APPEND(VAR, VAL)
AC_DEFUN(GAWK_AC_SYS_LARGEFILE_SPACE_APPEND,
  [case $2 in
   no) ;;
   ?*)
     case "[$]$1" in
     '') $1=$2 ;;
     *) $1=[$]$1' '$2 ;;
     esac ;;
   esac])

dnl Internal subroutine of GAWK_AC_SYS_LARGEFILE.
dnl GAWK_AC_SYS_LARGEFILE_MACRO_VALUE(C-MACRO, CACHE-VAR, CODE-TO-SET-DEFAULT)
AC_DEFUN(GAWK_AC_SYS_LARGEFILE_MACRO_VALUE,
  [AC_CACHE_CHECK([for $1], $2,
     [$2=no
changequote(, )dnl
      $3
      for gawk_flag in $gawk_cv_sys_largefile_CFLAGS no; do
	case "$gawk_flag" in
	-D$1)
	  $2=1 ;;
	-D$1=*)
	  $2=`expr " $gawk_flag" : '[^=]*=\(.*\)'` ;;
	esac
      done
changequote([, ])dnl
      ])
   if test "[$]$2" != no; then
     AC_DEFINE_UNQUOTED([$1], [$]$2)
   fi])

AC_DEFUN(GAWK_AC_SYS_LARGEFILE,
  [AC_REQUIRE([AC_CANONICAL_HOST])
   AC_ARG_ENABLE(largefile,
     [  --disable-largefile     omit support for large files])
   if test "$enable_largefile" != no; then
     AC_CHECK_TOOL(GETCONF, getconf)
     GAWK_AC_SYS_LARGEFILE_FLAGS(CFLAGS)
     GAWK_AC_SYS_LARGEFILE_FLAGS(LDFLAGS)
     GAWK_AC_SYS_LARGEFILE_FLAGS(LIBS)
	
     for gawk_flag in $gawk_cv_sys_largefile_CFLAGS no; do
       case "$gawk_flag" in
       no) ;;
       -D_FILE_OFFSET_BITS=*) ;;
       -D_LARGEFILE_SOURCE | -D_LARGEFILE_SOURCE=*) ;;
       -D_LARGE_FILES | -D_LARGE_FILES=*) ;;
       -D?* | -I?*)
	 GAWK_AC_SYS_LARGEFILE_SPACE_APPEND(CPPFLAGS, "$gawk_flag") ;;
       *)
	 GAWK_AC_SYS_LARGEFILE_SPACE_APPEND(CFLAGS, "$gawk_flag") ;;
       esac
     done
     GAWK_AC_SYS_LARGEFILE_SPACE_APPEND(LDFLAGS, "$gawk_cv_sys_largefile_LDFLAGS")
     GAWK_AC_SYS_LARGEFILE_SPACE_APPEND(LIBS, "$gawk_cv_sys_largefile_LIBS")
     GAWK_AC_SYS_LARGEFILE_MACRO_VALUE(_FILE_OFFSET_BITS,
       gawk_cv_sys_file_offset_bits,
       [case "$host_os" in
	# HP-UX 10.20 and later
	hpux10.[2-9][0-9]* | hpux1[1-9]* | hpux[2-9][0-9]*)
	  gawk_cv_sys_file_offset_bits=64 ;;
	esac])
     GAWK_AC_SYS_LARGEFILE_MACRO_VALUE(_LARGEFILE_SOURCE,
       gawk_cv_sys_largefile_source,
       [case "$host_os" in
	# HP-UX 10.20 and later
	hpux10.[2-9][0-9]* | hpux1[1-9]* | hpux[2-9][0-9]*)
	  gawk_cv_sys_largefile_source=1 ;;
	esac])
     GAWK_AC_SYS_LARGEFILE_MACRO_VALUE(_LARGE_FILES,
       gawk_cv_sys_large_files,
       [case "$host_os" in
	# AIX 4.2 and later
	aix4.[2-9]* | aix4.1[0-9]* | aix[5-9].* | aix[1-9][0-9]*)
	  gawk_cv_sys_large_files=1 ;;
	esac])
   fi
  ])

dnl Check for AIX and add _XOPEN_SOURCE_EXTENDED
AC_DEFUN(GAWK_AC_AIX_TWEAK, [
AC_MSG_CHECKING([for AIX compilation hacks])
AC_CACHE_VAL(gawk_cv_aix_hack, [
if test -d /lpp/bos
then
	CFLAGS="$CFLAGS -D_XOPEN_SOURCE_EXTENDED=1"
	gawk_cv_aix_hack=yes
else
	gawk_cv_aix_hack=no
fi
])dnl
AC_MSG_RESULT([${gawk_cv_aix_hack}])
])dnl
