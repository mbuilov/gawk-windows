BEGIN {
	PROCINFO["sorted_in"] = 1
}
{ a[$0]++ }
END {
	for (i in a)
		print i
}
