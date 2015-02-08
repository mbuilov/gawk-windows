BEGIN {
	PROCINFO["nonfatal"] = 1
	print > "/dev/no/such/file"
	print ERRNO
}
