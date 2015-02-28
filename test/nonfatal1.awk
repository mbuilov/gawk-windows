BEGIN {
	PROCINFO["NONFATAL"]
	# note that ":" is not a valid hostname character
	print |& "/inet/tcp/0/local:host/25"
	print ERRNO
}
