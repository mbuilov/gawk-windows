BEGIN {
	if (PROCINFO["platform"] == "windows")
		cmd = "find /v \"\" 1>&2 & ping -n 2 127.0.0.1 > NUL"
	else
		cmd = "cat - 1>&2; sleep 2"
	PROCINFO[cmd, "NONFATAL"] = 1
	print "test1" |& cmd; close(cmd, "to")
	fflush(cmd)
	print "test2" |& cmd; print gensub(/number/, "descriptor", 1, ERRNO)
}
