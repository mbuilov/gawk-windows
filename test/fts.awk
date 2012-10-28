@load "filefuncs"

BEGIN {
	Level = 0

	system("rm -fr d1 d2")
	system("mkdir d1 d2 ; touch d1/f1 d1/f2 d2/f1 d2/f2")
	pathlist[1] = "d1"
	pathlist[2] = "d2"
	flags = FTS_PHYSICAL
	fts(pathlist, flags, data)

	output = "fts.ok"
	traverse(data)
	close(output)

	ftswalk(pathlist, data2)
	output = "_fts"
	traverse(data2)
	close(output)

	system("rm -fr d1 d2")
}

function indent(        i)
{
	for (i = 1; i <= Level; i++)
		printf("\t") > output
}

function sort_traverse(data,	sorted, i)
{
	asorti(data, sorted)
	for (i = 1; i in sorted; i++) {
		indent()
		printf("%s --> %s\n", sorted[i], data[sorted[i]]) > output
	}
}

function traverse(data,         i)
{
	for (i in data) {
		if (isarray(data[i])) {
			indent()
			printf("%s:\n", i) > output

			Level++
			if (("mtime" in data[i]) && ! isarray(data[i][mtime])) {
				sort_traverse(data[i])
			} else {
				traverse(data[i])
			}
			Level--
		} else {
			indent()
			printf("%s --> %s\n", i, data[i]) > output
		}
	}
}


function ftswalk(pathlist, data,	i, toppath)
{
	delete data
	for (i = 1; i in pathlist; i++) {
		toppath = pathlist[i]
		data[toppath]["junk"]++	# create array
		delete data[toppath]["junk"]
		process(pathlist[i], data)
	}
}

# enter process with pathname, array for that path already created but
# empty

function process(pathname, data_array,
		stat_data, i, direntry, command, shortname)	# locals
{
	stat(pathname, stat_data)
	if (stat_data["type"] == "file") {
		shortname = strrstr(pathname, "/")
		data_array["path"] = pathname
		for (i in stat_data) {
			if (i == "name")
				data_array["stat"][i] = shortname
			else
				data_array["stat"][i] = stat_data[i]
		}

		return
	}

	# stuff for a directory

	data_array[pathname]["."]["path"] = pathname
	for (i in stat_data)
		data_array[pathname]["."]["stat"][i] = stat_data[i]

	command = ("ls -f " pathname)
	while ((command | getline direntry) > 0) {
		if (direntry == "." || direntry == "..")
			continue
		data_array[pathname][direntry]["junk"]++
		delete data_array[pathname][direntry]["junk"]
		process(pathname "/" direntry,
			data_array[pathname][direntry])
	}
	close(command)
}

function strrstr(string, delim,		ind)
{
	if ((ind = index(string, delim)) == 0)
		return string

	do {
		string = substr(string, ind + 1)
	} while ((ind = index(string, delim)) > 0)

	return string
}
