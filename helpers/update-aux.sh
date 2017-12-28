#! /bin/bash

# This script is only useful for the maintainer ...
#
# It updates all the build-aux files from current GNULIB.
# We don't bother to print any messages about what we copied,
# as Git will tell us what, if anything, changed.

(cd /usr/local/src/Gnu/gnulib && git pull)

GL=/usr/local/src/Gnu/gnulib/build-aux

for i in $(cd $GL ; echo *)
do
	for dir in . extension/build-aux doc
	do
		if [ -f $GL/$i ] && [ -f $dir/$i ]
		then
			cp $GL/$i $dir/$i
		fi
	done
done
