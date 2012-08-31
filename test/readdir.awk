@load "readdir"

BEGIN {
	readdir_do_ftype(2)
}

{ print }
