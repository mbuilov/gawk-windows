#!/bin/sh

# quoteconvert2.sh --- test locale dependent output of numbers
# Michal Jaegermann, michal@harddata.com, 2014/Mar/1

#AWK="../gawk"
SCRIPT=`basename $0`

if [ "$AWK" = "" ]
then
        echo $0: You must set AWK >&2
        exit 1
fi

# The last entry on this list represents "a locale with a typo".

llist="
C
en_US
en_US.UTF-8
de_DE
de_DE.UTF-8
fr_FR
fr_FR.UTF-8
pt_PT
pt_PT.UTF-8
pt_BR
pt_BR.UTF-8
ru_RU
ru_RU.UTF-8
pl_PX
"

error=0
for lc in $llist ; do
    wanted=`LC_ALL=$lc locale -c LC_NUMERIC 2>/dev/null | \
	$AWK 'NR == 2 { decimal_point = $0 }
	      NR == 3 { thousands_sep = $0 }
	      END  { printf("123%s456%s789%s15\n",
		       thousands_sep, thousands_sep, decimal_point)}'`
    got=`LC_ALL=$lc $AWK "BEGIN {printf(\"%'.2f\n\",123456789.15)}"`
    if [ "$wanted" != "$got" ] ; then
	echo "$lc - unexpected output $got instead of $wanted"
	error=1
    fi
done

[ "$error" = 0 ] && echo ok || echo bummer
exit $error
