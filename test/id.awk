function function1()
{
	print "function1"
}

BEGIN { 
	an_array[1] = 1

	if (PROCINFO["platform"] == "windows") {
		for (i in PROCINFO["identifiers"])
		      printf("%s -> %s\n", i, PROCINFO["identifiers"][i]) | "sort /C /L C"
		close("sort /C /L C")
	} else {
		for (i in PROCINFO["identifiers"])
		      printf("%s -> %s\n", i, PROCINFO["identifiers"][i]) | "sort"
		close("sort")
	}
}
