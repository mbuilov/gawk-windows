# Makefile for GNU Awk test suite.
#
# Copyright (C) 1988-1995 the Free Software Foundation, Inc.
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

SHELL = /bin/sh

bigtest:	basic poundbang gawk.extensions

basic:	msg swaplns messages argarray longwrds \
	getline fstabplus compare arrayref rs fsrs rand \
	fsbs negexp asgext anchgsub splitargv awkpath nfset reparse \
	convfmt arrayparm paramdup nonl

gawk.extensions: fieldwdth ignrcase posix manyfiles igncfs argtest \
		badargs

extra:	regtest inftest

poundbang::
	cp ../gawk /tmp && chmod +x poundbang && ./poundbang poundbang >tmp
	rm -f /tmp/gawk
	cmp poundbang.good tmp && rm -f tmp

msg::
	@echo 'Any output from "cmp" is bad news, although some differences'
	@echo 'in floating point values are probably benign -- in particular,'
	@echo 'some systems may omit a leading zero and the floating point'
	@echo 'precision may lead to slightly different output in a few cases.'

swaplns::
	@../gawk -f swaplns.awk data >tmp
	cmp swaplns.good tmp && rm -f tmp

messages::
	@../gawk -f messages.awk >out2 2>out3
	{ cmp out1.good out1 && cmp out2.good out2 && cmp out3.good out3 && rm -f out1 out2 out3; } || { test -d /dev/fd && echo IT IS OK THAT THIS TEST FAILED; }

argarray::
	@TEST=test echo just a test | ../gawk -f argarray.awk argarray.awk - >tmp
	cmp argarray.good tmp && rm -f tmp

fstabplus::
	@echo '1		2' | ../gawk -f fstabplus >tmp
	cmp fstabplus.good tmp && rm -f tmp

fsrs::
	@../gawk -f fsrs.awk fsrs.in >tmp
	cmp fsrs.good tmp && rm -f tmp

igncfs::
	@../gawk -f igncfs.awk igncfs.in >tmp
	cmp igncfs.good tmp && rm -f tmp

longwrds::
	@../gawk -f longwrds.awk manpage | sort >tmp
	cmp longwrds.good tmp && rm -f tmp

fieldwdth::
	@echo '123456789' | ../gawk -v FIELDWIDTHS="2 3 4" '{ print $$2}' >tmp
	cmp fieldwdth.good tmp && rm -f tmp

ignrcase::
	@echo xYz | ../gawk -v IGNORECASE=1 '{ sub(/y/, ""); print}' >tmp
	cmp ignrcase.good tmp && rm -f tmp

regtest::
	@echo 'Some of the output from regtest is very system specific, do not'
	@echo 'be distressed if your output differs from that distributed.'
	@echo 'Manual inspection is called for.'
	AWK=`pwd`/../gawk ./regtest

posix::
	@echo '1:2,3 4' | ../gawk -f posix >tmp
	cmp posix.good tmp && rm -f tmp

manyfiles::
	@rm -rf junk
	@mkdir junk
	@../gawk 'BEGIN { for (i = 1; i <= 300; i++) print i, i}' >tmp
	@../gawk -f manyfiles.awk tmp tmp
	@echo "This number better be 1 ->" | tr -d '\012'
	@wc -l junk/* | ../gawk '$$1 != 2' | wc -l
	@rm -rf junk tmp

compare::
	@../gawk -f compare.awk 0 1 compare.in >tmp
	cmp compare.good tmp && rm -f tmp

arrayref::
	@../gawk -f arrayref >tmp
	cmp arrayref.good tmp && rm -f tmp

rs::
	@../gawk -v RS="" '{ print $$1, $$2}' rs.data >tmp
	cmp rs.good tmp && rm -f tmp

fsbs::
	@../gawk -v FS='\' '{ print $$1, $$2 }' fsbs.in >tmp
	cmp fsbs.good tmp && rm -f tmp

inftest::
	@echo This test is very machine specific...
	@../gawk -f inftest.awk >tmp
	cmp inftest.good tmp && rm -f tmp

getline::
	@../gawk -f getline.awk getline.awk getline.awk >tmp
	cmp getline.good tmp && rm -f tmp

rand::
	@echo The following line should just be 19 random numbers between 1 and 100
	@../gawk -f rand.awk

negexp::
	@../gawk 'BEGIN { a = -2; print 10^a }' >tmp
	cmp negexp.good tmp && rm -f tmp

asgext::
	@../gawk -f asgext.awk asgext.in >tmp
	cmp asgext.good tmp && rm -f tmp

anchgsub::
	@../gawk -f anchgsub.awk anchgsub.in >tmp
	cmp anchgsub.good tmp && rm -f tmp

splitargv::
	@../gawk -f splitargv.awk splitargv.in >tmp
	cmp splitargv.good tmp && rm -f tmp

awkpath::
	@AWKPATH=".:lib" ../gawk -f awkpath.awk >tmp
	cmp awkpath.good tmp && rm -f tmp

nfset::
	@../gawk -f nfset.awk nfset.in >tmp
	cmp nfset.good tmp && rm -f tmp

reparse::
	@../gawk -f reparse.awk reparse.in >tmp
	cmp reparse.good tmp && rm -f tmp

argtest::
	@../gawk -f argtest.awk -x -y abc >tmp
	cmp argtest.good tmp && rm -f tmp

badargs::
	@-../gawk -f 2>&1 | grep -v patchlevel >tmp
	cmp badargs.good tmp && rm -f tmp

convfmt::
	@../gawk -f convfmt.awk >tmp
	cmp convfmt.good tmp && rm -f tmp

arrayparm::
	@-../gawk -f arrayparm.awk >tmp 2>&1
	cmp arrayparm.good tmp && rm -f tmp

paramdup::
	@-../gawk -f paramdup.awk >tmp 2>&1
	cmp paramdup.good tmp && rm -f tmp

nonl::
	@-../gawk --lint -f nonl.awk /dev/null >tmp 2>&1
	cmp nonl.good tmp && rm -f tmp

clean:
	rm -fr tmp core junk
