:: Windows Gawk testsuite runner script.
:: Carefully ported from the appropriate makefiles by Michael M. Builov (mbuilov@gmail.com)
:: Public domain.

:: Note: line ends _must_ be CRLF, otherwise cmd.exe may interpret script incorrectly.

@echo off
setlocal

:: Note: these environment variables are taken in account (default value in parents):
::
:: BLD_DIST       (dist)      - path to gawk distribution directory
:: GAWKLOCALE     ()          - default value of LC_ALL and LANG environment variables
:: GAWK_TEST_ARGS ()          - additional agruments passed to gawk executable; this can be "-M" or "--bignum"

:: re-execute batch file to avoid pollution of the environment variables space of the caller process
if not defined TESTBATRECURSIVE (
  set TERM_PROGRAM=
  set "TESTBATRECURSIVE=%CMDCMDLINE:"=%"
  %SYSTEMROOT%\System32\cmd.exe /s /c "call "%~dpnx0" "%*""
  exit /b
)

:: run commands from the directory of this batch file
pushd "%~dp0"

if not defined BLD_DIST (set BLD_DIST=dist) else set "BLD_DIST=%BLD_DIST:"=%"

:: make BLD_DIST path absolute
call :get_abs_dir "%BLD_DIST%" BLD_DIST

if not "%~2"=="" (set DO_TEST_ONLY=x) else set "DO_TEST_ONLY=%~1"

if defined DO_TEST_ONLY (
  if not "%DO_TEST_ONLY%"=="basic" (
  if not "%DO_TEST_ONLY%"=="unix" (
  if not "%DO_TEST_ONLY%"=="ext" (
  if not "%DO_TEST_ONLY%"=="cpu" (
  if not "%DO_TEST_ONLY%"=="locale" (
  if not "%DO_TEST_ONLY%"=="shlib" (
  if not "%DO_TEST_ONLY%"=="extra" (
  if not "%DO_TEST_ONLY%"=="inet" (
  if not "%DO_TEST_ONLY%"=="mpfr" (
    echo Usage:
    echo.
    echo %~nx0         - run all tests
    echo %~nx0 basic   - run only basic tests
    echo %~nx0 unix    - run only unix tests
    echo %~nx0 ext     - run only extension tests
    echo %~nx0 cpu     - run only machine tests
    echo %~nx0 locale  - run only locale tests
    echo %~nx0 shlib   - run only tests of extension DLLs
    echo %~nx0 extra   - run only extra tests
    echo %~nx0 inet    - run only inet tests
    echo %~nx0 mpfr    - run only mpfr tests
    if not "%TESTBATRECURSIVE:test.bat=%"=="%TESTBATRECURSIVE%" ((echo.)&set /p DUMMY="Hit ENTER to close...")
    exit /b 1
))))))))))

set "FIND=%SYSTEMROOT%\System32\Find.exe"

:: strange enough, but the 'more' command will not work if environment variable MORE is defined
:: WindowsXP: if environment variable 'MORE' is not defined, this command will raise ERRORLEVEL!
set MORE=

call :tests || goto :test_err

echo OK!
exit /b 0

:test_err
echo *** Test failed! ***
exit /b 1

:::::: TESTSUITE :::::
:tests

::NOTE: when running commands via cmd.exe, use two double quotes for escaping
:: a double quote in command arguments, escaping with a backslash simply do not
:: works in non-primitive cases, e.g.:
::  gawk.exe "BEGIN{print\"&echo \"}"
:: will print \"}"
:: - becase cmd.exe ignores backslashes and interprets unscaped text as commands.

echo.
echo.===============================================================
echo.Any output from FC is bad news, although some differences
echo.in floating point values are probably benign -- in particular,
echo.some systems may omit a leading zero and the floating point
echo.precision may lead to slightly different output in a few cases.
echo.===============================================================

setlocal

:: note: here ERRORLEVEL is zero due to setlocal

set TESTS_UNSUPPORTED=0
set TESTS_PARTIAL=0
set TESTS_SKIPPED=0

set GAWK="%BLD_DIST%\gawk.exe" %GAWK_TEST_ARGS%

:: path to gawk executable in double-double quotes + additional arguments
set "QGAWK=%GAWK:"=""%"

:: For testing, use only standard windows utilities
call :execq "set ""Path=%SYSTEMROOT%\system32;%SYSTEMROOT%;%SYSTEMROOT%\System32\Wbem"""

call :change_locale C || goto :exit_local
%GAWK% -f "test\printlang.awk" || ((echo.failed to execute: %GAWK% -f "test\printlang.awk") & goto :exit_local)

call :exec cd test
set err=%ERRORLEVEL%
if not %err% equ 0 goto :test_err_no_cd
call :tests_in_directory
set err=%ERRORLEVEL%
call :exec cd ..
if %err% equ 0 set err=%ERRORLEVEL%
:test_err_no_cd

if %err% equ 0 echo Unsupported tests: %TESTS_UNSUPPORTED%
if %err% equ 0 echo Partial tests:     %TESTS_PARTIAL%
if %err% equ 0 echo Skipped tests:     %TESTS_SKIPPED%

endlocal & exit /b %err%

:tests_in_directory
call :execq "set AWKPATH=." || exit /b

if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="basic" goto :try_unix_tests
echo.======== Starting basic tests ========
call :basic_tests           || exit /b
echo.======== Done with basic tests ========

:try_unix_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="unix" goto :try_ext_tests
echo.======== Starting Unix tests ========
call :unix_tests            || exit /b
echo.======== Done with Unix tests ========

:try_ext_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="ext" goto :try_cpu_tests
echo.======== Starting gawk extension tests ========
call :ext_tests             || exit /b
echo.======== Done with gawk extension tests ========

:try_cpu_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="cpu" goto :try_locale_tests
echo.======== Starting machine-specific tests ========
call :machine_tests         || exit /b
echo.======== Done with machine-specific tests ========

:try_locale_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="locale" goto :try_shlib_tests
echo.======== Starting tests that can vary based on character set or locale support ========
echo.**************************************************************************
echo.* Some or all of these tests may fail if you have inadequate or missing  *
echo.* locale support. At least en_US.UTF-8, fr_FR.UTF-8, ru_RU.UTF-8 and     *
echo.* ja_JP.UTF-8 are needed. The el_GR.iso88597 is optional but helpful.    *
echo.**************************************************************************
call :charset_tests         || exit /b
echo.======== Done with tests that can vary based on character set or locale support ========

:try_shlib_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="shlib" goto :try_extra_tests
echo.======== Starting shared library tests ========
call :shlib_tests           || exit /b
echo.======== Done with shared library tests ========

:try_extra_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="extra" goto :try_inet_tests
echo.======== Starting extra tests ========
call :extra_tests           || exit /b
echo.======== Done with extra tests ========

:try_inet_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="inet" goto :try_mpfr_tests
echo.======== Starting inet tests ========
call :inet_tests            || exit /b
echo.======== Done with inet tests ========

:try_mpfr_tests
if defined DO_TEST_ONLY if not "%DO_TEST_ONLY%"=="mpfr" goto :end_of_tests
echo.======== Starting MPFR tests ========
call :mpfr_tests            || exit /b
echo.======== Done with MPFR tests ========

:end_of_tests

if defined DO_TEST_ONLY (
  echo.%DO_TEST_ONLY% TESTS PASSED
) else (
  echo.ALL TESTS PASSED
)
exit /b

:::::: BASIC_TESTS (266 upstream tests + 1 additional test + 2 debug tests) :::::
:basic_tests

call :runtest_in      addcomma                                || exit /b
call :runtest_in      anchgsub                                || exit /b
call :runtest_in      anchor                                  || exit /b

call :execq "copy argarray.in argarray.input > NUL"           || exit /b
call :execq "echo just a test | %QGAWK% -f argarray.awk ./argarray.input - > _argarray" || exit /b
call :cmpdel argarray && call :exec del /q argarray.input     || exit /b

call :runtest_in      arrayind1                               || exit /b
call :runtest         arrayind2                               || exit /b
call :runtest         arrayind3                               || exit /b
call :runtest_fail    arrayparm                               || exit /b
call :runtest         arrayprm2                               || exit /b
call :runtest         arrayprm3                               || exit /b
call :runtest         arrayref                                || exit /b
call :runtest         arrymem1                                || exit /b
call :runtest         arryref2                                || exit /b
call :runtest_fail    arryref3                                || exit /b
call :runtest_fail    arryref4                                || exit /b
call :runtest_fail    arryref5                                || exit /b
call :runtest         arynasty                                || exit /b
call :runtest         arynocls -v "INPUT=arynocls.in"         || exit /b
call :runtest_fail    aryprm1                                 || exit /b
call :runtest_fail    aryprm2                                 || exit /b
call :runtest_fail    aryprm3                                 || exit /b
call :runtest_fail    aryprm4                                 || exit /b
call :runtest_fail    aryprm5                                 || exit /b
call :runtest_fail    aryprm6                                 || exit /b
call :runtest_fail    aryprm7                                 || exit /b
call :runtest         aryprm8                                 || exit /b
call :runtest         aryprm9                                 || exit /b
call :runtest         arysubnm                                || exit /b
call :runtest         aryunasgn                               || exit /b
call :runtest_in      asgext                                  || exit /b

setlocal
call :exec set AWKPATH=lib                                    || goto :exit_local
call :runtest         awkpath                                 || goto :exit_local
endlocal

call :runtest_in      assignnumfield                          || exit /b
call :runtest         assignnumfield2                         || exit /b
call :runtest_in      back89                                  || exit /b
call :runtest_in      backgsub                                || exit /b
call :runtest_fail    badassign1                              || exit /b
call :runtest_fail    badbuild                                || exit /b
call :runtest_fail    callparam                               || exit /b
call :runtest_in      childin                                 || exit /b

call :runtest         clobber                                 || exit /b
call :cmpdel_         clobber.ok seq                          || exit /b

call :runtest         closebad                                || exit /b
call :runtest_in      clsflnam                                || exit /b
call :runtest         compare 0 1 compare.in                  || exit /b
call :runtest         compare2                                || exit /b
call :runtest_in      concat1                                 || exit /b
call :runtest         concat2                                 || exit /b
call :runtest         concat3                                 || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      concat4                                 || goto :exit_local
endlocal

call :runtest         concat5                                 || exit /b
call :runtest         convfmt                                 || exit /b
call :runtest_in      datanonl                                || exit /b
call :runtest_fail    defref --lint                           || exit /b
call :runtest         delargv                                 || exit /b
call :runtest         delarpm2                                || exit /b
call :runtest         delarprm                                || exit /b
call :runtest_fail    delfunc                                 || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      dfamb1                                  || goto :exit_local
endlocal

call :runtest         dfastress                               || exit /b
call :runtest         dynlj                                   || exit /b
call :runtest_in      escapebrace --posix                     || exit /b
call :runtest         eofsplit                                || exit /b
call :runtest_fail_   eofsrc1 -f eofsrc1a.awk -f eofsrc1b.awk || exit /b
call :runtest         exit2                                   || exit /b
call :runtest_ok      exitval1                                || exit /b
call :runtest         exitval2                                || exit /b
call :runtest_fail    exitval3                                || exit /b
call :runtest_fail    fcall_exit                              || exit /b
call :runtest_fail_in fcall_exit2                             || exit /b
call :runtest_in      fldchg                                  || exit /b
call :runtest_in      fldchgnf                                || exit /b
call :runtest_in      fldterm                                 || exit /b
call :runtest_fail_in fnamedat                                || exit /b
call :runtest_fail    fnarray                                 || exit /b
call :runtest_fail    fnarray2                                || exit /b
call :runtest_fail    fnaryscl                                || exit /b
call :runtest_fail    fnasgnm                                 || exit /b
call :runtest_fail    fnmisc                                  || exit /b
call :runtest         fordel                                  || exit /b
call :runtest         forref                                  || exit /b
call :runtest         forsimp                                 || exit /b
call :runtest_in      fsbs                                    || exit /b
call :runtest_in      fscaret                                 || exit /b
call :runtest_in      fsnul1                                  || exit /b
call :runtest_in      fsrs                                    || exit /b
call :runtest         fsspcoln  """FS=[ :]+""" fsspcoln.in    || exit /b
call :runtest_in      fstabplus                               || exit /b
call :runtest         funsemnl                                || exit /b
call :runtest_fail    funsmnam                                || exit /b
call :runtest_in      funstack                                || exit /b
call :runtest_in      getline                                 || exit /b
call :runtest         getline2 getline2.awk getline2.awk      || exit /b
call :runtest         getline3                                || exit /b
call :runtest_in      getline4                                || exit /b
call :runtest         getline5                                || exit /b
call :runtest         getlnbuf                 getlnbuf.in    || exit /b
call :runtest_        getlnbuf -f gtlnbufv.awk getlnbuf.in    || exit /b
call :runtest_in      getnr2tb                                || exit /b
call :runtest_in      getnr2tm                                || exit /b
call :runtest_fail    gsubasgn                                || exit /b
call :runtest         gsubtest                                || exit /b
call :runtest         gsubtst2                                || exit /b
call :runtest_in      gsubtst3 --re-interval                  || exit /b
call :runtest         gsubtst4                                || exit /b
call :runtest_in      gsubtst5                                || exit /b

setlocal
call :change_locale C                                         || goto :exit_local
call :runtest         gsubtst6                                || goto :exit_local
endlocal

call :runtest_in      gsubtst7                                || exit /b
call :runtest_in      gsubtst8                                || exit /b
call :runtest         hex                                     || exit /b
call :runtest_in      hex2                                    || exit /b
call :runtest         hsprint                                 || exit /b
call :runtest_in      inpref                                  || exit /b
call :runtest         inputred                                || exit /b
call :runtest         intest                                  || exit /b
call :runtest         intprec                                 || exit /b
call :runtest         iobug1                                  || exit /b
call :runtest         leaddig                                 || exit /b
call :runtest_in      leadnl                                  || exit /b
call :runtest_in      litoct --traditional                    || exit /b
call :runtest_in      longsub                                 || exit /b
call :runtest_in      longwrds -v "SORT=sort"                 || exit /b
call :runtest_in      manglprm                                || exit /b
call :runtest         math                                    || exit /b
call :runtest_in      membug1                                 || exit /b
call :runtest         memleak                                 || exit /b

call :execq "%QGAWK% -f messages.awk >_out2 2>_out3" && ^
call :cmpdel out1 && ^
call :cmpdel out2 && ^
call :cmpdel out3                                             || exit /b

call :runtest         minusstr                                || exit /b
call :runtest_in      mmap8k                                  || exit /b
call :runtest         nasty                                   || exit /b
call :runtest         nasty2                                  || exit /b
call :runtest         negexp                                  || exit /b
call :runtest         negrange                                || exit /b
call :runtest_in      nested                                  || exit /b
call :runtest_in      nfldstr                                 || exit /b
call :runtest         nfloop                                  || exit /b
call :runtest_fail    nfneg                                   || exit /b
call :runtest_in      nfset                                   || exit /b
call :runtest_in      nlfldsep                                || exit /b
call :runtest_in      nlinstr                                 || exit /b
call :runtest         nlstrina                                || exit /b
call :runtest         noeffect --lint                         || exit /b
call :runtest_fail_   nofile """{}""" no/such/file            || exit /b
call :runtest         nofmtch --lint                          || exit /b
call :runtest_in      noloop1                                 || exit /b
call :runtest_in      noloop2                                 || exit /b
call :runtest_in      nonl --lint                             || exit /b
call :runtest_fail    noparms                                 || exit /b

call :execq "<NUL set /p=A B C D E | %QGAWK% ""{ print $NF }"" - nors.in > _nors" && ^
call :cmpdel nors                                             || exit /b

call :runtest_fail    nulinsrc                                || exit /b
call :runtest_in      nulrsend                                || exit /b
call :runtest_in      numindex                                || exit /b
call :runtest_mpfr    numrange                                || exit /b
call :runtest         numstr1                                 || exit /b
call :runtest_in      numsubstr                               || exit /b
call :runtest         octsub                                  || exit /b
call :runtest_in      ofmt                                    || exit /b
call :runtest         ofmta                                   || exit /b
call :runtest_in      ofmtbig                                 || exit /b
call :runtest_in      ofmtfidl                                || exit /b
call :runtest_in      ofmts                                   || exit /b
call :runtest         ofmtstrnum                              || exit /b
call :runtest_in      ofs1                                    || exit /b
call :runtest_in      onlynl                                  || exit /b
call :runtest         opasnidx                                || exit /b
call :runtest         opasnslf                                || exit /b
call :runtest_fail    paramasfunc1 --posix                    || exit /b
call :runtest_fail    paramasfunc2 --posix                    || exit /b
call :runtest_fail    paramdup                                || exit /b
call :runtest_fail    paramres                                || exit /b
call :runtest         paramtyp                                || exit /b
call :runtest         paramuninitglobal                       || exit /b
call :runtest_in      parse1                                  || exit /b
call :runtest_in      parsefld                                || exit /b
call :runtest_fail    parseme                                 || exit /b
call :runtest         pcntplus                                || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         posix_compare --posix                   || goto :exit_local
endlocal

call :runtest         posix2008sub --posix                    || exit /b
call :runtest_in      prdupval                                || exit /b
call :runtest         prec                                    || exit /b
call :runtest         printf0 --posix                         || exit /b
call :runtest         printf1                                 || exit /b
call :runtest         printfchar                              || exit /b
call :runtest_fail    prmarscl                                || exit /b
call :runtest         prmreuse                                || exit /b
call :runtest         prt1eval                                || exit /b
call :runtest         prtoeval                                || exit /b
call :runtest_mpfr    rand                                    || exit /b

setlocal
call :exec set GAWKTEST_NO_TRACK_MEM=1                        || goto :exit_local
call :runtest randtest "-vRANDOM=" "-vNSAMPLES=1024" "-vMAX_ALLOWED_SIGMA=5" "-vNRUNS=50" || goto :exit_local
endlocal

call :runtest_in      range1                                  || exit /b

setlocal
call :change_locale C                                         || goto :exit_local
call :runtest         range2                                  || goto :exit_local
endlocal

call :runtest_fail    readbuf                                 || exit /b
call :runtest_in      rebrackloc                              || exit /b
call :runtest         rebt8b1                                 || exit /b
call :runtest_in      rebuild                                 || exit /b
call :runtest         redfilnm "srcdir=." redfilnm.in         || exit /b
call :runtest_in      regeq                                   || exit /b
call :runtest_in      regexpbrack                             || exit /b
call :runtest_in      regexpbrack2                            || exit /b
call :runtest         regexprange                             || exit /b
call :runtest         regrange                                || exit /b
call :runtest_in      reindops                                || exit /b
call :runtest_in      reparse                                 || exit /b
call :runtest_in      resplit                                 || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      rri1                                    || goto :exit_local
endlocal

call :runtest_in      rs                                      || exit /b
call :runtest_in      rscompat --traditional                  || exit /b
call :runtest_in      rsnul1nl                                || exit /b

setlocal
:: Suppose that block size for pipe is at most 128kB:
set "COMMAND=%QGAWK% ""BEGIN { for ^(i = 1; i ^<= 128*64+1; i++^) print """"abcdefgh123456\n"""" }"" 2>&1"
set "COMMAND=%COMMAND% | %QGAWK% ""BEGIN { RS = """"""""; ORS = """"\n\n"""" }; { print }"" 2>&1"
set "COMMAND=%COMMAND% | %QGAWK% "" /^^[^^a]/; END{ print NR }"""
call :execq "%COMMAND% >_rsnulbig" && call :cmpdel rsnulbig   || goto :exit_local
endlocal

setlocal
set "COMMAND=%QGAWK% ""BEGIN { ORS = """"""""; n = """"\n""""; for ^(i = 1; i ^<= 10; i++^) n = ^(n n^); "
set "COMMAND=%COMMAND%for ^(i = 1; i ^<= 128; i++^) print n; print """"abc\n"""" }"" 2>&1"
set "COMMAND=%COMMAND% | %QGAWK% ""BEGIN { RS = """"""""; ORS = """"\n\n"""" };{ print }"" 2>&1"
set "COMMAND=%COMMAND% | %QGAWK% "" /^^[^^a]/; END { print NR }"""
call :execq "%COMMAND% >_rsnulbig2" && call :cmpdel rsnulbig2 || goto :exit_local
endlocal

call :runtest_in      rsnulw                                  || exit /b
call :runtest         rstest1                                 || exit /b
call :runtest         rstest2                                 || exit /b
call :runtest         rstest3                                 || exit /b
call :runtest         rstest4                                 || exit /b
call :runtest         rstest5                                 || exit /b
call :runtest_in      rswhite                                 || exit /b
call :runtest_fail    scalar                                  || exit /b
call :runtest_fail    sclforin                                || exit /b
call :runtest_fail    sclifin                                 || exit /b
call :runtest_in      setrec0                                 || exit /b
call :runtest         setrec1                                 || exit /b
call :runtest         sigpipe1                                || exit /b
call :runtest         sortempty                               || exit /b
call :runtest_in      sortglos                                || exit /b

setlocal
call :exec set LC_ALL=C                                       || goto :exit_local
call :runtest         spacere                                 || goto :exit_local
endlocal

call :runtest_in      splitargv                               || exit /b
call :runtest         splitarr                                || exit /b
call :runtest         splitdef                                || exit /b
call :runtest_in      splitvar                                || exit /b
call :runtest         splitwht                                || exit /b
call :runtest         status-close                            || exit /b
call :runtest         strcat1                                 || exit /b
call :runtest_in      strfieldnum                             || exit /b
call :runtest         strnum1                                 || exit /b
call :runtest         strnum2                                 || exit /b
call :runtest_in      strtod                                  || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      subamp                                  || goto :exit_local
endlocal

call :runtest_in      subback                                 || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         subi18n                                 || goto :exit_local
endlocal

call :runtest         subsepnm                                || exit /b
call :runtest         subslash                                || exit /b
call :runtest         substr                                  || exit /b
call :runtest_in      swaplns                                 || exit /b
call :runtest_fail    synerr1                                 || exit /b
call :runtest_fail    synerr2                                 || exit /b
call :runtest_fail    synerr3                                 || exit /b
call :runtest         tailrecurse                             || exit /b
call :runtest_in      tradanch --traditional                  || exit /b
call :runtest_fail_in trailbs                                 || exit /b

call :runtest         tweakfld tweakfld.in && ^
call :exec del /q errors.cleanup                              || exit /b

call :runtest         uninit2 --lint                          || exit /b
call :runtest         uninit3 --lint                          || exit /b
call :runtest         uninit4 --lint                          || exit /b
call :runtest         uninit5 --lint                          || exit /b
call :runtest         uninitialized --lint                    || exit /b
call :runtest_fail    unterm                                  || exit /b
call :runtest_in      uparrfs                                 || exit /b
call :runtest         uplus                                   || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      wideidx                                 || goto :exit_local
call :runtest         wideidx2                                || goto :exit_local
call :runtest         widesub                                 || goto :exit_local
call :runtest         widesub2                                || goto :exit_local
call :runtest_in      widesub3                                || goto :exit_local
call :runtest         widesub4                                || goto :exit_local
endlocal

call :runtest_in      wjposer1                                || exit /b
call :runtest         zero2                                   || exit /b
call :runtest         zeroe0                                  || exit /b
call :runtest         zeroflag                                || exit /b

:: + 1 additional test
:: this test is not in upstream yet - check that gawk warns about incorrect use of character class references
call :execq "%QGAWK% ""/[:space:]/"" < NUL 2>&1 | ""%FIND%"" ""[[:space:]]"" > NUL" || exit /b

:: + 2 debug tests
:: ARRAYDEBUG_TESTS
%GAWK% "BEGIN{adump(f, -1)}" < NUL > NUL 2>&1 || (
  set /A TESTS_SKIPPED+=2 & >&2 echo *** Skipped 2 tests: arrdbg, arrdbg-mpfr - gawk was not compiled to support the array debug tests
  goto :skip_arr_debug_test
)

call :execq """%BLD_DIST%\gawk.exe"" -f arrdbg.awk -v ""okfile=arrdbg.ok"" -v ""mpfr_okfile=arrdbg-mpfr.ok"" | ""%FIND%"" ""array_f"" > _arrdbg" && ^
call :cmpdel arrdbg && call :exec del /q arrdbg.ok arrdbg-mpfr.ok || exit /b

%GAWK% --version | "%FIND%" "MPFR" > NUL 2>&1 || (
  set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: arrdbg-mpfr - gawk was built without MPFR support
  goto :skip_arr_debug_test
)

call :execq """%BLD_DIST%\gawk.exe"" -M -f arrdbg.awk -v ""okfile=arrdbg.ok"" -v ""mpfr_okfile=arrdbg-mpfr.ok"" | ""%FIND%"" ""array_f"" > _arrdbg-mpfr" && ^
call :cmpdel arrdbg-mpfr && call :exec del /q arrdbg.ok arrdbg-mpfr.ok || exit /b

:skip_arr_debug_test
exit /b 0

:::::: UNIX_TESTS (10 of 11 upstream tests - 1 test is unsupported) :::::
:unix_tests

call :execq "(fflush.bat %QGAWK%) >_fflush" && ^
call :cmpdel fflush                                           || exit /b
call :runtest         getlnhd                                 || exit /b
call :execq "(localenl.bat %QGAWK%)"                          || exit /b

:: check if we have enough privileges to run wmic
<NUL 2>&1 1>NUL wmic process /? | "%FIND%" /v "" > NUL && (
  set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: winpid - not enough privileges to run wmic
  goto :skip_winpid
)
setlocal
set "EGAWK=%GAWK:"=\""%"
call :execq "del /q winpid.done 2> NUL"                       || goto :exit_local
call :execq "<nul wmic process call create ""%EGAWK% -f .\winpid.awk"",""%CD%"" 2>&1 | ""%FIND%"" ""ProcessId"" > _winpid" || goto :exit_local
call :waitfor winpid.done                                     || goto :exit_local
call :cmpdel winpid                                           || goto :exit_local
call :exec del /q winpid.ok winpid.done                       || goto :exit_local
endlocal
:skip_winpid

call :runtest pipeio1 && call :exec del /q test1 test2        || exit /b

setlocal
set OK_SUFFIX=_win
call :runtest         pipeio2 "-vSRCDIR=."                    || goto :exit_local
endlocal

:: UNSUPPORTED test: poundbang - cannot be ported to Windows
set /A TESTS_UNSUPPORTED+=1 & >&2 echo *** Unsupported 1 test: poundbang

setlocal
set "RTLENCMD=%QGAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND=%QGAWK% ""BEGIN {printf """"0\n\n\n1\n\n\n\n\n2\n\n""""; exit}"""
call :execq "%COMMAND% | %RTLENCMD% > _rtlen" && ^
call :cmpdel rtlen                                            || goto :exit_local
endlocal

setlocal
set "RTLENCMD=%QGAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND=%QGAWK% ""BEGIN {printf """"0""""; exit}"" | %RTLENCMD%"
set "COMMAND=%COMMAND%&%QGAWK% ""BEGIN {printf """"0\n""""; exit}"" | %RTLENCMD%"
set "COMMAND=%COMMAND%&%QGAWK% ""BEGIN {printf """"0\n\n""""; exit}"" | %RTLENCMD%"
call :execq "(%COMMAND%) > _rtlen01" && ^
call :cmpdel rtlen01                                          || goto :exit_local
endlocal

call :runtest_fail_ space -f """ """ .\space.awk              || exit /b

setlocal
call :exec set TZ=UTC                                         || goto :exit_local
call :runtest strftlng                                        || goto :exit_local
endlocal

exit /b 0

:::::: GAWK_EXT_TESTS (206 of 212 upstream tests - 6 tests are unsupported) :::::
:ext_tests

call :runtest_fail    aadelete1                               || exit /b
call :runtest_fail    aadelete2                               || exit /b
call :runtest         aarray1                                 || exit /b
call :runtest         aasort                                  || exit /b
call :runtest         aasorti                                 || exit /b
call :runtest         argtest -x -y abc                       || exit /b
call :runtest         arraysort                               || exit /b
call :runtest         arraysort2                              || exit /b
call :runtest_mpfr    arraytype                               || exit /b
call :runtest_in      backw                                   || exit /b

setlocal
set OK_SUFFIX=_win
call :execq "%QGAWK% -f 2>&1 | ""%FIND%"" /v ""patchlevel"" | ""%FIND%"" /v ""--parsedebug"" > _badargs" && ^
call :cmpdel badargs                                          || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win
call :execq "(echo x&echo y&echo z) > Xfile"                   || goto :exit_local
call :runtest beginfile1 beginfile1.awk . ./no/such/file Xfile || goto :exit_local
call :exec del /q Xfile                                        || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win
call :exec set LC_ALL=C                                       || goto :exit_local
call :exec set AWK=%GAWK%                                     || goto :exit_local
call :execq "(beginfile2.bat) > _beginfile2 2>&1" && ^
call :cmpdel beginfile2                                       || goto :exit_local
endlocal

call :runtest_ binmode1 -v "BINMODE=3" """BEGIN { print BINMODE }""" || exit /b

setlocal
set OK_SUFFIX=_win
call :change_locale "en_US.UTF-8"                                                      || goto :exit_local
:: BINMODE=2 is needed for PC tests.
call :execq "%QGAWK% -b -vBINMODE=2 -f charasbytes.awk charasbytes.in > _charasbytes1" || goto :exit_local
call :execq "echo 0000000000000000 > _charasbytes2"                                    || goto :exit_local
call :execq "fc /b _charasbytes1 _charasbytes2 | ""%FIND%"" ""0000"" > _charasbytes"   || goto :exit_local
call :cmpdel charasbytes                                                               || goto :exit_local
call :exec del /q _charasbytes1 _charasbytes2                                          || goto :exit_local
endlocal

call :execq "(for /L %%%%i in (1,1,3) do @%QGAWK% -f colonwarn.awk %%%%i < colonwarn.in) > _colonwarn" && ^
call :cmpdel colonwarn                                        || exit /b

setlocal
call :change_locale C                                         || goto :exit_local
call :runtest         clos1way                                || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win
call :runtest_fail_in clos1way2                               || goto :exit_local
endlocal

call :runtest_fail    clos1way3                               || exit /b
call :runtest_fail    clos1way4                               || exit /b

call :execq_fail "%QGAWK% -f clos1way5.awk > _clos1way5 2> clos1way5.tmp1" clos1way5.tmp2 && ^
call :execq """%FIND%"" ""clos1way5"" < clos1way5.tmp1 >> _clos1way5" && ^
call :execq "type clos1way5.tmp2 >> _clos1way5" && ^
call :cmpdel clos1way5 && call :exec del /q clos1way5.tmp1 clos1way5.tmp2 || exit /b

setlocal
set OK_SUFFIX=_win
call :execq "%QGAWK% -f clos1way6.awk > _clos1way6.out 2> _clos1way6.err" && ^
call :execq "copy /b _clos1way6.err + _clos1way6.out _clos1way6 >NUL"     || goto :exit_local
call :cmpdel clos1way6 && call :exec del /q _clos1way6.out _clos1way6.err || goto :exit_local
endlocal

call :runtest         crlf                                    || exit /b
call :runtest_fail_   dbugeval --debug -f NUL "< dbugeval.in" || exit /b
call :runtest_fail_in dbugeval2 --debug                       || exit /b
call :runtest_fail_in dbugeval3 --debug                       || exit /b
call :runtest_fail_in dbugtypedre1 --debug                    || exit /b
call :runtest_fail_in dbugtypedre2 --debug                    || exit /b
call :runtest         delsub                                  || exit /b

:: UNSUPPORTED tests: under windows, can't pass to console program more than 3 standard handles (stdin, stdout and stderr)
::call :runtest_ devfd 1 /dev/fd/4 /dev/fd/5 "4<devfd.in4" "5<devfd.in5"  || exit /b
::call :runtest  devfd1 "4<devfd.in1" "5<devfd.in2"                       || exit /b
::call :runtest_ devfd2 1 /dev/fd/4 /dev/fd/5 "4<devfd.in1" "5<devfd.in2" || exit /b
set /A TESTS_UNSUPPORTED+=3 & >&2 echo *** Unsupported 3 tests: devfd devfd1 devfd2

call :runtest_in      dfacheck1                               || exit /b

call :execq "%QGAWK% --dump-variables 1 < dumpvars.in > NUL" && ^
call :execq "< awkvars.out ""%FIND%"" /v ""ENVIRON"" | ""%FIND%"" /v ""PROCINFO"" > _dumpvars" && ^
call :cmpdel dumpvars && call :exec del /q awkvars.out        || exit /b

setlocal
set OK_SUFFIX=_win
call :runtest         errno errno.in                          || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win
call :execq "(exit.bat %QGAWK%) > _exit 2>&1" && ^
call :cmpdel exit                                             || goto :exit_local
endlocal

call :runtest_in      fieldwdth                               || exit /b
call :runtest_mpfr    forcenum --non-decimal-data             || exit /b
call :runtest_in      fpat1                                   || exit /b
call :runtest         fpat2                                   || exit /b
call :runtest_in      fpat3                                   || exit /b
call :runtest         fpat4                                   || exit /b
call :runtest_in      fpat5                                   || exit /b
call :runtest_in      fpat6                                   || exit /b
call :runtest_in      fpat7                                   || exit /b
call :runtest_in      fpat8                                   || exit /b
call :runtest_in      fpatnull                                || exit /b
call :runtest_in      fsfwfs                                  || exit /b
call :runtest_in      funlen                                  || exit /b
call :runtest_fail    functab1                                || exit /b
call :runtest_fail    functab2                                || exit /b
call :runtest         functab3                                || exit /b
call :runtest_in      fwtest                                  || exit /b
call :runtest_in      fwtest2                                 || exit /b
call :runtest_in      fwtest3                                 || exit /b
call :runtest_in      fwtest4                                 || exit /b
call :runtest_in      fwtest5                                 || exit /b
call :runtest_fail_in fwtest6                                 || exit /b
call :runtest_in      fwtest7                                 || exit /b
call :runtest_fail_in fwtest8                                 || exit /b
call :runtest         genpot --gen-pot                        || exit /b
call :runtest_in      gensub                                  || exit /b
call :runtest         gensub2                                 || exit /b
call :runtest_in      gensub3                                 || exit /b
call :runtest         getlndir                                || exit /b
call :runtest         gnuops2                                 || exit /b
call :runtest         gnuops3                                 || exit /b
call :runtest         gnureops                                || exit /b
call :runtest_fail    gsubind                                 || exit /b
call :runtest         icasefs                                 || exit /b
call :runtest_in      icasers                                 || exit /b
call :runtest         id                                      || exit /b
call :runtest_in      igncdym                                 || exit /b
call :runtest_in      igncfs                                  || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         ignrcas2                                || goto :exit_local
endlocal

call :runtest         ignrcas4                                || exit /b
call :runtest_in      ignrcase                                || exit /b

setlocal
set "SCRIPT=""BEGIN {print sandwich^(""""a"""", """"b"""", """"c""""^)}"""
call :runtest_        incdupe --lint -i inclib -i inclib.awk "%SCRIPT%" || goto :exit_local
endlocal

call :runtest_fail_   incdupe2 --lint -f inclib -f inclib.awk  || exit /b
call :runtest_        incdupe3 --lint -f hello -f hello.awk    || exit /b
call :runtest_fail_   incdupe4 --lint -f hello -i hello.awk    || exit /b
call :runtest_fail_   incdupe5 --lint -i hello -f hello.awk    || exit /b
call :runtest_fail_   incdupe6 --lint -i inchello -f hello.awk || exit /b
call :runtest_fail_   incdupe7 --lint -f hello -i inchello     || exit /b

call :runtest         include                                 || exit /b

setlocal
set "SCRIPT=""BEGIN {print sandwich^(""""a"""", """"b"""", """"c""""^)}"""
call :runtest_        include2 --include inclib "%SCRIPT%"    || goto :exit_local
endlocal

call :runtest         indirectbuiltin                         || exit /b
call :runtest_in      indirectcall                            || exit /b
call :runtest         indirectcall2                           || exit /b
call :runtest         intarray --non-decimal-data             || exit /b

call :execq "%QGAWK% -f iolint_win.awk > _iolint_win.out 2> _iolint_win.err" && ^
call :execq "copy /b _iolint_win.err + _iolint_win.out _iolint_win >NUL"     || exit /b
call :cmpdel iolint_win && call :exec del /q _iolint_win.out _iolint_win.err f1 f2 catcat "echo hello" || exit /b

call :runtest         isarrayunset                            || exit /b
call :runtest         lint                                    || exit /b
call :runtest         lintexp    --lint                       || exit /b
call :runtest         lintindex  --lint                       || exit /b
call :runtest         lintint    --lint                       || exit /b
call :runtest         lintlength --lint                       || exit /b
call :runtest         lintplus   --lint                       || exit /b
call :runtest_in      lintold    --lint-old                   || exit /b
call :runtest         lintset                                 || exit /b
call :runtest_fail    lintwarn   --lint                       || exit /b

if exist junk call :exec rd /q /s junk                        || exit /b
call :exec md junk                                            || exit /b
call :execq "%QGAWK% ""BEGIN { for ^(i = 1; i ^<= 1030; i++^) print i, i}"" >_manyfiles" || exit /b
call :execq "%QGAWK% -f manyfiles.awk _manyfiles _manyfiles"                             || exit /b
call :execq "(wc.bat ""junk\*"") | %QGAWK% ""$1 != 2"" | (wc.bat > _manyfiles)"          || exit /b
call :cmpdel manyfiles                                        || exit /b
call :exec rd /q /s junk                                      || exit /b

call :runtest         match1                                  || exit /b
call :runtest_fail    match2                                  || exit /b
call :runtest_in      match3                                  || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         mbstr1                                  || goto :exit_local
call :runtest_in      mbstr2                                  || goto :exit_local
endlocal

call :runtest_fail_   mixed1 -f NUL --source """BEGIN {return junk}""" || exit /b
call :runtest_in      mktime                                  || exit /b
call :runtest_fail    muldimposix --posix                     || exit /b
call :runtest_fail    nastyparm                               || exit /b

setlocal
call :exec set TZ=GMT                                         || goto :exit_local
call :execq "%QGAWK% -f negtime.awk > _negtime 2>&1"          || goto :exit_local
call :execq "%QGAWK% -f checknegtime.awk negtime.ok _negtime" || goto :exit_local
call :exec del /q _negtime                                    || goto :exit_local
endlocal

call :execq "(next.bat %QGAWK%) > _next 2>&1" && ^
call :cmpdel next                                             || exit /b

call :runtest         nondec                                  || exit /b
call :runtest         nondec2 --non-decimal-data              || exit /b

call :execq "%QGAWK% -f nonfatal1.awk 2>&1 | %QGAWK% ""{print gensub^(/invalid[:].*$/, """"invalid"""", 1, $0^)}"" >_nonfatal1" && ^
call :cmpdel nonfatal1                                        || exit /b

call :runtest         nonfatal2                               || exit /b
call :runtest         nonfatal3                               || exit /b
call :runtest_        nsawk1a -f nsawk1.awk                   || exit /b
call :runtest_        nsawk1b -v "I=fine" -f nsawk1.awk       || exit /b
call :runtest_        nsawk1c -v "awk::I=fine" -f nsawk1.awk  || exit /b
call :runtest_        nsawk2a -v "I=fine" -f nsawk2.awk       || exit /b
call :runtest_        nsawk2b -v "awk::I=fine" -f nsawk2.awk  || exit /b
call :runtest_fail    nsbad                                   || exit /b
call :runtest_fail_   nsbad_cmd  -v "foo:bar=3" -v "foo:::blat=4" 1 /dev/null || exit /b
call :runtest         nsforloop                               || exit /b
call :runtest         nsfuncrecurse                           || exit /b
call :runtest         nsindirect1                             || exit /b
call :runtest         nsindirect2                             || exit /b

call :execq "%QGAWK% -f nsprof1.awk --pretty-print=_nsprof1" && ^
call :cmpdel nsprof1                                          || exit /b

call :execq "%QGAWK% -f nsprof2.awk --pretty-print=_nsprof2" && ^
call :cmpdel nsprof2                                          || exit /b

call :runtest         patsplit                                || exit /b
call :runtest_in      posix                                   || exit /b
call :runtest_fail    printfbad1                              || exit /b
call :runtest         printfbad2 --lint printfbad2.in         || exit /b
call :runtest         printfbad3                              || exit /b
call :runtest_fail    printfbad4                              || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         printhuge                               || goto :exit_local
endlocal

call :runtest         procinfs                                || exit /b

call :execq "%QGAWK% --profile=ap-profile0.out -f profile0.awk profile0.in > NUL" || exit /b
call :execq "more +2 ap-profile0.out > _profile0 && del /q ap-profile0.out"       || exit /b
call :cmpdel profile0                                                             || exit /b

call :execq "%QGAWK% -f xref.awk dtdgport.awk > _profile1.out1"        || exit /b
call :execq "%QGAWK% --pretty-print=ap-profile1.out -f xref.awk"       || exit /b
call :execq "%QGAWK% -f ap-profile1.out dtdgport.awk > _profile1.out2" || exit /b
call :cmpdel_ _profile1.out1 _profile1.out2                            || exit /b
call :exec del /q ap-profile1.out _profile1.out1                       || exit /b

call :execq "%QGAWK% --profile=ap-profile2.out -v ""sortcmd=sort"" -f xref.awk dtdgport.awk > NUL" || exit /b
call :execq "more +2 ap-profile2.out > _profile2 && del /q ap-profile2.out"                        || exit /b
call :cmpdel profile2                                                                              || exit /b

call :execq "%QGAWK% --profile=ap-profile3.out -f profile3.awk > NUL"       || exit /b
call :execq "more +2 ap-profile3.out > _profile3 && del /q ap-profile3.out" || exit /b
call :cmpdel profile3                                                       || exit /b

call :execq "%QGAWK% -f profile4.awk --pretty-print=_profile4" || exit /b
call :cmpdel profile4                                          || exit /b

call :execq "%QGAWK% -f profile5.awk --pretty=_profile5.out 2> _profile5.err" || exit /b
call :execq "copy /b _profile5.out + _profile5.err _profile5 >NUL"            || exit /b
call :cmpdel profile5 /t && call :exec del /q _profile5.out _profile5.err     || exit /b

call :execq "%QGAWK% --profile=ap-profile6.out -f profile6.awk > NUL"       || exit /b
call :execq "more +2 ap-profile6.out > _profile6 && del /q ap-profile6.out" || exit /b
call :cmpdel profile6                                                       || exit /b

call :execq "%QGAWK% --profile=ap-profile7.out -f profile7.awk > NUL"       || exit /b
call :execq "more +2 ap-profile7.out > _profile7 && del /q ap-profile7.out" || exit /b
call :cmpdel profile7                                                       || exit /b

call :execq "%QGAWK% -f profile8.awk --pretty-print=_profile8" || exit /b
call :cmpdel profile8                                          || exit /b

call :execq "%QGAWK% -f profile9.awk --pretty-print=_profile9" || exit /b
call :cmpdel profile9                                          || exit /b

call :execq "%QGAWK% -f profile10.awk --pretty-print=_profile10" || exit /b
call :cmpdel profile10                                           || exit /b

call :execq "%QGAWK% -f profile11.awk --pretty-print=_profile11" || exit /b
call :cmpdel profile11                                           || exit /b

call :runtest         profile12 "--profile=ap-profile12.out" profile12.in || exit /b
call :exec del /q ap-profile12.out                                        || exit /b

call :execq "%QGAWK% -f profile13.awk --pretty-print=_profile13.out 2> _profile13.err" || exit /b
call :execq "copy /b _profile13.out + _profile13.err _profile13 >NUL"                  || exit /b
call :cmpdel profile13 && call :exec del /q _profile13.out _profile13.err              || exit /b

call :execq "%QGAWK% -f profile14.awk --pretty-print=_profile14" || exit /b
call :cmpdel profile14                                           || exit /b

call :execq "%QGAWK% -f profile15.awk --pretty-print=_profile15" || exit /b
call :cmpdel profile15                                           || exit /b

:: UNSUPPORTED tests: pseudo-terminal api is not supported yet on Windows
::call :runtest         pty1                                    || exit /b
::call :runtest         pty2                                    || exit /b
set /A TESTS_UNSUPPORTED+=2 & >&2 echo *** Unsupported 2 tests: pty1 pty2

setlocal
call :exec set AWKBUFSIZE=4096                                || goto :exit_local
call :runtest_in        rebuf                                 || goto :exit_local
endlocal

call :runtest           regnul1                               || exit /b
call :runtest           regnul2                               || exit /b
call :runtest           regx8bit                              || exit /b
call :runtest           reginttrad --traditional -r           || exit /b
call :runtest_in        reint --re-interval                   || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        reint2 --re-interval                  || goto :exit_local
endlocal

call :runtest_in        rsgetline                             || exit /b

call :execq "type rsgetline.in | %QGAWK% -f rsgetline.awk > _rsglstdin 2>&1" && ^
call :cmpdel rsglstdin                                        || exit /b

call :runtest_in        rsstart1                              || exit /b
call :runtest_in        rsstart2                              || exit /b

call :execq "type rsstart1.in | %QGAWK% -f rsstart2.awk > _rsstart3" && ^
call :cmpdel rsstart3                                         || exit /b

call :runtest_in        rstest6                               || exit /b
call :runtest_fail      sandbox1 --sandbox                    || exit /b
call :runtest           shadow --lint                         || exit /b
call :runtest           shadowbuiltin                         || exit /b
call :runtest_in        sortfor                               || exit /b
call :runtest_in        sortfor2                              || exit /b
call :runtest           sortu                                 || exit /b

call :execq_fail "%QGAWK% --source=""BEGIN { a = 5;"" --source=""print a }"" > _sourcesplit 2>&1" _sourcesplit && ^
call :cmpdel sourcesplit                                      || exit /b

call :runtest_in        split_after_fpat                      || exit /b
call :runtest_in        splitarg4                             || exit /b
call :runtest_in        strftfld                              || exit /b

:: check if we have enough privileges to run wmic
<NUL 2>&1 1>NUL wmic path Win32_UTCTime /? | "%FIND%" /v "" > NUL && (
  set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: strftime - not enough privileges to run wmic
  goto :skip_strftime
)
setlocal
call :change_locale C                                                             || goto :exit_local
call :exec set TZ=GMT0                                                            || goto :exit_local
call :execq "%QGAWK% -v OUTPUT=_strftime -v DATECMD=gmt_time.bat -f strftime.awk" || goto :exit_local
call :cmpdel strftime && call :exec del /q strftime.ok                            || goto :exit_local
endlocal
:skip_strftime

call :runtest           strtonum                              || exit /b
call :runtest           strtonum1                             || exit /b
call :runtest           stupid1                               || exit /b
call :runtest           stupid2                               || exit /b
call :runtest           stupid3                               || exit /b
call :runtest           stupid4                               || exit /b
call :runtest           switch2                               || exit /b
call :runtest           symtab1                               || exit /b
call :runtest           symtab2                               || exit /b
call :runtest_fail      symtab3                               || exit /b
call :runtest_in        symtab4                               || exit /b
call :runtest_in        symtab5                               || exit /b
call :runtest_fail      symtab6                               || exit /b
call :runtest_fail_in   symtab7                               || exit /b

call :execq "%QGAWK% -d__symtab8 -f symtab8.awk symtab8.in > _symtab8" || exit /b
call :execq "< __symtab8 ""%FIND%"" /v ""ENVIRON"" | ""%FIND%"" /v ""PROCINFO"" | ""%FIND%"" /v ""FILENAME"" >> _symtab8" || exit /b
call :cmpdel symtab8 && call :exec del /q __symtab8           || exit /b

call :runtest           symtab9                               || exit /b
call :exec del /q testit.txt                                  || exit /b

call :runtest_fail_in   symtab10 --debug                      || exit /b
call :runtest           symtab11                              || exit /b

:: UNSUPPORTED test: reading files with timeout is not supported yet on windows
::call :runtest           timeout                               || exit /b
set /A TESTS_UNSUPPORTED+=1 & >&2 echo *** Unsupported 1 test: timeout

call :runtest           typedregex1                           || exit /b
call :runtest           typedregex2                           || exit /b
call :runtest           typedregex3                           || exit /b
call :runtest           typedregex4 -v "x=@/foo/" "y=@/bar/" /dev/null || exit /b
call :runtest_in        typedregex5                           || exit /b
call :runtest_in        typedregex6                           || exit /b
call :runtest           typeof1                               || exit /b
call :runtest           typeof2                               || exit /b
call :runtest_fail      typeof3                               || exit /b
call :runtest           typeof4                               || exit /b
call :runtest_in        typeof5                               || exit /b
call :runtest           watchpoint1 -D watchpoint1.in "< watchpoint1.script" || exit /b

exit /b 0

:::::: MACHINE_TESTS (3 tests) :::::
:machine_tests

call :runtest           double1                               || exit /b
call :runtest           double2                               || exit /b
call :runtest           intformat                             || exit /b

exit /b 0

:::::: LOCALE_CHARSET_TESTS (22 tests) :::::
:charset_tests

call :runtest           asort                                 || exit /b
call :runtest           asorti                                || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        backbigs1                             || goto :exit_local
call :runtest_in        backsmalls1                           || goto :exit_local
call :runtest           backsmalls2                           || goto :exit_local
endlocal

call :runtest           fmttest                               || exit /b
call :runtest_mpfr      fnarydel                              || exit /b
call :runtest_mpfr      fnparydl                              || exit /b

%GAWK% "--locale=JAPANESE_japan.20932" --version > NUL 2>&1 || (
  set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: jarebug - JAPANESE_japan.20932 locale is not supported by the OS
  goto :skip_jp_test
)
setlocal
call :change_locale "JAPANESE_japan.20932"                    || goto :exit_local
call :runtest           jarebug jarebug.in                    || goto :exit_local
endlocal
:skip_jp_test

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest           lc_num1                               || goto :exit_local
call :runtest_in        mbfw1                                 || goto :exit_local
call :runtest_in        mbprintf1                             || goto :exit_local
endlocal

%GAWK% "--locale=ja_JP.UTF-8" --version > NUL 2>&1 || (
  set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: mbprintf2 - ja_JP.UTF-8 locale is not supported by the OS
  goto :skip_ja_jp_test
)
setlocal
call :change_locale "ja_JP.UTF-8"                             || goto :exit_local
call :runtest           mbprintf2                             || goto :exit_local
endlocal
:skip_ja_jp_test

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        mbprintf3                             || goto :exit_local
call :runtest_in        mbprintf4                             || goto :exit_local
call :runtest           mbprintf5 mbprintf5.in                || goto :exit_local
endlocal

setlocal
call :change_locale "ru_RU.UTF-8"                             || goto :exit_local
call :runtest_in        mtchi18n                              || goto :exit_local
endlocal

setlocal
call :change_locale "fr_FR.UTF-8"                             || goto :exit_local
call :execq "%QGAWK% -f nlstringtest.awk > _nlstringtest 2>&1" && ^
call :cmpdel_ nlstringtest-nogettext.ok _nlstringtest         || goto :exit_local
endlocal

call :runtest           rebt8b2                               || exit /b

setlocal
call :change_locale "en_US.UTF-8"                             || goto :exit_local
set "RTLENCMD=%QGAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND=%QGAWK% ""BEGIN {printf """"0\n\n\n1\n\n\n\n\n2\n\n""""; exit}"""
call :execq "%COMMAND% | %RTLENCMD% > _rtlenmb" && call :cmpdel rtlenmb || goto :exit_local
endlocal

call :runtest           sort1                                 || exit /b
call :runtest_in        sprintfc                              || exit /b

exit /b 0

:::::: SHLIB_TESTS (23 of 25 upstream tests - 2 tests are not supported, + 11 additional tests) :::::
:shlib_tests

call :execq "%QGAWK% --version | %QGAWK% ""/API/ { exit 1 }""" && (
  set /A TESTS_SKIPPED+=34 & >&2 echo *** Skipped 34 tests: shlib tests not supported on this system
  exit /b 0
)

call :runtest_in        apiterm                               || exit /b

setlocal
call :get_abs_dir ".." TOP
call :runtest           filefuncs -v "builddir=""%TOP:\=/%""" """%BLD_DIST%\filefuncs.dll""" || goto :exit_local
endlocal

call :runtest           fnmatch                               || exit /b

:: UNSUPPORTED tests: fork is not available under Windows yet
::call :runtest           fork                                  || exit /b
::call :runtest           fork2                                 || exit /b
set /A TESTS_UNSUPPORTED+=2 & >&2 echo *** Unsupported 2 tests: fork fork2

call :execq "%QGAWK% -f fts.awk" && ^
call :cmpdel fts && call :exec del /q fts.ok                  || exit /b

call :runtest           functab4                              || exit /b

call :execq "%QGAWK% -f getfile.awk -v ""TESTEXT_QUIET=1"" -ltestext < getfile.awk > _getfile 2>&1" || exit /b
call :execq "< _getfile   ""%FIND%"" /v ""get_file:""  > ap-getfile.out" || exit /b
call :execq "< _getfile   ""%FIND%""    ""get_file:"" >> ap-getfile.out" || exit /b
call :execq "< getfile.ok ""%FIND%"" /v ""get_file:""  > getfile.ok1"    || exit /b
call :execq "< getfile.ok ""%FIND%""    ""get_file:"" >> getfile.ok1"    || exit /b
call :cmpdel_ getfile.ok1 ap-getfile.out /t && call :exec del /q _getfile getfile.ok1 || exit /b

setlocal
call :execq "copy /b /y inplace.1.in _inplace1.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace1.2 > NUL"       || goto :exit_local
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%QGAWK% -i inplace %SCRIPT% _inplace1.1 - _inplace1.2 < inplace.in > _inplace1 2>&1" || goto :exit_local
call :cmpdel inplace1 && ^
call :cmpdel inplace1.1 && ^
call :cmpdel inplace1.2 || goto :exit_local
endlocal

setlocal
call :execq "copy /b /y inplace.1.in _inplace2.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace2.2 > NUL"       || goto :exit_local
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%QGAWK% -i inplace -v inplace::suffix=.bak %SCRIPT% _inplace2.1 - _inplace2.2 < inplace.in > _inplace2 2>&1" || goto :exit_local
call :cmpdel inplace2 && ^
call :cmpdel inplace2.1 && ^
call :cmpdel inplace2.2 && ^
call :cmpdel inplace2.1.bak && ^
call :cmpdel inplace2.2.bak || goto :exit_local
endlocal

setlocal
call :execq "copy /b /y inplace.1.in _inplace2bcomp.1 > NUL"  || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace2bcomp.2 > NUL"  || goto :exit_local
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%QGAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT% _inplace2bcomp.1 - _inplace2bcomp.2 < inplace.in > _inplace2bcomp 2>&1" || goto :exit_local
call :cmpdel inplace2bcomp && ^
call :cmpdel inplace2bcomp.1 && ^
call :cmpdel inplace2bcomp.2 && ^
call :cmpdel inplace2bcomp.1.orig && ^
call :cmpdel inplace2bcomp.2.orig || goto :exit_local
endlocal

setlocal
call :execq "copy /b /y inplace.1.in _inplace3.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace3.2 > NUL"       || goto :exit_local
set "SCRIPT1=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
set "SCRIPT2=""BEGIN {print """"Before""""} {gsub^(/bar/, """"foo""""^); print} END {print """"After""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%QGAWK% -i inplace -v inplace::suffix=.bak %SCRIPT1% _inplace3.1 - _inplace3.2 < inplace.in > _inplace3 2>&1"  || goto :exit_local
call :execq "%QGAWK% -i inplace -v inplace::suffix=.bak %SCRIPT2% _inplace3.1 - _inplace3.2 < inplace.in >> _inplace3 2>&1" || goto :exit_local
call :cmpdel inplace3 && ^
call :cmpdel inplace3.1 && ^
call :cmpdel inplace3.2 && ^
call :cmpdel inplace3.1.bak && ^
call :cmpdel inplace3.2.bak || goto :exit_local
endlocal

setlocal
call :execq "copy /b /y inplace.1.in _inplace3bcomp.1 > NUL"  || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace3bcomp.2 > NUL"  || goto :exit_local
set "SCRIPT1=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
set "SCRIPT2=""BEGIN {print """"Before""""} {gsub^(/bar/, """"foo""""^); print} END {print """"After""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%QGAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT1% _inplace3bcomp.1 - _inplace3bcomp.2 < inplace.in > _inplace3bcomp 2>&1"  || goto :exit_local
call :execq "%QGAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT2% _inplace3bcomp.1 - _inplace3bcomp.2 < inplace.in >> _inplace3bcomp 2>&1" || goto :exit_local
call :cmpdel inplace3bcomp && ^
call :cmpdel inplace3bcomp.1 && ^
call :cmpdel inplace3bcomp.2 && ^
call :cmpdel inplace3bcomp.1.orig && ^
call :cmpdel inplace3bcomp.2.orig || goto :exit_local
endlocal

call :runtest           ordchr                                || exit /b

call :execq "%QGAWK% --load ordchr ""BEGIN {print chr^(ord^(""""z""""^)^)}"" > _ordchr2 2>&1" && ^
call :cmpdel ordchr2                                          || exit /b

setlocal & set TESTS_PARTIAL=0
call :get_abs_dir ".." TOP
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%TOP%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic file link, ignoring)
call :execq "mklink ""%TOP%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic directory link, ignoring)
call :execq "%QGAWK% -f readdir.awk ""%TOP%"" > _readdir"     || goto :exit_local
call :execq """%BLD_DIST%\helpers\tst_ls"" -afi ""%TOP%"" > _dirlist"   || goto :exit_local
call :execq """%BLD_DIST%\helpers\tst_ls"" -lna ""%TOP%"" > _longlist1" || goto :exit_local
call :execq "more +1 _longlist1 > _longlist"                  || goto :exit_local
call :execq "%QGAWK% -f readdir0.awk -v extout=_readdir -v dirlist=_dirlist -v longlist=_longlist > readdir.ok" || goto :exit_local
call :execq "del /q ""%TOP%\FFF"" 2>NUL"
call :execq "rd /q ""%TOP%\DDD"" 2>NUL"
call :cmpdel readdir && call :exec del /q readdir.ok _dirlist _longlist1 _longlist || goto :exit_local
endlocal & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

setlocal & set TESTS_PARTIAL=0
call :get_abs_dir ".." TOP
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%TOP%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic file link, ignoring)
call :execq "mklink ""%TOP%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic directory link, ignoring)
call :execq "%QGAWK% -lreaddir -F/ ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%TOP%"" > readdir_test.ok" || goto :exit_local
call :execq "%QGAWK% -lreaddir_test ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%TOP%"" > _readdir_test"  || goto :exit_local
call :execq "del /q ""%TOP%\FFF"" 2>NUL"
call :execq "rd /q ""%TOP%\DDD"" 2>NUL"
call :cmpdel readdir_test && call :exec del /q readdir_test.ok || goto :exit_local
endlocal & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

setlocal & set TESTS_PARTIAL=0
call :get_abs_dir ".." TOP
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%TOP%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic file link, ignoring)
call :execq "mklink ""%TOP%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & >&2 echo *** Partial 1 test: can't create symbolic directory link, ignoring)
call :execq "%QGAWK% -lreaddir -F/ -f readdir_retest.awk ""%TOP%"" > readdir_retest.ok"    || goto :exit_local
call :execq "%QGAWK% -lreaddir_test -F/ -f readdir_retest.awk ""%TOP%"" > _readdir_retest" || goto :exit_local
call :execq "del /q ""%TOP%\FFF"" 2>NUL"
call :execq "rd /q ""%TOP%\DDD"" 2>NUL"
call :cmpdel readdir_retest && call :exec del /q readdir_retest.ok || goto :exit_local
endlocal & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

call :execq "%QGAWK% -l readfile ""BEGIN {printf """"%%%%s"""", readfile^(""""Makefile.am""""^)}"" > _readfile 2>&1" && ^
call :cmpdel_ Makefile.am _readfile                           || exit /b

call :runtest           readfile2 readfile2.awk readdir.awk   || exit /b
call :runtest           revout                                || exit /b
call :runtest           revtwoway                             || exit /b
call :runtest_in        rwarray -lrwarray  "-v FORMAT=3.1"    || exit /b

:: + 1 additional test
call :runtest_in        rwarray -lrwarray0 "-v FORMAT=3.0"    || exit /b

setlocal
call :get_abs_dir ".." TOP
call :execq "%QGAWK% "" /^^^(@load^|BEGIN^)/,/^^}/"" ""%TOP:\=/%/extension/testext.c"" > testext.awk" || goto :exit_local
call :runtest_mpfr      testext                               || goto :exit_local
call :exec del /q testext.awk testexttmp.txt                  || goto :exit_local
endlocal

call :runtest           time                                  || exit /b

:: + 10 additional tests (not in upstream)
call :runtest           fnmatch_bugs                          || exit /b
call :runtest           fnmatch_more                          || exit /b
call :runtest           fnmatch_fbsd                          || exit /b
call :runtest           fnmatch_obsd "--locale=C"             || exit /b
call :runtest           fnmatchmb "--locale=en_US.UTF-8"      || exit /b

setlocal
set OK_SUFFIX=_win
call :runtest           fnmatch_glibc                         || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win_utf8
call :exec set GAWKWINLOCALE=UTF8                             || goto :exit_local
call :runtest           fnmatch_glibc "--locale=en_US.UTF-8"  || goto :exit_local
endlocal

setlocal
set OK_SUFFIX=_win
call :runtest           fnmatch_de "--locale=German_Germany.28591" || goto :exit_local
endlocal

call :execq "%QGAWK% -f fnmatch_u8_de.awk --locale=de_DE.UTF-8 > _fnmatch_u8_de" && ^
call :cmpdel_ fnmatch_de_win.ok _fnmatch_u8_de                || exit /b

call :runtest           fnmatch_u8_en "--locale=en_US.UTF-8"  || exit /b

exit /b 0

:::::: EXTRA_TESTS (2 of 3 upstream tests - 1 test is unsupported, + 3 additional tests) :::::
:extra_tests

if not defined GAWK_TEST_ARGS goto :inftest_no_mpfr
if "%GAWK_TEST_ARGS:-M=%%GAWK_TEST_ARGS:--bignum=%"=="%GAWK_TEST_ARGS%%GAWK_TEST_ARGS%" goto :inftest_no_mpfr
call :execq "%QGAWK% -f inftest.awk | %QGAWK% ""{ if ^(""""loop terminated"""" == $0^) exit 0; if ^($1 != $2 """"000""""^) exit 1; }""" || exit /b
goto :after_inftest
:inftest_no_mpfr
call :execq "%QGAWK% -f inftest.awk > _inftest"               || exit /b
call :execq "more inftest.ok > _inftest.ok"                   || exit /b
call :cmpdel_ _inftest.ok _inftest /c && call :exec del _inftest.ok || exit /b
:after_inftest

:: regtest: reg/exp-eq.awk, reg/func.awk, reg/func2.awk

call :execq "%QGAWK% -f reg\exp-eq.awk < reg\exp-eq.in > reg\_exp-eq 2>&1" && ^
call :cmpdel_ reg\exp-eq.good reg\_exp-eq                     || exit /b

:: convert forward slashes to backward ones
call :execq "(for /f ""tokens=1* delims=/"" %%%%a in (reg\func.good) do (echo %%%%a\%%%%b)) > reg\_func.good" || exit /b
call :execq "%QGAWK% -f reg\func.awk < reg\func.in > reg\_func 2>&1 && cmd /c ""exit /b 1"" || cmd /c ""exit /b 0""" && ^
call :cmpdel_ reg\_func.good reg\_func && call :exec del reg\_func.good || exit /b

:: convert forward slashes to backward ones
call :execq "cmd /s /c ""for /f ""tokens=1* delims=/"" %%%%a in ^(reg\func2.good^) do @^(^(echo %%%%a\%%%%b^)^&exit /b^)"" > reg\_func2.good" || exit /b
call :execq "more +1 reg\func2.good >> reg\_func2.good" || exit /b
call :execq "%QGAWK% -f reg\func2.awk < reg\func2.in > reg\_func2 2>&1 && cmd /c ""exit /b 1"" || cmd /c ""exit /b 0""" && ^
call :cmpdel_ reg\_func2.good reg\_func2 && call :exec del reg\_func2.good || exit /b

:: UNSUPPORTED
:: this test fails under Windows - lower case greek letter 0xf2 (Greek_Greece.28597),
:: when converting to upper case, do not maps to 0xd3 (Greek_Greece.28597)
::setlocal
::call :change_locale "Greek_Greece.28597"                      || goto :exit_local
::call :runtest ignrcas3                                        || goto :exit_local
::endlocal
set /A TESTS_UNSUPPORTED+=1 & >&2 echo *** Unsupported 1 test: ignrcas3

:: + 3 additional tests (not in upstream)
call :runtest           unicode-non-bmp1 "--locale=ru_RU.UTF-8" "<" unicode-non-bmp.txt || exit /b
call :runtest           unicode-non-bmp2 "--locale=ru_RU.UTF-8" "<" unicode-non-bmp.txt || exit /b
call :runtest           unicode-non-bmp3 "--locale=ru_RU.UTF-8" "<" unicode-non-bmp.txt || exit /b

exit /b

:::::: INET_TESTS (3*4 network tests) :::::
:inet_tests

call :inet_test  9 DISCARD inetdis || exit /b
call :inet_test 13 DAYTIME inetday || exit /b
call :inet_test  7 DAYTIME inetech || exit /b

exit /b 0

:inet_test
:: %1 - 7, 9, 13 (port)
:: %2 - ECHO, DISCARD, DAYTIME
:: %3 - inetdis, inetday, inetech
call :checksvc udp 4 "0.0.0.0" %1 %2 && (call :%3 udp "127.0.0.1" %1   || exit /b)
call :checksvc udp 6 "[::]"    %1 %2 && (call :%3 udp "[::1]"     %1 6 || exit /b)
call :checksvc tcp 4 "0.0.0.0" %1 %2 && (call :%3 tcp "127.0.0.1" %1   || exit /b)
call :checksvc tcp 6 "[::]"    %1 %2 && (call :%3 tcp "[::1]"     %1 6 || exit /b)
exit /b 0

:inetdis
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - 9
:: %4 - <empty> or 6
call :execq "%QGAWK% ""BEGIN { print """"%1:%~2"""" ^|^& """"/inet%4/%1/0/%~2/%3""""}"" > _inetdis 2>&1" || exit /b
call :execq "rem.>inetdis.ok" && call :cmpdel inetdis && call :exec del /q inetdis.ok || exit /b
exit /b

:inetday
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - 13
:: %4 - <empty> or 6
call :execq "%QGAWK% ""BEGIN { print """""""" ^|^& """"/inet%4/%1/0/%~2/%3""""; """"/inet%4/%1/0/%~2/%3"""" ^|^& getline; print $0; if ^(PROCINFO[""""errno""""] ^> 0^) print """"errno="""" PROCINFO[""""errno""""]}"" > _inetday" || exit /b
:: test may faile under heavy system load - retry
call :execq """%FIND%"" ""errno=10053"" < _inetday" && (echo retrying...) && goto :inetday
call :execq """%FIND%"" ""errno=10054"" < _inetday" && (echo retrying...) && goto :inetday
call :execq """%FIND%"" /v """" < _inetday" && call :exec del /q _inetday || exit /b
exit /b

:inetech
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - 7
:: %4 - <empty> or 6
call :execq "%QGAWK% ""BEGIN { print """"[%1 %~2 %3]"""" ^|^& """"/inet%4/%1/0/%~2/%3""""; """"/inet%4/%1/0/%~2/%3"""" ^|^& getline; print $0}"" > _inetech" || exit /b
call :execq "(echo.[%1 %~2 %3]) > inetech.ok" && call :cmpdel inetech && call :exec del /q inetech.ok || exit /b
exit /b

:checksvc
:: %1 - udp, tcp
:: %2 - 4 or 6 (IPv4/IPv6)
:: %3 - "0.0.0.0" or "[::]" (address)
:: %4 - 7, 9, 13 (port)
:: %5 - ECHO, DISCARD, DAYTIME
(for /f "tokens=1,2" %%a in ('netstat -na') do (echo [%%a:%%b])) | "%FIND%" /i "[%1:%~3:%4]" >NUL && exit /b
set /A TESTS_SKIPPED+=1 & >&2 echo *** Skipped 1 test: no %1 %5 service on local IPv%2 port %4, skipping test
exit /b 1

:::::: NEED_MPFR (20 tests) :::::
:mpfr_tests

%GAWK% --version | %GAWK% " /MPFR/ { exit 1 }" && (
  set /A TESTS_SKIPPED+=20 & >&2 echo *** Skipped 20 tests: gawk was built without MPFR support
  exit /b 0
)

call :runtest           mpfrbigint   -M                       || exit /b
call :runtest_in        mpfrbigint2  -M --non-decimal-data    || exit /b
call :runtest_in        mpfrcase     -M                       || exit /b
call :runtest_in        mpfrcase2    -M                       || exit /b
call :runtest           mpfrexprange -M "-vPREC=53"           || exit /b
call :runtest_in        mpfrfield    -M                       || exit /b
call :runtest           mpfrieee     -M "-vPREC=double"       || exit /b

call :execq "%QGAWK% -M -p- -f mpfrmemok1.awk > __mpfrmemok1" && ^
call :execq "more +1 __mpfrmemok1 > _mpfrmemok1" && ^
call :cmpdel mpfrmemok1 && call :exec del /q __mpfrmemok1     || exit /b

call :runtest           mpfrnegzero  -M                       || exit /b
call :runtest_in        mpfrnonum    -M                       || exit /b
call :runtest_in        mpfrnr       -M                       || exit /b
call :runtest           mpfrrem      -M                       || exit /b
call :runtest           mpfrrnd      -M "-vPREC=53"           || exit /b
call :runtest           mpfrrndeval  -M                       || exit /b
call :runtest           mpfrsort     -M "-vPREC=53"           || exit /b
call :runtest           mpfrsqrt     -M                       || exit /b
call :runtest           mpfrstrtonum -M                       || exit /b
call :runtest           mpgforcenum  -M                       || exit /b
call :runtest_          mpfruplus    -M -f "uplus.awk"        || exit /b
call :runtest           mpfranswer42 -M                       || exit /b

exit /b 0

::::::::::::::::: support routines :::::::::::::

:runtest_in
call :runtest      %1 %2 %3 "<" %1.in
exit /b

:runtest_fail_in
call :runtest_fail %1 %2 %3 "<" %1.in
exit /b

:runtest_ok_in
call :runtest_ok   %1 %2 %3 "<" %1.in
exit /b

:runtest
:: run test %1
:: if called from runtest_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_      %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_fail
:: run test %1, then append "EXIT CODE: 2" to _%1, then compare _%1 with %1.ok
:: if called from runtest_fail_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_fail_ %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_ok
:: run test %1, then append "EXIT CODE: 0" to _%1, then compare _%1 with %1.ok
:: if called from runtest_ok_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_ok_   %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_
:: run gawk with parameters, then compare _%1 with %1.ok
:: if called from runtest_in -> runtest
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
setlocal
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%QGAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq "%COMMAND:^^=^%" && call :cmpdel %1
endlocal & exit /b

:runtest_fail_
:: run gawk with parameters, then append "EXIT CODE: 2" to _%1, then compare _%1 with %1.ok
:: if called from runtest_fail_in -> runtest_fail
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
:: awk should fail
setlocal
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%QGAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq_fail "%COMMAND:^^=^%" "_%1" && call :cmpdel %1
endlocal & exit /b

:runtest_ok_
:: run gawk with parameters, then append "EXIT CODE: 0" to _%1, then compare _%1 with %1.ok
:: if called from runtest_ok_in -> runtest_ok
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
:: awk should NOT fail
setlocal
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%QGAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq_ok "%COMMAND:^^=^%" "_%1" && call :cmpdel %1
endlocal & exit /b

:cmpdel
:: compare %1.ok with _%1 -> if equal, delete _%1
:: %2 - optional flags
call :cmpdel_ %1%OK_SUFFIX%.ok _%1 %2
exit /b

:cmpdel_
:: compare %1 with %2 -> if equal, delete %2
:: %3 - optional flags
call :execq "fc %3 %1 %2 > NUL && del /q %2" || (fc /n %3 %1 %2 & exit /b 1)
exit /b

:exec
:: simple command, without quotes, redirections, etc.
echo %*
%*
exit /b

:execq
:: %1 - complex command in double-quotes, rules:
:: 1) double-quote must be escaped by two double-quotes,
:: 2) special symbols inside quoted string must be escaped by ^ (and ^ must not be the last character on a line),
:: 3) special symbols outside quoted string should not be escaped;
::  (escaping symbols inside quoted string is needed to mark symbols that do not need escaping - this is
::   used for echoing the command text in the form ready to copy-paste and execute from the command line)
:: Example:
:: call :execq "%QGAWK% ""BEGIN { for ^(i = 1; i ^<= 1030; i++^) print i, i}"" >_manyfiles"
 
set "COMMAND=%~1"
:: escape special symbols for ECHO
set "COMMAND=%COMMAND:>=^>%"
set "COMMAND=%COMMAND:<=^<%"
set "COMMAND=%COMMAND:&=^&%"
set "COMMAND=%COMMAND:|=^|%"
set "COMMAND=%COMMAND:(=^(%"
set "COMMAND=%COMMAND:)=^)%"
:: escaped symbols inside quoted string do not need to be escaped for ECHO
set "COMMAND=%COMMAND:^^^<=<%"
set "COMMAND=%COMMAND:^^^>=>%"
set "COMMAND=%COMMAND:^^^&=&%"
set "COMMAND=%COMMAND:^^^|=|%"
set "COMMAND=%COMMAND:^^^(=(%"
set "COMMAND=%COMMAND:^^^)=)%"
set "COMMAND=%COMMAND:^^^^=^%"
:: echo the command text, replacing "" with "
echo %COMMAND:""="%
:: re-read command for running
set "COMMAND=%~1"
:: unescape symbols inside double-quotes
set "COMMAND=%COMMAND:^^<=<%"
set "COMMAND=%COMMAND:^^>=>%"
set "COMMAND=%COMMAND:^^&=&%"
set "COMMAND=%COMMAND:^^|=|%"
set "COMMAND=%COMMAND:^^(=(%"
set "COMMAND=%COMMAND:^^)=)%"
set "COMMAND=%COMMAND:^^^^=^%"
:: run the command text, replacing "" with "
%COMMAND:""="%
exit /b

:execq_fail
:: %1 - complex command in double-quotes that should fail
:: %2 - file to append exit code to
call :execq "%~1" && exit /b 1
:: reset ERRORLEVEL
setlocal & set "err1=%ERRORLEVEL%"
cmd /c exit /b 0
call :execq "(echo.EXIT CODE: %err1%) >> ""%~2"""
endlocal & exit /b

:execq_ok
:: %1 - complex command in double-quotes that should not fail
:: %2 - file to append exit code to
call :execq "%~1" || exit /b
call :execq "(echo.EXIT CODE: %ERRORLEVEL%) >> ""%~2"""
exit /b

:waitfor
:: wait for file "%~1"
setlocal & set timeout=0
:waitfor1
if exist "%~1" exit /b
if %timeout% GEQ 60 (
  echo file "%~1" wasn't created in %timeout% seconds
  exit /b 1
)
ping -n 2 127.0.0.1 > NUL
set /A timeout+=1
goto :waitfor1

:change_locale
if defined LANGUAGE call :exec set LANGUAGE=
if not defined GAWKLOCALE (call :execq "set ""LC_ALL=%~1""") else (call :execq "set ""LC_ALL=%GAWKLOCALE%""")
if not defined GAWKLOCALE (call :execq "set ""LANG=%~1""") else (call :execq "set ""LANG=%GAWKLOCALE%""")
exit /b

:exit_local
:: exit local scope
endlocal & exit /b

:get_abs_dir
:: get absoulte path to directory
:: %1 - (relative) path to change current directory to
:: %2 - result variable name
pushd "%~1"
for /f "tokens=* delims=" %%a in ("%CD%") do set "%2=%%~a"
popd
exit /b

:runtest_mpfr
:: %1 - test name
:: %2, %3 - arguments
:: check for -M or --bignum in %GAWK_TEST_ARGS%
if not defined GAWK_TEST_ARGS goto :runtest
if "%GAWK_TEST_ARGS:-M=%%GAWK_TEST_ARGS:--bignum=%"=="%GAWK_TEST_ARGS%%GAWK_TEST_ARGS%" goto :runtest
call :runtest_ %1-mpfr -f %1.awk %2 %3
exit /b
