#!/bin/sh

AWK=../gawk
$AWK 'BEGIN { OFS = ORS = ""; for (j = 1; j <= 4; j++) for (i = 1; i <= 16; i++) print j}' > _rec.in
for i in 1 2 3 4 5
do
$AWK -f record.awk -vinfile='_rec.in' -e 'BEGIN {
reclen = 16
record(r, infile, reclen, "r+")
FIELDWIDTHS="8 4 4"
for (i = 1; i in r; i++) {
	$0 = r[i]
	print $1
}
delete r[1]
unbind_array(r)
print "--" }'
done
rm -f _rec.in
