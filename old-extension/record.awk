# record.awk -- represent fixed-length records in a file as an array.
# Each element in the array corresponds to a record in the file.
# The records are numbered starting from 1, and each record read in
# from the file is cached. If opened using mode "r+",
# changes to the array are reflected in the file immediately i.e.
# writing to an element writes the data into the file.
#
# Usage:
# 	record(r, path [, reclen [, mode]])
#		r      -- array to bind
#		path   -- filename
#		reclen -- length of each record 
#		mode   -- "r" for reading (default), "r+" for reading and writing
#
#	With reclen <= 0, entire file is treated as one record #1.
#
#		record(r, "data.in", 80, "r+")
#		r[10] = r[1]
#		for (i = 1; i in r; i++)
#			print r[i]	
#		delete r[1]
#
# See Also: testrecord.sh
#
#
# TODO:
#	* implement deferred writing
#	* limit memory usage for read cache
#	* use fixed size buffer when deleting a record
#

BEGIN {
	extension("./fileop.so")
	extension("./bindarr.so")
}

# _record_count --- return the number of records in file

function _record_count(symbol, rd)
{
	if (! ("rectot" in rd))
		rd["rectot"] = ("reclen" in rd) ?
				int(filesize(rd["path"]) / rd["reclen"]) : 1
	return rd["rectot"]
}

# _record_exists --- check if record exists

function _record_exists(symbol, rd, recnum,
		path, mode, reclen, rectot)
{
	path = rd["path"]
	reclen = ("reclen" in rd) ? rd["reclen"] : filesize(path)
	mode = rd["mode"]
	rectot = _record_count(symbol, rd)

	recnum = int(recnum)
	if (recnum <= 0 || recnum > rectot)
		return 0

	if (! (recnum in symbol)) {
		fseek(path, mode, (recnum - 1) * reclen, "SEEK_SET")
		symbol[recnum] = fread(path, mode, reclen)
	}
	return 0
}

# _record_lookup --- lookup a record

function _record_lookup(symbol, rd, recnum,
		path, mode, reclen, rectot)
{
	path = rd["path"]
	reclen = ("reclen" in rd) ? rd["reclen"] : filesize(path)
	mode = rd["mode"]
	rectot = _record_count(symbol, rd)

	recnum = int(recnum)
	if (recnum <= 0 || recnum > rectot) {
		ERRNO = sprintf("record: %s: reference to non-existent record #%d", path, recnum)
		return -1
	}

	if (! (recnum in symbol)) {
		fseek(path, mode, (recnum - 1) * reclen, "SEEK_SET")
		symbol[recnum] = fread(path, mode, reclen)
	}
	return 0
}

# _record_clear --- remove all records

function _record_clear(symbol, rd,
		path, mode)
{
	path = rd["path"]
	mode = rd["mode"]
	if (mode == "r") {
		ERRNO = sprintf("record: cannot delete record from file `%s' opened only for reading", path)
		return -1
	}
	ftruncate(path, mode, 0)
	delete rd["reclen"]
	return 0
}

# _record_delete --- delete a record from the file

function _record_delete(symbol, rd, recnum,
		path, mode, reclen, rectot)
{
	path = rd["path"]
	reclen = ("reclen" in rd) ? rd["reclen"] : filesize(path)
	mode = rd["mode"]

	if (mode == "r") {
		ERRNO = sprintf("record: cannot delete record from file `%s' opened only for reading", path)
		return -1
	}

	recnum = int(recnum)
	if (! ("reclen" in rd)) {
		# entire file is record #1
		ftruncate(path, mode, 0)
		delete rd["reclen"]
		return 0
	}

	sz = filesize(path)
	rectot = int(sz / reclen)

	recstart = (recnum - 1) * reclen
	off = sz - (recstart + reclen)

	fseek(path, mode, -off, "SEEK_END")
	tmp = fread(path, mode, off)
	fseek(path, mode, recstart, "SEEK_SET")
	if (fwrite(path, mode, tmp) != length(tmp))
		return -1
	flush(path, mode)
	ftruncate(path, mode, sz - reclen)

	rd["rectot"] = rectot - 1
	for (i = recnum + 1; i <= rectot; i++) {
		if (i in symbol) {
			symbol[i - 1] = symbol[i]
			delete symbol[i]
		}
	}
	return 0
}

# _record_store --- write a record to file

function _record_store(symbol, rd, recnum,
		path, mode, reclen, val)
{
	path = rd["path"]
	reclen = ("reclen" in rd) ? rd["reclen"] : filesize(path)
	mode = rd["mode"]

	if (mode == "r") {
		ERRNO = sprintf("record: cannot write to file `%s' opened only for reading", path)
		return -1
	}

	recnum = int(recnum)
	val = symbol[recnum]
	if (! ("reclen" in rd)) {
		# the entire file is record #1
		if (reclen != 0)
			ftruncate(path, mode, 0)
	} else if (length(val) != reclen) {
		ERRNO = sprintf("record: %s: invalid length for record #%d", path, recnum)
		return -1
	}

	fseek(path, mode, (recnum - 1) * reclen, "SEEK_SET")
	if (fwrite(path, mode, val) != length(val))
		return -1
	flush(path, mode)
	return 0
}

# _record_fetchall --- retrieve all the records

function _record_fetchall(symbol, rd,
		path, mode, reclen, rectot, recnum)
{
	path = rd["path"]
	reclen = ("reclen" in rd) ? rd["reclen"] : filesize(path)
	mode = rd["mode"]
	rectot = _record_count(symbol, rd)

	if (rd["loaded"])
		return 0
	for (recnum = 1; recnum <= rectot; recnum++) {
		if (! (recnum in symbol)) {
			fseek(path, mode, (recnum - 1) * reclen, "SEEK_SET")
			symbol[recnum] = fread(path, mode, reclen)
		}
	}
	rd["loaded"] = 1
	return 0
}

# _record_init --- initialization routine

function _record_init(symbol, rd)
{
	if (! file_exists(rd["path"])) {
		ERRNO = sprintf("record: cannot open file `%s' for reading", rd["path"])
		return -1
	}
	return 0
}

# _record_fini --- cleanup routine

function _record_fini(symbol, rd)
{
	fclose(rd["path"], rd["mode"])
}

# record --- bind an array to a file with fixed-length records

function record(array, path, reclen, mode,	rd)
{
	if (path == "") {
		print "fatal: record: empty string value for filename" > "/dev/stderr"
		exit(1)
	}

	# register our array routines
	rd["init"] = "_record_init"
	rd["fini"] = "_record_fini"
	rd["count"] = "_record_count"
	rd["exists"] = "_record_exists"
	rd["lookup"] = "_record_lookup"
	rd["delete"] = "_record_delete"
	rd["store"] = "_record_store"
	rd["clear"] = "_record_clear"
	rd["fetchall"] = "_record_fetchall"

	rd["path"] = path
	if (reclen > 0)
		rd["reclen"] = reclen
	rd["mode"] = mode == "r+" ? "r+" : "r"

	delete array
	bind_array(array, rd)
}
