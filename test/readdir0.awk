{
	ino = $1
	name = $NF
	type = substr($2, 1, 1)
	if (type == "-")
		type = "f"

	printf "%s/%s/%s\n", ino, name, type
}
