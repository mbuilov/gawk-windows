@load "fnmatch"

BEGIN {
	print "FNM_NOMATCH =", FNM_NOMATCH 
	# can't print the values; they vary from system to system
	for (i in FNM)
		printf("\"%s\" is an element in FNM\n", i)

	printf("fnmatch(\"*.a\", \"foo.a\", 0)  = %d\n", fnmatch("*.a", "foo.a", 0) )
	printf("fnmatch(\"*.a\", \"foo.c\", 0) = %d\n", fnmatch("*.a", "foo.c", 0))
}
