extproc sh

#! /bin/sh

case $# in
2)	;;
*)	echo "Usage: mungeconf sysfile distfile" >&2 ; exit 2 ;;
esac

sed '/^#/d; /^MAKE_*/d' $1 |
sed '1s:.*:s~__SYSTEM__~&~:
2,$s:^\([^ 	]*\)[ 	].*:s~^/\\* #define[ 	]*\1.*~#define	&~:' >sedscr
sed -f sedscr $2

cmd /c del sedscr
