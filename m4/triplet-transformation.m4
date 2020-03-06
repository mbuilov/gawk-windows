dnl
dnl _AX_TRIPLET_TRANSFORMATION(SYSTEM-TYPE, SED-SCRIPT [, HEADER])
dnl with
dnl     SYSTEM-TYPE : `build', `host' or `target'
dnl     SED-SCRIPT  : valid sed script transforming the triplet in
dnl                    variable `ac_cv_<SYSTEM-TYPE>'
dnl     HEADER      : message header printed when triplet conversion
dnl                    is applied
dnl
dnl Read the triplet from the cache variable `ac_cv_<SYSTEM-TYPE>',
dnl try to transform it using the sed script SED-SCRIPT.  If the
dnl transformation results in a change
dnl   - backup the original triplet in the cache variable
dnl      `<PROGRAM>_cv_<SYSTEM-TYPE>_orig'
dnl   - save the transformed one in `ac_cv_<SYSTEM-TYPE>'
dnl   - update the variables `<SYSTEM-TYPE>-(os|cpu|vendor)' (which is the
dnl       ultimate goal of this macro).
dnl
AC_DEFUN([_AX_TRIPLET_TRANSFORMATION],
[AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
AC_REQUIRE_AUX_FILE([config.sub])dnl
dnl This part should appear in the configure script AFTER the block
dnl handling the original sytem-type variables build/host/target.
AS_VAR_PUSHDEF([ax_triplet_orig],[]AC_PACKAGE_TARNAME[_cv_$1_orig])
dnl The real configure variable `ac_cv_target' might be void.
if test x$ac_cv_$1 != x; then
dnl here the transformation happens.
  ax_tt_var=`echo $ac_cv_$1 | sed '$2'`
  if test x$ax_tt_var = x; then
    AC_MSG_NOTICE([sed script: $1 triplet conversion `$ac_cv_$1' with sed script `$2'failed.])
dnl test if transformed triplet is known to config.sub, hence probably to most of the autotools.
  elif test x$ax_tt_var != x`$SHELL "$ac_aux_dir/config.sub" $ax_tt_var`; then
    AC_MSG_NOTICE([`config.sub' test: $1 triplet conversion of `$ac_cv_$1' produced invalid triplet `$ax_tt_var'.])
dnl did anything change at all?
  elif test x$ax_tt_var != x$ac_cv_$1; then
dnl print the header just once in one configure script
    m4_ifnblank([$3],[dnl
      if test x$ax_tt_header_is_printed != xyes; then
        ax_tt_header_is_printed=yes
        AC_MSG_NOTICE([$3])
      fi])
    ax_triplet_orig=$ac_cv_$1
    ac_cv_$1=$ax_tt_var
    AC_MSG_NOTICE([$1: replacing $ax_triplet_orig -> $ac_cv_$1])
dnl Better neutralize ax_tt_var because we call another macro whic might use this
dnl variable, too.
    ax_tt_var=
dnl use of autoconf internal: rerun the triplet split cpu/vendor/host
    _AC_CANONICAL_SPLIT([$1])
  fi
  ax_tt_var=
fi
AS_VAR_POPDEF([ax_triplet_orig])dnl
])dnl _AX_TRIPLET_TRANSFORMATION

dnl
dnl AX_CANONICAL_HOST(PLATFORM-PATTERN, SED-SCRIPT [, TITLE])
dnl
dnl If UNAME matches PLATFORM-PATTERN convert the build and/or host triplets
dnl using SED-SCRIPT.  Have configure emit a message if a transformation
dnl occurs.
dnl
dnl This macro works __inside__ configure as if it was called like so on the
dnl command line:
dnl
dnl #! /usr/bin/bash
dnl  case $(uname) in
dnl   <PLATFORM-PATTERN>)
dnl     test -z $mytriplet && mytriplet=$(./config.sub $(./config.guess))
dnl     ./configure --host=$(echo $mytriplet | sed '<SED-SCRIPT>')
dnl     ;;
dnl   *) ./configure
dnl  esac
dnl
dnl The macro calls implicitly AC_CANONICAL_HOST.
dnl
dnl To bypass the macro call such that it should not have any effect
dnl on the configure run pass the NON-VOID variable `ax_disable_triplet_transform'
dnl to configure. E.g.,
dnl
dnl    ./configure ax_disable_triplet_transform=yes
dnl
dnl In this case it defaults to calling `AC_CANONICAL_HOST'.
dnl
AC_DEFUN_ONCE([AX_CANONICAL_HOST],
[AC_BEFORE([$0],[AC_CANONICAL_BUILD])dnl
AC_BEFORE([$0],[AC_CANONICAL_HOST])dnl
ax_tt_header_is_printed=:
dnl skip case statement if ax_disable_triplet_transform is set to
dnl any non-void string or UNNAME does not match .
AS_CASE([x$ax_disable_triplet_transform/`uname`],[x/$1],
  [_AX_TRIPLET_TRANSFORMATION([build],[$2],[$3])
   _AX_TRIPLET_TRANSFORMATION([host],[$2],[$3])
   AC_CANONICAL_HOST
])])dnl AX_CANONICAL_HOST

dnl
dnl GAWK_CANONICAL_HOST
dnl
dnl This hackery shall fool LIBTOOL and make it believe under MSYS
dnl platforms that for build and/or host triplets with vendor `msys'
dnl we are on CYGWIN where LIBTOOL(2.4.6/Jan 2020) can handle well
dnl shared module libraries.
dnl
dnl Call `GAWK_CANONICAL_HOST' instead of `AC_CANONICAL_HOST' high up
dnl in configure.ac.
dnl
dnl Bypass the transformation procedure and simply call AC_CANONICAL_HOST instead
dnl with
dnl
dnl     ./configure ax_disable_triplet_transform=yes
dnl
dnl Note that with this macro every setting for CYGWIN will be applied to MSYS.
dnl It is tested for the predominant use of generating shared module libraries
dnl on MSYS.
dnl
dnl TODO remove when MSYS is supported by LIBTOOL.
dnl
AC_DEFUN_ONCE([GAWK_CANONICAL_HOST],
[AX_CANONICAL_HOST([MSYS*],[s|-msys$|-cygwin|],[Triplet conversion on MSYS platform:])
])dnl GAWK_CANONICAL_HOST
