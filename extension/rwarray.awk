BEGIN {
	extension("./rwarray.so","dlload")

	while ((getline word < "/usr/share/dict/words") > 0)
		dict[word] = word word

	for (i in dict)
		printf("dict[%s] = %s\n", i, dict[i]) > "orig.out"
	close("orig.out");

	writea("orig.bin", dict)

	reada("orig.bin", dict)

	for (i in dict)
		printf("dict[%s] = %s\n", i, dict[i]) > "new.out"
	close("new.out");

	ret = system("cmp orig.out new.out")

	if (ret == 0)
		print "old and new are equal - GOOD"
	else
		print "old and new are not equal - BAD"

	if (ret == 0 && !("keepit" in ENVIRON))
		system("rm orig.bin orig.out new.out")
}
