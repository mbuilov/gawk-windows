BEGIN {
	print "system"
	if (PROCINFO["platform"] == "windows")
		command = "(for /L %L in (0,0,1) do @dir) | rem"
	else
		command = "yes | true"
	system(command)

	print "pipe to command"
	print "hi" | command
	close(command)

	print "pipe from command"
	command | getline x
	close(command)
}
