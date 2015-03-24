function print_result(category, fname, builtin_result, indirect_result)
{
	if (builtin_result == indirect_result)
		printf("%s: %s: pass\n", category, fname)
	else
		printf("%s: %s: fail: builtin: %s \tindirect: %s\n", category, fname,
				builtin_result, indirect_result)
}

BEGIN {
	fun = "sub"
	$0 = "ff11bb"
	b1 = sub("f", "q")
	$0 = "ff11bb"
	i1 = @fun("f", "q")
	print_result("string", fun, b1, i1)
	exit
}


BEGIN {
# math functions

	fun = "and"
	b1 = and(0x11, 0x01)
	i1 = @fun(0x11, 0x01)
	print_result("math", fun, b1, i1)

	fun = "atan2"
	b1 = atan2(-1, 0)
	i1 = @fun(-1, 0)
	print_result("math", fun, b1, i1)

	fun = "compl"
	b1 = compl(0x1111)
	i1 = @fun(0x1111)
	print_result("math", fun, b1, i1)

	fun = "cos"
	b1 = cos(3.1415927 / 4)
	i1 = @fun(3.1415927 / 4)
	print_result("math", fun, b1, i1)

	fun = "exp"
	b1 = exp(2)
	i1 = @fun(2)
	print_result("math", fun, b1, i1)

	fun = "int"
	b1 = int(3.1415927)
	i1 = @fun(3.1415927)
	print_result("math", fun, b1, i1)

	fun = "log"
	b1 = log(10)
	i1 = @fun(10)
	print_result("math", fun, b1, i1)

	fun = "lshift"
	b1 = lshift(1, 2)
	i1 = @fun(1, 2)
	print_result("math", fun, b1, i1)

	fun = "or"
	b1 = or(0x10, 0x01)
	i1 = @fun(0x10, 0x01)
	print_result("math", fun, b1, i1)

	fun = "rand"
	srand(1)
	b1 = rand();
	srand(1)
	i1 = @fun()
	print_result("math", fun, b1, i1)

	fun = "rshift"
	b1 = rshift(0x10, 1)
	i1 = @fun(0x10, 1)
	print_result("math", fun, b1, i1)

	fun = "sin"
	b1 = sin(3.1415927 / 4)
	i1 = @fun(3.1415927 / 4)
	print_result("math", fun, b1, i1)

	fun = "sqrt"
	b1 = sqrt(2)
	i1 = @fun(2)
	print_result("math", fun, b1, i1)

	srand()
	fun = "srand"
	b1 = srand()
	i1 = @fun()
	print_result("math", fun, b1, i1)

	fun = "xor"
	b1 = xor(0x11, 0x01)
	i1 = @fun(0x11, 0x01)
	print_result("math", fun, b1, i1)

# string functions

#	fun = "gensub"
#	b1 = gensub("f", "q","g", "ff11bb")
#	i1 = @fun("f", "q", "g", "ff11bb")
#	print_result("string", fun, b1, i1)

#	fun = "gsub"
#	x = "ff11bb"
#	b1 = gsub("f", "q", x)
#	i1 = @fun("f", "q", x)
#	print_result("string", fun, b1, i1)

	fun = "index"
	b1 = index("hi, how are you", "how")
	i1 = @fun("hi, how are you", "how")
	print_result("string", fun, b1, i1)

	fun = "dcgettext"
	b1 = dcgettext("hello, world")
	i1 = @fun("hello, world")
	print_result("string", fun, b1, i1)

	fun = "dcngettext"
	b1 = dcngettext("hello, world", "howdy", 2)
	i1 = @fun("hello, world", "howdy", 2)
	print_result("string", fun, b1, i1)

	fun = "length"
	b1 = length("hi, how are you")
	i1 = @fun("hi, how are you")
	print_result("string", fun, b1, i1)

	fun = "sprintf"
	b1 = sprintf("%s world", "hello")
	i1 = @fun("%s world", "hello")
	print_result("string", fun, b1, i1)

	fun = "strtonum"
	b1 = strtonum("0xdeadbeef")
	i1 = @fun("0xdeadbeef")
	print_result("string", fun, b1, i1)

	fun = "sub"
	x = "ff11bb"
	b1 = sub("f", "q", x)
	i1 = @fun("f", "q", x)
	print_result("string", fun, b1, i1)

	fun = "substr"
	b1 = substr("0xdeadbeef", 7, 4)
	i1 = @fun("0xdeadbeef", 7, 4)
	print_result("string", fun, b1, i1)

	fun = "tolower"
	b1 = tolower("0xDeAdBeEf")
	i1 = @fun("0xDeAdBeEf")
	print_result("string", fun, b1, i1)

	fun = "toupper"
	b1 = toupper("0xDeAdBeEf")
	i1 = @fun("0xDeAdBeEf")
	print_result("string", fun, b1, i1)

# time functions

	fun = "mktime"
	b1 = mktime("1990 02 11 12 00 00")
	i1 = @fun("1990 02 11 12 00 00")
	print_result("time", fun, b1, i1)

	then = b1
	fun = "strftime"
	b1 = strftime(PROCINFO["strftime"], then)
	i1 = @fun(PROCINFO["strftime"], then)
	print_result("time", fun, b1, i1)

	fun = "systime"
	b1 = systime()
	i1 = @fun()
	print_result("time", fun, b1, i1)

# regexp functions

#	fun = "match"
#	print_result("regexp", fun, b1, i1)

#	fun = "patsplit"
#	print_result("regexp", fun, b1, i1)

#	fun = "split"
#	print_result("regexp", fun, b1, i1)

# array functions

	split("z y x w v u t", data)
	fun = "asort"
	asort(data, newdata)
	@fun(data, newdata2)
	print_result("array", fun, b1, i1)
	for (i in newdata) {
		if (! (i in newdata2) || newdata[i] != newdata2[i]) {
			print fun ": failed, index", i
			exit
		}
	}

	for (i in data)
		data2[data[i]] = i

	fun = "asorti"
	asort(data2, newdata)
	@fun(data2, newdata2)
	print_result("array", fun, b1, i1)
	for (i in newdata) {
		if (! (i in newdata2) || newdata[i] != newdata2[i]) {
			print fun ": failed, index", i
			exit
		}
	}

	arr[1] = arr[2] = 42
	fun = "isarray"
	b1 = isarray(arr)
	i1 = @fun(arr)
	print_result("array", fun, b1, i1)

# i/o functions

	print("hi") > "x1.out"
	print("hi") > "x2.out"

	fun = "fflush"
	b1 = fflush("x1.out")
	i1 = @fun("x2.out")
	print_result("i/o", fun, b1, i1)

	fun = "close"
	b1 = close("x1.out")
	i1 = @fun("x2.out")
	print_result("i/o", fun, b1, i1)

	fun = "system"
	b1 = system("rm x1.out")
	i1 = @fun("rm x2.out")
	print_result("i/o", fun, b1, i1)
}
