$! gawk_ident.com - Append ident with version to gawk.
$!
$!
$ open/read cfgh config.h
$cfgh_loop:
$ read/end=cfgh_loop_end cfgh line_in
$ key1 = f$element(0, " ", line_in)
$ if key1 .nes. "#define" then goto cfgh_loop
$ key2 = f$element(1, " ", line_in)
$ if key2 .nes. "VERSION" then goto cfgh_loop
$ version_string = f$element(2, " ", line_in) - """" - """"
$ ver_major = f$element(0, ".", version_string)
$ ver_minor = f$element(1, ".", version_string)
$ REL = ver_major + "." + ver_minor
$ PATCHLVL = f$element(2, ".", version_string)
$cfgh_loop_end:
$ close cfgh
$!
$ open/append Fopt gawk.opt
$ write Fopt "identification=""V''REL'.''PATCHLVL'"""
$ close Fopt
