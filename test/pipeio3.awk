BEGIN {
	outputCommand = "cart"
	inputCommand = "ls"

	print "doing first print" > "/dev/stderr"
	print "hello" | outputCommand

	print "doing first getline" > "/dev/stderr"
	inputCommand | getline

	print "doing second print" > "/dev/stderr"
	print "hello" | outputCommand

	print "made it here" > "/dev/stderr"
}
