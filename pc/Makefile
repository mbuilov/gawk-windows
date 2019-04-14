# Makefile for gawk (GNU awk)                                      Dec 2010
#
# - for GNU C (djgpp)         [32bit protected-mode executable for DOS]
# - for GNU C (emx)           [32bit executable for OS/2 or DOS or Windows32]
# - for GNU C (mingw32)	      [Windows32 executable for Windows 9x/NT/2K/XP/7]

# Tested with GNU make on Windows, OS/2 and DOS.

# Copyright (C) 1989-2016 Free Software Foundation, Inc.

# This Makefile is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

default:
	@echo "Enter $(MAK) target "
	@echo " where 'target' is chosen from                          "
	@echo "  djgpp ... DOS 32-bit exe [GNU C, Delorie, v2]         "
	@echo "  djgpp-mpfr . Like djgpp, but with MPFR                "
	@echo "   [You will need to have GNU MPFR library installed.]  "
	@echo "  djgpp-readline . Like djgpp, but with readline        "
	@echo "   [You will need to have GNU readline library installed.] "
	@echo "  djgpp-readline-mpfr . djgpp with readline and MPFR    "
	@echo "  emx ..... OS/2 32-bit exe [emx/gcc; uses emxlibc.dll] "
	@echo "  emxnt ... NT exe [emx/gcc with RSXNT]                 "
	@echo "  emxbnd .. OS/2 and DOS 32-bit exe [emx/gcc]           "
	@echo "  mingw32 . Windows32 exe [Mingw32 GNU C]               "
	@echo "  mingw32-readline . Like mingw32, but with readline    "
	@echo "   [You will need to have GNU readline library installed.] "
	@echo "  mingw32-mpfr . Like mingw32, but with MPFR            "
	@echo "   [You will need to have GNU MPFR library installed.]  "
	@echo "  mingw32-readline-mpfr . mingw32 with readline and MPFR "
	@echo "  ----------------------------------------------------- "
	@echo "  test .... Perform tests (see README_d/README.pc)      "
	@echo "  install . Install gawk under $(prefix)/               "
	@echo "  doc ..... Create documentation                        "

# Support dropped in 4.0
# - for DJGPP v1.x            [DOS 32bit protected-mode executable]
# - for MS-Visual C/C++ 4.x   [Windows32 executable for Windows 9x/NT]
# - for Microsoft C 7         [16bit ececutable for DOS]
# - for Microsoft C 6.00A     [16bit executable for OS/2 or DOS]
#	@echo "  djgppv1 . DOS 32-bit exe [GNU C, Delorie, v1]         "
#	@echo "  msc ..... DOS exe [Microsoft C 7 & 8 (AKA 1.52)]      "
#	@echo "  msc6 .... DOS exe [Microsoft C 6.00a]                 "
#	@echo "  msc6os2 . OS/2 exe [Microsoft C 6.00a]                "
#	@echo "  msc6bnd . OS/2 and DOS exe [Microsoft C 6.00a]        "
#	@echo "  vcWin32 . Windows32 exe [Microsoft Visual C]          "
# Support dropped in 3.0
# - for Microsoft C 5.1       [16bit executable for OS/2 or DOS]
#	@echo "  msc51     DOS exe [Microsoft C 5.1]                   "
#	@echo "  msc51bnd  OS/2 and DOS exe [Microsoft C 5.1]          "

#======================= Configuration ==================================
RSPFILE = gawk.rsp
#
# Choose method for passing arguments to the linker.
#
# If compiling under OS/2 or if make can pass long lines
#LDRSP = $(GAWKOBJS)
#LNKRSP = $(LDRSP)
#
# else use brain-dead approach (emxbnd will need 'tr').
RSP = $(RSPFILE)
LDRSP = @$(RSP)
LNKRSP = $(LDRSP)
#------------------------------------------------------------------------
# Some makes do not define MAKE (and ndmake does not allow a define).
# Define MAK to be your make command.
#MAKE = dmake
MAK = $(MAKE) $(MAKEFILE)
#MAK = $(MAKE)
#MAKEFILE = -f Makefile
#MAK = make45 $(MAKEFILE)
#------------------------------------------------------------------------
# Define the base directory for the install. "make install" will install
# in bin, lib/awk, man, and info under $(prefix)/. Most likely, you should
# edit config.h so that $(prefix)/lib/awk appears as part of DEFPATH.
#prefix =
prefix = c:/gnu
pkgdatadir = $(prefix)/lib/awk
pkgextensiondir = $(prefix)/lib/gawk
DEFLIBPATH = "\"$(pkgextensiondir)\""
LOCALEDIR="\"$(prefix)/share/locale\""
SHLIBEXT = "\"dll\""
infodir = $(prefix)/share/info
mandir = $(prefix)/share/man
#
# Define the install method. Method 1 is Unix-like (and requires cat
# and cp); method 2 uses gawk and batch files.
install = 1
#------------------------------------------------------------------------
# To work around command-line length problems, this makefile assumes
# that $($X) can be expanded.
DO_LNK = $($(LNK))
DO_BIND= $($(BIND))
#------------------------------------------------------------------------
# To allow Make find files in the support subdirectory.
VPATH = .;./support
#========================================================================
# End of general configuration. Some platform-specific configuration
# notes appear below.

#========================================================================
#========================== DJGPP =======================================
#========================================================================

ifneq ($(DJGPP),)
prefix = /dev/env/DJDIR
pkgdatadir = $(prefix)/share/awk
SHLIBEXT = "\"dxe\""
endif
LDJG = $(CC) $(LF) -o gawk.exe $(LDRSP) $(LF2)
BDJG = stubify -g awk.exe | stubedit awk.exe runfile=gawk

djgpp:
	$(MAK) all \
	CC=gcc O=.o CF=-O2 \
	LNK=LDJG LF=-s LF2=-lm \
	BIND=BDJG

djgpp-debug:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -g' \
	LNK=LDJG LF2=-lm \
	BIND=BDJG

djgpp-mpfr:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -DHAVE_MPFR' \
	LNK=LDJG LF=-s LF2="-lmpfr -lgmp -lm" \
	BIND=BDJG

djgpp-readline:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -DHAVE_LIBREADLINE' \
	LNK=LDJG LF=-s LF2="-lreadline -lm" \
	BIND=BDJG

djgpp-readline-mpfr:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -DHAVE_LIBREADLINE -DHAVE_MPFR' \
	LNK=LDJG LF=-s LF2="-lreadline -lmpfr -lgmp -lm" \
	BIND=BDJG

#========================================================================
#========================== EMX =========================================
#========================================================================

# Link command for OS/2 versions.
LEMX = $(CC) $(LF) -o $@ $(GAWKOBJS) gawk.def -lbsd $(LF2)

# Link and bind for DOS and OS/2 versions.
# emx-09 needs '-p' emx option here or in EMXOPT environ var.
# The following works with 0.9a or newer
LEMXBND = $(CC) $(LF) -o gawk $(LDRSP) gawk.def -lbsd $(LF2)
BEMX = emxbind -bs gawk  -p
# The following works with 0.9c or newer
#LEMXBND = $(CC) $(LF) -o a.out $(LDRSP) gawk.def -lbsd $(LF2)
#BEMX = emxbind -bs -o $@ a.out -p
#BEMX = emxbind -bs /emx/bin/emx.exe a.out $@ -p
BEMXD = emxbind -b -o $@ a.out -p

emx:
	$(MAK) all \
	"CC=gcc -Zomf" O=.obj "CF=-O -DOS2" \
	LNK=LEMX "LF=-s -Zcrtdll -Zstack 512" RSP=

emxnt:
	$(MAK) all \
	"CC=gcc -Zwin32 -Zcrtdll=rsxntcs" O=.o "CF=-O -DOS2" \
	LNK=LEMX "LF=-s -Zstack 512" RSP=

emxbnd:
	$(MAK) all \
	CC=gcc O=.o "CF=-O -DOS2 -DMSDOS" OBJ=popen.o \
	LNK=LEMXBND \
	BIND=BEMX "P=|tr \" \" \"\n\""

emxbnd-debug:
	$(MAK) all \
	CC=gcc O=.o "CF=-g -DOS2 -DMSDOS" OBJ=popen.o \
	LNK=LEMXBND \
	BIND=BEMXD "P=|tr \" \" \"\n\""

#========================================================================
#========================== MINGW32 =====================================
#========================================================================

LMINGW32 = $(CC) $(LF) -o $@ $(GAWKOBJS) $(LF2)
# The following might work around command-line length limitations:
#LMINGW32 = $(CC) $(LF) -o $@ *.o $(LF2)

mingw32:
	$(MAK) all \
	CC=gcc O=.o CF="-D__USE_MINGW_ANSI_STDIO -O2 -gdwarf-2 -g3 -Wno-deprecated-declarations" \
	OBJ=popen.o LNK=LMINGW32 LF="-gdwarf-2 -g3" \
        LF2="-lws2_32 -lmsvcp60" RSP=

mingw32-readline:
	$(MAK) all \
	CC=gcc O=.o \
	CF="-D__USE_MINGW_ANSI_STDIO -DHAVE_LIBREADLINE -O2 -gdwarf-2 -g3 -Wno-deprecated-declarations" \
	OBJ=popen.o LNK=LMINGW32 LF="-gdwarf-2 -g3" \
	LF2="-lreadline -lws2_32 -lmsvcp60 -Wl,--enable-auto-import" RSP=

mingw32-mpfr:
	$(MAK) all \
	CC=gcc O=.o \
	CF="-D__USE_MINGW_ANSI_STDIO -DHAVE_MPFR -O2 -gdwarf-2 -g3 -Wno-deprecated-declarations" \
	OBJ=popen.o LNK=LMINGW32 LF="-gdwarf-2 -g3" \
	LF2="-lmpfr -lgmp -lws2_32 -lmsvcp60 -Wl,--enable-auto-import" RSP=

mingw32-readline-mpfr:
	$(MAK) all \
	CC=gcc O=.o \
	CF="-D__USE_MINGW_ANSI_STDIO -DHAVE_LIBREADLINE -DHAVE_MPFR -O2 -gdwarf-2 -g3 -Wno-deprecated-declarations" \
	OBJ=popen.o LNK=LMINGW32 LF="-gdwarf-2 -g3" \
	LF2="-lmpfr -lgmp -lreadline -lws2_32 -lmsvcp60 -Wl,--enable-auto-import" RSP=

# Define BIND for BINDless compiles, otherwise $($(BIND)) may break.
BIND = EMPTY
PBIND = EMPTY
EMPTY=

CFLAGS = $(CF) -DGAWK -I. -I./support -DHAVE_CONFIG_H -DDEFLIBPATH=$(DEFLIBPATH) -DSHLIBEXT=$(SHLIBEXT) -DLOCALEDIR=$(LOCALEDIR)

# object files
AWKOBJS1  = array$O builtin$O eval$O field$O floatcomp$O gawkmisc$O io$O main$O
AWKOBJS2  = ext$O msg$O node$O profile$O re$O replace$O version$O symbol$O
AWKOBJS3  = debug$O cint_array$O int_array$O mpfr$O str_array$O command$O
AWKOBJS4  = gawkapi$O
AWKOBJS = $(AWKOBJS1) $(AWKOBJS2) $(AWKOBJS3) $(AWKOBJS4)

ALLOBJS = $(AWKOBJS) awkgram$O getid$O $(OBJ)

# LIBOBJS
#       GNU and other stuff that gawk uses as library routines.
LIBOBJS= getopt$O getopt1$O dfa$O regex$O random$O localeinfo$O

GAWKOBJS = $(ALLOBJS) $(LIBOBJS)

# clear out suffixes list
# .SUFFIXES:
.SUFFIXES: .c $O

.c$O:
	$(CC) -c $(CFLAGS) $<

# rules to build gawk
all : gawk.exe

gawk.exe:: $(GAWKOBJS) $(RSP)
	$(DO_LNK)
	$(DO_BIND)

$(RSPFILE) : $(GAWKOBJS)
	echo $(AWKOBJS1)$P > $@
	echo $(AWKOBJS2)$P >> $@
	echo $(AWKOBJS3)$P >> $@
	echo $(AWKOBJS4)$P >> $@
	echo awkgram$O getid$O $(OBJ) $(LIBOBJS)$P >> $@

# Notes to dependencies:
# 1. The dependency on getopt.h is because unistd.h includes it,
#    and we have -I. on the compiler command line.  unistd.h is
#    included by awk.h.
# 2. custom.h is not mentioned because pc ports don't use it.
$(ALLOBJS) $(LIBOBJS): \
  awk.h regex.h config.h gettext.h mbsupport.h protos.h dfa.h getopt.h nonposix.h

builtin$O:	floatmagic.h random.h popen.h

random$O:	random.h

node$O:		floatmagic.h

command$O debug$O:	cmd.h

dfa$O:		xalloc.h

gawkmisc$O:	pc/gawkmisc.pc socket.h

getopt$O getopt1$O :	getopt_int.h

io$O:		popen.h socket.h in.h

regex$O:	regcomp.c regexec.c regex_internal.c regex_internal.h

eval$O:		interpret.h

localeinfo$O:	localeinfo.h

# A bug in ndmake requires the following rule
awkgram$O: awk.h awkgram.c
	$(CC) -c $(CFLAGS) awkgram.c

awkgram.c:	awkgram.y
	bison -o awkgram.tmp awkgram.y
	sed "s/parse error/syntax error/g" < awkgram.tmp > $@

alloca$O:	alloca.c

popen$O:	popen.h

install: install$(install)
	-$(MAKE) -C extension install-extensions

# Note: The line that copies gawk.exe into awk.exe should come before
# the line after it, which copies *awk.exe, to support the DJGPP "hard
# link to executables" feature, whereby awk.exe is actually a small
# stub that invokes gawk.exe.
install1:
	echo extproc sh $(prefix)/bin/igawk.cmd > igawk.cmd
	echo shift >> igawk.cmd
	cat pc/awklib/igawk >> igawk.cmd
	cat pc/awklib/igawk.bat > igawk.bat
	-mkdir "$(prefix)"
	-mkdir "$(prefix)/include"
	-mkdir "$(prefix)/bin"
	-mkdir "$(prefix)/share"
	-mkdir "$(prefix)/share/man"
	-mkdir "$(prefix)/lib"
	-mkdir "$(pkgdatadir)" "$(prefix)/share/man/man1" "$(prefix)/share/info"
	-mkdir "$(prefix)/lib/gawk"
	cp gawk.exe $(prefix)/bin/awk.exe
	cp *awk.exe igawk.bat igawk.cmd pc/awklib/igawk $(prefix)/bin
	cp gawkapi.h $(prefix)/include
	cp awklib/eg/lib/* pc/awklib/igawk.awk $(pkgdatadir)
	cp doc/*.1 $(prefix)/share/man/man1
	cp doc/*.info $(prefix)/share/info
	cp doc/*.png $(prefix)/share/info
	cp doc/*.txt $(prefix)/share/info
	rm -f $(prefix)/share/info/awkforai.txt

# install2 is equivalent to install1, but doesn't require cp, sed, etc.
install2:
	gawk -v prefix=$(prefix) -f install.awk

install-strip: install$(install)
	strip "$(prefix)/bin"/*.exe
	-$(MAKE) -C extension $@

clean:
	-rm -rf gawk *.exe gawk.map *.o *.obj core a.out $(RSPFILE) $(PRSPFILE) $(DRSPFILE) $(DYN_EXP) awkgram.tmp
#	cd doc && $(MAKE) clean
#	cd test && $(MAKE) clean
#	cd awklib && $(MAKE) clean

awklib/eg: doc/gawk.texi
	rm -fr awklib/eg
	sh -c "cd awklib && ../gawk -f extract.awk ../doc/gawk.texi"

check:
	@echo "Running the tests requires several unix-like utilities.  The"
	@echo "recommendation is to copy pc/Makefile.tst to test/Makefile.  Under"
	@echo "DOS, it may be necessary to run make from the test directory."
# The `-k' option to make should be unnecessary if using pc/Makefile.tst.
	sh -c "cd test && $(MAK) -k AWK=../gawk.exe"
#	sh -c "cd test && $(MAK) AWK=../gawk.exe bigtest extra"

test:	check

# for those who have the necessary tools:
TAGS:
	etags awk.h *.y custom.h *.c *.h

tags:
	ctags awk.h *.y custom.h *.c *.h

#========================================================================
#================================= DOC ==================================
#========================================================================

DVIS = ./doc/gawk.dvi ./doc/gawkinet.dvi
PDFS = ./doc/gawk.pdf ./doc/gawkinet.pdf
PSS = ./doc/gawk.ps ./doc/gawkinet.ps
HTMLS = ./doc/gawk.html ./doc/gawkinet.html
INFOS = ./doc/gawk.info ./doc/gawkinet.info
TEXINFOS = ./doc/gawk.texi ./doc/gawkinet.texi

TEXI2DVI = texi2dvi --build-dir=./doc
TEXI2PDF = $(TEXI2DVI) --pdf --batch
DVIPS = dvips
MAKEINFO = makeinfo --no-split --force
MAKEINFOHTML = $(MAKEINFO) --html

TROFF = groff -t -Tps -U
#SEDME = sed -e "s/^level0 restore/level0 restore flashme 100 72 moveto (Copyright `date '+%m-%d-%y %T'`, FSF, Inc. (all)) show/" \
#		-e "s/^\/level0 save def/\/level0 save def 30 -48 translate/"

SEDME = sed "s/^\/level0 save def/\/level0 save def 30 -48 translate/"

SEDME2 = sed "/%%Page: 10 10/,/0 Cg EP/d"

.SUFFIXES: .dvi .html .info .pdf .ps .texi

.texi.info:
	$(MAKEINFO) -o $@ $<

.texi.html:
	$(MAKEINFOHTML) -o $@ $<

.texi.dvi:
	$(TEXI2DVI) -o $@ $<

.texi.pdf:
	$(TEXI2PDF) -o $@ $<

.dvi.ps:
	$(DVIPS) -o $@ $<

./doc/awkcard.tr: ./doc/awkcard.in
	cd doc
	sed "s,SRCDIR,.," < awkcard.in > awkcard.tr
	cd ..

./doc/awkcard.nc: export GROFF_TMPDIR ?= .
./doc/awkcard.nc: ./doc/macros ./doc/cardfonts ./doc/no.colors ./doc/awkcard.tr ./doc/ad.block ./doc/awkcard.in ./doc/setter.outline
	cd doc
	$(TROFF) ./macros ./cardfonts ./no.colors awkcard.tr | $(SEDME) | cat ./setter.outline - | $(SEDME2) > awkcard.ps
	cd ..

./doc/awkcard.ps: ./doc/awkcard.nc
	cd doc
	touch awkcard.nc
	cd ..

./doc/awkcard.pdf: ./doc/awkcard.ps
	cd doc
	ps2pdf ./awkcard.ps ./awkcard.pdf
	cd ..

doc: $(INFOS) $(DVIS) $(HTMLS) $(PSS) $(PDFS) ./doc/awkcard.ps ./doc/awkcard.pdf
