# This file describes the semantics for hard regex constants
# As much as possible it's executable code so that it can be used
# (or split into) test cases for development and regression testing.

function simple_tests(	fbre, numresult, strresult)
{
	# usable as case value
	switch ("foobaaar") {
	case @/fo+ba+r/:
		print "switch-case: ok"
		break
	default:
		print "switch-case: fail"
		break
	}

	# usable with ~ and !~
	if ("foobaaar" ~ @/fo+ba+r/)
		print "match ~: ok"
	else
		print "match ~: fail"

	if ("quasimoto" !~ @/fo+ba+r/)
		print "match !~: ok"
	else
		print "match !~: fail"

	# assign to variable, use in match
	fbre = @/fo+ba+r/
	if ("foobaaar" ~ fbre)
		print "variable match ~: ok"
	else
		print "variable match ~: fail"

	if ("quasimoto" !~ fbre)
		print "variable match !~: ok"
	else
		print "variable match !~: fail"

	# Use as numeric value, should be zero
	numresult = fbre + 42
	if (numresult == 42)
		print "variable as numeric value: ok"
	else
		print "variable as numeric value: fail"

	# Use as string value, should be string value of text
	strresult = "<" fbre ">"
	if (strresult == "<fo+ba+r>")
		print "variable as string value: ok"
	else
		print "variable as string value: fail", strresult

	# typeof should work
	if (typeof(@/fo+ba+r/) == "regexp")
		print "typeof constant: ok"
	else
		print "typeof constant: fail"

	if (typeof(fbre) == "regexp")
		print "typeof variable: ok"
	else
		print "typeof variable: fail"

	# conversion to number, works. should it be fatal?
	fbre++
	if (fbre == 1)
		print "conversion to number: ok"
	else
		print "conversion to number: fail"

	if (typeof(fbre) == "scalar_n")
		print "typeof variable after conversion: ok"
	else
		print "typeof variable after conversion: fail"
}

BEGIN {
	simple_tests()

	# use with match, constant
	# use with match, variable
	# use with sub, constant
	# use with sub, variable
	# use with gsub, constant
	# use with gsub, variable
	# use with gensub, constant
	# use with gensub, variable
	# use with split, constant
	# use with split, variable
	# use with patsplit, constant
	# use with patsplit, variable

	# indirect call tests...
}
