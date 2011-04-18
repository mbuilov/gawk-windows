BEGIN{
  for (IGNORECASE=0; IGNORECASE <= 1; IGNORECASE++) {
	SORT_STR = -1

 	makea(a)
 	asort1(a, "")
	printf("---end asort(a), IGNORECASE = %d---\n", IGNORECASE)

	makea(a)
	asort2(a, "")
	printf("---end asort(a, b), IGNORECASE = %d---\n", IGNORECASE)

	makea(a)
	SORT_STR = ""
	asort1(a)
	printf("---end asort(a, a), IGNORECASE = %d---\n", IGNORECASE)

	makea(a)
	SORT_STR = "num"
	asort2(a, "")
	printf("---end asort(a, b, \"num\"), IGNORECASE = %d---\n", IGNORECASE)

	makea(a)
	SORT_STR = "desc str"
	asort1(a, "")
	printf("---end asort(a, a, \"desc str\"), IGNORECASE = %d---\n", IGNORECASE)

	makea(a)
	SORT_STR = "val str"
	proc_sort(a, "")
	printf("---end PROCINFO[\"sorted_in\"] = \"val str\", IGNORECASE = %d---\n",
		IGNORECASE)

	makea(a)
	SORT_STR = "val num"
	proc_sort(a, "")
	printf("---end PROCINFO[\"sorted_in\"] = \"val num\", IGNORECASE = %d---\n",
		IGNORECASE)

	makea(a)
	SORT_STR = "desc val str"
	proc_sort(a, "")
	printf("---end PROCINFO[\"sorted_in\"] = \"desc val str\", IGNORECASE = %d---\n",
		IGNORECASE)

	makea(a)
	SORT_STR = "desc val num"
	proc_sort(a, "")
	printf("---end PROCINFO[\"sorted_in\"] = \"desc val num\", IGNORECASE = %d---\n",
		IGNORECASE)
  }
}

function makea(aa)
{
  delete aa
  aa[1] = "barz";
  aa[2] = "blattt";
  aa[3] = "Zebra";
  aa[4] = 1234;
  aa[5] = 234;
  aa[6][1] = 4321;
#  aa[6][2] = 432;
  aa[6][3] = "tttalb";
  aa[6][2] = "arbeZ";
  aa[6][4] = "zrab";
}


# source array != destination array 
function asort2(c, s,	d, k, m) 
{
  if (SORT_STR < 0)
	m = asort(c, d);
  else
	m = asort(c, d, SORT_STR);
  for(k=1; k <= m; k++) {
	if (isarray(d[k]))
		asort2(d[k], s"["k"]")
	else
	    	printf("%10s:%10s%10s\n", s"["k"]", c[k], d[k])
  }
}

# source array == destination array
function asort1(c, s,	k, m) 
{
  if (SORT_STR < 0)
	m = asort(c)
  else if (SORT_STR != "")
	m = asort(c, c, SORT_STR)
  else
	m = asort(c, c);

  for(k=1; k <= m; k++) {
	if (isarray(c[k]))
		asort1(c[k], s"["k"]")
	else
	    	printf("%10s:%10s\n", s"["k"]", c[k])
  }
}


function proc_sort(c, s,	k) 
{
  PROCINFO["sorted_in"] = SORT_STR
  for(k in c) {
	if (isarray(c[k]))
		proc_sort(c[k], s"["k"]")
	else
	    	printf("%10s:%10s\n", s"["k"]", c[k])
  }
}
