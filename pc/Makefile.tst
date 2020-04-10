# Makefile for GNU Awk test suite.
#
# Copyright (C) 1988-2018 the Free Software Foundation, Inc.
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

# ============================================================================
# MS-DOS & OS/2 Notes: READ THEM!
# ============================================================================

# As of version 2.91, efforts to make this makefile run in MS-DOS and OS/2
# have started in earnest.  The following steps need to be followed in order 
# to run this makefile:
#
# 1. The first thing that you will need to do is to convert all of the 
#    files ending in ".ok" in the test directory, all of the files ending 
#    in ".good" (or ".goo") in the test/reg directory, and mmap8k.in from
#    having a linefeed to having carriage return/linefeed at the end of each
#    line. There are various public domain UNIX to DOS converters and any 
#    should work.  Alternatively, you can use diff instead of cmp--most 
#    versions of diff don't care about how the lines end.
#
# 2. You will need an sh-compatible shell.  Please refer to the "README.pc"
#    file in the README_d directory for information about obtaining a copy.
#    You will also need various UNIX utilities.  At a minimum, you will 
#    need: rm, tr, cmp (or diff, see above), cat, wc, and sh.  
#    You should also have a UNIX-compatible date program.
#
# The makefile has only been tested with dmake 3.8 and DJGPP Make 3.74 or
# later.  After making all of these changes, typing "dmake check extra"
# or "make check extra" (with DJGPP Make) should run successfully.

# The Bash shell (compiled with djgpp) works very well with the
# djgpp-compiled gawk.  It is currently the recommended shell to use
# for testing, along with DJGPP make.  See README.pc for 
# more information on OS/2 and DOS shells.

# You will almost certainly need to change some of the values (MACROS) 
# defined on the next few lines.  

# This won't work unless you have "sh" and set SHELL equal to it (Make 3.74
# or later which comes with DJGPP will work with SHELL=/bin/sh if you have
# sh.exe anywhere on your PATH).
#SHELL = e:\bin\sh.exe
SHELL = /bin/sh

# Point to gawk
AWK = AWKLIBPATH=../extension $(AWKPROG)
AWKPROG = ../gawk.exe
# Point $(LS) to a version of ls.exe that reports true Windows file
# index numbers, because this is what the readdir test expects.
# Otherwise, the readdir test will fail.  (The MSYS ls.exe doesn't
# report full index values.)
LS = ls.exe

# Set your cmp command here (you can use most versions of diff instead of cmp
# if you don't want to convert the .ok files to the DOS CR/LF format).
# This is also an issue for the "mmap8k" test.  If it fails, make sure that
# mmap8k.in has CR/LFs or that you've used diff.
#
# The following comment is for users of OSs which support long file names
# (such as Windows 95) for all versions of gawk (both 16 & 32-bit).
# If you use a shell which doesn't support long filenames, temporary files
# created by this makefile will be truncated by your shell.  "_argarra" is an
# example of this.  If $(CMP) is a DJGPP-compiled program, then it will fail
# because it looks for the long filename (eg. _argarray).  To fix this, you
# need to set LFN=n in your shell's environment.
# NOTE: Setting LFN in the makefile most probably won't help you because LFN
# needs to be an environment variable.
#CMP = cmp
# See the comment above for why you might want to set CMP to "env LFN=n diff"
#CMP = env LFN=n diff
#CMP = diff
CMP = diff -u
#CMP = gcmp

# cmp replacement program for PC where the error messages aren't
# exactly the same.  Should run even on old awk.
TESTOUTCMP = $(AWK) -f ../testoutcmp.awk

# Set your "cp," "mv," and "mkdir" commands here.  Note: DOS's copy must take
# forward slashes.
CP = cp
#CP = : && command -c copy
#CP  = command.com /c copy

#MV = cmd.exe /c ren
MV = mv

MKDIR = mkdir
#MKDIR = gmkdir
#MKDIR = : && command -c mkdir
#MKDIR  = command.com /c mkdir

# Set your unix-style date function here
#DATE = date
DATE = gdate

# Set sort command
SORT = sort

# MS-DOS and OS/2 use ; as a PATH delimiter
PATH_SEPARATOR = ;

# Argument to -F to produce -F/, should be // for MinGW, / otherwise,
# because MSYS Bash converts a single slash to the equivalent Windows
# directory.
SLASH = /
#SLASH = //

# Non-default GREP_OPTIONS might fail the badargs test
export GREP_OPTIONS=

# ============================================================================
# You shouldn't need to modify anything below this line.
# ============================================================================

srcdir = .
abs_srcdir = .
abs_builddir = .
top_srcdir = "$(srcdir)"/..
abs_top_builddir = "$(top_srcdir)"

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = core core.* fmtspcl.ok

# try to keep these sorted. each letter starts a new line
BASIC_TESTS = \
	addcomma anchgsub anchor argarray arrayind1 arrayind2 arrayind3 arrayparm \
	arrayprm2 arrayprm3 arrayref arrymem1 arryref2 arryref3 arryref4 arryref5 \
	arynasty arynocls aryprm1 aryprm2 aryprm3 aryprm4 aryprm5 aryprm6 aryprm7 \
	aryprm8 aryprm9 arysubnm aryunasgn asgext awkpath \
	assignnumfield assignnumfield2 \
	back89 backgsub badassign1 badbuild \
	callparam childin clobber closebad clsflnam compare compare2 \
	concat1 concat2 concat3 concat4 concat5 convfmt \
	datanonl defref delargv delarpm2 delarprm delfunc dfamb1 dfastress dynlj \
	escapebrace eofsplit eofsrc1 exit2 exitval1 exitval2 exitval3 \
	fcall_exit fcall_exit2 fldchg fldchgnf fldterm fnamedat fnarray fnarray2 \
	fnaryscl fnasgnm fnmisc fordel forref forsimp fsbs fscaret fsnul1 \
	fsrs fsspcoln fstabplus funsemnl funsmnam funstack \
	getline getline2 getline3 getline4 getline5 getlnbuf getnr2tb getnr2tm \
	gsubasgn gsubtest gsubtst2 gsubtst3 gsubtst4 gsubtst5 gsubtst6 gsubtst7 \
	gsubtst8 \
	hex hex2 hsprint \
	inpref inputred intest intprec iobug1 \
	leaddig leadnl litoct longsub longwrds \
	manglprm math membug1 memleak messages minusstr mmap8k \
	nasty nasty2 negexp negrange nested nfldstr nfloop nfneg nfset nlfldsep \
	nlinstr nlstrina noeffect nofile nofmtch noloop1 noloop2 nonl noparms \
	nors nulinsrc nulrsend numindex numrange numstr1 numsubstr \
	octsub ofmt ofmta ofmtbig ofmtfidl ofmts ofmtstrnum ofs1 onlynl \
	opasnidx opasnslf \
	paramasfunc1 paramasfunc2 paramdup paramres paramtyp paramuninitglobal \
	parse1 parsefld parseme pcntplus posix2008sub prdupval prec printf0 \
	printf1 printfchar prmarscl prmreuse prt1eval prtoeval \
	rand randtest range1 range2 readbuf rebrackloc rebt8b1 rebuild redfilnm regeq \
	regexpbrack regexpbrack2 regexprange regrange reindops reparse resplit \
	rri1 rs rscompat rsnul1nl rsnulbig rsnulbig2 rsnulw \
	rstest1 rstest2 rstest3 rstest4 rstest5 rswhite \
	scalar sclforin sclifin setrec0 setrec1 \
	sigpipe1 sortempty sortglos spacere splitargv splitarr \
	splitdef splitvar splitwht status-close strcat1 strnum1 strnum2 strtod \
	subamp subback subi18n subsepnm subslash substr swaplns synerr1 synerr2 synerr3 \
	tailrecurse tradanch trailbs tweakfld \
	uninit2 uninit3 uninit4 uninit5 uninitialized unterm uparrfs uplus \
	wideidx wideidx2 widesub widesub2 widesub3 widesub4 wjposer1 \
	zero2 zeroe0 zeroflag

UNIX_TESTS = \
	fflush getlnhd localenl pid pipeio1 pipeio2 poundbang rtlen rtlen01 \
	space strftlng

GAWK_EXT_TESTS = \
	aadelete1 aadelete2 aarray1 aasort aasorti argtest arraysort arraysort2 \
	arraytype \
	backw badargs beginfile1 beginfile2 binmode1 \
	charasbytes colonwarn clos1way clos1way2 clos1way3 clos1way4 clos1way5 \
	clos1way6 crlf \
	dbugeval dbugeval2 dbugeval3 dbugtypedre1 dbugtypedre2 delsub \
	devfd devfd1 devfd2 dfacheck1 dumpvars \
	errno exit fieldwdth forcenum \
	fpat1 fpat2 fpat3 fpat4 fpat5 fpat6 fpat7 fpat8 fpatnull \
	fsfwfs funlen functab1 functab2 functab3 \
	fwtest fwtest2 fwtest3 fwtest4 fwtest5 fwtest6 fwtest7 fwtest8 \
	genpot gensub gensub2 gensub3 getlndir gnuops2 gnuops3 gnureops gsubind \
	icasefs icasers id igncdym igncfs ignrcas2 ignrcas4 ignrcase incdupe \
	incdupe2 incdupe3 incdupe4 incdupe5 incdupe6 incdupe7 include include2 \
	indirectbuiltin indirectcall indirectcall2 intarray isarrayunset \
	lint lintexp lintindex lintint lintlength lintold lintset lintwarn \
	manyfiles match1 match2 match3 mbstr1 mbstr2 mixed1 mktime muldimposix \
	nastyparm negtime next nondec nondec2 nonfatal1 nonfatal2 nonfatal3 \
	nsawk1a nsawk1b nsawk1c nsawk2a nsawk2b \
	nsbad nsbad_cmd nsforloop nsfuncrecurse nsindirect1 nsindirect2 nsprof1 nsprof2 \
	patsplit posix printfbad1 printfbad2 printfbad3 printfbad4 printhuge \
	procinfs profile0 profile1 profile2 profile3 profile4 profile5 profile6 \
	profile7 profile8 profile9 profile10 profile11 profile12 profile13 pty1 pty2 \
	rebuf regnul1 regnul2 regx8bit reginttrad reint reint2 rsgetline rsglstdin \
	rsstart1 rsstart2 rsstart3 rstest6 \
	sandbox1 shadow shadowbuiltin sortfor sortfor2 sortu \
	sourcesplit split_after_fpat \
	splitarg4 strftfld strftime strtonum strtonum1 \
	stupid1 stupid2 stupid3 stupid4 \
	switch2 symtab1 symtab2 symtab3 symtab4 symtab5 symtab6 symtab7 \
	symtab8 symtab9 symtab10 symtab11 \
	timeout typedregex1 typedregex2 typedregex3 typedregex4 \
	typedregex5 typedregex6 \
	typeof1 typeof2 typeof3 typeof4 typeof5 \
	watchpoint1

ARRAYDEBUG_TESTS = arrdbg
EXTRA_TESTS = inftest regtest ignrcas3 
INET_TESTS = inetdayu inetdayt inetechu inetecht
MACHINE_TESTS = double1 double2 intformat
LOCALE_CHARSET_TESTS = \
	asort asorti backbigs1 backsmalls1 backsmalls2 \
	fmttest fnarydel fnparydl jarebug lc_num1 mbfw1 \
	mbprintf1 mbprintf2 mbprintf3 mbprintf4 mbprintf5 \
	mtchi18n nlstringtest rebt8b2 rtlenmb sort1 sprintfc

SHLIB_TESTS = \
	apiterm \
	filefuncs fnmatch fork fork2 fts functab4 \
	getfile \
	inplace1 inplace2 inplace2bcomp inplace3 inplace3bcomp \
	ordchr ordchr2 \
	readdir readdir_test readdir_retest readfile readfile2 revout \
	revtwoway rwarray \
	testext time


# List of the tests which should be run with --debug option:
NEED_DEBUG = dbugtypedre1 dbugtypedre2 dbugeval2 dbugeval3 symtab10

# List of the tests which should be run with --lint option:
NEED_LINT = \
	defref fmtspcl lintexp lintindex lintint lintlength lintwarn \
	noeffect nofmtch nonl shadow uninit2 uninit3 uninit4 uninit5 uninitialized


# List of the tests which should be run with --lint-old option:
NEED_LINT_OLD = lintold

# List of tests that must be run with -M
NEED_MPFR = mpfrbigint mpfrbigint2 mpfrexprange mpfrfield mpfrieee mpfrmemok1 \
	mpfrnegzero mpfrnr mpfrrem mpfrrnd mpfrrndeval mpfrsort mpfrsqrt \
	mpfrstrtonum mpgforcenum mpfruplus mpfranswer42


# List of tests that need --non-decimal-data
NEED_NONDEC = mpfrbigint2 nondec2 intarray forcenum

# List of tests that need --posix
NEED_POSIX = escapebrace printf0 posix2008sub paramasfunc1 paramasfunc2 muldimposix

# List of tests that need --pretty-print
NEED_PRETTY = nsprof1 nsprof2 \
	profile4 profile5 profile8 profile9 profile10 profile11 profile13


# List of tests that need --re-interval
NEED_RE_INTERVAL = gsubtst3 reint reint2

# List of tests that need --sandbox
NEED_SANDBOX = sandbox1

# List of tests that need --traditional
NEED_TRADITIONAL = litoct tradanch rscompat

# Lists of tests that run a shell script
RUN_SHELL = exit fflush localenl next randtest rtlen rtlen01

# List of the tests which fail with EXIT CODE 1
FAIL_CODE1 = \
	badassign1 badbuild callparam delfunc fcall_exit fcall_exit2 \
	fnamedat fnarray fnarray2 fnasgnm fnmisc funsmnam gsubasgn \
	incdupe2 lintwarn match2 mixed1 noparms paramasfunc1 paramasfunc2 \
	paramdup paramres parseme readbuf synerr1 synerr2 unterm


# List of files which have .ok versions for MPFR
CHECK_MPFR = \
	rand fnarydel fnparydl


# Lists of tests that need particular locales
NEED_LOCALE_C = \
	clos1way gsubtst6 range2

NEED_LOCALE_EN = \
	backbigs1 backsmalls1 backsmalls2 concat4 dfamb1 ignrcas2 lc_num1 \
	mbfw1 mbprintf1 mbprintf3 mbprintf4 mbstr1 mbstr2 printhuge reint2 \
	rri1 subamp subi18n wideidx wideidx2 widesub widesub2 widesub3 widesub4


# Unused at the moment, since nlstringtest has additional stufff it does
# NEED_LOCALE_FR =
# Same for ignrcas3
# NEED_LOCALE_GR =
NEED_LOCALE_JP = mbprintf2
NEED_LOCALE_RU = mtchi18n

# List of tests that fail on DJGPP
EXPECTED_FAIL_DJGPP = \
	backbigs1 backsmalls1 backw beginfile1 clos1way clos1way2 \
	clos1way3 clos1way4 clos1way5 clos1way6 errno getlndir \
	getlnhd gnuops3 gnureops id ignrcas3 inetdayt inetdayu \
	inetecht inetechu inftest jarebug mbfw1 mbprintf1 mbprintf4 \
	mbstr1 mbstr2 mpfrsqrt nonfatal1 nonfatal2 nonfatal3 pid pty1 pty2 \
	randtest regx8bit strtod sigpipe1 symtab6 timeout


# List of tests that fail on MinGW
EXPECTED_FAIL_MINGW = \
	backbigs1 backsmalls1 clos1way6 devfd devfd1 devfd2 \
	errno exitval2 fork fork2 fts getfile getlnhd ignrcas3 inetdayt \
	inetecht mbfw1 mbprintf1 mbprintf4 mbstr1 mbstr2 pid pipeio2 \
	pty1 pty2 readdir rstest4 rstest5 status-close timeout


# List of tests that fail on z/OS
EXPECTED_FAIL_ZOS = \
	aasort aasorti arraysort asort asorti backbigs1 backsmalls1 \
	backsmalls2 beginfile1 beginfile2 charasbytes clos1way6 concat4 \
	dfamb1 double1 double2 errno fmttest forcenum getlndir gsubtst5 \
	ignrcas2 lc_num1 localenl localenl mbfw1 mbprintf1 mbprintf2 \
	mbprintf3 mbprintf4 mbprintf5 mbstr1 mbstr2 mtchi18n nlstringtest \
	nofile nonfatal2 numrange printhuge profile5 profile5 rebt8b2 \
	regrange reint2 rri1 sigpipe1 sort1 sortfor sortu space sprintfc \
	subamp subi18n symtab1 symtab11 symtab8 timeout wideidx wideidx2 \
	widesub widesub2 widesub3 widesub4


# List of the files that appear in manual tests or are for reserve testing:
GENTESTS_UNUSED = Makefile.in checknegtime.awk dtdgport.awk fix-fmtspcl.awk \
	fmtspcl-mpfr.ok fmtspcl.awk fmtspcl.tok gtlnbufv.awk hello.awk \
	inchello.awk inclib.awk inplace.1.in inplace.2.in inplace.in \
	printfloat.awk readdir0.awk valgrind.awk xref.awk


# List of tests on MinGW or DJGPP that need a different cmp program
NEED_TESTOUTCMP = \
	beginfile2 double2 exit fmttest hsprint posix profile5 space



# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if `make -j' in effect.
check:	msg \
	printlang \
	basic-msg-start  basic           basic-msg-end \
	unix-msg-start   unix-tests      unix-msg-end \
	extend-msg-start gawk-extensions arraydebug-tests extend-msg-end \
	machine-msg-start machine-tests machine-msg-end \
	charset-tests-all \
	shlib-msg-start  shlib-tests     shlib-msg-end \
	mpfr-msg-start   mpfr-tests      mpfr-msg-end
	@-$(MAKE) pass-fail || { $(MAKE) diffout; exit 1; }

basic:	$(BASIC_TESTS)

unix-tests: $(UNIX_TESTS)

gawk-extensions: $(GAWK_EXT_TESTS)

charset-tests-all:
	@-case `uname` in \
	*MINGW* | *MS-DOS*) \
		$(MAKE) charset-msg-start charset-tests charset-msg-end ;; \
	*) \
		if locale -a | grep -i 'en_US.UTF.*8' > /dev/null && \
		   locale -a | grep -i 'fr_FR.UTF.*8' > /dev/null && \
		   locale -a | grep -i 'ru_RU.UTF.*8' > /dev/null && \
		   locale -a | grep -i 'ja_JP.UTF.*8' > /dev/null  ; \
		then \
			$(MAKE) charset-msg-start charset-tests charset-msg-end; \
		else \
			echo %%%%%%%%%% Inadequate locale support: skipping charset tests. ; \
			echo %%%%%%%%%% At least ENU_USA.1252, FRA_FRA.1252, RUS_RUS.1251 and JPN_JPN.932 are needed. ; \
		fi ;; \
	esac

charset-tests: $(LOCALE_CHARSET_TESTS)

extra:	$(EXTRA_TESTS) inet

inet:	inetmesg $(INET_TESTS)

machine-tests: $(MACHINE_TESTS)

# The blank between ' and /MPFR/ is for running tests on Windows under
# MSYS, which thinks /MPFR is a Unix-style file name and converts it
# to Windows format, butchering it in the process.  Likewise for /API/
# in the next shlib-tests.
mpfr-tests:
	@-if $(AWK) --version | $(AWK) ' /MPFR/ { exit 1 }' ; then \
	echo MPFR tests not supported on this system ; \
	else $(MAKE) $(NEED_MPFR) ; \
	fi

arraydebug-tests:
	@-if echo $(CFLAGS) | grep ARRAYDEBUG > /dev/null ; then \
	$(MAKE) $(ARRAYDEBUG_TESTS) ; \
	else echo gawk is not compiled to support the array debug tests ; \
	fi

shlib-tests:
	@-if $(AWK) --version | $(AWK) ' /API/ { exit 1 }' ; then \
	echo shlib tests not supported on this system ; \
	else $(MAKE) shlib-real-tests ; \
	fi

shlib-real-tests: $(SHLIB_TESTS)

msg::
	@echo ''
	@echo 'Any output from "$(CMP)" is bad news, although some differences'
	@echo 'in floating point values are probably benign -- in particular,'
	@echo 'some systems may omit a leading zero and the floating point'
	@-echo 'precision may lead to slightly different output in a few cases.'

printlang::
	@-$(AWK) -f "$(srcdir)"/printlang.awk

basic-msg-start:
	@-echo "======== Starting basic tests ========"

basic-msg-end:
	@-echo "======== Done with basic tests ========"

unix-msg-start:
	@-echo "======== Starting Unix tests ========"

unix-msg-end:
	@-echo "======== Done with Unix tests ========"

extend-msg-start:
	@-echo "======== Starting gawk extension tests ========"

extend-msg-end:
	@-echo "======== Done with gawk extension tests ========"

machine-msg-start:
	@-echo "======== Starting machine-specific tests ========"

machine-msg-end:
	@-echo "======== Done with machine-specific tests ========"

charset-msg-start:
	@echo "======== Starting tests that can vary based on character set or locale support ========"
	@echo "**************************************************************************"
	@echo "* Some or all of these tests may fail if you have inadequate or missing  *"
	@echo "* locale support. At least ENU_USA.1252, FRA_FRA.1252, RUS_RUS.1251 and     *"
	@echo "* JPN_JPN.932 are needed. The ell_GRC.1253 is optional but helpful.    *"
	@echo "* However, if you see this message, the Makefile thinks you have what    *"
	@echo "* you need ...                                                           *"
	@-echo "**************************************************************************"

charset-msg-end:
	@-echo "======== Done with tests that can vary based on character set or locale support ========"

shlib-msg-start:
	@-echo "======== Starting shared library tests ========"

shlib-msg-end:
	@-echo "======== Done with shared library tests ========"

mpfr-msg-start:
	@-echo "======== Starting MPFR tests ========"

mpfr-msg-end:
	@-echo "======== Done with MPFR tests ========"

# This test is a PITA because increasingly, /tmp is getting
# mounted noexec.  So, we'll test it locally.  Sigh.
#
# More PITA; some systems have medium short limits on #! paths,
# so this can still fail
poundbang::
	@echo $@
	@sed "s;/tmp/gawk;`pwd`/$(AWKPROG);" < "$(srcdir)"/poundbang.awk > ./_pbd.awk
	@chmod +x ./_pbd.awk
	@if ./_pbd.awk "$(srcdir)"/poundbang.awk > _`basename $@` ; \
	then : ; \
	else \
		sed "s;/tmp/gawk;$(AWKPROG);" < "$(srcdir)"/poundbang.awk > ./_pbd.awk ; \
		chmod +x ./_pbd.awk ; \
		LC_ALL=$${GAWKLOCALE:-C} LANG=$${GAWKLOCALE:-C} ./_pbd.awk "$(srcdir)"/poundbang.awk > _`basename $@`;  \
	fi
	@-$(CMP) "$(srcdir)"/poundbang.awk _`basename $@` && rm -f _`basename $@` _pbd.awk

messages::
	@echo $@
	@$(AWK) -f "$(srcdir)"/messages.awk >_out2 2>_out3 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/out1.ok _out1 && $(CMP) "$(srcdir)"/out2.ok _out2 && $(CMP) "$(srcdir)"/out3.ok _out3 && rm -f _out1 _out2 _out3

argarray::
	@echo $@
	@cp "$(srcdir)"/argarray.in ./argarray.input
	@TEST=test echo just a test | $(AWK) -f "$(srcdir)"/argarray.awk ./argarray.input - >_$@ || echo EXIT CODE: $$? >> _$@
	@rm -f ./argarray.input
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regtest::
	@echo $@
	@echo 'Some of the output from regtest is very system specific, do not'
	@echo 'be distressed if your output differs from that distributed.'
	@echo 'Manual inspection is called for.'
	@-AWK=$(AWKPROG) "$(srcdir)"/regtest.sh

manyfiles::
	@echo $@
	@rm -rf junk
	@mkdir junk
	@$(AWK) 'BEGIN { for (i = 1; i <= 1030; i++) print i, i}' >_$@
	@$(AWK) -f "$(srcdir)"/manyfiles.awk _$@ _$@ || echo EXIT CODE: $$? >> _$@
	@wc -l junk/* | $(AWK) '$$1 != 2' | wc -l | sed "s/  *//g" > _$@
	@rm -rf junk
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

compare::
	@echo $@
	@$(AWK) -f "$(srcdir)"/compare.awk 0 1 "$(srcdir)"/compare.in >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

inftest::
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@echo This test is very machine specific...
	@$(AWK) -f "$(srcdir)"/inftest.awk | sed "s/inf/Inf/g" >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getline2::
	@echo $@
	@$(AWK) -f "$(srcdir)"/getline2.awk "$(srcdir)"/getline2.awk "$(srcdir)"/getline2.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

awkpath::
	@echo $@
	@AWKPATH="$(srcdir)$(PATH_SEPARATOR)$(srcdir)/lib" $(AWK) -f awkpath.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

argtest::
	@echo $@
	@$(AWK) -f "$(srcdir)"/argtest.awk -x -y abc >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

badargs::
	@echo $@
	@-$(AWK) -f 2>&1 | grep -v patchlevel >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strftime::
	@echo $@
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=C ; export GAWKLOCALE; \
	TZ=GMT0; export TZ; \
	$(AWK) -v OUTPUT=_$@ -v DATECMD="$(DATE)" -f "$(srcdir)"/strftime.awk || echo EXIT CODE: $$? >> _$@
	@-$(CMP) strftime.ok _$@ && rm -f _$@ strftime.ok || exit 0

devfd::
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@$(AWK) 1 /dev/fd/4 /dev/fd/5 4<"$(srcdir)"/devfd.in4 5<"$(srcdir)"/devfd.in5 >_$@ 2>&1 || echo EXIT CODE: $$? >> _$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# This cannot be autogenerated; we want it to read the input name
# on the command line.
errno:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

tweakfld::
	@echo $@
	@$(AWK) -f "$(srcdir)"/tweakfld.awk "$(srcdir)"/tweakfld.in >_$@ || echo EXIT CODE: $$? >> _$@
	@rm -f errors.cleanup
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# AIX /bin/sh exec's the last command in a list, therefore issue a ":"
# command so that pid.sh is fork'ed as a child before being exec'ed.
pid::
	@echo $@
	@echo Expect $@ to fail with DJGPP and MinGW.
	@AWKPATH="$(srcdir)" AWK=$(AWKPROG) $(SHELL) "$(srcdir)"/pid.sh $$$$ > _`basename $@` ; :
	@-$(CMP) "$(srcdir)"/pid.ok _`basename $@` && rm -f _`basename $@`

strftlng::
	@echo $@
	@TZ=UTC; export TZ; $(AWK) -f "$(srcdir)"/strftlng.awk >_$@
	@if $(CMP) "$(srcdir)"/strftlng.ok _$@ >/dev/null 2>&1 ; then : ; else \
	TZ=UTC0; export TZ; $(AWK) -f "$(srcdir)"/strftlng.awk >_$@ ; \
	fi
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nors::
	@echo $@
	@echo A B C D E | tr -d '\12\15' | $(AWK) '{ print $$NF }' - "$(srcdir)"/nors.in > _$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# fmtspcl.ok: fmtspcl.tok Makefile fix-fmtspcl.awk
# 	@$(AWK) -v "sd=$(srcdir)" -f "$(srcdir)/fix-fmtspcl.awk" < "$(srcdir)"/fmtspcl.tok > $@ 2>/dev/null
# 
# fmtspcl: fmtspcl.ok
# 	@echo $@
# 	@$(AWK) $(AWKFLAGS) -f "$(srcdir)"/fmtspcl.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
# 	@-if test -z "$$AWKFLAGS" ; then $(CMP) $@.ok _$@ && rm -f _$@ ; else \
# 	$(CMP) "$(srcdir)"/$@-mpfr.ok _$@ && rm -f _$@ ; \
# 	fi

rebuf::
	@echo $@
	@AWKBUFSIZE=4096 AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsglstdin:: 
	@echo $@
	@cat  "$(srcdir)"/rsgetline.in | AWKPATH="$(srcdir)" $(AWK) -f rsgetline.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

pipeio1::
	@echo $@
	@$(AWK) -f "$(srcdir)"/pipeio1.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@rm -f test1 test2
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

pipeio2::
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@$(AWK) -v SRCDIR="$(srcdir)" -f "$(srcdir)"/pipeio2.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clobber::
	@echo $@
	@$(AWK) -f "$(srcdir)"/clobber.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/clobber.ok seq && $(CMP) "$(srcdir)"/clobber.ok _$@ && rm -f _$@
	@-rm -f seq

arynocls::
	@echo $@
	@-AWKPATH="$(srcdir)" $(AWK) -v INPUT="$(srcdir)"/arynocls.in -f arynocls.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getlnbuf::
	@echo $@
	@-AWKPATH="$(srcdir)" $(AWK) -f getlnbuf.awk "$(srcdir)"/getlnbuf.in > _$@ || echo EXIT CODE: $$? >> _$@
	@-AWKPATH="$(srcdir)" $(AWK) -f gtlnbufv.awk "$(srcdir)"/getlnbuf.in > _2$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/getlnbuf.ok _$@ && $(CMP) "$(srcdir)"/getlnbuf.ok _2$@ && rm -f _$@ _2$@

inetmesg::
	@echo These tests only work if your system supports the services
	@echo "'discard'" at port 9 and "'daytimed'" at port 13. Check your
	@-echo file /etc/services and do "'netstat -a'".

inetechu::
	@echo Expect $@ to fail with DJGPP.
	@echo This test is for establishing UDP connections
	@-$(AWK) 'BEGIN {print "" |& "/inet/udp/0/127.0.0.1/9"}'

inetecht::
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo This test is for establishing TCP connections
	@-$(AWK) 'BEGIN {print "" |& "/inet/tcp/0/127.0.0.1/9"}'

inetdayu::
	@echo Expect $@ to fail with DJGPP.
	@echo This test is for bidirectional UDP transmission
	@-$(AWK) 'BEGIN { print "" |& "/inet/udp/0/127.0.0.1/13"; \
	"/inet/udp/0/127.0.0.1/13" |& getline; print $0}'

inetdayt::
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo This test is for bidirectional TCP transmission
	@-$(AWK) 'BEGIN { print "" |& "/inet/tcp/0/127.0.0.1/13"; \
	"/inet/tcp/0/127.0.0.1/13" |& getline; print $0}'

redfilnm::
	@echo $@
	@$(AWK) -f "$(srcdir)"/redfilnm.awk srcdir="$(srcdir)" "$(srcdir)"/redfilnm.in >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

space::
	@echo $@ $(ZOS_FAIL)
	@$(AWK) -f ' ' "$(srcdir)"/space.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@ || echo EXIT CODE: $$? >> _$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsnulbig::
	@echo $@
	@ : Suppose that block size for pipe is at most 128kB:
	@$(AWK) 'BEGIN { for (i = 1; i <= 128*64+1; i++) print "abcdefgh123456\n" }' 2>&1 | \
	$(AWK) 'BEGIN { RS = ""; ORS = "\n\n" }; { print }' 2>&1 | \
	$(AWK) ' /^[^a]/; END{ print NR }' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsnulbig2::
	@echo $@
	@$(AWK) 'BEGIN { ORS = ""; n = "\n"; for (i = 1; i <= 10; i++) n = (n n); \
		for (i = 1; i <= 128; i++) print n; print "abc\n" }' 2>&1 | \
		$(AWK) 'BEGIN { RS = ""; ORS = "\n\n" };{ print }' 2>&1 | \
		$(AWK) ' /^[^a]/; END { print NR }' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# This test makes sure gawk exits with a zero code.
# Thus, unconditionally generate the exit code.
exitval1::
	@echo $@
	@$(AWK) -f "$(srcdir)"/exitval1.awk >_$@ 2>&1; echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fsspcoln::
	@echo $@
	@$(AWK) -f "$(srcdir)"/$@.awk 'FS=[ :]+' "$(srcdir)"/$@.in >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsstart3::
	@echo $@
	@head "$(srcdir)"/rsstart1.in | $(AWK) -f "$(srcdir)"/rsstart2.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# FIXME: Gentests can't really deal with something that is both a shell script
# and requires a locale. We might can fix that ...
rtlenmb::
	@echo $@
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE ; \
	"$(srcdir)"/rtlen.sh >_$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nofile::
	@echo $@ $(ZOS_FAIL)
	@$(AWK) '{}' no/such/file >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

binmode1::
	@echo $@
	@$(AWK) -v BINMODE=3 'BEGIN { print BINMODE }' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

devfd1::
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@$(AWK) -f "$(srcdir)"/$@.awk 4< "$(srcdir)"/devfd.in1 5< "$(srcdir)"/devfd.in2 >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# The program text is the '1' which will print each record. How compact can you get?
devfd2::
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@$(AWK) 1 /dev/fd/4 /dev/fd/5 4< "$(srcdir)"/devfd.in1 5< "$(srcdir)"/devfd.in2 >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mixed1::
	@echo $@
	@$(AWK) -f /dev/null --source 'BEGIN {return junk}' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbprintf5::
	@echo $@ $(ZOS_FAIL)
	@-case `uname` in \
	CYGWIN* | MSYS* | MINGW32* | *MS-DOS*) echo this test fails on this system --- skipping $@ ;; \
	*) \
	[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE ; \
	$(AWK) -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >> _$@ ; \
	$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; \
	esac

printfbad2: printfbad2.ok
	@echo $@
	@$(AWK) --lint -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.in 2>&1 | sed 's;$(srcdir)/;;g' >_$@ || echo EXIT CODE: $$?  >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

beginfile1::
	@echo Expect $@ to fail with DJGPP.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk "$(srcdir)"/$@.awk . ./no/such/file Makefile  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

beginfile2:
	@echo $@ $(ZOS_FAIL)
	@-( cd "$(srcdir)" && LC_ALL=C AWK="$(abs_builddir)/$(AWKPROG)" $(abs_srcdir)/$@.sh $(abs_srcdir)/$@.in ) > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dumpvars::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --dump-variables 1 < "$(srcdir)"/$@.in >/dev/null 2>&1 || echo EXIT CODE: $$? >>_$@ || echo EXIT CODE: $$? >> _$@
	@grep -v ENVIRON < awkvars.out | grep -v PROCINFO > _$@; rm awkvars.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile0:
	@echo $@
	@$(AWK) --profile=ap-$@.out -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.in > /dev/null
	@sed 1,2d < ap-$@.out > _$@; rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile1:
	@echo $@
	@$(AWK) -f "$(srcdir)"/xref.awk "$(srcdir)"/dtdgport.awk > _$@.out1
	@$(AWK) --pretty-print=ap-$@.out -f "$(srcdir)"/xref.awk
	@$(AWK) -f ./ap-$@.out "$(srcdir)"/dtdgport.awk > _$@.out2 ; rm ap-$@.out
	@-$(CMP) _$@.out1 _$@.out2 && rm _$@.out[12] || { echo EXIT CODE: $$? >>_$@ ; \
	cp "$(srcdir)"/dtdgport.awk $@.ok ; }

profile2:
	@echo $@
	@$(AWK) --profile=ap-$@.out -v sortcmd=$(SORT) -f "$(srcdir)"/xref.awk "$(srcdir)"/dtdgport.awk > /dev/null
	@sed 1,2d < ap-$@.out > _$@; rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile3:
	@echo $@
	@$(AWK) --profile=ap-$@.out -f "$(srcdir)"/$@.awk > /dev/null
	@sed 1,2d < ap-$@.out > _$@; rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --pretty=_$@ -f $@.awk 2> _$@.err
	@cat _$@.err >> _$@ ; rm -f _$@.err
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile6:
	@echo $@
	@$(AWK) --profile=ap-$@.out -f "$(srcdir)"/$@.awk > /dev/null
	@sed 1,2d < ap-$@.out > _$@; rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile7:
	@echo $@
	@$(AWK) --profile=ap-$@.out -f "$(srcdir)"/$@.awk > /dev/null
	@sed 1,2d < ap-$@.out > _$@; rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile12:
	@echo $@
	@$(AWK) --profile=ap-$@.out -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.in > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@rm ap-$@.out
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrieee:
	@echo $@
	@$(AWK) -M -vPREC=double -f "$(srcdir)"/$@.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrexprange:
	@echo $@
	@$(AWK) -M -vPREC=53 -f "$(srcdir)"/$@.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrrnd:
	@echo $@
	@$(AWK) -M -vPREC=53 -f "$(srcdir)"/$@.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrsort:
	@echo $@
	@$(AWK) -M -vPREC=53 -f "$(srcdir)"/$@.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfruplus:
	@echo $@
	@$(AWK) -M -f "$(srcdir)"/uplus.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfranswer42:
	@echo $@
	@$(AWK) -M -f "$(srcdir)"/mpfranswer42.awk > _$@ 2>&1 || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrmemok1:
	@echo $@
	@$(AWK) -p- -M -f "$(srcdir)"/$@.awk 2>&1 | sed 1d > _$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

jarebug::
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@"$(srcdir)"/$@.sh "$(AWKPROG)" "$(srcdir)"/$@.awk "$(srcdir)"/$@.in "_$@" || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ordchr2::
	@echo $@
	@$(AWK) --load ordchr 'BEGIN {print chr(ord("z"))}' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# N.B. If the test fails, create readfile.ok so that "make diffout" will work
readfile::
	@echo $@
	@$(AWK) -l readfile 'BEGIN {printf "%s", readfile("$(srcdir)/Makefile.am")}' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)/Makefile.am" _$@ && rm -f _$@ || cp -p "$(srcdir)/Makefile.am" $@.ok

readfile2::
	@echo $@
	@$(AWK) -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.awk "$(srcdir)"/readdir.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsawk1a::
	@echo $@
	@$(AWK) -f "$(srcdir)"/nsawk1.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsawk1b::
	@echo $@
	@$(AWK) -v I=fine -f "$(srcdir)"/nsawk1.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsawk1c::
	@echo $@
	@$(AWK) -v awk::I=fine -f "$(srcdir)"/nsawk1.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsawk2a::
	@echo $@
	@$(AWK) -v I=fine -f "$(srcdir)"/nsawk2.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsawk2b::
	@echo $@
	@$(AWK) -v awk::I=fine -f "$(srcdir)"/nsawk2.awk > _$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

include2::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --include inclib 'BEGIN {print sandwich("a", "b", "c")}' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -i inclib -i inclib.awk 'BEGIN {print sandwich("a", "b", "c")}' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe2::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -f inclib -f inclib.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe3::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -f hello -f hello.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe4::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -f hello -i hello.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe5::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -i hello -f hello.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe6::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -i inchello -f hello.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

incdupe7::
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --lint -f hello -i inchello >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

inplace1::
	@echo $@
	@cp "$(srcdir)"/inplace.1.in _$@.1
	@cp "$(srcdir)"/inplace.2.in _$@.2
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace 'BEGIN {print "before"} {gsub(/foo/, "bar"); print} END {print "after"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
	@-$(CMP) "$(srcdir)"/$@.1.ok _$@.1 && rm -f _$@.1
	@-$(CMP) "$(srcdir)"/$@.2.ok _$@.2 && rm -f _$@.2

inplace2::
	@echo $@
	@cp "$(srcdir)"/inplace.1.in _$@.1
	@cp "$(srcdir)"/inplace.2.in _$@.2
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v inplace::suffix=.bak 'BEGIN {print "before"} {gsub(/foo/, "bar"); print} END {print "after"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
	@-$(CMP) "$(srcdir)"/$@.1.ok _$@.1 && rm -f _$@.1
	@-$(CMP) "$(srcdir)"/$@.1.bak.ok _$@.1.bak && rm -f _$@.1.bak
	@-$(CMP) "$(srcdir)"/$@.2.ok _$@.2 && rm -f _$@.2
	@-$(CMP) "$(srcdir)"/$@.2.bak.ok _$@.2.bak && rm -f _$@.2.bak

inplace2bcomp::
	@echo $@
	@cp "$(srcdir)"/inplace.1.in _$@.1
	@cp "$(srcdir)"/inplace.2.in _$@.2
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v INPLACE_SUFFIX=.orig 'BEGIN {print "before"} {gsub(/foo/, "bar"); print} END {print "after"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
	@-$(CMP) "$(srcdir)"/$@.1.ok _$@.1 && rm -f _$@.1
	@-$(CMP) "$(srcdir)"/$@.1.orig.ok _$@.1.orig && rm -f _$@.1.orig
	@-$(CMP) "$(srcdir)"/$@.2.ok _$@.2 && rm -f _$@.2
	@-$(CMP) "$(srcdir)"/$@.2.orig.ok _$@.2.orig && rm -f _$@.2.orig

inplace3::
	@echo $@
	@cp "$(srcdir)"/inplace.1.in _$@.1
	@cp "$(srcdir)"/inplace.2.in _$@.2
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v inplace::suffix=.bak 'BEGIN {print "before"} {gsub(/foo/, "bar"); print} END {print "after"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v inplace::suffix=.bak 'BEGIN {print "Before"} {gsub(/bar/, "foo"); print} END {print "After"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >>_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
	@-$(CMP) "$(srcdir)"/$@.1.ok _$@.1 && rm -f _$@.1
	@-$(CMP) "$(srcdir)"/$@.1.bak.ok _$@.1.bak && rm -f _$@.1.bak
	@-$(CMP) "$(srcdir)"/$@.2.ok _$@.2 && rm -f _$@.2
	@-$(CMP) "$(srcdir)"/$@.2.bak.ok _$@.2.bak && rm -f _$@.2.bak

inplace3bcomp::
	@echo $@
	@cp "$(srcdir)"/inplace.1.in _$@.1
	@cp "$(srcdir)"/inplace.2.in _$@.2
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v INPLACE_SUFFIX=.orig 'BEGIN {print "before"} {gsub(/foo/, "bar"); print} END {print "after"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@AWKPATH="$(srcdir)"/../awklib/eg/lib $(AWK) -i inplace -v INPLACE_SUFFIX=.orig 'BEGIN {print "Before"} {gsub(/bar/, "foo"); print} END {print "After"}' _$@.1 - _$@.2 < "$(srcdir)"/inplace.in >>_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
	@-$(CMP) "$(srcdir)"/$@.1.ok _$@.1 && rm -f _$@.1
	@-$(CMP) "$(srcdir)"/$@.1.orig.ok _$@.1.orig && rm -f _$@.1.orig
	@-$(CMP) "$(srcdir)"/$@.2.ok _$@.2 && rm -f _$@.2
	@-$(CMP) "$(srcdir)"/$@.2.orig.ok _$@.2.orig && rm -f _$@.2.orig

testext::
	@echo $@
	@$(AWK) ' /^(@load|BEGIN)/,/^}/' "$(top_srcdir)"/extension/testext.c > testext.awk
	@$(AWK) -f ./testext.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ testext.awk testexttmp.txt

getfile:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -v TESTEXT_QUIET=1 -ltestext -f $@.awk < $(srcdir)/$@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

readdir:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@if [ "`uname`" = Linux ] && [ "`stat -f . 2>/dev/null | awk 'NR == 2 { print $$NF }'`" = nfs ];  then \
	echo This test may fail on GNU/Linux systems when run on NFS or JFS filesystems.; \
	echo If it does, try rerunning on an ext'[234]' filesystem. ; \
	fi
	@$(AWK) -f "$(srcdir)"/readdir.awk "$(top_srcdir)" > _$@ || echo EXIT CODE: $$? >> _$@
	@$(LS) -afi "$(top_srcdir)" > _dirlist
	@$(LS) -lna "$(top_srcdir)" | sed 1d > _longlist
	@$(AWK) -f "$(srcdir)"/readdir0.awk -v extout=_$@  \
		-v dirlist=_dirlist -v longlist=_longlist > $@.ok
	@-$(CMP) $@.ok _$@ && rm -f $@.ok _$@ _dirlist _longlist

readdir_test:
	@echo $@
	@$(AWK) -lreaddir -F$(SLASH) '{printf "[%s] [%s] [%s] [%s]\n", $$1, $$2, $$3, $$4}' "$(top_srcdir)" > $@.ok
	@$(AWK) -lreaddir_test '{printf "[%s] [%s] [%s] [%s]\n", $$1, $$2, $$3, $$4}' "$(top_srcdir)" > _$@
	@-$(CMP) $@.ok _$@ && rm -f $@.ok _$@

readdir_retest:
	@echo $@
	@$(AWK) -lreaddir -F$(SLASH) -f "$(srcdir)"/$@.awk "$(top_srcdir)" > $@.ok
	@$(AWK) -lreaddir_test -F$(SLASH) -f "$(srcdir)"/$@.awk "$(top_srcdir)" > _$@
	@-$(CMP) $@.ok _$@ && rm -f $@.ok _$@

fts:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@case `uname` in \
	IRIX) \
	echo This test may fail on IRIX systems when run on an NFS filesystem.; \
	echo If it does, try rerunning on an xfs filesystem. ;; \
	CYGWIN* | MSYS*) \
	echo This test may fail on CYGWIN systems when run on an NFS filesystem.; \
	echo If it does, try rerunning on an ntfs filesystem. ;; \
	esac
	@$(AWK) -f "$(srcdir)"/fts.awk || echo EXIT CODE: $$? >> _$@
	@-$(CMP) $@.ok _$@ && rm -f $@.ok _$@

# BINMODE=2 is needed for PC tests.
charasbytes:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -b -v BINMODE=2 -f $@.awk "$(srcdir)"/$@.in | \
	od -c -t x1 | tr '	' ' ' | sed -e 's/  */ /g' -e 's/ *$$//' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab6:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab8:
	@echo $@ $(ZOS_FAIL)
	@$(AWK) -d__$@ -f "$(srcdir)"/$@.awk "$(srcdir)"/$@.in >_$@
	@grep -v '^ENVIRON' __$@ | grep -v '^PROCINFO' | grep -v '^FILENAME' >> _$@ ; rm __$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab9:
	@echo $@
	@$(AWK) -f "$(srcdir)"/$@.awk >_$@ || echo EXIT CODE: $$? >> _$@
	@rm -f testit.txt
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

reginttrad:
	@echo $@
	@$(AWK) --traditional -r -f "$(srcdir)"/$@.awk > _$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

colonwarn:
	@echo $@
	@for i in 1 2 3 ; \
	do $(AWK) -f "$(srcdir)"/$@.awk $$i < "$(srcdir)"/$@.in ; \
	done > _$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dbugeval::
	@echo $@
	@-if [ -t 0 ]; then \
	$(AWK) --debug -f /dev/null < "$(srcdir)"/$@.in > _$@  2>&1 || echo EXIT CODE: $$? >>_$@ ; \
	$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; \
	fi

filefuncs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk -v builddir="$(abs_top_builddir)"  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

genpot:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk --gen-pot >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

negtime::
	@echo $@
	@TZ=GMT AWKPATH="$(srcdir)" $(AWK) -f $@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-AWKPATH="$(srcdir)" $(AWK) -f checknegtime.awk "$(srcdir)"/$@.ok _$@ && rm -f _$@

watchpoint1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -D -f $@.awk $(srcdir)/$@.in < $(srcdir)/$@.script >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

pty1:
	@echo $@
	@echo Expect $@ to fail with DJGPP and MinGW.
	@-case `uname` in \
	*[Oo][Ss]/390*) : ;; \
	*) AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@ ; \
	$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ;; \
	esac

pty2:
	@echo $@
	@echo Expect $@ to fail with DJGPP and MinGW.
	@-case `uname` in \
	*[Oo][Ss]/390*) : ;; \
	*) AWKPATH="$(srcdir)" $(AWK) -f $@.awk | od -c | $(AWK) '{ $$1 = $$1 ; print }' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@ ; \
	$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ;; \
	esac

ignrcas3::
	@echo $@
	@echo Expect $@ to fail with DJGPP and MinGW.
	@-if locale -a | grep ell_GRC.1253 > /dev/null ; then \
	[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ell_GRC.1253 ; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@ ; \
	$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; \
	fi

arrdbg:
	@echo $@
	@$(AWK) -v "okfile=./$@.ok" -f "$(srcdir)"/$@.awk | grep array_f >_$@ || echo EXIT CODE: $$? >> _$@
	@-$(CMP) ./$@.ok _$@ && rm -f _$@ ./$@.ok

sourcesplit:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) --source='BEGIN { a = 5;' --source='print a }' >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

eofsrc1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@a.awk -f $@b.awk >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsbad_cmd:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -v foo:bar=3 -v foo:::blat=4 1 /dev/null >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# Use [:] in the regexp to keep MSYS from converting the /'s to \'s.
nonfatal1:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk 2>&1 | $(AWK) '{print gensub(/invalid[:].*$$/, "invalid", 1, $$0)}' >_$@ || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# 4/2018: On first call to $(CMP), send to /dev/null even with -s for MinGW.
nlstringtest::
	@echo $@
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=FRA_FRA.1252 ; export GAWKLOCALE ; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk "$(srcdir)" >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-if $(CMP) -s "$(srcdir)"/nlstringtest-nogettext.ok _$@ > /dev/null ; \
	then \
		rm -f _$@ ; \
	else \
		$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; \
	fi

longwrds:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk -v SORT="$(SORT)" < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

spacere:
	@echo $@
	@LC_ALL=C AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex4:
	@echo $@
	@$(AWK) -v x=@$(SLASH)foo/ -f "$(srcdir)"/$@.awk y=@$(SLASH)bar/ /dev/null >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@
Gt-dummy:
# file Maketests, generated from Makefile.am by the Gentests program
addcomma:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

anchgsub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

anchor:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayind1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayind2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayind3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayparm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayprm2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayprm3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrayref:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arrymem1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arryref2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arryref3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arryref4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arryref5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arynasty:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm6:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm7:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm8:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryprm9:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arysubnm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aryunasgn:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

asgext:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

assignnumfield:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

assignnumfield2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

back89:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

backgsub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

badassign1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

badbuild:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

callparam:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

childin:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

closebad:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clsflnam:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

compare2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

concat1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

concat2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

concat3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

concat4:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

concat5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

convfmt:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

datanonl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

defref:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

delargv:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

delarpm2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

delarprm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

delfunc:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dfamb1:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dfastress:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dynlj:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

escapebrace:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

eofsplit:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

exit2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

exitval2:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

exitval3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fcall_exit:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fcall_exit2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fldchg:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fldchgnf:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fldterm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnamedat:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnarray:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnarray2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnaryscl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnasgnm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnmisc:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fordel:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

forref:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

forsimp:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fsbs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fscaret:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fsnul1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fsrs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fstabplus:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

funsemnl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

funsmnam:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

funstack:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getline:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getline3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getline4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getline5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getnr2tb:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getnr2tm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubasgn:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtest:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --re-interval < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst5:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst6:
	@echo $@
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=C; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst7:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubtst8:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

hex:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

hex2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

hsprint:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

inpref:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

inputred:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

intest:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

intprec:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

iobug1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

leaddig:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

leadnl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

litoct:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --traditional < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

longsub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

manglprm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

math:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

membug1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

memleak:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

minusstr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mmap8k:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nasty:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nasty2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

negexp:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

negrange:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nested:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nfldstr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nfloop:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nfneg:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nfset:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nlfldsep:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nlinstr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nlstrina:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

noeffect:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nofmtch:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

noloop1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

noloop2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nonl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

noparms:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nulinsrc:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nulrsend:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

numindex:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

numrange:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

numstr1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

numsubstr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

octsub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmt:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmta:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmtbig:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmtfidl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmts:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofmtstrnum:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ofs1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

onlynl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

opasnidx:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

opasnslf:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramasfunc1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramasfunc2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramdup:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramres:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramtyp:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

paramuninitglobal:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

parse1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

parsefld:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

parseme:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

pcntplus:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

posix2008sub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prdupval:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prec:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printf0:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printf1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printfchar:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prmarscl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prmreuse:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prt1eval:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

prtoeval:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rand:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) $(AWKFLAGS) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-if test -z "$$AWKFLAGS" ; then $(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; else \
	$(CMP) "$(srcdir)"/$@-mpfr.ok _$@ && rm -f _$@ ; \
	fi

randtest:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

range1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

range2:
	@echo $@
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=C; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

readbuf:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rebrackloc:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rebt8b1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rebuild:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regeq:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regexpbrack:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regexpbrack2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regexprange:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regrange:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

reindops:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

reparse:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

resplit:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rri1:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rscompat:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --traditional < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsnul1nl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsnulw:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest4:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest5:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rswhite:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

scalar:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sclforin:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sclifin:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

setrec0:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

setrec1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sigpipe1:
	@echo Expect $@ to fail with DJGPP.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sortempty:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sortglos:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitargv:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitarr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitdef:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitvar:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitwht:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

status-close:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strcat1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strnum1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strnum2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strtod:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

subamp:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

subback:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

subi18n:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

subsepnm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

subslash:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

substr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

swaplns:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

synerr1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

synerr2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

synerr3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

tailrecurse:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

tradanch:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --traditional < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

trailbs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uninit2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uninit3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uninit4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uninit5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uninitialized:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

unterm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uparrfs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

uplus:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

wideidx:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

wideidx2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

widesub:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

widesub2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

widesub3:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

widesub4:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

wjposer1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

zero2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

zeroe0:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

zeroflag:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fflush:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getlnhd:
	@echo $@
	@echo Expect $@ to fail with DJGPP and MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

localenl:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rtlen:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rtlen01:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aadelete1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aadelete2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aarray1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aasort:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

aasorti:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arraysort:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arraysort2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

arraytype:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

backw:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=C; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way2:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way3:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way4:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way5:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

clos1way6:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

crlf:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dbugeval2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --debug < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dbugeval3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --debug < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dbugtypedre1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --debug < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dbugtypedre2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --debug < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

delsub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

dfacheck1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

exit:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fieldwdth:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

forcenum:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --non-decimal-data >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat6:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat7:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpat8:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fpatnull:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fsfwfs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

funlen:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

functab1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

functab2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

functab3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest6:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest7:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fwtest8:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gensub:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gensub2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gensub3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

getlndir:
	@echo Expect $@ to fail with DJGPP.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gnuops2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gnuops3:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gnureops:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

gsubind:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

icasefs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

icasers:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

id:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

igncdym:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

igncfs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ignrcas2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ignrcas4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ignrcase:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

include:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

indirectbuiltin:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

indirectcall:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

indirectcall2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

intarray:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --non-decimal-data >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

isarrayunset:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lint:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintexp:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintindex:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintint:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintlength:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintold:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint-old < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintset:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

lintwarn:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

match1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

match2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

match3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbstr1:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbstr2:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mktime:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

muldimposix:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --posix >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nastyparm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

next:
	@echo $@
	@-$(LOCALES) AWK="$(AWKPROG)" "$(srcdir)"/$@.sh  > _$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nondec:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nondec2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --non-decimal-data >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nonfatal2:
	@echo Expect $@ to fail with DJGPP.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nonfatal3:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsbad:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsforloop:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsfuncrecurse:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsindirect1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsindirect2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsprof1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

nsprof2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

patsplit:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

posix:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printfbad1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printfbad3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printfbad4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

printhuge:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

procinfs:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile8:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile9:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile10:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile11:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

profile13:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --pretty-print=_$@ >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regnul1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regnul2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

regx8bit:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

reint:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --re-interval < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

reint2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --re-interval < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsgetline:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsstart1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rsstart2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rstest6:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sandbox1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --sandbox >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

shadow:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --lint >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

shadowbuiltin:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sortfor:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sortfor2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sortu:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

split_after_fpat:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

splitarg4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strftfld:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strtonum:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

strtonum1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

stupid1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

stupid2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

stupid3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

stupid4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

switch2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab1:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab7:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab10:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  --debug < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

symtab11:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

timeout:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typedregex6:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typeof1:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typeof2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typeof3:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typeof4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

typeof5:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

double1:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

double2:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

intformat:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

asort:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

asorti:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

backbigs1:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

backsmalls1:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

backsmalls2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fmttest:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(TESTOUTCMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnarydel:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) $(AWKFLAGS) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-if test -z "$$AWKFLAGS" ; then $(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; else \
	$(CMP) "$(srcdir)"/$@-mpfr.ok _$@ && rm -f _$@ ; \
	fi

fnparydl:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) $(AWKFLAGS) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-if test -z "$$AWKFLAGS" ; then $(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@ ; else \
	$(CMP) "$(srcdir)"/$@-mpfr.ok _$@ && rm -f _$@ ; \
	fi

lc_num1:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbfw1:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbprintf1:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbprintf2:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbprintf3:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mbprintf4:
	@echo Expect $@ to fail with DJGPP and MinGW.
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=ENU_USA.1252; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mtchi18n:
	@echo $@ $(ZOS_FAIL)
	@[ -z "$$GAWKLOCALE" ] && GAWKLOCALE=RUS_RUS.1251; export GAWKLOCALE; \
	AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rebt8b2:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sort1:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

sprintfc:
	@echo $@ $(ZOS_FAIL)
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

apiterm:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fnmatch:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fork:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

fork2:
	@echo $@
	@echo Expect $@ to fail with MinGW.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

functab4:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

ordchr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

revout:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

revtwoway:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

rwarray:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

time:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrbigint:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrbigint2:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M --non-decimal-data < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrfield:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrnegzero:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrnr:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M < "$(srcdir)"/$@.in >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrrem:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrrndeval:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrsqrt:
	@echo $@
	@echo Expect $@ to fail with DJGPP.
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpfrstrtonum:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

mpgforcenum:
	@echo $@
	@AWKPATH="$(srcdir)" $(AWK) -f $@.awk  -M >_$@ 2>&1 || echo EXIT CODE: $$? >>_$@
	@-$(CMP) "$(srcdir)"/$@.ok _$@ && rm -f _$@

# end of file Maketests

# Targets generated for other tests:

Maketests: $(srcdir)/Makefile.am $(srcdir)/Gentests
	files=`cd "$(srcdir)" && echo *.awk *.in *.sh`; \
	$(AWK) -f "$(srcdir)"/Gentests "$(srcdir)"/Makefile.am $$files > $@

clean-local:
	rm -fr _* core core.* fmtspcl.ok junk strftime.ok test1 test2 \
	seq *~ readfile.ok fork.tmp.* testext.awk fts.ok readdir.ok \
	readdir_test.ok readdir_retest.ok \
	profile1.ok

# An attempt to print something that can be grepped for in build logs
pass-fail:
	@-COUNT=`ls _* 2>/dev/null | wc -l` ; \
	if test $$COUNT = 0 ; \
	then	echo ALL TESTS PASSED ; \
	else	echo $$COUNT TESTS FAILED ; exit 1; \
	fi

# This target for my convenience to look at all the results
# Don't use POSIX or bash-isms so that it'll work on !@#$%^&*() Solaris.
diffout:
	for i in _* ; \
	do  \
		if [ "$$i" != "_*" ]; then \
		echo ============== $$i ============= ; \
		base=`echo $$i | sed 's/^_//'` ; \
		if [ -r $${base}.ok ]; then \
		diff -c $${base}.ok $$i ; \
		else \
		diff -c "$(srcdir)"/$${base}.ok  $$i ; \
		fi ; \
		fi ; \
	done | more

# make things easier for z/OS
zos-diffout:
	@for i in $(EXPECTED_FAIL_ZOS); do \
		if [ -f _$$i ]; then mv -f _$$i X_$$i ; fi ; \
		if [ ! -f X_$$i ]; then echo $$i apparently passed! please check ; fi ; done
	@echo checking for new failures ...
	@-$(MAKE) diffout

# convenient way to scan valgrind results for errors
valgrind-scan:
	@echo "Scanning valgrind log files for problems:"
	@-$(AWK) -f "$(srcdir)"/valgrind.awk log.[0-9]*

# This target is for testing with electric fence.
efence:
	for i in $$(ls _* | sed 's;_\(.*\);\1;') ; \
	do \
		bad=$$(wc -l < _$$i) \
		ok=$$(wc -l < $$i.ok) ; \
		if (( $$bad == $$ok + 2 )) ; \
		then \
			rm _$$i ; \
		fi ; \
	done

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
