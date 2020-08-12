#! /bin/bash

# This script is only useful for the maintainer ...
#
# It updates all the build-aux files from current GNULIB.
# We don't bother to print any messages about what we copied,
# as Git will tell us what, if anything, changed.

(cd /usr/local/src/Gnu/gnulib && git pull)

GL=/usr/local/src/Gnu/gnulib/lib

FILE_LIST="cdefs.h
dfa.c
dfa.h
flexmember.h
intprops.h
libc-config.h
localeinfo.c
localeinfo.h
regcomp.c
regex.c
regexec.c
regex.h
regex_internal.c
regex_internal.h
verify.h
xalloc.h"

for i in $FILE_LIST
do
	if [ -f $GL/$i ] && [ -f $i ]
	then
		cp $GL/$i $i
	fi
done
