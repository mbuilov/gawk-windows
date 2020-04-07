@load "fnmatch"

BEGIN {
	printf("fnmatch(\"*.Й\", \"фЫв.й\", FNM[\"CASEFOLD\"])  = %d\n", fnmatch("*.Й", "фЫв.й", FNM["CASEFOLD"]) )
	if (fnmatch("*.Й", "фЫв.ц", FNM["CASEFOLD"]) == FNM_NOMATCH)
		printf("fnmatch(\"*.Й\", \"фЫв.ц\", FNM[\"CASEFOLD\"]) == FNM_NOMATCH\n")
}
