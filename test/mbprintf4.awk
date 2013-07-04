# printf with multi-byte text encoding, %c and %s, width and precision, and left-alignment.
{
	print "printf %c " $0
	printf "|%c|\n", $0
	printf "|%1c|\n", $0
	printf "|%3c|\n", $0
	# precision is ignored by %c.
	printf "|%3.1c|\n", $0
	printf "|%3.5c|\n", $0
	print "printf %-c " $0
	printf "|%-c|\n", $0
	printf "|%-1c|\n", $0
	printf "|%-3c|\n", $0
	# precision is ignored by %c.
	printf "|%-3.1c|\n", $0
	printf "|%-3.5c|\n", $0
	printf ORS

	print "printf %s " $0
	printf "|%s|\n", $0
	printf "|%1s|\n", $0
	printf "|%3s|\n", $0
	printf "|%3.1s|\n", $0
	printf "|%3.5s|\n", $0
	print "printf %-s " $0
	printf "|%-s|\n", $0
	printf "|%-1s|\n", $0
	printf "|%-3s|\n", $0
	printf "|%-3.1s|\n", $0
	printf "|%-3.5s|\n", $0
	printf ORS ORS
}
