BEGIN {
	LINT = 1

	# `%.*s' used for input file and for output file
	print "hi" > "f1"
	fflush("f1")
	getline x < "f1"
	print close("f1")
	print close("f1")
	fflush()

	# `%.*s' used for input file and input pipe
	# `%.*s' used for input file and two-way pipe
	# `%.*s' used for input pipe and two-way pipe
	getline data3 < "echo hello"
	"echo hello" |& getline data2
	"echo hello" | getline data

	print data, data2, data3

	print close("echo hello")
	print close("echo hello")
	print close("echo hello")
	fflush()

	# all should be ok, but the last close should fail
	getline x < "find /v \"\""
	print "foo" | "find /v \"\""
	print close("find /v \"\"")
	print close("find /v \"\"")
	fflush()

	# all should be ok, but the last close should fail
	getline x < "cat"
	print "foo" | "cat"
	print close("cat")
	print close("cat")
	fflush()

	# `%.*s' used for input file and output pipe
	getline x < "cat.bat"
	print "foo" | "cat.bat"
	print close("cat.bat")
	print close("cat.bat")
	fflush()

	# unnecessary mixing of `>' and `>>' for file `%.*s'
	print "foo" >  "f2"
	print "bar" >> "f2"
	print close("f2")
	print close("f2")	# -1 expected here
	fflush()

	# `%.*s' used for output file and output pipe"
	print "junk" > "catcat"
	print "hello_md5" | "catcat"
	print close("catcat")
	print close("catcat")
	fflush()

	# `%.*s' used for input pipe and output file
	"echo hello" | getline junk
	print "hello" > "echo hello"
	print close("echo hello")
	print close("echo hello")
	fflush()

	# `%.*s' used for output file and output pipe
	# `%.*s' used for output file and two-way pipe
	# `%.*s' used for output pipe and two-way pipe
	print "hello1" > "catcat"
	print "hello2" | "catcat"
	print "hello3" |& "catcat"
	print close("catcat")
	print close("catcat")
	print close("catcat")
	fflush()

	# `%.*s' used for input pipe and output pipe
	"echo hello" | getline junk
	print "hello" | "echo hello"
	print close("echo hello")
	print close("echo hello")
	fflush()
}
