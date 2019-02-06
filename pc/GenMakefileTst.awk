#!/usr/bin/gawk -f

# This program generates pc/Makefile.tst

# Skip the first part of Makefile.in
/Get rid of core files/ { process = 1 }

! process	{ next }

# Skip stuff that PC doesn't need
/^CMP =/, /^\.PRECIOUS/	{ next }

# Process the bits in Makefile.in we need to generate the tests properly

# Tests that fail on DJGPP
/^EXPECTED_FAIL_DJGPP *=/,/[^\\]$/ {
	print
	gsub(/(^EXPECTED_FAIL_DJGPP *=|\\$)/,"")
	for (i = 1; i <= NF; i++)
		djgpp[$i]

	next
}

# Tests that fail on MinGW
/^EXPECTED_FAIL_MINGW *=/,/[^\\]$/ {
	print
	gsub(/(^EXPECTED_FAIL_MINGW *=|\\$)/,"")
	for (i = 1; i <= NF; i++)
		mingw[$i]

	next
}

# Tests that need a different cmp program
/^NEED_TESTOUTCMP *=/,/[^\\]$/ {
	print
	gsub(/(^NEED_TESTOUTCMP *=|\\$)/,"")
	for (i = 1; i <= NF; i++)
		testoutcmp[$i]

	next
}

# Start of a target
! in_recipe && /^[[:alpha:]_][-[:alnum:]_]*:/ {
	in_recipe = 1
	start_new_recipe()
	next
}

# Empty targets, one after the other
in_recipe && /^[[:alpha:]_][-[:alnum:]_]*:/ {
	print_recipe()
	start_new_recipe()
	next
}

# Collect recipe lines
in_recipe && /^\t/ {
	recipe_lines[++line] = substitutions(name, $0)
	next
}

# End of collected recipe, print it out
in_recipe && /(^[^\t])|(^$)/ {
	in_recipe = 0
	print_recipe()

	print
	next
}

# Default is to print input lines
process		{ print }

# substitutions --- replace *nix values with what's needed on Windows

function substitutions(test, string)
{
	# locales
	gsub(/en_US.UTF-8/, "ENU_USA.1252", string)
	gsub(/fr_FR.UTF-8/, "FRA_FRA.1252", string)
	gsub(/ja_JP.UTF-8/, "JPN_JPN.932", string)
	gsub(/el_GR.iso88597/, "ell_GRC.1253", string)
	gsub(/ru_RU.UTF-8/, "RUS_RUS.1251", string)

	# command for `ls'
	gsub(/@ls/, "@$(LS)", string)

	# MSYS needs "/" to be doubled
	gsub(/-F\//, "-F$(SLASH)", string)
	gsub(/=@\//, "=@$(SLASH)", string)

	if (string ~ /\$\(CMP\)/ && test in testoutcmp) {
		gsub(/\$\(CMP\)/, "$(TESTOUTCMP)", string)
		delete testoutcmp[test]
	}

	return string
}

# print_recipe --- print out the recipe

function print_recipe(		i, start)
{
	print recipe_lines[1]	# target:
	if (line == 1)
		return

	# First line if it's @echo $@
	if (recipe_lines[2] == "\t@echo $@") {
		start = 3
		print recipe_lines[2]
	} else
		start = 2

	# print the right warning
	if (name in djgpp && name in mingw) {
		print "\t@echo Expect $@ to fail with DJGPP and MinGW."
	} else if (name in djgpp) {
		print "\t@echo Expect $@ to fail with DJGPP."
	} else if (name in mingw) {
		print "\t@echo Expect $@ to fail with MinGW."
	}

	# find first line from the end that starts with @
	# make sure it starts with @-
	for (i = line; i >= start; i--) {
		if (recipe_lines[i] !~ /^\t@/)
			continue

		if (recipe_lines[i] ~ /^\t@-/)
			break
		else if (recipe_lines[i] ~ /^\t@/) {
			recipe_lines[i] = "\t@-" substr(recipe_lines[i], 3)
			break
		}
	}

	# Print all the lines but the last
	for (i = start; i <= line; i++)
		print recipe_lines[i]
}

# start_new_recipe --- start collecting data from scratch

function start_new_recipe()
{
	# get the name of the target
	# use match(), not gensub(), so that this script is fully portable
	match($0, /^[[:alpha:]_][-[:alnum:]_]*:/)
	name = substr($0, RSTART, RLENGTH - 1)	# leave out the colon

	delete recipe_lines
	line = 0
	recipe_lines[++line] = $0
}
