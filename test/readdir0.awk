BEGIN {
	while ((getline x < extout) > 0) {
		numrec++
		if ((split(x, f, "/") == 3) && (f[3] == "u"))
			num_unknown++
	}
	close(extout)
	if ((numrec > 0) && (num_unknown == numrec)) {
		print "Notice: this filesystem does not appear to support file type information" > "/dev/stderr"
		ftype_unknown = 1
	}
}

BEGIN {
	dir = ARGV[1]
	delete ARGV[1]
	ls_afi = "ls -afi " dir
	ls_al = ("ls -lna " dir " | sed 1d")

	for (i = 1; (ls_afi | getline) > 0; i++) {
		# inode number is $1, filename is read of record
		inode = $1
		$1 = ""
		$0 = $0
		sub(/^ */, "")
		names[i] = $0
		ino[names[i]] = inode
	}
	close(ls_afi)

	for (j = 1; (ls_al | getline) > 0; j++) {
		type_let = substr($0, 1, 1)
		if (type_let == "-")
			type_let = "f"
		$1 = $2 = $3 = $4 = $5 = $6 = $7 = $8 = ""
		$0 = $0
		sub(/^ */, "")
		type[$0] = type_let
	}
	close(ls_al)

	if (i != j)
		printf("mismatch: %d from `ls -afi' and %d from `ls -l'\n", i, j) > "/dev/stderr"
	
	for (i = 1; i in names; i++)
		printf("%s/%s/%s\n", ino[names[i]], names[i], (ftype_unknown ? "u" : type[names[i]]))
}
