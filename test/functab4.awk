@load "filefuncs"

function foo()
{
	print "foo!"
}

BEGIN {
	x = FUNCTAB["chdir"]
	print "x =", x
	@x("/tmp")
	printf "we are now in --> "
	system("/bin/pwd || /usr/bin/pwd")
}
