#
# Makefile.am --- automake input file for gawk
#
# Copyright (C) 2000-2005 the Free Software Foundation, Inc.
#
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
#
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

# Automatic de-ANSI-fication if needed, make .bz2 files also.
AUTOMAKE_OPTIONS = ansi2knr dist-bzip2

# This undocumented variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

# This insures that make flags get passed down to child makes.
AM_MAKEFLAGS = 'CFLAGS=$(CFLAGS)' 'LDFLAGS=$(LDFLAGS)'

# Stuff to include in the dist that doesn't need it's own
# Makefile.am files
EXTRA_DIST = \
	COPYING \
	FUTURES \
	INSTALL \
	LIMITATIONS \
	NEWS \
	POSIX.STD \
	PROBLEMS \
	README_d \
	bisonfix.awk \
	config.guess \
	config.rpath  \
	config.sub \
	depcomp \
	extension \
	m4 \
	missing \
	missing_d \
	pc \
	posix \
	regcomp.c \
	regex_internal.c \
	regex_internal.h \
	regexec.c \
	unsupported \
	version.in \
	vms \
	ylwrap

# The order to do things in.
# Build explicitly in "." in order to build gawk first, so
# that `make check' without a prior `make' works.
SUBDIRS = \
	. \
	awklib \
	doc \
	po \
	test

# what to make and install
bin_PROGRAMS = gawk pgawk

# sources for both gawk and pgawk
base_sources = \
	array.c \
	awk.h \
	awkgram.y \
	builtin.c \
	custom.h \
	dfa.c \
	dfa.h \
	ext.c \
	field.c \
	gawkmisc.c \
	getopt.c \
	getopt.h \
	getopt1.c \
	getopt_int.h \
	gettext.h \
	hard-locale.h \
	io.c \
	mbsupport.h \
	main.c \
	msg.c \
	node.c \
	protos.h \
	random.c \
	random.h \
	re.c \
	regex.c \
	regex.h \
	replace.c \
	version.c

gawk_SOURCES = $(base_sources) eval.c profile.c
pgawk_SOURCES = $(base_sources) eval_p.c profile_p.c

# Get extra libs as needed, Automake will supply LIBINTL and SOCKET_LIBS.
LDADD = $(LIBINTL) $(SOCKET_LIBS)

# Directory for gawk's data files. Automake supplies datadir.
pkgdatadir = $(datadir)/awk

# stuff for compiling gawk/pgawk
DEFPATH="\".$(PATH_SEPARATOR)$(pkgdatadir)\""

DEFS= -DDEFPATH=$(DEFPATH) -DHAVE_CONFIG_H -DGAWK -DLOCALEDIR="\"$(datadir)/locale\""

AM_CPPFLAGS =

# Get rid of core files when cleaning
CLEANFILES = core core.*

MAINTAINERCLEANFILES = version.c awkgram.c

# We want hard links for install-exec-hook, below
LN= ln

SUFFIXES = .i
.c.i:
	$(COMPILE) -E $< > $@

# First, add a link from gawk to gawk-X.Y.Z.
# Same for pgawk.
#
# For GNU systems where gawk is awk, add a link to awk.
# (This is done universally, which may not always be right, but
# there's no easy way to distinguish GNU from non-GNU systems.)
install-exec-hook:
	(cd $(DESTDIR)$(bindir); \
	$(LN) gawk$(EXEEXT) gawk-$(VERSION)$(EXEEXT) 2>/dev/null ; \
	$(LN) pgawk$(EXEEXT) pgawk-$(VERSION)$(EXEEXT) 2>/dev/null ; \
	if [ ! -f awk$(EXEEXT) ]; \
	then	$(LN_S) gawk$(EXEEXT) awk$(EXEEXT); \
	fi; exit 0)

# Undo the above when uninstalling
uninstall-links:
	(cd $(DESTDIR)$(bindir); \
	if [ -f awk$(EXEEXT) ] && cmp awk$(EXEEXT) gawk$(EXEEXT) > /dev/null; then rm -f awk$(EXEEXT); fi ; \
	rm -f gawk-$(VERSION)$(EXEEXT) pgawk-$(VERSION)$(EXEEXT); exit 0)

uninstall-recursive: uninstall-links

# force there to be a gawk executable before running tests
check-local: gawk$(EXEEXT) pgawk$(EXEEXT)

# A little extra clean up when making distributions.
# FIXME: most of this rule should go away upon switching to libtool.
dist-hook:
	cd $(distdir)/extension ; rm -f *.o *.so

# Special rules for individual files
awkgram.c: awkgram.y
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $<
	awk -f $(srcdir)/bisonfix.awk y.tab.c > $*.c && rm y.tab.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi

# This is for my development & testing.
efence: gawk
	$(CC) $(LDFLAGS) -o gawk $$(ls *.o | grep -v '_p.o$$') $(LIBS) -lefence

diffout:
	@$(MAKE) -C test $@
