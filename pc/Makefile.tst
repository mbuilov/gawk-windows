# Makefile for GNU Awk test suite.
#
# Copyright (C) 1988-1995 the Free Software Foundation, Inc.
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

# ============================================================================
# MS-DOS & OS/2 Notes: READ THEM!
# ============================================================================

# As of version 2.91, efforts to make this makefile run in MS-DOS and OS/2
# have started in earnest.  The following steps need to be followed in order 
# to run this makefile:
#
# 1. The first thing that you will need to do is to convert all of the 
#    files ending in ".ok" in the test directory and all of the files ending 
#    in ".good" (or ".goo") in the test/reg directory from having a linefeed
#    to having carriage return/linefeed at the end of each line. There are 
#    various public domain UNIX to DOS converters and any should work.
#
# 2. You will need an sh-compatible shell.  Please refer to the "README.pc"
#    file in the README_d directory for information about obtaining a copy.
#    You will also need various UNIX utilities.  At a minimum, you will 
#    need: rm, tr, cmp, cat, wc, and sh.  
#    You should also have a UNIX-compatible date program.
#
# 3. You will need a \tmp directory on the same drive as the test directory
#    for the poundba (called poundbang in the UNIX makefile) test.
#
# The makefile has only been tested with dmake 3.8.  After making all of these
# changes, typing "dmake bigtest extra" should run successfully.

# So far, the only MS-DOS & OS/2 shell that this has been found to work with
# is Stewartson's sh 2.3.  That version of sh will sometimes send long 
# command-line arguments to programs using the @ notation.  You may need
# to disable this feature of sh for programs that you have which don't support
# that feature.  For more information about the @ notation please refer to 
# the sh documentation.

# You will almost certainly need to change some of the values (MACROS) 
# defined on the next few lines.  

# .USESHELL is used by dmake.
.USESHELL = yes

# Using EMXSHELL=/bin/sh with emx versions can exhaust lower mem.
# The .SWAP setting forces (DOS-only) dmake to swap itself out.
#.SWAP: childin fflush

# This won't work unless you have "sh" and set SHELL equal to it.
#SHELL = e:\bin\sh.exe
SHELL = /bin/sh

# Point to gawk
AWK = ../gawk.exe

# Set your cmp command here
CMP = cmp
#CMP = gcmp

# Set your "cp" command here.  Note: It must take forward slashes.
# 'command -c copy' will work for MS-DOS if "command=noexpand switch export" is
# set in extend.lst.
#CP = cp
#CP = gcp
CP = command -c copy

# Set your unix-style date function here
#DATE = date
DATE = gdate

# Set your mkdir command here.
#MKDIR = /bin/mkdir
MKDIR = command -c mkdir

# ============================================================================
# You shouldn't need to modify anything below this line.
# ============================================================================

srcdir = .

bigtest:	basic poundba   gawk.extensions

basic:	msg swaplns messages argarray longwrds \
	getline fstabplus compare arrayref rs fsrs rand \
	fsbs negexp asgext anchgsub splitargv awkpath nfset reparse \
	convfmt arrayparm paramdup nonl defref nofmtch litoct resplit \
	rswhite prmarscl sclforin sclifin intprec childin noeffect \
	numsubstr pcntplus prmreuse math fflush fldchg

gawk.extensions: fieldwdth ignrcase posix manyfiles igncfs argtest \
		badargs strftime gensub gnureops

extra:	regtes  inftest

poundba::
# The need for "basename" has been removed for MS-DOS & OS/2 systems which
# lack it.
#	cp $(AWK) /tmp/gawk && $(srcdir)/poundbang $(srcdir)/poundbang >_`basename $@`
	$(CP) $(AWK) /tmp/gawk.exe && $(srcdir)/poundbang $(srcdir)/poundbang >_$@
#	rm -f /tmp/gawk
	rm -f /tmp/gawk.exe
#	$(CMP) $(srcdir)/poundbang.ok _`basename $@` && rm -f _`basename $@`
	$(CMP) $(srcdir)/poundbang.ok _$@ && rm -f _$@

msg::
	@echo 'Any output from "cmp" is bad news, although some differences'
	@echo 'in floating point values are probably benign -- in particular,'
	@echo 'some systems may omit a leading zero and the floating point'
	@echo 'precision may lead to slightly different output in a few cases.'

swaplns::
	@echo 'If swaplns fails make sure that all of the .ok files have CR/LFs.'
	@$(AWK) -f $(srcdir)/swaplns.awk $(srcdir)/swaplns.in >_$@
	$(CMP) $(srcdir)/swaplns.ok _$@ && rm -f _$@

messages::
	@echo 'If messages fails, set sh to swap to disk only (in sh.rc).'
	@$(AWK) -f $(srcdir)/messages.awk >out2 2>out3
#	{ $(CMP) $(srcdir)/out1.ok out1 && $(CMP) $(srcdir)/out2.ok out2 && $(CMP) $(srcdir)/out3.ok out3 && rm -f out1 out2 out3; } || { test -d /dev/fd && echo IT IS OK THAT THIS TEST FAILED; }
	{ $(CMP) $(srcdir)/out1.ok out1 && $(CMP) $(srcdir)/out2.ok out2 && $(CMP) $(srcdir)/out3.ok out3; } || { test -d /dev/fd && echo OK TEST FAILED; }
	rm -f out1 out2 out3

argarray::
	@case $(srcdir) in \
	.)	: ;; \
	*)	cp $(srcdir)/argarray.in . ;; \
	esac
	@TEST=test echo just a test | $(AWK) -f $(srcdir)/argarray.awk ./argarray.in - >_$@
	$(CMP) $(srcdir)/argarray.ok _$@ && rm -f _$@

fstabplus::
	@echo '1		2' | $(AWK) -f $(srcdir)/fstabplus.awk >_$@
	$(CMP) $(srcdir)/fstabplus.ok _$@ && rm -f _$@

fsrs::
	@$(AWK) -f $(srcdir)/fsrs.awk $(srcdir)/fsrs.in >_$@
	$(CMP) $(srcdir)/fsrs.ok _$@ && rm -f _$@

igncfs::
	@$(AWK) -f $(srcdir)/igncfs.awk $(srcdir)/igncfs.in >_$@
	$(CMP) $(srcdir)/igncfs.ok _$@ && rm -f _$@

longwrds::
	@$(AWK) -f $(srcdir)/longwrds.awk $(srcdir)/manpage | sort >_$@
	$(CMP) $(srcdir)/longwrds.ok _$@ && rm -f _$@

fieldwdth::
	@echo '123456789' | $(AWK) -v FIELDWIDTHS="2 3 4" '{ print $$2}' >_$@
	$(CMP) $(srcdir)/fieldwdth.ok _$@ && rm -f _$@

ignrcase::
	@echo xYz | $(AWK) -v IGNORECASE=1 '{ sub(/y/, ""); print}' >_$@
	$(CMP) $(srcdir)/ignrcase.ok _$@ && rm -f _$@

regtes::
	@echo 'Some of the output from regtest is very system specific, do not'
	@echo 'be distressed if your output differs from that distributed.'
	@echo 'Manual inspection is called for.'
	AWK=`pwd`/$(AWK) $(srcdir)/regtest

posix::
	@echo 'posix test may fail due to 1.500000e+000 not being equal to'
	@echo '1.500000e+00 for MSC 7.0 gawk.'
	@echo '1:2,3 4' | $(AWK) -f $(srcdir)/posix.awk >_$@
#	$(CMP) $(srcdir)/posix.ok _$@ && rm -f _$@
	-$(CMP) $(srcdir)/posix.ok _$@ && rm -f _$@

manyfiles::
	@rm -rf junk
#	@mkdir junk
	@$(MKDIR) junk
	@$(AWK) 'BEGIN { for (i = 1; i <= 300; i++) print i, i}' >_$@
	@$(AWK) -f $(srcdir)/manyfiles.awk _$@ _$@
#	@echo "This number better be 1 ->" | tr -d '\012'
	@echo "This number better be 1 ->" | tr -d '\012\015'
#	@wc -l junk/* | $(AWK) '$$1 != 2' | wc -l
	@wc -l "junk/*" | $(AWK) '$$1 != 2' | wc -l
# The quotes above are for people with a "wc" that doesn't support sh's "@"
# argument passing.
	@rm -rf junk _$@

compare::
	@$(AWK) -f $(srcdir)/compare.awk 0 1 $(srcdir)/compare.in >_$@
	$(CMP) $(srcdir)/compare.ok _$@ && rm -f _$@

arrayref::
	@$(AWK) -f $(srcdir)/arrayref.awk >_$@
	$(CMP) $(srcdir)/arrayref.ok _$@ && rm -f _$@

rs::
	@$(AWK) -v RS="" '{ print $$1, $$2}' $(srcdir)/rs.in >_$@
	$(CMP) $(srcdir)/rs.ok _$@ && rm -f _$@

fsbs::
	@$(AWK) -v FS='\' '{ print $$1, $$2 }' $(srcdir)/fsbs.in >_$@
	$(CMP) $(srcdir)/fsbs.ok _$@ && rm -f _$@

inftest::
	@echo This test is very machine specific...
	@echo 'MSC 7.0 gawk generates a floating point exception.'
	@echo 'EMX gawk uses #INF rather than Inf.'
#	@$(AWK) -f $(srcdir)/inftest.awk >_$@
	@-$(AWK) -f $(srcdir)/inftest.awk >_$@
#	$(CMP) $(srcdir)/inftest.ok _$@ && rm -f _$@
	-$(CMP) $(srcdir)/inftest.ok _$@ && rm -f _$@

getline::
	@$(AWK) -f $(srcdir)/getline.awk $(srcdir)/getline.awk $(srcdir)/getline.awk >_$@
	$(CMP) $(srcdir)/getline.ok _$@ && rm -f _$@

rand::
	@echo The following line should just be 19 random numbers between 1 and 100
	@$(AWK) -f $(srcdir)/rand.awk

negexp::
	@$(AWK) 'BEGIN { a = -2; print 10^a }' >_$@
	$(CMP) $(srcdir)/negexp.ok _$@ && rm -f _$@

asgext::
	@$(AWK) -f $(srcdir)/asgext.awk $(srcdir)/asgext.in >_$@
	$(CMP) $(srcdir)/asgext.ok _$@ && rm -f _$@

anchgsub::
	@$(AWK) -f $(srcdir)/anchgsub.awk $(srcdir)/anchgsub.in >_$@
	$(CMP) $(srcdir)/anchgsub.ok _$@ && rm -f _$@

splitargv::
	@$(AWK) -f $(srcdir)/splitargv.awk $(srcdir)/splitargv.in >_$@
	$(CMP) $(srcdir)/splitargv.ok _$@ && rm -f _$@

awkpath::
# MS-DOS and OS/2 use ; as a PATH delimiter
#	@AWKPATH="$(srcdir):$(srcdir)/lib" $(AWK) -f awkpath.awk >_$@
	@AWKPATH="$(srcdir);$(srcdir)/lib" $(AWK) -f awkpath.awk >_$@
	$(CMP) $(srcdir)/awkpath.ok _$@ && rm -f _$@

nfset::
	@$(AWK) -f $(srcdir)/nfset.awk $(srcdir)/nfset.in >_$@
	$(CMP) $(srcdir)/nfset.ok _$@ && rm -f _$@

reparse::
	@$(AWK) -f $(srcdir)/reparse.awk $(srcdir)/reparse.in >_$@
	$(CMP) $(srcdir)/reparse.ok _$@ && rm -f _$@

argtest::
	@$(AWK) -f $(srcdir)/argtest.awk -x -y abc >_$@
	$(CMP) $(srcdir)/argtest.ok _$@ && rm -f _$@

badargs::
# For MS-DOS & OS/2, we use " rather than ' in the usage statement.
	@-$(AWK) -f 2>&1 | grep -v patchlevel >_$@
# Next line converts " to ' for $(CMP) to work with UNIX badargs.ok
	@cat _$@ | tr '\042' '\047' > _$@.2
#	$(CMP) $(srcdir)/badargs.ok _$@ && rm -f _$@
	$(CMP) $(srcdir)/badargs.ok _$@.2 && rm -f _$@ _$@.2

convfmt::
	@$(AWK) -f $(srcdir)/convfmt.awk >_$@
	$(CMP) $(srcdir)/convfmt.ok _$@ && rm -f _$@

arrayparm::
	@-AWKPATH=$(srcdir) $(AWK) -f arrayparm.awk >_$@ 2>&1
	$(CMP) $(srcdir)/arrayparm.ok _$@ && rm -f _$@

paramdup::
	@-AWKPATH=$(srcdir) $(AWK) -f paramdup.awk >_$@ 2>&1
	$(CMP) $(srcdir)/paramdup.ok _$@ && rm -f _$@

nonl::
#	@-AWKPATH=$(srcdir) $(AWK) --lint -f nonl.awk /dev/null >_$@ 2>&1
	@-AWKPATH=$(srcdir) $(AWK) --lint -f nonl.awk NUL >_$@ 2>&1
	$(CMP) $(srcdir)/nonl.ok _$@ && rm -f _$@

defref::
	@-AWKPATH=$(srcdir) $(AWK) --lint -f defref.awk >_$@ 2>&1
	$(CMP) $(srcdir)/defref.ok _$@ && rm -f _$@

nofmtch::
	@-AWKPATH=$(srcdir) $(AWK) --lint -f nofmtch.awk >_$@ 2>&1
	$(CMP) $(srcdir)/nofmtch.ok _$@ && rm -f _$@

strftime::
	: this test could fail on slow machines or on a second boundary,
	: so if it does, double check the actual results
#	@date | $(AWK) '{ $$3 = sprintf("%02d", $$3 + 0) ; print }' > strftime.ok
	@$(DATE) | $(AWK) '{ $$3 = sprintf("%02d", $$3 + 0) ; print }' > strftime.ok
	@$(AWK) 'BEGIN { print strftime() }' >_$@
	-$(CMP) strftime.ok _$@ && rm -f _$@ strftime.ok

litoct::
	@echo ab | $(AWK) --traditional -f $(srcdir)/litoct.awk >_$@
	$(CMP) $(srcdir)/litoct.ok _$@ && rm -f _$@

gensub::
	@$(AWK) -f $(srcdir)/gensub.awk $(srcdir)/gensub.in >_$@
	$(CMP) $(srcdir)/gensub.ok _$@ && rm -f _$@

resplit::
	@echo 'If resplit fails, check extend.lst and remove "unix" by the "gawk=" line'
	@echo a:b:c d:e:f | $(AWK) '{ FS = ":"; $$0 = $$0; print $$2 }' > _$@
	$(CMP) $(srcdir)/resplit.ok _$@ && rm -f _$@

rswhite::
	@$(AWK) -f $(srcdir)/rswhite.awk $(srcdir)/rswhite.in > _$@
	$(CMP) $(srcdir)/rswhite.ok _$@ && rm -f _$@

prmarscl::
	@-AWKPATH=$(srcdir) $(AWK) -f prmarscl.awk > _$@ 2>&1
	$(CMP) $(srcdir)/prmarscl.ok _$@ && rm -f _$@

sclforin::
	@-AWKPATH=$(srcdir) $(AWK) -f sclforin.awk > _$@ 2>&1
	$(CMP) $(srcdir)/sclforin.ok _$@ && rm -f _$@

sclifin::
	@-AWKPATH=$(srcdir) $(AWK) -f sclifin.awk > _$@ 2>&1
	$(CMP) $(srcdir)/sclifin.ok _$@ && rm -f _$@

intprec::
	@-$(AWK) -f $(srcdir)/intprec.awk > _$@ 2>&1
	$(CMP) $(srcdir)/intprec.ok _$@ && rm -f _$@

childin::
	@echo hi | $(AWK) 'BEGIN { "cat" | getline; print; close("cat") }' > _$@
	$(CMP) $(srcdir)/childin.ok _$@ && rm -f _$@

noeffect::
	@-AWKPATH=$(srcdir) $(AWK) --lint -f noeffect.awk > _$@ 2>&1
	$(CMP) $(srcdir)/noeffect.ok _$@ && rm -f _$@

numsubstr::
	@-AWKPATH=$(srcdir) $(AWK) -f numsubstr.awk $(srcdir)/numsubstr.in >_$@
	$(CMP) $(srcdir)/numsubstr.ok _$@ && rm -f _$@

gnureops::
	@$(AWK) -f $(srcdir)/gnureops.awk >_$@
	$(CMP) $(srcdir)/gnureops.ok _$@ && rm -f _$@

pcntplus::
	@$(AWK) -f $(srcdir)/pcntplus.awk >_$@
	$(CMP) $(srcdir)/pcntplus.ok _$@ && rm -f _$@

prmreuse::
	@$(AWK) -f $(srcdir)/prmreuse.awk >_$@
	$(CMP) $(srcdir)/prmreuse.ok _$@ && rm -f _$@

math::
	@$(AWK) -f $(srcdir)/math.awk >_$@
	$(CMP) $(srcdir)/math.ok _$@ && rm -f _$@

fflush::
	@$(srcdir)/fflush.sh >_$@
	$(CMP) $(srcdir)/fflush.ok _$@ && rm -f _$@

fldchg::
	@$(AWK) -f $(srcdir)/fldchg.awk $(srcdir)/fldchg.in >_$@
	$(CMP) $(srcdir)/fldchg.ok _$@ && rm -f _$@

clean:
	rm -fr _* core junk
