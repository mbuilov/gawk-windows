# cmp replacement program for PC where the error messages aren't
# exactly the same, and neither are e+NNN exponents.
# should run even on old awk
#
# Copyright (C) 2011-2015 the Free Software Foundation, Inc.
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

{
	if (FNR == NR)
		file = 0
	else
		file = 1
	gsub(/\r/, "", $0)
	lines[file, FNR] = $0
}

END {
	if (NR/2 != FNR) {
		printf("testoutcmp: warning: files are not of equal length!\n") > "/dev/stderr"
		exit 1
	}

	status = 0;
	for (i = 1; i <= FNR; i++) {
		good = lines[0, i]
		actual = lines[1, i]
		if (good == actual)
			continue

		l = length(good)
		if (substr(good, l, 1) == ")")
			l--
		if (substr(good, 1, l) == substr(actual, 1, l))
			continue

		# For exponents
		actual1 = gensub(/( ?)([-+]?[0-9.][0-9.]?+e[-+])0([0-9][0-9])/, " \\1\\2\\3", "g", actual)
		if (good == actual1)
		    continue
		actual1 = gensub(/([-+]?0)([0-9.]+e[-+])0([0-9][0-9])/, "\\10\\2\\3", "g", actual)
		if (good == actual1)
		    continue
		actual1 = gensub(/( ?)([-+]?)([1-9.][0-9.]?+e[-+])0([0-9][0-9])/, "\\1\\20\\3\\4", "g", actual)
		if (good == actual1)
		    continue
		actual1 = gensub(/([-+]?[0-9.]+e[-+])0([0-9][0-9])/, "\\1\\2 ", "g", actual)
		if (good == actual1)
		    continue
		actual1 = gensub(/([-+]?[0-9.]+e[-+])0([0-9][0-9])/, "\\1\\2", "g", actual)
		if (good == actual1)
		    continue
		# For exit test
		actual1 = gensub(/([01]) nul/, "\\1 null", "g", actual)
		if (good == actual1)
		    continue
		else {
			printf("-%s\n", good) > "/dev/stderr"
			printf("+%s\n", actual) > "/dev/stderr"
			status = 1
		}
	}

	exit status
}
