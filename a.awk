BEGIN {
	f = "foo"
	p = "o+"
	fun = "match"
	@fun(f, p)
	print RSTART, RLENGTH
}
