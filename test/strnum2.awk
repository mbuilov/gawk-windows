BEGIN {
	split("1.234", f)
	OFMT = "%.1f"
	# check whether strnum is displayed the same way before and
	# after force_number is called. Also, should numeric strings
	# be formatted with OFMT or show the original string value?
	print f[1]
	x = f[1]+0	# trigger conversion to NUMBER
	print f[1]
}
