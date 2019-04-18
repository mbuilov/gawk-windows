#
# awklib/Makefile.am --- automake input file for gawk
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

## process this file with automake to produce Makefile.in

EXTRA_DIST = ChangeLog ChangeLog.0 ChangeLog.1 \
		extract.awk eg $(srcdir)/stamp-eg
# With some locales, the script extract.awk fails.
# So we fix the locale to some sensible value.

if TEST_CROSS_COMPILE
AWKPROG = LC_ALL=C LANG=C awk$(EXEEXT)
else
AWKPROG = LC_ALL=C LANG=C "$(abs_top_builddir)/gawk$(EXEEXT)"
endif

# Get config.h from the build directory and custom.h from the source directory.
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

pkgdatadir = $(datadir)/awk
pkglibexecdir = $(libexecdir)/awk

pkglibexec_PROGRAMS = pwcat grcat
AUXAWK = passwd.awk group.awk
nodist_grcat_SOURCES = grcat.c
nodist_pwcat_SOURCES = pwcat.c

CLEANFILES = $(nodist_grcat_SOURCES) $(nodist_pwcat_SOURCES)

all: $(srcdir)/stamp-eg $(AUXPROGS) $(AUXAWK)

install-exec-hook: $(AUXAWK)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	for i in $(AUXAWK) $(srcdir)/eg/lib/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done

# pkglibexecdir and pkgdatadir are removed in the top level Makefile's uninstall
uninstall-local:
	rm -fr $(DESTDIR)$(pkglibexecdir)/* $(DESTDIR)$(pkgdatadir)/*

clean-local:
	rm -f $(AUXAWK) *.exe
	rm -fr eg.old
	rm -fr grcat.dSYM pwcat.dSYM

$(srcdir)/stamp-eg: $(srcdir)/../doc/gawktexi.in $(srcdir)/../doc/gawkinet.texi
	cd $(srcdir) && \
	rm -fr eg && \
	rm -fr stamp-eg && \
	$(AWKPROG) -f ./extract.awk ../doc/gawktexi.in ../doc/gawkinet.texi
	@echo 'some makes are stupid and will not check a directory' > $(srcdir)/stamp-eg
	@echo 'against a file, so this file is a place holder. gack.' >> $(srcdir)/stamp-eg

pwcat.c: $(srcdir)/eg/lib/pwcat.c
	rm -f $@
	$(LN_S) $(srcdir)/eg/lib/pwcat.c .

grcat.c: $(srcdir)/eg/lib/grcat.c
	rm -f $@
	$(LN_S) $(srcdir)/eg/lib/grcat.c .

$(srcdir)/eg/lib/pwcat.c $(srcdir)/eg/lib/grcat.c $(srcdir)/eg/prog/igawk.sh \
$(srcdir)/eg/lib/passwdawk.in $(srcdir)/eg/lib/groupawk.in: stamp-eg; @:

passwd.awk: $(srcdir)/eg/lib/passwdawk.in
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < $(srcdir)/eg/lib/passwdawk.in > passwd.awk

group.awk: $(srcdir)/eg/lib/groupawk.in
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < $(srcdir)/eg/lib/groupawk.in > group.awk

distclean-local:
	rm -fr .deps
