@load "fnmatch"

BEGIN {
	print "FNM_NOMATCH =", FNM_NOMATCH 
	for (i in FNM)
		printf("FNM[\"%s\"] = %d\n", i, FNM[i])

	printf("fnmatch(\"*.a\", \"foo.a\", 0)  = %d\n", fnmatch("*.a", "foo.a", 0) )
	printf("fnmatch(\"*.a\", \"foo.c\", 0) = %d\n", fnmatch("*.a", "foo.c", 0))
}
