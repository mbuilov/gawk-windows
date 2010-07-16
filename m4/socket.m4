dnl
dnl socket.m4 --- autoconf input file for gawk
dnl
dnl Copyright (C) 1995, 1996, 1998, 1999, 2000, 2003, 2004 the Free Software Foundation, Inc.
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
dnl

dnl Find the socket libraries
dnl largely stolen from AC_PATH_XTRA
AC_DEFUN([GAWK_AC_LIB_SOCKETS], [
gawk_have_sockets=no
# Check for system-dependent location of socket libraries

SOCKET_LIBS=
if test "$ISC" = yes; then
  SOCKET_LIBS="-lnsl_s -linet"
else
  # Martyn.Johnson@cl.cam.ac.uk says this is needed for Ultrix, if the X
  # libraries were built with DECnet support.  And karl@cs.umb.edu says
  # the Alpha needs dnet_stub (dnet does not exist).
  #
  # ADR: Is this needed just for sockets???
#  AC_CHECK_LIB(dnet, dnet_ntoa, [SOCKET_LIBS="$SOCKET_LIBS -ldnet"])
#  if test $ac_cv_lib_dnet_ntoa = no; then
#    AC_CHECK_LIB(dnet_stub, dnet_ntoa,
#	[SOCKET_LIBS="$SOCKET_LIBS -ldnet_stub"])
#  fi

  # msh@cis.ufl.edu says -lnsl (and -lsocket) are needed for his 386/AT,
  # to get the SysV transport functions.
  # chad@anasazi.com says the Pyramid MIS-ES running DC/OSx (SVR4)
  # needs -lnsl.
  # The nsl library prevents programs from opening the X display
  # on Irix 5.2, according to dickey@clark.net.
  AC_CHECK_FUNC(gethostbyname)
  if test $ac_cv_func_gethostbyname = no; then
    AC_CHECK_LIB(nsl, gethostbyname, SOCKET_LIBS="$SOCKET_LIBS -lnsl")
  fi

  # lieder@skyler.mavd.honeywell.com says without -lsocket,
  # socket/setsockopt and other routines are undefined under SCO ODT
  # 2.0.  But -lsocket is broken on IRIX 5.2 (and is not necessary
  # on later versions), says simon@lia.di.epfl.ch: it contains
  # gethostby* variants that don't use the nameserver (or something).
  # -lsocket must be given before -lnsl if both are needed.
  # We assume that if connect needs -lnsl, so does gethostbyname.
  AC_CHECK_FUNC(connect)
  if test $ac_cv_func_connect = no; then
    AC_CHECK_LIB(socket, connect, SOCKET_LIBS="-lsocket $SOCKET_LIBS"
    				  gawk_have_sockets=yes, ,
	$SOCKET_LIBS)
  else
    gawk_have_sockets=yes
  fi
fi

if test "${gawk_have_sockets}" = "yes"
then
	AC_MSG_CHECKING([where to find the socket library calls])
	case "${SOCKET_LIBS}" in
	?*)	gawk_lib_loc="${SOCKET_LIBS}" ;;
	*)	gawk_lib_loc="the standard library" ;;
	esac
	AC_MSG_RESULT([${gawk_lib_loc}])

	AC_DEFINE(HAVE_SOCKETS, 1, [we have sockets on this system])
fi
AC_SUBST(SOCKET_LIBS)dnl
])dnl
