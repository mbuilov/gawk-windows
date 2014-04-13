$!
$! Patch the Macro32 compiler and optional assemble
$!-----------------------------------------------------
$ patched_macro = "sys$disk:[]macro32_exactcase.exe"
$ if f$search(patched_macro) .eqs. ""
$ then
$   copy sys$system:macro32.exe 'patched_macro'
$   patch @[.vms]macro32_exactcase.patch
$ endif
$! Usage:
$ xfer_file_source = p1
$ if f$search(p1) .nes. ""
$ then
$    define/user macro32 'patched_macro'
$    macro/lis 'p1'
$ endif
