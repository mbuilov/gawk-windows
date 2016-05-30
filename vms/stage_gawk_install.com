$! File: stage_gawk_install.com
$!
$! Stages the build products to new_gnu:[...] for testing and for building
$! a kit.
$!
$! If p1 starts with "R" then remove instead of install.
$!
$! The file PCSI_GAWK_FILE_LIST.TXT is read in to get the files other
$! than the release notes file and the source backup file.
$!
$! The PCSI system can really only handle ODS-2 format filenames and
$! assumes that there is only one source directory.  It also assumes that
$! all destination files with the same name come from the same source file.
$!
$!
$! 29-Sep-2013  J. Malmberg
$!
$!===========================================================================
$!
$ arch_type = f$getsyi("ARCH_NAME")
$ arch_code = f$extract(0, 1, arch_type)
$!
$ mode = "install"
$ code = f$extract(0, 1, p1)
$ if code .eqs. "R" .or. code .eqs. "r" then mode = "remove"
$!
$!  First create the directories
$!--------------------------------
$ if mode .eqs. "install"
$ then
$   create/dir new_gnu:[bin]/prot=o:rwed
$   create/dir new_gnu:[vms_bin]/prot=o:rwed
$   create/dir new_gnu:[vms_help]/prot=o:rwed
$   create/dir new_gnu:[lib]/prot=o:rwed
$   create/dir new_gnu:[usr.bin]/prot=o:rwed
$   create/dir new_gnu:[usr.include]/prot=o:rwed
$   create/dir new_gnu:[usr.lib.gawk]/prot=o:rwed
$   create/dir new_gnu:[usr.share.awk]/prot=o:rwed
$   create/dir new_gnu:[usr.share.doc.gawk]/prot=o:rwed
$   create/dir new_gnu:[usr.share.info]/prot=o:rwed
$   create/dir new_gnu:[usr.src.gawk.extension.vms]/prot=o:rwed
$   create/dir new_gnu:[usr.share.man.man1]/prot=o:rwed
$ endif
$!
$ if mode .eqs. "install"
$ then
$    copy [.vms]gnv_gawk_startup.com -
         new_gnu:[vms_bin]gnv$gawk_startup.com
$ else
$    file = "new_gnu:[vms_bin]gnv$gawk_startup.com"
$    if f$search(file) .nes. "" then delete 'file';*
$ endif
$!
$!
$!   Read through the file list to set up aliases and rename commands.
$!---------------------------------------------------------------------
$ open/read flst [.vms]pcsi_gawk_file_list.txt
$!
$inst_alias_loop:
$   ! Skip the aliases
$   read/end=inst_file_loop_end flst line_in
$   line_in = f$edit(line_in,"compress,trim,uncomment")
$   if line_in .eqs. "" then goto inst_alias_loop
$   pathname = f$element(0, " ", line_in)
$   linkflag = f$element(1, " ", line_in)
$   if linkflag .nes. "->" then goto inst_alias_done
$   goto inst_alias_loop
$!
$inst_file_loop:
$!
$   read/end=inst_file_loop_end flst line_in
$   line_in = f$edit(line_in,"compress,trim,uncomment")
$   if line_in .eqs. "" then goto inst_file_loop
$!
$inst_alias_done:
$!
$!
$!   Skip the directories as we did them above.
$!   Just process the files.
$   tdir = f$parse(line_in,,,"DIRECTORY")
$   tdir_len = f$length(tdir)
$   tname = f$parse(line_in,,,"NAME")
$   lctname = f$edit(tname, "LOWERCASE")
$   ttype = f$parse(line_in,,,"TYPE")
$   if arch_code .eqs. "V"
$   then
$       tname = lctname
$       ttype = f$edit(ttype, "LOWERCASE")
$       tdir = f$edit(tdir, "LOWERCASE")
$   endif
$   if tname .eqs. "" then goto inst_file_loop
$   if ttype .eqs. ".dir" then goto inst_file_loop
$!
$!   if p1 starts with "R" then remove instead of install.
$!
$!   If gnv$xxx.exe, then:
$!       Source is []gnv$gawk.exe
$!       Destination1 is new_gnu:[bin]gnv$gawk.exe
$!       Destination2 is new_gnu:[bin]xxx.  (alias)
$!       Destination2 is new_gnu:[bin]xxx.exe  (alias)
$!       We put all in new_gnu:[bin] instead of some in [usr.bin] because
$!       older GNV kits incorrectly put some images in [bin] and [bin]
$!       comes first in the search list.
$   if f$locate("gnv$", tname) .eq. 0
$   then
$       myfile_len = f$length(tname)
$       myfile = f$extract(4, myfile_len, tname)
$       source = "[]''myfile'''ttype'"
$       dest1 = "new_gnu:[bin]''tname'''ttype'"
$       dest2 = "new_gnu:[bin]''myfile'."
$       dest3 = "new_gnu:[bin]''myfile'.exe"
$       if mode .eqs. "install"
$       then
$           if f$search(dest1) .eqs. "" then copy 'source' 'dest1'
$           if f$search(dest2) .eqs. "" then set file/enter='dest2' 'dest1'
$           if f$search(dest3) .eqs. "" then set file/enter='dest3' 'dest1'
$       else
$           if f$search(dest2) .nes. "" then set file/remove 'dest2';*
$           if f$search(dest3) .nes. "" then set file/remove 'dest3';*
$           if f$search(dest1) .nes. "" then delete 'dest1';*
$       endif
$       goto inst_file_loop
$   endif
$!
$!   If lib.gawk] then
$!       source is sys$disk:[]
$!       dest is new_gnu:[usr.lib.gawk]
$    if f$locate("lib.gawk]", tdir) .lt. tdir_len
$    then
$!       ! Not yet available on VAX/VMS
$        if arch_code .eqs. "V" then goto inst_file_loop
$!
$        source = "[]''tname'''ttype'"
$        dest = "new_gnu:[usr.lib.gawk]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$!   If .vms_bin] then
$!       source is sys$disk:[]
$!       dest is [vms_bin]
$   if (f$locate("vms_bin]", tdir) .lt. tdir_len)
$   then
$       if (ttype .eqs. ".cld")
$       then
$           source = "sys$disk:[]''tname'''ttype'"
$       else
$           source = "sys$disk:[.vms]''tname'''ttype'"
$       endif
$       dest = "new_gnu:[vms_bin]''tname'''ttype'"
$       if mode .eqs. "install"
$       then
$           if f$search(dest) .eqs. "" then copy 'source' 'dest'
$       else
$           if f$search(dest) .nes. "" then delete 'dest';*
$       endif
$       goto inst_file_loop
$   endif
$!
$!   If .vms_hlp] then
$!       source is sys$disk:[.vms]
$!       dest is [vms_help]
$   if (f$locate("vms_help]", tdir) .lt. tdir_len) .and. (ttype .eqs. ".hlp")
$   then
$       source = "sys$disk:[.vms]''tname'''ttype'"
$       dest = "new_gnu:[vms_help]''tname'''ttype'"
$       if mode .eqs. "install"
$       then
$           if f$search(dest) .eqs. "" then copy 'source' 'dest'
$       else
$           if f$search(dest) .nes. "" then delete 'dest';*
$       endif
$       goto inst_file_loop
$   endif
$!
$!   If doc.gawk] then
$!       source is sys$disk:[] or [.readme_d]
$!       dest is [usr.share.doc.gawk]
$   if f$locate(".doc.gawk]", tdir) .lt. tdir_len
$   then
$       if lctname .eqs. "readme" .and. ttype .nes. "."
$       then
$           source = "sys$disk:[.README_D]''tname'''ttype'"
$           if f$search(source) .eqs. ""
$           then
$!              ! This may be an NFS mangled name on VAX.
$               stype = ttype - "."
$               if stype .nes. "vms" then stype = "$" + stype
$               source = "sys$disk:[.$README_$D]$README.''stype'"
$           endif
$       else
$           source = "sys$disk:[]''tname'''ttype'"
$           if f$search(source) .eqs. ""
$           then
$               source = "sys$disk:[]$''tname'''ttype'"
$           endif
$       endif
$       dest = "new_gnu:[usr.share.doc.gawk]''tname'''ttype'"
$       if mode .eqs. "install"
$       then
$           if f$search(dest) .eqs. "" then copy 'source' 'dest'
$       else
$           if f$search(dest) .nes. "" then delete 'dest';*
$       endif
$       goto inst_file_loop
$   endif
$!
$!   If .awk] then
$!       source is sys$disk:[.awklib.eg.lib]
$!       dest is new_gnu:[usr.share.awk]
$    if f$locate(".awk]", tdir) .lt. tdir_len
$    then
$        source = "[.awklib.eg.lib]''tname'''ttype'"
$        dest = "new_gnu:[usr.share.awk]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$!   If *.info then
$!       source is [.doc]gawk.info
$!       dest is [.usr.share.info]
$    if ttype .eqs. ".info"
$    then
$        source = "[.doc]''tname'''ttype'"
$        dest = "new_gnu:[usr.share.info]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$!   If *.h then
$!       source is []*.h
$!       dest is [.usr.include]
$    if ttype .eqs. ".h"
$    then
$        source = "[]''tname'''ttype'"
$        dest = "new_gnu:[usr.include]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$!   If *.opt then
$!       source is [.vms]gawk_plugin.opt
$!       dest is [.usr.src.gawk.extension.vms]
$    if ttype .eqs. ".opt"
$    then
$        source = "[.vms]''tname'''ttype'"
$        dest = "new_gnu:[usr.src.gawk.extension.vms]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$!   If xxx.1 then
$!       source is [.doc]xxx.1
$!       dest is [usr.share.man.man1]
$    if ttype .eqs. ".1"
$    then
$        source = "[.doc]''tname'''ttype'"
$        dest = "new_gnu:[usr.share.man.man1]''tname'''ttype'"
$        if mode .eqs. "install"
$        then
$            if f$search(dest) .eqs. "" then copy 'source' 'dest'
$        else
$            if f$search(dest) .nes. "" then delete 'dest';*
$        endif
$        goto inst_file_loop
$    endif
$!
$    goto inst_file_loop
$!
$inst_file_loop_end:
$!
$close flst
$!
$all_exit:
$   exit
