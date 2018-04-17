BEGIN {
	PROCINFO["NONFATAL"]
	# note the bad characters in the hostname
	print |& "/inet/tcp/0/local@#!%host/25"
	print (ERRNO != "")
}
