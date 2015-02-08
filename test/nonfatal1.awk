BEGIN {
	PROCINFO["nonfatal"]
	print |& "/inet/tcp/0/ti10/357"
	print ERRNO
}
