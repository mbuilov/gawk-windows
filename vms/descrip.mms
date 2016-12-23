# Descrip.MMS -- Makefile for building GNU awk on VMS.
#
# usage:
#  $ MMS /Description=[.vms]Descrip.MMS gawk
#	possibly add `/Macro=("GNUC=1")' to compile with GNU C,
#	or add `/Macro=("GNUC=1","DO_GNUC_SETUP=1")' to compile with GNU C
#	on a system where GCC is not installed as a defined command,
#	or add `/Macro=("VAXC=1")' to compile with VAX C,
#	or add `/Macro=("VAXC=1","CC=cc/VAXC")' to compile with VAX C on
#	a system which has DEC C installed as the default compiler.
#
# gawk.exe :
#	This is the default target.  DEC C has become the default compiler.
#
# awkgram.c :
#	If you don't have bison but do have VMS POSIX or DEC/Shell,
#	change the PARSER and PASERINIT macros to use yacc.  If you don't
#	have either yacc or bison, you'll have to make sure that the
#	distributed version of "awkgram.c" has its modification date later
#	than the date of "awkgram.y", so that MMS won't try to build that
#	target.  If you use bison and it is already defined system-wide,
#	comment out the PARSERINIT definition.
#
# command.c :
#	Similar to awkgram.c; built from command.y with yacc or bison.
#
# install.help :
#	You can make the target 'install.help' to load the VMS help text
#	into a help library.  Modify the HELPLIB macro if you don't want
#	to put entry into the regular VMS library.  (If you use an alternate
#	help library, it must already exist; this target won't create it.)
#
# gawk.dvi :
#	If you have TeX, you can make the target 'gawk.dvi' to process
#	_The_GAWK_Manual_ from gawk.texi.  You'll need to use a device
#	specific post-processor on gawk.dvi in order to get printable data.
#	The full output is approximately 325 pages.
#

# location of various source files, relative to the 'main' directory
SUPPORT = [.support]
VMSDIR	= [.vms]
DOCDIR	= [.doc]
MISSNGD	= [.missing_d]
MAKEFILE = $(VMSDIR)Descrip.MMS

# debugging &c		!'ccflags' is an escape to allow external compile flags
#CCFLAGS = /noOpt/Debug

# a comma separated list of macros to define
# Do not specify _POSIX_EXIT here, other tricks are used for this.
CDEFS	= "GAWK","HAVE_CONFIG_H"

.ifdef GNUC
# assumes VAX
CC	= gcc
CFLAGS	= /Incl=([],$(VMSDIR))/Obj=[]/Def=($(CDEFS)) $(CCFLAGS)
LIBS	= gnu_cc:[000000]gcclib.olb/Library,sys$library:vaxcrtl.olb/Library
.ifdef DO_GNUC_SETUP
# in case GCC command verb needs to be manually defined
.first
	set command gnu_cc:[000000]gcc
.endif	!DO_GNUC_SETUP
.else	!!GNUC
.ifdef VAXC
# always VAX; versions of VAX C older than V3.2 won't work
CC	= cc
CFLAGS	= /Incl=[]/Obj=[]/Opt=noInline/Def=($(CDEFS)) $(CCFLAGS)
LIBS	= sys$share:vaxcrtl.exe/Shareable
.else	!!VAXC
# neither GNUC nor VAXC, assume DECC (same for either VAX or Alpha)
.ifdef __VAX__
CFLOAT  =
.else
CFLOAT	= /float=ieee/ieee_mode=denorm_results
.endif
CNAME	= /NAME=(AS_IS,SHORT)
CC	= cc/DECC/Prefix=All/NESTED_INCLUDE=NONE$(CFLOAT)
CINC1   = [],[.VMS],$(SUPPORT)
CFLAGS	= /Incl=($(CINC1))/Obj=[]/Def=($(CDEFS))$(CNAME) $(CCFLAGS)
CEFLAGS = /Incl=($(CINC1),[.missing_d],[.extension])$(CNAME) $(CCFLAGS)
LIBS	=	# DECC$SHR instead of VAXCRTL, no special link option needed
.endif	!VAXC
.endif	!GNUC


PARSER	= bison
PARSERINIT = set command gnu_bison:[000000]bison
#PARSER	= yacc
#PARSERINIT = yacc := posix/run/path=posix """/bin/yacc"
#PARSERINIT = yacc := $shell$exe:yacc

# this is used for optional target 'install.help'
HELPLIB = sys$help:helplib.hlb
#HELPLIB = sys$help:local.hlb

#
########  nothing below this line should need to be changed  ########
#

ECHO = write sys$output
NOOP = continue

# object files
GAWKOBJ = eval.obj,profile.obj
AWKOBJ1 = array.obj,awkgram.obj,builtin.obj,cint_array.obj,\
	command.obj,debug.obj,dfa.obj,ext.obj,field.obj,\
	floatcomp.obj,gawkapi.obj,gawkmisc.obj,getopt.obj,getopt1.obj

AWKOBJ2 = int_array.obj,io.obj,localeinfo.obj,main.obj,mpfr.obj,msg.obj,\
	node.obj,random.obj,re.obj,regex.obj,replace.obj,\
	str_array.obj,symbol.obj,version.obj

AWKOBJS = $(AWKOBJ1),$(AWKOBJ2)

# VMSOBJS
#	VMS specific stuff
VMSCODE = vms_misc.obj,vms_popen.obj,vms_fwrite.obj,vms_args.obj,\
	vms_gawk.obj,vms_cli.obj,vms_crtl_init.obj
VMSCMD	= gawk_cmd.obj			# built from .cld file
VMSOBJS = $(VMSCODE),$(VMSCMD)

DOCS= $(DOCDIR)gawk.1,$(DOCDIR)gawk.texi,$(DOCDIR)texinfo.tex

# generic target
all : gawk gawk_debug
      @	$(NOOP)

# dummy target to allow building "gawk" in addition to explicit "gawk.exe"
gawk : gawk.exe
      @	$(ECHO) "$< is upto date"

gawk_debug : gawk_debug.exe
      @	$(ECHO) "$< is upto date"

# rules to build gawk
gawk.exe : $(GAWKOBJ) $(AWKOBJS) $(VMSOBJS) gawk.opt
	$(LINK) $(LINKFLAGS)/EXE=$(MMS$TARGET) gawk.opt/options

gawk_debug.exe : $(GAWKOBJ) $(AWKOBJS) $(VMSOBJS) gawk.opt
	$(LINK) $(LINKFLAGS)/DEBUG/EXE=$(MMS$TARGET) gawk.opt/options

gawk.opt : $(MAKEFILE) config.h         # create linker options file
      @	open/write opt sys$disk:[]gawk.opt	! ~ 'cat <<close >gawk.opt'
      @	write opt "! GAWK -- GNU awk"
      @ write opt "$(GAWKOBJ)"
      @ write opt "$(AWKOBJ1)"
      @ write opt "$(AWKOBJ2)"
      @ write opt "$(VMSOBJS)"
      @ write opt "psect_attr=environ,noshr	!extern [noshare] char **"
      @ write opt "stack=48	!preallocate more pages (default is 20)"
      @ write opt "iosegment=128	!ditto (default is 32)"
      @	write opt "$(LIBS)"
      @ close opt
      $ @$(VMSDIR)gawk_ident.com

$(VMSCODE)	: awk.h config.h $(VMSDIR)redirect.h $(VMSDIR)vms.h
$(AWKOBJS)	: awk.h gettext.h mbsupport.h $(SUPPORT)regex.h \
		  $(SUPPORT)dfa.h config.h $(VMSDIR)redirect.h
$(GAWKOBJ)	: awk.h config.h $(VMSDIR)redirect.h

#-----------------------------------------------------------------------------
# Older versions of MMS have problems handling lower case file names typically
# found on ODS-5 disks. Fix this by adding explicit dependencies.
#_____________________________________________________________________________

array.obj	: array.c
awkgram.obj	: awkgram.c awk.h
builtin.obj	: builtin.c floatmagic.h $(SUPPORT)random.h
cint_array.obj	: cint_array.c
command.obj	: command.c cmd.h
debug.obj	: debug.c cmd.h
dfa.obj		: $(SUPPORT)dfa.c $(SUPPORT)dfa.h
ext.obj		: ext.c
eval.obj	: eval.c
field.obj	: field.c
floatcomp.obj	: floatcomp.c
gawkaoi.obj	: gawkapi.c
gawkmisc.obj	: gawkmisc.c $(VMSDIR)gawkmisc.vms
getopt.obj	: $(SUPPORT)getopt.c
getopt1.obj	: $(SUPPORT)getopt1.c
int_array.obj	: int_array.c
io.obj		: io.c
localeinfo.obj  : $(SUPPORT)localeinfo.c
main.obj	: main.c
msg.obj		: msg.c
mpfr.obj	: mpfr.c
node.obj	: node.c
profile.obj	: profile.c
random.obj	: $(SUPPORT)random.c $(SUPPORT)random.h
re.obj		: re.c
regex.obj	: $(SUPPORT)regex.c $(SUPPORT)regcomp.c \
		  $(SUPPORT)regex_internal.c $(SUPPORT)regexec.c \
		  $(SUPPORT)regex.h $(SUPPORT)regex_internal.h
str_array.obj	: str_array.c
symbol.obj	: symbol.c
version.obj	: version.c
vms_misc.obj	: $(VMSDIR)vms_misc.c
vms_popen.obj	: $(VMSDIR)vms_popen.c
vms_fwrite.obj	: $(VMSDIR)vms_fwrite.c
vms_args.obj	: $(VMSDIR)vms_args.c
vms_gawk.obj	: $(VMSDIR)vms_gawk.c
vms_cli.obj	: $(VMSDIR)vms_cli.c
vms_crtl_init.obj : $(VMSDIR)vms_crtl_init.c
replace.obj	: replace.c $(MISSNGD)system.c $(MISSNGD)memcmp.c \
		  $(MISSNGD)memcpy.c $(MISSNGD)memset.c $(MISSNGD)memmove.c \
		  $(MISSNGD)strncasecmp.c $(MISSNGD)strerror.c \
		  $(MISSNGD)strftime.c $(MISSNGD)strchr.c $(MISSNGD)strtod.c \
		  $(MISSNGD)strtoul.c $(MISSNGD)tzset.c $(MISSNGD)mktime.c \
		  $(MISSNGD)snprintf.c $(MISSNGD)getaddrinfo.c \
		  $(MISSNGD)usleep.c \
		  $(MISSNGD)setenv.c $(MISSNGD)strcoll.c $(MISSNGD)wcmisc.c

# bison or yacc have not been ported to current VMS versions
# When that changes, this can be restored.
# bison or yacc required
# awkgram.c	: awkgram.y	# foo.y :: yacc => y[_]tab.c, bison => foo_tab.c
#     @- if f$search("ytab.c")	.nes."" then  delete ytab.c;*	 !POSIX yacc
#     @- if f$search("y_tab.c")	.nes."" then  delete y_tab.c;*	 !DEC/Shell yacc
#     @- if f$search("awkgram_tab.c").nes."" then  delete awkgram_tab.c;* !bison
#      - $(PARSERINIT)
#	$(PARSER) $(YFLAGS) $<
#     @- if f$search("ytab.c")	.nes."" then  rename/new_vers ytab.c  $@
#     @- if f$search("y_tab.c")	.nes."" then  rename/new_vers y_tab.c $@
#     @- if f$search("awkgram_tab.c").nes."" then \
#            rename/new_vers awkgram_tab.c $@

# command.c	: command.y
#     @- if f$search("ytab.c")	.nes."" then  delete ytab.c;*
#     @- if f$search("y_tab.c")	.nes."" then  delete y_tab.c;*
#     @- if f$search("command_tab.c").nes."" then  delete command_tab.c;*
#      - $(PARSERINIT)
#	$(PARSER) $(YFLAGS) $<
#     @- if f$search("ytab.c")	.nes."" then  rename/new_vers ytab.c  $@
#     @- if f$search("y_tab.c")	.nes."" then  rename/new_vers y_tab.c $@
#     @- if f$search("command_tab.c").nes."" then \
#            rename/new_vers command_tab.c $@

config_vms.h : $(VMSDIR)generate_config_vms_h_gawk.com
     $ @$(VMSDIR)generate_config_vms_h_gawk.com

config.h	: configh.in config_vms.h $(VMSDIR)config_h.com
     $ @$(VMSDIR)config_h.com

$(VMSCMD)	: $(VMSDIR)gawk.cld
	set command $(CLDFLAGS)/object=$@ $<

# special target for loading the help text into a VMS help library
install.help	: $(VMSDIR)gawk.hlp
	library/help $(HELPLIB) $< /log


# Build dynamic extensions - Alpha/Itanium only.
.ifdef __VAX__
# VAX not complete yet.
plug_opt = [.VMS.VAX]gawk_plugin_xfer.opt
.else
plug_opt = [.vms]gawk_plugin.opt
.endif

ext_gawkdirfd_h = [.extension]gawkdirfd.h config.h nonposix.h

extensions : filefuncs.exe fnmatch.exe inplace.exe ordchr.exe readdir.exe \
	revoutput.exe revtwoway.exe rwarray.exe testext.exe time.exe

filefuncs.exe : filefuncs.obj stack.obj gawkfts.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), stack.obj, gawkfts.obj, \
		$(plug_opt)/opt

fnmatch.exe : fnmatch.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

inplace.exe : inplace.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

ordchr.exe : ordchr.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

readdir.exe : readdir.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

revoutput.exe : revoutput.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

revtwoway.exe : revtwoway.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

rwarray.exe : rwarray.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

testext.exe : testext.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

time.exe : time.obj $(plug_opt)
	link/share=$(MMS$TARGET) $(MMS$SOURCE), $(plug_opt)/opt

stack.obj : [.extension]stack.c config.h gawkapi.h \
	[.extension]gawkfts.h, [.extension]stack.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

gawkfts.obj : [.extension]gawkfts.c config.h [.extension]gawkfts.h \
	$(ext_gawkdirfd_h)
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H, ZOS_USS, "fchdir(x)=(-1)") \
	/object=$(MMS$TARGET) $(MMS$SOURCE)

filefuncs.obj : [.extension]filefuncs.c config.h gawkapi.h \
	[.extension]gawkfts.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

fnmatch.obj : [.extension]fnmatch.c config.h gawkapi.h \
	[.missing_d]fnmatch.h [.missing_d]fnmatch.c
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

inplace.obj : [.extension]inplace.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

ordchr.obj : [.extension]ordchr.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

readdir.obj : [.extension]readdir.c config.h gawkapi.h \
	$(ext_gawkdirfd_h)
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H, HAVE_DIRENT_H) \
	/object=$(MMS$TARGET) $(MMS$SOURCE)

revoutput.obj : [.extension]revoutput.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

revtwoway.obj : [.extension]revtwoway.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H, HAVE_GETDTABLESIZE) \
	/object=$(MMS$TARGET) $(MMS$SOURCE)

rwarray.obj : [.extension]rwarray.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

testext.obj : [.extension]testext.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)

time.obj : [.extension]time.c config.h gawkapi.h
    $(CC)$(CEFLAGS)/define=(HAVE_CONFIG_H)/object=$(MMS$TARGET) $(MMS$SOURCE)


# miscellaneous other targets
tidy :
      - if f$search("*.*;-1").nes."" then  purge
      - if f$search("[.*]*.*;-1").nes."" then  purge [.*]

clean :
      - if f$search ("*.obj")    .nes. "" then delete *.obj;*
      - if f$search ("*.lis")    .nes. "" then delete *.lis;*
      - if f$search ("gawk.opt") .nes. "" then delete gawk.opt;*

spotless : clean tidy
      - if f$search("config.h").nes."" then delete config.h;*
      - if f$search("config_vms.h").nes."" then delete config_vms.h;*
      - if f$search("gawk.exe").nes."" then  delete gawk.exe;*
      - if f$search("*.dsf").nes."" then  delete *.dsf;*
      - if f$search("*.map").nes."" then  delete *.map;*
      - if f$search("*.pcsi$desc").nes."" then  delete *.pcsi$desc;*
      - if f$search("*.pcsi$text").nes."" then  delete *.pcsi$text;*
      - if f$search("gawk*_src.bck").nes."" then  delete gawk*_src.bck;*
      - if f$search("*.release_notes").nes."" then  delete *.release_notes;*
      - if f$search("filefuncs.exe").nes."" then  delete filefuncs.exe;*
      - if f$search("fnmatch.exe").nes."" then  delete fnmatch.exe;*
      - if f$search("inplace.exe").nes."" then  delete inplace.exe;*
      - if f$search("ordchr.exe").nes."" then  delete ordchr.exe;*
      - if f$search("readdir.exe").nes."" then  delete readdir.exe;*
      - if f$search("revoutput.exe").nes."" then  delete revoutput.exe;*
      - if f$search("revtwoway.exe").nes."" then  delete revtwoway.exe;*
      - if f$search("rwarray.exe").nes."" then  delete rwarray.exe;*
      - if f$search("testext.exe").nes."" then  delete testext.exe;*
      - if f$search("time.exe").nes."" then  delete time.exe;*
      - if f$search("gawk_verb.cld").nes."" then  delete gawk_verb.cld;*
      - if f$search("gawk.dvi").nes."" then  delete gawk.dvi;*
      - if f$search("[.doc]texindex.exe").nes."" then \
            delete [.doc]texindex.exe;*
      - if f$search("[.cxx_repository]*.*;").nes."" then \
            delete [.cxx_repository]*.*;*

#
# Note: this only works if you kept a copy of [.support]texindex.c
# from a gawk 2.x distribution and put it into [.doc]texindex.c.
# Also, depending on the fonts available with the version of TeX
# you have, you might need to edit [.doc]texinfo.tex and change
# the reference to "lcircle10" to be "circle10".
#
# build gawk.dvi from within the 'doc' subdirectory
#
gawk.dvi : [.doc]texindex.exe [.doc]gawk.texi
      @ set default [.doc]
      @ write sys$output " Warnings from TeX are expected during the first pass"
	TeX gawk.texi
	mcr []texindex gawk.cp gawk.fn gawk.ky gawk.pg gawk.tp gawk.vr
      @ write sys$output " Second pass"
	TeX gawk.texi
	mcr []texindex gawk.cp gawk.fn gawk.ky gawk.pg gawk.tp gawk.vr
      @ write sys$output " Third (final) pass"
	TeX gawk.texi
     -@ purge
     -@ delete \
         gawk.lis;,.aux;,gawk.%%;,.cps;,.fns;,.kys;,.pgs;,.toc;,.tps;,.vrs;
      @ rename/new_vers gawk.dvi [-]*.*
      @ set default [-]

# Note: [.doc]texindex.c is not included with the gawk 3.x distribution.
# Expect lots of "implicitly declared function" diagnostics from DEC C.
#
[.doc]texindex.exe : [.doc]texindex.c
      @ set default [.doc]
	$(CC) /noOpt/noList/Define=("lines=tlines") texindex.c
      @ open/Write opt texindex.opt
      @ write opt "texindex.obj"
      @ write opt "$(LIBS)"
      @ close opt
	$(LINK) /noMap/Exe=texindex.exe texindex.opt/Options
     -@ delete texindex.obj;*,texindex.opt;*
      @ set default [-]

#eof
