# dbarray.awk -- persistent array with sqlite database backend

# @load "bindarr"

BEGIN {
	extension("./bindarr.so")
}

function _db_count(symbol, sq,
		sth, ret, count)
{
	sth = sq["sqlc"]
	printf "SELECT count(col1) FROM %s;\n", sq["table"] |& sth
	close(sth, "to")
	ret = (sth |& getline count)
	if (close(sth) != 0 || ret <= 0)
		return -1
	return count
}

function _db_exists(symbol, sq, subs,
		sth, ret, row, qsubs)
{
	if (! (subs in symbol)) {
		sth = sq["sqlc"]

		# double up single quotes
		qsubs = gensub(/'/, "''", "g", subs)

		printf "SELECT col2 FROM %s WHERE col1='%s';\n", sq["table"], qsubs |& sth
		close(sth, "to")
		ret = (sth |& getline row)
		if (close(sth) != 0 || ret < 0)
			return -1
		if (ret == 0)	# non-existent row
			return 0
		if (row == sq["null"])
			symbol[subs]	# install null string as value
		else
			symbol[subs] = row
	}
	return 0
}

function _db_lookup(symbol, sq, subs,
		sth, ret, row, qsubs)
{
	if (! (subs in symbol)) {
		sth = sq["sqlc"]

		# double up single quotes
		qsubs = gensub(/'/, "''", "g", subs)

		printf "SELECT col2 FROM %s WHERE col1='%s';\n", sq["table"], qsubs |& sth
		close(sth, "to")
		ret = (sth |& getline row)
		if (close(sth) != 0 || ret < 0)
			return -1

		if (ret > 0) {
			if (row == sq["null"])
				symbol[subs]	# install null string as value
			else
				symbol[subs] = row
		} else {
			# Not there, install it with NULL as value
			printf "INSERT INTO %s (col1) VALUES('%s');\n", sq["table"], qsubs |& sth
			close(sth, "to")
			ret = (sth |& getline)
			if (close(sth) != 0 || ret < 0)
				return -1
		}
	}
	return 0
}

function _db_clear(symbol, sq,
		sth, ret)
{
	sth = sq["sqlc"]
	printf "DELETE FROM %s;\n", sq["table"] |& sth
	close(sth, "to")
	ret = (sth |& getline)
	if (close(sth) != 0 || ret < 0)
		return -1
	return 0
}

function _db_delete(symbol, sq, subs,
		sth, ret, qsubs)
{
	sth = sq["sqlc"]
	qsubs = gensub(/'/, "''", "g", subs)
	printf "DELETE FROM %s WHERE col1='%s';\n", sq["table"], qsubs |& sth
	close(sth, "to")
	ret = (sth |& getline)
	if (close(sth) != 0 || ret < 0)
		return -1
	return 0
}

function _db_store(symbol, sq, subs,
		sth, ret, qsubs, qval)
{
	sth = sq["sqlc"]

	qval = gensub(/'/, "''", "g", symbol[subs])
	qsubs = gensub(/'/, "''", "g", subs)
	printf "UPDATE %s SET col2='%s' WHERE col1='%s';\n", \
			sq["table"], qval, qsubs |& sth
	close(sth, "to")
	ret = (sth |& getline)
	if (close(sth) != 0 || ret < 0)
		return -1
	return 0
}

function _db_fetchall(symbol, sq,
		sth, ret, save_RS, save_FS)
{
	sth = sq["sqlc2"]

	if (! sq["loaded"]) {
		printf "SELECT col1, col2 FROM %s;\n", sq["table"] |& sth
		close(sth, "to")
		save_RS = RS
		save_FS = FS
		RS = "\n\n"
		FS = "\n"
		while ((ret = (sth |& getline)) > 0) {
			sub(/^ *col1 = /, "", $1)
			sub(/^ *col2 = /, "", $2)
			if ($2 == sq["null"])
				symbol[$1]	# install null string as value
			else
				symbol[$1] = $2
		}
		RS = save_RS
		FS = save_FS
		if (ret < 0 || close(sth) != 0)
			return -1
		sq["loaded"] = 1
	}
}


function _db_init(symbol, sq,
		sth, table, ret)
{
	sth = sq["sqlc"]
	table = sq["table"]

	# check if table exists
	printf ".tables %s\n", table |& sth
	close(sth, "to")
	ret = (sth |& getline)
	if (close(sth) != 0 || ret < 0)
		return -1
	if (ret > 0 && $0 == table) {
		# verify schema
		printf ".schema %s\n", table |& sth
		close(sth, "to")
		ret = (sth |& getline)
		if (close(sth) != 0 || ret <= 0)
			return -1
		if ($0 !~ /\(col1 TEXT PRIMARY KEY, col2 TEXT\)/) {
			printf "table %s: Invalid column name or type(s)\n", table > "/dev/stderr"
			return -1
		}
	} else {
		# table does not exist, create it.
		printf "CREATE TABLE %s (col1 TEXT PRIMARY KEY, col2 TEXT);\n", table |& sth
		close(sth, "to")
		ret = (sth |& getline)
		if (close(sth) != 0 || ret < 0)
			return -1
	}
	return 0
}

#function _db_fini(tie, a, subs) {}

function db_bind(arr, database, table,		sq)
{
	if (! database) {
		print "db_bind: must specify a database name" > "/dev/stderr"
		exit(1)
	}

	if (! table) {
		print "db_bind: must specify a table name" > "/dev/stderr"
		exit(1)
	}

	# string used by the sqlite3 client to represent NULL
	sq["null"] = "(null)"

	sq["sqlc"] = sprintf("sqlite3 -nullvalue '%s' %s", sq["null"], database)
	# sqlite command used in _db_fetchall 
	sq["sqlc2"] = sprintf("sqlite3 -line -nullvalue '%s' %s", sq["null"], database)

	sq["table"] = table

	# register our array routines
	sq["init"] = "_db_init"
	sq["count"] = "_db_count"
	sq["exists"] = "_db_exists"
	sq["lookup"] = "_db_lookup"
	sq["delete"] = "_db_delete"
	sq["store"] = "_db_store"
	sq["clear"] = "_db_clear"
	sq["fetchall"] = "_db_fetchall"

#	sq["fini"] = "_db_fini";

	bind_array(arr, sq)
}

function db_unbind(arr)
{
	unbind_array(arr)
}
