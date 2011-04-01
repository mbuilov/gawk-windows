# Date: Thu, 31 Mar 2011 12:29:09 -0600
# From: j.eh@mchsi.com

BEGIN { a[100]=a[1]=a["x"]=a["y"]=1; PROCINFO["sorted_in"]="num";
for (i in a) print i, a[i] }

BEGIN { a[100]=a[1]=a["x"]=1; PROCINFO["sorted_in"]="num";
for (i in a) print i, a[i] }

BEGIN { a[0]=a[100]=a[1]=a["x"]=1;  PROCINFO["sorted_in"]="num";
for (i in a) print i, a[i] }
