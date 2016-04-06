BEGIN {
	cmd = "echo test1; echo test2; sleep 2"
	cmd |& getline x
	print x
	close(cmd, "from")
	cmd |& getline x
	print x
	print ERRNO
}
