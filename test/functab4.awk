@load "filefuncs"

function foo()
{
	print "foo!"
}

BEGIN {
	f = FUNCTAB["foo"]
	@f()

	ret1 = stat(".", data1)
	print "ret1 =", ret1

	f = "stat"
	ret2 = @f(".", data2)
	print "ret2 =", ret2

	problem = 0
	for (i in data1) {
		if (! isarray(data1[i])) {
#			print i, data1[i]
			if (! (i in data2) || data1[i] != data2[i]) {
				printf("mismatch element \"%s\"\n", i)
				problems++
			}
		}
	}
	print(problems ? (problems+0) "encountered" : "no problems encountered")
}
