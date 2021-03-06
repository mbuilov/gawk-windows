BEGIN {
	# We use "&&" and not ";" so it works with Windows shells as well.
	if (PROCINFO["platform"] == "windows")
		cmd = "echo test1& ping -n 2 127.0.0.1 >NUL&echo test2& ping -n 2 127.0.0.1 >NUL"
	else
		cmd = "echo test1&& sleep 1&& echo test2&& sleep 1"
	cmd |& getline x
	print x
	close(cmd, "from")
	cmd |& getline x
	print x
	print ERRNO
}
