$! gawk_verb.com - build the gawk_verb.cld from the gawk.cld.
$!
$! The CLD file needed to modify a DCL command table is different
$! from the CLD file needed to build the product by specifying an image.
$!
$! So read in the [.vms]gawk.cld and generate a gawk_verb.cld.
$!
$! 23-Dec-2012 - J. Malmberg
$!
$outfile = "sys$disk:[]gawk_verb.cld"
$infile = "[.vms]gawk.cld"
$open/read cld 'infile'
$create 'outfile'
$open/append cldv 'outfile'
$loop:
$read cld/end=loop_end line_in
$if f$locate("image gawk", line_in) .lt. f$length(line_in)
$then
$    write cldv "    image gnv$gnu:[usr.bin]gnv$gawk"
$    goto loop
$endif
$write cldv line_in
$goto loop
$loop_end:
$close cldv
$close cld
