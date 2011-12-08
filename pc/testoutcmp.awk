# cmp replacement program for PC where the error messages aren't
# exactly the same.  should run even on old awk

{
	if (FNR == NR)
		file = 0
	else
		file = 1
	gsub(/\r/, "", $0)
	lines[file, FNR] = $0
}

END {
	if (NR/2 != FNR) {
		printf("testoutcmp: warning: files are not of equal length!\n") > "/dev/stderr"
		exit 1
	}

	for (i = 1; i <= FNR; i++) {
		good = lines[0, i]
		actual = lines[1, i]
		if (good == actual)
			continue

		l = length(good)
		if (substr(good, l, 1) == ")")
			l--
		if (substr(good, 1, l) == substr(actual, 1, l))
			continue
		else {
			printf("%s and %s are not equal\n", ARGV[1],
				ARGV[2]) > "/dev/stderr"
			exit 1
		}
	}

	exit 0
}
