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
$	echo	= "write sys$output"
$	cmp	= "diff/Output=_NL:/Maximum=1"
$	igncascmp = "''cmp'/Ignore=Case"
$	rm	= "delete/noConfirm/noLog"
$	mv	= "rename/New_Vers"
$	gawk = "$sys$disk:[-]gawk"
$	AWKPATH_srcdir = "define/User AWKPATH sys$disk:[]"
$
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
$		list = "basic unix_tests gawk_ext vms_tests charset_tests machine_tests"
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
		  + " delarprm delarpm2 delfunc dynlj eofsplit exitval1" -
		  + " exitval2 fcall_exit fcall_exit2 fldchg fldchgnf" -
		  + " fnamedat fnarray fnarray2 fnaryscl fnasgnm fnmisc" -
		  + " fordel forref forsimp fsbs fsspcoln fsrs fstabplus" -
		  + " funsemnl funsmnam funstack getline getline2 getline3"
$		gosub list_of_tests
$		list = "getlnbuf getnr2tb getnr2tm gsubasgn gsubtest" -
		  + " gsubtst2 gsubtst3 gsubtst4 gsubtst5 gsubtst6 hex" -
		  + " hsprint inputred intest intprec iobug1" -
		  + " leaddig leadnl litoct longsub longwrds"-
		  + " manglprm math membug1 messages minusstr mmap8k" -
		  + " mtchi18n nasty nasty2 negexp nested nfldstr nfneg"
$		gosub list_of_tests
$		list = "nfset nlfldsep nlinstr nlstrina noeffect nofile" -
		  + " nofmtch noloop1 noloop2 nonl noparms nors nulrsend" -
		  + " numindex numsubstr octsub ofmt ofmtbig ofmtfidl" -
		  + " ofmts onlynl opasnidx opasnslf paramdup" -
		  + " paramres paramtyp parse1 parsefld parseme pcntplus" -
		  + " prdupval prec printf0 printf1 prmarscl prmreuse"
$		gosub list_of_tests
$		list = "prt1eval prtoeval psx96sub rand rebt8b1" -
		  + " redfilnm regeq reindops reparse resplit rs rsnul1nl" -
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
		  + " poundbang space strftlng"
$		gosub list_of_tests
$		return
$
$gnu:
$gawk_ext:	echo "gawk_ext... (gawk.extensions)"
$		list = "argtest backw badargs binmode1" -
		  + " clos1way devfd devfd1 devfd2" -
		  + " fieldwdth funlen fsfwfs fwtest fwtest2 gensub" -
		  + " gensub2 getlndir gnuops2 gnuops3 gnureops icasefs" -
		  + " icasers igncdym igncfs ignrcase ignrcas2 lint"
$		gosub list_of_tests
$		list = "lintold match1 match2 match3 manyfiles" -
		  + " mbprintf3 mbstr1" -
		  + " nondec nondec2 posix procinfs printfbad1" -
		  + " printfbad2 regx8bit rebuf reint reint2 rsstart1" -
		  + " rsstart2 rsstart3 rstest6 shadow strtonum strftime"
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
		  + " mbprintf1 mbprintf2 rebt8b2 sort1 sprintfc whiny"
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
$fsfwfs:
$fsrs:
$funlen:
$funstack:
$fwtest:
$fwtest2:
$gensub:
$getline3:
$getnr2tb:
$getnr2tm:
$gsubtest:
$gsubtst2:
$gsubtst4:
$gsubtst5:
$hex:
$icasers:
$igncfs:
$igncdym:
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
$onlynl:
$parse1:
$parsefld:
$prdupval:
$prec:
$prtoeval:
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
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$! more common tests, without a data file: gawk -f 'test'.awk
$arrayref:
$arrymem1:
$arynasty:
$arysubnm:
$asort:
$asorti:
$closebad:
$compare2:
$convfmt:
$delarprm:
$!!double1:
$!!double2:
$dynlj:
$fnarydel:
$fnparydl:
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
$nlstrina:
$nondec:
$octsub:
$paramtyp:
$pcntplus:
$printf1:
$procinfs:
$prt1eval:
$psx96sub:
$rebt8b1:
$rebt8b2:
$regx8bit:
$sort1:
$splitdef:
$splitwht:
$strnum1:
$strtonum:
$substr:
$zero2:
$zeroflag:
$! common without 'test'.in
$	echo "''test'"
$	gawk -f 'test'.awk >_'test'.tmp
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$double1:
$double2:
$lc_num1:
$mbprintf1:
$	echo "''test' skipped"
$	return
$
$msg:
$	echo "Any output from ""DIF"" is bad news, although some differences"
$	echo "in floating point values are probably benign -- in particular,"
$	echo "some systems may omit a leading zero and the floating point"
$	echo "precision may lead to slightly different output in a few cases."
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
$	echo "poundbang: not supported"
$	return
$
$messages:	echo "messages"
$	set noOn
$	gawk -f messages.awk > out2 >& out3
$	cmp out1.ok out1.
$	if $status then  rm out1.;
$	cmp out2.ok out2.
$	if $status then  rm out2.;
$	cmp out3.ok out3.
$	if $status then  rm out3.;
$	set On
$	return
$
$argarray:	echo "argarray"
$	define/User TEST "test"			!this is useless...
$	gawk -f argarray.awk ./argarray.in - >_argarray.tmp
just a test
$	cmp argarray.ok _argarray.tmp
$	if $status then  rm _argarray.tmp;
$	return
$
$fstabplus:	echo "fstabplus"
$	gawk -f fstabplus.awk >_fstabplus.tmp
1		2
$	cmp fstabplus.ok _fstabplus.tmp
$	if $status then  rm _fstabplus.tmp;
$	return
$
$longwrds:	echo "longwrds"
$	gawk -v "SORT=sort sys$input: _longwrds.tmp" -f longwrds.awk longwrds.in >_NL:
$	cmp longwrds.ok _longwrds.tmp
$	if $status then  rm _longwrds.tmp;
$	return
$
$fieldwdth:	echo "fieldwdth"
$	gawk -v "FIELDWIDTHS=2 3 4" "{ print $2}" >_fieldwdth.tmp
123456789
$	cmp fieldwdth.ok _fieldwdth.tmp
$	if $status then  rm _fieldwdth.tmp;
$	return
$
$ignrcase:	echo "ignrcase"
$	gawk -v "IGNORECASE=1" "{ sub(/y/, """"); print}" >_ignrcase.tmp
xYz
$	cmp ignrcase.ok _ignrcase.tmp
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
$	cmp posix.ok _posix.tmp
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
$	cmp compare.ok _compare.tmp
$	if $status then  rm _compare.tmp;
$	return
$
$rs:		echo "rs"
$	gawk -v "RS=" "{ print $1, $2}" rs.in >_rs.tmp
$	cmp rs.ok _rs.tmp
$	if $status then  rm _rs.tmp;
$	return
$
$fsbs:		echo "fsbs"
$	gawk -v "FS=\" "{ print $1, $2 }" fsbs.in >_fsbs.tmp
$	cmp fsbs.ok _fsbs.tmp
$	if $status then  rm _fsbs.tmp;
$	return
$
$inftest:	echo "inftest"
$     !!  echo "This test is very machine specific..."
$	set noOn
$	gawk -f inftest.awk >_inftest.tmp
$     !!  cmp inftest.ok _inftest.tmp	!just care that gawk doesn't crash...
$	if $status then  rm _inftest.tmp;
$	set On
$	return
$
$getline2:	echo "getline2"
$	gawk -f getline2.awk getline2.awk getline2.awk >_getline2.tmp
$	cmp getline2.ok _getline2.tmp
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
$	cmp negexp.ok _negexp.tmp
$	if $status then  rm _negexp.tmp;
$	return
$
$awkpath:	echo "awkpath"
$	define/User AWK_LIBRARY [],[.lib]
$	gawk -f awkpath.awk >_awkpath.tmp
$	cmp awkpath.ok _awkpath.tmp
$	if $status then  rm _awkpath.tmp;
$	return
$
$argtest:	echo "argtest"
$	gawk -f argtest.awk -x -y abc >_argtest.tmp
$	cmp argtest.ok _argtest.tmp
$	if $status then  rm _argtest.tmp;
$	return
$
$badargs:	echo "badargs"
$	on error then continue
$	gawk -f 2>&1 >_badargs.too
$!	search/Match=Nor _badargs.too "patchlevel" /Output=_badargs.tmp
$	gawk "/patchlevel/{next}; {gsub(""\"""",""'""); print}" <_badargs.too >_badargs.tmp
$	cmp badargs.ok _badargs.tmp
$	if $status then  rm _badargs.tmp;,_badargs.too;
$	return
$
$paramdup:	echo "paramdup"
$	set noOn
$	gawk -f paramdup.awk >_paramdup.tmp 2>&1
$	if .not.$status then call exit_code 1 _paramdup.tmp
$	set On
$	cmp paramdup.ok _paramdup.tmp
$	if $status then  rm _paramdup.tmp;
$	return
$
$nonl:		echo "nonl"
$	! This one might fail, depending on the tool used to unpack the
$	! distribution.  Some will add a final newline if the file lacks one.
$	AWKPATH_srcdir
$	gawk --lint -f nonl.awk _NL: >_nonl.tmp 2>&1
$	cmp nonl.ok _nonl.tmp
$	if $status then  rm _nonl.tmp;
$	return
$
$defref:	echo "defref"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f defref.awk >_defref.tmp 2>&1
$	if .not.$status then call exit_code 2 _defref.tmp
$	set On
$	cmp defref.ok _defref.tmp
$	if $status then  rm _defref.tmp;
$	return
$
$nofmtch:	echo "nofmtch"
$	AWKPATH_srcdir
$	gawk --lint -f nofmtch.awk >_nofmtch.tmp 2>&1
$	cmp nofmtch.ok _nofmtch.tmp
$	if $status then  rm _nofmtch.tmp;
$	return
$
$strftime:	echo "strftime"
$	! this test could fail on slow machines or on a second boundary,
$	! so if it does, double check the actual results
$!!	date | gawk -v "OUTPUT"=_strftime.tmp -f strftime.awk
$	now = f$time()
$	wkd = f$extract(0,3,f$cvtime(now,,"WEEKDAY"))
$	mon = f$cvtime(now,"ABSOLUTE","MONTH")
$	mon = f$extract(0,1,mon) + f$edit(f$extract(1,2,mon),"LOWERCASE")
$	day = f$cvtime(now,,"DAY")
$	tim = f$extract(0,8,f$cvtime(now,,"TIME"))
$	tz  = ""
$	yr  = f$cvtime(now,,"YEAR")
$	if f$trnlnm("FTMP").nes."" then  close/noLog ftmp
$	open/Write ftmp strftime.in
$	write ftmp wkd," ",mon," ",day," ",tim," ",tz," ",yr
$	close ftmp
$	gawk -v "OUTPUT"=_strftime.tmp -f strftime.awk strftime.in
$	set noOn
$	cmp strftime.ok _strftime.tmp
$	if $status then  rm _strftime.tmp;,strftime.ok;*,strftime.in;*
$	set On
$	return
$
$litoct:	echo "litoct"
$	gawk --traditional -f litoct.awk >_litoct.tmp
ab
$	cmp litoct.ok _litoct.tmp
$	if $status then  rm _litoct.tmp;
$	return
$
$resplit:	echo "resplit"
$	gawk -- "{ FS = "":""; $0 = $0; print $2 }" >_resplit.tmp
a:b:c d:e:f
$	cmp resplit.ok _resplit.tmp
$	if $status then  rm _resplit.tmp;
$	return
$
$intprec:	echo "intprec"
$	gawk -f intprec.awk >_intprec.tmp 2>&1
$	cmp intprec.ok _intprec.tmp
$	if $status then  rm _intprec.tmp;
$	return
$
$childin:	echo "childin skipped"
$	return
$! note: this `childin' test currently [gawk 3.0.3] fails for vms
$!!childin:	echo "childin"
$	echo "note: type ``hi<return><ctrl/Z>'",-
	     "' if testing appears to hang in `childin'"
$!!	@echo hi | gawk "BEGIN { ""cat"" | getline; print; close(""cat"") }" >_childin.tmp
$	gawk "BEGIN { ""type sys$input:"" | getline; print; close(""type sys$input:"") }" >_childin.tmp
hi
$	cmp childin.ok _childin.tmp
$	if $status then  rm _childin.tmp;
$	return
$
$noeffect:	echo "noeffect"
$	AWKPATH_srcdir
$	gawk --lint -f noeffect.awk >_noeffect.tmp 2>&1
$	cmp noeffect.ok _noeffect.tmp
$	if $status then  rm _noeffect.tmp;
$	return
$
$numsubstr:	echo "numsubstr"
$	AWKPATH_srcdir
$	gawk -f numsubstr.awk numsubstr.in >_numsubstr.tmp
$	cmp numsubstr.ok _numsubstr.tmp
$	if $status then  rm _numsubstr.tmp;
$	return
$
$prmreuse:	echo "prmreuse"
$	if f$search("prmreuse.ok").eqs."" then  create prmreuse.ok
$	gawk -f prmreuse.awk >_prmreuse.tmp
$	cmp prmreuse.ok _prmreuse.tmp
$	if $status then  rm _prmreuse.tmp;
$	return
$
$fflush:
$	echo "fflush: not supported"
$	return
$!!fflush:	echo "fflush"
$	! hopelessly Unix-specific
$!!	@fflush.sh >_fflush.tmp
$	cmp fflush.ok _fflush.tmp
$	if $status then  rm _fflush.tmp;
$	return
$
$getlnhd:
$	echo "getlnhd skipped"
$	return
$!!getlnhd:	echo "getlnhd"
$	gawk -f getlnhd.awk >_getlnhd.tmp
$	cmp getlnhd.ok _getlnhd.tmp
$	if $status then  rm _getlnhd.tmp;
$	return
$
$tweakfld:	echo "tweakfld"
$	gawk -f tweakfld.awk tweakfld.in >_tweakfld.tmp
$	if f$search("errors.cleanup").nes."" then  rm errors.cleanup;*
$	cmp tweakfld.ok _tweakfld.tmp
$	if $status then  rm _tweakfld.tmp;
$	return
$
$clsflnam:	echo "clsflnam"
$	if f$search("clsflnam.ok").eqs."" then  create clsflnam.ok
$	gawk -f clsflnam.awk clsflnam.in >_clsflnam.tmp 2>&1
$	cmp clsflnam.ok _clsflnam.tmp
$	if $status then  rm _clsflnam.tmp;
$	return
$
$mmap8k:	echo "mmap8k"
$	gawk "{ print }" mmap8k.in >_mmap8k.tmp
$	cmp mmap8k.in _mmap8k.tmp
$	if $status then  rm _mmap8k.tmp;
$	return
$
$eofsplit:	echo "eofsplit"
$	if f$search("eofsplit.ok").eqs."" then  create eofsplit.ok
$	gawk -f eofsplit.awk >_eofsplit.tmp
$	cmp eofsplit.ok _eofsplit.tmp
$	if $status then  rm _eofsplit.tmp;
$	return
$
$back89:		echo "back89"
$	gawk "/a\8b/" back89.in >_back89.tmp
$	cmp back89.ok _back89.tmp
$	if $status then  rm _back89.tmp;
$	return
$
$tradanch:	echo "tradanch"
$	if f$search("tradanch.ok").eqs."" then  create tradanch.ok
$	gawk --traditional -f tradanch.awk tradanch.in >_tradanch.tmp
$	cmp tradanch.ok _tradanch.tmp
$	if $status then  rm _tradanch.tmp;
$	return
$
$pid:		echo "pid"
$	pid = f$integer("%x" + f$getjpi("","PID"))
$	ppid = f$integer("%x" + f$getjpi("","OWNER"))
$	gawk -v "ok_pid=''pid'" -v "ok_ppid=''ppid'" -f pid.awk >_pid.tmp >& _NL:
$	cmp pid.ok _pid.tmp
$	if $status then  rm _pid.tmp;
$	return
$
$strftlng:	echo "strftlng"
$	define/User TZ "UTC"		!useless
$	gawk -f strftlng.awk >_strftlng.tmp
$	cmp strftlng.ok _strftlng.tmp
$	if $status then  rm _strftlng.tmp;
$	return
$
$nfldstr:	echo "nfldstr"
$	if f$search("nfldstr.ok").eqs."" then  create nfldstr.ok
$	gawk "$1 == 0 { print ""bug"" }" >_nfldstr.tmp

$	cmp nfldstr.ok _nfldstr.tmp
$	if $status then  rm _nfldstr.tmp;
$	return
$
$nors:		echo "nors"
$!! there's no straightforward way to supply non-terminated input on the fly
$!!	@echo A B C D E | tr -d '\12' | $(AWK) '{ print $$NF }' - $(srcdir)/nors.in > _$@
$!! so just read a line from sys$input instead
$	gawk "{ print $NF }" - nors.in >_nors.tmp
A B C D E
$	cmp nors.ok _nors.tmp
$	if $status then  rm _nors.tmp;
$	return
$
$reint:		echo "reint"
$	gawk --re-interval -f reint.awk reint.in >_reint.tmp
$	cmp reint.ok _reint.tmp
$	if $status then  rm _reint.tmp;
$	return
$
$noparms:	echo "noparms"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f noparms.awk >_noparms.tmp 2>&1
$	if .not.$status then call exit_code 1 _noparms.tmp
$	set On
$	cmp noparms.ok _noparms.tmp
$	if $status then  rm _noparms.tmp;
$	return
$
$pipeio1:	echo "pipeio1"
$	cat = "TYPE"	!close enough, as long as we avoid .LIS default suffix
$	define/User test1 []test1.
$	define/User test2 []test2.
$	gawk -f pipeio1.awk >_pipeio1.tmp
$	rm test1.;,test2.;
$	cmp pipeio1.ok _pipeio1.tmp
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
$	cmp pipeio2.ok _pipeio2.tmp
$	if $status then  rm _pipeio2.tmp;
$	return
$
$clobber:	echo "clobber"
$	gawk -f clobber.awk >_clobber.tmp
$	cmp clobber.ok seq.
$	if $status then  rm seq.;*
$	cmp clobber.ok _clobber.tmp
$	if $status then  rm _clobber.tmp;
$	return
$
$nasty:	echo "nasty"
$	set noOn
$	gawk -f nasty.awk >_nasty.tmp
$	call fixup_LRL nasty.ok
$	call fixup_LRL _nasty.tmp "purge"
$	cmp nasty.ok _nasty.tmp
$	if $status then  rm _nasty.tmp;
$	set On
$	return
$
$nasty2:	echo "nasty2"
$	set noOn
$	gawk -f nasty2.awk >_nasty2.tmp
$	call fixup_LRL nasty2.ok
$	call fixup_LRL _nasty2.tmp "purge"
$	cmp nasty2.ok _nasty2.tmp
$	if $status then  rm _nasty2.tmp;
$	set On
$	return
$
$arrayparm:
$fnarray:
$fnaryscl:
$funsmnam:
$match2:
$opasnslf:
$opasnidx:
$paramres:
$parseme:
$printfbad1:
$prmarscl:
$subslash:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not.$status then call exit_code 2 _'test'.tmp
$	set On
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$arynocls:	echo "arynocls"
$	gawk -v "INPUT"=arynocls.in -f arynocls.awk >_arynocls.tmp
$	cmp arynocls.ok _arynocls.tmp
$	if $status then  rm _arynocls.tmp;
$	return
$
$getlnbuf:	echo "getlnbuf"
$	gawk -f getlnbuf.awk getlnbuf.in >_getlnbuf.tmp
$	gawk -f gtlnbufv.awk getlnbuf.in >_getlnbuf.too
$	cmp getlnbuf.ok _getlnbuf.tmp
$	if $status then  rm _getlnbuf.tmp;
$	cmp getlnbuf.ok _getlnbuf.too
$	if $status then  rm _getlnbuf.too;
$	return
$
$lint:	echo "lint"
$	AWKPATH_srcdir
$	gawk -f lint.awk >_lint.tmp 2>&1
$	cmp lint.ok _lint.tmp
$	if $status then  rm _lint.tmp;
$	return
$
$lintold:	echo "lintold"
$	AWKPATH_srcdir
$	gawk -f lintold.awk --lint-old <lintold.in >_lintold.tmp 2>&1
$	cmp lintold.ok _lintold.tmp
$	if $status then  rm _lintold.tmp;
$	return
$
$ofmtbig:	echo "ofmtbig"
$	set noOn
$	gawk -f ofmtbig.awk ofmtbig.in >_ofmtbig.tmp 2>&1
$	set On
$	cmp ofmtbig.ok _ofmtbig.tmp
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
$	gawk -f - nl:
BEGIN { print "" |& "/inet/udp/0/127.0.0.1/13";
	"/inet/udp/0/127.0.0.1/13" |& getline; print $0}
$	set On
$	return
$
$inetdayt:	echo "inetdayt"
$	echo "this test is for bidirectional TCP transmission"
$	set noOn
$	gawk -f - nl:
BEGIN { print "" |& "/inet/tcp/0/127.0.0.1/13";
	"/inet/tcp/0/127.0.0.1/13" |& getline; print $0}
$	set On
$	return
$
$redfilnm:	echo "redfilnm"
$	gawk -f redfilnm.awk srcdir="." redfilnm.in >_redfilnm.tmp
$	cmp redfilnm.ok _redfilnm.tmp
$	if $status then  rm _redfilnm.tmp;
$	return
$
$leaddig:	echo "leaddig"
$	gawk -v "x=2E"  -f leaddig.awk >_leaddig.tmp
$	cmp leaddig.ok _leaddig.tmp
$	if $status then  rm _leaddig.tmp;
$	return
$
$clos1way:
$	echo "clos1way: not supported"
$	return
$!!clos1way:	echo "clos1way"
$	gawk -f clos1way.awk >_clos1way.tmp
$	cmp clos1way.ok _clos1way.tmp
$	if $status then  rm _clos1way.tmp;
$	return
$
$shadow:	echo "shadow"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f shadow.awk >_shadow.tmp 2>&1
$	set On
$	cmp shadow.ok _shadow.tmp
$	if $status then  rm _shadow.tmp;
$	return
$
$longsub:	echo "longsub"
$	set noOn
$	gawk -f longsub.awk longsub.in >_longsub.tmp
$!! the records here are too long for DIFF to handle
$!! so assume success as long as gawk doesn't crash
$!!	call fixup_LRL longsub.ok
$!!	call fixup_LRL _longsub.tmp "purge"
$!!	cmp longsub.ok _longsub.tmp
$	if $status then  rm _longsub.tmp;
$	set On
$	return
$
$arrayprm3:	echo "arrayprm3"
$	gawk -f arrayprm3.awk arrayprm3.in >_arrayprm3.tmp
$	cmp arrayprm3.ok _arrayprm3.tmp
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
$delfunc:
$nfneg:
$numindex:
$scalar:
$sclforin:
$sclifin:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not.$status then call exit_code 2 _'test'.tmp
$	set On
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$fnamedat:
$fnasgnm:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk <'test'.in >_'test'.tmp 2>&1
$	if .not.$status then call exit_code 2 _'test'.tmp
$	set On
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$exitval2:	echo "exitval2 skipped"
$	return
$!!exitval2:	echo "exitval2"
$	gawk -f exitval2.awk exitval2.in >_exitval2.tmp
$	cmp exitval2.ok _exitval2.tmp
$	if $status then  rm _exitval2.tmp;
$	return
$
$fcall_exit2:
$fnarray2:
$fnmisc:
$gsubasgn:
$unterm:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp 2>&1
$	if .not.$status then call exit_code 1 _'test'.tmp
$	set On
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$getline:	echo "getline skipped"
$	return
$!!getline:	echo "getline"
$	gawk -f getline.awk getline.in >_getline.tmp
$	cmp getline.ok _getline.tmp
$	if $status then  rm _getline.tmp;
$	return
$
$gsubtst3:	echo "gsubtst3"
$	gawk --re-interval -f gsubtst3.awk gsubtst3.in >_gsubtst3.tmp
$	cmp gsubtst3.ok _gsubtst3.tmp
$	if $status then  rm _gsubtst3.tmp;
$	return
$
$! FIXME: gawk generates an extra, empty output file while running this test...
$iobug1:	echo "iobug1"
$	cat = "TYPE sys$input:"
$	true = "exit 1"	!success
$	gawk -f iobug1.awk iobug1.in >_iobug1.tmp
$	cmp iobug1.ok _iobug1.tmp
$	if $status then  rm _iobug1.tmp;
$	return
$
$rstest4:	echo "rstest4 skipped"
$	return
$!!rstest4:	echo "rstest4"
$	gawk -f rstest4.awk rstest4.in >_rstest4.tmp
$	cmp rstest4.ok _rstest4.tmp
$	if $status then  rm _rstest4.tmp;
$	return
$
$rstest5:	echo "rstest5 skipped"
$	return
$!!rstest5:	echo "rstest5"
$	gawk -f rstest5.awk rstest5.in >_rstest5.tmp
$	cmp rstest5.ok _rstest5.tmp
$	if $status then  rm _rstest5.tmp;
$	return
$
$fcall_exit:
$synerr1:
$synerr2:
$	echo "''test'"
$	set noOn
$	gawk -f 'test'.awk >_'test'.tmp 2>&1
$	if .not.$status then call exit_code 1 _'test'.tmp
$	set On
$	cmp 'test'.ok _'test'.tmp
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
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$space:		echo "space"
$	set noOn
$	gawk -f " " space.awk >_space.tmp 2>&1
$	if .not.$status then call exit_code 2 _space.tmp
$	set On
$!	we get a different error from what space.ok expects
$	gawk "{gsub(""file specification syntax error"", ""no such file or directory""); print}" -
		_space.tmp >_space.too
$	rm _space.tmp;
$	mv _space.too _space.tmp
$	igncascmp space.ok _space.tmp
$	if $status then  rm _space.tmp;
$	return
$
$printf0:	echo "printf0"
$	gawk --posix -f printf0.awk >_printf0.tmp
$	cmp printf0.ok _printf0.tmp
$	if $status then  rm _printf0.tmp;
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
$	cmp rsnulbig.ok _rsnulbig.tmp
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
$	cmp rsnulbig2.ok _rsnulbig2.tmp
$	if $status then  rm _rsnulbig2.tmp;
$	return
$
$whiny:		echo "whiny"
$	! WHINY_USERS=1 $(AWK) -f $(srcdir)/$@.awk $(srcdir)/$@.in >_$@
$	Define/User WHINY_USERS 1
$	gawk -f whiny.awk whiny.in >_whiny.tmp
$	cmp whiny.ok _whiny.tmp
$	if $status then  rm _whiny.tmp;
$	return
$
$subamp:
$wideidx:
$widesub2:
$widesub3:
$	echo "''test'"
$	gosub define_gawklocale
$	gawk -f 'test'.awk 'test'.in >_'test'.tmp
$	cmp 'test'.ok _'test'.tmp
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
$	cmp 'test'.ok _'test'.tmp
$	if $status then  rm _'test'.tmp;
$	return
$
$! This test is somewhat suspect for vms due to exit code manipulation
$exitval1:	echo "exitval1"
$	gawk -f exitval1.awk >_exitval1.tmp 2>&1
$	if $status then  call exit_code 0 _exitval1.tmp
$	cmp exitval1.ok _exitval1.tmp
$	if $status then  rm _exitval1.tmp;
$	return
$
$fsspcoln:	echo "fsspcoln"
$	gawk -f fsspcoln.awk "FS=[ :]+" fsspcoln.in >_forspcoln.tmp
$	cmp fsspcoln.ok _forspcoln.tmp
$	if $status then  rm _forspcoln.tmp;
$	return
$
$getlndir:	echo "getlndir"
$	! assume we're running in the test subdirectory; we don't want to
$	! perform a messy conversion of [] into its file specification
$	gawk -v "SRCDIR=[-]test.dir" -f getlndir.awk >_getlndir.tmp
$!	getlndir.ok expects "Is a directory", we see "is a directory"
$	igncascmp getlndir.ok _getlndir.tmp
$	if $status then  rm _getlndir.tmp;
$	return
$
$rsstart2:	echo "rsstart2"
$	gawk -f rsstart2.awk rsstart1.in >_rsstart2.tmp
$	cmp rsstart2.ok _rsstart2.tmp
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
$	cmp rsstart3.ok _rsstart3.tmp
$	if $status then  rm _rsstart3.tmp;
$	return
$
$nondec2:	echo "nondec2"
$	gawk --non-decimal-data -v "a=0x1" -f nondec2.awk >_nondec2.tmp
$	cmp nondec2.ok _nondec2.tmp
$	if $status then  rm _nondec2.tmp;
$	return
$
$nofile:	echo "nofile"
$!	gawk "{}" no/such/file >_nofile.tmp 2>&1
$!	nofile.ok expects no/such/file, but using that name in the test would
$!	yield "file specification syntax error" instead of "no such file..."
$	set noOn
$	gawk "{}" no-such-file >_nofile.tmp 2>&1
$	if .not.$status then call exit_code 2 _nofile.tmp
$	set On
$!	restore altered file name
$	gawk "{gsub(""no-such-file"", ""no/such/file""); print}" _nofile.tmp >_nofile.too
$	rm _nofile.tmp;
$	mv _nofile.too _nofile.tmp
$!	nofile.ok expects "No such file ...", we see "no such file ..."
$	igncascmp nofile.ok _nofile.tmp
$	if $status then  rm _nofile.tmp;
$	return
$
$binmode1:	echo "binmode1"
$	gawk -v "BINMODE=3" "BEGIN { print BINMODE }" >_binmode1.tmp
$	cmp binmode1.ok _binmode1.tmp
$	if $status then  rm _binmode1.tmp;
$	return
$
$subi18n:	echo "subi18n"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f subi18n.awk >_subi18n.tmp
$	cmp subi18n.ok _subi18n.tmp
$	if $status then  rm _subi18n.tmp;
$	return
$
$concat4:	echo "concat4"
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f concat4.awk concat4.in >_concat4.tmp
$	cmp concat4.ok _concat4.tmp
$	if $status then  rm _concat4.tmp;
$	return
$
$devfd:		echo "devfd: not supported"
$	return
$!!devfd:	echo "devfd"
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in4 5< devfd.in5 >_devfd.tmp
$	cmp devfd.ok _devfd.tmp
$	if $status then  rm _devfd.tmp;
$	return
$
$devfd1:	echo "devfd1: not supported"
$	return
$!!devfd1:	echo "devfd1"
$	gawk -f devfd1.awk 4< devfd.in1 5< devfd.in2 >_devfd1.tmp
$	cmp devfd1.ok _devfd1.tmp
$	if $status then  rm _devfd1.tmp;
$	return
$
$devfd2:	echo "devfd2: not supported"
$	return
$!!devfd2:	echo "devfd2"
$! The program text is the '1' which will print each record. How compact can you get?
$	gawk 1 /dev/fd/4 /dev/fd/5 4< /devfd.in1 5< devfd.in2 >_devfd2.tmp
$	cmp devfd2.ok _devfd2.tmp
$	if $status then  rm _devfd2.tmp;
$	return
$
$mixed1:	echo "mixed1"
$	set noOn
$	gawk -f /dev/null --source "BEGIN {return junk}" >_mixed1.tmp 2>&1
$	if .not.$status then call exit_code 1 _mixed1.tmp
$	set On
$	cmp mixed1.ok _mixed1.tmp
$	if $status then  rm _mixed1.tmp;
$	return
$
$mtchi18n:	echo "mtchi18n"
$	define/User GAWKLOCALE "ru_RU.UTF-8"
$	gawk -f mtchi18n.awk mtchi18n.in >_mtchi18n.tmp
$	cmp mtchi18n.ok _mtchi18n.tmp
$	if $status then  rm _mtchi18n.tmp;
$	return
$
$reint2:	echo "reint2"
$	gosub define_gawklocale
$	gawk --re-interval -f reint2.awk reint2.in >_reint2.tmp
$	cmp reint2.ok _reint2.tmp
$	if $status then  rm _reint2.tmp;
$	return
$
$localenl:	echo "localenl skipped"
$	return
$!!localenl:	echo "localenl"
$	@localenl.com /Output=_localenl.tmp	! sh ./localenl.sh >tmp.
$	cmp localenl.ok _localenl.tmp
$	if $status then  rm _localenl.tmp;
$	return
$
$mbprintf1:	echo "mbprintf1 skipped"
$	return
$!!mbprintf1:	echo "mbprintf1"
$! Makefile test exports this, but we don't want to impact user's environment
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbprintf1.awk mbprintf1.in >_mbprintf1.tmp
$	cmp mbprintf1.ok _mbprintf1.tmp
$	if $status then  rm _mbprintf1.tmp;
$	return
$
$mbprintf2:	echo "mbprintf2"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "ja_JP.UTF-8"
$	gawk -f mbprintf2.awk >_mbprintf2.tmp
$	cmp mbprintf2.ok _mbprintf2.tmp
$	if $status then  rm _mbprintf2.tmp;
$	return
$
$mbprintf3:	echo "mbprintf3"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbprintf3.awk mbprintf3.in >_mbprintf2.tmp
$	cmp mbprintf3.ok _mbprintf2.tmp
$	if $status then  rm _mbprintf2.tmp;
$	return
$
$mbfw1:		echo "mbfw1 skipped"
$	return
$!!mbfw1:		echo "mbfw1"
$! Makefile test exports this, ...
$	define/User GAWKLOCALE "en_US.UTF-8"
$	gawk -f mbfw1.awk mbfw1.in >_mbfw1.tmp
$	cmp mbfw1.ok _mbfw1.tmp
$	if $status then  rm _mbfw1.tmp;
$	return
$
$gsubtst6:	echo "gsubtst6"
$	define/User GAWKLOCALE "C"
$	gawk -f gsubtst6.awk >_gsubtst6.tmp
$	cmp gsubtst6.ok _gsubtst6.tmp
$	if $status then  rm _gsubtst6.tmp;
$	return
$
$mbstr1:	echo "mbstr1"
$	gosub define_gawklocale
$	AWKPATH_srcdir
$	gawk -f mbstr1.awk >_mbstr1.tmp
$	cmp mbstr1.ok _mbstr1.tmp
$	if $status then  rm _mbstr1.tmp;
$	return
$
$printfbad2:	echo "printfbad2"
$	set noOn
$	gawk --lint -f printfbad2.awk printfbad2.in >_printfbad2.tmp 2>&1
$	set On
$	cmp printfbad2.ok _printfbad2.tmp
$	if $status then  rm _printfbad2.tmp;
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
$		cmp fmtspcl.ok _fmtspcl.tmp
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
$	cmp intformat.ok _intformat.tmp
$	if $status then  rm _intformat.tmp;
$	return
$
$vms_cmd:	echo "vms_cmd"
$	if f$search("vms_cmd.ok").eqs.""
$	then create vms_cmd.ok
World!
$	endif
$	gawk /Commands="BEGIN { print ""World!"" }" _NL: /Output=_vms_cmd.tmp
$	cmp vms_cmd.ok _vms_cmd.tmp
$	if $status then  rm _vms_cmd.tmp;
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
$	cmp vms_io1.ok _vms_io1.tmp
$	if $status then  rm _vms_io1.tmp;
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
$	cmp _NL: _vms_io2.tmp
$	if $status then  rm _vms_io2.tmp;
$	cmp vms_io2.ok _vms_io2.vfc
$	if $status then  rm _vms_io2.vfc;*
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
$	define/user sys$error nl:
$	define/user sys$output nl:
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
$	else	cmp nl: nl:	!deassign/user sys${error,output}
$	endif
$ endsubroutine !fixup_LRL
$
$! add a fake "EXIT CODE" record to the end of the temporary output file
$! to simulate the ``|| echo EXIT CODE $$? >>_$@'' shell script usage
$exit_code: subroutine
$	if f$trnlnm("FTMP").nes."" then  close/noLog ftmp
$	open/Append ftmp 'p2'
$	write ftmp "EXIT CODE: ",p1
$	close ftmp
$ endsubroutine !exit_code
$
$!NOTREACHED
$ exit
