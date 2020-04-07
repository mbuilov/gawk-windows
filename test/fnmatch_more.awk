@load "fnmatch"

BEGIN {
	printf("fnmatch(\"123[x\\]]456\", \"123]456\", 0)  = %d\n", fnmatch("123[x\\]]456", "123]456", 0) )
	printf("fnmatch(\"123[]x]456\", \"123]456\", 0)  = %d\n", fnmatch("123[]x]456", "123]456", 0) )
	printf("fnmatch(\"123?456\", \"123]456\", 0)  = %d\n", fnmatch("123?456", "123]456", 0) )
	printf("fnmatch(\"123[]]456\", \"123]456\", 0)  = %d\n", fnmatch("123[]]456", "123]456", 0) )
	printf("fnmatch(\"123[\\]]456\", \"123]456\", 0)  = %d\n", fnmatch("123[\\]]456", "123]456", 0) )

	pat = "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z"
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy", 0))
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyzz\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyzz", 0))
	printf("fnmatch(\"" pat "\", \"aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz\", 0)  = %d\n",
		fnmatch(pat, "aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz", 0))

	pat = "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*z*"
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy", 0))
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyzz\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz", 0))
	printf("fnmatch(\"" pat "\", \"aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz\", 0)  = %d\n",
		fnmatch(pat, "aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz", 0))

	pat = "a*b*c*d*e*f*g*h*i*j*k*l*m*n*o*p*q*r*s*t*u*v*w*x*y*"
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyy", 0))
	printf("fnmatch(\"" pat "\", \"aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyzz\", 0)  = %d\n",
		fnmatch(pat, "aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz", 0))
	printf("fnmatch(\"" pat "\", \"aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz\", 0)  = %d\n",
		fnmatch(pat, "aaaaccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqqrrrrssssttttuuuuvvvvwwwwxxxxyyyyz", 0))

	printf("fnmatch(\"a*b/*\", \"abbb/.x\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/*", "abbb/.x", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*\", \"abbb/.x\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*", "abbb/.x", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*/*\", \"abbb/.x/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*/*", "abbb/.x/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*/.*\", \"abbb/.x/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*/.*", "abbb/.x/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*m/*\", \"abbb/.x/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*m/*", "abbb/.x/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*x/*\", \"abbb/.x/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*x/*", "abbb/.x/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*x/*\", \"abbb/.xyx/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*x/*", "abbb/.xyx/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*x/.*\", \"abbb/.x/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*x/.*", "abbb/.x/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/.*x/.*\", \"abbb/.xyx/.q\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/.*x/.*", "abbb/.xyx/.q", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"a*b/*\", \"abbb/xy\", PATHNAME | PERIOD)  = %d\n", fnmatch("a*b/*", "abbb/xy", or(FNM["PATHNAME"], FNM["PERIOD"])) )
	printf("fnmatch(\"[\", \"[\", 0)  = %d\n", fnmatch("[", "[", 0) )

	printf("fnmatch(\"[[:alpha:]'[:alpha:]\", \"a\", 0)  = %d\n", fnmatch("[[:alpha:]'[:alpha:]", "a", 0) )
}
