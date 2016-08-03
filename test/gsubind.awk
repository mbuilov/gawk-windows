BEGIN {
	f = "foo"
#	p = @/o/
p = "o"
	gsub(p, "q", f)
	print f
#	fun = "gsub"
#	@fun(p, "q", f)
#	print f
}
