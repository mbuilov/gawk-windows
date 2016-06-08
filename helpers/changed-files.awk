#! /usr/bin/gawk -f

/^2014-/	{ nextfile }

/^\t\*/	{
	sub(/^\t\*[[:space:]]*/, "")
	sub(/[:\[(].*/, "")
	gsub(/,/, "")
	for (i = 1; i <= NF; i++) {
		fname = $i
		if (! (fname in names))
			names[fname]++
	}
}

END {
	for (i in names)
		print i
}
