$! Compare_gawk_source.com
$!
$! This procedure compares the files in two directories and reports the
$! differences.
$!
$! It needs to be customized to the local site directories.
$!
$! This is used by me for these purposes:
$!     1. Compare the original source of a project with an existing
$!        VMS port.
$!     2. Compare the checked out repository of a project with the
$!        the local working copy to make sure they are in sync.
$!     3. Keep a copy directory up to date.  The third is needed by
$!        me because VMS Backup can create a saveset of files from a
$!        NFS mounted volume.
$!
$! First the files in the original source directory which is assumed to be
$! under source codde control are compared with the copy directory.
$!
$! Then the files are are only in the copy directory are listed.
$!
$! The result will five diagnostics about of files:
$!    1. Files that are not generation 1.
$!    2. Files missing in the copy directory.
$!    3. Files in the copy directory not in the source directory.
$!    4. Files different from the source directory.
$!    5. Files that VMS DIFF can not process.
$!
$! This needs to be run on an ODS-5 volume.
$!
$! If UPDATE is given as a second parameter, files missing or different in the
$! copy directory will be updated.
$!
$! By default:
$!    The source directory is source_root:[gawk.reference.gawk],
$!    the logical used on my system for the GNV Mecurial repository checkout.
$!    If source_root: is not defined, then src_root:[gawk] will be
$!    translated to something like DISK:[dir.gawk.reference.gawk]
$!    and then DISK:[dir.gawk.vms_source.gawk] will be used.
$!
$!    The copy directory is vms_root:[gawk]
$!    The UPDATE parameter is ignored.
$!
$!    This setting is used to make sure that the working vms directory
$!    and the VMS specific repository checkout directory have the same
$!    contents if they are different.
$!
$! If P1 is "SRCBCK" then this
$!     The source directory tree is: src_root:[gawk]
$!     The copy directory is src_root1:[gawk]
$!
$!   src_root1:[gawk] is used by me to work around that VMS backup will
$!   not use NFS as a source directory so I need to make a copy.
$!
$!   This is to make sure that the backup save set for the unmodified
$!   source is up to date.
$!
$!   If your repository checkout is not on an NFS mounted volume, you do not
$!   need to use this option or have the logical name src_root1 defined.
$!
$! If P1 is "VMSBCK" then this changes the two directories:
$!    The source directory is vms_root:[gawk]
$!    The copy directory is vms_root1:[gawk]
$!
$!   vms_root:
$!   src_root1:[gawk] is used by me to work around that VMS backup will
$!   not use NFS as a source directory so I need to make a copy.
$!
$!   This is to make sure that the backup save set for the unmodified
$!   source is up to date.
$!
$!   If your repository checkout is not on an NFS mounted volume, you do not
$!   need to use this option or have the logical name src_root1 defined.
$!
$! 02-Jan-2014   J. Malmberg
$!==========================================================================
$!
$! Update missing/changed files.
$update_file = 0
$if (p2 .eqs. "UPDATE")
$then
$   update_file = 1
$endif
$!
$myproc = f$environment("PROCEDURE")
$myprocdir = f$parse(myproc,,,"DIRECTORY") - "[" - "]" - "<" - ">"
$myprocdir = f$edit(myprocdir, "LOWERCASE")
$mydefault = f$environment("DEFAULT")
$mydir = f$parse(mydefault,,,"DIRECTORY")
$mydir = f$edit(mydir, "LOWERCASE")
$odelim = f$extract(0, 1, mydir)
$mydir = mydir - "[" - "]" - "<" - ">"
$mydev = f$parse(mydefault,,,"DEVICE")
$!
$ref = ""
$if P1 .eqs. ""
$then
$   ref_base_dir = myprocdir - ".vms"
$   wrk_base_dir = mydir
$   update_file = 0
$   resultd = f$parse("src_root:",,,,"NO_CONCEAL")
$   resultd = f$edit(resultd, "LOWERCASE")
$   resultd = resultd - "][" - "><" - ".;" - ".."
$   resultd_len = f$length(resultd) - 1
$   delim = f$extract(resultd_len, 1, resultd)
$   ref_root_base = mydir + delim
$   if f$locate(".reference.", resultd) .lt. resultd_len
$   then
$      resultd = resultd - ref_root_base - "reference." + "vms_source."
$   else
$      resultd = resultd - ref_root_base - "gnu." + "gnu_vms."
$   endif
$   ref = resultd + ref_base_dir
$   wrk = "VMS_ROOT:" + odelim + wrk_base_dir
$   resultd_len = f$length(resultd) - 1
$   resultd = f$extract(0, resultd_len, resultd) + delim
$   ref_root_dir = f$parse(resultd,,,"DIRECTORY")
$   ref_root_dir = f$edit(ref_root_dir, "LOWERCASE")
$   ref_root_dir = ref_root_dir - "[" - "]"
$   ref_base_dir = ref_root_dir + "." + ref_base_dir
$endif
$!
$if p1 .eqs. "SRCBCK"
$then
$   ref_base_dir = "gawk"
$   wrk_base_dir = "gawk"
$   ref = "src_root:[" + ref_base_dir
$   wrk = "src_root1:[" + wrk_base_dir
$   if update_file
$   then
$       if f$search("src_root1:[000000]gawk.dir") .eqs. ""
$       then
$           create/dir/prot=o:rwed src_root1:[gawk]
$       endif
$   endif
$endif
$!
$!
$if p1 .eqs. "VMSBCK"
$then
$   ref_base_dir = "gawk"
$   wrk_base_dir = "gawk"
$   ref = "vms_root:[" + ref_base_dir
$   wrk = "vms_root1:[" + wrk_base_dir
$   if update_file
$   then
$       if f$search("vms_root1:[000000]gawk.dir") .eqs. ""
$       then
$           create/dir/prot=o:rwed vms_root1:[gawk]
$       endif
$   endif
$endif
$!
$!
$if ref .eqs. ""
$then
$   write sys$output "Unknown compare type specified!"
$   exit 44
$endif
$!
$!
$!
$! Future - check the device types involved for the
$! the syntax to check.
$ODS2_SYNTAX = 0
$NFS_MANGLE = 0
$PWRK_MANGLE = 0
$!
$vax = f$getsyi("HW_MODEL") .lt. 1024
$if vax
$then
$   ODS2_SYNTAX = 1
$endif
$!
$report_missing = 1
$!
$if .not. ODS2_SYNTAX
$then
$   set proc/parse=extended
$endif
$!
$loop:
$  ref_spec = f$search("''ref'...]*.*;",1)
$  if ref_spec .eqs. "" then goto loop_end
$!
$  ref_dev = f$parse(ref_spec,,,"DEVICE")
$  ref_dir = f$parse(ref_spec,,,"DIRECTORY")
$  ref_dir = f$edit(ref_dir, "LOWERCASE")
$  ref_name = f$parse(ref_spec,,,"NAME")
$  ref_type = f$parse(ref_spec,,,"TYPE")
$!
$!
$  if f$locate(".CVS]", ref_dir) .lt. f$length(ref_dir) then goto loop
$  if f$locate(".cvs]", ref_dir) .lt. f$length(ref_dir) then goto loop
$  if f$locate(".$cvs]", ref_dir) .lt. f$length(ref_dir) then goto loop
$  if f$locate(".^.git", ref_dir) .lt. f$length(ref_dir) then goto loop
$  if f$locate(".$5ngit", ref_dir) .lt. f$length(ref_dir) then goto loop
$!
$  rel_path = ref_dir - "[" - ref_base_dir
$!  rel_path_len = f$length(rel_path) - 1
$!  delim = f$extract(rel_path_len, 1, rel_path)
$!  rel_path = rel_path - ".]" - ".>" - "]" - ">"
$!  rel_path = rel_path + delim
$!
$  if ODS2_SYNTAX
$  then
$  endif
$!
$  wrk_path = wrk + rel_path
$!
$  ref_name_type = ref_name + ref_type
$!
$  if ref_name_type .eqs. "CVS.DIR" then goto loop
$  if ref_name_type .eqs. "cvs.dir" then goto loop
$  if ref_name_type .eqs. "$CVS.DIR" then goto loop
$  if ref_name_type .eqs. "^.git.DIR" then goto loop
$  if ref_name_type .eqs. "$5ngit.DIR" then goto loop
$  if ref_name_type .eqs. "$5NGIT.DIR" then goto loop
$  if ODS2_SYNTAX
$  then
$!
$  endif
$!
$  wrk_spec = wrk_path + ref_name_type
$!
$!
$  wrk_chk = f$search(wrk_spec, 0)
$  if wrk_chk .eqs. ""
$  then
$    if report_missing
$    then
$      write sys$output "''wrk_spec' is missing"
$    endif
$    if update_file
$    then
$      copy/log 'ref_spec' 'wrk_spec'
$    endif
$    goto loop
$  endif
$!
$  wrk_name = f$parse(wrk_spec,,,"NAME")
$  wrk_type = f$parse(wrk_spec,,,"TYPE")
$  wrk_fname = wrk_name + wrk_type"
$  ref_fname = ref_name + ref_type
$!
$  if ref_fname .nes. wrk_fname
$  then
$    write sys$output "''wrk_spc' wrong name, should be ""''ref_fname'"""
$  endif
$!
$  ref_type = f$edit(ref_type, "UPCASE")
$  if ref_type .eqs. ".DIR" then goto loop
$!
$  if ODS2_SYNTAX
$  then
$       ref_fname = f$edit(ref_fname, "LOWERCASE")
$  endif
$!
$! These files have records to long to diff, and we don't change them anyway.
$  ref_skip = 0
$  if ref_type .eqs. ".GMO" then ref_skip = 1
$  if ref_type .eqs. ".PDF" then ref_skip = 1
$  if ref_type .eqs. ".PNG" then ref_skip = 1
$  if ref_type .eqs. ".JPG" then ref_skip = 1
$  if ref_fname .eqs. "inftest.ok" then ref_skip = 1
$  if ref_fname .eqs. "longsub.in" then ref_skip = 1
$  if ref_fname .eqs. "longsub.ok" then ref_skip = 1
$  if ref_fname .eqs. "nasty2.ok" then ref_skip = 1
$  if ref_fname .eqs. "profile5.awk" then ref_skip = 1
$  if ref_fname .eqs. "profile5.ok" then ref_skip = 1
$  if ref_fname .eqs. "po.m4" then ref_skip = 1
$  if ref_fname .eqs. "configure." then ref_skip = 1
$  if ref_fname .eqs. "libtool.m4" then ref_skip = 1
$!
$!
$  if ref_skip .ne. 0
$  then
$    if report_missing
$    then
$        write sys$output "Skipping diff of ''ref_fname'"
$    endif
$    goto loop
$  endif
$!
$!
$  wrk_ver = f$parse(wrk_chk,,,"VERSION")
$  if wrk_ver .nes. ";1"
$  then
$    write sys$output "Version for ''wrk_spec' is not 1"
$  endif
$  set noon
$  diff/out=nl: 'wrk_spec' 'ref_spec'
$  if $severity .nes. "1"
$  then
$    write sys$output "''wrk_spec' is different from ''ref_spec'"
$    if update_file
$    then
$       delete 'wrk_spec';*
$       copy/log 'ref_spec' 'wrk_spec'
$    endif
$  endif
$  set on
$
$!
$  goto loop
$loop_end:
$!
$!
$missing_loop:
$! For missing loop, check the latest generation.
$  ref_spec = f$search("''wrk'...]*.*;")
$  if ref_spec .eqs. "" then goto missing_loop_end
$!
$  ref_dev = f$parse(ref_spec,,,"DEVICE")
$  ref_dir = f$parse(ref_spec,,,"DIRECTORY")
$  ref_dir = f$edit(ref_dir, "LOWERCASE")
$  ref_name = f$parse(ref_spec,,,"NAME")
$  ref_type = f$parse(ref_spec,,,"TYPE")
$!
$  rel_path = ref_dir - "[" - wrk_base_dir
$!
$!
$  wrk_path = ref + rel_path
$  wrk_spec = wrk_path + ref_name + ref_type
$  wrk_name = f$parse(wrk_spec,,,"NAME")
$  wrk_type = f$parse(wrk_spec,,,"TYPE")
$!
$  wrk_fname = wrk_name + wrk_type"
$  ref_fname = ref_name + ref_type
$!
$  wrk_skip = 0
$  ref_utype = f$edit(ref_type,"UPCASE")
$  ref_ufname = f$edit(ref_fname,"UPCASE")
$!
$!
$  if wrk_skip .eq. 0
$  then
$     wrk_chk = f$search(wrk_spec, 0)
$     if wrk_chk .eqs. ""
$     then
$         if report_missing
$         then
$             write sys$output "''wrk_spec' is missing"
$         endif
$         goto missing_loop
$     endif
$  else
$     goto missing_loop
$  endif
$!
$  if ref_fname .nes. wrk_fname
$  then
$    write sys$output "''wrk_spc' wrong name, should be ""''ref_fname'"""
$  endif
$!
$  if ref_utype .eqs. ".DIR" then goto missing_loop
$!
$  wrk_ver = f$parse(wrk_chk,,,"VERSION")
$  if wrk_ver .nes. ";1"
$  then
$    write sys$output "Version for ''wrk_spec' is not 1"
$  endif
$!
$  goto missing_loop
$!
$!
$missing_loop_end:
$!
$exit
