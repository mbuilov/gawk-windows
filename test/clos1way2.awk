{
	cmd = "cat - 1>&2; sleep 2"
	print |& cmd; close(cmd, "to")
	fflush(cmd)
	print |& cmd; print ERRNO
}
