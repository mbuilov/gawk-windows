$! vmstest.com -- DCL script to perform test/Makefile actions for VMS
$!
$! Usage:
$!  $ set default [-.test]
$!  $ @[-.vms]vmstest.com bigtest
$! This assumes that newly built gawk.exe is in the next directory up.
$!
$	echo	= "write sys$output"
$	cmp	= "diff/Output=_NL:/Maximum=1"
$	rm	= "delete/noConfirm/noLog"
$	gawk = "$sys$disk:[-]gawk"
$	AWKPATH_srcdir = "define/User AWKPATH sys$disk:[]"
$
$	if p1.eqs."" then  p1 = "bigtest"
$	gosub 'p1'
$	if p2.nes."" then  gosub 'p2'
$	if p3.nes."" then  gosub 'p3'
$	if p4.nes."" then  gosub 'p4'
$	if p5.nes."" then  gosub 'p5'
$	if p6.nes."" then  gosub 'p6'
$	if p7.nes."" then  gosub 'p7'
$	if p8.nes."" then  gosub 'p8'
$	exit
$
$all:
$bigtest:	bigtest_list = "basic unix_tests gawk_ext vms_tests"
$		echo "bigtest"
$bigtest_loop:	bigtest_test = f$element(0," ",bigtest_list)
$		bigtest_list = bigtest_list - bigtest_test - " "
$		if bigtest_test.nes." " then  gosub 'bigtest_test'
$		if bigtest_list.nes.""	then  goto   bigtest_loop
$		return
$
$basic:		basic_lst1 = "msg swaplns messages argarray longwrds" -
		  + " getline2 fstabplus compare arrayref rs fsrs rand" -
		  + " fsbs negexp asgext anchgsub splitargv awkpath nfset" -
		  + " reparse convfmt arrayparm paramdup nonl defref" -
		  + " nofmtch litoct resplit rswhite prmarscl sclforin" -
		  + " sclifin intprec childin noeffect numsubstr pcntplus" -
		  + " prmreuse math fldchg fldchgnf reindops sprintfc" -
		  + " backgsub tweakfld clsflnam mmap8k fnarray dynlj" -
		  + " substr eofsplit prt1eval splitwht back89 tradanch"
$		basic_lst2 = "nlfldsep splitvar intest nfldstr nors" -
		  + " fnarydel noparms funstack clobber delarprm prdupval" -
		  + " nasty nasty2 zeroflag getnr2tm getnr2tb printf1" -
		  + " funsmnam fnamedat numindex subslash opasnslf" -
		  + " opasnidx arynocls getlnbuf arysubnm fnparydl" -
		  + " nlstrina octsub nlinstr ofmt hsprint ofmts parseme" -
		  + " splitdef fnaryscl fnasgnm ofmtbig paramtyp rsnul1nl" -
		  + " datanonl regeq redfilnm strtod leaddig arynasty" -
		  + " psx96sub addcomma"
$		basic_lst3 = "rebt8b1 rebt8b2 leadnl funsemnl ofmtfidl" -
		  + " onlynl arrymem1 compare2 minusstr membug1 forsimp" -
		  + " concat1 longsub"
$		echo "basic"
$basic_loop1:	basic_test = f$element(0," ",basic_lst1)
$		basic_lst1 = basic_lst1 - basic_test - " "
$		if basic_test.nes." " then  gosub 'basic_test'
$		if basic_lst1.nes.""  then  goto   basic_loop1
$basic_loop2:	basic_test = f$element(0," ",basic_lst2)
$		basic_lst2 = basic_lst2 - basic_test - " "
$		if basic_test.nes." " then  gosub 'basic_test'
$		if basic_lst2.nes.""  then  goto   basic_loop2
$basic_loop3:	basic_test = f$element(0," ",basic_lst3)
$		basic_lst3 = basic_lst3 - basic_test - " "
$		if basic_test.nes." " then  gosub 'basic_test'
$		if basic_lst3.nes.""  then  goto   basic_loop3
$		return
$
$unix_tests:	unix_tst_list = "fflush getlnhd pid pipeio1" -
		  + " pipeio2 poundbang strftlng"
$		echo "unix_tests"
$unix_tst_loop: unix_tst_test = f$element(0," ",unix_tst_list)
$		unix_tst_list = unix_tst_list - unix_tst_test - " "
$		if unix_tst_test.nes." " then  gosub 'unix_tst_test'
$		if unix_tst_list.nes.""  then  goto   unix_tst_loop
$		return
$
$gawk_ext:	gawk_ext_list = "argtest badargs clos1way fieldwdth" -
		  + " fsfwfs gensub gnuops2 gnureops igncdym igncfs" -
		  + " ignrcase lint manyfiles nondec posix procinfs" -
		  + " regx8bit reint shadow sort1 strftime"
$		echo "gawk_ext (gawk.extensions)"
$gawk_ext_loop: gawk_ext_test = f$element(0," ",gawk_ext_list)
$		gawk_ext_list = gawk_ext_list - gawk_ext_test - " "
$		if gawk_ext_test.nes." " then  gosub 'gawk_ext_test'
$		if gawk_ext_list.nes.""  then  goto   gawk_ext_loop
$		return
$
$vms_tests:	vms_tst_list = "vms_io1"
$		echo "vms_tests"
$vms_tst_loop: vms_tst_test = f$element(0," ",vms_tst_list)
$		vms_tst_list = vms_tst_list - vms_tst_test - " "
$		if vms_tst_test.nes." " then  gosub 'vms_tst_test'
$		if vms_tst_list.nes.""  then  goto   vms_tst_loop
$		return
$
$extra:		extra_list = "regtest inftest inet"
$		echo "extra"
$extra_loop: extra_test = f$element(0," ",extra_list)
$		extra_list = extra_list - extra_test - " "
$		if extra_test.nes." " then  gosub 'extra_test'
$		if extra_list.nes.""  then  goto   extra_loop
$		return
$
$inet:		inet_list = "inetechu inetecht inetdayu inetdayt"
$		echo "inet"
$		type sys$input:
 The inet tests only work if gawk has been built with tcp/ip socket
 support and your system supports the services "discard" at port 9
 and "daytimed" at port 13.
$inet_loop: inet_test = f$element(0," ",inet_list)
$		inet_list = inet_list - inet_test - " "
$		if inet_test.nes." " then  gosub 'inet_test'
$		if inet_list.nes.""  then  goto   inet_loop
$		return
$
$poundbang:
$	echo "poundbang:  useless for VMS, so skipped"
$	return
$
$msg:
$	echo "Any output from ""DIFF"" is bad news, although some differences"
$	echo "in floating point values are probably benign -- in particular,"
$	echo "some systems may omit a leading zero and the floating point"
$	echo "precision may lead to slightly different output in a few cases."
$	return
$
$swaplns:	echo "swaplns"
$	gawk -f swaplns.awk swaplns.in >tmp.
$	cmp swaplns.ok tmp.
$	if $status then  rm tmp.;
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
$	gawk -f argarray.awk ./argarray.in - >tmp.
just a test
$	cmp argarray.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fstabplus:	echo "fstabplus"
$	gawk -f fstabplus.awk >tmp.
1		2
$	cmp fstabplus.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fsrs:		echo "fsrs"
$	gawk -f fsrs.awk fsrs.in >tmp.
$	cmp fsrs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$igncfs:	echo "igncfs"
$	gawk -f igncfs.awk igncfs.in >tmp.
$	cmp igncfs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$longwrds:	echo "longwrds"
$	gawk -v "SORT=sort sys$input: tmp." -f longwrds.awk longwrds.in >_NL:
$	cmp longwrds.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fieldwdth:	echo "fieldwdth"
$	gawk -v "FIELDWIDTHS=2 3 4" "{ print $2}" >tmp.
123456789
$	cmp fieldwdth.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ignrcase:	echo "ignrcase"
$	gawk -v "IGNORECASE=1" "{ sub(/y/, """"); print}" >tmp.
xYz
$	cmp ignrcase.ok tmp.
$	if $status then  rm tmp.;
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
$	gawk -f posix.awk >tmp.
1:2,3 4
$	cmp posix.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$manyfiles:	echo "manyfiles"
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]").eqs."" then  create/Dir/Prot=(O:rwed) [.junk]
$	gawk "BEGIN { for (i = 1; i <= 300; i++) print i, i}" >tmp.
$	echo "This may take quite a while..."
$	echo ""
$	gawk -f manyfiles.awk tmp. tmp.
$	define/User sys$error _NL:
$	define/User sys$output tmp.too
$	search/Match=Nor/Output=_NL:/Log [.junk]*.* ""
$!/Log output: "%SEARCH-S-NOMATCH, <filename> - <#> records" plus 1 line summary
$	gawk "$4!=2{++count}; END{if(NR!=301||count!=1){print ""Failed!""}}" tmp.too
$	rm tmp.;,tmp.too;,[.junk]*.*;*,[]junk.dir;
$	return
$
$compare:	echo "compare"
$	gawk -f compare.awk 0 1 compare.in >tmp.
$	cmp compare.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arrayref:	echo "arrayref"
$	gawk -f arrayref.awk >tmp.
$	cmp arrayref.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rs:		echo "rs"
$	gawk -v "RS=" "{ print $1, $2}" rs.in >tmp.
$	cmp rs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fsbs:		echo "fsbs"
$	gawk -v "FS=\" "{ print $1, $2 }" fsbs.in >tmp.
$	cmp fsbs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$inftest:	echo "inftest"
$     !!  echo "This test is very machine specific..."
$	set noOn
$	gawk -f inftest.awk >tmp.
$     !!  cmp inftest.ok tmp.		!just care that gawk doesn't crash...
$	if $status then  rm tmp.;
$	set On
$	return
$
$getline2:	echo "getline2"
$	gawk -f getline2.awk getline2.awk getline2.awk >tmp.
$	cmp getline2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rand:		echo "rand"
$	echo "The following line should just be 19 random numbers between 1 and 100"
$	echo ""
$	gawk -f rand.awk
$	return
$
$negexp:	echo "negexp"
$	gawk "BEGIN { a = -2; print 10^a }" >tmp.
$	cmp negexp.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$asgext:	echo "asgext"
$	gawk -f asgext.awk asgext.in >tmp.
$	cmp asgext.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$anchgsub:	echo "anchgsub"
$	gawk -f anchgsub.awk anchgsub.in >tmp.
$	cmp anchgsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitargv:	echo "splitargv"
$	gawk -f splitargv.awk splitargv.in >tmp.
$	cmp splitargv.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$awkpath:	echo "awkpath"
$	define/User AWK_LIBRARY [],[.lib]
$	gawk -f awkpath.awk >tmp.
$	cmp awkpath.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nfset:		echo "nfset"
$	gawk -f nfset.awk nfset.in >tmp.
$	cmp nfset.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reparse:	echo "reparse"
$	gawk -f reparse.awk reparse.in >tmp.
$	cmp reparse.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$argtest:	echo "argtest"
$	gawk -f argtest.awk -x -y abc >tmp.
$	cmp argtest.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$badargs:	echo "badargs"
$	on error then continue
$	gawk -f 2>&1 >tmp.too
$!	search/Match=Nor tmp. "patchlevel" /Output=tmp.
$	gawk "/patchlevel/{next}; {gsub(""\"""",""'""); print}" <tmp.too >tmp.
$	cmp badargs.ok tmp.
$	if $status then  rm tmp.;,tmp.too;
$	return
$
$convfmt:	echo "convfmt"
$	gawk -f convfmt.awk >tmp.
$	cmp convfmt.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arrayparm:	echo "arrayparm"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f arrayparm.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp arrayparm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$paramdup:	echo "paramdup"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f paramdup.awk >tmp. 2>&1
$	if .not.$status then call exit_code 1
$	set On
$	cmp paramdup.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nonl:		echo "nonl"
$	! This one might fail, depending on the tool used to unpack the
$	! distribution.  Some will add a final newline if the file lacks one.
$	AWKPATH_srcdir
$	gawk --lint -f nonl.awk _NL: >tmp. 2>&1
$	cmp nonl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$defref:	echo "defref"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f defref.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp defref.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nofmtch:	echo "nofmtch"
$	AWKPATH_srcdir
$	gawk --lint -f nofmtch.awk >tmp. 2>&1
$	cmp nofmtch.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$strftime:	echo "strftime"
$	! this test could fail on slow machines or on a second boundary,
$	! so if it does, double check the actual results
$!!	date | gawk -v "OUTPUT"=tmp. -f strftime.awk
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
$	gawk -v "OUTPUT"=tmp. -f strftime.awk strftime.in
$	set noOn
$	cmp strftime.ok tmp.
$	if $status then  rm tmp.;,strftime.ok;*,strftime.in;*
$	set On
$	return
$
$litoct:	echo "litoct"
$	gawk --traditional -f litoct.awk >tmp.
ab
$	cmp litoct.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$gensub:	echo "gensub"
$	gawk -f gensub.awk gensub.in >tmp.
$	cmp gensub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$resplit:	echo "resplit"
$	gawk -- "{ FS = "":""; $0 = $0; print $2 }" >tmp.
a:b:c d:e:f
$	cmp resplit.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rswhite:	echo "rswhite"
$	gawk -f rswhite.awk rswhite.in >tmp.
$	cmp rswhite.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prmarscl:	echo "prmarscl"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f prmarscl.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp prmarscl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sclforin:	echo "sclforin"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f sclforin.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp sclforin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sclifin:	echo "sclifin"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f sclifin.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp sclifin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$intprec:	echo "intprec"
$	gawk -f intprec.awk >tmp. 2>&1
$	cmp intprec.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$childin:	echo "childin:  currently fails for the VMS port, so skipped"
$	return
$! note: this `childin' test currently [gawk 3.0.3] fails for vms
$!!childin:	echo "childin"
$	echo "note: type ``hi<return><ctrl/Z>'",-
	     "' if testing appears to hang in `childin'"
$!!	@echo hi | gawk "BEGIN { ""cat"" | getline; print; close(""cat"") }" >tmp.
$	gawk "BEGIN { ""type sys$input:"" | getline; print; close(""type sys$input:"") }" >tmp.
hi
$	cmp childin.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$noeffect:	echo "noeffect"
$	AWKPATH_srcdir
$	gawk --lint -f noeffect.awk >tmp. 2>&1
$	cmp noeffect.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$numsubstr:	echo "numsubstr"
$	AWKPATH_srcdir
$	gawk -f numsubstr.awk numsubstr.in >tmp.
$	cmp numsubstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$gnureops:	echo "gnureops"
$	gawk -f gnureops.awk >tmp.
$	cmp gnureops.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pcntplus:	echo "pcntplus"
$	gawk -f pcntplus.awk >tmp.
$	cmp pcntplus.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prmreuse:	echo "prmreuse"
$	if f$search("prmreuse.ok").eqs."" then  create prmreuse.ok
$	gawk -f prmreuse.awk >tmp.
$	cmp prmreuse.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$math:		echo "math"
$	gawk -f math.awk >tmp.
$	cmp math.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fflush:
$	echo "fflush:  hopeless for VMS, so skipped"
$	return
$!!fflush:	echo "fflush"
$	! hopelessly Unix-specific
$!!	@fflush.sh >tmp.
$	cmp fflush.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fldchg:	echo "fldchg"
$	gawk -f fldchg.awk fldchg.in >tmp.
$	cmp fldchg.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fldchgnf:	echo "fldchgnf"
$	gawk -f fldchgnf.awk fldchgnf.in >tmp.
$	cmp fldchgnf.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reindops:	echo "reindops"
$	gawk -f reindops.awk reindops.in >tmp.
$	cmp reindops.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sprintfc:	echo "sprintfc"
$	gawk -f sprintfc.awk sprintfc.in >tmp.
$	cmp sprintfc.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getlnhd:
$	echo "getlnhd:  uses Unix-specific command so won't work on VMS"
$	return
$!!getlnhd:	echo "getlnhd"
$	gawk -f getlnhd.awk >tmp.
$	cmp getlnhd.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$backgsub:	echo "backgsub"
$	gawk -f backgsub.awk backgsub.in >tmp.
$	cmp backgsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$tweakfld:	echo "tweakfld"
$	gawk -f tweakfld.awk tweakfld.in >tmp.
$	if f$search("errors.cleanup").nes."" then  rm errors.cleanup;*
$	cmp tweakfld.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clsflnam:	echo "clsflnam"
$	if f$search("clsflnam.ok").eqs."" then  create clsflnam.ok
$	gawk -f clsflnam.awk clsflnam.in >tmp. 2>&1
$	cmp clsflnam.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$mmap8k:	echo "mmap8k"
$	gawk "{ print }" mmap8k.in >tmp.
$	cmp mmap8k.in tmp.
$	if $status then  rm tmp.;
$	return
$
$fnarray:	echo "fnarray"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f fnarray.awk >tmp. 2>&1
$	if .not.$status then call exit_code 1
$	set On
$	cmp fnarray.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$dynlj:		echo "dynlj"
$	gawk -f dynlj.awk >tmp.
$	cmp dynlj.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$substr:	echo "substr"
$	gawk -f substr.awk >tmp.
$	cmp substr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$eofsplit:	echo "eofsplit"
$	if f$search("eofsplit.ok").eqs."" then  create eofsplit.ok
$	gawk -f eofsplit.awk >tmp.
$	cmp eofsplit.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prt1eval:	echo "prt1eval"
$	gawk -f prt1eval.awk >tmp.
$	cmp prt1eval.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitwht:	echo "splitwht"
$	gawk -f splitwht.awk >tmp.
$	cmp splitwht.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$back89:		echo "back89"
$	gawk "/a\8b/" back89.in >tmp.
$	cmp back89.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$tradanch:	echo "tradanch"
$	if f$search("tradanch.ok").eqs."" then  create tradanch.ok
$	gawk --traditional -f tradanch.awk tradanch.in >tmp.
$	cmp tradanch.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nlfldsep:	echo "nlfldsep"
$	gawk -f nlfldsep.awk nlfldsep.in >tmp.
$	cmp nlfldsep.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitvar:	echo "splitvar"
$	gawk -f splitvar.awk splitvar.in >tmp.
$	cmp splitvar.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$intest:	echo "intest"
$	gawk -f intest.awk >tmp.
$	cmp intest.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pid:		echo "pid"
$	if f$search("pid.ok").eqs."" then  create pid.ok
$	if f$trnlnm("FTMP").nes."" then  close/noLog ftmp
$	open/Write ftmp _pid.in
$	write ftmp f$integer("%x" + f$getjpi("","PID"))
$	write ftmp f$integer("%x" + f$getjpi("","OWNER"))
$	close ftmp
$	gawk -f pid.awk _pid.in >tmp. >& _NL:
$	rm _pid.in;
$	cmp pid.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$strftlng:	echo "strftlng"
$	define/User TZ "UTC"		!useless
$	gawk -f strftlng.awk >tmp.
$	cmp strftlng.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nfldstr:	echo "nfldstr"
$	if f$search("nfldstr.ok").eqs."" then  create nfldstr.ok
$	gawk "$1 == 0 { print ""bug"" }" >tmp.

$	cmp nfldstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nors:		echo "nors"
$!! there's no straightforward way to supply non-terminated input on the fly
$!!	@echo A B C D E | tr -d '\12' | $(AWK) '{ print $$NF }' - $(srcdir)/nors.in > _$@
$!! so just read a line from sys$input instead
$	gawk "{ print $NF }" - nors.in >tmp.
A B C D E
$	cmp nors.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnarydel:	echo "fnarydel"
$	gawk -f fnarydel.awk >tmp.
$	cmp fnarydel.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$reint:		echo "reint"
$	gawk --re-interval -f reint.awk reint.in >tmp.
$	cmp reint.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$noparms:	echo "noparms"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f noparms.awk >tmp. 2>&1
$	if .not.$status then call exit_code 1
$	set On
$	cmp noparms.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pipeio1:	echo "pipeio1"
$	cat = "TYPE"	!close enough, as long as we avoid .LIS default suffix
$	define/User test1 []test1.
$	define/User test2 []test2.
$	gawk -f pipeio1.awk >tmp.
$	rm test1.;,test2.;
$	cmp pipeio1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$pipeio2:
$	echo "pipeio2:  uses Unix-specific command so won't work on VMS"
$	return
$!!pipeio2:	echo "pipeio2"
$	cat = "gawk -- {print}"
$	tr  = "??"	!unfortunately, no trivial substitution available...
$	gawk -v "SRCDIR=." -f pipeio2.awk >tmp.
$	cmp pipeio2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$funstack:	echo "funstack"
$	gawk -f funstack.awk funstack.in >tmp.
$	cmp funstack.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clobber:	echo "clobber"
$	gawk -f clobber.awk >tmp.
$	cmp clobber.ok seq.
$	if $status then  rm seq.;*
$	cmp clobber.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$delarprm:	echo "delarprm"
$	gawk -f delarprm.awk >tmp.
$	cmp delarprm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$prdupval:	echo "prdupval"
$	gawk -f prdupval.awk prdupval.in >tmp.
$	cmp prdupval.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nasty:	echo "nasty"
$	set noOn
$	gawk -f nasty.awk >tmp.
$	call fixup_LRL nasty.ok
$	call fixup_LRL tmp. "purge"
$	cmp nasty.ok tmp.
$	if $status then  rm tmp.;
$	set On
$	return
$
$nasty2:	echo "nasty2"
$	set noOn
$	gawk -f nasty2.awk >tmp.
$	call fixup_LRL nasty2.ok
$	call fixup_LRL tmp. "purge"
$	cmp nasty2.ok tmp.
$	if $status then  rm tmp.;
$	set On
$	return
$
$zeroflag:	echo "zeroflag"
$	gawk -f zeroflag.awk >tmp.
$	cmp zeroflag.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getnr2tm:	echo "getnr2tm"
$	gawk -f getnr2tm.awk getnr2tm.in >tmp.
$	cmp getnr2tm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getnr2tb:	echo "getnr2tb"
$	gawk -f getnr2tb.awk getnr2tb.in >tmp.
$	cmp getnr2tb.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$printf1:	echo "printf1"
$	gawk -f printf1.awk >tmp.
$	cmp printf1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$funsmnam:	echo "funsmnam"
$	set noOn
$	gawk -f funsmnam.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp funsmnam.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnamedat:	echo "fnamedat"
$	set noOn
$	gawk -f fnamedat.awk < fnamedat.in >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp fnamedat.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$numindex:	echo "numindex"
$	set noOn
$	gawk -f numindex.awk < numindex.in >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp numindex.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$subslash:	echo "subslash"
$	set noOn
$	gawk -f subslash.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp subslash.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$opasnslf:	echo "opasnslf"
$	set noOn
$	gawk -f opasnslf.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp opasnslf.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$opasnidx:	echo "opasnidx"
$	set noOn
$	gawk -f opasnidx.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp opasnidx.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arynocls:	echo "arynocls"
$	gawk -v "INPUT"=arynocls.in -f arynocls.awk >tmp.
$	cmp arynocls.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$igncdym:	echo "igncdym"
$	gawk -f igncdym.awk igncdym.in >tmp.
$	cmp igncdym.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$getlnbuf:	echo "getlnbuf"
$	gawk -f getlnbuf.awk getlnbuf.in >tmp.
$	gawk -f gtlnbufv.awk getlnbuf.in >tmp2.
$	cmp getlnbuf.ok tmp.
$	if $status then  rm tmp.;
$	cmp getlnbuf.ok tmp2.
$	if $status then  rm tmp2.;
$	return
$
$arysubnm:	echo "arysubnm"
$	gawk -f arysubnm.awk >tmp.
$	cmp arysubnm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnparydl:	echo "fnparydl"
$	gawk -f fnparydl.awk >tmp.
$	cmp fnparydl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nondec:	echo "nondec"
$	gawk -f nondec.awk >tmp.
$	cmp nondec.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nlstrina:	echo "nlstrina"
$	AWKPATH_srcdir
$	gawk -f nlstrina.awk >tmp.
$	cmp nlstrina.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$octsub:	echo "octsub"
$	AWKPATH_srcdir
$	gawk -f octsub.awk >tmp.
$	cmp octsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$nlinstr:	echo "nlinstr"
$	gawk -f nlinstr.awk nlinstr.in >tmp.
$	cmp nlinstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ofmt:	echo "ofmt"
$	gawk -f ofmt.awk ofmt.in >tmp.
$	cmp ofmt.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$hsprint:	echo "hsprint"
$	gawk -f hsprint.awk >tmp.
$	cmp hsprint.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fsfwfs:	echo "fsfwfs"
$	gawk -f fsfwfs.awk fsfwfs.in >tmp.
$	cmp fsfwfs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ofmts:	echo "ofmts"
$	gawk -f ofmts.awk ofmts.in >tmp.
$	cmp ofmts.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$parseme:	echo "parseme"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f parseme.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp parseme.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$splitdef:	echo "splitdef"
$	gawk -f splitdef.awk >tmp.
$	cmp splitdef.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnaryscl:	echo "fnaryscl"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f fnaryscl.awk >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp fnaryscl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$fnasgnm:	echo "fnasgnm"
$	set noOn
$	AWKPATH_srcdir
$	gawk -f fnasgnm.awk < fnasgnm.in >tmp. 2>&1
$	if .not.$status then call exit_code 2
$	set On
$	cmp fnasgnm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$lint:	echo "lint"
$	AWKPATH_srcdir
$	gawk -f lint.awk >tmp. 2>&1
$	cmp lint.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$procinfs:	echo "procinfs"
$	gawk -f procinfs.awk >tmp.
$	cmp procinfs.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$sort1:	echo "sort1"
$	gawk -f sort1.awk >tmp.
$	cmp sort1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ofmtbig:	echo "ofmtbig"
$	set noOn
$	gawk -f ofmtbig.awk ofmtbig.in >tmp. 2>&1
$	set On
$	cmp ofmtbig.ok tmp.
$	if $status then  rm tmp.;
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
$paramtyp:	echo "paramtyp"
$	gawk -f paramtyp.awk >tmp.
$	cmp paramtyp.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rsnul1nl:	echo "rsnul1nl"
$	gawk -f rsnul1nl.awk rsnul1nl.in >tmp.
$	cmp rsnul1nl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$datanonl:	echo "datanonl"
$	gawk -f datanonl.awk datanonl.in >tmp.
$	cmp datanonl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$regeq:	echo "regeq"
$	gawk -f regeq.awk regeq.in >tmp.
$	cmp regeq.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$redfilnm:	echo "redfilnm"
$	gawk -f redfilnm.awk srcdir="." redfilnm.in >tmp.
$	cmp redfilnm.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$strtod:	echo "strtod"
$	gawk -f strtod.awk strtod.in >tmp.
$	cmp strtod.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$leaddig:	echo "leaddig"
$	gawk -v "x=2E"  -f leaddig.awk >tmp.
$	cmp leaddig.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clos1way:
$	echo "clos1way:  uses unsupported `|&' redirection, so skipped"
$	return
$!!clos1way:	echo "clos1way"
$	gawk -f clos1way.awk >tmp.
$	cmp clos1way.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arynasty:	echo "arynasty"
$	gawk -f arynasty.awk >tmp.
$	cmp arynasty.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$shadow:	echo "shadow"
$	set noOn
$	AWKPATH_srcdir
$	gawk --lint -f shadow.awk >tmp. 2>&1
$	set On
$	cmp shadow.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$regx8bit:	echo "regx8bit"
$	gawk -f regx8bit.awk >tmp.
$	cmp regx8bit.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$psx96sub:	echo "psx96sub"
$	gawk -f psx96sub.awk >tmp.
$	cmp psx96sub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$addcomma:	echo "addcomma"
$	gawk -f addcomma.awk addcomma.in >tmp.
$	cmp addcomma.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$gnuops2:	echo "gnuops2"
$	gawk -f gnuops2.awk >tmp.
$	cmp gnuops2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rebt8b1:	echo "rebt8b1"
$	gawk -f rebt8b1.awk >tmp.
$	cmp rebt8b1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$rebt8b2:	echo "rebt8b2"
$	gawk -f rebt8b2.awk >tmp.
$	cmp rebt8b2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$leadnl:	echo "leadnl"
$	gawk -f leadnl.awk leadnl.in >tmp.
$	cmp leadnl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$funsemnl:	echo "funsemnl"
$	gawk -f funsemnl.awk >tmp.
$	cmp funsemnl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$ofmtfidl:	echo "ofmtfidl"
$	gawk -f ofmtfidl.awk ofmtfidl.in >tmp.
$	cmp ofmtfidl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$onlynl:	echo "onlynl"
$	gawk -f onlynl.awk onlynl.in >tmp.
$	cmp onlynl.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$arrymem1:	echo "arrymem1"
$	gawk -f arrymem1.awk >tmp.
$	cmp arrymem1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$compare2:	echo "compare2"
$	gawk -f compare2.awk >tmp.
$	cmp compare2.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$minusstr:	echo "minusstr"
$	gawk -f minusstr.awk >tmp.
$	cmp minusstr.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$membug1:	echo "membug1"
$	gawk -f membug1.awk membug1.in >tmp.
$	cmp membug1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$forsimp:	echo "forsimp"
$	gawk -f forsimp.awk >tmp.
$	cmp forsimp.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$concat1:	echo "concat1"
$	gawk -f concat1.awk concat1.in >tmp.
$	cmp concat1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$longsub:	echo "longsub"
$	gawk -f longsub.awk longsub.in >tmp.
$!! the records here are too long for DIFF to handle
$!! so assume success as long as gawk doesn't crash
$!!	call fixup_LRL longsub.ok
$!!	call fixup_LRL tmp. "purge"
$!!	cmp longsub.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$vms_io1:	echo "vms_io1"
$	if f$search("vms_io1.ok").eqs.""
$	then create vms_io1.ok
Hello
$	endif
$ !	define/User dbg$input sys$command:
$	gawk /Input=sys$input _NL: /Output=tmp.
# prior to 3.0.4, gawk crashed doing any redirection after closing stdin
BEGIN { print "Hello" >"/dev/stdout" }
$	cmp vms_io1.ok tmp.
$	if $status then  rm tmp.;
$	return
$
$clean:
$	if f$search("tmp.")	 .nes."" then  rm tmp.;*
$	if f$search("tmp.too")	 .nes."" then  rm tmp.too;*
$	if f$search("out%.")	 .nes."" then  rm out%.;*
$	if f$search("strftime.ok").nes."" then  rm strftime.ok;*
$	if f$search("test%.")	 .nes."" then  rm test%.;*
$	if f$search("seq.")	 .nes."" then  rm seq.;*
$	if f$search("_pid.in")	 .nes."" then  rm _pid.in;*
$	if f$search("[.junk]*.*").nes."" then  rm [.junk]*.*;*
$	if f$parse("[.junk]")	 .nes."" then  rm []junk.dir;1
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
$	open/Append ftmp tmp.
$	write ftmp "EXIT CODE: ",p1
$	close ftmp
$ endsubroutine !exit_code
$
$!NOTREACHED
$ exit
