$! File: remove_old_gawk.com
$!
$! This is a procedure to remove the old gawk images that were installed
$! by the GNV kits and replace them with links to the new image.
$!
$! 02-Jan-2014  J. Malmberg	Gawk version
$!
$!==========================================================================
$!
$vax = f$getsyi("HW_MODEL") .lt. 1024
$old_parse = ""
$if .not. VAX
$then
$   old_parse = f$getjpi("", "parse_style_perm")
$   set process/parse=extended
$endif
$!
$old_cutils = "gawk,awk,"
$!
$!
$ i = 0
$cutils_loop:
$   file = f$element(i, ",", old_cutils)
$   if file .eqs. "" then goto cutils_loop_end
$   if file .eqs. "," then goto cutils_loop_end
$   call update_old_image 'file'
$   call update_old_image 'file' '[usr.bin]'
$   i = i + 1
$   goto cutils_loop
$cutils_loop_end:
$!
$!
$if .not. VAX
$then
$   file = "gnv$gnu:[usr.share.man.cat1]awk^.1.gz"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "gnv$gnu:[usr.share.man.cat1]gawk^.1.gz"
$   if f$search(file) .nes. "" then delete 'file';*
$   file = "gnv$gnu:[usr.share.man.cat1]iawk^.1.gz"
$   if f$search(file) .nes. "" then delete 'file';*
$endif
$!
$!
$if .not. VAX
$then
$   set process/parse='old_parse'
$endif
$!
$all_exit:
$  exit
$!
$! Remove old image or update it if needed.
$!-------------------------------------------
$update_old_image: subroutine
$!
$ file = p1
$ fdir = p2
$ if fdir .eqs. "" then fdir = "[bin]"
$! First get the FID of the new gawk image.
$! Don't remove anything that matches it.
$ new_gawk = f$search("GNV$GNU:[user.bin]GNV$''file'.EXE")
$!
$ new_gawk_fid = "No_new_gawk_fid"
$ if new_gawk .nes. ""
$ then
$   new_gawk_fid = f$file_attributes(new_gawk, "FID")
$ endif
$!
$!
$!
$! Now get check the "''file'." and "''file'.exe"
$! May be links or copies.
$! Ok to delete and replace.
$!
$!
$ old_gawk_fid = "No_old_gawk_fid"
$ old_gawk = f$search("gnv$gnu:''fdir'''file'.")
$ old_gawk_exe_fid = "No_old_gawk_fid"
$ old_gawk_exe = f$search("gnv$gnu:''fdir'''file'.exe")
$ if old_gawk_exe .nes. ""
$ then
$   old_gawk_exe_fid = f$file_attributes(old_gawk_exe, "FID")
$ endif
$!
$ if old_gawk .nes. ""
$ then
$   fid = f$file_attributes(old_gawk, "FID")
$   if fid .nes. new_gawk_fid
$   then
$       if fid .eqs. old_gawk_exe_fid
$       then
$           set file/remove 'old_gawk'
$       else
$           delete 'old_gawk'
$       endif
$       if new_gawk .nes. ""
$       then
$           set file/enter='old_gawk' 'new_gawk'
$       endif
$   endif
$ endif
$!
$ if old_gawk_exe .nes. ""
$ then
$   if old_gawk_fid .nes. new_gawk_fid
$   then
$       delete 'old_gawk_exe'
$       if new_gawk .nes. ""
$       then
$           set file/enter='old_gawk_exe' 'new_gawk'
$       endif
$   endif
$ endif
$!
$ exit
$ENDSUBROUTINE ! Update old image
