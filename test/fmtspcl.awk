function display(x, str,	i, res) {
	for (i = 0; i < n; i++) {
		if ((res = sprintf(formats[i],x)) != str)
			printf "sprintf(%s,%s) = %s (!= %s)\n",
			       formats[i], x, res, str
	}
}

BEGIN {
	nan = sqrt(-1)
	nan_str = sprintf("%f", nan)
	nnan_str = sprintf("%f", -nan)
	inf = -log(0)
	inf_str = sprintf("%f", inf)
	ninf_str = sprintf("%f", -inf)

	n = 0
	formats[n++] = "%a"
	formats[n++] = "%f"
	formats[n++] = "%g"
	formats[n++] = "%x"
	formats[n++] = "%d"
	formats[n++] = "%s"
	display(nan, nan_str)
	display(-nan, nnan_str)
	display(inf, inf_str)
	display(-inf, ninf_str)

	# Now test uppercase floating-point format strings
	for (j = 0; j < n; j++)
		formats[j] = toupper(formats[j])

	n -= 3		# interger and string formats don't count (%x, %d, %s)
	nan_str = toupper(nan_str)
	nnan_str = toupper(nnan_str)
	inf_str = toupper(inf_str)
	ninf_str = toupper(ninf_str)

	display(nan, nan_str)
	display(-nan, nnan_str)
	display(inf, inf_str)
	display(-inf, ninf_str)
}
