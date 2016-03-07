#! /usr/bin/gawk -f

BEGIN {
	address_re = "0x[[:xdigit:]]+"
	bracketed_address =  "\\[(([[:space:]]*[[:digit:]]*:)?|[[:alpha:]_]+ = )0x[[:xdigit:]]+\\]"
}

{
	line[NR] = $0
	extract_addresses($0, NR)
}

END {
	for (i = 1; i <= NR; i++) {
		if (line[i] !~ address_re) {
			print line[i]
			continue
		}

		translate(line[i])
	}
}

# Global arrays
#
# Address[line]		--- Address of instruction - first hex number
# Target[address] = 1 	--- Address is target of a jump
# Newaddr[address]	--- Replacement address, counting from 1

function extract_addresses(line, num,	data, i, n, seps, addr)
{
	if (line !~ address_re)
		return

	split(line, data, bracketed_address, seps)
	n = length(seps)

	for (i = 1; i <= n; i++) {
		addr = gensub(".*(" address_re ").*", "\\1", 1, seps[i])
		if (i == 1)
			Address[num] = addr
		else {
			Target[addr]++
			if (! (addr in Newaddr))
				Newaddr[addr] = new_address()
		}
	}
}

function new_address()
{
	return sprintf("%8d", ++Address_seed)
}

function translate(line,	n, data, seps, i, newline)
{
	split(line, data, address_re, seps)
	n = length(seps)
	newline = line
	for (i = 1; i <= n; i++) {
		if (! (seps[i] in Target)) {
			gsub(seps[i], "        ", newline)
			continue
		}
		gsub(seps[i], Newaddr[seps[i]], newline)
	}

	gsub(/ =  +/, " = ", newline)

	print newline
}
