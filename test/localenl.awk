# this file was generated from the localenl.sh by the following sed:
# sed -n -e '/BEGIN/!d' -e ':b' -e '/^'"'"'/q' -e 'p;n;b b' localenl.sh
BEGIN { RS="\0"; }
{ 
  if (match($0, /\n[^2\n]*2/)) { got2=1; } else { print "no match 2"; }
  if (match($0, /\n[^3\n]*3/)) { got3=1; } else { print "no match 3"; }
  if (match($0, /\n[^4\n]*4/)) { got4=1; } else { print "no match 4"; }
  if (match($0, /\n[^5\t]*5/)) { got5=1; } else { print "no match 5"; }
  if (match($0, /\n[^6\n]*6/)) { got6=1; } else { print "no match 6"; }
  if (match($0, /\n[a-z]*7\n/)){ got7=1; } else { print "no match 7"; }
  if (match($0, /\n[^8\n]*8/)) { got8=1; } else { print "no match 8"; }
  if (match($0, /8.[^9\n]+9/)) { got9=1; } else { print "no match 9"; }
}

END { exit(!(got2 && got3 && got4 && got5 && got6 && got7 && got8 && got9)); }
