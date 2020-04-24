@echo off
setlocal

:: Supported build targets:
::
:: <no args>    - build everything, then run all tests
:: awk          - build only the GAWK executable
:: ext          - build only extension DLLs
:: test         - run all tests
:: test basic   - run only basic tests
:: test unix    - run only unix tests
:: test ext     - run only extension tests
:: test cpu     - run only machine tests
:: test locale  - run only locale tests
:: test shlib   - run only tests of extension DLLs
:: clean        - cleanup intermediate build results
:: distclean    - cleanup intermediate build results and contents of the "dist" directory

:: If needed, next variables may be redefined in this batch file:
::
:: COMPILE_AS_CXX  - undefine to compile with C compiler,
:: DEBUG_BUILD     - define to build debug versions of gawk.exe and extension DLLs
:: DO_ANALYZE      - define to enable static analysis of the sources during the compilation


set DO_BUILD_GAWK=x
set DO_BUILD_EXT=x
set DO_TEST=x
set DO_TEST_ONLY=

if "test"=="%~1" (
  set DO_BUILD_GAWK=
  set DO_BUILD_EXT=
  set "DO_TEST_ONLY=%~2"
) else (if "awk"=="%~1" (
  set DO_BUILD_EXT=
  set DO_TEST=
) else (if "ext"=="%~1" (
  set DO_BUILD_GAWK=
  set DO_TEST=
) else (
  if "clean"=="%~1" goto :do_clean
  if "distclean"=="%~1" (
    del dist\gawk.exe dist\*.dll dist\*.pdb 2>NUL
:do_clean
    del *.ilk *.pdb *.obj pc\*.obj support\*.obj extension\*.obj *.exp *.lib helpers\*.exe 2>NUL
    exit /b
  )
  if not ""=="%~1" (
    echo ERROR: unknown build target: "%~1"
    exit /b 1
  )
)))

:: compile sources by the C++ compiler
set COMPILE_AS_CXX=x
rem set COMPILE_AS_CXX=

:: do debug build with all debugging options enabled
rem set DEBUG_BUILD=x
set DEBUG_BUILD=

:: enable static analysis of the sources
rem set DO_ANALYZE=x
set DO_ANALYZE=

:: individual debugging options
rem set GAWKDEBUG=x
if defined DEBUG_BUILD (set GAWKDEBUG=x) else (set GAWKDEBUG=)
rem set ARRAYDEBUG=x
if defined DEBUG_BUILD (set ARRAYDEBUG=x) else (set ARRAYDEBUG=)
rem set MEMDEBUG=x
if defined DEBUG_BUILD (set MEMDEBUG=x) else (set MEMDEBUG=)

:: off by default, because is too noisy
set REGEXDEBUG=
rem if defined DEBUG_BUILD (set REGEXDEBUG=x) else (set REGEXDEBUG=)

:: off by default, because is too slow
set MEMDEBUGLEAKS=
rem if defined DEBUG_BUILD (set MEMDEBUGLEAKS=x) else (set MEMDEBUGLEAKS=)

:: disable deprecation warnings
set "NODEPRECATE=/D_CRT_NONSTDC_NO_DEPRECATE /D_CRT_SECURE_NO_DEPRECATE /D_WINSOCK_DEPRECATED_NO_WARNINGS"

rem 4013 - 'function' undefined; assuming extern returning int
rem 4820 - 'bytes' bytes padding added after construct 'member_name'
rem 4131 - uses old-style declarator
rem 5045 - Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
rem 4464 - relative include path contains '..'
set "WARNING_OPTIONS=/Wall /we4013 /wd4820 /wd4131 /wd5045 /wd4464 /Zc:strictStrings /Zc:ternary /Zc:rvalueCast"

if not defined DEBUG_BUILD (
  rem 4711 - function 'function' selected for automatic inline expansion
  rem 4710 - 'function': function not inlined
  set "WARNING_OPTIONS=%WARNING_OPTIONS% /wd4711 /wd4710"
)

set "CMN_DEFINES=/D_REGEX_LARGE_OFFSETS /D__restrict=__restrict /D__restrict_arr= /D_Noreturn=__declspec(noreturn)"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% /analyze"
)

if defined COMPILE_AS_CXX (
  :: compile as c++
  set "CMN_DEFINES=%CMN_DEFINES% /TP"

  :: c++ specific warnings
  rem 4061 - enumerator 'name' in switch of enum 'name' is not explicitly handled by a case label
  rem 4800 - Implicit conversion from 'int' to bool. Possible information loss
  rem 4623 - 'name': default constructor was implicitly defined as deleted
  rem 4626 - 'name': assignment operator was implicitly defined as deleted
  rem 5027 - 'name': move assignment operator was implicitly defined as deleted
  rem 5039 - 'name': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
  rem 4611 - interaction between '_setjmp' and C++ object destruction is non-portable
  rem 4514 - 'name': unreferenced inline function has been removed
  rem 4710 - 'name': function not inlined
  rem 4711 - function 'name' selected for automatic inline expansion
  rem 4582 - 'name': constructor is not implicitly called
  set "WARNING_OPTIONS=%WARNING_OPTIONS% /wd4061 /wd4800 /wd4623 /wd4626 /wd5027 /wd5039 /wd4611 /wd4514 /wd4710 /wd4711 /wd4582"
)

:: gawk-specific defines
set "GAWK_DEFINES=/DGAWK /DEXEC_HOOK /Dgetenv=getenv /DGAWK_STATIC_CRT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=/nologo /c %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Od /Z7 /EHsc /D_DEBUG /MTd"
  set "GAWKLIB=lib /nologo"
  set "GAWKLINK=link /nologo /DEBUG /INCREMENTAL:NO wsetargv.obj"
  set "EXTLINK=link /nologo /DEBUG /INCREMENTAL:NO /DLL /SUBSYSTEM:CONSOLE /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntimed /DEFAULTLIB:libucrtd"
) else (
  :: release options
  set "CMNOPTS=/nologo /c %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Ox /GF /Gy /GS- /GL /EHsc /DNDEBUG /MT"
  set "GAWKLIB=lib /nologo /LTCG"
  set "GAWKLINK=link /nologo /LTCG wsetargv.obj"
  set "EXTLINK=link /nologo /LTCG /DLL /SUBSYSTEM:CONSOLE /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntime /DEFAULTLIB:libucrt"
)

if defined GAWKDEBUG     (set "GAWK_DEFINES=%GAWK_DEFINES% /DGAWKDEBUG")
if defined ARRAYDEBUG    (set "GAWK_DEFINES=%GAWK_DEFINES% /DARRAYDEBUG")
if defined MEMDEBUG      (set "GAWK_DEFINES=%GAWK_DEFINES% /DMEMDEBUG")
if defined REGEXDEBUG    (set "GAWK_DEFINES=%GAWK_DEFINES% /DDEBUG")
if defined MEMDEBUGLEAKS (set "GAWK_DEFINES=%GAWK_DEFINES% /DMEMDEBUGLEAKS")

set "GAWKCC=cl %CMNOPTS% %GAWK_DEFINES% /DHAVE___INLINE /DHAVE_CONFIG_H /DLOCALEDIR=\"\" /DDEFPATH=\".\" /DDEFLIBPATH=\"\" /DSHLIBEXT=\"dll\" /Isupport /Ipc /I."
set "EXTCC=cl %CMNOPTS% /DGAWK_STATIC_CRT /DHAVE_CONFIG_H /Iextension /Ipc /I."

set CALL_STAT=0

if defined DO_BUILD_GAWK (
  if not exist dist call :exec md dist || goto :build_err
  call :support || goto :build_err
  call :gawk    || goto :build_err
)
if defined DO_BUILD_EXT (
  if not exist dist call :exec md dist || goto :build_err
  call :exts    || goto :build_err
)
if defined DO_TEST (
  call :helpers || goto :build_err
  call :tests   || goto :test_err
)

echo CALL_STAT=%CALL_STAT%
echo OK!
exit /b 0

:::::: COMPILATION :::::
:support

call :exec %GAWKCC% /Fosupport\getopt.obj     support\getopt.c     || exit /b
call :exec %GAWKCC% /Fosupport\getopt1.obj    support\getopt1.c    || exit /b
call :exec %GAWKCC% /Fosupport\random.obj     support\random.c     || exit /b
call :exec %GAWKCC% /Fosupport\dfa.obj        support\dfa.c        || exit /b
call :exec %GAWKCC% /Fosupport\localeinfo.obj support\localeinfo.c || exit /b
call :exec %GAWKCC% /Fosupport\regex.obj      support\regex.c      || exit /b

call :exec %GAWKLIB% /OUT:support\libsupport.a ^
support\getopt.obj     ^
support\getopt1.obj    ^
support\random.obj     ^
support\dfa.obj        ^
support\localeinfo.obj ^
support\regex.obj         || exit /b

exit /b 0

:gawk

call :exec %GAWKCC% /Foarray.obj      array.c             || exit /b
call :exec %GAWKCC% /Foawkgram.obj    awkgram.c           || exit /b
call :exec %GAWKCC% /Fobuiltin.obj    builtin.c           || exit /b
call :exec %GAWKCC% /Fopc\xstat.obj   pc\xstat.c          || exit /b
call :exec %GAWKCC% /Fopc\popen.obj   pc\popen.c          || exit /b
call :exec %GAWKCC% /Fopc\getid.obj   pc\getid.c          || exit /b
call :exec %GAWKCC% /Focint_array.obj cint_array.c        || exit /b
call :exec %GAWKCC% /Focommand.obj    command.c           || exit /b
call :exec %GAWKCC% /Fodebug.obj      debug.c             || exit /b
call :exec %GAWKCC% /Foeval.obj       eval.c              || exit /b
call :exec %GAWKCC% /Foext.obj        ext.c               || exit /b
call :exec %GAWKCC% /Fofield.obj      field.c             || exit /b
call :exec %GAWKCC% /Fofloatcomp.obj  floatcomp.c         || exit /b
call :exec %GAWKCC% /Fogawkapi.obj    gawkapi.c           || exit /b
call :exec %GAWKCC% /Fogawkmisc.obj   gawkmisc.c          || exit /b
call :exec %GAWKCC% /Foint_array.obj  int_array.c         || exit /b
call :exec %GAWKCC% /Foio.obj         io.c                || exit /b
call :exec %GAWKCC% /Fomain.obj       main.c              || exit /b
call :exec %GAWKCC% /Fompfr.obj       mpfr.c              || exit /b
call :exec %GAWKCC% /Fomsg.obj        msg.c               || exit /b
call :exec %GAWKCC% /Fonode.obj       node.c              || exit /b
call :exec %GAWKCC% /Foprofile.obj    profile.c           || exit /b
call :exec %GAWKCC% /Fore.obj         re.c                || exit /b
call :exec %GAWKCC% /Foreplace.obj    replace.c           || exit /b
call :exec %GAWKCC% /Fostr_array.obj  str_array.c         || exit /b
call :exec %GAWKCC% /Fosymbol.obj     symbol.c            || exit /b
call :exec %GAWKCC% /Foversion.obj    version.c           || exit /b

call :exec %GAWKLINK% /DEFAULTLIB:WS2_32.lib /OUT:dist\gawk.exe ^
array.obj             ^
awkgram.obj           ^
builtin.obj           ^
pc\xstat.obj          ^
pc\popen.obj          ^
pc\getid.obj          ^
cint_array.obj        ^
command.obj           ^
debug.obj             ^
eval.obj              ^
ext.obj               ^
field.obj             ^
floatcomp.obj         ^
gawkapi.obj           ^
gawkmisc.obj          ^
int_array.obj         ^
io.obj                ^
main.obj              ^
mpfr.obj              ^
msg.obj               ^
node.obj              ^
profile.obj           ^
re.obj                ^
replace.obj           ^
str_array.obj         ^
symbol.obj            ^
version.obj           ^
support\libsupport.a       || exit /b

exit /b 0

:exts

call :ext testext        || exit /b
call :ext readdir        || exit /b
call :ext readdir_test /DREADDIR_TEST readdir || exit /b
call :ext inplace        || exit /b
call :ext time           || exit /b

call :ext_cc stack       || exit /b
call :ext_cc wreaddir    || exit /b
call :ext_cc wreadlink   || exit /b
call :ext_cc gawkfts "/DFTS_ALIGN_BY=sizeof(void*)" || exit /b
call :ext_cc filefuncs   || exit /b
call :ext_ld filefuncs "extension\stack.obj extension\wreaddir.obj extension\wreadlink.obj extension\gawkfts.obj extension\filefuncs.obj" || exit /b

call :ext rwarray        || exit /b
call :ext rwarray0       || exit /b
call :ext revoutput      || exit /b
call :ext revtwoway      || exit /b
call :ext readfile       || exit /b
call :ext ordchr         || exit /b
call :ext intdiv         || exit /b
call :ext fnmatch /Imissing_d     || exit /b

:: not ported yet...
::call :ext fork         || exit /b

exit /b 0

:ext_cc
:: %1 - object file base name
:: %2 - (optional) additional compiler options
:: %3 - (optional) source file base name
if ""=="%~3" (
  call :execq "%EXTCC% %~2 /Foextension\%~1.obj extension\%~1.c"
) else (
  call :execq "%EXTCC% %~2 /Foextension\%~1.obj extension\%~3.c"
)
exit /b

:ext_ld
:: %1 - dll base name
:: %2 - object file(s) (pathnames)
call :exec %EXTLINK% /OUT:"dist\%~1.dll" /IMPLIB:"%~1.lib" %~2
exit /b

:ext
:: %1 - extension name
:: %2 - (optional) additional compiler options
:: %3 - (optional) name of source file
call :ext_cc "%~1" "%~2" "%~3"             || exit /b
call :ext_ld "%~1" "extension\%~1.obj"     || exit /b
exit /b 0

:helpers
call :execq "cl /nologo /Fehelpers\tst_ls.exe helpers\tst_ls.c /MT" || exit /b
exit /b 0

:::::: TESTSUITE :::::
:tests

echo.
echo.===============================================================
echo.Any output from FC is bad news, although some differences
echo.in floating point values are probably benign -- in particular,
echo.some systems may omit a leading zero and the floating point
echo.precision may lead to slightly different output in a few cases.
echo.===============================================================

setlocal & set CALL_STAT=0

:: don't call abort() in gawk.exe - it shows a message box window
call :exec set GAWKTESTING=1 || goto :exit_local

call :change_locale C || goto :exit_local
dist\gawk.exe -f "test\printlang.awk" || ((echo.failed to execute: dist\gawk.exe -f "test\printlang.awk") & goto :exit_local)

call :exec cd test
set err=%ERRORLEVEL%
if not %err% equ 0 goto :test_no_cd
call :tests_in_directory
set err=%ERRORLEVEL%
call :exec cd ..
if %err% equ 0 set err=%ERRORLEVEL%
:test_no_cd

endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b %err%

:tests_in_directory
call :execq "set AWKPATH=." || exit /b
set "GAWK=..\dist\gawk.exe"

if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="basic" goto :try_unix_tests)
echo.======== Starting basic tests ========
call :basic_tests           || exit /b
echo.======== Done with basic tests ========

:try_unix_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="unix" goto :try_ext_tests)
echo.======== Starting Unix tests ========
call :unix_tests            || exit /b
echo.======== Done with Unix tests ========

:try_ext_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="ext" goto :try_cpu_tests)
echo.======== Starting gawk extension tests ========
call :ext_tests             || exit /b
echo.======== Done with gawk extension tests ========

:try_cpu_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="cpu" goto :try_locale_tests)
echo.======== Starting machine-specific tests ========
call :machine_tests         || exit /b
echo.======== Done with machine-specific tests ========

:try_locale_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="locale" goto :try_shlib_tests)
echo.======== Starting tests that can vary based on character set or locale support ========
echo.**************************************************************************
echo.* Some or all of these tests may fail if you have inadequate or missing  *
echo.* locale support. At least en_US.UTF-8, fr_FR.UTF-8, ru_RU.UTF-8 and     *
echo.* ja_JP.UTF-8 are needed. The el_GR.iso88597 is optional but helpful.    *
echo.**************************************************************************
call :charset_tests         || exit /b
echo.======== Done with tests that can vary based on character set or locale support ========

:try_shlib_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="shlib" goto :end_of_tests)
echo.======== Starting shared library tests ========
call :shlib_tests           || exit /b
echo.======== Done with shared library tests ========

:end_of_tests
if defined DO_TEST_ONLY (
  if not "%DO_TEST_ONLY%"=="basic" (
  if not "%DO_TEST_ONLY%"=="unix" (
  if not "%DO_TEST_ONLY%"=="ext" (
  if not "%DO_TEST_ONLY%"=="cpu" (
  if not "%DO_TEST_ONLY%"=="locale" (
  if not "%DO_TEST_ONLY%"=="shlib" (
    echo.ERROR: unknown sub-test name: "%DO_TEST_ONLY%"
    exit /b 1
)))))))

if defined DO_TEST_ONLY (
  echo.%DO_TEST_ONLY% TESTS PASSED
) else (
  echo.ALL TESTS PASSED
)
exit /b

:::::: BASIC TESTS :::::
:basic_tests

call :runtest_in      addcomma                          || exit /b
call :runtest_in      anchgsub                          || exit /b
call :runtest_in      anchor                            || exit /b

call :execq "copy argarray.in argarray.input > NUL"     || exit /b
call :execq "echo just a test | %GAWK% -f argarray.awk ./argarray.input - > _argarray" || exit /b
call :cmpdel argarray                                   || exit /b
call :exec del /q argarray.input

call :runtest_in      arrayind1                         || exit /b
call :runtest         arrayind2                         || exit /b
call :runtest         arrayind3                         || exit /b
call :runtest_fail    arrayparm                         || exit /b
call :runtest         arrayprm2                         || exit /b
call :runtest         arrayprm3                         || exit /b
call :runtest         arrayref                          || exit /b
call :runtest         arrymem1                          || exit /b
call :runtest         arryref2                          || exit /b
call :runtest_fail    arryref3                          || exit /b
call :runtest_fail    arryref4                          || exit /b
call :runtest_fail    arryref5                          || exit /b
call :runtest         arynasty                          || exit /b
call :runtest         arynocls -v "INPUT=arynocls.in"   || exit /b
call :runtest_fail    aryprm1                           || exit /b
call :runtest_fail    aryprm2                           || exit /b
call :runtest_fail    aryprm3                           || exit /b
call :runtest_fail    aryprm4                           || exit /b
call :runtest_fail    aryprm5                           || exit /b
call :runtest_fail    aryprm6                           || exit /b
call :runtest_fail    aryprm7                           || exit /b
call :runtest         aryprm8                           || exit /b
call :runtest         aryprm9                           || exit /b
call :runtest         arysubnm                          || exit /b
call :runtest         aryunasgn                         || exit /b
call :runtest_in      asgext                            || exit /b

setlocal & set CALL_STAT=0
call :exec set AWKPATH=lib                              || goto :exit_local
call :runtest         awkpath                           || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      concat4                                 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest         concat5                                 || exit /b
call :runtest         convfmt                                 || exit /b
call :runtest_in      datanonl                                || exit /b
call :runtest_fail    defref --lint                           || exit /b
call :runtest         delargv                                 || exit /b
call :runtest         delarpm2                                || exit /b
call :runtest         delarprm                                || exit /b
call :runtest_fail    delfunc                                 || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      dfamb1                                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

setlocal & set CALL_STAT=0
call :change_locale C                                         || goto :exit_local
call :runtest         gsubtst6                                || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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
call :runtest_in      litoct  --traditional                   || exit /b
call :runtest_in      longsub                                 || exit /b
call :runtest_in      longwrds -v "SORT=sort"                 || exit /b
call :runtest_in      manglprm                                || exit /b
call :runtest         math                                    || exit /b
call :runtest_in      membug1                                 || exit /b
call :runtest         memleak                                 || exit /b

call :execq "%GAWK% -f messages.awk >_out2 2>_out3" && ^
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

call :execq "<NUL set /p=A B C D E | %GAWK% ""{ print $NF }"" - nors.in > _nors" && ^
call :cmpdel nors                                             || exit /b

call :runtest_fail    nulinsrc                                || exit /b
call :runtest_in      nulrsend                                || exit /b
call :runtest_in      numindex                                || exit /b
call :runtest         numrange                                || exit /b
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
call :runtest         rand                                    || exit /b

setlocal & set CALL_STAT=0
call :exec set GAWKTEST_NO_TRACK_MEM=1                        || goto :exit_local
call :runtest randtest "-vRANDOM=" "-vNSAMPLES=1024" "-vMAX_ALLOWED_SIGMA=5" "-vNRUNS=50" || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      range1                                  || exit /b

setlocal & set CALL_STAT=0
call :change_locale C                                         || goto :exit_local
call :runtest         range2                                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      rri1                                    || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      rs                                      || exit /b
call :runtest_in      rscompat --traditional                  || exit /b
call :runtest_in      rsnul1nl                                || exit /b

setlocal & set CALL_STAT=0
:: Suppose that block size for pipe is at most 128kB:
set "COMMAND=%GAWK% ""BEGIN { for ^(i = 1; i ^<= 128*64+1; i++^) print """"abcdefgh123456\n"""" }"" 2>&1"
set "COMMAND=%COMMAND% | %GAWK% ""BEGIN { RS = """"""""; ORS = """"\n\n"""" }; { print }"" 2>&1"
set "COMMAND=%COMMAND% | %GAWK% "" /^^[^^a]/; END{ print NR }"""
call :execq "%COMMAND% >_rsnulbig" && call :cmpdel rsnulbig   || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set "COMMAND=%GAWK% ""BEGIN { ORS = """"""""; n = """"\n""""; for ^(i = 1; i ^<= 10; i++^) n = ^(n n^); "
set "COMMAND=%COMMAND%for ^(i = 1; i ^<= 128; i++^) print n; print """"abc\n"""" }"" 2>&1"
set "COMMAND=%COMMAND% | %GAWK% ""BEGIN { RS = """"""""; ORS = """"\n\n"""" };{ print }"" 2>&1"
set "COMMAND=%COMMAND% | %GAWK% "" /^^[^^a]/; END { print NR }"""
call :execq "%COMMAND% >_rsnulbig2" && call :cmpdel rsnulbig2 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

setlocal & set CALL_STAT=0
call :exec set LC_ALL=C                                       || goto :exit_local
call :runtest         spacere                                 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      splitargv                               || exit /b
call :runtest         splitarr                                || exit /b
call :runtest         splitdef                                || exit /b
call :runtest_in      splitvar                                || exit /b
call :runtest         splitwht                                || exit /b
call :runtest         status-close                            || exit /b
call :runtest         strcat1                                 || exit /b
call :runtest         strnum1                                 || exit /b
call :runtest         strnum2                                 || exit /b
call :runtest_in      strtod                                  || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      subamp                                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      subback                                 || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         subi18n                                 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in      wideidx                                 || goto :exit_local
call :runtest         wideidx2                                || goto :exit_local
call :runtest         widesub                                 || goto :exit_local
call :runtest         widesub2                                || goto :exit_local
call :runtest_in      widesub3                                || goto :exit_local
call :runtest         widesub4                                || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      wjposer1                                || exit /b
call :runtest         zero2                                   || exit /b
call :runtest         zeroe0                                  || exit /b
call :runtest         zeroflag                                || exit /b

exit /b 0

:::::: UNIX TESTS :::::
:unix_tests

call :execq "(fflush.bat ""%GAWK%"") >_fflush" && call :cmpdel fflush || exit /b
call :runtest         getlnhd                                 || exit /b
call :execq "(localenl.bat ""%GAWK%"")"                       || exit /b

call :execq "del /q winpid.done 2> NUL"
call :execq "wmic process call create CommandLine=""\""%CD%\%GAWK%\"" -f .\winpid.awk"" CurrentDirectory=""%CD%"" 2>&1 | find ""ProcessId"" > _winpid" || exit /b
call :waitfor winpid.done                                     || exit /b
call :cmpdel winpid                                           || exit /b
call :exec del /q winpid.ok winpid.done                       || exit /b

call :runtest pipeio1 && call :exec del /q test1 test2        || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest pipeio2 "-vSRCDIR=."                            || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

:: UNSUPPORTED test: poundbang - cannot be ported to windows

setlocal & set CALL_STAT=0
set "RTLENCMD=%GAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND=%GAWK% ""BEGIN {printf """"0\n\n\n1\n\n\n\n\n2\n\n""""; exit}"""
call :execq "%COMMAND% | %RTLENCMD% > _rtlen" && call :cmpdel rtlen || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set "RTLENCMD=%GAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND="
set "COMMAND=%COMMAND%%GAWK% ""BEGIN {printf """"0""""; exit}"" | %RTLENCMD%"
set "COMMAND=%COMMAND%&%GAWK% ""BEGIN {printf """"0\n""""; exit}"" | %RTLENCMD%"
set "COMMAND=%COMMAND%&%GAWK% ""BEGIN {printf """"0\n\n""""; exit}""| %RTLENCMD%"
call :execq "(%COMMAND%) > _rtlen01" && call :cmpdel rtlen01  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_fail_ space -f """ """ .\space.awk              || exit /b

setlocal & set CALL_STAT=0
call :exec set TZ=UTC                                         || goto :exit_local
call :runtest strftlng                                        || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

exit /b 0

:::::: EXTENSION TESTS :::::
:ext_tests

call :runtest_fail    aadelete1                               || exit /b
call :runtest_fail    aadelete2                               || exit /b
call :runtest         aarray1                                 || exit /b
call :runtest         aasort                                  || exit /b
call :runtest         aasorti                                 || exit /b
call :runtest         argtest -x -y abc                       || exit /b
call :runtest         arraysort                               || exit /b
call :runtest         arraysort2                              || exit /b
call :runtest         arraytype                               || exit /b
call :runtest_in      backw                                   || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :execq "%GAWK% -f 2>&1 | find /v ""patchlevel"" | find /v ""--parsedebug"" > _badargs" && ^
call :cmpdel badargs                                          || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :execq "(echo x&echo y&echo z) > Xfile"                   || goto :exit_local
call :runtest beginfile1 beginfile1.awk . ./no/such/file Xfile || goto :exit_local
call :exec del /q Xfile
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :exec set LC_ALL=C                                       || goto :exit_local
call :exec set AWK=%GAWK%                                     || goto :exit_local
call :execq "(beginfile2.bat) > _beginfile2 2>&1" && ^
call :cmpdel beginfile2                                       || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_ binmode1 -v "BINMODE=3" """BEGIN { print BINMODE }""" || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :change_locale "en_US.UTF-8"                                                          || goto :exit_local
:: BINMODE=2 is needed for PC tests.
call :execq "%GAWK% -b -vBINMODE=2 -f charasbytes.awk charasbytes.in > _charasbytes1"      || goto :exit_local
call :execq "echo 0000000000000000 > _charasbytes2"                                        || goto :exit_local
call :execq "fc /b _charasbytes1 _charasbytes2 | find ""0000"" > _charasbytes"             || goto :exit_local
call :cmpdel charasbytes                                                                   || goto :exit_local
call :exec del /q _charasbytes1 _charasbytes2                                              || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :execq "(for /L %%%%i in (1,1,3) do @%GAWK% -f colonwarn.awk %%%%i < colonwarn.in) > _colonwarn" && ^
call :cmpdel colonwarn                                        || exit /b

setlocal & set CALL_STAT=0
call :change_locale C                                         || goto :exit_local
call :runtest         clos1way                                || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest_fail_in clos1way2                               || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_fail    clos1way3                               || exit /b
call :runtest_fail    clos1way4                               || exit /b
call :runtest_fail    clos1way5                               || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :execq "%GAWK% -f clos1way6.awk > _clos1way6.out 2> _clos1way6.err" && ^
call :execq "copy /b _clos1way6.err + _clos1way6.out _clos1way6 >NUL" || goto :exit_local
call :cmpdel clos1way6 && call :exec del /q _clos1way6.out _clos1way6.err || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

call :runtest_in      dfacheck1                               || exit /b

call :execq "%GAWK% --dump-variables 1 < dumpvars.in > NUL" && ^
call :execq "find /v ""ENVIRON"" < awkvars.out | find /v ""PROCINFO"" > _dumpvars" && ^
call :cmpdel dumpvars && call :exec del /q awkvars.out        || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest         errno errno.in                          || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :execq "(exit.bat %GAWK%) > _exit 2>&1" && ^
call :cmpdel exit                                             || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in      fieldwdth                               || exit /b
call :runtest         forcenum --non-decimal-data             || exit /b
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

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         ignrcas2                                || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest         ignrcas4                                || exit /b
call :runtest_in      ignrcase                                || exit /b

setlocal & set CALL_STAT=0
set "SCRIPT=""BEGIN {print sandwich^(""""a"""", """"b"""", """"c""""^)}"""
call :runtest_        incdupe --lint -i inclib -i inclib.awk "%SCRIPT%" || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_fail_   incdupe2 --lint -f inclib -f inclib.awk    || exit /b
call :runtest_        incdupe3 --lint -f hello -f hello.awk      || exit /b
call :runtest_fail_   incdupe4 --lint -f hello -i hello.awk      || exit /b
call :runtest_fail_   incdupe5 --lint -i hello -f hello.awk      || exit /b
call :runtest_fail_   incdupe6 --lint -i inchello -f hello.awk   || exit /b
call :runtest_fail_   incdupe7 --lint -f hello -i inchello       || exit /b

call :runtest         include                                 || exit /b

setlocal & set CALL_STAT=0
set "SCRIPT=""BEGIN {print sandwich^(""""a"""", """"b"""", """"c""""^)}"""
call :runtest_        include2 --include inclib "%SCRIPT%"    || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest         indirectbuiltin                         || exit /b
call :runtest_in      indirectcall                            || exit /b
call :runtest         indirectcall2                           || exit /b
call :runtest         intarray --non-decimal-data             || exit /b
call :runtest         isarrayunset                            || exit /b
call :runtest         lint                                    || exit /b
call :runtest         lintexp    --lint                       || exit /b
call :runtest         lintindex  --lint                       || exit /b
call :runtest         lintint    --lint                       || exit /b
call :runtest         lintlength --lint                       || exit /b
call :runtest_in      lintold    --lint-old                   || exit /b
call :runtest         lintset                                 || exit /b
call :runtest_fail    lintwarn   --lint                       || exit /b

if exist junk call :exec rd /q /s junk                        || exit /b
call :exec md junk                                            || exit /b
call :execq "%GAWK% ""BEGIN { for ^(i = 1; i ^<= 1030; i++^) print i, i}"" >_manyfiles" || exit /b
call :execq "%GAWK% -f manyfiles.awk _manyfiles _manyfiles"                             || exit /b
call :execq "wc.bat ""junk\*"" | %GAWK% ""$1 != 2"" | wc.bat > _manyfiles"              || exit /b
call :cmpdel manyfiles                                        || exit /b
call :exec rd /q /s junk                                      || exit /b

call :runtest         match1                                  || exit /b
call :runtest_fail    match2                                  || exit /b
call :runtest_in      match3                                  || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         mbstr1                                  || goto :exit_local
call :runtest_in      mbstr2                                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_fail_   mixed1 -f NUL --source """BEGIN {return junk}""" || exit /b
call :runtest_in      mktime                                  || exit /b
call :runtest_fail    muldimposix --posix                     || exit /b
call :runtest_fail    nastyparm                               || exit /b

setlocal & set CALL_STAT=0
call :exec set TZ=GMT                                         || goto :exit_local
call :execq "%GAWK% -f negtime.awk > _negtime 2>&1"           || goto :exit_local
call :execq "%GAWK% -f checknegtime.awk negtime.ok _negtime"  || goto :exit_local
call :exec del /q _negtime                                    || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :execq "(next.bat %GAWK%) > _next 2>&1" && ^
call :cmpdel next                                             || exit /b

call :runtest         nondec                                  || exit /b
call :runtest         nondec2 --non-decimal-data              || exit /b

call :execq "%GAWK% -f nonfatal1.awk 2>&1 | %GAWK% ""{print gensub^(/invalid[:].*$/, """"invalid"""", 1, $0^)}"" >_nonfatal1" && ^
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

call :execq "%GAWK% -f nsprof1.awk --pretty-print=_nsprof1" && ^
call :cmpdel nsprof1                                          || exit /b

call :execq "%GAWK% -f nsprof2.awk --pretty-print=_nsprof2" && ^
call :cmpdel nsprof2                                          || exit /b

call :runtest         patsplit                                || exit /b  
call :runtest_in      posix                                   || exit /b  
call :runtest_fail    printfbad1                              || exit /b  
call :runtest         printfbad2 --lint printfbad2.in         || exit /b
call :runtest         printfbad3                              || exit /b
call :runtest_fail    printfbad4                              || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         printhuge                               || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest         procinfs                                || exit /b

call :execq "%GAWK% --profile=ap-profile0.out -f profile0.awk profile0.in > NUL" || exit /b
call :execq "more +2 ap-profile0.out > _profile0 && del /q ap-profile0.out"      || exit /b
call :cmpdel profile0                                         || exit /b

call :execq "%GAWK% -f xref.awk dtdgport.awk > _profile1.out1"        || exit /b
call :execq "%GAWK% --pretty-print=ap-profile1.out -f xref.awk"       || exit /b
call :execq "%GAWK% -f ap-profile1.out dtdgport.awk > _profile1.out2" || exit /b
call :cmpdel_ _profile1.out1 _profile1.out2                           || exit /b
call :exec del /q ap-profile1.out _profile1.out1                      || exit /b

call :execq "%GAWK% --profile=ap-profile2.out -v sortcmd=sort -f xref.awk dtdgport.awk > NUL" || exit /b
call :execq "more +2 ap-profile2.out > _profile2 && del /q ap-profile2.out"                   || exit /b
call :cmpdel profile2                                         || exit /b

call :execq "%GAWK% --profile=ap-profile3.out -f profile3.awk > NUL"   || exit /b
call :execq "more +2 ap-profile3.out > _profile3 && del /q ap-profile3.out" || exit /b
call :cmpdel profile3                                         || exit /b

call :execq "%GAWK% -f profile4.awk --pretty-print=_profile4" || exit /b
call :cmpdel profile4                                         || exit /b

call :execq "%GAWK% -f profile5.awk --pretty=_profile5.out 2> _profile5.err" || exit /b
call :execq "copy /b _profile5.out + _profile5.err _profile5 >NUL" || exit /b
call :cmpdel profile5 /t && call :exec del /q _profile5.out _profile5.err || exit /b

call :execq "%GAWK% --profile=ap-profile6.out -f profile6.awk > NUL"   || exit /b
call :execq "more +2 ap-profile6.out > _profile6 && del /q ap-profile6.out" || exit /b
call :cmpdel profile6                                         || exit /b

call :execq "%GAWK% --profile=ap-profile7.out -f profile7.awk > NUL"   || exit /b
call :execq "more +2 ap-profile7.out > _profile7 && del /q ap-profile7.out" || exit /b
call :cmpdel profile7                                         || exit /b

call :execq "%GAWK% -f profile8.awk --pretty-print=_profile8" || exit /b
call :cmpdel profile8                                         || exit /b

call :execq "%GAWK% -f profile9.awk --pretty-print=_profile9" || exit /b
call :cmpdel profile9                                         || exit /b

call :execq "%GAWK% -f profile10.awk --pretty-print=_profile10" || exit /b
call :cmpdel profile10                                        || exit /b

call :execq "%GAWK% -f profile11.awk --pretty-print=_profile11" || exit /b
call :cmpdel profile11                                        || exit /b

call :runtest         profile12 "--profile=ap-profile12.out" profile12.in || exit /b
call :exec del /q ap-profile12.out                            || exit /b

call :execq "%GAWK% -f profile13.awk --pretty-print=_profile13.out 2> _profile13.err" || exit /b
call :execq "copy /b _profile13.out + _profile13.err _profile13 >NUL"  || exit /b
call :cmpdel profile13 && call :exec del /q _profile13.out _profile13.err || exit /b

:: UNSUPPORTED tests: pseudo-terminal api is not supported yet on Windows
::call :runtest         pty1                                    || exit /b
::call :runtest         pty2                                    || exit /b

setlocal & set CALL_STAT=0
call :exec set AWKBUFSIZE=4096                                || goto :exit_local
call :runtest_in        rebuf                                 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           regnul1                               || exit /b
call :runtest           regnul2                               || exit /b
call :runtest           regx8bit                              || exit /b
call :runtest           reginttrad --traditional -r           || exit /b
call :runtest_in        reint --re-interval                   || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        reint2 --re-interval                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest_in        rsgetline                             || exit /b

call :execq "type rsgetline.in | %GAWK% -f rsgetline.awk > _rsglstdin 2>&1" && ^
call :cmpdel rsglstdin                                        || exit /b

call :runtest_in        rsstart1                              || exit /b
call :runtest_in        rsstart2                              || exit /b

call :execq "type rsstart1.in | %GAWK% -f rsstart2.awk > _rsstart3" && ^
call :cmpdel rsstart3                                         || exit /b

call :runtest_in        rstest6                               || exit /b
call :runtest_fail      sandbox1 --sandbox                    || exit /b
call :runtest           shadow --lint                         || exit /b
call :runtest           shadowbuiltin                         || exit /b
call :runtest_in        sortfor                               || exit /b
call :runtest_in        sortfor2                              || exit /b
call :runtest           sortu                                 || exit /b

call :execq_fail "%GAWK% --source=""BEGIN { a = 5;"" --source=""print a }"" > _sourcesplit 2>&1" _sourcesplit && ^
call :cmpdel sourcesplit                                      || exit /b

call :runtest_in        split_after_fpat                      || exit /b
call :runtest_in        splitarg4                             || exit /b
call :runtest_in        strftfld                              || exit /b

setlocal & set CALL_STAT=0
call :change_locale C                                         || goto :exit_local
call :exec set TZ=GMT0                                        || goto :exit_local
call :execq "%GAWK% -v OUTPUT=_strftime -v DATECMD=gmt_time.bat -f strftime.awk" || goto :exit_local
call :cmpdel strftime && call :exec del /q strftime.ok        || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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

call :execq "%GAWK% -d__symtab8 -f symtab8.awk symtab8.in > _symtab8" || exit /b
call :execq "type __symtab8 | find /v ""ENVIRON"" | find /v ""PROCINFO"" | find /v ""FILENAME"" >> _symtab8" || exit /b
call :cmpdel symtab8 && call :exec del /q __symtab8           || exit /b

call :runtest           symtab9                               || exit /b
call :exec del /q testit.txt                                  || exit /b

call :runtest_fail_in   symtab10 --debug                      || exit /b
call :runtest           symtab11                              || exit /b

:: UNSUPPORTED test: reading files with timeout is not supported yet on windows
::call :runtest           timeout                               || exit /b

call :runtest           typedregex1                           || exit /b
call :runtest           typedregex2                           || exit /b
call :runtest           typedregex3                           || exit /b
call :runtest typedregex4 -v "x=@/foo/" "y=@/bar/" /dev/null  || exit /b
call :runtest_in        typedregex5                           || exit /b
call :runtest_in        typedregex6                           || exit /b
call :runtest           typeof1                               || exit /b
call :runtest           typeof2                               || exit /b
call :runtest_fail      typeof3                               || exit /b
call :runtest           typeof4                               || exit /b
call :runtest_in        typeof5                               || exit /b
call :runtest watchpoint1 -D watchpoint1.in "< watchpoint1.script" || exit /b

(echo.%GAWK_DEFINES%) | find "ARRAYDEBUG" > NUL || ^
rem. && ((echo gawk was not compiled to support the array debug tests) & exit /b 0)

call :runtest           arrdbg -v "okfile=arrdbg.ok" "| find ""array_f""" && ^
call :exec del /q arrdbg.ok                                   || exit /b

exit /b 0

:::::: MACHINE-SPECIFIC TESTS :::::
:machine_tests

call :runtest           double1                               || exit /b
call :runtest           double2                               || exit /b
call :runtest           intformat                             || exit /b

exit /b 0

:::::: CHARACTER SET TESTS :::::
:charset_tests

call :runtest           asort                                 || exit /b
call :runtest           asorti                                || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        backbigs1                             || goto :exit_local
call :runtest_in        backsmalls1                           || goto :exit_local
call :runtest           backsmalls2                           || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           fmttest                               || exit /b
call :runtest           fnarydel                              || exit /b
call :runtest           fnparydl                              || exit /b

"%GAWK%" "--locale=JAPANESE_japan.20932" --version > NUL 2>&1
if ERRORLEVEL 1 (
  echo.JAPANESE_japan.20932 locale is not supported by the OS, skipping the test
  goto :skip_jp_test
)
setlocal & set CALL_STAT=0
call :change_locale "JAPANESE_japan.20932"                    || goto :exit_local
call :runtest           jarebug jarebug.in                    || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%
:skip_jp_test

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest           lc_num1                               || goto :exit_local
call :runtest_in        mbfw1                                 || goto :exit_local
call :runtest_in        mbprintf1                             || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :change_locale "ja_JP.UTF-8"                             || goto :exit_local
call :runtest           mbprintf2                             || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest_in        mbprintf3                             || goto :exit_local
call :runtest_in        mbprintf4                             || goto :exit_local
call :runtest           mbprintf5 mbprintf5.in                || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :change_locale "ru_RU.UTF-8"                             || goto :exit_local
call :runtest_in        mtchi18n                              || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :change_locale "fr_FR.UTF-8"                             || goto :exit_local
call :execq "%GAWK% -f nlstringtest.awk > _nlstringtest 2>&1" && ^
call :cmpdel_ nlstringtest-nogettext.ok _nlstringtest         || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           rebt8b2                               || exit /b

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
set "RTLENCMD=%GAWK% ""BEGIN {RS=""""""""}; {print length^(RT^)}"""
set "COMMAND=%GAWK% ""BEGIN {printf """"0\n\n\n1\n\n\n\n\n2\n\n""""; exit}"""
call :execq "%COMMAND% | %RTLENCMD% > _rtlenmb" && call :cmpdel rtlenmb || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           sort1                                 || exit /b
call :runtest_in        sprintfc                              || exit /b

exit /b 0

:::::: SHARED LIBRARY TESTS :::::
:shlib_tests

call :execq "%GAWK% --version | %GAWK% ""/API/ { exit 1 }""" && (
  echo shlib tests not supported on this system
  exit /b 0
)

call :runtest_in        apiterm                               || exit /b

setlocal & set CALL_STAT=0
call :get_top_srcdir
call :runtest           filefuncs -v "builddir=""%top_srcdir:\=/%""" "dist\filefuncs.dll" || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           fnmatch                               || exit /b
call :runtest           fnmatch_bugs                          || exit /b
call :runtest           fnmatch_more                          || exit /b
call :runtest           fnmatch_fbsd                          || exit /b
call :runtest           fnmatch_obsd "--locale=C"             || exit /b
call :runtest           fnmatchmb "--locale=en_US.UTF-8"      || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest           fnmatch_glibc                         || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest           fnmatch_de "--locale=German_Germany.28591" || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :execq "%GAWK% -f fnmatch_u8_de.awk --locale=de_DE.UTF-8 > _fnmatch_u8_de" && ^
call :cmpdel_ fnmatch_de_win.ok _fnmatch_u8_de                || exit /b

call :runtest           fnmatch_u8_en "--locale=en_US.UTF-8"  || exit /b

:: UNSUPPORTED tests: fork is not available under Windows yet
::call :runtest           fork                                  || exit /b
::call :runtest           fork2                                 || exit /b

call :execq "%GAWK% -f fts.awk" && ^
call :cmpdel fts && call :exec del /q fts.ok                  || exit /b

call :runtest           functab4                              || exit /b

call :execq "%GAWK% -f getfile.awk -v ""TESTEXT_QUIET=1"" -ltestext < getfile.awk > _getfile 2>&1" || exit /b
call :execq "type _getfile   | find /v ""get_file:""  > ap-getfile.out" || exit /b
call :execq "type _getfile   | find    ""get_file:"" >> ap-getfile.out" || exit /b
call :execq "type getfile.ok | find /v ""get_file:""  > getfile.ok1"    || exit /b
call :execq "type getfile.ok | find    ""get_file:"" >> getfile.ok1"    || exit /b
call :cmpdel_ getfile.ok1 ap-getfile.out /t && call :exec del /q _getfile getfile.ok1 || exit /b

setlocal & set CALL_STAT=0
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "copy /b /y inplace.1.in _inplace1.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace1.2 > NUL"       || goto :exit_local
call :execq "%GAWK% -i inplace %SCRIPT% _inplace1.1 - _inplace1.2 < inplace.in > _inplace1 2>&1" || goto :exit_local
call :cmpdel inplace1 && ^
call :cmpdel inplace1.1 && ^
call :cmpdel inplace1.2 || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :execq "copy /b /y inplace.1.in _inplace2.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace2.2 > NUL"       || goto :exit_local
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%GAWK% -i inplace -v inplace::suffix=.bak %SCRIPT% _inplace2.1 - _inplace2.2 < inplace.in > _inplace2 2>&1" || goto :exit_local
call :cmpdel inplace2 && ^
call :cmpdel inplace2.1 && ^
call :cmpdel inplace2.2 && ^
call :cmpdel inplace2.1.bak && ^
call :cmpdel inplace2.2.bak || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :execq "copy /b /y inplace.1.in _inplace2bcomp.1 > NUL"  || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace2bcomp.2 > NUL"  || goto :exit_local
set "SCRIPT=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%GAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT% _inplace2bcomp.1 - _inplace2bcomp.2 < inplace.in > _inplace2bcomp 2>&1" || goto :exit_local
call :cmpdel inplace2bcomp && ^
call :cmpdel inplace2bcomp.1 && ^
call :cmpdel inplace2bcomp.2 && ^
call :cmpdel inplace2bcomp.1.orig && ^
call :cmpdel inplace2bcomp.2.orig || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :execq "copy /b /y inplace.1.in _inplace3.1 > NUL"       || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace3.2 > NUL"       || goto :exit_local
set "SCRIPT1=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
set "SCRIPT2=""BEGIN {print """"Before""""} {gsub^(/bar/, """"foo""""^); print} END {print """"After""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%GAWK% -i inplace -v inplace::suffix=.bak %SCRIPT1% _inplace3.1 - _inplace3.2 < inplace.in > _inplace3 2>&1" || goto :exit_local
call :execq "%GAWK% -i inplace -v inplace::suffix=.bak %SCRIPT2% _inplace3.1 - _inplace3.2 < inplace.in >> _inplace3 2>&1" || goto :exit_local
call :cmpdel inplace3 && ^
call :cmpdel inplace3.1 && ^
call :cmpdel inplace3.2 && ^
call :cmpdel inplace3.1.bak && ^
call :cmpdel inplace3.2.bak || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :execq "copy /b /y inplace.1.in _inplace3bcomp.1 > NUL"  || goto :exit_local
call :execq "copy /b /y inplace.2.in _inplace3bcomp.2 > NUL"  || goto :exit_local
set "SCRIPT1=""BEGIN {print """"before""""} {gsub^(/foo/, """"bar""""^); print} END {print """"after""""}"""
set "SCRIPT2=""BEGIN {print """"Before""""} {gsub^(/bar/, """"foo""""^); print} END {print """"After""""}"""
call :exec set AWKPATH=../awklib/eg/lib                       || goto :exit_local
call :execq "%GAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT1% _inplace3bcomp.1 - _inplace3bcomp.2 < inplace.in > _inplace3bcomp 2>&1" || goto :exit_local
call :execq "%GAWK% -i inplace -v INPLACE_SUFFIX=.orig %SCRIPT2% _inplace3bcomp.1 - _inplace3bcomp.2 < inplace.in >> _inplace3bcomp 2>&1" || goto :exit_local
call :cmpdel inplace3bcomp && ^
call :cmpdel inplace3bcomp.1 && ^
call :cmpdel inplace3bcomp.2 && ^
call :cmpdel inplace3bcomp.1.orig && ^
call :cmpdel inplace3bcomp.2.orig || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           ordchr                                || exit /b
call :execq "%GAWK% --load ordchr ""BEGIN {print chr^(ord^(""""z""""^)^)}"" > _ordchr2 2>&1" && ^
call :cmpdel ordchr2                                          || exit /b

setlocal & set CALL_STAT=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -f readdir.awk ""%top_srcdir%"" > _readdir" || goto :exit_local
call :execq "..\helpers\tst_ls -afi ""%top_srcdir%"" > _dirlist"   || goto :exit_local
call :execq "..\helpers\tst_ls -lna ""%top_srcdir%"" > _longlist1" || goto :exit_local
call :execq "more +1 _longlist1 > _longlist"                  || goto :exit_local
call :execq "%GAWK% -f readdir0.awk -v extout=_readdir -v dirlist=_dirlist -v longlist=_longlist > readdir.ok" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir && call :exec del /q readdir.ok _dirlist _longlist1 _longlist || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -lreaddir -F/ ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%top_srcdir%"" > readdir_test.ok" || goto :exit_local
call :execq "%GAWK% -lreaddir_test ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%top_srcdir%"" > _readdir_test" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir_test && call :exec del /q readdir_test.ok || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

setlocal & set CALL_STAT=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -lreaddir -F/ -f readdir_retest.awk ""%top_srcdir%"" > readdir_retest.ok" || goto :exit_local
call :execq "%GAWK% -lreaddir_test -F/ -f readdir_retest.awk ""%top_srcdir%"" > _readdir_retest" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir_retest && call :exec del /q readdir_retest.ok || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :execq "%GAWK% -l readfile ""BEGIN {printf """"%%%%s"""", readfile^(""""Makefile.am""""^)}"" > _readfile 2>&1" || exit /b
call :cmpdel_ Makefile.am _readfile                           || exit /b

call :runtest           readfile2 readfile2.awk readdir.awk   || exit /b 
call :runtest           revout                                || exit /b 
call :runtest           revtwoway                             || exit /b 
call :runtest_in        rwarray -lrwarray  "-v FORMAT=3.1"    || exit /b
call :runtest_in        rwarray -lrwarray0 "-v FORMAT=3.0"    || exit /b

setlocal & set CALL_STAT=0
call :get_top_srcdir
call :execq "%GAWK% "" /^^^(@load^|BEGIN^)/,/^^}/"" ""%top_srcdir:\=/%/extension/testext.c"" > testext.awk" || goto :exit_local
call :runtest           testext                               || goto :exit_local
call :exec del /q testext.awk testexttmp.txt                  || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

call :runtest           time                                  || exit /b

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
:: if called from runtest_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_      %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_fail
:: if called from runtest_fail_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_fail_ %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_ok
:: if called from runtest_ok_in
:: %4 = "<"
:: %5 = %1.in
call :runtest_ok_   %1 -f %1.awk %2 %3 %4 %5
exit /b

:runtest_
:: run test %1, then compare _%1 with %1.ok
:: if called from runtest_in -> runtest
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
setlocal & set CALL_STAT=0
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%GAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq "%COMMAND:^^=^%" && call :cmpdel %1
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:runtest_fail_
:: run test %1, then append "EXIT CODE: 2" to _%1, then compare _%1 with %1.ok
:: if called from runtest_fail_in -> runtest_fail
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
:: awk should fail
setlocal & set CALL_STAT=0
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%GAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq_fail "%COMMAND:^^=^%" "_%1" && call :cmpdel %1
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:runtest_ok_
:: run test %1, then append "EXIT CODE: 0" to _%1, then compare _%1 with %1.ok
:: if called from runtest_ok_in -> runtest_ok
:: %2 = -f
:: %3 = %1.awk
:: %4 = %2
:: %5 = %3
:: %6 = "<"
:: %7 = %1.in
:: awk should NOT fail
setlocal & set CALL_STAT=0
:: %~7 doubles ^ in the value, un-double it
set "COMMAND=%GAWK% %~2 %~3 %~4 %~5 %~6 %~7 > _%1 2>&1"
call :execq_ok "%COMMAND:^^=^%" "_%1" && call :cmpdel %1
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

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
set /A "CALL_STAT+=1"
echo %*
%*
exit /b

:execq
:: %1 - complex command in double-quotes
:: note: replace "" with " in arguments
set /A "CALL_STAT+=1"
set "COMMAND=%~1"
:: escape symbols for ECHO
set "COMMAND=%COMMAND:>=^>%"
set "COMMAND=%COMMAND:<=^<%"
set "COMMAND=%COMMAND:&=^&%"
set "COMMAND=%COMMAND:|=^|%"
set "COMMAND=%COMMAND:(=^(%"
set "COMMAND=%COMMAND:)=^)%"
:: escaped symbols inside double-quotes do not need to be escaped for ECHO
set "COMMAND=%COMMAND:^^^<=<%"
set "COMMAND=%COMMAND:^^^>=>%"
set "COMMAND=%COMMAND:^^^&=&%"
set "COMMAND=%COMMAND:^^^|=|%"
set "COMMAND=%COMMAND:^^^(=(%"
set "COMMAND=%COMMAND:^^^)=)%"
set "COMMAND=%COMMAND:^^^^=^%"
echo %COMMAND:""="%
:: now run the command
set "COMMAND=%~1"
:: unescape symbols inside double-quotes
set "COMMAND=%COMMAND:^^<=<%"
set "COMMAND=%COMMAND:^^>=>%"
set "COMMAND=%COMMAND:^^&=&%"
set "COMMAND=%COMMAND:^^|=|%"
set "COMMAND=%COMMAND:^^(=(%"
set "COMMAND=%COMMAND:^^)=)%"
set "COMMAND=%COMMAND:^^^^=^%"
%COMMAND:""="%
exit /b

:execq_fail
:: %1 - complex command in double-quotes that should fail
:: %2 - file to append exit code to
call :execq "%~1" && exit /b 1
:: reset ERRORLEVEL
setlocal & set "err1=%ERRORLEVEL%" & set CALL_STAT=0
cmd /c exit /b 0
call :execq "(echo.EXIT CODE: %err1%) >> ""%~2"""
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

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
SET /A "timeout+=1"
goto :waitfor1

:build_err
echo CALL_STAT=%CALL_STAT%
echo *** Compilation failed! ***
exit /b 1

:test_err
echo CALL_STAT=%CALL_STAT%
echo *** Test failed! ***
exit /b 1

:change_locale
call :exec set LANGUAGE=
if not defined GAWKLOCALE (call :execq "set ""LC_ALL=%~1""") else (call :execq "set ""LC_ALL=%GAWKLOCALE%""")
if not defined GAWKLOCALE (call :execq "set ""LANG=%~1""") else (call :execq "set ""LANG=%GAWKLOCALE%""")
exit /b

:exit_local
:: exit local scope accounting CALL_STAT
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:get_top_srcdir
for /f %%a in ("%CD%") do set "top_srcdir=%%~dpa"
set "top_srcdir=%top_srcdir:~0,-1%"
exit /b
