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

EXTRA_DIST = ChangeLog extract.awk eg $(srcdir)/stamp-eg
# With some locales, the script extract.awk fails.
# So we fix the locale to some sensible value.
AWKPROG = LC_ALL=C LANG=C ../gawk$(EXEEXT)

# Get config.h from the build directory and custom.h from the source directory.
AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

pkgdatadir = $(datadir)/awk
pkglibexecdir = $(libexecdir)/awk

bin_SCRIPTS = igawk
pkglibexec_PROGRAMS = pwcat grcat
AUXAWK = passwd.awk group.awk
nodist_grcat_SOURCES = grcat.c
nodist_pwcat_SOURCES = pwcat.c

all: $(srcdir)/stamp-eg $(AUXPROGS) igawk $(AUXAWK)

install-exec-hook: $(AUXAWK)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	for i in $(AUXAWK) $(srcdir)/eg/lib/*.awk ; do \
		progname=`echo $$i | sed 's;.*/;;'` ; \
		$(INSTALL_DATA) $$i $(DESTDIR)$(pkgdatadir)/$$progname ; \
	done

# pkglibexecdir and pkgdatadir are removed in the top level Makefile's uninstall
uninstall-local:
	rm -fr $(DESTDIR)$(pkglibexecdir)/* $(DESTDIR)$(pkgdatadir)/*
	rm -f $(DESTDIR)$(bindir)/igawk

clean-local:
	rm -f $(AUXAWK) igawk *.exe
	rm -fr eg.old

$(srcdir)/stamp-eg: $(srcdir)/../doc/gawk.texi $(srcdir)/../doc/gawkinet.texi
	cd $(srcdir) && \
	mv eg eg.old && \
	rm -fr  stamp-eg && \
	$(AWKPROG) -f extract.awk ../doc/gawk.texi ../doc/gawkinet.texi
	@echo 'some makes are stupid and will not check a directory' > $(srcdir)/stamp-eg
	@echo 'against a file, so this file is a place holder. gack.' >> $(srcdir)/stamp-eg
	cd $(srcdir) && \
	if [ -d eg.old ] && [ -d eg.old/CVS ] ; then \
	mv eg.old/CVS eg ; \
	mv eg.old/data/CVS eg/data ; \
	mv eg.old/lib/CVS eg/lib ; \
	mv eg.old/misc/CVS eg/misc ; \
	mv eg.old/network/CVS eg/network ; \
	mv eg.old/prog/CVS eg/prog ; \
	rm -fr eg.old ; fi

$(srcdir)/eg/lib/pwcat.c $(srcdir)/eg/lib/grcat.c $(srcdir)/eg/prog/igawk.sh \
$(srcdir)/eg/lib/passwdawk.in $(srcdir)/eg/lib/groupawk.in: stamp-eg; @:

pwcat$(EXEEXT): $(srcdir)/eg/lib/pwcat.c
	$(COMPILE) $(srcdir)/eg/lib/pwcat.c $(LDFLAGS) -o $@

grcat$(EXEEXT): $(srcdir)/eg/lib/grcat.c
	$(COMPILE) $(srcdir)/eg/lib/grcat.c $(LDFLAGS) -o $@

igawk: $(srcdir)/eg/prog/igawk.sh
	cp $(srcdir)/eg/prog/igawk.sh $@ ; chmod 755 $@

passwd.awk: $(srcdir)/eg/lib/passwdawk.in
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < $(srcdir)/eg/lib/passwdawk.in > passwd.awk

group.awk: $(srcdir)/eg/lib/groupawk.in
	sed 's;/usr/local/libexec/awk;$(pkglibexecdir);' < $(srcdir)/eg/lib/groupawk.in > group.awk
