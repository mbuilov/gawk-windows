# sidebar.awk --- add support for sidebars, other stuff to gawk.texi

# Copyright (C) 2013, 2016 the Free Software Foundation, Inc.
# 
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
# 
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

BEGIN {
	print "% ****************************************************"
	print "% * DO NOT MODIFY THIS FILE!!!!                      *"
	print "% * It was generated from gawktexi.in by sidebar.awk *"
	print "% * Edit gawktexi.in instead.                        *"
	print "% ****************************************************"
}

/^@sidebar/ {
	sub(/^@sidebar[ \t]+/, "", $0)
	title = $0
	body = ""
	collecting = 1
	next
}

/^@end[ \t]+sidebar[ \t]*$/ {
	collecting = 0
	printf "@cindex riquadro @subentry %s\n", title
	printf "@ifdocbook\n"
		printf "@docbook\n"
		printf "<sidebar><title>%s</title>\n", title
		printf "@end docbook\n"
			print  body
		print ""
		printf "@docbook\n"
		printf "</sidebar>\n"
		printf "@end docbook\n"
	printf "@end ifdocbook\n\n"

	printf "@ifnotdocbook\n"
		printf "@cartouche\n"
		printf "@center @b{%s}\n\n", title
			print  body
		printf "@end cartouche\n"
	printf "@end ifnotdocbook\n"
	body = ""
	next
}

collecting == 1 {
	body = body RS $0
	next
}

{ print }
