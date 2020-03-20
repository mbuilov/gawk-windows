#
# Makefile.am --- automake input file for gawk
#
# Copyright (C) 2000-2020 the Free Software Foundation, Inc.
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

# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @CFLAGS@
AM_LDFLAGS = @LDFLAGS@

# Stuff to include in the dist that doesn't need it's own
# Makefile.am files
EXTRA_DIST = \
	ChangeLog.0 \
	ChangeLog.1 \
	COPYING \
	INSTALL \
	NEWS \
	NEWS.0 \
	NEWS.1 \
	POSIX.STD \
	README_d \
	config.guess \
	config.rpath  \
	config.sub \
	depcomp \
	m4 \
	missing \
	missing_d \
	po/README \
	pc \
	posix \
	vms \
	ylwrap

# The order to do things in.
#
# Build in support first, since we need the support library.
#
# Build explicitly in "." in order to build gawk first, so
# that `make check' without a prior `make' works.
SUBDIRS = support .

# Build in extension before test so that
# 	./configure && make check
# works properly too.
if ENABLE_EXTENSIONS
SUBDIRS += extension
endif

# Build in awklib after in doc, since we want to extract
# sample files if doc/gawk.texi changed.
SUBDIRS += extras doc awklib po test

# what to make and install
bin_PROGRAMS = gawk
include_HEADERS = gawkapi.h
BUILT_SOURCES = $(srcdir)/pc/Makefile.tst

# sources for both gawk and dgawk
base_sources = \
	array.c \
	awk.h \
	awkgram.y \
	builtin.c \
	cint_array.c \
	cmd.h \
	command.y \
	custom.h \
	debug.c \
	eval.c \
	ext.c \
	field.c \
	floatcomp.c \
	floatmagic.h \
	gawkapi.c \
	gawkapi.h \
	gawkmisc.c \
	gettext.h \
	int_array.c \
	interpret.h \
	io.c \
	mbsupport.h \
	main.c \
	mpfr.c \
	msg.c \
	node.c \
	nonposix.h \
	profile.c \
	protos.h \
	re.c \
	replace.c \
	str_array.c \
	symbol.c \
	version.c

gawk_SOURCES = $(base_sources)

# Get extra libs as needed, Automake will supply LIBINTL and SOCKET_LIBS.
LDADD = support/libsupport.a \
	$(LIBSIGSEGV) $(LIBINTL) $(SOCKET_LIBS) $(LIBREADLINE) $(LIBMPFR)

# Directory for gawk's data files. Automake supplies datadir.
pkgdatadir = $(datadir)/awk

# stuff for compiling gawk/pgawk
DEFPATH='".$(PATH_SEPARATOR)$(pkgdatadir)"'

# shared library support:
SHLIBEXT = "\"$(GAWKLIBEXT)"\"
DEFLIBPATH="\"$(pkgextensiondir)\""

DEFS= -DDEFPATH=$(DEFPATH) -DDEFLIBPATH=$(DEFLIBPATH) -DSHLIBEXT=$(SHLIBEXT) -DHAVE_CONFIG_H -DGAWK -DLOCALEDIR='"$(datadir)/locale"' -I"$(srcdir)/support"

# Get rid of core files when cleaning
CLEANFILES = core core.*

# We want hard links for install-exec-hook, below
LN= ln

# For some make's, e.g. OpenBSD, that don't define this
RM = rm -f

# First, add a link from gawk to gawk-X.Y.Z.
#
# For GNU systems where gawk is awk, add a link to awk.
# (This is done universally, which may not always be right, but
# there's no easy way to distinguish GNU from non-GNU systems.)
#
# Use the transform, in case --program-prefix=XXX
install-exec-hook:
	(cd $(DESTDIR)$(bindir); \
	name=`echo gawk | sed '$(transform)'` ; \
	$(LN) $${name}$(EXEEXT) gawk-$(VERSION)$(EXEEXT) 2>/dev/null ; \
	if [ ! -f awk$(EXEEXT) ]; \
	then	$(LN_S) $${name}$(EXEEXT) awk$(EXEEXT); \
	fi; exit 0)

# Undo the above when uninstalling
uninstall-links:
	(cd $(DESTDIR)$(bindir); \
	name=`echo gawk | sed '$(transform)'` ; \
	if [ -f awk$(EXEEXT) ] && cmp awk$(EXEEXT) $${name}$(EXEEXT) > /dev/null; then rm -f awk$(EXEEXT); fi ; \
	rm -f gawk-$(VERSION)$(EXEEXT); exit 0)

uninstall-recursive: uninstall-links

# force there to be a gawk executable before running tests
check-local: gawk$(EXEEXT)

# A little extra clean up when making distributions.
dist-hook:
	cd "$(distdir)"/extension ; rm -f *.o *.so
	cd "$(srcdir)"/pc ; \
	chmod u+w config.h ; \
	sed -n -f configpk.sed < ../configure.ac > /tmp/tmp.sed ; \
	sed -f config.sed < ../configh.in > /tmp/config.tmp ; \
	sed -f /tmp/tmp.sed < /tmp/config.tmp > config.h ; \
	$(RM) /tmp/tmp.sed /tmp/config.tmp
	pwd
	chmod u+w "$(distdir)"/pc/config.h
	cp "$(srcdir)"/pc/config.h "$(distdir)"/pc/config.h

# Special rules for individual files

$(srcdir)/awkgram.c: awkgram.y
	$(YACC) -o $@ $(AM_YFLAGS) $(YFLAGS) $<
	sed 's/parse error/syntax error/g' < $@ > $@.tmp && mv $@.tmp $@

$(srcdir)/command.c: command.y
	$(YACC) -o $@ -p zz $<
	sed 's/parse error/syntax error/g' < $@ > $@.tmp && mv $@.tmp $@

$(srcdir)/pc/Makefile.tst: test/Makefile.in pc/Makefile.tst.prologue pc/GenMakefileTst.awk
	$(AM_V_GEN)$(MKDIR_P) "$(srcdir)"/pc && \
	cat "$(srcdir)"/pc/Makefile.tst.prologue > "$(srcdir)"/pc/Makefile.tst && \
	$(AWK) -f "$(srcdir)"/pc/GenMakefileTst.awk "$(srcdir)"/test/Makefile.in >> "$(srcdir)"/pc/Makefile.tst

# This is for my development & testing.
efence: gawk
	$(CC) $(AM_LDFLAGS) -o gawk $$(ls *.o | grep -v '_p.o$$') $(LDADD) $(LIBS) -lefence

diffout valgrind-scan zos-diffout:
	@cd test && $(MAKE) $(AM_MAKEFLAGS) $@

valgrind:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=full --log-file=log.%p"; \
	make valgrind-scan

valgrind-noleak:
	cd test; rm -f log.[0-9]*; \
	make check VALGRIND="valgrind --leak-check=no --log-file=log.%p"; \
	make valgrind-scan

spell:
	cd "$(srcdir)"/doc ; $(MAKE) spell

distclean-local:
	rm -fr .deps
