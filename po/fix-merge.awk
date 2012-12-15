#! /usr/bin/gawk -f

/^<<<<<<< HEAD/	{ next }
/^=======/, /^>>>>>>> / { next }

{ print }
