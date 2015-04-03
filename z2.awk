BEGIN {
	f = "foo"
	p = @/o/
//	gsub(p, "q", f)
	fun = "gsub"
	@fun(p, "q", f)
	print f
}
