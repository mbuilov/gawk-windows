# Makefile for GNU Awk - ST version.
#
# This makefile hand edited from Makefile automatically generated
# by configure - gcc 2.x.x compiler with TOS version of libraries
# and modified system().
# Check comments in this Makefile and adjust to your needs!!
#
# Copyright (C) 1986, 1988-1992 the Free Software Foundation, Inc.
# 
# This file is part of GAWK, the GNU implementation of the
# AWK Progamming Language.
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
# along with GAWK; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

srcdir = .
VPATH = .

# CC = gcc
CC = cgcc  # when cross-compiling
#YACC = byacc
YACC = bison -y
# basename of parser output - adjust to your parser requirements
YOUTPUT = awk_tab


# WIDTH and EXT have to be both defined or both undefined
# WIDTH = -mshort
# EXT = 16
LIBS = -lpml$(EXT)
LIBOBJS = strncasecmp.o
ST_LIBOBJS = stack.o tmpnam.o system.o 

# CFLAGS= -g -DGAWK -DHAVE_CONFIG_H $(WIDTH) -I. -I..
CFLAGS= -DGAWK -DHAVE_CONFIG_H $(WIDTH) -O2 -I. -I.. -Wall \
  -fomit-frame-pointer # -pipe #
# keep only global symbols and use long symbol names
# xstrip -k in target gawk.ttp removes all symbols but _stksize
# allowing for stack size manipulations without recompiling (with fixstk)
LDFLAGS= -Xlinker -x -G $(WIDTH)

# object files
AWKOBJS = main.o eval.o builtin.o msg.o iop.o io.o field.o array.o \
	node.o version.o re.o getopt.o getopt1.o

ALLOBJS = $(AWKOBJS) awktab.o

# GNUOBJS
#	GNU stuff that gawk uses as library routines.
GNUOBJS= rx.o dfa.o

all:	gawk.ttp

gawk.ttp: $(ALLOBJS) $(GNUOBJS) $(REOBJS) $(LIBOBJS) $(ST_LIBOBJS)
	$(CC) -o $@ $(LDFLAGS) \
	$(ALLOBJS) $(GNUOBJS) $(REOBJS) $(LIBOBJS) $(ST_LIBOBJS) $(LIBS)
	toglclr -fload $@
#	xstrip -k $@

$(AWKOBJS) $(GNUOBJS):	awk.h dfa.h regex.h config.h

# cheat with defines to force an inclusion of a proper code
getopt.o:	getopt.h
	$(CC) $(CFLAGS) -D_LIBC -D__alloca=__builtin_alloca -c getopt.c

getopt1.o:	getopt.h

main.o: patchlevel.h

awktab.c: awk.y
	$(YACC) -v awk.y
	sed '/^extern char .malloc(), .realloc();$$/d' $(YOUTPUT).c >awktab.c
	rm $(YOUTPUT).c

# rules for $(LIBOBJS) and $(ST_LIBOBJS)

strncasecmp.o: missing/strncasecmp.c config.h
	$(CC) $(CFLAGS) -c missing/strncasecmp.c

stack.o: atari/stack.c
	$(CC) $(CFLAGS) -c atari/stack.c
	
tmpnam.o: atari/tmpnam.c
	$(CC) $(CFLAGS) -c atari/tmpnam.c

# this is an optional replacement for a library module.
system.o: atari/system.c
	$(CC) $(CFLAGS) -c atari/system.c

clean:
	rm -rf gawk.ttp *.o core

distclean:	clean
	rm -f Makefile *.orig *.rej */*.orig */*.rej awk.output gmon.out \
	      make.out y.output config.h config.status

mostlyclean:	clean

realclean:	distclean
	rm -f awktab.c $(ALLDOC)

gawk.dvi: gawk.texi
	tex gawk.texi; texindex gawk.??
	tex gawk.texi; texindex gawk.??
	tex gawk.texi
	rm -f gawk.?? gawk.???

gawk.info:	gawk.texi
	makeinfo gawk.texi

# not really (or not with every shell) - but you have an idea
test:	gawk
	cd test; $(MAKE) -k

check:	test
