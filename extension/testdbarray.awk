@include "dbarray.awk"

# $ ../gawk -f testdbarray.awk
# $ ../gawk -f testdbarray.awk
# ...
# $ ../gawk -vINIT=1 -f testdbarray.awk


BEGIN {
	# bind array 'A' to the table 'table_A' in sqlite3 database 'testdb'
	db_bind(A, "testdb", "table_A")

	if (INIT)	# detele table and start over
		delete A

	lenA = length(A)
	A[++lenA] = strftime()
	PROCINFO["sorted_in"] = "@ind_num_asc"
	for (item in A)
		print item, ":", A[item]
}
