# Makefile for GNU Awk.
#
# Copyright (C) 1988 Free Software Foundation
# Rewritten by Arnold Robbins, September 1988 
#
# GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY.  No author or distributor accepts responsibility to anyone for
# the consequences of using it or for whether it serves any particular
# purpose or works at all, unless he says so in writing. Refer to the GAWK
# General Public License for full details. 
#
# Everyone is granted permission to copy, modify and redistribute GAWK, but
# only under the conditions described in the GAWK General Public License.  A
# copy of this license is supposed to have been given to you along with GAWK
# so you can know your rights and responsibilities.  It should be in a file
# named COPYING.  Among other things, the copyright notice and this notice
# must be preserved on all copies. 
#
# In other words, go ahead and share GAWK, but don't try to stop anyone else
# from sharing it farther.  Help stamp out software hoarding! 
#

# CFLAGS: options to the C compiler
#
#	-O	optimize
#	-g	include dbx/sdb info
#	-gg	include gdb debugging info; only for GCC
#	-pg	include new (gmon) profiling info
#	-p	include old style profiling info (System V)
#
#	-DNOVPRINTF - system has no vprintf and associated routines
#	-DHASDOPRNT - system needs version of vprintf et al. defined in awk5.c
#		      and has a BSD compatable doprnt()
#	-DNOMEMCMP  - system lacks memcmp()
#	-DUSG       - system is generic-ish System V.
#
OPTIMIZE=-O
DEBUG=#-DDEBUG #-DFUNC_TRACE -DMEMDEBUG
DEBUGGER=#-g
PROFILE=#-pg
SYSV=
BSD=#-DHASDOPRNT
MEMCMP=#-DNOMEMCMP
VPRINTF=#-DNOVPRINTF
  
FLAGS= $(OPTIMIZE) $(SYSV) $(DEBUG) $(BSD) $(MEMCMP) $(VPRINTF)
CFLAGS= $(FLAGS) $(DEBUGGER) $(PROFILE) 
LDFLAGS= #-Bstatic

SRC =	awk1.c awk2.c awk3.c awk4.c awk5.c \
	awk6.c awk7.c awk8.c awk9.c regex.c version.c do_free.c awka.c

AWKOBJS = awk1.o awk2.o awk3.o awk4.o awk5.o awk6.o awk7.o awk8.o awk9.o \
		version.o do_free.o awka.o 
ALLOBJS = $(AWKOBJS) awk.tab.o

# Parser to use on grammar -- if you don't have bison use the first one
#PARSER = yacc
PARSER = bison

# S5OBJS
#	Set equal to alloca.o if your system is S5 and you don't have
#	alloca. Uncomment the rule below to actually make alloca.o.
S5OBJS=

# GETOPT
#	Set equal to getopt.o if you have a generic BSD system. The
#	generic BSD getopt is reported to not work with gawk. The
#	gnu getopt is supplied in getopt.c
GETOPT=

# LIBOBJS
#	Stuff that awk uses as library routines, but not in /lib/libc.a.
LIBOBJS= regex.o $(S5OBJS) $(GETOPT)

# DOCS
#	Documentation for users
#
# Someday:
#DOCS=gawk.1 gawk.texinfo
DOCS= gawk.1

# We don't distribute shar files, but they're useful for mailing.
UPDATES = Makefile awk.h awk.y \
	$(SRC) regex.h

SHARS = $(DOCS) COPYING README PROBLEMS $(UPDATES) awk.tab.c \
	alloca.s alloca.c getopt.c

gawk: $(ALLOBJS) $(LIBOBJS)
	$(CC) -o gawk $(CFLAGS) $(ALLOBJS) $(LIBOBJS) -lm $(LDFLAGS)

$(AWKOBJS): awk.h

awk.tab.o: awk.h awk.tab.c

awk.tab.c: awk.y
	$(PARSER) -v awk.y
	-mv -f y.tab.c awk.tab.c

# Alloca: uncomment this if your system (notably System V boxen)
# does not have alloca in /lib/libc.a
#
#alloca.o: alloca.s
#	/lib/cpp < alloca.s | sed '/^#/d' > t.s
#	as t.s -o alloca.o
#	rm t.s

# If your machine is not supported by the assembly version of alloca.s,
# use the C version instead.  This uses the default rules to make alloca.o.
#
#alloca.o: alloca.c

lint: $(SRC)
	lint -h $(FLAGS) $(SRC) awk.tab.c

clean:
	rm -f gawk *.o core awk.output awk.tab.c gmon.out make.out

awk.shar: $(SHARS)
	shar -f awk -c $(SHARS)
 
awk.tar: $(SHARS)
	tar cvf awk.tar $(SHARS)

updates.tar:	$(UPDATES)
	tar cvf gawk.tar $(UPDATES)
 
awk.tar.Z:	awk.tar
	compress < awk.tar > awk.tar.Z

doc: $(DOCS)
	nroff -man $(DOCS) | col > $(DOCS).out

# This command probably won't be useful to the rest of the world, but makes
# life much easier for me.
dist:	awk.tar awk.tar.Z

diff:
	for i in RCS/*; do rcsdiff -c -b $$i > `basename $$i ,v`.diff; done

update:	$(UPDATES)
	sendup $?
	touch update
