$! File: Build_gawk_release_notes.com
$!
$! Build the release note file from the three components:
$!    1. The gawk_release_note_start.txt
$!    2. readme. file from the Gawk distribution.
$!    3. The gawk_build_steps.txt.
$!
$! Set the name of the release notes from the GNV_PCSI_FILENAME_BASE
$! logical name.
$!
$!
$! 31-Dec-2013  J. Malmberg
$!
$!===========================================================================
$!
$ base_file = f$trnlnm("GNV_PCSI_FILENAME_BASE")
$ if base_file .eqs. ""
$ then
$   write sys$output "@MAKE_PCSI_GAWK_KIT_NAME.COM has not been run."
$   goto all_exit
$ endif
$!
$ gawk_vms_readme = f$search("sys$disk:[.readme_d]readme.vms")
$ if gawk_vms_readme .eqs. ""
$ then
$   gawk_vms_readme = f$search("sys$disk:[.$README_$D]$README.VMS")
$ endif
$ if gawk_vms_readme .eqs. ""
$ then
$   write sys$output "Can not find gawk README.VMS file."
$   goto all_exit
$ endif
$!
$ gawk_readme = f$search("sys$disk:[]readme.")
$ if gawk_readme .eqs. ""
$ then
$   gawk_readme = f$search("sys$disk:[]$README.")
$ endif
$ if gawk_readme .eqs. ""
$ then
$   write sys$output "Can not find gawk readme file."
$   goto all_exit
$ endif
$!
$ gawk_copying = f$search("sys$disk:[]copying.")
$ if gawk_copying .eqs. ""
$ then
$   gawk_copying = f$search("sys$disk:[]$COPYING.")
$ endif
$ if gawk_copying .eqs. ""
$ then
$   write sys$output "Can not find gawk copying file."
$   goto all_exit
$ endif
$!
$ type/noheader sys$disk:[.vms]gawk_release_note_start.txt,-
        'gawk_readme',-
        'gawk_vms_readme',-
        'gawk_copying', -
        sys$disk:[.vms]gawk_build_steps.txt -
        /out='base_file'.release_notes
$!
$ purge 'base_file'.release_notes
$ rename 'base_file.release_notes ;1
$!
$all_exit:
$   exit
