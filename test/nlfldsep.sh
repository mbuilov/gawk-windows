#! /bin/sh
AWK=${AWK-../gawk}

echo 'some stuff
more stuffA
junk
stuffA
final' | $AWK 'BEGIN { RS = "A" }
{print NF; for (i = 1; i <= NF; i++) print $i ; print ""}'

