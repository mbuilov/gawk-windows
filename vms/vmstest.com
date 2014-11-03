$! vmstest.com -- DCL script to perform test/Makefile actions for VMS
$!
$! Usage:
$!  $ set default [-.test]
$!  $ @[-.vms]vmstest.com bigtest
$! This assumes that newly built gawk.exe is in the next directory up.
$
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
$
$	echo	= "write sys$output"
$	cmp	= "diff/Output=_NL:/Maximum=1"
$	delsym  = "delete/symbol/local/nolog"
$	igncascmp = "''cmp'/Ignore=Case"
$	sumslp  = "edit/Sum"
$	rm	= "delete/noConfirm/noLog"
$	mv	= "rename/New_Vers"
$	gawk = "$sys$disk:[-]gawk"
$	AWKPATH_srcdir = "define/User AWKPATH sys$disk:[]"
$	AWKLIBPATH_dir = "define/User AWKLIBPATH sys$disk:[-]"
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
$	gosub list_of_tests
$	echo "done."
$	exit
$
$all:
$bigtest:	echo "bigtest..."
$		! omits "printlang" and "extra"
$		list = "basic unix_tests gawk_ext vms_tests charset_tests" -
		  + " machine_tests"
$		gosub list_of_tests
$		return
$
$basic:		echo "basic..."
$		list = "msg addcomma anchgsub argarray arrayparm arrayref" -
		  + " arrymem1 arrayprm2 arrayprm3 arryref2 arryref3" -
		  + " arryref4 arryref5 arynasty arynocls aryprm1 aryprm2" -
		  + " aryprm3 aryprm4 aryprm5 aryprm6 aryprm7 aryprm8" -
		  + " arysubnm asgext awkpath back89 backgsub childin" -
		  + " clobber closebad clsflnam compare compare2 concat1"
$		gosub list_of_tests
$		list = "concat2 concat3 concat4 convfmt datanonl defref" -
		  + " delargv delarprm delarpm2 delfunc dfastress dynlj" -
		  + " eofsplit exitval1" -
		  + " exitval2 fcall_exit fcall_exit2 fldchg fldchgnf" -
		  + " fnamedat fnarray fnarray2 fnaryscl fnasgnm fnmisc" -
		  + " fordel forref forsimp fsbs fsspcoln fsrs fstabplus" -
		  + " funsemnl funsmnam funstack getline getline2 getline3" -
		  + " getline4 getline5"
$		gosub list_of_tests
$		list = "getlnbuf getnr2tb getnr2tm gsubasgn gsubtest" -
		  + " gsubtst2 gsubtst3 gsubtst4 gsubtst5 gsubtst6" -
		  + " gsubtst7 gsubtst8 hex" -
		  + " hsprint inputred intest intprec iobug1" -
		  + " leaddig leadnl litoct longsub longwrds"-
		  + " manglprm math membug1 messages minusstr mmap8k" -
		  + " mtchi18n nasty nasty2 negexp negrange nested" -
		  + " nfldstr nfneg"
$		gosub list_of_tests
$		list = "nfset nlfldsep nlinstr nlstrina noeffect nofile" -
		  + " nofmtch noloop1 noloop2 nonl noparms nors nulrsend" -
		  + " numindex numsubstr octsub ofmt ofmtbig ofmtfidl" -
		  + " ofmta ofmts ofs1 onlynl opasnidx opasnslf paramdup" -
		  + " paramres paramtyp paramuninitglobal parse1 parsefld" -
		  + " parseme pcntplus" -
		  + " posix2008sub prdupval prec printf0 printf1 prmarscl"
$		gosub list_of_tests
$		list = "prmreuse prt1eval prtoeval rand range1 rebt8b1" -
		  + " redfilnm regeq regexprange regrange reindops reparse resplit rri1 rs rsnul1nl" -
		  + " rsnulbig rsnulbig2 rstest1 rstest2 rstest3 rstest4" -
		  + " rstest5 rswhite scalar sclforin sclifin sortempty" -
		  + " splitargv splitarr splitdef splitvar splitwht" -
		  + " strcat1 strtod strnum1 subamp subi18n"
$		gosub list_of_tests
$		list = "subsepnm subslash substr swaplns synerr1 synerr2" -
		  + " tradanch tweakfld uninit2 uninit3 uninit4 uninit5" -
		  + " uninitialized unterm uparrfs wideidx wideidx2" -
		  + " widesub widesub2 widesub3 widesub4 wjposer1 zeroe0" -
		  + " zeroflag zero2"
$		gosub list_of_tests
$		return
$
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
		  + " argtest arraysort backw badargs beginfile1 binmode1" -
		  + " colonwarn clos1way charasbytes delsub devfd devfd1 devfd2 dumpvars exit" -
		  + " fieldwdth fpat1 fpat2 fpat3 fpatnull funlen functab1" -
		  + " functab2 functab3 fsfwfs" -
		  + " fwtest fwtest2 fwtest3" -
		  + " gensub gensub2 getlndir gnuops2 gnuops3 gnureops" -
		  + " icasefs id icasers igncdym igncfs ignrcase ignrcas2" -
		  + " incdupe incdupe2 incdupe3 incdupe4 incdupe5 incdupe6 incdupe7"
$		gosub list_of_tests
$		list = "include2 indirectcall lint lintold lintwarn match1" -
		  + " match2 match3 manyfiles mbprintf3 mbstr1" -
		  + " nastyparm next nondec" -
		  + " nondec2 patsplit posix profile1 procinfs printfbad1" -
		  + " printfbad2 printfbad3 profile2 profile3 pty1" -
		  + " regx8bit rebuf reginttrad reint reint2 rsstart1 rsstart2 rsstart3 rstest6" -
		  + " shadow sortfor sortu split_after_fpat splitarg4" -
		  + " strtonum strftime switch2 symtab1 symtab2 symtab3" -
		  + " symtab4 symtab5 symtab6 symtab7 symtab8 symtab9"
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
$		list = "asort asorti fmttest fnarydel fnparydl lc_num1 mbfw1" -
		  + " mbprintf1 mbprintf2 rebt8b2 rtlenmb sort1 sprintfc"
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
$		list = "regtest inftest inet"
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
$extension:	echo "extension...."
$		list = "inplace1 filefuncs fnmatch fts functab4 ordchr" -
		+ " readdir revout revtwoway rwarray time"
		gosub list_of_tests
		return

$
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
$arrayprm2:
$arryref2:
$aryprm8:
$asgext:
$backgsub:
$backw:
$concat1:
$concat2:
$concat3:
$datanonl:
$delarpm2:
$fldchg:
$fldchgnf:
$fmttest:
$fordel:
$fpat1:
$fpat3:
$fpatnull:
$fsfwfs:
$fsrs:
$funlen:
$funstack:
$fwtest:
$fwtest2:
$fwtest3:
$gensub:
$getline3:
$getline4:
$getnr2tb:
$getnr2tm:
$gsubtest:
$gsubtst2:
$gsubtst4:
$gsubtst5:
$gsubtst7:
$gsubtst8:
$hex:
$icasers:
$igncfs:
$igncdym:
$indirectcall:
$inputred:
$leadnl:
$manglprm:
$match3:
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
$prec:
$prtoeval:
$range1:
$rebuf:
$regeq:
$reindops:
$reparse:
$rsnul1nl:
$rsstart1:
$rstest1:
$rstest2:
$rstest3:
$rstest6:
$rswhite:
$sortempty:
$sortfor:
$split_after_fpat:
$splitarg4:
$splitargv:
$splitarr:
$splitvar:
$sprintfc:
$strcat1:
$strtod:
$subsepnm:
$swaplns:
$uparrfs:
$wjposer1:
$zeroe0:
$! common with 'test'.in
$	echo "''test'"
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$! more common tests, without a data file: gawk -f 'test'.awk
$aarray1:
$aasort:
$aasorti:
$arrayref:
$arraysort:
$arrymem1:
$arynasty:
$arysubnm:
$asort:
$asorti:
$closebad:
$compare2:
$convfmt:
$delargv:
$delarprm:
$delsub:
$dynlj:
$fnarydel:
$fnparydl:
$fpat2:
$forref:
$forsimp:
$funsemnl:
$gensub2:
$gnuops2:
$gnuops3:
$gnureops:
$hsprint:
$icasefs:
$intest:
$match1:
$math:
$minusstr:
$negrange:
$nlstrina:
$nondec:
$octsub:
$ofmta:
$paramtyp:
$paramuninitglobal:
$patsplit:
$pcntplus:
$printf1:
$procinfs:
$prt1eval:
$rebt8b1:
$rebt8b2:
$regexprange:
$regrange:
$regx8bit:
$sort1:
$sortu:
$splitdef:
$splitwht:
$strnum1:
$strtonum:
$substr:
$switch2:
$zero2:
$zeroflag:
$! common without 'test'.in
$	echo "''test'"
$	gawk -f 'test'.awk >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$colonwarn:	echo "''test'"
$	gawk -f 'test'.awk 1 < 'test'.in > _'test'.tmp
$	gawk -f 'test'.awk 2 < 'test'.in > _'test'_2.tmp
$	gawk -f 'test'.awk 3 < 'test'.in > _'test'_3.tmp
$	append _'test'_2.tmp,_'test'_3.tmp _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp;1
$	if $status then  rm _'test'*.tmp;*
$	return
$
$double1:
$double2:
$lc_num1:
$mbprintf1:
$	echo "''test' skipped"
$	return
$
$getline5:	echo "''test'"
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
$	if $status then  rm _'test'.tmp;*,f.;*
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
$
$printlang:
$! the default locale is C, with LC_LANG and LC_ALL left empty;
$! showing that at the very beginning may cause some confusion about
$! whether gawk requires those, so we don't run this as the first test
$	gawk -f printlang.awk
$	return
$
$poundbang:
$pty1:
$	echo "''test': not supported"
$	return
$
$
$messages:	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk > _out2 >& _out3
$	cmp out1.ok sys$disk:[]_out1.
$	if $status then  rm _out1.;
$	cmp out2.ok sys$disk:[]_out2.
$	if $status then  rm _out2.;
$	cmp out3.ok sys$disk:[]_out3.
$	if $status then  rm _out3.;
$	set On
$	return
$
$argarray:	echo "argarray"
$	define/User TEST "test"			!this is useless...
$	gawk -f argarray.awk ./argarray.in - >_argarray.tmp
just a test
$	cmp argarray.ok sys$disk:[]_argarray.tmp
$	if $status then  rm _argarray.tmp;
$	return
$
$fstabplus:	echo "fstabplus"
$	gawk -f fstabplus.awk >_fstabplus.tmp
1		2
$	cmp fstabplus.ok sys$disk:[]_fstabplus.tmp
$	if $status then  rm _fstabplus.tmp;
$	return
$
$longwrds:	echo "longwrds"
$	gawk -v "SORT=sort sys$input: _longwrds.tmp" -f longwrds.awk longwrds.in >_NL:
$	cmp longwrds.ok sys$disk:[]_longwrds.tmp
$	if $status then  rm _longwrds.tmp;
$	return
$
$fieldwdth:	echo "fieldwdth"
$	gawk -v "FIELDWIDTHS=2 3 4" "{ print $2}" >_fieldwdth.tmp
123456789
$	cmp fieldwdth.ok sys$disk:[]_fieldwdth.tmp
$	if $status then  rm _fieldwdth.tmp;
$	return
$
$ignrcase:	echo "ignrcase"
$	gawk -v "IGNORECASE=1" "{ sub(/y/, """"); print}" >_ignrcase.tmp
xYz
$	cmp ignrcase.ok sys$disk:[]_ignrcase.tmp
$	if $status then  rm _ignrcase.tmp;
$	return
$
$regtest:
$  if f$search("regtest.com").eqs.""
$  then echo "regtest:  not available"
$  else echo "regtest"
$	echo "Some of the output from regtest is very system specific, do not"
$	echo "be distressed if your output differs from that distributed."
$	echo "Manual inspection is called for."
$	@regtest.com
$ endif
$	return
$
$posix: echo "posix"
$	gawk -f posix.awk >_posix.tmp
1:2,3 4
$	cmp posix.ok sys$disk:[]_posix.tmp
$	if $status then  rm _posix.tmp;
$	return
$
$manyfiles:	echo "manyfiles"
$!! this used to use a hard-coded value of 300 simultaneously open
$!! files, but if our open file quota is generous enough then that
$!! wouldn't exercise the ability to handle more than the maximum
$!! number allowed at once
$	f_cnt = 300
$	chnlc = f$getsyi("CHANNELCNT")
$	fillm = f$getjpi("","FILLM")
$	if fillm.ge.chnlc then  fillm = chnlc - 1
$	if fillm.ge.f_cnt then  f_cnt = fillm + 10
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]").eqs."" then  create/Dir/Prot=(O:rwed) [.junk]
$	gawk -- "BEGIN {for (i = 1; i <= ''f_cnt'; i++) print i, i}" >_manyfiles.dat
$	echo "(processing ''f_cnt' files; this may take quite a while)"
$	set noOn	! continue even if gawk fails
$	gawk -f manyfiles.awk _manyfiles.dat _manyfiles.dat
$	set On
$	define/User sys$error _NL:
$	define/User sys$output _manyfiles.tmp
$	search/Match=Nor/Output=_NL:/Log [.junk]*.* ""
$!/Log output: "%SEARCH-S-NOMATCH, <filename> - <#> records" plus 1 line summary
$	gawk -v "F_CNT=''f_cnt'" -f - _manyfiles.tmp
$deck	!some input begins with "$"
$4 != 2 {++count}
END {if (NR != F_CNT+1 || count != 1) {print "\nFailed!"}}
$eod
$	rm _manyfiles.tmp;,_manyfiles.dat;,[.junk]*.*;*,[]junk.dir;
$	return
$
$compare:	echo "compare"
$	gawk -f compare.awk 0 1 compare.in >_compare.tmp
$	cmp compare.ok sys$disk:[]_compare.tmp
$	if $status then  rm _compare.tmp;
$	return
$
$rs:		echo "rs"
$	gawk -v "RS=" "{ print $1, $2}" rs.in >_rs.tmp
$	cmp rs.ok sys$disk:[]_rs.tmp
$	if $status then  rm _rs.tmp;
$	return
$
$fsbs:		echo "fsbs"
$	gawk -v "FS=\" "{ print $1, $2 }" fsbs.in >_fsbs.tmp
$	cmp fsbs.ok sys$disk:[]_fsbs.tmp
$	if $status then  rm _fsbs.tmp;
$	return
$
$inftest:	echo "inftest"
$     !!  echo "This test is very machine specific..."
$	set noOn
$	gawk -f inftest.awk >_inftest.tmp
$     !!  cmp inftest.ok sys$disk:[]_inftest.tmp	!just care that gawk doesn't crash...
$	if $status then  rm _inftest.tmp;
$	set On
$	return
$
$getline2:	echo "getline2"
$	gawk -f getline2.awk getline2.awk getline2.awk >_getline2.tmp
$	cmp getline2.ok sys$disk:[]_getline2.tmp
$	if $status then  rm _getline2.tmp;
$	return
$
$rand:		echo "rand"
$	echo "The following line should just be 19 random numbers between 1 and 100"
$	echo ""
$	gawk -f rand.awk
$	return
$
$negexp:	echo "negexp"
$	gawk "BEGIN { a = -2; print 10^a }" >_negexp.tmp
$	cmp negexp.ok sys$disk:[]_negexp.tmp
$	if $status then  rm _negexp.tmp;
$	return
$
$awkpath:	echo "awkpath"
$	define/User AWK_LIBRARY [],[.lib]
$	gawk -f awkpath.awk >_awkpath.tmp
$	cmp awkpath.ok sys$disk:[]_awkpath.tmp
$	if $status then  rm _awkpath.tmp;
$	return
$
$argtest:	echo "argtest"
$	gawk -f argtest.awk -x -y abc >_argtest.tmp
$	cmp argtest.ok sys$disk:[]_argtest.tmp
$	if $status then  rm _argtest.tmp;
$	return
$
$badargs:	echo "badargs"
$	on error then continue
$	gawk -f 2>&1 >_badargs.too
$!	search/Match=Nor _badargs.too "patchlevel" /Output=_badargs.tmp
$	gawk "/patchlevel/{next}; {gsub(""\"""",""'""); print}" <_badargs.too >_badargs.tmp
$	cmp badargs.ok sys$disk:[]_badargs.tmp
$	if $status then  rm _badargs.tmp;,_badargs.too;
$	return
$
$nonl:		echo "nonl"
$	! This one might fail, depending on the tool used to unpack the
$	! distribution.  Some will add a final newline if the file lacks one.
$	AWKPATH_srcdir
$	gawk --lint -f nonl.awk _NL: >_nonl.tmp 2>&1
$	cmp nonl.ok sys$disk:[]_nonl.tmp
$	if $status then  rm _nonl.tmp;
$	return
$
$defref:	echo "defref"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f defref.awk >_defref.tmp 2>&1
$	if .not. $status then call exit_code '$status' _defref.tmp
$	set On
$	cmp defref.ok sys$disk:[]_defref.tmp
$	if $status then  rm _defref.tmp;
$	return
$
$nofmtch:	echo "nofmtch"
$	AWKPATH_srcdir
$	gawk --lint -f nofmtch.awk >_nofmtch.tmp 2>&1
$	cmp nofmtch.ok sys$disk:[]_nofmtch.tmp
$	if $status then  rm _nofmtch.tmp;
$	return
$
$strftime:	echo "strftime"
$	! this test could fail on slow machines or on a second boundary,
$	! so if it does, double check the actual results
$	! This test needs SYS$TIMEZONE_NAME and SYS$TIMEZONE_RULE
$	! to be properly defined.
$	! This test now needs GNV Corutils to work
$	date_bin = "gnv$gnu:[bin]gnv$date.exe"
$	if f$search(date_bin) .eqs. ""
$	then
$		echo "''test' skipped"
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
$	if $status then  rm _strftime.tmp;,strftime.ok;*,strftime.in;*
$	set On
$	return
$
$litoct:	echo "litoct"
$	gawk --traditional -f litoct.awk >_litoct.tmp
ab
$	cmp litoct.ok sys$disk:[]_litoct.tmp
$	if $status then  rm _litoct.tmp;
$	return
$
$resplit:	echo "resplit"
$	gawk -- "{ FS = "":""; $0 = $0; print $2 }" >_resplit.tmp
a:b:c d:e:f
$	cmp resplit.ok sys$disk:[]_resplit.tmp
$	if $status then  rm _resplit.tmp;
$	return
$
$intprec:	echo "intprec"
$	gawk -f intprec.awk >_intprec.tmp 2>&1
$	cmp intprec.ok sys$disk:[]_intprec.tmp
$	if $status then  rm _intprec.tmp;
$	return
$
$incdupe:   echo "''test'"
$   set noOn
$   gawk --lint -i inclib -i inclib.awk "BEGIN {print sandwich(""a"", ""b"", ""c"")}" > _'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$incdupe2:   echo "''test'"
$   set noOn
$   gawk --lint -f inclib -f inclib.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$incdupe3:   echo "''test'"
$   gawk --lint -f hello -f hello.awk >_'test'.tmp 2>&1
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   return
$
$incdupe4:   echo "''test'"
$   set NoOn
$   gawk --lint -f hello -i hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$incdupe5:   echo "''test'"
$   set NoOn
$   gawk --lint -i hello -f hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$incdupe6:   echo "''test'"
$   set NoOn
$   gawk --lint -i inchello -f hello.awk >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$incdupe7:   echo "''test'"
$   set NoOn
$   gawk --lint -f hello -i inchello >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$include2:   echo "''test'"
$   gawk -i inclib "BEGIN {print sandwich(""a"", ""b"", ""c"")}" >_'test'.tmp 2>&1
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   return
$
$id:
$symtab1:
$symtab2:
$symtab3:   echo "''test'"
$   set noOn
$   gawk -f 'test'.awk  >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$symtab4:
$symtab5:
$symtab7:   echo "''test'"
$   set noOn
$   gawk -f 'test'.awk <'test'.in >_'test'.tmp 2>&1
$   if .not. $status then call exit_code '$status' _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*
$   set On
$   return
$
$symtab6:   echo "''test'"
$   set noOn
$   gawk -d__'test'.tmp -f 'test'.awk
$   pipe search __'test'.tmp "ENVIRON","PROCINFO" /match=nor > _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*,__'test'.tmp;*
$   set On
$   return
$
$symtab8:   echo "''test'"
$   set noOn
$   gawk -d__'test'.tmp -f 'test'.awk 'test'.in > _'test'.tmp
$   pipe search __'test'.tmp "ENVIRON","PROCINFO","FILENAME" /match=nor > ___'test'.tmp
$   convert/append ___'test'.tmp _'test'.tmp
$   cmp 'test'.ok sys$disk:[]_'test'.tmp
$   if $status then rm _'test'.tmp;*,__'test'.tmp;*,___'test'.tmp;*
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
$   old_rm = rm			! Remember old definition of rm
$   rm = "!"			! Redefine rm to something harmless
$   gawk -f 'test'.awk  >_'test'.tmp
$   rm = old_rm			! Restore old value
$   delsym old_rm
$   cmp 'test'.ok sys$disk:[]_'test'.tmp;-0	! -0 is the lowest version
$   if $status then rm _'test'.tmp;*,testit.txt;*
$   return
$
$childin:	echo "''test'"
$	cat = "type sys$input"
$	gawk -f 'test'.awk < 'test'.in > _'test'.tmp
$	delsym cat
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$noeffect:	echo "noeffect"
$	AWKPATH_srcdir
$	gawk --lint -f noeffect.awk >_noeffect.tmp 2>&1
$	cmp noeffect.ok sys$disk:[]_noeffect.tmp
$	if $status then  rm _noeffect.tmp;
$	return
$
$numsubstr:	echo "numsubstr"
$	AWKPATH_srcdir
$	gawk -f numsubstr.awk numsubstr.in >_numsubstr.tmp
$	cmp numsubstr.ok sys$disk:[]_numsubstr.tmp
$	if $status then  rm _numsubstr.tmp;
$	return
$
$prmreuse:	echo "prmreuse"
$	if f$search("prmreuse.ok").eqs."" then  create prmreuse.ok
$	gawk -f prmreuse.awk >_prmreuse.tmp
$	cmp prmreuse.ok sys$disk:[]_prmreuse.tmp
$	if $status then  rm _prmreuse.tmp;
$	return
$
$fflush:
$	echo "fflush: not supported"
$	return
$!!fflush:	echo "fflush"
$	! hopelessly Unix-specific
$!!	@fflush.sh >_fflush.tmp
$	cmp fflush.ok sys$disk:[]_fflush.tmp
$	if $status then  rm _fflush.tmp;
$	return
$
$getlnhd:
$	echo "getlnhd skipped"
$	return
$!!getlnhd:	echo "getlnhd"
$	gawk -f getlnhd.awk >_getlnhd.tmp
$	cmp getlnhd.ok sys$disk:[]_getlnhd.tmp
$	if $status then  rm _getlnhd.tmp;
$	return
$
$tweakfld:	echo "tweakfld"
$	gawk -f tweakfld.awk tweakfld.in >_tweakfld.tmp
$	if f$search("errors.cleanup").nes."" then  rm errors.cleanup;*
$	cmp tweakfld.ok sys$disk:[]_tweakfld.tmp
$	if $status then  rm _tweakfld.tmp;
$	return
$
$clsflnam:	echo "clsflnam"
$	if f$search("clsflnam.ok").eqs."" then  create clsflnam.ok
$	gawk -f clsflnam.awk clsflnam.in >_clsflnam.tmp 2>&1
$	cmp clsflnam.ok sys$disk:[]_clsflnam.tmp
$	if $status then  rm _clsflnam.tmp;
$	return
$
$mmap8k:	echo "mmap8k"
$	gawk "{ print }" mmap8k.in >_mmap8k.tmp
$	cmp mmap8k.in sys$disk:[]_mmap8k.tmp
$	if $status then  rm _mmap8k.tmp;
$	return
$
$eofsplit:	echo "eofsplit"
$	if f$search("eofsplit.ok").eqs."" then  create eofsplit.ok
$	gawk -f eofsplit.awk >_eofsplit.tmp
$	cmp eofsplit.ok sys$disk:[]_eofsplit.tmp
$	if $status then  rm _eofsplit.tmp;
$	return
$
$back89:		echo "back89"
$	gawk "/a\8b/" back89.in >_back89.tmp
$	cmp back89.ok sys$disk:[]_back89.tmp
$	if $status then  rm _back89.tmp;
$	return
$
$tradanch:	echo "tradanch"
$	if f$search("tradanch.ok").eqs."" then  create tradanch.ok
$	gawk --traditional -f tradanch.awk tradanch.in >_tradanch.tmp
$	cmp tradanch.ok sys$disk:[]_tradanch.tmp
$	if $status then  rm _tradanch.tmp;
$	return
$
$reginttrad:	echo "''test'"
$	gawk --traditional -r -f 'test'.awk >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$pid:		echo "pid"
$	pid = f$integer("%x" + f$getjpi("","PID"))
$	ppid = f$integer("%x" + f$getjpi("","OWNER"))
$	gawk -v "ok_pid=''pid'" -v "ok_ppid=''ppid'" -f pid.awk >_pid.tmp >& _NL:
$	cmp pid.ok sys$disk:[]_pid.tmp
$	if $status then  rm _pid.tmp;
$	return
$
$strftlng:	echo "strftlng"
$	define/User TZ "UTC"		!useless
$	gawk -f strftlng.awk >_strftlng.tmp
$	cmp strftlng.ok sys$disk:[]_strftlng.tmp
$	if $status then  rm _strftlng.tmp;
$	return
$
$nfldstr:	echo "nfldstr"
$	if f$search("nfldstr.ok").eqs."" then  create nfldstr.ok
$	gawk "$1 == 0 { print ""bug"" }" >_nfldstr.tmp

$	cmp nfldstr.ok sys$disk:[]_nfldstr.tmp
$	if $status then  rm _nfldstr.tmp;
$	return
$
$nors:		echo "nors"
$!! there's no straightforward way to supply non-terminated input on the fly
$!!	@echo A B C D E | tr -d '\12' | $(AWK) '{ print $$NF }' - $(srcdir)/nors.in > _$@
$!! so just read a line from sys$input instead
$	gawk "{ print $NF }" - nors.in >_nors.tmp
A B C D E
$	cmp nors.ok sys$disk:[]_nors.tmp
$	if $status then  rm _nors.tmp;
$	return
$
$reint:		echo "reint"
$	gawk --re-interval -f reint.awk reint.in >_reint.tmp
$	cmp reint.ok sys$disk:[]_reint.tmp
$	if $status then  rm _reint.tmp;
$	return
$
$noparms:	echo "noparms"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f noparms.awk >_noparms.tmp 2>&1
$	if .not. $status then call exit_code '$status' _noparms.tmp
$	set On
$	cmp noparms.ok sys$disk:[]_noparms.tmp
$	if $status then  rm _noparms.tmp;
$	return
$
$pipeio1:	echo "pipeio1"
$	cat = "TYPE"	!close enough, as long as we avoid .LIS default suffix
$	define/User test1 []test1.
$	define/User test2 []test2.
$	gawk -f pipeio1.awk >_pipeio1.tmp
$	rm test1.;,test2.;
$	cmp pipeio1.ok sys$disk:[]_pipeio1.tmp
$	if $status then  rm _pipeio1.tmp;
$	return
$
$pipeio2:
$	echo "pipeio2 skipped"
$	return
$!!pipeio2:	echo "pipeio2"
$	cat = "gawk -- {print}"
$	tr  = "??"	!unfortunately, no trivial substitution available...
$	gawk -v "SRCDIR=." -f pipeio2.awk >_pipeio2.tmp
$	cmp pipeio2.ok sys$disk:[]_pipeio2.tmp
$	if $status then  rm _pipeio2.tmp;
$	return
$
$clobber:	echo "clobber"
$	gawk -f clobber.awk >_clobber.tmp
$	cmp clobber.ok sys$disk:[]seq.
$	if $status then  rm seq.;*
$	cmp clobber.ok sys$disk:[]_clobber.tmp
$	if $status then  rm _clobber.tmp;
$	return
$
$nasty:	echo "nasty"
$	set noOn
$	gawk -f nasty.awk >_nasty.tmp
$	call fixup_LRL nasty.ok
$	call fixup_LRL _nasty.tmp "purge"
$	cmp nasty.ok sys$disk:[]_nasty.tmp
$	if $status
$	then
$	    rm _nasty.tmp;
$	    file = "lcl_root:[]nasty.ok"
$	    if f$search(file) .nes. "" then rm 'file';*
$	endif
$	set On
$	return
$
$nasty2:	echo "nasty2"
$	set noOn
$	gawk -f nasty2.awk >_nasty2.tmp
$	call fixup_LRL nasty2.ok
$	call fixup_LRL _nasty2.tmp "purge"
$	cmp nasty2.ok sys$disk:[]_nasty2.tmp
$	if $status
$	then
$	    rm _nasty2.tmp;
$	    file = "lcl_root:[]nasty2.ok"
$	    if f$search(file) .nes. "" then rm 'file';*
$	endif
$	set On
$	return
$
$aadelete1:
$aadelete2:
$arrayparm:
$fnaryscl:
$functab1:
$functab2:
$functab3:
$nastyparm:
$opasnslf:
$opasnidx:
$printfbad1:
$prmarscl:
$subslash:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$arynocls:	echo "arynocls"
$	gawk -v "INPUT"=arynocls.in -f arynocls.awk >_arynocls.tmp
$	cmp arynocls.ok sys$disk:[]_arynocls.tmp
$	if $status then  rm _arynocls.tmp;
$	return
$
$getlnbuf:	echo "getlnbuf"
$	gawk -f getlnbuf.awk getlnbuf.in >_getlnbuf.tmp
$	gawk -f gtlnbufv.awk getlnbuf.in >_getlnbuf.too
$	cmp getlnbuf.ok sys$disk:[]_getlnbuf.tmp
$	if $status then  rm _getlnbuf.tmp;
$	cmp getlnbuf.ok sys$disk:[]_getlnbuf.too
$	if $status then  rm _getlnbuf.too;
$	return
$
$lint:	echo "lint"
$	AWKPATH_srcdir
$	gawk -f lint.awk >_lint.tmp 2>&1
$	cmp lint.ok sys$disk:[]_lint.tmp
$	if $status then  rm _lint.tmp;
$	return
$
$lintold:	echo "lintold"
$	AWKPATH_srcdir
$	gawk -f lintold.awk --lint-old <lintold.in >_lintold.tmp 2>&1
$	cmp lintold.ok sys$disk:[]_lintold.tmp
$	if $status then  rm _lintold.tmp;
$	return
$
$ofmtbig:	echo "ofmtbig"
$	set noOn
$	gawk -f ofmtbig.awk ofmtbig.in >_ofmtbig.tmp 2>&1
$	set On
$	cmp ofmtbig.ok sys$disk:[]_ofmtbig.tmp
$	if $status then  rm _ofmtbig.tmp;
$	return
$
$inetechu:	echo "inetechu"
$	echo "this test is for establishing UDP connections"
$	set noOn
$	gawk -- "BEGIN {print """" |& ""/inet/udp/0/127.0.0.1/9""}"
$	set On
$	return
$
$inetecht:	echo "inetecht"
$	echo "this test is for establishing TCP connections"
$	set noOn
$	gawk -- "BEGIN {print """" |& ""/inet/tcp/0/127.0.0.1/9""}"
$	set On
$	return
$
$inetdayu:	echo "inetdayu"
$	echo "this test is for bidirectional UDP transmission"
$	set noOn
$	gawk -f - _NL:
BEGIN { print "" |& "/inet/udp/0/127.0.0.1/13";
	"/inet/udp/0/127.0.0.1/13" |& getline; print $0}
$	set On
$	return
$
$inetdayt:	echo "inetdayt"
$	echo "this test is for bidirectional TCP transmission"
$	set noOn
$	gawk -f - _NL:
BEGIN { print "" |& "/inet/tcp/0/127.0.0.1/13";
	"/inet/tcp/0/127.0.0.1/13" |& getline; print $0}
$	set On
$	return
$
$redfilnm:	echo "redfilnm"
$	gawk -f redfilnm.awk srcdir="." redfilnm.in >_redfilnm.tmp
$	cmp redfilnm.ok sys$disk:[]_redfilnm.tmp
$	if $status then  rm _redfilnm.tmp;
$	return
$
$leaddig:	echo "leaddig"
$	gawk -v "x=2E"  -f leaddig.awk >_leaddig.tmp
$	cmp leaddig.ok sys$disk:[]_leaddig.tmp
$	if $status then  rm _leaddig.tmp;
$	return
$
$clos1way:
$	echo "clos1way: not supported"
$	return
$!!clos1way:	echo "clos1way"
$	gawk -f clos1way.awk >_clos1way.tmp
$	cmp clos1way.ok sys$disk:[]_clos1way.tmp
$	if $status then  rm _clos1way.tmp;
$	return
$
$shadow:	echo "shadow"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f shadow.awk >_shadow.tmp 2>&1
$	set On
$	cmp shadow.ok sys$disk:[]_shadow.tmp
$	if $status then  rm _shadow.tmp;
$	return
$
$lintwarn:	echo "lintwarn"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f lintwarn.awk >_lintwarn.tmp 2>&1
$	if .not. $status then call exit_code '$status' _lintwarn.tmp
$	set On
$	cmp lintwarn.ok sys$disk:[]_lintwarn.tmp
$	if $status then  rm _lintwarn.tmp;
$	return
$
$longsub:	echo "longsub"
$	set noOn
$	gawk -f longsub.awk longsub.in >_longsub.tmp
$!! the records here are too long for DIFF to handle
$!! so assume success as long as gawk doesn't crash
$!!	call fixup_LRL longsub.ok
$!!	call fixup_LRL _longsub.tmp "purge"
$!!	cmp longsub.ok sys$disk:[]_longsub.tmp
$	if $status then  rm _longsub.tmp;
$	set On
$	return
$
$arrayprm3:	echo "arrayprm3"
$	gawk -f arrayprm3.awk arrayprm3.in >_arrayprm3.tmp
$	cmp arrayprm3.ok sys$disk:[]_arrayprm3.tmp
$	if $status then  rm _arrayprm3.tmp;
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
$dfastress:
$nfneg:
$numindex:
$scalar:
$sclforin:
$sclifin:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
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
$	if $status then  rm _'test'.tmp;
$	return
$
$exitval2:	echo "exitval2 skipped"
$	return
$!!exitval2:	echo "exitval2"
$	gawk -f exitval2.awk exitval2.in >_exitval2.tmp
$	cmp exitval2.ok sys$disk:[]_exitval2.tmp
$	if $status then  rm _exitval2.tmp;
$	return
$
$delfunc:
$fcall_exit2:
$fnamedat:
$fnarray2:
$fnasgnm:
$fnmisc:
$gsubasgn:
$unterm:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$getline:	echo "getline skipped"
$	return
$!!getline:	echo "getline"
$	gawk -f getline.awk getline.in >_getline.tmp
$	cmp getline.ok sys$disk:[]_getline.tmp
$	if $status then  rm _getline.tmp;
$	return
$
$gsubtst3:	echo "gsubtst3"
$	gawk --re-interval -f gsubtst3.awk gsubtst3.in >_gsubtst3.tmp
$	cmp gsubtst3.ok sys$disk:[]_gsubtst3.tmp
$	if $status then  rm _gsubtst3.tmp;
$	return
$
$! FIXME: gawk generates an extra, empty output file while running this test...
$iobug1:	echo "iobug1"
$	cat = "TYPE sys$input:"
$	true = "exit 1"	!success
$		oldout = f$search("_iobug1.tmp;")
$	gawk -f iobug1.awk iobug1.in >_iobug1.tmp
$		badout = f$search("_iobug1.tmp;-1")
$		if badout.nes."" .and. badout.nes.oldout then  rm 'badout'
$	cmp iobug1.ok sys$disk:[]_iobug1.tmp
$	if $status then  rm _iobug1.tmp;
$	return
$
$rstest4:	echo "rstest4 skipped"
$	return
$!!rstest4:	echo "rstest4"
$	gawk -f rstest4.awk rstest4.in >_rstest4.tmp
$	cmp rstest4.ok sys$disk:[]_rstest4.tmp
$	if $status then  rm _rstest4.tmp;
$	return
$
$rstest5:	echo "rstest5 skipped"
$	return
$!!rstest5:	echo "rstest5"
$	gawk -f rstest5.awk rstest5.in >_rstest5.tmp
$	cmp rstest5.ok sys$disk:[]_rstest5.tmp
$	if $status then  rm _rstest5.tmp;
$	return
$
$fcall_exit:
$fnarray:
$funsmnam:
$match2:
$paramdup:
$paramres:
$parseme:
$synerr1:
$synerr2:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$uninit2:
$uninit3:
$uninit4:
$uninit5:
$uninitialized:
$	echo "''test'"
$	gawk --lint -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$space:		echo "space"
$	set noOn
$	gawk -f " " space.awk >_space.tmp 2>&1
$	if .not. $status then call exit_code '$status' _space.tmp
$	set On
$!	we get a different error from what space.ok expects
$	gawk "{gsub(""file specification syntax error"", ""no such file or directory""); print}" -
		_space.tmp >_space.too
$	rm _space.tmp;
$	mv _space.too _space.tmp
$	igncascmp space.ok sys$disk:[]_space.tmp
$	if $status then  rm _space.tmp;
$	return
$
$posix2008sub:
$printf0:
$	echo "''test'"
$	gawk --posix -f 'test'.awk >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$rsnulbig:	echo "rsnulbig"
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
$	if $status then  rm _rsnulbig.tmp;
$	return
$
$rsnulbig2:	echo "rsnulbig2"
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
$	if $status then  rm _rsnulbig2.tmp;
$	return
$
$subamp:
$wideidx:
$widesub2:
$widesub3:
$	echo "''test'"
$	gosub define_gawklocale
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$ignrcas2:
$!!lc_num1:
$wideidx2:
$widesub:
$widesub4:
$	echo "''test'"
$	gosub define_gawklocale
$	gawk -f 'test'.awk >_'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$! This test is somewhat suspect for vms due to exit code manipulation
$exitval1:	echo "exitval1"
$	gawk -f exitval1.awk >_exitval1.tmp 2>&1
$	if $status then  call exit_code '$status' _exitval1.tmp
$	cmp exitval1.ok sys$disk:[]_exitval1.tmp
$	if $status then  rm _exitval1.tmp;
$	return
$
$fsspcoln:	echo "fsspcoln"
$	gawk -f fsspcoln.awk "FS=[ :]+" fsspcoln.in >_forspcoln.tmp
$	cmp fsspcoln.ok sys$disk:[]_forspcoln.tmp
$	if $status then  rm _forspcoln.tmp;
$	return
$
$getlndir:	echo "getlndir"
$	! assume we're running in the test subdirectory; we don't want to
$	! perform a messy conversion of [] into its file specification
$	gawk -v "SRCDIR=[-]test.dir" -f getlndir.awk >_getlndir.tmp
$!	getlndir.ok expects "Is a directory", we see "is a directory"
$	igncascmp getlndir.ok sys$disk:[]_getlndir.tmp
$	if $status then  rm _getlndir.tmp;
$	return
$
$rsstart2:	echo "rsstart2"
$	gawk -f rsstart2.awk rsstart1.in >_rsstart2.tmp
$	cmp rsstart2.ok sys$disk:[]_rsstart2.tmp
$	if $status then  rm _rsstart2.tmp;
$	return
$
$! rsstart3 fails, presumeably due to PIPE's use of print file format
$rsstart3:	echo "rsstart3 skipped"
$	return
$!!rsstart3:	echo "rsstart3"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "With PIPE, ''test' will finish quickly."
$		On warning then  $
$		pipeok = 1
$	endif
$	! head rsstart1.in | gawk -f rsstart2.awk >_rsstart3.tmp
$	! splitting this into two steps would make it the same as rsstart2
$	set noOn
$	pipe -
	gawk -- "FNR <= 10" rsstart1.in | -
	gawk -f rsstart2.awk >_rsstart3.tmp
$	set On
$	cmp rsstart3.ok sys$disk:[]_rsstart3.tmp
$	if $status then  rm _rsstart3.tmp;
$	return
$
$rtlen:
$rtlen01:
$rtlenmb:
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
$   else
$	call/Output=_rtlen01.tmp do__rtlen01
$	! first test yields 1 instead of 0 due to forced newline
$	gawk -- "FNR==1 {sub(""1"",""0"")}; {print}" _rtlen01.tmp >_rtlen01.too
$	rm _rtlen01.tmp;
$	mv _rtlen01.too _rtlen01.tmp
$   endif
$	cmp 'f' sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
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
$	gawk --non-decimal-data -v "a=0x1" -f nondec2.awk >_nondec2.tmp
$	cmp nondec2.ok sys$disk:[]_nondec2.tmp
$	if $status then  rm _nondec2.tmp;
$	return
$
$nofile:	echo "nofile"
$!	gawk "{}" no/such/file >_nofile.tmp 2>&1
$!	nofile.ok expects no/such/file, but using that name in the test would
$!	yield "file specification syntax error" instead of "no such file..."
$	set noOn
$	gawk "{}" no-such-file >_nofile.tmp 2>&1
$	if .not. $status then call exit_code '$status' _nofile.tmp
$	set On
$!	restore altered file name
$	gawk "{gsub(""no-such-file"", ""no/such/file""); print}" _nofile.tmp >_nofile.too
$	rm _nofile.tmp;
$	mv _nofile.too _nofile.tmp
$!	nofile.ok expects "No such file ...", we see "no such file ..."
$	igncascmp nofile.ok sys$disk:[]_nofile.tmp
$	if $status then  rm _nofile.tmp;
$	return
$
$binmode1:	echo "binmode1"
$	gawk -v "BINMODE=3" "BEGIN { print BINMODE }" >_binmode1.tmp
$	cmp binmode1.ok sys$disk:[]_binmode1.tmp
$	if $status then  rm _binmode1.tmp;
$	return
$
$subi18n:	echo "''test'"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f 'test'.awk > _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$rri1:
$concat4:	echo "''test'"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f 'test'.awk 'test'.in > _'test'.tmp
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$devfd:		echo "devfd: not supported"
$	return
$!!devfd:	echo "devfd"
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in4 5< devfd.in5 >_devfd.tmp
$	cmp devfd.ok sys$disk:[]_devfd.tmp
$	if $status then  rm _devfd.tmp;
$	return
$
$devfd1:	echo "devfd1: not supported"
$	return
$!!devfd1:	echo "devfd1"
$	gawk -f devfd1.awk 4< devfd.in1 5< devfd.in2 >_devfd1.tmp
$	cmp devfd1.ok sys$disk:[]_devfd1.tmp
$	if $status then  rm _devfd1.tmp;
$	return
$
$devfd2:	echo "devfd2: not supported"
$	return
$!!devfd2:	echo "devfd2"
$! The program text is the '1' which will print each record. How compact can you get?
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in1 5< devfd.in2 >_devfd2.tmp
$	cmp devfd2.ok sys$disk:[]_devfd2.tmp
$	if $status then  rm _devfd2.tmp;
$	return
$
$charasbytes:
$! This test used "od" on Unix to verify the result. As this is not available
$! we must try as best as possible using DUMP and SEARCH, instead of comparing
$! to charasbytes.ok
$!
$	echo "''test'"
$	gawk -b -f 'test'.awk 'test'.in >_'test'.tmp
$	pipe dump/byte/block=count:1 _charasbytes.tmp | -
		search sys$pipe /noout " 00 00 00 00 00 00 00 00 00 00 00 00 0A 5A 5A 5A"
$	if $severity .eq. 1 then	rm _'test'.tmp;*
$	return
$
$mixed1:	echo "mixed1"
$	set noOn
$	gawk -f /dev/null --source "BEGIN {return junk}" >_mixed1.tmp 2>&1
$	if .not. $status then call exit_code '$status' _mixed1.tmp
$	set On
$	cmp mixed1.ok sys$disk:[]_mixed1.tmp
$	if $status then  rm _mixed1.tmp;
$	return
$
$mtchi18n:	echo "mtchi18n"
$	define/User GAWKLOCALE "ru_RU.UTF-8"
$	gawk -f mtchi18n.awk mtchi18n.in >_mtchi18n.tmp
$	cmp mtchi18n.ok sys$disk:[]_mtchi18n.tmp
$	if $status then  rm _mtchi18n.tmp;
$	return
$
$reint2:	echo "reint2"
$	gosub define_gawklocale
$	gawk --re-interval -f reint2.awk reint2.in >_reint2.tmp
$	cmp reint2.ok sys$disk:[]_reint2.tmp
$	if $status then  rm _reint2.tmp;
$	return
$
$localenl:	echo "localenl skipped"
$	return
$!!localenl:	echo "localenl"
$	@localenl.com /Output=_localenl.tmp	! sh ./localenl.sh >tmp.
$	cmp localenl.ok sys$disk:[]_localenl.tmp
$	if $status then  rm _localenl.tmp;
$	return
$
$mbprintf1:	echo "mbprintf1 skipped"
$	return
$!!mbprintf1:	echo "mbprintf1"
$! Makefile test exports this, but we don't want to impact user's environment
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbprintf1.awk mbprintf1.in >_mbprintf1.tmp
$	cmp mbprintf1.ok sys$disk:[]_mbprintf1.tmp
$	if $status then  rm _mbprintf1.tmp;
$	return
$
$mbprintf2:	echo "mbprintf2"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "ja_JP.UTF-8"
$	gawk -f mbprintf2.awk >_mbprintf2.tmp
$	cmp mbprintf2.ok sys$disk:[]_mbprintf2.tmp
$	if $status then  rm _mbprintf2.tmp;
$	return
$
$mbprintf3:	echo "mbprintf3"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbprintf3.awk mbprintf3.in >_mbprintf2.tmp
$	cmp mbprintf3.ok sys$disk:[]_mbprintf2.tmp
$	if $status then  rm _mbprintf2.tmp;
$	return
$
$mbfw1:		echo "mbfw1 skipped"
$	return
$!!mbfw1:		echo "mbfw1"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbfw1.awk mbfw1.in >_mbfw1.tmp
$	cmp mbfw1.ok sys$disk:[]_mbfw1.tmp
$	if $status then  rm _mbfw1.tmp;
$	return
$
$gsubtst6:	echo "gsubtst6"
$	define/User GAWKLOCALE "C"
$	gawk -f gsubtst6.awk >_gsubtst6.tmp
$	cmp gsubtst6.ok sys$disk:[]_gsubtst6.tmp
$	if $status then  rm _gsubtst6.tmp;
$	return
$
$mbstr1:	echo "mbstr1"
$	gosub define_gawklocale
$	AWKPATH_srcdir
$	gawk -f mbstr1.awk >_mbstr1.tmp
$	cmp mbstr1.ok sys$disk:[]_mbstr1.tmp
$	if $status then  rm _mbstr1.tmp;
$	return
$
$printfbad2:
$printfbad3:
$	echo "''test'"
$	set noOn
$	gawk --lint -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then	 rm _'test'.tmp;
$	return
$
$fmtspcl:	echo "fmtspcl: not supported"
$	return
$!!fmtspcl:
$! fmtspcl ought to work if gawk was compiled to use IEEE floating point
$	if floatmode.lt.0 then  gosub calc_floatmode
$	if floatmode.lt.2
$	then	echo "fmtspcl: not supported"
$	else	echo "fmtspcl"
$		gawk -f fmtspcl.awk >_fmtspcl.tmp 2>&1
$		cmp fmtspcl.ok sys$disk:[]_fmtspcl.tmp
$		if $status then  rm _fmtspcl.tmp;
$	endif
$	return
$
$intformat:	echo "intformat"
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
$	if $status then  rm _intformat.tmp;
$	return
$
$! ugh... BEGINFILE functionality works fine, but test is heavily Unix-centric
$beginfile1:	echo "beginfile1"
$	! complications:  "." is a filename, not the current directory
$	!  (even "[]" is actually "[].", that same filename, but we can
$	!  handle hacking it more easily);
$	!  "no/such/file" yields syntax error rather than no such file
$	!  when subdirectories no/ and no/such/ don't exist;
$	!  vms test suite doesn't generate Makefile;
$	!  "is a directory" and "no such file" aren't capitalized
$	! gawk -f beginfile1.awk beginfile1.awk . ./no/such/file "Makefile" >_beginfile1.tmp 2>&1
$	gawk -f beginfile1.awk beginfile1.awk [] ./no-such-file "Makefile.in" >_beginfile1.tmp 2>&1
$	gawk -f - _beginfile1.tmp >_beginfile1.too
{ if (gsub("\\[\\]",".")) gsub("no such file or directory","is a directory")
  gsub("no-such-file","file"); gsub("Makefile.in","Makefile"); print }
$	rm _beginfile1.tmp;
$	mv _beginfile1.too _beginfile1.tmp
$	igncascmp beginfile1.ok sys$disk:[]_beginfile1.tmp
$	if $status then  rm _beginfile1.tmp;
$	return
$
$dumpvars: echo "dumpvars"
$	gawk --dump-variables 1 <dumpvars.in >_NL: 2>&1
$	mv awkvars.out _dumpvars.tmp
$	cmp dumpvars.ok sys$disk:[]_dumpvars.tmp
$	if $status then  rm _dumpvars.tmp;
$	return
$
$profile1: echo "''test'"
$ ! FIXME: for both gawk invocations which pipe output to SORT,
$ ! two output files get created; the top version has real output
$ ! but there's also an empty lower version.
$		oldout = f$search("_''test'.tmp1")
$	gawk --pretty-print -v "sortcmd=SORT sys$intput: sys$output:" -
			-f xref.awk dtdgport.awk > _'test'.tmp1
$		badout = f$search("_''test'.tmp1;-1")
$		if badout.nes."" .and. badout.nes.oldout then  rm 'badout'
$		oldout = f$search("_''test'.tmp2")
$	gawk -v "sortcmd=SORT sys$intput: sys$output:" -
			-f awkprof.out dtdgport.awk > _'test'.tmp2
$		badout = f$search("_''test'.tmp2;-1")
$		if badout.nes."" .and. badout.nes.oldout then  rm 'badout'
$	cmp _'test'.tmp1 sys$disk:[]_'test'.tmp2
$	if $status then  rm _'test'.tmp%;,awkprof.out;
$	return
$
$profile2:  echo "''test'"
$	gawk --profile -v "sortcmd=SORT sys$input: sys$output:" -
			-f xref.awk dtdgport.awk > _NL:
$	! sed <awkprof.out 1,2d >_profile2.tmp
$	sumslp awkprof.out /update=sys$input: /output=_'test'.tmp
-1,2
/
$	rm awkprof.out;
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;*
$	return
$
$profile3: echo "''test'"
$	gawk --profile -f 'test'.awk > _NL:
$	! sed <awkprof.out 1,2d >_profile3.tmp
$	sumslp awkprof.out /update=sys$input: /output=_'test'.tmp
-1,2
/
$	rm awkprof.out;
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then  rm _'test'.tmp;*
$	return
$
$next:	echo "next"
$	set noOn
$	gawk "{next}" _NL:                                   > _next.tmp 2>&1
$	gawk "function f() {next}; {f()}" _NL:               >>_next.tmp 2>&1
$	gawk "function f() {next}; BEGIN{f()}" _NL:          >>_next.tmp 2>&1
$	gawk "function f() {next}; {f()}; END{f()}" _NL:     >>_next.tmp 2>&1
$	gawk "function f() {next}; BEGINFILE{f()}" _NL:      >>_next.tmp 2>&1
$	gawk "function f() {next}; {f()}; ENDFILE{f()}" _NL: >>_next.tmp 2>&1
$	set On
$	cmp next.ok sys$disk:[]_next.tmp
$	if $status then  rm _next.tmp;
$	return
$
$exit:	echo "exit"
$	if .not.pipeok
$	then	echo "Without the PIPE command, ''test' can't be run."
$		On warning then  return
$		pipe echo "PIPE command is available; running exit test"
$		On warning then  $
$		pipeok = 1
$	else	echo "PIPE command is available; running exit test"
$	endif
$	set noOn
$	call/Output=_exit.tmp do__exit
$	set On
$	cmp exit.ok sys$disk:[]_exit.tmp
$	if $status then  rm _exit.tmp;
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
$	if f$search("vms_cmd.ok").eqs.""
$	then create vms_cmd.ok
World!
$	endif
$	gawk /Commands="BEGIN { print ""World!"" }" _NL: /Output=_vms_cmd.tmp
$	cmp vms_cmd.ok sys$disk:[]_vms_cmd.tmp
$	if $status then  rm _vms_cmd.tmp;,vms_cmd.ok;*
$	return
$
$vms_io1:	echo "vms_io1"
$	if f$search("vms_io1.ok").eqs.""
$	then create vms_io1.ok
Hello
$	endif
$ !	define/User dbg$input sys$command:
$	gawk -f - >_vms_io1.tmp
# prior to 3.0.4, gawk crashed doing any redirection after closing stdin
BEGIN { print "Hello" >"/dev/stdout" }
$	cmp vms_io1.ok sys$disk:[]_vms_io1.tmp
$	if $status then  rm _vms_io1.tmp;,vms_io1.ok;*
$	return
$
$vms_io2:	echo "vms_io2"
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
$	if $status then  rm _vms_io2.vfc;*,vms_io2.ok;*
$	return
$!
$!
$inplace1:
$	set process/parse=extended ! ODS-5 only
$	echo "''test'"
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
$	if $status then rm _'test'.2.tmp;,_'test'.2;,_'test'.awk;
$	return
$!
$filefuncs:
$fnmatch:
$functab4:
$ordchr:
$revout:
$revtwoway:
$time:
$	echo "''test'"
$	filefunc_file = "[-]gawkapi.o"
$	open/write gapi 'filefunc_file'
$	close gapi
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not. $status then call exit_code '$status' _'test'.tmp
$	set On
$	cmp 'test'.ok sys$disk:[]_'test'.tmp
$	if $status then rm _'test'.tmp;
$	if f$search(filefunc_file) .nes. "" then rm 'filefunc_file';*
$	return
$!
$rwarray:
$	echo "''test'"
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
$	if $status then rm _'test'.tmp;,orig.bin;,orig.out;,new.out;
$	return
$!
$readdir:
$fts:
$	echo "''test'"
$	set noOn
$	AWKLIBPATH_dir
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not. $status
$	then
$	    call exit_code '$status' _'test'.tmp
$	    write sys$output _'test'.tmp
$	else
$	    if f$search("_''test'.tmp") .nes. "" then rm _'test'.tmp;*
$	    if f$search("_''test'.") .nes. "" then rm _'test'.;*
$	endif
$	set On
$	return
$
$clean:
$	if f$search("_*.*")	 .nes."" then  rm _*.tmp;*
$	if f$search("_*.too")	 .nes."" then  rm _*.too;*
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
$	! gawk uses the C run-time libary's getenv() function to look up
$	! GAWKLOCALE, so a symbol provides another way to supply the value;
$	! we don't want to override logical or symbol if either is present
$	if f$trnlnm("GAWKLOCALE").eqs.""
$	then
$	    if f$type(gawklocale).nes."STRING" .or. "''gawklocale'".eqs.""
$	    then
$		define/User GAWKLOCALE "en_US.UTF-8"
$	    endif
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
