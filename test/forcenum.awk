BEGIN {
	# first, make some strnums
	nf = split("|5apple|+NaN| 6|0x1az|011Q|027", f, "|")
	for (i = 1; i <= nf; i++) {
		x = f[i]+0	# trigger strnum conversion to number or string
		printf "[%s] -> %g (type %s)\n", f[i], f[i], typeof(f[i])
	}
}
