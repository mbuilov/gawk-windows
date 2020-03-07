$! vmstest.com -- DCL script to perform test/Makefile actions for VMS
$!
$! Usage:
$!  $ set default [-.test]
$!  $ @[-.vms]vmstest.com bigtest
$! This assumes that newly built gawk.exe is in the next directory up.
$!
$! Note to other maintainers:
$! *  when this test script is run via automation through RSH or SSH,
$!    on non-VAX, for some reason a second empty version of the output
$!    file is generated.
$!    For the tests to pass, this must be detected and deleted.
$!    This issue does not show up when running this script from
$!    an interactive SSH session.
$! *  The tests where Gawk could return an error code need to be
$!    wrapped in a set On / set noOn pair or it will abort this
$!    script, leaving behind an open "junit" file logical in DCL.
$!
$! 3.1.7:  changed to share code among many common tests, and
$!	   to put results for test foo into _foo.tmp instead of tmp.
$!
$! 4.0.71:  New tests:
$!	functab1,functab2,functab3,id,incdupe,incdupe2,	incdupe3,include2
$!	symtab1,symtab2,symtab3,symtab4,symtab5,symtab6
$!
$! 4.0.75:  New  tests
$!	basic:	rri1,getline5,incdupe4,incdupe5,incdupe6,incdupe7
$!	ext:	colonwarn,reginttrad,symtab7,symtab8,symtab9
$!
$! 4.1.1: New tests implemented in 4.1.3a
$!
$!	basic:	badassign1,dfamb1,exit2,nfloop
$!	ext:	profile4,profile5
$!	locale:	backbigs1,backsmalls1,backsmalls2,mbprintf4
$!              backgigs1 requires a locale not supplied with VMS.
$!
$! 4.1.2: New tests implemented in 4.1.3a
$!	basic:	badbuild,callparam,exitval3,inpref,paramasfunc1,
$!		paramasfunc2,regexpbrack,sortglos
$!	ext:	crlf,dbugeval,fpat4,genpot,indirectbuiltin,
$!		printfbad4,printhuge,profile0,profile6,profile7,
$!		regnul1,regnul2,rsgetline,rsglstdin
$!	charset_all: charset_msg_start,charset_tests,charset_msg_end
$!
$! 4.1.3: New tests - implemented in 4.1.3a
$!	basic:	rebrackloc
$!
$! 4.1.3a: New tests
$!	vms_debug: Not a test, switches to a debug gawk for following tests
$!	basic:	aryprm9,exitval2,nulinsrc,rstest4,rstest5
$!	ext:	mbstr2,profile8,watchpoint1
$!      shlib:  inplace2,testext,lc_num1,mbprintf1
$!		mbprintf2,mbfprintf3,mbfprintf4,mbfw1
$!
$! 4.1.3c: New tests
$!      ext:	clos1way2, clos1way3, clos1way4, clos1way5
$!
$! 4.1.3f: New tests
$!      basic:  arrayind1,rscompat,sigpipe1
$!
$! 4.1.3.g: New tests
$!      basic:  fsnul1, hex2, mixed1, subback
$!	ext:    fpat4, symtab10
$!
$! 4.1.3.i: New tests
$!	basic:  ofmtstrnum
$!	extra:  ignrcas3
$!
$! 4.2.0: New tests
$!      basic:   aryunasgn, concat5, memleak
$!      ext:     fpat5, fpat6, fwtest5, fwtest6, fwtest7, fwtest8,
$!               sourcesplit
$!      charset: mbprintf5
$!
$! 4.2+: New tests
$!      basic:   numstr1, setrec0, setrec1
$!      extra:   isarrayunset, nlstringtest
$!
$!
$	echo	= "write sys$output"
$	cmp	= "diff/Output=_NL:/Maximum=1"
$	delsym  = "delete/symbol/local/nolog"
$	igncascmp = "''cmp'/Ignore=Case"
$	sumslp  = "edit/Sum"
$	rm	= "delete/noConfirm/noLog"
$	mv	= "rename/New_Vers"
$	sort	= "sort"
$	gawk = "$sys$disk:[-]gawk"
$	AWKPATH_srcdir = "define/User AWKPATH sys$disk:[]"
$	AWKLIBPATH_dir = "define/User AWKLIBPATH sys$disk:[-]"
$	arch_name = f$edit(f$getsyi("arch_name"),"upcase,trim")
$
$!	Make sure that the default directory exists on a search list.
$	def_dir = f$environment("default")
$	create/dir 'def_dir'
$	listdepth = 0
$	pipeok = 0
$	floatmode = -1	! 0: D_float, 1: G_float, 2: IEEE T_float
$
$	list = p1+" "+p2+" "+p3+" "+p4+" "+p5+" "+p6+" "+p7+" "+p8
$	list = f$edit(list,"TRIM,LOWERCASE")
$	if list.eqs."" then  list = "all"	! bigtest
$	gosub create_junit_test_header
$	gosub list_of_tests
$	gosub finish_junit_test
$	echo "done."
$	exit
$!
$create_junit_test_header:
$	junit_count = 0
$	temp_fdl = "sys$disk:[]stream_lf.fdl"
$	arch_code = f$extract(0, 1, arch_name)
$!
$       if f$trnlnm("junit",,, "SUPERVISOR") .nes. ""
$       then
$           close junit
$       endif
$       if f$trnlnm("junit_hdr",,, "SUPERVISOR") .nes. ""
$       then
$           close junit_hdr
$       endif
$	junit_hdr_file = "sys$disk:[]test_output.xml"
$	if f$search(junit_hdr_file) .nes. "" then delete 'junit_hdr_file';*
$	junit_body_file = "sys$disk:[]test_body_tmp.xml"
$	if f$search(junit_body_file) .nes. "" then delete 'junit_body_file';*
$!!
$	if arch_code .nes. "V"
$	then
$	    create 'junit_hdr_file'/fdl="RECORD; FORMAT STREAM_LF;"
$	    create 'junit_body_file'/fdl="RECORD; FORMAT STREAM_LF;"
$	else
$	    if f$search(temp_fdl) .nes. "" then delete 'temp_fdl';*
$	    create 'temp_fdl'
RECORD
	FORMAT		stream_lf
$	    continue
$	    create 'junit_hdr_file'/fdl='temp_fdl'
$	    create 'junit_body_file'/fdl='temp_fdl'
$	endif
$	open/append junit 'junit_body_file'
$	if f$search(temp_fdl) .nes. "" then delete 'temp_fdl';*
$	return
$!
$finish_junit_test:
$	open/append junit_hdr 'junit_hdr_file'
$	write junit_hdr "<?xml version=""1.0"" encoding=""UTF-8""?>"
$	write junit_hdr "<testsuite name=""gawk"""
$	write junit_hdr " tests=""''junit_count'"">"
$	close junit_hdr
$	write junit "</testsuite>"
$	close junit
$	append 'junit_body_file' 'junit_hdr_file'
$       delete 'junit_body_file';*
$	return
$!
$junit_report_skip:
$	write sys$output "Skipping test ''test' reason ''skip_reason'."
$	junit_count = junit_count + 1
$	write junit "  <testcase name=""''test'"""
$	write junit "   classname=""''test_class'"">"
$	write junit "     <skipped/>"
$	write junit "  </testcase>"
$	return
$!
$junit_report_fail_diff:
$	fail_msg = "failed"
$	fail_type = "diff"
$!	fall through to junit_report_fail
$junit_report_fail:
$	write sys$output "failing test ''test' reason ''fail_msg'."
$	junit_count = junit_count + 1
$	write junit "  <testcase name=""''test'"""
$	write junit "   classname=""''test_class'"">"
$	write junit -
  "     <failure message=""''fail_msg'"" type=""''fail_type'"" >"
$	write junit "     </failure>"
$	write junit "  </testcase>"
$	return
$!
$junit_report_pass:
$	junit_count = junit_count + 1
$	write junit "  <testcase name=""''test'"""
$	write junit "   classname=""''test_class'"">"
$	write junit "  </testcase>"
$	return
$
$vms_debug:	echo "Switching to gawk_debug.exe"
$		gawk = "$sys$disk:[-]gawk_debug.exe"
$		return
$!
$all:
$
$bigtest:	echo "bigtest..."
$		! omits "printlang" and "extra"
$		list = "basic unix_tests gawk_ext vms_tests charset_tests" -
		  + " machine_tests"
$		gosub list_of_tests
$		return
$
$basic:		echo "basic..."
$		list = "msg addcomma anchgsub argarray arrayind1" -
		  + " arrayparm arrayref arrayprm2 arrayprm3 arrymem1" -
		  + " arryref2 arryref3 arryref4 arryref5 arynasty" -
		  + " arynocls aryprm1 aryprm2 aryprm3 aryprm4 aryprm5" -
		  + " aryprm6 aryprm7 aryprm8 aryprm9 arysubnm aryunasgn" -
                  + " asgext awkpath" -
		  + " back89 backgsub badassign1 badbuild"
$		gosub list_of_tests
$		list = "callparam childin clobber closebad clsflnam" -
		  + " compare compare2 concat1 concat2 concat3 concat4" -
		  + " concat5 convfmt " -
		  + " datanonl defref delargv delarprm delarpm2 delfunc" -
		  + " dfamb1 dfastress dynlj" -
		  + " eofsplit exit2 exitval1 exitval2 exitval3"
$		gosub list_of_tests
$		list = "fcall_exit fcall_exit2 fldchg fldchgnf" -
		  + " fnamedat fnarray fnarray2 fnaryscl fnasgnm fnmisc" -
		  + " fordel forref forsimp fsbs fsnul1 fsspcoln fsrs" -
		  + " fstabplus funsemnl funsmnam funstack"
$		gosub list_of_tests
$		list = "getline getline2 getline3 getline4 getline5 " -
		  + " getlnbuf getnr2tb getnr2tm gsubasgn gsubtest" -
		  + " gsubtst2 gsubtst3 gsubtst4 gsubtst5 gsubtst6" -
		  + " gsubtst7 gsubtst8" -
		  + " hex hex2 hsprint" -
		  + " inpref inputred intest intprec iobug1" -
		  + " leaddig leadnl litoct longsub longwrds"-
		  + " manglprm math membug1 messages memleak minusstr mixed1" -
		  + " mmap8k mtchi18n"
$		gosub list_of_tests
$		list = "nasty nasty2 negexp negrange nested nfldstr" -
		  + " nfloop nfneg nfset nlfldsep nlinstr nlstrina" -
		  + " noeffect nofile nofmtch noloop1 noloop2 nonl" -
		  + " noparms nors nulinsrc nulrsend numindex numstr1" -
		  + " numsubstr" -
		  + " octsub ofmt ofmtbig ofmtfidl" -
		  + " ofmta ofmts ofs1 onlynl opasnidx opasnslf
$		gosub list_of_tests
$		list = "paramasfunc1 paramasfunc2 paramdup" -
		  + " paramres paramtyp paramuninitglobal" -
		  + " parse1 parsefld parseme pcntplus posix2008sub" -
		  + " prdupval prec printf0 printf1 prmarscl prmreuse" -
		  + " prt1eval prtoeval"
$		gosub list_of_tests
$		list = "rand range1 rebrackloc rebt8b1 redfilnm regeq" -
		  + " regexpbrack regexprange regrange reindops reparse" -
		  + " resplit rri1 rs rscompat rsnul1nl rsnulbig rsnulbig2" -
		  + " rstest1 rstest2 rstest3 rstest4 rstest5 rswhite"
$		gosub list_of_tests
$		list = "scalar sclforin sclifin setrec0 setrec1 sigpipe1" -
		  + " sortempty sortglos" -
		  + " splitargv splitarr splitdef splitvar splitwht" -
		  + " strcat1 strtod strnum1 subamp subback subi18n subsepnm" -
		  + " subslash substr swaplns synerr1 synerr2"
$		gosub list_of_tests
$		list = "tradanch tweakfld" -
		  + " uninit2 uninit3 uninit4 uninit5 uninitialized" -
		  + " unterm uparrfs" -
		  + " wideidx wideidx2 widesub widesub2 widesub3 widesub4" -
		  + " wjposer1" -
		  + " zeroe0 zeroflag zero2"
$		gosub list_of_tests
$		return
$!
$unix:
$unix_tests:	echo "unix_tests..."
$		list = "fflush getlnhd localenl pid pipeio1 pipeio2" -
		  + " poundbang rtlen rtlen01 space strftlng"
$		gosub list_of_tests
$		return
$
$gnu:
$gawk_ext:	echo "gawk_ext... (gawk.extensions)"
$		list = "aadelete1 aadelete2 aarray1 aasort aasorti" -
		  + " argtest arraysort" -
		  + " backw badargs beginfile1 binmode1" -
		  + " colonwarn clos1way clos1way2 clos1way3 clos1way4" -
		  + " clos1way5 charasbytes crlf" -
		  + " dbugeval delsub devfd devfd1 devfd2 dumpvars" -
		  + " exit" -
		  + " fieldwdth fpat1 fpat2 fpat3 fpat4 fpat5 fpat6 " -
		  + " fpatnull funlen functab1 functab2 functab3 fsfwfs " -
		  + " fwtest fwtest2 fwtest3 fwtest4 fwtest5 fwtest6 " -
		  + " fwtest7 fwtest8"
$		gosub list_of_tests
$		list = "genpot gensub gensub2 getlndir gnuops2 gnuops3" -
		  + " gnureops" -
		  + " icasefs id icasers igncdym igncfs ignrcase ignrcas2" -
		  + " incdupe incdupe2 incdupe3 incdupe4 incdupe5" -
		  + " incdupe6 incdupe7 include2 indirectbuiltin indirectcall"
$		gosub list_of_tests
$		list = "lint lintold lintwarn" -
		  + " match1 match2 match3 manyfiles mbprintf3 mbstr1 mbstr2" -
		  + " nastyparm next nondec nondec2" -
		  + " patsplit posix printfbad1 printfbad2 printfbad3" -
		  + " printfbad4 printhuge procinfs profile0 profile1" -
		  + " profile2 profile3 profile4 profile5 profile6" -
		  + " profile7 profile8 pty1"
$		gosub list_of_tests
$		list = "regx8bit rebuf reginttrad regnul1 regnul2" -
		  + " reint reint2 rsgetline rsglstdin rsstart1 rsstart2" -
		  + " rsstart3 rstest6" -
		  + " shadow sortfor sortu sourcesplit split_after_fpat" -
		  + " splitarg4 strtonum strftime switch2 symtab1 symtab2" -
		  + " symtab3 symtab4 symtab5 symtab6 symtab7 symtab8" -
		  + " symtab9 symtab10 watchpoint1"
$		gosub list_of_tests
$		return
$
$vms:
$vms_tests:	echo "vms_tests..."
$		list = "vms_cmd vms_io1 vms_io2"
$		gosub list_of_tests
$		return
$
$locale_tests:
$charset_tests:	echo "charset_tests..."
$		! without i18n kit, VMS only supports the C locale
$		! and several of these fail
$		list = "asort asorti" -
		  + " backbigs1 backsmalls1 backsmalls2" -
		  + " fmttest fnarydel fnparydl" -
		  + " lc_num1 mbfw1" -
		  + " mbprintf1 mbprintf2 mbprintf4 mbprintf5" -
		  + " rebt8b2 rtlenmb" -
		  + " sort1 sprintfc"
$		gosub list_of_tests
$		return
$!
$charset_all:  echo "All charset tests..."
$		list = "charset_msg_start charset_tests charset_msg_end"
$		gosub list_of_tests
$		return
$
$hardware:
$machine_tests:	echo "machine_tests..."
$		! these depend upon the floating point format in use
$		list = "double1 double2 fmtspcl intformat"
$		gosub list_of_tests
$		return
$
$extra:		echo "extra..."
$		list = "regtest inftest inet ignrcas3 isarrayunset" -
                  + " nlstringtest"
$		gosub list_of_tests
$		return
$
$inet:		echo "inet..."
$		type sys$input:
 The inet tests only work if gawk has been built with TCP/IP socket
 support and your system supports the services "discard" at port 9
 and "daytimed" at port 13.
$		list = "inetechu inetecht inetdayu inetdayt"
$		gosub list_of_tests
$		return
$!
$shlib:
$extension:	echo "extension...."
$		list = "fnmatch filefuncs fork fork2 fts functab4" -
		  + " inplace1 inplace2" -
		  + " ordchr ordchr2" -
		  + " readdir readfile readfile2 revout revtwoway rwarray" -
		  + " testext time"
$		gosub list_of_tests
$		return
$!
$mpfr:
$		test_class = "mpfr"
$		skip_reason = "Not yet implmented on VMS"
$		! mpfr has not yet been ported to VMS.
$		gosub junit_report_skip
$		return
$!
$! list_of_tests: process 'list', a space-separated list of tests.
$! Some tests assign their own 'list' and call us recursively,
$! so we have to emulate a local stack to prevent the current list
$! from being clobbered by nested execution.  We do this by making
$! and operating on list1, list2, &c depending upon call depth.
$! Lower level tests access higher numbered entries; the lower
$! entries in use by caller or caller's caller are kept intact.
$list_of_tests:
$		listdepth = listdepth + 1	! increment calling depth
$		list'listdepth' = f$edit(list,"COMPRESS,TRIM")
$next_test:
$		test = f$element(0," ",list'listdepth')
$		list'listdepth' = list'listdepth' - test - " "
$		gosub 'test'
$		if list'listdepth'.nes.""  then  goto next_test
$		listdepth = listdepth - 1	! reset
$		return
$
$
$! common tests, not needing special set up: gawk -f 'test'.awk 'test'.in
$addcomma:
$anchgsub:
$asgext:
$back89:
$backgsub:
$concat1:
$datanonl:
$dfamb1:
$fldchg:
$fldchgnf:
$fsnul1:
$fsrs:
$funstack:
$getline4:
$getnr2tb:
$getnr2tm:
$gsubtst5:
$gsubtst7:
$gsubtst8:
$hex2:
$inpref:
$leadnl:
$manglprm:
$membug1:
$nested:
$nfset:
$nlfldsep:
$nlinstr:
$noloop1:
$noloop2:
$nulrsend:
$ofmt:
$ofmtfidl:
$ofmts:
$ofs1:
$onlynl:
$parse1:
$parsefld:
$prdupval:
$range1:
$rebrackloc:
$regeq:
$regexpbrack:
$reindops:
$reparse:
$rsnul1nl:
$rswhite:
$setrec0:
$sortglos:
$splitargv:
$splitvar:
$strtod:
$subback:
$swaplns:
$uparrfs:
$wjposer1:
$	test_class = "basic"
$       test_in = "''test'.in"
$	goto common_with_test_in_redir
$!
$sprintfc:
$	test_class = "charset_tests"
$       test_in = "''test'.in"
$	goto common_with_test_in_redir
$!
$backw:
$fpat1:
$fpat3:
$fpat5:
$fpat6:
$fpatnull:
$fsfwfs:
$funlen:
$fwtest:
$fwtest2:
$fwtest4:
$fwtest5:
$fwtest6:
$fwtest7:
$fwtest8:
$gensub:
$icasers:
$igncdym:
$igncfs:
$indirectcall:
$match3:
$rebuf:
$rsstart1:
$sortfor:
$split_after_fpat:
$splitarg4:
$       test_in = "''test'.in"
$	test_class = "gawk_ext"
$	goto common_with_test_in_redir
$!
$fwtest3:
$       test_in = "fwtest2.in"
$	test_class = "gawk_ext"
$	goto common_with_test_in_redir
$!
$common_with_test_in_redir:
$! common with 'test'.in redirected.
$!
$	echo "''test'"
$!	if f$search("sys$disk:[]_''test'.tmp2;*") .nes. ""
$!	then
$!	    delete sys$disk:[]_'test'.tmp2;*
$!	endif
$	if f$search("sys$disk:[]_''test'.tmp;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;*
$	endif
$       tmp_output = "_''test'.tmp"
$       set noOn
$	gawk -f 'test'.awk < 'test_in' >'tmp_output' 2>&1
$	gawk_status = $status
$	set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	if .not. gawk_status
$       then
$	    call exit_code 'gawk_status' _'test'.tmp
$       endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$! Gawk treats < 'test_in' differently than just 'test_in' on VMS
$! and some tests care about this.
$common_with_test_input:
$! common with 'test'.in
$!
$	echo "''test'"
$	if f$search("sys$disk:[]_''test'.tmp;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;*
$	endif
$       set noOn
$	gawk -f 'test'.awk 'test_in' >_'test'.tmp
$	gawk_status = $status
$	set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	if .not. gawk_status
$       then
$	    call exit_code 'gawk_status' _'test'.tmp
$       endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$!
$arrayind1:
$	echo "''test'"
$	test_class = "basic"
$       define/user sys$error _'test'.tmp
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp2
$       append _'test'.tmp2 _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;,_'test'.tmp2;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$indirectbuiltin: ! 4.1.2
$	echo "''test'"
$	test_class = "gawk_ext"
$	! No shell simulation in gawk
$	temp_file = "sys$disk:[]_'test'.tmp"
$	if f$search(temp_file) .nes. "" then delete 'temp_file';*
$	old_rm = rm
$	rm = "continue ! "
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	rm = old_rm
$	! gawk subprocesses creating new generation of stdout
$	! instead of appending to open one.
$	cmp 'test'.ok sys$disk:[]_'test'.tmp;1
$	if $status
$	then
$	    rm _'test'.tmp;*,x1.out;,x2.out;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$! Sourcesplit test
$sourcesplit:
$	echo "''test'"
$	test_class = "gawk_ext"
$       source1 = "BEGIN { a = 5;"
$       source2 = "print a }"
$	if f$search("sys$disk:[]_''test'.tmp2;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp2;*
$	endif
$       set noOn
$       define/user sys$error _'test'.tmp
$	gawk --source="''source1'" --source="''source2'" >_'test'.tmp2
$	gawk_status = $status
$	if f$search("sys$disk:[]_''test'.tmp2;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp2;2
$	endif
$	if .not. gawk_status then call exit_code 'gawk_status' _'test'.tmp
$       set On
$       append _'test'.tmp2 _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;,_'test'.tmp2;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$
$! more common tests, without a data file: gawk -f 'test'.awk
$ofmta:
$  	if arch_name .eqs. "VAX"
$	then
$	    test_class = "basic"
$	    skip_reason = "skipped on VAX"
$	    gosub junit_report_skip
$	    return
$	endif
$arrayref:
$arryref2:
$arrymem1:
$arynasty:
$arysubnm:
$aryunasgn:
$badassign1:
$badbuild:
$callparam:
$closebad:
$compare2:
$concat2:
$concat3:
$concat5:
$convfmt:
$delargv:
$delarpm2:
$delarprm:
$dynlj:
$exit2:
$fordel:
$forref:
$forsimp:
$funsemnl:
$getline3:
$hsprint:
$intest:
$math:
$memleak:
$minusstr:
$negrange:
$nulinsrc:
$numstr1:
$nlstrina:
$octsub:
$ofmtstrnum:
$paramtyp:
$paramuninitglobal:
$pcntplus:
$printf1:
$prt1eval:
$rebt8b1:
$regexprange:
$regrange:
$setrec1:
$splitdef:
$splitwht:
$strnum1:
$substr:
$zero2:
$zeroflag:
$	test_class = "basic"
$	goto common_without_test_in
$isarrayunset:
$	test_class = "extra"
$	goto common_without_test_in
$!
$aarray1:
$aasort:
$aasorti:
$arraysort:
$delsub:
$fpat2:
$gensub2:
$gnuops2:
$gnuops3:
$gnureops:
$icasefs:
$match1:
$nondec:
$patsplit:
$procinfs:
$regx8bit:
$sortu:
$strtonum:
$switch2:
$	test_class = "gawk_ext"
$	goto common_without_test_in
$!
$asort:
$asorti:
$fnarydel:
$fnparydl:
$rebt8b2:
$sort1:
$	test_class = "charset_tests"
$	goto common_without_test_in
$!
$common_without_test_in:
$! common without 'test'.in
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 2>&1 >_'test'.tmp
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$colonwarn:	echo "''test'"
$	test_class = "gawk_ext"
$	gawk -f 'test'.awk 1 < 'test'.in > _'test'.tmp
$	gawk -f 'test'.awk 2 < 'test'.in > _'test'_2.tmp
$	gawk -f 'test'.awk 3 < 'test'.in > _'test'_3.tmp
$	if f$search("sys$disk:[]_''test'_%.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'_%.tmp;2
$	endif
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	append _'test'_2.tmp,_'test'_3.tmp _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp;1
$	if $status
$	then
$	    rm _'test'*.tmp;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$dbugeval:
$	echo "''test'"
$	test_class = "gawk_ext"
$	if f$getdvi("SYS$COMMAND", "TRM")
$	then
$	    set noOn
$	    gawk --debug -f /dev/null < 'test'.in 2>&1 > _'test.tmp'
$	    if .not. $status then call exit_code '$status' _'test'.tmp
$	    set On
$	    cmp 'test'.ok sys$disk:[]_'test'.tmp;1
$	    if $status
$	    then
$		rm _'test'*.tmp;*
$		gosub junit_report_pass
$	    else
$		gosub junit_report_fail_diff
$	    endif
$	else
$	    skip_reason = "Skipping because not a terminal."
$	    gosub junit_report_skip
$	endif
$	return
$!
$rsglstdin:
$	echo "''test'"
$	test_class = "gawk_ext"
$	set noOn
$	define/user sys$input rsgetline.in
$	if f$search("sys$disk:[]_''test'.tmp;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;*
$	endif
$	gawk -f rsgetline.awk 2>&1 > _'test'.tmp
$       gawk_status = $status
$	set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	if .not. gawk_status then call exit_code 'gawk_status' _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$genpot:
$	echo "''test'"
$	test_class = "gawk_ext"
$	set noOn
$	gawk -f 'test'.awk --gen-pot 2>&1 >_'test'.tmp
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$paramasfunc1:
$paramasfunc2:
$	echo "''test'"
$	test_class = "gawk_ext"
$	set noOn
$	gawk -f 'test'.awk --posix 2>&1 >_'test'.tmp
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$watchpoint1:
$	echo "''test'"
$	test_class = "gawk_ext"
$	set noOn
$	gawk "-D" -f 'test'.awk 'test'.in < 'test'.script 2>&1 >_'test'.tmp
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$!
$fork:		! 4.0.2
$fork2:		! 4.0.2
$	test_class = "shlib"
$	skip_reasons = "''test' not implemented on VMS"
$	gosub junit_report_skip
$	return
$!
$testext:
$	echo "''test'"
$	test_class = "shlib"
$  	if arch_name .eqs. "VAX"
$	then
$	    skip_reason = "ODS-5 required"
$	    gosub junit_report_skip
$	    return
$	endif
$	set process/parse=extended ! ODS-5 only
$	gawk "/^(@load|BEGIN)/,/^}/" [-.extension]'test'.c > _'test'.awk
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f _'test'.awk >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	gawk "{gsub(""no children"",""No child processes"")}1" -
	    _'test'.tmp >_'test'.tmp1
$	rm sys$disk:[]_'test'.tmp;*
$	mv sys$disk:[]_'test'.tmp1 sys$disk:[]_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$!
$double1:
$double2:
$	echo "''test'"
$	test_class = "hardware"
$	skip_reason = "skipped"
$	gosub junit_report_skip
$	return
$
$getline5:	echo "''test'"
$	test_class = "basic"
$	! Use of echo and rm inside the awk script makes it necessary
$	! for some temporary redefinitions. The VMS gawk.exe also creates
$	! multiple output files. Only the first contains the data.
$	old_echo = echo
$	old_rm = rm
$	echo = "pipe write sys$output"
$	rm = "!"
$	gawk -f 'test'.awk > _'test'.tmp
$	echo = old_echo
$	rm   = old_rm
$	delsym old_echo
$	delsym old_rm
$	cmp 'test'.ok sys$disk:[]_'test'.tmp;1
$	if $status
$	then
$	    rm _'test'.tmp;*,f.;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$msg:
$	! first show gawk's version (without copyright notice)
$	gawk --version >_msg.tmp
$	gawk "FNR == 1 {print; exit}" _msg.tmp
$	rm _msg.tmp;
$	echo "Any output from ""DIF"" is bad news, although some differences"
$	echo "in floating point values are probably benign -- in particular,"
$	echo "some systems may omit a leading zero and the floating point"
$	echo "precision may lead to slightly different output in a few cases."
$	echo ""
$	return
$!
$charset_msg_start:
$ echo "======== Starting tests that can vary based on character set ========"
$ echo "======== or locale support                                   ========"
$ echo "*********************************************************************"
$ echo "* Some or all of these tests may fail if you have inadequate or     *"
$ echo "* missing locale support. At least en_US.UTF-8, ru_RU.UTF-8 and     *"
$ echo "* ja_JP.UTF-8 are needed. However, if you see this message, then    *"
$ echo "* this test script thinks you have what you need ...                *"
$ echo "*********************************************************************"
$ echo ""
$	return
$!
$charset_msg_end:
$ echo "======== Done with tests that can vary based on character set ========"
$ echo "======== or locale support                                    ========"
$ echo ""
$	return
$!
$printlang:
$! the default locale is C, with LC_LANG and LC_ALL left empty;
$! showing that at the very beginning may cause some confusion about
$! whether gawk requires those, so we don't run this as the first test
$	gawk -f printlang.awk
$	return
$
$poundbang:
$	echo "''test'"
$	test_class = "unix_tests"
$	skip_reason = "not supported"
$	gosub junit_report_skip
$	return
$!
$pty1:
$	echo "''test'"
$	test_class = "gawk_ext"
$	skip_reason = "not supported"
$	gosub junit_report_skip
$	return
$!
$rscompat:
$	echo "''test'"
$	test_class = "basic"
$	gawk --traditional -f 'test'.awk 'test'.in >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$
$
$messages:	echo "''test'"
$	test_class = "basic"
$	set noOn
$	gawk -f 'test'.awk > _out2 >& _out3
$	cmp out1.ok sys$disk:[]_out1.
$	if $status then  rm _out1.;
$	cmp out2.ok sys$disk:[]_out2.
$	if $status then  rm _out2.;
$	cmp out3.ok sys$disk:[]_out3.
$	if $status
$	then
$	    rm _out3.;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$argarray:	echo "argarray"
$	test_class = "basic"
$	copy sys$disk:[]argarray.in sys$disk:[]argarray.input
$       purge sys$disk:[]argarray.input
$	define/User TEST "test"			!this is useless...
$	gawk -f argarray.awk ./argarray.input - >_argarray.tmp
just a test
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp argarray.ok sys$disk:[]_argarray.tmp
$	if $status
$	then
$	    rm _argarray.tmp;,sys$disk:[]argarray.input;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fstabplus:	echo "fstabplus"
$	test_class = "basic"
$	gawk -f fstabplus.awk >_fstabplus.tmp
1		2
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp fstabplus.ok sys$disk:[]_fstabplus.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$longwrds:	echo "longwrds"
$	test_class = "basic"
$	gawk -v "SORT=sort sys$input: _longwrds.tmp" -f longwrds.awk longwrds.in >_NL:
$	cmp longwrds.ok sys$disk:[]_longwrds.tmp
$	if $status
$	then
$	    rm _longwrds.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fieldwdth:	echo "fieldwdth"
$	test_class = "gawk_ext"
$	gawk -v "FIELDWIDTHS=2 3 4" "{ print $2}" >_fieldwdth.tmp
123456789
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp fieldwdth.ok sys$disk:[]_fieldwdth.tmp
$	if $status
$	then
$	    rm _fieldwdth.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$ignrcase:	echo "ignrcase"
$	test_class = "gawk_ext"
$	gawk -v "IGNORECASE=1" "{ sub(/y/, """"); print}" >_ignrcase.tmp
xYz
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp ignrcase.ok sys$disk:[]_ignrcase.tmp
$	if $status
$	then
$	    rm _ignrcase.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$regtest:
$  echo "regtest"
$  test_class = "extra"
$  if f$search("regtest.com").eqs.""
$  then
$	skip_reason = "Not implemented on VMS"
$	gosub junit_report_skip
$  else
$	echo "regtest"
$	echo "Some of the output from regtest is very system specific, do not"
$	echo "be distressed if your output differs from that distributed."
$	echo "Manual inspection is called for."
$	@regtest.com
$	skip_reason = "Not implemented on VMS"
$	gosub junit_report_skip
$ endif
$	return
$
$posix: echo "posix"
$	test_class = "gawk_ext"
$	gawk -f posix.awk >_posix.tmp
1:2,3 4
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp posix.ok sys$disk:[]_posix.tmp
$	if $status
$	then
$	    rm _posix.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$manyfiles:	echo "manyfiles"
$	test_class = "gawk_ext"
$!! this used to use a hard-coded value of 300 simultaneously open
$!! files, but if our open file quota is generous enough then that
$!! wouldn't exercise the ability to handle more than the maximum
$!! number allowed at once
$	f_cnt = 300
$	chnlc = f$getsyi("CHANNELCNT")
$	fillm = f$getjpi("","FILLM")
$	if fillm.ge.chnlc then  fillm = chnlc - 1
$	if fillm.ge.f_cnt then  f_cnt = fillm + 10
$	if f$search("sys$disk:[.junk]*.*").nes.""
$	then
$	    rm sys$disk:[.junk]*.*;*
$	endif
$	if f$parse("sys$disk:[.junk]") .eqs. ""
$	then
$	    create/Dir/Prot=(O:rwed) sys$disk:[.junk]
$	endif
$	gawk -- "BEGIN {for (i = 1; i <= ''f_cnt'; i++) print i, i}" -
           >_manyfiles.dat
$	echo "(processing ''f_cnt' files; this may take quite a while)"
$	set noOn	! continue even if gawk fails
$	gawk -f manyfiles.awk _manyfiles.dat _manyfiles.dat
$	define/User sys$error _NL:
$	define/User sys$output _manyfiles.tmp
$	search/Match=Nor/Output=_NL:/Log sys$disk:[.junk]*.* ""
$!/Log output: "%SEARCH-S-NOMATCH, <filename> - <#> records" plus 1 line summary
$	gawk -v "F_CNT=''f_cnt'" -f - _manyfiles.tmp
$deck	!some input begins with "$"
$4 != 2 {++count}
END {if (NR != F_CNT+1 || count != 1) {print "\nFailed!"}}
$eod
$	set On
$	skip_reason = "Test detection not implemented yet"
$	gosub junit_report_skip
$	rm sys$disk:_manyfiles.tmp;,sys$disk:_manyfiles.dat;
$       rm sys$disk:[.junk]*.*;*,sys$disk:[]junk.dir;
$	return
$
$compare:	echo "compare"
$	test_class = "basic"
$	gawk -f compare.awk 0 1 compare.in >_compare.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp compare.ok sys$disk:[]_compare.tmp
$	if $status
$	then
$	    rm _compare.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rs:		echo "rs"
$	test_class = "basic"
$	gawk -v "RS=" "{ print $1, $2}" rs.in >_rs.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp rs.ok sys$disk:[]_rs.tmp
$	if $status
$	then
$	    rm _rs.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fsbs:		echo "fsbs"
$	test_class = "basic"
$	gawk -f fsbs.awk fsbs.in >_fsbs.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp fsbs.ok sys$disk:[]_fsbs.tmp
$	if $status
$	then
$	    rm _fsbs.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$inftest:	echo "inftest"
$	test_class = "extra"
$     !!  echo "This test is very machine specific..."
$	set noOn
$	gawk -f inftest.awk >_inftest.tmp
$     !!  cmp inftest.ok sys$disk:[]_inftest.tmp	!just care that gawk doesn't crash...
$	if $status
$	then
$	    rm _inftest.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$getline2:	echo "getline2"
$	test_class = "basic"
$	gawk -f getline2.awk getline2.awk getline2.awk >_getline2.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp getline2.ok sys$disk:[]_getline2.tmp
$	if $status
$	then
$	    rm _getline2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rand:		echo "rand"
$	test_class = "basic"
$	echo "The following line should just be 19 random numbers between 1 and 100"
$	echo ""
$	gawk -f rand.awk
$	skip_reason = "Test detection not implemented yet"
$	gosub junit_report_skip
$	return
$
$negexp:	echo "negexp"
$	test_class = "basic"
$	gawk "BEGIN { a = -2; print 10^a }" >_negexp.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp negexp.ok sys$disk:[]_negexp.tmp
$	if $status
$	then
$	    rm _negexp.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$awkpath:	echo "awkpath"
$	test_class = "basic"
$	define/User AWK_LIBRARY [],[.lib]
$	gawk -f awkpath.awk >_awkpath.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp awkpath.ok sys$disk:[]_awkpath.tmp
$	if $status
$	then
$	    rm _awkpath.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$argtest:	echo "argtest"
$	test_class = "gawk_ext"
$	gawk -f argtest.awk -x -y abc >_argtest.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp argtest.ok sys$disk:[]_argtest.tmp
$	if $status
$	then
$	    rm _argtest.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$badargs:	echo "badargs"
$	test_class = "gawk_ext"
$	on error then continue
$	gawk -f 2>&1 >_badargs.too
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$!	search/Match=Nor _badargs.too "patchlevel" /Output=_badargs.tmp
$	gawk "/patchlevel/{next}; {gsub(""\"""",""'""); print}" <_badargs.too >_badargs.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp badargs.ok sys$disk:[]_badargs.tmp
$	if $status
$	then
$	    rm _badargs.tmp;,_badargs.too;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nonl:		echo "nonl"
$	test_class = "basic"
$	! This one might fail, depending on the tool used to unpack the
$	! distribution.  Some will add a final newline if the file lacks one.
$	AWKPATH_srcdir
$	gawk --lint -f nonl.awk _NL: >_nonl.tmp 2>&1
$	cmp nonl.ok sys$disk:[]_nonl.tmp
$	if $status
$	then
$	    rm _nonl.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$defref:	echo "defref"
$	test_class = "basic"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f defref.awk >_defref.tmp 2>&1
$	if .not. $status then call exit_code '$status' _defref.tmp
$	set On
$	cmp defref.ok sys$disk:[]_defref.tmp
$	if $status
$	then
$	    rm _defref.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nofmtch:	echo "nofmtch"
$	test_class = "basic"
$	AWKPATH_srcdir
$	gawk --lint -f nofmtch.awk >_nofmtch.tmp 2>&1
$	cmp nofmtch.ok sys$disk:[]_nofmtch.tmp
$	if $status
$	then
$	    rm _nofmtch.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$strftime:	echo "strftime"
$	test_class = "gawk_ext"
$	! this test could fail on slow machines or on a second boundary,
$	! so if it does, double check the actual results
$	! This test needs SYS$TIMEZONE_NAME and SYS$TIMEZONE_RULE
$	! to be properly defined.
$	! This test now needs GNV Corutils to work
$	date_bin = "gnv$gnu:[bin]gnv$date.exe"
$	if f$search(date_bin) .eqs. ""
$	then
$		skip_reason = "Need GNV Coreutils gnv$date.exe"
$		gosub junit_report_skip
$		return
$	endif
$	date := $'date_bin'
$!!	date | gawk -v "OUTPUT"=_strftime.tmp -f strftime.awk
$	now = f$time()
$	wkd = f$extract(0,3,f$cvtime(now,,"WEEKDAY"))
$	mon = f$cvtime(now,"ABSOLUTE","MONTH")
$	mon = f$extract(0,1,mon) + f$edit(f$extract(1,2,mon),"LOWERCASE")
$	day = f$cvtime(now,,"DAY")
$	tim = f$extract(0,8,f$cvtime(now,,"TIME"))
$!	Can not use tz as it shows up in the C environment.
$	timezone = f$trnlnm("SYS$TIMEZONE_NAME")
$	yr  = f$cvtime(now,,"YEAR")
$	if f$trnlnm("FTMP").nes."" then  close/noLog ftmp
$	open/Write ftmp strftime.in
$	write ftmp wkd," ",mon," ",day," ",tim," ",timezone," ",yr
$	close ftmp
$	gawk -v "OUTPUT"=_strftime.tmp -f strftime.awk strftime.in
$	set noOn
$	cmp strftime.ok sys$disk:[]_strftime.tmp
$	if $status
$	then
$	    rm _strftime.tmp;,strftime.ok;*,strftime.in;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$litoct:	echo "litoct"
$	test_class = "basic"
$	gawk --traditional -f litoct.awk >_litoct.tmp
ab
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp litoct.ok sys$disk:[]_litoct.tmp
$	if $status
$	then
$	    rm _litoct.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$resplit:	echo "resplit"
$	test_class = "gawk_ext"
$	gawk -- "{ FS = "":""; $0 = $0; print $2 }" >_resplit.tmp
a:b:c d:e:f
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp resplit.ok sys$disk:[]_resplit.tmp
$	if $status
$	then
$	    rm _resplit.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$intprec:	echo "intprec"
$	test_class = "basic"
$	gawk -f intprec.awk >_intprec.tmp 2>&1
$	cmp intprec.ok sys$disk:[]_intprec.tmp
$	if $status
$	then
$	    rm _intprec.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$incdupe:   echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   gawk --lint -i inclib -i inclib.awk "BEGIN {print sandwich(""a"", ""b"", ""c"")}" > _'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$       rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$incdupe2:   echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   gawk --lint -f inclib -f inclib.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$incdupe3:   echo "''test'"
$   test_class = "gawk_ext"
$   gawk --lint -f hello -f hello.awk >_'test'.tmp 2>&1
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   return
$
$incdupe4:   echo "''test'"
$   test_class = "gawk_ext"
$   set NoOn
$   gawk --lint -f hello -i hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$incdupe5:   echo "''test'"
$   test_class = "gawk_ext"
$   set NoOn
$   gawk --lint -i hello -f hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$incdupe6:   echo "''test'"
$   test_class = "gawk_ext"
$   set NoOn
$   gawk --lint -i inchello -f hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$incdupe7:   echo "''test'"
$   test_class = "gawk_ext"
$   set NoOn
$   gawk --lint -f hello -i inchello >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$include2:   echo "''test'"
$   test_class = "gawk_ext"
$   gawk -i inclib "BEGIN {print sandwich(""a"", ""b"", ""c"")}" >_'test'.tmp 2>&1
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   return
$
$id:
$symtab1:
$symtab2:
$symtab3:   echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   old_sort = sort
$   sort = "@sys$disk:[-.vms]vms_sort.com"
$   gawk -f 'test'.awk  >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   sort = old_sort
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$symtab4:
$symtab5:
$symtab7:   echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   gawk -f 'test'.awk <'test'.in >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$symtab6:   echo "''test'"
$   test_class = "gawk_ext"
$   tmp_error = "sys$disk:[]__''test'.tmp2"
$   set noOn
$   if f$search(tmp_error) then delete 'tmp_error';*
$   define/user sys$error 'tmp_error'
$   gawk -d__'test'.tmp -f 'test'.awk
$   gawk_status = $status
$   if f$search("sys$disk:[]__''test'.tmp") .eqs. ""
$   then
$       copy 'tmp_error' sys$disk:[]__'test.tmp'
$   else
$       append 'tmp_error' sys$disk:[]__'test.tmp'
$   endif
$   if .not. gawk_status then call exit_code 'gawk_status' __'test'.tmp
$   pipe search __'test'.tmp "ENVIRON","PROCINFO" /match=nor > _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*,__'test'.tmp;*,'tmp_error';*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$symtab8:   echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   gawk -d__'test'.tmp -f 'test'.awk 'test'.in > _'test'.tmp
$   if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$   then
$	delete sys$disk:[]_'test'.tmp;2
$   endif
$   pipe search __'test'.tmp "ENVIRON","PROCINFO","FILENAME" /match=nor > ___'test'.tmp
$   convert/append ___'test'.tmp _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*,__'test'.tmp;*,___'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$
$!-----------------------------------------------------------------------------------
$! This awk script performs some cleanup by doing "system (rm testit.txt)". This is
$! good for Unix but a pain for VMS as we must specify version number when deleting
$! a file. The workaround is to define "rm" as a VMS comment and deleting the file
$! outside of the awk script.
$! Additionally each awk "system" call results in a new version of the output file.
$! so we need to compensate for that as well.
$!-----------------------------------------------------------------------------------
$symtab9:   echo "''test'"
$   test_class = "gawk_ext"
$   old_rm = rm			! Remember old definition of rm
$   rm = "!"			! Redefine rm to something harmless
$   gawk -f 'test'.awk  >_'test'.tmp
$   rm = old_rm			! Restore old value
$   delsym old_rm
$   cmp 'test'.ok sys$disk:[]_'test'.tmp;-0	! -0 is the lowest version
$   if $status
$   then
$	rm _'test'.tmp;*,testit.txt;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   return
$!
$symtab10:	echo "''test'"
$   test_class = "gawk_ext"
$   set noOn
$   gawk "-D" -f 'test'.awk < 'test'.in > _'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status
$   then
$	rm _'test'.tmp;*
$	gosub junit_report_pass
$   else
$	gosub junit_report_fail_diff
$   endif
$   set On
$   return
$!
$ignrcas3:	echo "''test'"
$   test_class = "extra"
$   if f$search("sys$i18n_locale:el_gr_iso8859-7.locale") .nes. ""
$   then
$	define/user LC_ALL "el_gr_iso8859-7"
$	define/user GAWKLOCALE "el_gr_iso8859-7"
$	AWKPATH_srcdir
$!	goto common_without_test_in
$	skip_reason = "VMS EL_GR_ISO8859-7 locale fails test"
$	gosub junit_report_skip
$   else
$	skip_reason = "EL_GR_ISO8859-7 locale not installed"
$	gosub junit_report_skip
$   endif
$   return
$!
$nlstringtest:	echo "''test'"
$   test_class = "extra"
$   ! This locale does not seem to be available from the HPE I18N kit.
$   ! So this test has not been run on VMS.
$   if f$search("sys$i18n_locale:fr_fr_utf-8.locale") .nes. ""
$   then
$	define/user LC_ALL "fr_fr_utf-8"
$	define/user GAWKLOCALE "fr_fr_utf-8"
$	AWKPATH_srcdir
$!	goto common_without_test_in
$	skip_reason = "VMS FR_FR_UTF-8 locale test not validated."
$	gosub junit_report_skip
$   else
$	skip_reason = "FR_FR_UTF-8 locale not installed"
$	gosub junit_report_skip
$   endif
$   return
$!
$childin:	echo "''test'"
$	test_class = "basic"
$	cat = "type sys$input"
$	gawk -f 'test'.awk < 'test'.in > _'test'.tmp
$	delsym cat
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$noeffect:	echo "noeffect"
$	test_class = "basic"
$	AWKPATH_srcdir
$	gawk --lint -f noeffect.awk >_noeffect.tmp 2>&1
$	cmp noeffect.ok sys$disk:[]_noeffect.tmp
$	if $status
$	then
$	    rm _noeffect.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$numsubstr:	echo "numsubstr"
$	test_class = "basic"
$	AWKPATH_srcdir
$	gawk -f numsubstr.awk numsubstr.in >_numsubstr.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp numsubstr.ok sys$disk:[]_numsubstr.tmp
$	if $status
$	then
$	    rm _numsubstr.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$prmreuse:	echo "prmreuse"
$	test_class = "basic"
$	if f$search("prmreuse.ok").eqs."" then  create prmreuse.ok
$	gawk -f prmreuse.awk >_prmreuse.tmp
$	cmp prmreuse.ok sys$disk:[]_prmreuse.tmp
$	if $status
$	then
$	    rm _prmreuse.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fflush:
$	echo "fflush"
$	test_class = "unix_tests"
$	skip_reason = "not supported"
$	gosub junit_report_skip
$	return
$!!fflush:	echo "fflush"
$	! hopelessly Unix-specific
$!!	@fflush.sh >_fflush.tmp
$	cmp fflush.ok sys$disk:[]_fflush.tmp
$	if $status
$	then
$	    rm _fflush.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$getlnhd:
$	echo "getlnhd"
$	!Expects a Unix shell
$	test_class = "unix_tests"
$	skip_reason = "Expects a Unix shell"
$	gosub junit_report_skip
$	return
$!!getlnhd:	echo "getlnhd"
$	gawk -f getlnhd.awk >_getlnhd.tmp
$	cmp getlnhd.ok sys$disk:[]_getlnhd.tmp
$	if $status
$	then
$	    rm _getlnhd.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$tweakfld:	echo "tweakfld"
$	test_class = "basic"
$	gawk -f tweakfld.awk tweakfld.in >_tweakfld.tmp
$	if f$search("errors.cleanup").nes."" then  rm errors.cleanup;*
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp tweakfld.ok sys$disk:[]_tweakfld.tmp
$	if $status
$	then
$	    rm _tweakfld.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$clsflnam:	echo "clsflnam"
$	test_class = "basic"
$	if f$search("clsflnam.ok").eqs."" then  create clsflnam.ok
$	gawk -f clsflnam.awk clsflnam.in >_clsflnam.tmp 2>&1
$	cmp clsflnam.ok sys$disk:[]_clsflnam.tmp
$	if $status
$	then
$	    rm _clsflnam.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mmap8k:	echo "mmap8k"
$	test_class = "basic"
$	gawk "{ print }" mmap8k.in >_mmap8k.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp mmap8k.in sys$disk:[]_mmap8k.tmp
$	if $status
$	then
$	    rm _mmap8k.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$eofsplit:	echo "eofsplit"
$	test_class = "basic"
$	if f$search("eofsplit.ok").eqs."" then  create eofsplit.ok
$	gawk -f eofsplit.awk >_eofsplit.tmp
$	cmp eofsplit.ok sys$disk:[]_eofsplit.tmp
$	if $status
$	then
$	    rm _eofsplit.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$tradanch:	echo "tradanch"
$	test_class = "basic"
$	if f$search("tradanch.ok").eqs."" then  create tradanch.ok
$	gawk --traditional -f tradanch.awk tradanch.in >_tradanch.tmp
$	cmp tradanch.ok sys$disk:[]_tradanch.tmp
$	if $status
$	then
$	    rm _tradanch.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$reginttrad:	echo "''test'"
$	test_class = "basic"
$	gawk --traditional -r -f 'test'.awk >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$pid:		echo "pid"
$	test_class = "unix_tests"
$	pid = f$integer("%x" + f$getjpi("","PID"))
$	ppid = f$integer("%x" + f$getjpi("","OWNER"))
$	gawk -v "ok_pid=''pid'" -v "ok_ppid=''ppid'" -f pid.awk >_pid.tmp >& _NL:
$	cmp pid.ok sys$disk:[]_pid.tmp
$	if $status
$	then
$	    rm _pid.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$strftlng:	echo "strftlng"
$	test_class = "unix_tests"
$	define/User TZ "UTC"		!useless
$	gawk -f strftlng.awk >_strftlng.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp strftlng.ok sys$disk:[]_strftlng.tmp
$	if $status
$	then
$	    rm _strftlng.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nfldstr:	echo "nfldstr"
$	test_class = "basic"
$	if f$search("nfldstr.ok").eqs."" then  create nfldstr.ok
$	gawk "$1 == 0 { print ""bug"" }" >_nfldstr.tmp

$	cmp nfldstr.ok sys$disk:[]_nfldstr.tmp
$	if $status
$	then
$	    rm _nfldstr.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nors:		echo "nors"
$	test_class = "basic"
$!! there's no straightforward way to supply non-terminated input on the fly
$!!	@echo A B C D E | tr -d '\12' | $(AWK) '{ print $$NF }' - $(srcdir)/nors.in > _$@
$!! so just read a line from sys$input instead
$	gawk "{ print $NF }" - nors.in >_nors.tmp
A B C D E
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp nors.ok sys$disk:[]_nors.tmp
$	if $status
$	then
$	    rm _nors.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$reint:		echo "reint"
$	test_class = "gawk_ext"
$       set noOn
$	gawk --re-interval -f reint.awk reint.in >_reint.tmp
$       gawk_status = $status
$       set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	if .not. gawk_status
$       then
$	    call exit_code 'gawk_status' _'test'.tmp
$       endif
$	cmp reint.ok sys$disk:[]_reint.tmp
$	if gawk_status
$	then
$	    rm _reint.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$noparms:	echo "noparms"
$	test_class = "basic"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f noparms.awk >_noparms.tmp 2>&1
$	if .not. $status then call exit_code '$status' _noparms.tmp
$	set On
$	cmp noparms.ok sys$disk:[]_noparms.tmp
$	if $status
$	then
$	    rm _noparms.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$pipeio1:	echo "pipeio1"
$	test_class = "unix_tests"
$	cat = "TYPE"	!close enough, as long as we avoid .LIS default suffix
$	define/User test1 []test1.
$	define/User test2 []test2.
$	gawk -f pipeio1.awk >_pipeio1.tmp
$	rm test1.;,test2.;
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp pipeio1.ok sys$disk:[]_pipeio1.tmp
$	if $status
$	then
$	    rm _pipeio1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$pipeio2:
$	test_class = "unix_tests"
$	echo "pipeio2 skipped"
$	! Expects the sed utility and Posix shell
$	return
$!!pipeio2:	echo "pipeio2"
$	cat = "gawk -- {print}"
$	tr  = "??"	!unfortunately, no trivial substitution available...
$	gawk -v "SRCDIR=." -f pipeio2.awk >_pipeio2.tmp
$	cmp pipeio2.ok sys$disk:[]_pipeio2.tmp
$	if $status
$	then
$	    rm _pipeio2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$clobber:	echo "clobber"
$	test_class = "basic"
$	gawk -f clobber.awk >_clobber.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp clobber.ok sys$disk:[]seq.
$	if $status then  rm seq.;*
$	cmp clobber.ok sys$disk:[]_clobber.tmp
$	if $status
$	then
$	    rm _clobber.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nasty:	echo "nasty"
$	test_class = "basic"
$	set noOn
$	gawk -f nasty.awk >_nasty.tmp
$	call fixup_LRL nasty.ok
$	call fixup_LRL _nasty.tmp "purge"
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp nasty.ok sys$disk:[]_nasty.tmp
$	if $status
$	then
$	    rm _nasty.tmp;
$	    file = "lcl_root:[]nasty.ok"
$	    if f$search(file) .nes. "" then rm 'file';*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$nasty2:	echo "nasty2"
$	test_class = "basic"
$	set noOn
$	gawk -f nasty2.awk >_nasty2.tmp
$	call fixup_LRL nasty2.ok
$	call fixup_LRL _nasty2.tmp "purge"
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp nasty2.ok sys$disk:[]_nasty2.tmp
$	if $status
$	then
$	    rm _nasty2.tmp;
$	    file = "lcl_root:[]nasty2.ok"
$	    if f$search(file) .nes. "" then rm 'file';*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$arrayparm:
$arrayprm2:
$fnaryscl:
$opasnslf:
$opasnidx:
$printfbad1:
$prmarscl:
$subslash:
$	test_class = "basic"
$	goto test_error_capture1
$!
$functab1:
$functab2:
$functab3:
$aadelete1:
$aadelete2:
$nastyparm:
$	test_class = "gawk_ext"
$	goto test_error_capture1
$!
$test_error_capture1:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$arynocls:	echo "arynocls"
$	test_class = "basic"
$	gawk -v "INPUT"=arynocls.in -f arynocls.awk >_arynocls.tmp
$	cmp arynocls.ok sys$disk:[]_arynocls.tmp
$	if $status
$	then
$	    rm _arynocls.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$getlnbuf:	echo "getlnbuf"
$	test_class = "basic"
$	gawk -f getlnbuf.awk getlnbuf.in >_getlnbuf.tmp
$	gawk -f gtlnbufv.awk getlnbuf.in >_getlnbuf.too
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp getlnbuf.ok sys$disk:[]_getlnbuf.tmp
$	if $status then  rm _getlnbuf.tmp;
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$	cmp getlnbuf.ok sys$disk:[]_getlnbuf.too
$	if $status
$	then
$	    rm _getlnbuf.too;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$lint:	echo "lint"
$	test_class = "gawk_ext"
$	AWKPATH_srcdir
$	gawk -f lint.awk >_lint.tmp 2>&1
$	cmp lint.ok sys$disk:[]_lint.tmp
$	if $status
$	then
$	    rm _lint.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$lintold:	echo "lintold"
$	test_class = "gawk_ext"
$	AWKPATH_srcdir
$       set noOn
$	gawk -f lintold.awk --lint-old <lintold.in >_lintold.tmp 2>&1
$       set on
$	cmp lintold.ok sys$disk:[]_lintold.tmp
$	if $status
$	then
$	    rm _lintold.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$ofmtbig:	echo "ofmtbig"
$	test_class = "basic"
$	set noOn
$	gawk -f ofmtbig.awk ofmtbig.in >_ofmtbig.tmp 2>&1
$	set On
$	cmp ofmtbig.ok sys$disk:[]_ofmtbig.tmp
$	if $status
$	then
$	    rm _ofmtbig.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$inetechu:	echo "inetechu"
$	test_class = "inet"
$	echo "this test is for establishing UDP connections"
$	set noOn
$	gawk -- "BEGIN {print """" |& ""/inet/udp/0/127.0.0.1/9""}"
$	! report skip as we do not validate yet.
$	skip_reason = "Test not validated."
$	gosub junit_report_skip
$	set On
$	return
$
$inetecht:	echo "inetecht"
$	test_class = "inet"
$	echo "this test is for establishing TCP connections"
$	set noOn
$	gawk -- "BEGIN {print """" |& ""/inet/tcp/0/127.0.0.1/9""}"
$	! report skip as we do not validate yet.
$	skip_reason = "Test not validated."
$	gosub junit_report_skip
$	set On
$	return
$
$inetdayu:	echo "inetdayu"
$	test_class = "inet"
$	echo "this test is for bidirectional UDP transmission"
$	set noOn
$	gawk -f - _NL:
BEGIN { print "" |& "/inet/udp/0/127.0.0.1/13";
	"/inet/udp/0/127.0.0.1/13" |& getline; print $0}
$	! report skip as we do not validate yet.
$	skip_reason = "Test not validated."
$	gosub junit_report_skip
$	set On
$	return
$
$inetdayt:	echo "inetdayt"
$	test_class = "inet"
$	echo "this test is for bidirectional TCP transmission"
$	set noOn
$	gawk -f - _NL:
BEGIN { print "" |& "/inet/tcp/0/127.0.0.1/13";
	"/inet/tcp/0/127.0.0.1/13" |& getline; print $0}
$	! report skip as we do not validate yet.
$	skip_reason = "Test not validated."
$	gosub junit_report_skip
$	set On
$	return
$
$redfilnm:	echo "redfilnm"
$	test_class = "basic"
$	gawk -f redfilnm.awk srcdir="." redfilnm.in >_redfilnm.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp redfilnm.ok sys$disk:[]_redfilnm.tmp
$	if $status
$	then
$	    rm _redfilnm.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$leaddig:	echo "leaddig"
$	test_class = "basic"
$	gawk -v "x=2E"  -f leaddig.awk >_leaddig.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp leaddig.ok sys$disk:[]_leaddig.tmp
$	if $status
$	then
$	    rm _leaddig.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$clos1way:
$clos1way2:
$clos1way3:
$clos1way4:
$clos1way5:
$	test_class = "gawk_ext"
$	echo "''test'"
$	skip_reason = "Test not supported."
$	gosub junit_report_skip
$	return
$#clos1way:	echo "''test'"
$	gawk -f clos1way.awk >_clos1way.tmp
$	cmp clos1way.ok sys$disk:[]_clos1way.tmp
$	if $status
$	then
$	    rm _clos1way.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$shadow:	echo "shadow"
$	test_class = "gawk_ext"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f shadow.awk >_shadow.tmp 2>&1
$	set On
$	cmp shadow.ok sys$disk:[]_shadow.tmp
$	if $status
$	then
$	    rm _shadow.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$lintwarn:	echo "lintwarn"
$	test_class = "basic"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f lintwarn.awk >_lintwarn.tmp 2>&1
$	if .not. $status then call exit_code '$status' _lintwarn.tmp
$	set On
$	cmp lintwarn.ok sys$disk:[]_lintwarn.tmp
$	if $status
$	then
$	    rm _lintwarn.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$longsub:	echo "longsub"
$	test_class = "basic"
$	set noOn
$	gawk -f longsub.awk longsub.in >_longsub.tmp
$!! the records here are too long for DIFF to handle
$!! so assume success as long as gawk doesn't crash
$!!	call fixup_LRL longsub.ok
$!!	call fixup_LRL _longsub.tmp "purge"
$!!	cmp longsub.ok sys$disk:[]_longsub.tmp
$	if $status
$	then
$	    rm _longsub.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$arrayprm3:	echo "arrayprm3"
$	test_class = "basic"
$	gawk -f arrayprm3.awk arrayprm3.in >_arrayprm3.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp arrayprm3.ok sys$disk:[]_arrayprm3.tmp
$	if $status
$	then
$	    rm _arrayprm3.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$arryref3:
$arryref4:
$arryref5:
$aryprm1:
$aryprm2:
$aryprm3:
$aryprm4:
$aryprm5:
$aryprm6:
$aryprm7:
$aryprm8:
$aryprm9:
$dfastress:
$nfneg:
$numindex:
$scalar:
$sclforin:
$sclifin:
$	echo "''test'"
$	test_class = "basic"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$sigpipe1:
$	test_class = "basic"
$	echo "''test'"
$	skip_reason = "Test not supported."
$	gosub junit_report_skip
$	return
$
$   !
$   ! For tests requiring exit code 2
$   !
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk <'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$exitval2:
$	echo "''test'"
$	test_class = "basic"
$	cmdfile = "sys$disk:[]_''test'.com"
$	if f$search(cmdfile) .nes. "" then delete 'cmdfile';*
$	open/write xxx 'cmdfile'
$	write xxx "$!'f$verify(0,0)'
$	write xxx "$read := read"
$	write xxx "$define/user sys$input sys$command"
$	write xxx "$pipe read sys$input x ; write sys$output x ; exit %x003A151"
$	close xxx
$	! Unix exit 42 is VMS %x003A151
$	read := "@''cmdfile' !"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp
$	if $status .ne. 1 then call exit_code '$status' _'test'.tmp
$	delete/symbol/local read
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*
$	    if f$search(cmdfile) .nes. "" then rm 'cmdfile';*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$exitval3:
$	echo "''test'"
$	test_class = "basic"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp
$	! Unix exit 42 is VMS %x003A151
$	if $status .ne. 1 then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$lc_num1:
$fmttest:
$backsmalls2:
$	test_class = "charset_tests"
$       test_in = "''test'.in"
$       goto common_test_optional_input
$!
$fpat4:
$regnul1:
$regnul2:
$crlf:
$rsgetline:
$rstest6:
$	test_class = "gawk_ext"
$       test_in = "''test'.in"
$	goto common_with_test_input
$!
$delfunc:
$fcall_exit2:
$fnamedat:
$fnarray2:
$fnasgnm:
$fnmisc:
$gsubasgn:
$gsubtest:
$gsubtst2:
$gsubtst4:
$hex:
$inputred:
$nfloop:
$prec:
$prtoeval:
$rstest1:
$rstest2:
$rstest3:
$sortempty:
$splitarr:
$strcat1:
$subsepnm:
$unterm:
$zeroe0:
$	test_class = "basic"
$common_test_optional_input:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$getline:	echo "getline"
$	test_class = "basic"
$	!Expects a Unix shell
$	skip_reason = "Requires a Unix shell."
$	gosub junit_report_skip
$	return
$!!getline:	echo "getline"
$	gawk -f getline.awk getline.in >_getline.tmp
$	cmp getline.ok sys$disk:[]_getline.tmp
$	if $status
$	then
$	    rm _getline.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$gsubtst3:	echo "gsubtst3"
$	test_class = "basic"
$	gawk --re-interval -f gsubtst3.awk gsubtst3.in >_gsubtst3.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp gsubtst3.ok sys$disk:[]_gsubtst3.tmp
$	if $status
$	then
$	    rm _gsubtst3.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$! FIXME: gawk generates an extra, empty output file while running this test...
$iobug1:	echo "iobug1"
$	test_class = "basic"
$	cat = "TYPE sys$input:"
$	true = "exit 1"	!success
$		oldout = f$search("_iobug1.tmp;")
$	gawk -f iobug1.awk iobug1.in >_iobug1.tmp
$		badout = f$search("_iobug1.tmp;-1")
$		if badout.nes."" .and. badout.nes.oldout then  rm 'badout'
$	cmp iobug1.ok sys$disk:[]_iobug1.tmp
$	if $status
$	then
$	    rm _iobug1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rstest4:
$	echo "rstest4"
$	test_class = "basic"
$	old_echo = echo
$	echo = "write sys$output """
$	gawk -f rstest4.awk >_rstest4.tmp1
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	echo = old_echo
$	gawk "{gsub(""< A>"",""<a>"")}1" _'test'.tmp1 >_'test'.tmp
$	rm sys$disk:[]_'test'.tmp1;*
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp rstest4.ok sys$disk:[]_rstest4.tmp
$!	if $status then  rm _rstest4.tmp;
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rstest5:
$	echo "rstest5"
$	test_class = "basic"
$	old_echo = echo
$	echo = "write sys$output """
$	gawk -f rstest5.awk >_rstest5.tmp1
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	echo = old_echo
$	gawk "{gsub("" '"","""");gsub(""'"","""")}1" _'test'.tmp1 >_'test'.tmp
$	rm sys$disk:[]_'test'.tmp1;*
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp rstest5.ok sys$disk:[]_rstest5.tmp
$	if $status
$	then
$	    rm _rstest5.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fcall_exit:
$fnarray:
$funsmnam:
$paramdup:
$paramres:
$parseme:
$synerr1:
$synerr2:
$	test_class = "basic"
$	goto test_error_capture1
$match2:
$	test_class = "gawk_ext"
$	goto test_error_capture1
$!
$uninit2:
$uninit3:
$uninit4:
$uninit5:
$uninitialized:
$	echo "''test'"
$	test_class = "basic"
$	gawk --lint -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$space:		echo "space"
$	test_class = "unix_tests"
$	set noOn
$	gawk -f " " space.awk >_space.tmp 2>&1
$	if .not. $status then call exit_code '$status' _space.tmp
$	set On
$!	we get a different error from what space.ok expects
$	gawk "{gsub(""file specification syntax error"", ""no such file or directory""); print}" -
		_space.tmp >_space.too
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$	rm _space.tmp;
$	mv _space.too _space.tmp
$	igncascmp space.ok sys$disk:[]_space.tmp
$	if $status
$	then
$	    rm _space.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$posix2008sub:
$printf0:
$	echo "''test'"
$	test_class = "basic"
$	gawk --posix -f 'test'.awk >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rsnulbig:	echo "rsnulbig"
$	test_class = "basic"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "With PIPE, this will probably take quite a while..."
$		On warning then  $
$		pipeok = 1
$	else	echo "This will probably take quite a while too..."
$	endif
$	set noOn
$	pipe -
	gawk -- "BEGIN {for (i = 1; i <= 128*64+1; i++) print ""abcdefgh123456\n""}" | -
	gawk -- "BEGIN {RS = """"; ORS = ""\n\n""}; {print}" | -
	gawk -- "/^[^a]/; END {print NR}" >_rsnulbig.tmp
$	set On
$	cmp rsnulbig.ok sys$disk:[]_rsnulbig.tmp
$	if $status
$	then
$	    rm _rsnulbig.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rsnulbig2:	echo "rsnulbig2"
$	test_class = "basic"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "With PIPE, this will probably take quite a while..."
$		On warning then  $
$		pipeok = 1
$	else	echo "This will probably take quite a while too..."
$	endif
$	set noOn
$	pipe -
	gawk -- "BEGIN {ORS=""""; n=""\n""; for (i=1; i<=10; i++) n=(n n); for (i=1; i<=128; i++) print n; print ""abc\n""}" | -
	gawk -- "BEGIN {RS=""""; ORS=""\n\n"" }; {print}" | -
	gawk -- "/^[^a]/; END {print NR}" >_rsnulbig2.tmp
$	set On
$	cmp rsnulbig2.ok sys$disk:[]_rsnulbig2.tmp
$	if $status
$	then
$	    rm _rsnulbig2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$backbigs1:	! 4.1.1
$	test_class = "charset_tests"
$	echo "''test'"
$	! needs a locale not on VMS currently
$	skip_reason = "Needs locale not on VMS Currently"
$	gosub junit_report_skip
$	return
$!
$backsmalls1:
$	test_class = "charset_tests"
$	gosub define_gawklocale
$	if f$trnlnm("LC_ALL") .nes. "utf8-50"
$	then
$	    echo "''test'"
$	    skip_reason = "Needs utf8-50 locale."
$	    gosub junit_report_skip
$	    return
$	else
$	    echo "''test'"
$	endif
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$subamp:
$wideidx:
$widesub2:
$widesub3:
$	echo "''test'"
$	test_class = "basic"
$	gosub define_gawklocale
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$ignrcas2:
$	test_class = "gawk_ext"
$	goto wide_tests_1
$wideidx2:
$widesub:
$widesub4:
$	test_class = "basic"
$	goto wide_tests_1
$!
$wide_tests_1:
$	echo "''test'"
$	gosub define_gawklocale
$	gawk -f 'test'.awk >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$! This test is somewhat suspect for vms due to exit code manipulation
$exitval1:	echo "exitval1"
$	test_class = "basic"
$	gawk -f exitval1.awk >_exitval1.tmp 2>&1
$	if $status then  call exit_code '$status' _exitval1.tmp
$	cmp exitval1.ok sys$disk:[]_exitval1.tmp
$	if $status
$	then
$	    rm _exitval1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fsspcoln:	echo "fsspcoln"
$	test_class = "basic"
$	gawk -f fsspcoln.awk "FS=[ :]+" fsspcoln.in >_forspcoln.tmp
$	if f$search("sys$disk:[]_forspcoln.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_forspcoln.tmp;2
$	endif
$	cmp fsspcoln.ok sys$disk:[]_forspcoln.tmp
$	if $status
$	then
$	    rm _forspcoln.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$getlndir:	echo "getlndir"
$	test_class = "gawk_ext"
$	! assume we're running in the test subdirectory; we don't want to
$	! perform a messy conversion of [] into its file specification
$	gawk -v "SRCDIR=[-]test.dir" -f getlndir.awk >_getlndir.tmp
$!	getlndir.ok expects "Is a directory", we see "is a directory"
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	igncascmp getlndir.ok sys$disk:[]_getlndir.tmp
$	if $status
$	then
$	    rm _getlndir.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rsstart2:	echo "rsstart2"
$	test_class = "gawk_ext"
$	gawk -f rsstart2.awk rsstart1.in >_rsstart2.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp rsstart2.ok sys$disk:[]_rsstart2.tmp
$	if $status
$	then
$	    rm _rsstart2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rsstart3:
$	echo "rsstart3"
$	test_class = "gawk_ext"
$!      rsstart3 with pipe fails,
$!	presumeably due to PIPE's use of print file format
$!	if .not.pipeok
$!	then	echo "Without the PIPE command, ''test' can't be run."
$!		On warning then  return
$!		pipe echo "With PIPE, ''test' will finish quickly."
$!		On warning then  $
$!		pipeok = 1
$!	endif
$	! head rsstart1.in | gawk -f rsstart2.awk >_rsstart3.tmp
$	! splitting this into two steps would make it the same as rsstart2
$	set noOn
$!$	pipe -
$!	  gawk -- "FNR <= 10" rsstart1.in | -
$!	  gawk -f rsstart2.awk >_rsstart3.tmp
$!
$	! DCL redirection to files is similar to pipes
$	! But not quite the same.
$	! DCL pipes are mailboxes, not the same as CRTL pipes,
$	! So eventually someone should look into why CRTL pipes and
$	! gawk are not always playing well together.
$	gawk -- "FNR <= 10" rsstart1.in > _'test'.tmp1
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	define/user sys$input _'test'.tmp1
$	gawk -f rsstart2.awk >_rsstart3.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	set On
$	cmp rsstart3.ok sys$disk:[]_rsstart3.tmp
$	if $status
$	then
$	    rm _rsstart3.tmp;, _rsstart3.tmp1;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rtlen:
$rtlen01:
$	test_class = "unix_tests"
$	goto rtlen_tests_1
$rtlenmb:
$	test_class = "charset_tests"
$	goto rtlen_tests_1
$!
$rtlen_tests_1:
$	echo "''test'"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "With PIPE, ''test' will finish quickly."
$		On warning then  $
$		pipeok = 1
$	endif
$   f = "''test'.ok"
$   if test.eqs."rtlen" .or. test.eqs."rtlenmb"
$   then
$	if test.eqs."rtlenmb" then  GAWKLOCALE = "en_US.UTF-8"
$	pipe -
	gawk -- "BEGIN {printf ""0\n\n\n1\n\n\n\n\n2\n\n""; exit}" | -
	gawk -- "BEGIN {RS=""""}; {print length(RT)}" >_'test'.tmp
$	if test.eqs."rtlenmb" then  delet_/Symbol/Local GAWKLOCALE
$	if test.eqs."rtlenmb" then  f = "rtlen.ok"
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$   else
$	call/Output=_rtlen01.tmp do__rtlen01
$	! first test yields 1 instead of 0 due to forced newline
$	gawk -- "FNR==1 {sub(""1"",""0"")}; {print}" _rtlen01.tmp >_rtlen01.too
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$	rm _rtlen01.tmp;
$	mv _rtlen01.too _rtlen01.tmp
$   endif
$	cmp 'f' sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$do__rtlen01: subroutine
$	gawk = gawk !PIPE won't propagate local symbols from outer procedure
$	pipe -
	gawk -- "BEGIN {printf ""0""; exit}" | -
	gawk -- "BEGIN {RS=""""}; {print length(RT)}"
$	pipe -
	gawk -- "BEGIN {printf ""0\n""; exit}" | -
	gawk -- "BEGIN {RS=""""}; {print length(RT)}"
$	pipe -
	gawk -- "BEGIN {printf ""0\n\n""; exit}" | -
	gawk -- "BEGIN {RS=""""}; {print length(RT)}"
$ endsubroutine !do__rtlen01
$
$nondec2:	echo "nondec2"
$	test_class = "gawk_ext"
$	gawk --non-decimal-data -v "a=0x1" -f nondec2.awk >_nondec2.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp nondec2.ok sys$disk:[]_nondec2.tmp
$	if $status
$	then
$	    rm _nondec2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$nofile:	echo "nofile"
$	test_class = "basic"
$!	gawk "{}" no/such/file >_nofile.tmp 2>&1
$!	nofile.ok expects no/such/file, but using that name in the test would
$!	yield "file specification syntax error" instead of "no such file..."
$	set noOn
$	gawk "{}" no-such-file >_nofile.tmp 2>&1
$	if .not. $status then call exit_code '$status' _nofile.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	set On
$!	restore altered file name
$	gawk "{gsub(""no-such-file"", ""no/such/file""); print}" _nofile.tmp >_nofile.too
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$	rm _nofile.tmp;
$	mv _nofile.too _nofile.tmp
$!	nofile.ok expects "No such file ...", we see "no such file ..."
$	igncascmp nofile.ok sys$disk:[]_nofile.tmp
$	if $status
$	then
$	    rm _nofile.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$binmode1:	echo "binmode1"
$	test_class = "gawk_ext"
$	gawk -v "BINMODE=3" "BEGIN { print BINMODE }" >_binmode1.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp binmode1.ok sys$disk:[]_binmode1.tmp
$	if $status
$	then
$	    rm _binmode1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$subi18n:	echo "''test'"
$	test_class = "basic"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f 'test'.awk > _'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$rri1:
$concat4:	echo "''test'"
$	test_class = "basic"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f 'test'.awk 'test'.in > _'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$devfd:		echo "devfd"
$	test_class = "gawk_ext"
$	skip_reason = "Not supported."
$	gosub junit_report_skip
$	return
$!!devfd:	echo "devfd"
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in4 5< devfd.in5 >_devfd.tmp
$	cmp devfd.ok sys$disk:[]_devfd.tmp
$	if $status
$	then
$	    rm _devfd.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$devfd1:	echo "devfd1"
$	test_class = "gawk_ext"
$	skip_reason = "Not supported."
$	gosub junit_report_skip
$	return
$!!devfd1:	echo "devfd1"
$	gawk -f devfd1.awk 4< devfd.in1 5< devfd.in2 >_devfd1.tmp
$	cmp devfd1.ok sys$disk:[]_devfd1.tmp
$	if $status
$	then
$	    rm _devfd1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$devfd2:	echo "devfd2"
$	test_class = "gawk_ext"
$	skip_reason = "Not supported."
$	gosub junit_report_skip
$	return
$!!devfd2:	echo "devfd2"
$! The program text is the '1' which will print each record. How compact can you get?
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in1 5< devfd.in2 >_devfd2.tmp
$	cmp devfd2.ok sys$disk:[]_devfd2.tmp
$	if $status
$	then
$	    rm _devfd2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$charasbytes:
$! This test used "od" on Unix to verify the result. As this is not available
$! we must try as best as possible using DUMP and SEARCH, instead of comparing
$! to charasbytes.ok
$!
$	echo "''test'"
$	test_class = "gawk_ext"
$	gawk -b -f 'test'.awk 'test'.in >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	pipe dump/byte/block=count:1 _charasbytes.tmp | -
		search sys$pipe /noout " 00 00 00 00 00 00 00 00 00 00 00 00 0A 5A 5A 5A"
$	if $severity .eq. 1
$	then
$	    rm _'test'.tmp;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mixed1:	echo "mixed1"
$	test_class = "basic"
$	set noOn
$	gawk -f /dev/null --source "BEGIN {return junk}" >_mixed1.tmp 2>&1
$	if .not. $status then call exit_code '$status' _mixed1.tmp
$	set On
$	cmp mixed1.ok sys$disk:[]_mixed1.tmp
$	if $status
$	then
$	    rm _mixed1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mtchi18n:	echo "mtchi18n"
$	test_class = "basic"
$	define/User GAWKLOCALE "ru_RU.UTF-8"
$	gawk -f mtchi18n.awk mtchi18n.in >_mtchi18n.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp mtchi18n.ok sys$disk:[]_mtchi18n.tmp
$	if $status
$	then
$	    rm _mtchi18n.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$reint2:	echo "reint2"
$	test_class = "gawk_ext"
$	gosub define_gawklocale
$       set noOn
$	gawk --re-interval -f reint2.awk reint2.in >_reint2.tmp
$	gawk_status = $status
$	set On
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	if .not. gawk_status
$       then
$	    call exit_code 'gawk_status' _'test'.tmp
$       endif
$	cmp reint2.ok sys$disk:[]_reint2.tmp
$	if gawk_status
$	then
$	    rm _reint2.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$localenl:	echo "localenl"
$	test_class = "unix_tests"
$	skip_reason = "Not supported."
$	gosub junit_report_skip
$	return
$!!localenl:	echo "localenl"
$	! localenl.com does not exist.
$	@localenl.com /Output=_localenl.tmp	! sh ./localenl.sh >tmp.
$	cmp localenl.ok sys$disk:[]_localenl.tmp
$	if $status
$	then
$	    rm _localenl.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mbprintf1:	echo "''test'"
$	test_class = "charset_tests"
$! Makefile test exports this, but we don't want to impact user's environment
$!	@GAWKLOCALE=en_US.UTF-8 ; export GAWKLOCALE ; \
$!	Only always have "utf8-20"
$	gosub define_gawklocale
$	gawk -f mbprintf1.awk mbprintf1.in >_mbprintf1.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp mbprintf1.ok sys$disk:[]_mbprintf1.tmp
$	if $status
$	then
$	    rm _mbprintf1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mbprintf2:
$	echo "''test'"
$	test_class = "charset_tests"
$! Makefile test exports this, ...
$!	@GAWKLOCALE=ja_JP.UTF-8 ; export GAWKLOCALE ; \
$!	Only always have "utf8-20"
$!      ja_jp_utf-8 is optional
$	gosub define_gawklocale_ja_jp
$	gawk -f 'test'.awk >_'test'.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$!
$mbprintf3:
$mbprintf4:
$	echo "''test'"
$	test_class = "charset_tests"
$! Makefile test exports this
$!	@GAWKLOCALE=en_US.UTF-8 ; export GAWKLOCALE ; \
$!	Only always have "utf8-20"
$	gosub define_gawklocale
$       set noOn
$	gawk -f 'test'.awk 'test'.in 2>&1 >_'test'.tmp
$       set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$mbprintf5:
$	echo "''test'"
$	test_class = "charset_tests"
$! Makefile test exports this
$!	@GAWKLOCALE=en_US.UTF-8 ; export GAWKLOCALE ; \
$!	Only always have "utf8-20"
$	gosub define_gawklocale
$       set noOn
$	gawk -f 'test'.awk 'test'.in 2>&1 >_'test'.tmp
$       set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    skip_reason = "Test not currently passing on VMS. UTF-8 issue."
$	    gosub junit_report_skip
$!	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$mbfw1:	echo "mbfw1"
$	test_class = "charset_tests"
$! Makefile test exports this, ...
$	!define/User GAWKLOCALE "en_US.UTF-8"
$	gosub define_gawklocale
$	gawk -f mbfw1.awk mbfw1.in >_mbfw1.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp mbfw1.ok sys$disk:[]_mbfw1.tmp
$	if $status
$	then
$	    rm _mbfw1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$gsubtst6:	echo "gsubtst6"
$	test_class = "basic"
$	define/User GAWKLOCALE "C"
$	gawk -f gsubtst6.awk >_gsubtst6.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp gsubtst6.ok sys$disk:[]_gsubtst6.tmp
$	if $status
$	then
$	    rm _gsubtst6.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$mbstr1:
$	echo "''test'"
$	test_class = "gawk_ext"
$	gosub define_gawklocale
$	AWKPATH_srcdir
$	gawk -f 'test'.awk 2>&1 >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$mbstr2:
$	echo "''test'"
$	test_class = "gawk_ext"
$	gosub define_gawklocale
$	AWKPATH_srcdir
$	gawk -f 'test'.awk < 'test'.in 2>&1 >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$printhuge:
$	echo "''test'"
$	test_class = "gawk_ext"
$	gosub define_gawklocale
$	AWKPATH_srcdir
$	gawk -f 'test'.awk 2>&1 >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$printfbad2:
$printfbad3:
$printfbad4:
$	echo "''test'"
$	test_class = "gawk_ext"
$	set noOn
$	gawk --lint -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$fmtspcl:	echo "fmtspcl"
$	test_class = "hardware"
$	skip_reason = "Not supported."
$	gosub junit_report_skip
$	return
$!!fmtspcl:
$! fmtspcl ought to work if gawk was compiled to use IEEE floating point
$	if floatmode.lt.0 then  gosub calc_floatmode
$	if floatmode.lt.2
$	then
$	    echo "fmtspcl: not supported"
$	    ! report skip ...
$	    return
$	endif
$	echo "fmtspcl"
$	gawk -f fmtspcl.awk >_fmtspcl.tmp 2>&1
$	cmp fmtspcl.ok sys$disk:[]_fmtspcl.tmp
$	if $status
$	then
$	    rm _fmtspcl.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$intformat:	echo "intformat"
$	test_class = "hardware"
$! note: we use the Alpha value for D_float; VAX value is slightly higher
$!	due to not losing precision by being processed via G_float
$	huge_0 = "1.70141183460469213e+38"		! D_float
$	huge_1 = "8.98846567431157854e+307"		! G_float
$	huge_2 = "1.79769313486231570e+308"		! IEEE T_float
$	if floatmode.lt.0 then  gosub calc_floatmode
$	hugeval = huge_'floatmode'
$	set noOn
$	gawk -v "HUGEVAL=''hugeval'" -f intformat.awk >_intformat.tmp 2>&1
$	set On
$	cmp intformat.ok sys$disk:[]_intformat.tmp
$	if $status
$	then
$	    rm _intformat.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$! ugh... BEGINFILE functionality works fine, but test is heavily Unix-centric
$beginfile1:	echo "beginfile1"
$	test_class = "gawk_ext"
$	! complications:  "." is a filename, not the current directory
$	!  (even "[]" is actually "[].", that same filename, but we can
$	!  handle hacking it more easily);
$	!  "no/such/file" yields syntax error rather than no such file
$	!  when subdirectories no/ and no/such/ don't exist;
$	!  vms test suite doesn't generate Makefile;
$	!  "is a directory" and "no such file" aren't capitalized
$	! gawk -f beginfile1.awk beginfile1.awk . -
$	!   ./no/such/file "Makefile" >_beginfile1.tmp 2>&1
$	! And NFS on ODS2 encodes the case in the filename with
$	! the $ character switching case.
$	makefile_in = "Makefile.in"
$	makefile_in_nfs_ods2 = "$M$akefile.in"
$	if f$search(makefile_in_nfs_ods2) .nes. ""
$	then
$	    makefile_in = makefile_in_nfs_ods2
$	endif
$	gawk -f beginfile1.awk beginfile1.awk [] -
	 ./no-such-file "''makefile_in'" >_beginfile1.tmp 2>&1
$	    gawk -f - _beginfile1.tmp >_beginfile1.too
{ if (gsub("\\[\\]",".")) gsub("no such file or directory","is a directory")
  gsub("no-such-file","file"); gsub("Makefile.in","Makefile");
  gsub("\\$M\\$akefile.in","Makefile"); print }
$	if f$search("sys$disk:[]_''test'.too;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.too;2
$	endif
$	rm _beginfile1.tmp;
$	mv _beginfile1.too _beginfile1.tmp
$	igncascmp beginfile1.ok sys$disk:[]_beginfile1.tmp
$	if $status
$	then
$	    rm _beginfile1.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$dumpvars: echo "dumpvars"
$	test_class = "gawk_ext"
$	gawk --dump-variables 1 <dumpvars.in >_NL: 2>&1
$	mv awkvars.out _dumpvars.tmp
$	gawk "!/ENVIRON|PROCINFO/" _dumpvars.tmp >_dumpvars.tmp1
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	rm sys$disk:[]_dumpvars.tmp;*
$	mv sys$disk:[]_dumpvars.tmp1 sys$disk:[]_dumpvars.tmp
$	cmp dumpvars.ok sys$disk:[]_dumpvars.tmp
$	if $status
$	then
$	    rm _dumpvars.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$!
$profile0:
$	echo "''test'"
$	test_class = "gawk_ext"
$	gawk --profile=_ap-'test'.out -f 'test'.awk 'test'.in > _NL:
$	! sed <awkprof.out 1,2d >_profile3.tmp
$	gawk "NR>2" _ap-'test'.out > sys$disk:[]_awkprof.out
$	if f$search("sys$disk:[]_awkprof.out;2") .nes. ""
$	then
$	    delete sys$disk:[]_awkprof.out;2
$	endif
$	convert/fdl=nla0: _awkprof.out sys$disk:[]_'test'.tmp
$	convert/fdl=nla0: 'test'.ok _'test'.ok
$	cmp _'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*,_'test'.ok;*,_ap-'test'.out;,_awkprof.out;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$profile1: echo "''test'"
$	test_class = "gawk_ext"
$ ! FIXME: for both gawk invocations which pipe output to SORT,
$ ! two output files get created; the top version has real output
$ ! but there's also an empty lower version.
$!	oldout = f$search("_''test'.tmp1")
$	gawk -f xref.awk dtdgport.awk >  _'test'.out1
$	gawk --pretty-print=ap-'test'.out -f xref.awk
$	gawk -f ap-'test'.out dtdgport.awk > _'test'.out2
$	cmp _'test'.out1 sys$disk:[]_'test'.out2
$	if $status
$	then
$	    rm ap-'test'.out;*, _'test'.out%;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$profile2:  echo "''test'"
$	test_class = "gawk_ext"
$	gawk --profile -v "sortcmd=SORT sys$input: sys$output:" -
			-f xref.awk dtdgport.awk > _NL:
$	! sed <awkprof.out 1,2d >_profile2.tmp
$	sumslp awkprof.out /update=sys$input: /output=_'test'.tmp
-1,2
/
$	rm awkprof.out;
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$profile4:
$profile5:
$profile8:
$	echo "''test'"
$	test_class = "gawk_ext"
$       if f$search("sys$disk:[]_''test'.tmp1") .nes. ""
$       then
$           delete _'test'.tmp1;*
$       endif
$	if f$search("sys$disk:[]_''test'.tmp2;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp2;*
$	endif
$       tmp_error = "_''test'.tmp2"
$	define/user GAWK_NO_PP_RUN 1
$       define/user sys$error 'tmp_error'
$       set noOn
$	gawk --pretty-print=_'test'.tmp1 -f 'test'.awk > _NL:
$       gawk_status = $status
$       set On
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	if f$search("sys$disk:[]_''test'.tmp2;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp2;2
$	endif
$       append _'test'.tmp2 _'test'.tmp1
$	convert/fdl=nla0: _'test'.tmp1 sys$disk:[]_'test'.tmp
$	convert/fdl=nla0: 'test'.ok _'test'.ok
$	if .not. gawk_status
$       then
$	    call exit_code 'gawk_status' _'test'.tmp
$       endif
$	cmp sys$disk:[]_'test'.ok sys$disk:[]_'test'.tmp
$       cmp_status = $status
$       if gawk_status then gawk_status = cmp_status
$	if gawk_status
$	then
$	    rm _'test'.tmp;*,_'test'.ok;*,_'test'.tmp1;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$profile6:
$profile7:
$profile3: echo "''test'"
$	test_class = "gawk_ext"
$       if f$search("''test'.tmp1") .nes. "" then delete 'test'.tmp1;*
$	gawk --profile=ap-'test'.out -f 'test'.awk > _NL:
$	! sed <awkprof.out 1,2d >_profile3.tmp
$	if f$search("sys$disk:[]_''test'.tmp1;*") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;*
$	endif
$	gawk "NR>2" ap-'test'.out > sys$disk:[]_'test'.tmp1
$	if f$search("sys$disk:[]_''test'.tmp1;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp1;2
$	endif
$	convert/fdl=nla0: _'test'.tmp1 sys$disk:[]_'test'.tmp
$	convert/fdl=nla0: 'test'.ok _'test'.ok
$	cmp _'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;*,_'test'.ok;*,_'test'.tmp1;*,ap-'test'.out;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$next:	echo "next"
$	test_class = "gawk_ext"
$	set noOn
$	gawk "{next}" _NL:                                   > _next.tmp 2>&1
$	gawk "function f() {next}; {f()}" _NL:               >>_next.tmp 2>&1
$	gawk "function f() {next}; BEGIN{f()}" _NL:          >>_next.tmp 2>&1
$	gawk "function f() {next}; {f()}; END{f()}" _NL:     >>_next.tmp 2>&1
$	gawk "function f() {next}; BEGINFILE{f()}" _NL:      >>_next.tmp 2>&1
$	gawk "function f() {next}; {f()}; ENDFILE{f()}" _NL: >>_next.tmp 2>&1
$	set On
$	cmp next.ok sys$disk:[]_next.tmp
$	if $status
$	then
$	    rm _next.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$exit:	echo "exit"
$	test_class = "gawk_ext"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "PIPE command is available; running exit test"
$		On warning then  $
$		pipeok = 1
$	else
$	    echo "PIPE command is available; running exit test"
$	endif
$	set noOn
$	call/Output=_exit.tmp do__exit
$	set On
$	cmp exit.ok sys$disk:[]_exit.tmp
$	if $status
$	then
$	    rm _exit.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$do__exit: subroutine
$	gawk = gawk !PIPE won't propagate local symbols from outer procedure
$	x = "BEGIN{print 1; exit; print 2}; NR>1{print}; END{print 3; exit; print 4}"
$	pipe gawk -- "BEGIN { print ""a\nb"" }" | gawk -- "''x'"
$	echo "-- 1"
$	x = "function f(){exit}; END{print NR;f();print NR}"
$	pipe gawk -- "BEGIN { print ""a\nb"" }" | gawk -- "''x'"
$	echo "-- 2"
$	x = "function f(){exit}; NR>1 {f()}; END{print NR; f();print NR}"
$	pipe gawk -- "BEGIN { print ""a\nb"" }" | gawk -- "''x'"
$	echo "-- 3"
$	x = "function f(){exit}; NR>1 {f()}; END{print NR;print NR}"
$	pipe gawk -- "BEGIN { print ""a\nb"" }" | gawk -- "''x'"
$	echo "-- 4"
$	x = "function f(){exit}; BEGINFILE {f()}; NR>1 {f()}; END{print NR}"
$	pipe gawk -- "BEGIN { print ""a\nb"" }" | gawk -- "''x'"
$	echo "-- 5"
$!	Ugh; extra quotes are needed here to end up with """" after "''y'"
$!	expansion and finally "" when gawk actually sees its command line.
$	y = "function strip(f) { sub(/.*\//, """""""", f); return f };"
$	x = "BEGINFILE{if(++i==1) exit;}; END{print i, strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 6"
$	x = "BEGINFILE{if(++i==1) exit;}; ENDFILE{print i++}; END{print i, strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 7"
$	x = "function f(){exit}; BEGINFILE{i++ && f()}; END{print NR,strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 8"
$	x = "function f(){exit}; BEGINFILE{i++ && f()}; ENDFILE{print i}; END{print NR,strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 9"
$	x = "function f(){exit}; BEGINFILE{i++}; ENDFILE{f(); print i}; END{print NR,strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 10"
$	x = "function f(){exit}; BEGINFILE{i++}; ENDFILE{i>1 && f(); print i, strip(FILENAME)}"
$	gawk "''y'''x'" /dev/null exit.sh
$	echo "-- 11"
$ endsubroutine !do__exit
$
$vms_cmd:	echo "vms_cmd"
$	test_class = "vms_tests"
$	if f$search("vms_cmd.ok").eqs.""
$	then create vms_cmd.ok
World!
$	endif
$	gawk /Commands="BEGIN { print ""World!"" }" _NL: /Output=_vms_cmd.tmp
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp vms_cmd.ok sys$disk:[]_vms_cmd.tmp
$	if $status
$	then
$	    rm _vms_cmd.tmp;,vms_cmd.ok;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$vms_io1:	echo "vms_io1"
$	test_class = "vms_tests"
$	if f$search("vms_io1.ok").eqs.""
$	then create vms_io1.ok
Hello
$	endif
$ !	define/User dbg$input sys$command:
$	gawk -f - >_vms_io1.tmp
# prior to 3.0.4, gawk crashed doing any redirection after closing stdin
BEGIN { print "Hello" >"/dev/stdout" }
$	if f$search("sys$disk:[]_''test'.tmp;2") .nes. ""
$	then
$	    delete sys$disk:[]_'test'.tmp;2
$	endif
$	cmp vms_io1.ok sys$disk:[]_vms_io1.tmp
$	if $status
$	then
$	    rm _vms_io1.tmp;,vms_io1.ok;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$
$vms_io2:	echo "vms_io2"
$	test_class = "vms_tests"
$	if f$search("vms_io2.ok").eqs.""
$	then create vms_io2.ok
xyzzy
$	endif
$	! VAXCRTL created all files in stream_LF format unless explicitly
$	! overridden by the program; with DECC$SHR, a new version of an
$	! existing file inherits the previous version's record format;
$	! for gawk versions older than 3.1.7, that resulted in
$	! "can't redirect to `File' (invalid record attributes)"
$	! when an awk program used >"File" (internally, not on command line)
$	! and File already existed as a batch log file or PIPE output file
$	create /FDL=sys$input: _vms_io2.vfc
file
  organization sequential
record
  format VFC	! variable with fixed control area (default size 2 bytes)
  carriage_control print
$	set noOn
$ !	define/User dbg$input sys$command:
$	gawk -- "BEGIN { print ""xyzzy"" >""_vms_io2.vfc"" }" >_vms_io2.tmp 2>&1
$	set On
$	cmp _NL: sys$disk:[]_vms_io2.tmp
$	if $status then  rm _vms_io2.tmp;
$	cmp vms_io2.ok sys$disk:[]_vms_io2.vfc
$	if $status
$	then
$	    rm _vms_io2.vfc;*,vms_io2.ok;*
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$!
$inplace1:
$inplace2:
$	echo "''test'"
$  	if arch_name .eqs. "VAX"
$	then
$	    skip_reason = "ODS-5 required"
$	    gosub junit_report_skip
$	    return
$	endif
$	set process/parse=extended ! ODS-5 only
$	test_class = "shlib"
$	filefunc_file = "[-]gawkapi.o"
$	open/write awkfile _'test'.awk
$	write awkfile "@load ""inplace"""
$!	write awkfile "BEGIN {print ""before""}"
$	write awkfile "   {gsub(/foo/, ""bar""); print}"
$!	write awkfile "END {print ""after""}"
$	close awkfile
$	copy inplace^.1.in _'test'.1
$	copy inplace^.2.in _'test'.2
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f _'test'.awk _'test'.1 <inplace.in >_'test'.1.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.1.tmp
$	AWKLIBPATH_dir
$	gawk -f _'test'.awk _'test'.2 <inplace.in >_'test'.2.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.2.tmp
$	set On
$	cmp 'test'.1.ok sys$disk:[]_'test'.1.tmp
$	if $status then rm _'test'.1.tmp;,_'test'.1;
$	cmp 'test'.2.ok sys$disk:[]_'test'.2.tmp
$	if $status
$	then
$	    rm _'test'.2.tmp;,_'test'.2;,_'test'.awk;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$filefuncs:
$	echo "''test'"
$	test_class = "shlib"
$	filefunc_file = "[-]gawkapi.o"
$	open/write gapi 'filefunc_file'
$	close gapi
$	set noOn
$	AWKLIBPATH_dir
$	gawk -v builddir="sys$disk:[-]" -
	    -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	if f$search(filefunc_file) .nes. "" then rm 'filefunc_file';*
$	return
$!
$fnmatch:
$functab4:
$ordchr:
$revout:
$revtwoway:
$time:
$	echo "''test'"
$	test_class = "shlib"
$	filefunc_file = "[-]gawkapi.o"
$	open/write gapi 'filefunc_file'
$	close gapi
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	if f$search(filefunc_file) .nes. "" then rm 'filefunc_file';*
$	return
$!
$ordchr2:
$	echo "''test'"
$	test_class = "shlib"
$	filefunc_file = "[-]gawkapi.o"
$	open/write gapi 'filefunc_file'
$	close gapi
$	set noOn
$	AWKLIBPATH_dir
$	gawk --load ordchr "BEGIN {print chr(ord(""z""))}" > _'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	if f$search(filefunc_file) .nes. "" then rm 'filefunc_file';*
$	return
$
$!
$rwarray:
$	echo "''test'"
$	test_class = "shlib"
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp orig.out new.out
$	if $status
$	then
$	    open/append tout _'test'.tmp
$	    write tout "old and new are equal - GOOD"
$	    close tout
$	endif
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;,orig.bin;,orig.out;,new.out;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	return
$!
$readdir:
$fts:
$	echo "''test'"
$	test_class = "shlib"
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status
$	then
$	    call exit_code '$status' _'test'.tmp
$	    write sys$output _'test'.tmp
$	    gosub junit_report_fail_diff
$	else
$	    gosub junit_report_pass
$	    if f$search("_''test'.tmp") .nes. "" then rm _'test'.tmp;*
$	    if f$search("_''test'.") .nes. "" then rm _'test'.;*
$	endif
$	set On
$	return
$!
$readfile:
$	echo "''test'"
$	test_class = "shlib"
$	if f$search("sys$disk:[-]readfile.exe") .eqs. ""
$	then
$	    echo "Readfile extension not currently building on VMS."
$	    skip_reason = "Readfile not building on VMS"
$	    gosub junit_report_skip
$	    return
$	else
$	    echo "Surprise! Found the readfile extension so attempting test."
$	endif
$	set noOn
$	AWKLIBPATH_dir
$	gawk -l readfile -
	    "BEGIN {printf-f ""%s"", readfile("Makefile.in")}" >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	cmp Makefile.in sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	    copy Makefile.in 'test'.ok
$	endif
$	set On
$	return
$!
$readfile2:
$	echo "''test'"
$	test_class = "shlib"
$	if f$search("sys$disk:[-]readfile.exe") .eqs. ""
$	then
$	    echo "Readfile extension not currently building on VMS."
$	    skip_reason = "Readfile not building on VMS"
$	    gosub junit_report_skip
$	    return
$	else
$	    echo "Surprise! Found the readfile extension so attempting test."
$	endif
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status
$	then
$	    rm _'test'.tmp;
$	    gosub junit_report_pass
$	else
$	    gosub junit_report_fail_diff
$	endif
$	set On
$	return
$
$clean:
$	if f$search("_*.tmp")	 .nes."" then  rm _*.tmp;*
$	if f$search("_*.out")	 .nes."" then  rm _*.out;*
$	if f$search("_*.ok")	 .nes."" then  rm _*.ok;*
$	if f$search("_*.1")	 .nes."" then  rm _*.1;*
$	if f$search("_*.2")	 .nes."" then  rm _*.2;*
$	if f$search("_*.awk")	 .nes."" then  rm _*.awk;*
$	if f$search("_*.too")	 .nes."" then  rm _*.too;*
$	if f$search("awkprofile.out") .nes."" then  rm awkprofile.out;*
$	if f$search("out%.")	 .nes."" then  rm out%.;*
$	if f$search("strftime.in").nes."" then  rm strftime.in;*
$	if f$search("strftime.ok").nes."" then  rm strftime.ok;*
$	if f$search("test%.")	 .nes."" then  rm test%.;*
$	if f$search("seq.")	 .nes."" then  rm seq.;*
$	if f$search("_pid.in")	 .nes."" then  rm _pid.in;*
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]")	 .nes."" then  rm []junk.dir;1
$	if f$search("_vms_io2.vfc")	 .nes."" then  rm _vms_io2.vfc;*
$	return
$
$! try to determine what type of double precision floating point gawk uses
$calc_floatmode:
$	! this is fragile; it might break if gawk changes overflow handling
$	Set noOn
$	gawk -- "BEGIN {print 10^308}" >_NL: 2>&1
$	if $status
$	then	floatmode = 2		! IEEE T_float
$	else	gawk -- "BEGIN {print 10^307}" >_NL: 2>&1
$		if $status
$		then	floatmode = 1	! Alpha/VAX G_float
$		else	floatmode = 0	! VAX D_float
$		endif
$	endif
$	Set On
$	return
$
$! assign temporary value to logical name GAWKLOCALE unless it already has one
$! [ -z "$GAWKLOCALE" ] && GAWKLOCALE=en_US.UTF-8
$define_gawklocale:
$	if f$search("sys$i18n_locale:utf8-50.locale") .nes. ""
$	then
$	    define/user LC_ALL "utf8-50"
$	else
$	    define/user LC_ALL "utf8-20"
$	endif
$	return
$!
$define_gawklocale_ja_jp:
$!	@GAWKLOCALE=ja_JP.UTF-8 ; export GAWKLOCALE ; \
$	if f$search("sys$i18n_locale:ja_jp_utf-8.locale") .nes. ""
$	then
$	    define/user LC_ALL "ja_JP_utf-8"
$	else
$	    write sys$output "Substituting UTF8-20 for ja_JP.UTF-8"
$	    define/user LC_ALL "utf8-20"
$	endif
$	return
$
$! make sure that the specified file's longest-record-length field is set;
$! otherwise DIFF will choke if any record is longer than 512 bytes
$fixup_LRL: subroutine
$	lrl = 0	!VMS V5.5-2 didn't support the LRL argument yet
$	define/user sys$error _NL:
$	define/user sys$output _NL:
$	lrl = f$file_attribute(p1,"LRL")
$	if lrl.eq.0 then  lrl = f$file_attribute(p1,"MRS")
$	if lrl.eq.0
$	then	convert/fdl=sys$input: 'p1' *.*
file
 organization sequential
record
 format stream_lf
 size 32767
$		if $status .and. p2.eqs."purge" then  rm 'p1';-1
$	else	cmp _NL: _NL:	!deassign/user sys${error,output}
$	endif
$ endsubroutine !fixup_LRL
$
$! add a fake "EXIT CODE" record to the end of the temporary output file
$! to simulate the ``|| echo EXIT CODE $$? >>_$@'' shell script usage
$! Unix code = vms_code & (255 * 2^3) >> 3
$exit_code: subroutine
$	unix_status = (p1 .and. %x7f8) / 8
$	if f$trnlnm("FTMP").nes."" then  close/noLog ftmp
$	open/Append ftmp 'p2'
$	write ftmp "EXIT CODE: ",'unix_status'
$	close ftmp
$ endsubroutine !exit_code
$
$!NOTREACHED
$ exit
