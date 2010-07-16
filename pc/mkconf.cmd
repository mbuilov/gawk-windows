extproc sh

#! /bin/sh
#
# mkconf -- produce a config.h from a known configuration

case "$#" in
1)	;;
*)	echo "Usage:  mkconf system_type" >&2
	echo "Known systems:  `cd config; echo ;ls -C`" >&2
	exit 2
	;;
esac

if [ -f config/$1 ]; then
	sh ./mungeconf.cmd config/$1 config.h-dist >config.h
	sed -n '/^#echo /s///p' config/$1
	sed -n '/^MAKE_.*/s//s,^##&## ,,/p' config/$1 >sedscr
	if [ -s sedscr ]
	then
		sed -f sedscr Makefile-dist >Makefile
	else
		cp Makefile-dist Makefile
	fi
	cmd /c del sedscr
else
	echo "\`$1' is not a known configuration."
	echo "Either construct one based on the examples in the config directory,"
	echo "or copy config.h-dist to config.h and edit it."
	exit 1
fi
