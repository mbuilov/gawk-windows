BEGIN {
	chapters["Getting Started"]++
	chapters["Invoking Gawk"]++
	chapters["Regexp"]++
	chapters["Reading Files"]++
	chapters["Printing"]++
	chapters["Expressions"]++
	chapters["Patterns and Actions"]++
	chapters["Arrays"]++
	chapters["Functions"]++
	chapters["Library Functions"]++
	chapters["Sample Programs"]++
	chapters["Advanced Features"]++
	chapters["Internationalization"]++
	chapters["Debugger"]++
	chapters["Arbitrary Precision Arithmetic"]++
	chapters["Dynamic Extensions"]++
	chapters["Language History"]++
	chapters["Installation"]++
	chapters["Notes"]++
	chapters["Basic Concepts"]++
	
	Pattern = ".*@ref\\{([^}]+)\\},.*"
}

$0 ~ Pattern {
	ref = gensub(Pattern, "\\1", 1, $0)
	if (! (ref in chapters))
		printf("%s:%d: %s\n", FILENAME, FNR, $0)
}

