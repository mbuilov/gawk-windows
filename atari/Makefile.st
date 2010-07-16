# Makefile for GNU Awk - ST version.
#
# This makefile hand edited from Makefile automatically generated
# by configure - gcc 2.x.x compiler with TOS version of libraries
# and modified system().
# Check comments in this Makefile and adjust to your needs!!
#
# This Makefile assumes that you are using Bourne compatible shell
# (like bash). If this is not the case you will have to edit various
# targets or perform some actions by hand.
#
# Copyright (C) 1986, 1988-1995 the Free Software Foundation, Inc.
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA



srcdir = .

# native compiler with freshly compiled gawk.ttp to fix awklib
CC = gcc
GAWK=../gawk.ttp
# cross-compiler and gawk already installed on the system (any awk will do)
# CC = cgcc
# GAWK = gawk
# WIDTH and EXT have to be both defined or both undefined
# WIDTH = -mshort -DINT_IS_16BIT
# EXT = 16
OFLAGS = -O2 -Wall -fomit-frame-pointer $(WIDTH)
LDFLAGS = $(WIDTH)
YACC = bison -y

INSTALL = cp -p
INSTALL_PROGRAM = ${INSTALL}
#INSTALL_DATA = ${INSTALL} -m 644
INSTALL_DATA = ${INSTALL}

LIBS = -lpml$(EXT)

ALLOCA = 

#all these definitions likely require changes
exec_prefix = ${prefix}
prefix = /usr/local
binprefix =
manprefix =

bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib
manexta = l
mandir = $(prefix)/man$(manexta)
manext = .$(manexta)
infodir = $(prefix)/info
datadir = $(prefix)/lib/awk
libexecdir = $(prefix)/libexec/awk

#DEFPATH = ".:$(datadir)"
# datadir is passed to the next Makefile level and through sed
# you may need many more backslashes than that if you have to use
# them at all - sigh...
DEFPATH = ".,c:\\lib\\awk,c:\\gnu\\lib\\awk"

SHELL = /bin/sh
CFLAGS = $(OFLAGS) -DGAWK -I. -I$(srcdir) -DHAVE_CONFIG_H

MFLAGS = "CC=$(CC)" "CFLAGS=$(CFLAGS) $(LDFLAGS)" GAWK=$(GAWK) \
  PWCAT=pwcat.ttp GRCAT=grcat.ttp GCOM='{sub(/\":\"/, "\",\""); print}' \
  "INSTALL_PROGRAM=$(INSTALL)" "INSTALL_DATA=$(INSTALL)" \
  "bindir=$(bindir)" \
  "libdir=$(libdir)" \
  "mandir=$(mandir)" \
  "infodir=$(infodir)" \
  "datadir=$(datadir)" \
  "libexecdir=$(libexecdir)"

# object files
AWKOBJS = array.o builtin.o eval.o field.o gawkmisc.o io.o main.o \
	missing.o msg.o node.o re.o version.o

ALLOBJS = $(AWKOBJS) awktab.o

# GNUOBJS
#	GNU stuff that gawk uses as library routines.
GNUOBJS= getopt.o getopt1.o regex.o dfa.o $(ALLOCA)

# source and documentation files
SRC = array.c builtin.c eval.c field.c gawkmisc.c io.c main.c \
	missing.c msg.c node.c re.c version.c

ALLSRC= $(SRC) awktab.c

AWKSRC= awk.h awk.y $(ALLSRC) patchlevel.h protos.h

GNUSRC = alloca.c dfa.c dfa.h regex.c regex.h getopt.h getopt.c getopt1.c

DOCS= doc/gawk.1.in doc/gawk.texi.in doc/texinfo.tex

TEXFILES= doc/gawk.aux doc/gawk.cp doc/gawk.cps doc/gawk.fn doc/gawk.fns \
	  doc/gawk.ky doc/gawk.kys doc/gawk.pg doc/gawk.pgs doc/gawk.toc \
	  doc/gawk.tp doc/gawk.tps doc/gawk.vr doc/gawk.vrs

ALLDOC= doc/gawk.dvi $(TEXFILES) doc/gawk.info*

# Release of gawk.  There can be no leading or trailing white space here!
REL=3.0
PROG=gawk.ttp

# clear out suffixes list
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $<

# rules to build $(PROG)
all:	$(PROG) awklib/all

alldoc:	all doc/all

$(PROG):	$(ALLOBJS) $(GNUOBJS) $(REOBJS)
	$(CC) -o $@ $(LDFLAGS) $(ALLOBJS) $(GNUOBJS) $(REOBJS) $(LIBS)
#	toglclr -fload $@
#	xstrip -k $@


$(ALLOBJS):	awk.h dfa.h regex.h config.h custom.h
 
$(GNUOBJS):	config.h custom.h

gawkmisc.o:	$(srcdir)/atari/gawkmisc.atr
		$(CC) -c  -DDEFPATH='$(DEFPATH)' $(CFLAGS) $(srcdir)/$<

# this rule needed or not - depending on your library
missing.o io.o:	
	$(CC) -c $(CFLAGS) -DPIPES_SIMULATED $(srcdir)/$<

# cheat with defines to force an inclusion of a proper code
getopt.o:	getopt.h
	$(CC) $(CFLAGS) -D_LIBC -D__alloca=__builtin_alloca -c getopt.c
  
getopt.o:	getopt.h

getopt1.o:	getopt.h

main.o:	patchlevel.h

awktab.c:	awk.y
	$(YACC) -v $(srcdir)/awk.y && \
	if test -f y.tab.c ; then mv y.tab.c ytab.c ; else true ; fi && \
	sed '/^extern char .malloc(), .realloc();$$/d' ytab.c >awktab.c && \
	rm ytab.c

# VMS POSIX make won't apply the default .c.o rule to awktab.o for some reason
awktab.o:	awktab.c awk.h
	$(CC) -c $(CFLAGS) $(srcdir)/awktab.c

alloca.o:	alloca.c

install:	$(PROG)
	$(INSTALL) $(PROG) $(bindir) && chmod 755 $(bindir)/$(PROG)
	cd awklib && $(MAKE) $(MFLAGS) install
#	cd doc && $(MAKE) $(MFLAGS) install

installdoc:	info
	cd doc && $(MAKE) $(MFLAGS) install

#	
#installtotal:	installdirs install installdoc


installdirs: mkinstalldirs
	$(srcdir)/mkinstalldirs $(bindir) $(datadir) \
		$(libdir) $(infodir) $(mandir)

uninstall:
	rm -f $(bindir)/$(PROG)
	cd awklib && $(MAKE) $(MFLAGS) uninstall
#	cd doc && $(MAKE) $(MFLAGS) uninstall

clean:
	rm -rf $(PROG) *.o core y.output
	cd awklib && $(MAKE) $(MFLAGS) clean
# the following does not always make sense (when crosscompiling)
#	cd test && $(MAKE) $(MFLAGS) clean
#	cd doc && $(MAKE) $(MFLAGS) clean


distclean:	clean
	rm -f Makefile *.orig *.rej */*.orig */*.rej awk.output gmon.out \
		make.out config.h config.status config.cache config.log stamp-h stamp-h.in
	cd doc && $(MAKE) $(MFLAGS) distclean

mostlyclean:	clean

maintainer-clean:	distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."
	rm -f awktab.c
	cd doc && $(MAKE) $(MFLAGS) maintainer-clean
	cd test && $(MAKE) $(MFLAGS) maintainer-clean
#	cd awklib && $(MAKE) $(MFLAGS) maintainer-clean

clobber:	maintainer-clean

TAGS:
	etags $(AWKSRC)
	ctags $(AWKSRC)

dvi:	$(srcdir)/doc/gawk.texi.in
	cd doc && $(MAKE) $(MFLAGS) dvi

info:	$(srcdir)/doc/gawk.texi.in
	cd doc && $(MAKE) $(MFLAGS) info

doc/all:
	cd doc && $(MAKE) $(MFLAGS) all

awklib/all:
	cd awklib && $(MAKE) $(MFLAGS) all

# to run this target you have to adjust test/Makefile quite a bit
# in order to make it paltable to your shell
#
check:	$(PROG)
	cd test; $(MAKE) -k

test:	check

