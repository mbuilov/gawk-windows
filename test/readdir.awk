@load "readdir"

BEGIN {
	readdir_do_ftype("stat")
}

{ print }
