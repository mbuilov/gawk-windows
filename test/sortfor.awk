{ a[$0]++ }
END {
	PROCINFO["sorted_in"] = "ascending"
	for (i in a)
		print i
	PROCINFO["sorted_in"] = "descending"
	for (i in a)
		print i
}
