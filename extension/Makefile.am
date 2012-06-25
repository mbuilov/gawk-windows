#
# extension/Makefile.am --- automake input file for gawk
#
# Copyright (C) 1995-2006, 2012 the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

## Process this file with automake to produce Makefile.in.

AM_CPPFLAGS = -I$(srcdir)/..

# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

# Note: rwarray does not currently compile.

pkgextension_LTLIBRARIES =	\
	filefuncs.la	\
	fork.la		\
	ordchr.la	\
	readfile.la	\
	rwarray.la	\
	testext.la	\
	time.la

MY_MODULE_FLAGS = -module -avoid-version -no-undefined

filefuncs_la_SOURCES  = filefuncs.c
filefuncs_la_LDFLAGS  = $(MY_MODULE_FLAGS)
fork_la_SOURCES       = fork.c
fork_la_LDFLAGS       = $(MY_MODULE_FLAGS)
ordchr_la_SOURCES     = ordchr.c
ordchr_la_LDFLAGS     = $(MY_MODULE_FLAGS)
readfile_la_SOURCES   = readfile.c
readfile_la_LDFLAGS   = $(MY_MODULE_FLAGS)
rwarray_la_SOURCES    = rwarray.c
rwarray_la_LDFLAGS    = $(MY_MODULE_FLAGS)
time_la_SOURCES       = time.c
time_la_LDFLAGS       = $(MY_MODULE_FLAGS)
testext_la_SOURCES    = testext.c
testext_la_LDFLAGS    = $(MY_MODULE_FLAGS)

EXTRA_DIST = \
	ChangeLog \
	ChangeLog.0
