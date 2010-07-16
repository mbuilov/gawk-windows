function abs(x) {
	return (x+0 >= 0) ? x : -x
}

function check(x,what,  f,res) {
	for (f in formats) {
		res = sprintf(f,x)
		if (formats[f] == "non-decimal") {
			if ((x >= 0) && (res !~ /e+/)) {
				if (abs(strtonum(res)-x) > 1e-5*abs(x))
				       printf "(sprintf(%s,%s) = %s)-%g = %g\n",
					      f,what,res,x,strtonum(res)-x
			}
		}
		else if (abs(res-x) > 1e-5*abs(x))
			printf "(sprintf(%s,%s) = %s)-%g = %g\n",
			       f,what,res,x,res-x
	}
}

function check_cons(fmt,base,rot,mexp,  i,j,dig,res,s) {
	# first off, check that zero formats properly
	if ((s = sprintf(fmt,0)) != "0")
		printf "(sprintf(%s,0) = %s) != 0\n",fmt,s

	res = "1"
	dig = 1
	j = 0
	for (i = 0; i <= mexp; i++) {
		s = sprintf(fmt,base^i)
		if (s ~ /e+/)
			return
		if (s != res)
			printf "(sprintf(%s,%d^%d) = %s) != %s\n",
			       fmt,base,i,s,res
		if (++j == rot) {
			dig = 1
			res = ("10"substr(res,2))
			j = 0
		}
		else {
			dig *= 2
			res = (dig substr(res,2))
		}
	}
}

BEGIN {
	formats["%s"] = ""
	formats["%d"] = ""
	formats["%.0f"] = ""
	formats["0%o"] = "non-decimal"
	formats["0x%x"] = "non-decimal"

	check(0,"0")
	for (i = 0; i <= 308; i++) {
		check(10^i,"10^"i)
		check(-10^i,"-10^"i)
	}
	for (i = 0; i <= 1023; i++) {
		check(2^i,"2^"i)
		check(-2^i,"-2^"i)
	}

	check_cons("%d",10,1,9)
	check_cons("%x",2,4,31)
	check_cons("%o",2,3,31)

	# make sure basic %d and %x are working properly
	printf "%d %d %x\n",3.7,-3.7,23.7

	# check another problem in gawk 3.1.5: precision over 30 crashes
	printf "%.55d\n",1
}
