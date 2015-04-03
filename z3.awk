function testit(count, re, repl,	fun, bi, n1, n2)
{
	$0 = "foo"
	n1 = gsub(re, repl)
	bi = $0

	$0 = "foo"
	fun = "gsub"
	n2 = @fun(re, repl)
	printf("%d: n1 = %d, bi -> %s, n2 = %d, indirect -> %s\n",
		count, n1, bi, n2, $0)
}

BEGIN {
	testit(1, @/o/, "q")
	p = @/o/
stopme()
	testit(2, p, "q")
}
