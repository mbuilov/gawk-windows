{
	# We use "&&" and not ";" so it works with Windows shells as well.
	if (PROCINFO["platform"] == "windows")
		cmd = "find /v \"\" 1>&2 & ping -n 2 127.0.0.1 > NUL"
	else
		cmd = "cat - 1>&2 && sleep 2"
	print |& cmd; close(cmd, "to")
	fflush(cmd)
	print |& cmd; print ERRNO
}
