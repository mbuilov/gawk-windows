BEGIN {
	# make some strnums
	nf = split("|5apple|+NaN| 6|0x1az|011Q|027", f, "|")
	for (i = 1; i <= nf; i++)
		printf "[%s] -> %g (type %s)\n", f[i], f[i], typeof(f[i])
}
