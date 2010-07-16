$! vmsbuild.com -- Commands to build GAWK		Pat Rankin, Dec'89
$!							   revised, Mar'90
$!						gawk 2.13  revised, Jun'91
$!						gawk 2.14  revised, Sep'92
$!						gawk 2.15  revised, Oct'93
$!						gawk 3.0   revised, Dec'95
$!						gawk 3.0.1 revised, Nov'96
$!						gawk 3.1.0 revised, Mar'01
$!						gawk 3.1.1 revised, Apr'02
$!
$ REL = "3.1"	!release version number
$ PATCHLVL = "5"
$!
$!
$ CCFLAGS = "/noList"	! "/noOpt/Debug"
$ CDEFS	  = "GAWK,HAVE_CONFIG_H"
$!
$ if p1.eqs."" then  p1 = "DECC"	!default compiler
$ if p1.eqs."GNUC"
$ then
$! assumes VAX
$   CC = "gcc"
$   if f$type(gcc).eqs."STRING" then  CC = gcc
$   CFLAGS = "/Incl=([],[.vms])/Obj=[]/Def=(''CDEFS')''CCFLAGS'"
$   LIBS = "gnu_cc:[000000]gcclib.olb/Library,sys$library:vaxcrtl.olb/Library"
$   if p2.eqs."DO_GNUC_SETUP" then  set command gnu_cc:[000000]gcc
$ else	!!GNUC
$  if p1.eqs."VAXC"
$  then
$!  always VAX; version V3.x of VAX C assumed (for V2.x, remove /Opt=noInline)
$   CC = "cc"
$   if f$trnlnm("DECC$CC_DEFAULT").nes."" then  CC = "cc/VAXC"
$   CFLAGS = "/Incl=[]/Obj=[]/Opt=noInline/Def=(''CDEFS')''CCFLAGS'"
$   LIBS = "sys$share:vaxcrtl.exe/Shareable"
$  else  !!VAXC
$!  neither GNUC nor VAXC, assume DECC (same for either VAX or Alpha)
$   CC = "cc/DECC/Prefix=All"
$   CFLAGS = "/Incl=[]/Obj=[]/Def=(''CDEFS')''CCFLAGS'"
$   LIBS = ""	! DECC$SHR instead of VAXCRTL, no special link option needed
$  endif !VAXC
$ endif !GNUC
$!
$ cc = CC + CFLAGS
$ show symbol cc
$!
$ if f$search("config.h").nes."" then -
    if f$cvtime(f$file_attr("config.h","RDT")).ges.-
       f$cvtime(f$file_attr("[.vms]vms-conf.h","RDT")) then  goto config_ok
$ v = f$verify(1)
$ copy [.vms]vms-conf.h []config.h
$! 'f$verify(v)'
$config_ok:
$ if f$search("awkgram.c").nes."" then  goto awkgram_ok
$	write sys$output " You must process `awkgram.y' with ""yacc"" or ""bison"""
$	if f$search("awkgram_tab.c").nes."" then -	!bison was run manually
	  write sys$output " or else rename `awkgram_tab.c' to `awkgramtab.c'."
$	if f$search("ytab.c").nes."" .or. f$search("y_tab.c").nes."" then - !yacc
	  write sys$output " or else rename `ytab.c' or `y_tab.c' to `awkgramtab.c'."
$	exit
$awkgram_ok:
$ v = f$verify(1)
$ cc array.c
$ cc awkgram.c
$ cc builtin.c
$ cc dfa.c
$ cc ext.c
$ cc field.c
$ cc gawkmisc.c
$ cc getopt.c
$ cc getopt1.c
$ cc io.c
$ cc main.c
$ cc msg.c
$ cc node.c
$ cc random.c
$ cc re.c
$ cc regex.c
$ cc replace.c
$ cc version.c
$ cc eval.c
$ cc profile.c
$ cc [.vms]vms_misc.c
$ cc [.vms]vms_popen.c
$ cc [.vms]vms_fwrite.c
$ cc [.vms]vms_args.c
$ cc [.vms]vms_gawk.c
$ cc [.vms]vms_cli.c
$ set command/Object=[]gawk_cmd.obj [.vms]gawk.cld
$! 'f$verify(v)'
$!
$ close/noLog Fopt
$ create gawk.opt
! GAWK -- GNU awk
array.obj,awkgram.obj,builtin.obj,dfa.obj,ext.obj,field.obj,gawkmisc.obj
getopt.obj,getopt1.obj,io.obj,main.obj,msg.obj,node.obj,random.obj
re.obj,regex.obj,replace.obj,version.obj,eval.obj,profile.obj
[]vms_misc.obj,vms_popen.obj,vms_fwrite.obj,vms_args.obj
[]vms_gawk.obj,vms_cli.obj,gawk_cmd.obj
psect_attr=environ,noshr	!extern [noshare] char **
stack=48	!preallocate more pages (default is 20)
iosegment=128	!ditto (default is 32)
$ open/append Fopt gawk.opt
$ write Fopt libs
$ write Fopt "identification=""V''REL'.''PATCHLVL'"""
$ close Fopt
$!
$ v = f$verify(1)
$ link/exe=gawk.exe gawk.opt/options
$! 'f$verify(v)'
$ exit
