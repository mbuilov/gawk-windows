@load "fnmatch"

BEGIN {
	printf("fnmatch(\"[\\]xyz]\", \"x\", 0)  = %d\n", fnmatch("[\\]xyz]", "x", 0) )
	printf("fnmatch(\"[\\]-_]]\", \"^\", 0)  = %d\n", fnmatch("[\\]-_]", "^", 0) )
	if (fnmatch("[[:alpha:]-?]", "=", 0) == FNM_NOMATCH)
		printf("fnmatch(\"[[:alpha:]-?]\", \"=\", 0)  = FNM_NOMATCH\n")
	else
		printf("fnmatch(\"[[:alpha:]-?]\", \"=\", 0)  = 0\n")
	if (fnmatch("[A-Z]", "^", FNM["CASEFOLD"]) == FNM_NOMATCH)
		printf("fnmatch(\"[A-Z]\", \"^\", FNM[\"CASEFOLD\"])  = FNM_NOMATCH\n")
	else
		printf("fnmatch(\"[A-Z]\", \"^\", FNM[\"CASEFOLD\"])  = 0\n")
	printf("fnmatch(\"X[a[:digit:]]Z\", \"XaZ\", 0)  = %d\n", fnmatch("X[a[:digit:]]Z", "XaZ", 0) )
	if (fnmatch("[z-a]", "z", 0) == FNM_NOMATCH)
		printf("fnmatch(\"[z-a]\", \"z\", 0)  = FNM_NOMATCH\n")
	else
		printf("fnmatch(\"[z-a]\", \"z\", 0)  = 0\n")
	printf("fnmatch(\"a[b\", \"a[b\", 0)  = %d\n", fnmatch("a[b", "a[b", 0) )
}
