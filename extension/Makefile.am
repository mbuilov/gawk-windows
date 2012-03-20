#
# extension/Makefile.am --- automake input file for gawk
#
# Copyright (C) 1995-2006 the Free Software Foundation, Inc.
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

INCLUDES = -I.. -I$(top_srcdir) -I$(top_srcdir)/intl 

lib_LTLIBRARIES =	\
	libarrayparm.la	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la	\
	libtestarg.la

# The arrayparm, zaxxon, and testarg libraries do not do anything useful,
# so do not install them...

install_libraries =	\
	libfilefuncs.la	\
	libfork.la	\
	libordchr.la	\
	libreadfile.la

libarrayparm_la_SOURCES  = arrayparm.c
libfilefuncs_la_SOURCES  = filefuncs.c
libfork_la_SOURCES       = fork.c
libordchr_la_SOURCES     = ordchr.c
libreadfile_la_SOURCES   = readfile.c
libtestarg_la_SOURCES    = testarg.c

install: $(install_libraries)
	for p in $(install_libraries); do \
		. ./$$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Installing $$dlname in $(DESTDIR)$(pkgextensiondir)/$$f" ; \
		$(INSTALL_PROGRAM) .libs/$$dlname $(DESTDIR)$(pkgextensiondir)/$$f ; \
	done

uninstall: $(install_libraries)
	for p in $(install_libraries); do \
		. ./$$p ; \
		f=`echo $$dlname | sed -e 's/^lib//' -e 's/\.0//g'` ; \
		echo "Removing $$dlname in $(DESTDIR)$(pkgextensiondir)/$$f" ; \
		rm -f $(DESTDIR)$(pkgextensiondir)/$$f ; \
	done

EXTRA_DIST = \
	ChangeLog \
	ChangeLog.0 \
	*.awk \
	doit \
	steps
