$! File: MAKE_PCSI_GAWK_KIT_NAME.COM
$!
$! Calculates the PCSI kit name for use in building an installation kit.
$! PCSI is HP's PolyCenter Software Installation Utility.
$!
$! The results are stored in as logical names so that other procedures
$! can use them.
$!
$! 92-Jan-2014  J. Malmberg	Gawk version
$!========================================================================
$!
$! Save default
$default_dir = f$environment("DEFAULT")
$!
$! Put things back on error.
$on warning then goto all_exit
$!
$! The producer is the name or common abbreviation for the entity that is
$! making the kit.  It must be set as a logical name before running this
$! procedure.
$!
$! HP documents the producer as the legal owner of the software, but for
$! open source work, it should document who is creating the package for
$! distribution.
$!
$producer = f$trnlnm("GNV_PCSI_PRODUCER")
$if producer .eqs. ""
$then
$   write sys$output "The logical name GNV_PCSI_PRODUCER needs to be defined."
$   write sys$output "This should be set to the common abbreviation or name of"
$   write sys$output "the entity creating this kit.  If you are an individual"
$   write sys$output "then use your initials as long as they do not match"
$   write sys$output "a different well known producer prefix."
$   goto all_exit
$endif
$producer_full_name = f$trnlnm("GNV_PCSI_PRODUCER_FULL_NAME")
$if producer_full_name .eqs. ""
$then
$   write sys$output "The logical name GNV_PCSI_PRODUCER_FULL_NAME needs to"
$   write sys$output "be defined.  This should be set to the full name of"
$   write sys$output "the entity creating this kit.  If you are an individual"
$   write sys$output "then use your name."
$   write sys$output "EX: DEFINE GNV_PCSI_PRODUCER_FULL_NAME ""First M. Last"""
$   goto all_exit
$endif
$!
$write sys$output "*****"
$write sys$output "***** Producer = ''producer'"
$write sys$output "*****"
$!
$!
$! Base is one of 'VMS', 'AXPVMS', 'I64VMS', 'VAXVMS' and indicates what
$! binaries are in the kit.  A kit with just 'VMS' can be installed on all
$! architectures.
$!
$base = "VMS"
$arch_type = f$getsyi("ARCH_NAME")
$code = f$extract(0, 1, arch_type)
$if (code .eqs. "I") then base = "I64VMS"
$if (code .eqs. "V") then base = "VAXVMS"
$if (code .eqs. "A") then base = "AXPVMS"
$!
$!
$product = "gawk"
$!
$!
$! We need to get the version from config.h.  It will have a lines like
$! #define PACKAGE_VERSION "4.1.0a"
$!
$!
$open/read/error=version_loop_end verf config.h
$version_loop:
$   read/end=version_loop_end verf line_in
$   if line_in .eqs. "" then goto version_loop
$   if f$locate("#define PACKAGE_VERSION", line_in) .ne. 0
$   then
$       goto version_loop
$   endif
$   tag = f$element(1, " ", line_in)
$   value = f$element(2, " ", line_in) - """" - """"
$   if tag .eqs. "PACKAGE_VERSION"
$   then
$       distversion = value
$       goto version_loop_end
$   endif
$   goto version_loop
$version_loop_end:
$close verf
$!
$!
$! Optional ECO file.
$ECO_LEVEL = ""
$vms_eco_file = "[.vms]vms_eco_level.h"
$if f$search(vms_eco_file) .nes. ""
$then
$   open/read ef 'vms_eco_file'
$ecolevel_loop:
$       read/end=ecolevel_loop_end ef line_in
$       prefix = f$element(0, " ", line_in)
$       if prefix .nes. "#define" then goto ecolevel_loop
$       key = f$element(1, " ", line_in)
$       value = f$element(2, " ", line_in) - """" - """"
$       if key .eqs. "VMS_ECO_LEVEL"
$       then
$           ECO_LEVEL = value
$           if ECO_LEVEL .eq. 0
$           then
$               ECO_LEVEL = ""
$           else
$               ECO_LEVEL = "E" + ECO_LEVEL
$           endif
$           goto ecolevel_loop_end
$       endif
$       goto ecolevel_loop
$ecolevel_loop_end:
$    close ef
$endif
$!
$raw_version = distversion
$!
$!
$! This translates to V0114-08 or D0115-01
$! We can not encode the snapshot date into the version due to the way that
$! the Polycenter Software Installation Utility evaluates the name.
$!
$! version_type = 'V' for a production release, and 'D' for a build from a
$! daily repository snapshot, and a code for a build from a pre-release branch
$majorver = f$element(0, ".", raw_version)
$minorver = f$element(1, ".", raw_version)
$update = f$element(2,".", raw_version)
$if update .eqs. "." then update = ""
$vtype = "V"
$if update .nes. ""
$then
$   update_len = f$length(update)
$   code = f$extract(update_len - 1, 1, update)
$   code = f$edit(code, "UPCASE")
$   if (code .ges. "A") .and. (code .les. "Z")
$   then
$       update = f$extract(0, update_len - 1, update)
$       vtype = code
$   endif
$endif
$if update .eqs. "0" then update = ""
$!
$!
$version_fao = "!2ZB!2ZB"
$mmversion = f$fao(version_fao, 'majorver', 'minorver')
$version = vtype + "''mmversion'"
$if update .nes. "" .or. ECO_LEVEL .nes. ""
$then
$!  The presence of an ECO implies an update
$   if update .eqs. "" .and. ECO_LEVEL .nes. "" then update = "0"
$   version = version + "-" + update + ECO_LEVEL
$   fversion = version
$else
$   fversion = version
$   version = version + "-"
$endif
$!
$! Kit type 1 is complete kit, the only type that this procedure will make.
$Kittype = 1
$!
$! Write out a logical name for the resulting base kit name.
$name = "''producer'-''base'-''product'-''version'-''kittype'"
$define GNV_PCSI_KITNAME "''name'"
$fname = "''product'-''fversion'"
$!
$! No ECO or Patch level
$fname_len = f$length(fname)
$if f$extract(fname_len - 1, 1, fname) .eqs. "-"
$then
$    fname = f$extract(0, fname_len - 1, fname)
$    fname_len = fname_len - 1
$endif
$if f$extract(fname_len - 1, 1, fname) .eqs. "-"
$then
$    fname = f$extract(0, fname_len - 1, fname)
$    fname_len = fname_len - 1
$endif
$define GNV_PCSI_FILENAME_BASE 'fname'
$write sys$output "*****"
$write sys$output "***** GNV_PCSI_KITNAME = ''name'."
$write sys$output "***** GNV_PCSI_FILENAME_BASE = ''fname'."
$write sys$output "*****"
$!
$all_exit:
$set def 'default_dir'
$exit '$status'
