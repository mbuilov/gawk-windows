# ../gawk -lsparr -f testsparr.awk
BEGIN {
	extension("./sparr.so")
	print SYS["time"]
	SYS["readline"] = "sparr.c";
	printf("File %s has %d lines\n", SYS["readline"], length(READLINE)) 
	SYS["readline"] = "testsparr.awk";
	printf("File %s has %d lines\n", SYS["readline"], length(READLINE)) 
	for (i = 1; i in READLINE; i++)
		print READLINE[i]

	system("sleep 1")

#	PROCINFO["/dev/stdin", "READ_TIMEOUT"] = 1000
#	getline < "/dev/stdin"

	print SYS["time"]
}
