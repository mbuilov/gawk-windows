# Makefile for gawk (GNU awk)                                      Dec 2010
#
# - for GNU C (djgpp)         [32bit protected-mode executable for DOS]
# - for GNU C (emx)           [32bit executable for OS/2 or DOS or Windows32]
# - for GNU C (mingw32)	      [Windows32 executable for Windows 9x/NT/2K/XP/7]

# Tested with GNU make on Windows, OS/2 and DOS.

default:
	@echo "Enter $(MAK) target "
	@echo " where 'target' is chosen from                          "
	@echo "  djgpp ... DOS 32-bit exe [GNU C, Delorie, v2]         "
	@echo "  emx ..... OS/2 32-bit exe [emx/gcc; uses emxlibc.dll] "
	@echo "  emxnt ... NT exe [emx/gcc with RSXNT]                 "
	@echo "  emxbnd .. OS/2 and DOS 32-bit exe [emx/gcc]           "
	@echo "  mingw32 . Windows32 exe [Mingw32 GNU C]                   "
	@echo "  ----------------------------------------------------- "
	@echo "  test .... Perform tests (see README_d/README.pc)      "
	@echo "  install . Install gawk under $(prefix)/               "

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
#	@echo "  vcWin32 . Windows32 exe [Microsoft Visual C]              "
# Support dropped in 3.0
# - for Microsoft C 5.1       [16bit executable for OS/2 or DOS]
#	@echo "  msc51     DOS exe [Microsoft C 5.1]                   "
#	@echo "  msc51bnd  OS/2 and DOS exe [Microsoft C 5.1]          "

#======================= Configuration ==================================
RSPFILE = gawk.rsp
PRSPFILE = pgawk.rsp
DRSPFILE = dgawk.rsp
#
# Choose method for passing arguments to the linker.
#
# If compiling under OS/2 or if make can pass long lines
#LDRSP = $(GAWKOBJS)
#LNKRSP = $(LDRSP)
#
# else use brain-dead approach (emxbnd will need 'tr').
RSP = $(RSPFILE)
PRSP = $(PRSPFILE)
DRSP = $(DRSPFILE)
LDRSP = @$(RSP)
PLDRSP = @$(PRSP)
DLDRSP = @$(DRSP)
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
#
# Define the install method. Method 1 is Unix-like (and requires cat,
# cp, mkdir, sed, and sh); method 2 uses gawk and batch files.
install = 1
#------------------------------------------------------------------------
# To work around command-line length problems, this makefile assumes
# that $($X) can be expanded.
DO_LNK = $($(LNK))
DO_BIND= $($(BIND))
DO_PLNK = $($(PLNK))
DO_PBIND= $($(PBIND))
DO_DLNK = $($(DLNK))
DO_DBIND= $($(DBIND))
#========================================================================
# End of general configuration. Some platform-specific configuration
# notes appear below.

#========================================================================
#========================== DJGPP =======================================
#========================================================================

ifneq ($(DJGPP),)
prefix = $(DJDIR)
pkgdatadir = $(prefix)/share/awk
endif
LDJG = $(CC) $(LF) -o gawk.exe $(LDRSP) $(LF2)
PLDJG = $(CC) $(LF) -o pgawk.exe $(PLDRSP) $(LF2)
DLDJG = $(CC) $(LF) -o dgawk.exe $(DLDRSP) $(LF2)
BDJG = stubify -g awk.exe | stubedit awk.exe runfile=gawk

djgpp:
	$(MAK) all \
	CC=gcc O=.o CF=-O2 \
	LNK=LDJG PLNK=PLDJG DLNK=DLDJG LF=-s LF2=-lm \
	BIND=BDJG PBIND='' DBIND=''

djgpp-debug:
	$(MAK) all \
	CC=gcc O=.o CF='-O2 -g' \
	LNK=LDJG PLNK=PLDJG DLNK=DLDJG LF2=-lm \
	BIND=BDJG PBIND='' DBIND=''

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
PLMINGW32 = $(CC) $(LF) -o $@ $(PGAWKOBJS) $(LF2)
DLMINGW32 = $(CC) $(LF) -o $@ $(DGAWKOBJS) $(LF2)
# The following might work around command-line length limitations:
#LMINGW32 = $(CC) $(LF) -o $@ *.o $(LF2)

mingw32:
	$(MAK) all \
	CC=gcc O=.o CF="-O2 -gdwarf-2 -g3" OBJ=popen.o \
	LNK=LMINGW32 PLNK=PLMINGW32 DLNK=DLMINGW32 \
	LF="-gdwarf-2 -g3" LF2=-lmsvcp60 RSP=

# Define BIND for BINDless compiles, otherwise $($(BIND)) may break.
BIND = EMPTY
PBIND = EMPTY
EMPTY=

# bitwise operations (-DBITOPS) and non-decimal input data (-DNONDECDATA) are
# undocumented in 3.0.3.  They may be enabled in config.h, or added to CFLAGS.
CFLAGS = $(CF) -DGAWK -I. -DHAVE_CONFIG_H $(DYN_FLAGS)

# object files
AWKOBJS1  = array$O builtin$O eval$O field$O floatcomp$O gawkmisc$O io$O main$O
AWKOBJS2  = ext$O msg$O node$O profile$O re$O replace$O version$O $(DYN_OBJ)
PAWKOBJS1 = array$O builtin$O eval_p$O field$O floatcomp$O gawkmisc$O io$O main$O
PAWKOBJS2 = ext$O msg$O node$O profile_p$O re$O replace$O version$O $(DYN_OBJ)
DAWKOBJS1  = array$O builtin$O debug$O eval_d$O field$O floatcomp$O gawkmisc$O io$O main$O
DAWKOBJS2  = ext$O msg$O node$O profile$O re$O replace$O version$O command$O $(DYN_OBJ)
AWKOBJS = $(AWKOBJS1) $(AWKOBJS2)

ALLOBJS = $(AWKOBJS) awkgram$O getid$O $(OBJ)

# LIBOBJS
#       GNU and other stuff that gawk uses as library routines.
LIBOBJS= getopt$O getopt1$O dfa$O regex$O random$O

GAWKOBJS = $(ALLOBJS) $(LIBOBJS)
PGAWKOBJS = $(PAWKOBJS1) $(PAWKOBJS2) $(LIBOBJS) awkgram$O getid$O $(OBJ)
DGAWKOBJS = $(DAWKOBJS1) $(DAWKOBJS2) $(LIBOBJS) awkgram$O getid$O $(OBJ)

# clear out suffixes list
# .SUFFIXES:
.SUFFIXES: .c $O

.c$O:
	$(CC) -c $(CFLAGS) $<

# rules to build gawk
all : gawk.exe pgawk.exe dgawk.exe

gawk.exe:: $(GAWKOBJS) $(RSP)
	$(DO_LNK)
	$(DO_BIND)

pgawk.exe:: $(PGAWKOBJS) $(PRSP)
	$(DO_PLNK)
	$(DO_PBIND)

dgawk.exe:: $(DGAWKOBJS) $(DRSP)
	$(DO_DLNK)
	$(DO_DBIND)

$(RSPFILE) : $(GAWKOBJS)
	echo $(AWKOBJS1)$P > $@
	echo $(AWKOBJS2)$P >> $@
	echo awkgram$O getid$O $(OBJ) $(LIBOBJS)$P >> $@

$(PRSPFILE) : $(PGAWKOBJS)
	echo $(PAWKOBJS1)$P > $@
	echo $(PAWKOBJS2)$P >> $@
	echo awkgram$O getid$O $(OBJ) $(LIBOBJS)$P >> $@

$(DRSPFILE) : $(DGAWKOBJS)
	echo $(DAWKOBJS1)$P > $@
	echo $(DAWKOBJS2)$P >> $@
	echo awkgram$O getid$O $(OBJ) $(LIBOBJS)$P >> $@

# Notes to dependencies:
# 1. The dependency on getopt.h is because unistd.h includes it,
#    and we have -I. on the compiler command line.  unistd.h is
#    included by awk.h.
# 2. custom.h is not mentioned because pc ports don't use it.
$(ALLOBJS) $(LIBOBJS) eval_p$O profile_p$O: \
  awk.h regex.h config.h gettext.h mbsupport.h protos.h dfa.h getopt.h

builtin$O random$O:	floatmagic.h random.h

debug$O:	floatmagic.h

command$O debug$O:	cmd.h

dfa$O:	xalloc.h

gawkmisc$O:	pc/gawkmisc.pc

getopt$O getopt1$O :	getopt_int.h

io$O:		popen.h

regex$O:	regcomp.c regexec.c regex_internal.h

eval_p$O:	eval.c

profile_p$O:	profile.c

# A bug in ndmake requires the following rule
awkgram$O: awk.h awkgram.c
	$(CC) -c $(CFLAGS) awkgram.c

awkgram.c:	awkgram.y
	bison -o $@ awkgram.y

alloca$O:	alloca.c


install: install$(install)

install1:
	echo extproc sh $(prefix)/bin/igawk.cmd > igawk.cmd
	echo shift >> igawk.cmd
	cat pc/awklib/igawk >> igawk.cmd
	sed "s;igawk;$(prefix)/bin/igawk;" pc/awklib/igawk.bat > igawk.bat
	sh mkinstal.sh $(prefix)/bin
	sh mkinstal.sh $(pkgdatadir) $(prefix)/man/man1 $(prefix)/info
	cp *awk.exe igawk.bat igawk.cmd pc/awklib/igawk $(prefix)/bin
	cp awklib/eg/lib/* pc/awklib/igawk.awk $(pkgdatadir)
	cp doc/*.1 $(prefix)/man/man1
	cp doc/gawk.info $(prefix)/info

# install2 is equivalent to install1, but doesn't require cp, sed, etc.
install2:
	gawk -v prefix=$(prefix) -f install.awk

clean:
	rm -rf gawk pgawk dgawk *.exe gawk.map *.o *.obj core a.out $(RSPFILE) $(PRSPFILE) $(DRSPFILE) $(DYN_EXP)
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
