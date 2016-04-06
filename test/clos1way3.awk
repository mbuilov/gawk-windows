BEGIN {
	cmd = "cat - 1>&2; sleep 2"
	print "test1" |& cmd
	close(cmd, "to")
	print "test2" |& cmd
	print ERRNO
}
