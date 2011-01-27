BEGIN {
	PROCINFO["sorted_for_loop"] = 1
}
{ a[$0]++ }
END {
	for (i in a)
		print i
}
