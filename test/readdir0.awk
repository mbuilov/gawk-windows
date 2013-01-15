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

{
	ino = $1
	name = $NF
	type = substr($2, 1, 1)
	if (type == "-")
		type = "f"

	printf "%s/%s/%s\n", ino, name, (ftype_unknown ? "u" : type)
}
