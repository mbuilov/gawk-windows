# strftime.awk ; test the strftime code
#
# input is the output of `date', see Makefile.in
#
# The mucking about with $0 and $NF is to avoid problems
# on cygwin, where the timezone field is empty and there
# are two consecutive blanks.

BEGIN {
	maxtries = 10
	datecmd = "date"
	fmt = "%a %b %d %H:%M:%S %Z %Y"

	# loop until before equals after, thereby protecting
	# against a race condition where the seconds field might have
	# incremented between running date and strftime
	i = 0
	while (1) {
		if (++i > maxtries) {
			printf "Warning: this system is so slow that after %d attempts, we could never get two sequential invocations of strftime to give the same result!\n", maxtries > "/dev/stderr"
			break
		}
		before = strftime(fmt)
		datecmd | getline sd
		after = strftime(fmt)
		close(datecmd)
		if (before == after) {
			if (i > 1)
				printf "Notice: it took %d loops to get the before and after strftime values to match\n", i > "/dev/stderr"
			break
		}
	}
	print sd > "strftime.ok"
	$0 = after
	$NF = $NF
	print > OUTPUT
}
