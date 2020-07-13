@echo off
setlocal

:: Note: these environment variables are taken in account (default value in parents):
::
:: CLCC         (cl)         - how to call cl, this may be for example "cl"
:: CLVER        {auto}       - cl version, e.g. for cl version 19.26.28806 CLVER must be set to 19026
:: CLLIB        (lib)        - how to call lib, this may be for example "lib"
:: CLLINK       (link)       - how to call link, this may be for example "link"
::
:: GCC          (gcc)        - how to call gcc, this may be for example "gcc -m32"
:: GXX          (g++)        - how to call g++, this may be for example "g++ -m32"
:: GCCVER       {auto}       - gcc version, e.g. for gcc version 9.13.31 GCCVER must be set to 9013
:: AR           (ar)         - how to call ar, this may be for example "ar"
:: GCCAR        (gcc-ar)     - how to call gcc-ar, this may be for example "gcc-ar"
::
:: CLANG        (clang)      - how to call clang, this may be for example "clang -m32"
:: CLANGXX      (clang++)    - how to call clang++, this may be for example "clang++ -m32"
:: CLANGVER     {auto}       - clang version, e.g. for clang version 9.13.31 CLANGVER must be set to 9013
:: AR           (ar)         - how to call ar, this may be for example "ar"
:: LLVMAR       (llvm-ar)    - how to call llvm-ar, this may be for example "llvm-ar"
::
:: CLANGMSVC    (clang)      - how to call clang-msvc c compiler, this may be for example "clang"
:: CLANGMSVCXX  (clang++)    - how to call clang-msvc c++ compiler, this may be for example "clang++"
:: CLANGMSVCVER {auto}       - clang-msvc compiler version, e.g. for version 11.23.6 CLANGMSVCVER must be set to 11023
:: CLLIB        (lib)        - how to call lib, this may be for example "lib"
:: LLVMAR       (llvm-ar)    - how to call llvm-ar, this may be for example "llvm-ar"

:: re-execute batch file to avoid pollution of the environment variables space of the caller process
if not defined BUILDBATRECURSIVE (
  set "BUILDBATRECURSIVE=%CMDCMDLINE:"=%"
  %WINDIR%\System32\cmd.exe /c ""%~dpnx0" %*"
  exit /b
)

pushd "%~dp0"

if ""=="%~1" (
:usage
  echo Usage:
  echo.
  echo %~nx0 all     {options}  - build everything
  echo %~nx0 awk     {options}  - build only the GAWK executable
  echo %~nx0 shlib   {options}  - build only extension DLLs
  echo %~nx0 helpers {options}  - build only test helpers
  echo.
  echo where {options}:  [gcc^|clang^|cl^|clang-msvc] [c++] [pedantic] [debug] [32^|64] [analyze]
  echo.
  echo Testing:
  echo.
  echo %~nx0 test         - run all tests
  echo %~nx0 test basic   - run only basic tests
  echo %~nx0 test unix    - run only unix tests
  echo %~nx0 test ext     - run only extension tests
  echo %~nx0 test cpu     - run only machine tests
  echo %~nx0 test locale  - run only locale tests
  echo %~nx0 test shlib   - run only tests of extension DLLs
  echo %~nx0 test extra   - run only extra tests
  echo %~nx0 clean        - cleanup intermediate build results
  echo %~nx0 distclean    - cleanup intermediate build results and contents of the "dist" directory
  if "%BUILDBATRECURSIVE:build.bat=%"=="%BUILDBATRECURSIVE%" ((echo.)&set /p DUMMY="Hit ENTER to continue...")
  exit /b 1
)

set "FIND=%WINDIR%\System32\Find.exe"

:: remove double quotes
if defined UNICODE_CTYPE set "UNICODE_CTYPE=%UNICODE_CTYPE:"=%"
if defined LIBUTF16      set "LIBUTF16=%LIBUTF16:"=%"
if defined MSCRTX        set "MSCRTX=%MSCRTX:"=%"
if defined SAL_DEFS_H    set "SAL_DEFS_H=%SAL_DEFS_H:"=%"

:: "all" by default
set TOOLCHAIN=
set DO_BUILD_AWK=x
set DO_BUILD_SHLIB=x
set DO_BUILD_HELPERS=x
set DO_TEST=
set DO_TEST_ONLY=

if "test"=="%~1" (
  set TOOLCHAIN=x
  set DO_BUILD_AWK=
  set DO_BUILD_SHLIB=
  set DO_BUILD_HELPERS=
  set DO_TEST=x
  set "DO_TEST_ONLY=%~2"
  goto :do_build
)
if "awk"=="%~1" (
  set DO_BUILD_SHLIB=
  set DO_BUILD_HELPERS=
  set DO_TEST=
) else (if "shlib"=="%~1" (
  set DO_BUILD_AWK=
  set DO_BUILD_HELPERS=
  set DO_TEST=
) else (if "helpers"=="%~1" (
  set DO_BUILD_AWK=
  set DO_BUILD_SHLIB=
  set DO_TEST=
) else (
  if "clean"=="%~1" goto :do_clean
  if "distclean"=="%~1" (
    echo del dist\gawk.exe dist\*.dll dist\*.pdb
    del dist\gawk.exe dist\*.dll dist\*.pdb 2>NUL
:do_clean
    echo del *.ilk *.pdb *.obj pc\*.obj *.a support\*.obj support\*.a extension\*.obj dist\*.exp dist\*.lib helpers\*.obj helpers\*.exe helpers\*.pdb *.dll.lib *.dll.exp
    del *.ilk *.pdb *.obj pc\*.obj *.a support\*.obj support\*.a extension\*.obj dist\*.exp dist\*.lib helpers\*.obj helpers\*.exe helpers\*.pdb *.dll.lib *.dll.exp 2>NUL
    if defined UNICODE_CTYPE (
      echo del "%UNICODE_CTYPE%\*.obj" "%UNICODE_CTYPE%\*.a"
      del "%UNICODE_CTYPE%\*.obj" "%UNICODE_CTYPE%\*.a" 2>NUL
    )
    if defined LIBUTF16 (
      echo del "%LIBUTF16%\*.obj" "%LIBUTF16%\*.a"
      del "%LIBUTF16%\*.obj" "%LIBUTF16%\*.a" 2>NUL
    )
    if defined MSCRTX (
      echo del "%MSCRTX%\*.obj" "%MSCRTX%\*.a"
      del "%MSCRTX%\*.obj" "%MSCRTX%\*.a" 2>NUL
    )
    exit /b
  )
  if not "all"=="%~1" (
    echo ERROR: unknown build target: "%~1"
    echo.
    goto :usage
  )
)))

call :checkbuildopt "%~2" || goto :usage
call :checkbuildopt "%~3" || goto :usage
call :checkbuildopt "%~4" || goto :usage
call :checkbuildopt "%~5" || goto :usage
call :checkbuildopt "%~6" || goto :usage
call :checkbuildopt "%~7" || goto :usage
if not ""=="%~8" call :badbuildopts || goto :usage

call :checkoptsuniq %* || (echo ERROR: There are duplicate options) && exit /b 1

:: enable static analysis of the sources
set DO_ANALYZE=
:: compile sources by the C++ compiler
set COMPILE_AS_CXX=
:: build with almost all warnings enabled
set BUILD_PEDANTIC=
:: do debug build with all debugging options enabled
set DEBUG_BUILD=
:: build for 32-bit or 64-bit CPU
set BUILDFORCPU=

setlocal
set SPECTOOLCHAIN=
(call :isoptionset "cl"         %*) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%cl"
(call :isoptionset "gcc"        %*) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%gcc"
(call :isoptionset "clang"      %*) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%clang"
(call :isoptionset "clang-msvc" %*) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%clangmsvc"
if defined SPECTOOLCHAIN (
if not "%SPECTOOLCHAIN%"=="cl" (
if not "%SPECTOOLCHAIN%"=="gcc" (
if not "%SPECTOOLCHAIN%"=="clang" (
if not "%SPECTOOLCHAIN%"=="clangmsvc" (
  (echo ERROR: Duplicate toolchain) && exit /b 1
)))))
endlocal & set "TOOLCHAIN=%SPECTOOLCHAIN%"

(call :isoptionset "c++"      %*) && set COMPILE_AS_CXX=x
(call :isoptionset "pedantic" %*) && set BUILD_PEDANTIC=x
(call :isoptionset "debug"    %*) && set DEBUG_BUILD=x
(call :isoptionset "analyze"  %*) && set DO_ANALYZE=x
(call :isoptionset "32"       %*) && set BUILDFORCPU=32
(call :isoptionset "64"       %*) && set "BUILDFORCPU=%BUILDFORCPU%64"

if "%BUILDFORCPU%"=="3264" ((echo ERROR: Duplicate CPU architecture) && exit /b 1)

:: choose toolchain
if not defined TOOLCHAIN cl > NUL 2>&1 && set TOOLCHAIN=cl
if not defined TOOLCHAIN gcc --version > NUL 2>&1 && set TOOLCHAIN=gcc
if not defined TOOLCHAIN clang --version | %FIND% /i "msvc" > NUL && set TOOLCHAIN=clangmsvc
if not defined TOOLCHAIN clang --version > NUL 2>&1 && set TOOLCHAIN=clang
if not defined TOOLCHAIN (
  echo Can't determine toolchan.
  echo Supported toolchains: CL, GCC, CLANG, CLANG-MSVC.
  echo.
  echo To setup CL toolchain, run:
  echo  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
  echo.
  echo To setup MINGW GCC toolchain, first run:
  echo  "C:\msys64\mingw64.exe"
  echo then make sure mingw-w64-x86_64-gcc is installed:
  echo  pacman -S mingw-w64-x86_64-gcc
  echo then in MSYS window, run cmd.exe ^(with proper environment^):
  echo  start "" cmd.exe
  echo then close MSYS window and in cmd.exe console change directory:
  echo  cd "%~dp0"
  echo.
  echo Notes:
  echo For GCC-i686     - install mingw-w64-i686-gcc
  echo For CLANG-i686   - install mingw-w64-i686-clang
  echo For CLANG-x86_64 - install mingw-w64-x86_64-clang
  exit /b 1
)

if not "%DO_BUILD_AWK%%DO_BUILD_HELPERS%"=="" (
  if not defined LIBUTF16 (
    echo Please define LIBUTF16 - path to the libutf16 library ^(https://github.com/mbuilov/libutf16^)
    exit /b 1
  )
  if not defined UNICODE_CTYPE (
    echo Please define UNICODE_CTYPE - path to the unicode_ctype library ^(https://github.com/mbuilov/unicode_ctype^)
    exit /b 1
  )
)

if not "%DO_BUILD_AWK%%DO_BUILD_SHLIB%"=="" (
  if not defined MSCRTX (
    echo Please define MSCRTX - path to the mscrtx library ^(https://github.com/mbuilov/mscrtx^)
    exit /b 1
  )
)

:: individual debugging options
rem set GAWKDEBUG=x
if defined DEBUG_BUILD (set GAWKDEBUG=x) else set GAWKDEBUG=
rem set ARRAYDEBUG=x
if defined DEBUG_BUILD (set ARRAYDEBUG=x) else set ARRAYDEBUG=
rem set MEMDEBUG=x
if defined DEBUG_BUILD (set MEMDEBUG=x) else set MEMDEBUG=

:: off by default, because is too noisy
set REGEXDEBUG=
rem if defined DEBUG_BUILD (set REGEXDEBUG=x) else set REGEXDEBUG=

:: off by default, because is too slow
set MEMDEBUGLEAKS=
rem if defined DEBUG_BUILD (set MEMDEBUGLEAKS=x) else set MEMDEBUGLEAKS=

:: gawk-specific defines
set "GAWK_DEFINES=/DGAWK /DEXEC_HOOK /D_REGEX_LARGE_OFFSETS"
if defined GAWKDEBUG     set "GAWK_DEFINES=%GAWK_DEFINES% /DGAWKDEBUG"
if defined ARRAYDEBUG    set "GAWK_DEFINES=%GAWK_DEFINES% /DARRAYDEBUG"
if defined MEMDEBUG      set "GAWK_DEFINES=%GAWK_DEFINES% /DMEMDEBUG"
if defined REGEXDEBUG    set "GAWK_DEFINES=%GAWK_DEFINES% /DDEBUG"
if defined MEMDEBUGLEAKS set "GAWK_DEFINES=%GAWK_DEFINES% /DMEMDEBUGLEAKS"

:: include sal_defs.h, if available
if defined SAL_DEFS_H    set "GAWK_DEFINES=%GAWK_DEFINES% /FI""%SAL_DEFS_H%"""

if not "%TOOLCHAIN%"=="cl" goto :toolchain1

:::::::::::::::::::::: CL :::::::::::::::::::::::

if defined BUILDFORCPU echo WARNING: Build option "%BUILDFORCPU%" is not supported for CL
if not defined CLCC   (set CLCC=cl)     else call :escape_cc CLCC
if not defined CLLIB  (set CLLIB=lib)   else call :escape_cc CLLIB
if not defined CLLINK (set CLLINK=link) else call :escape_cc CLLINK

if not defined CLVER (
  call :extract_ver CLVER "%CLCC% <NUL 2>&1" "Microsoft (R) C/C++ Optimizing Compiler Version " || (
    echo Couldn't determine cl compiler version, please specify CLVER explicitly ^(e.g. CLVER=19026 for 19.26.28806^)
    exit /b 1
  )
)

:: disable MSVC deprecation warnings
set "NODEPRECATE=/D_CRT_NONSTDC_NO_DEPRECATE /D_CRT_SECURE_NO_DEPRECATE /D_WINSOCK_DEPRECATED_NO_WARNINGS"

rem 4013 - 'function' undefined; assuming extern returning int
rem 4820 - 'bytes' bytes padding added after construct 'member_name'
rem 4131 - uses old-style declarator
rem 5045 - Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
rem 4464 - relative include path contains '..'
rem 4061 - enumerator 'name' in switch of enum 'name' is not explicitly handled by a case label
set "WARNING_OPTIONS=/Wall /we4013 /wd4820 /wd4131 /wd5045 /wd4464 /wd4061"

if not defined DEBUG_BUILD (
  rem 4711 - function 'function' selected for automatic inline expansion
  rem 4710 - 'function': function not inlined
  set "WARNING_OPTIONS=%WARNING_OPTIONS% /wd4711 /wd4710"
)

:: define _Noreturn for pc/config.h & support/cdefs.h
:: define __inline for gettext.h
set "CMN_DEFINES=/DNEED_C99_FEATURES /D_Noreturn=__declspec(noreturn) /D__inline=__inline"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% /analyze"
)

set "CMN_DEFINES=%CMN_DEFINES% /Zc:strictStrings /Zc:ternary /Zc:rvalueCast"

if not defined COMPILE_AS_CXX goto :cl_no_cxx

:: compile as c++
set "CMN_DEFINES=%CMN_DEFINES% /TP /Zc:__cplusplus"

:: c++ specific warnings
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
rem 4625 - 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
rem 5026 - 'type': move constructor was implicitly defined as deleted
set "WARNING_OPTIONS=%WARNING_OPTIONS% /wd4800 /wd4623 /wd4626 /wd5027 /wd5039 /wd4611 /wd4514 /wd4710 /wd4711 /wd4582 /wd4625 /wd5026"

:cl_no_cxx

:: gawk-specific defines
:: define _Restrict_ for for support/regex.h
:: define HAVE___INLINE for support/libc-config.h
:: define HAVE___RESTRICT for support/libc-config.h
set "GAWK_DEFINES=%GAWK_DEFINES% /D_Restrict_=__restrict /DHAVE___INLINE /DHAVE___RESTRICT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=/nologo %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Od /Z7 /EHsc /MTd"
  set "LIBABSTRACTLIB=%CLLIB% /nologo /OUT:"
  set "GAWKLINK=%CLLINK% /nologo /DEBUG /INCREMENTAL:NO"
  set "SHLIBLINK=%CLLINK% /nologo /DEBUG /INCREMENTAL:NO /DLL /SUBSYSTEM:CONSOLE /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntimed /DEFAULTLIB:libucrtd"
  set "TOOLLINK=%CLLINK% /nologo /DEBUG /INCREMENTAL:NO"
) else (
  :: release options
  set "CMNOPTS=/nologo %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Ox /GF /Gy /GS- /GL /EHsc /DNDEBUG /MT"
  set "LIBABSTRACTLIB=%CLLIB% /nologo /LTCG /OUT:"
  set "GAWKLINK=%CLLINK% /nologo /LTCG"
  set "SHLIBLINK=%CLLINK% /nologo /LTCG /DLL /SUBSYSTEM:CONSOLE /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntime /DEFAULTLIB:libucrt"
  set "TOOLLINK=%CLLINK% /nologo /LTCG"
)

set "UNICODE_CTYPECC=%CLCC% /c %CMNOPTS% /I""%UNICODE_CTYPE%"" /Fo"
set "LIBUTF16CC=%CLCC% /c %CMNOPTS% /I""%LIBUTF16%"" /Fo"
set "MSCRTXCC=%CLCC% /c %CMNOPTS% /I""%MSCRTX%"" /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /Fo"
set "GAWKCC=%CLCC% /c %CMNOPTS% %GAWK_DEFINES% /DGAWK_STATIC_CRT /DHAVE_CONFIG_H /DLOCALEDIR=\"\" /DDEFPATH=\".\" /DDEFLIBPATH=\"\" /DSHLIBEXT=\"dll\" /Isupport /Ipc /I. /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /I""%MSCRTX%"" /Fo"
set "SHLIBCC=%CLCC% /c %CMNOPTS% /DGAWK_STATIC_CRT /DHAVE_CONFIG_H /Iextension /Ipc /I. /I""%MSCRTX%"" /Fo"
set "TOOLCC=%CLCC% /c %CMNOPTS% /I""%MSCRTX%"" /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /Fo"

set CC_DEFINE=/D
set CC_INCLUDE=/I

goto :do_build

:cl_awk_ld
call :execq "%GAWKLINK:^^=^% /DEFAULTLIB:WS2_32.lib /OUT:dist\%~1 %~2"
exit /b

:cl_shlib_ld
call :execq "%SHLIBLINK:^^=^% /IMPLIB:%~1.lib /OUT:dist\%~1 %~2"
exit /b

:cl_tool_ld
call :execq "%TOOLLINK:^^=^% /OUT:helpers\%~1 %~2"
exit /b

:::::::::::::::::::::: CL END :::::::::::::::::::

:toolchain1
if not "%TOOLCHAIN%"=="gcc" goto :toolchain2

::::::::::::::::::::: GCC :::::::::::::::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined GCC    (set "GCC=gcc %BUILDFORCPU%") else call :escape_cc GCC "%BUILDFORCPU%"
if not defined GXX    (set "GXX=g++ %BUILDFORCPU%") else call :escape_cc GXX "%BUILDFORCPU%"
if not defined AR     (set AR=ar)                   else call :escape_cc AR
if not defined GCCAR  (set GCCAR=gcc-ar)            else call :escape_cc GCCAR

if not defined GCCVER (
  call :extract_ver GCCVER "%GCC% --verbose 2>&1" "gcc version " || (
    echo Couldn't determine gcc compiler version, please specify GCCVER explicitly ^(e.g. GCCVER=9003 for 9.3^)
    exit /b 1
  )
)

set "CMN_DEFINES=-fstrict-aliasing -DNEED_C99_FEATURES"

if defined DO_ANALYZE (
  if GCCVER GEQ 10000 (
    :: analyze sources
    set "CMN_DEFINES=%CMN_DEFINES% -fanalyzer"
  )
)

:: assume "%zu" printf-format specifier is supported by the underlying CRT library
set "CMN_DEFINES=%CMN_DEFINES% -D__USE_MINGW_ANSI_STDIO"

set WARNING_OPTIONS=-Wall

if not defined BUILD_PEDANTIC goto :gcc_no_pedantic

:: everything
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wextra"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Waddress"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Waddress-of-packed-member"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Waggregate-return"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Waggressive-loop-optimizations"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Walloc-zero"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Walloca"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Warray-bounds=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wattribute-alias=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wattribute-warning"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wattributes"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbool-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbool-operation"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbuiltin-declaration-mismatch"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbuiltin-macro-redefined"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcannot-profile"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcast-align=strict"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcast-function-type"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcast-qual"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wchar-subscripts"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wclobbered"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcomment"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wconversion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcoverage-mismatch"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcpp"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdangling-else"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdate-time"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeprecated"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeprecated-declarations"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdisabled-optimization"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdiv-by-zero"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdouble-promotion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wduplicated-branches"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wduplicated-cond"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wempty-body"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wendif-labels"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wenum-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wexpansion-to-defined"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wfloat-conversion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wfloat-equal"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-contains-nul"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-extra-args"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-nonliteral"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-overflow=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-security"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-signedness"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-truncation=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-y2k"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-zero-length"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wframe-address"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wfree-nonheap-object"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Whsa"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wif-not-aligned"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wignored-attributes"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wignored-qualifiers"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wimplicit-fallthrough=3"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winit-self"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winline"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wint-in-bool-context"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wint-to-pointer-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winvalid-memory-model"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winvalid-pch"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wlogical-not-parentheses"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wlogical-op"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wlto-type-mismatch"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmain"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmaybe-uninitialized"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmemset-elt-size"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmemset-transposed-args"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmisleading-indentation"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-attributes"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-braces"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-declarations"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-field-initializers"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-format-attribute"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-include-dirs"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-noreturn"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-profile"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmultichar"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmultistatement-macros"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnarrowing"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnonnull"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnonnull-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnormalized=nfc"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnull-dereference"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wodr"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wopenmp-simd"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Woverflow"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Woverlength-strings"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpacked"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpacked-bitfield-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpacked-not-aligned"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpadded"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wparentheses"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpedantic"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpointer-arith"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpointer-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpragmas"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpsabi"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wredundant-decls"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wrestrict"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wreturn-local-addr"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wreturn-type"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wscalar-storage-order"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsequence-point"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshadow"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshadow=local"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshadow=compatible-local"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshadow=global"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshift-count-negative"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshift-count-overflow"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshift-negative-value"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wshift-overflow=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsign-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsign-conversion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsizeof-array-argument"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsizeof-pointer-div"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsizeof-pointer-memaccess"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstack-protector"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-aliasing=1"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-overflow=5"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstringop-overflow=4"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstringop-truncation"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=cold"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=const"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=format"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=malloc"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=noreturn"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-attribute=pure"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-final-methods"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-final-types"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch-bool"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch-default"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch-enum"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch-unreachable"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsync-nand"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wtautological-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wtrampolines"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wtrigraphs"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wtype-limits"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wundef"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wuninitialized"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunknown-pragmas"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunreachable-code"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunsafe-loop-optimizations"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-but-set-parameter"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-but-set-variable"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-const-variable=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-function"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-label"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-local-typedefs"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-macros"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-parameter"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-result"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-value"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunused-variable"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvarargs"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvariadic-macros"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvector-operation-performance"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvla"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvolatile-register-var"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wwrite-strings"

:: GCC10
if %GCCVER% LSS 10000 goto :cmn_not_gcc_10
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstring-compare"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wzero-length-bounds"
:cmn_not_gcc_10

:: disable some warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-long-long"

:gcc_no_pedantic

if not defined COMPILE_AS_CXX goto :gcc_no_cxx

:: compile as c++11
set "CMN_DEFINES=-x c++ -std=c++11 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :gcc_do_build

set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wabi-tag"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Waligned-new=all"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc++11-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc++14-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc++1z-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wcatch-value=3"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wclass-conversion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wclass-memaccess"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wconditionally-supported"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wconversion-null"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wctor-dtor-privacy"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdelete-incomplete"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdelete-non-virtual-dtor"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeprecated-copy"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeprecated-copy-dtor"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Weffc++"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wextra-semi"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winherited-variadic-ctor"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winit-list-lifetime"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Winvalid-offsetof"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wliteral-suffix"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmultiple-inheritance"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnamespaces"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnoexcept"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnoexcept-type"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnon-template-friend"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnon-virtual-dtor"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Woverloaded-virtual"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpessimizing-move"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wplacement-new=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpmf-conversions"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wprio-ctor-dtor"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wredundant-move"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wregister"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wreorder"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsign-promo"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsized-deallocation"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-null-sentinel"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsubobject-linkage"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsuggest-override"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wsynth"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wtemplates"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wterminate"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wuseless-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvirtual-inheritance"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wvirtual-move-assign"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wzero-as-null-pointer-constant"

:: GCC10
if %GCCVER% LSS 10000 goto :cxx_not_gcc_10
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmismatched-tags"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wredundant-tags"
:cxx_not_gcc_10

goto :gcc_do_build

:gcc_no_cxx

:: compile as c99
set "CMN_DEFINES=-std=c99 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :gcc_do_build

set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wabsolute-value"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbad-function-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc++-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc11-c2x-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc99-c11-compat"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeclaration-after-statement"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdesignated-init"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdiscarded-array-qualifiers"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdiscarded-qualifiers"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wduplicate-decl-specifier"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Werror=implicit-function-declaration"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wimplicit"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wimplicit-function-declaration"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wimplicit-int"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wincompatible-pointer-types"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wint-conversion"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wjump-misses-init"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-parameter-type"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wmissing-prototypes"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wnested-externs"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wold-style-declaration"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wold-style-definition"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Woverride-init"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Woverride-init-side-effects"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpointer-sign"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wpointer-to-int-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-prototypes"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wunsuffixed-float-constants"

:gcc_do_build

:: gawk-specific defines
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/FI=-include %"
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/=-%"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -ggdb3 -fstack-protector-all"
) else (
  :: release options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
)

if defined COMPILE_AS_CXX (
  set "GAWKLINK=%GXX% -static -municode -mconsole"
  set "SHLIBLINK=%GXX% -static -municode -mconsole -mdll"
  set "TOOLLINK=%GXX% -static -municode -mconsole"
) else (
  set "GAWKLINK=%GCC% -static -municode -mconsole"
  set "SHLIBLINK=%GCC% -static -municode -mconsole -mdll"
  set "TOOLLINK=%GCC% -static -municode -mconsole"
)

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%AR% -csr "
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all"
) else (
  set "LIBABSTRACTLIB=%GCCAR% -csr "
  set "GAWKLINK=%GAWKLINK% -flto"
  set "SHLIBLINK=%SHLIBLINK% -flto"
  set "TOOLLINK=%TOOLLINK% -flto"
)

set "UNICODE_CTYPECC=%GCC% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%GCC% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%GCC% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%GCC% -c %CMNOPTS% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%GCC% -c %CMNOPTS% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%GCC% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"

set CC_DEFINE=-D
set CC_INCLUDE=-I

if defined DO_BUILD_SHLIB call :getdllstartup GCC || exit /b

goto :do_build

:gcc_awk_ld
call :execq "%GAWKLINK:^^=^% -o dist\%~1 %~2 -lws2_32"
exit /b

:gcc_shlib_ld
call :execq "%SHLIBLINK:^^=^% -e%SHLIBDLLMAIN% -o dist\%~1 %~2"
exit /b

:gcc_tool_ld
call :execq "%TOOLLINK:^^=^% -o helpers\%~1 %~2"
exit /b

::::::::::::::::::::: GCC END :::::::::::::::::::

:toolchain2
if not "%TOOLCHAIN%"=="clang" goto :toolchain3

:::::::::::::::::: CLANG (MINGW) ::::::::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined CLANG   (set "CLANG=clang %BUILDFORCPU%")     else call :escape_cc CLANG "%BUILDFORCPU%"
if not defined CLANGXX (set "CLANGXX=clang++ %BUILDFORCPU%") else call :escape_cc CLANGXX "%BUILDFORCPU%"
if not defined AR      (set AR=ar)                           else call :escape_cc AR
if not defined LLVMAR  (set LLVMAR=llvm-ar)                  else call :escape_cc LLVMAR

if not defined CLANGVER (
  call :extract_ver CLANGVER "%CLANG% --version 2>&1" "clang version " || (
    echo Couldn't determine clang compiler version, please specify CLANGVER explicitly ^(e.g. CLANGVER=10001 for 10.1^)
    exit /b 1
  )
)

set "CMN_DEFINES=-fstrict-aliasing -DNEED_C99_FEATURES"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% --analyze"
)

:: assume "%zu" printf-format specifier is supported by the underlying CRT library
set "CMN_DEFINES=%CMN_DEFINES% -D__USE_MINGW_ANSI_STDIO"

if not defined BUILD_PEDANTIC (
  set "WARNING_OPTIONS=-Wall"
  goto :clang_no_pedantic
)

:: everything
set WARNING_OPTIONS=-Weverything

:: disable some warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-padded"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-used-but-marked-unused"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-reserved-id-macro"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-implicit-fallthrough"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-switch-enum"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-covered-switch-default"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-unused-macros"

:clang_no_pedantic

if not defined COMPILE_AS_CXX goto :clang_no_cxx

:: compile as c++11
set "CMN_DEFINES=-x c++ -std=c++11 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :clang_do_build

:: disable some c++11 warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-old-style-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-zero-as-null-pointer-constant"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-c++98-compat-pedantic"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-global-constructors"
goto :clang_do_build

:clang_no_cxx

:: compile as c99
set "CMN_DEFINES=-std=c99 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :clang_do_build

:: disable some c99 warnings

:clang_do_build

:: gawk-specific defines
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/FI=-include %"
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/=-%"

:: define HAVE___INLINE for support/libc-config.h
:: define HAVE___RESTRICT for support/libc-config.h
if defined COMPILE_AS_CXX set "GAWK_DEFINES=%GAWK_DEFINES% -DHAVE___INLINE -DHAVE___RESTRICT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -gcodeview -fstack-protector-all"
) else (
  :: release options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
)

if defined COMPILE_AS_CXX (
  set "GAWKLINK=%CLANGXX% -static -municode -mconsole"
  set "SHLIBLINK=%CLANGXX% -static -municode -mconsole -mdll"
  set "TOOLLINK=%CLANGXX% -static -municode -mconsole"
) else (
  set "GAWKLINK=%CLANG% -static -municode -mconsole"
  set "SHLIBLINK=%CLANG% -static -municode -mconsole -mdll"
  set "TOOLLINK=%CLANG% -static -municode -mconsole"
)

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%AR% -csr "
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all"
) else (
  set "LIBABSTRACTLIB=%LLVMAR% -csr "
  set "GAWKLINK=%GAWKLINK% -flto -fuse-ld=lld"
  set "SHLIBLINK=%SHLIBLINK:-mdll=-shared% -flto -fuse-ld=lld"
  set "TOOLLINK=%TOOLLINK% -flto -fuse-ld=lld"
)

set "UNICODE_CTYPECC=%CLANG% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%CLANG% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%CLANG% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%CLANG% -c %CMNOPTS% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%CLANG% -c %CMNOPTS% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%CLANG% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"

set CC_DEFINE=-D
set CC_INCLUDE=-I

if defined DO_BUILD_SHLIB call :getdllstartup CLANG || exit /b

goto :do_build

:clang_awk_ld
call :execq "%GAWKLINK:^^=^% -o dist\%~1 %~2 -lws2_32"
exit /b

:clang_shlib_ld
call :execq "%SHLIBLINK:^^=^% -e%SHLIBDLLMAIN% -o dist\%~1 %~2"
exit /b

:clang_tool_ld
call :execq "%TOOLLINK:^^=^% -o helpers\%~1 %~2"
exit /b

:::::::::::::::: CLANG (MINGW) END ::::::::::::::

:toolchain3
if not "%TOOLCHAIN%"=="clangmsvc" goto :toolchain4

:::::::::::::::::: CLANG-MSVC :::::::::::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined CLANGMSVC   (set "CLANGMSVC=clang %BUILDFORCPU%")     else call :escape_cc CLANGMSVC "%BUILDFORCPU%%"
if not defined CLANGMSVCXX (set "CLANGMSVCXX=clang++ %BUILDFORCPU%") else call :escape_cc CLANGMSVCXX "%BUILDFORCPU%"
if not defined CLLIB       (set CLLIB=lib)                           else call :escape_cc CLLIB
if not defined LLVMAR      (set LLVMAR=llvm-ar)                      else call :escape_cc LLVMAR

if not defined CLANGMSVCVER (
  call :extract_ver CLANGMSVCVER "%CLANGMSVC% --version 2>&1" "clang version " || (
    echo Couldn't determine clang-msvc compiler version, please specify CLANGMSVCVER explicitly ^(e.g. CLANGMSVCVER=11023 for 11.23.6^)
    exit /b 1
  )
)

:: define _Noreturn for pc/config.h & support/cdefs.h
:: define __inline for gettext.h
set "CMN_DEFINES=-fstrict-aliasing -DNEED_C99_FEATURES -D_Noreturn=_Noreturn -D__inline=__inline"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% --analyze"
)

:: disable MSVC deprecation warnings
set "NODEPRECATE=-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE -D_WINSOCK_DEPRECATED_NO_WARNINGS"

if not defined BUILD_PEDANTIC (
  set "WARNING_OPTIONS=-Wall"
  goto :clangmsvc_no_pedantic
)

:: everything
set WARNING_OPTIONS=-Weverything

:: disable some warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-padded"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-used-but-marked-unused"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-reserved-id-macro"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-implicit-fallthrough"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-switch-enum"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-covered-switch-default"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-unused-macros"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-nonportable-system-include-path"

:clangmsvc_no_pedantic

if not defined COMPILE_AS_CXX goto :clangmsvc_no_cxx

:: compile as c++11
set "CMN_DEFINES=-x c++ -std=c++11 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :clangmsvc_do_build

:: disable some c++11 warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-old-style-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-zero-as-null-pointer-constant"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-c++98-compat-pedantic"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-global-constructors"
goto :clangmsvc_do_build

:clangmsvc_no_cxx

:: compile as c99
set "CMN_DEFINES=-std=c99 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :clangmsvc_do_build

:: disable some c99 warnings

:clangmsvc_do_build

:: gawk-specific defines
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/FI=-include %"
if defined GAWK_DEFINES set "GAWK_DEFINES=%GAWK_DEFINES:/=-%"

:: define _Restrict_ for support/regex.h
if defined COMPILE_AS_CXX set "GAWK_DEFINES=%GAWK_DEFINES% -D_Restrict_=__restrict"

:: define HAVE___INLINE for support/libc-config.h
:: define HAVE___RESTRICT for support/libc-config.h
if defined COMPILE_AS_CXX set "GAWK_DEFINES=%GAWK_DEFINES% -DHAVE___INLINE -DHAVE___RESTRICT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -gcodeview -fstack-protector-all"
) else (
  :: release options
  set "CMNOPTS=%NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
)

if defined COMPILE_AS_CXX (
  set "GAWKLINK=%CLANGMSVCXX% -static -municode -mconsole"
  set "SHLIBLINK=%CLANGMSVCXX% -static -municode -mconsole -shared"
  set "TOOLLINK=%CLANGMSVCXX% -static -municode -mconsole"
) else (
  set "GAWKLINK=%CLANGMSVC% -static -municode -mconsole"
  set "SHLIBLINK=%CLANGMSVC% -static -municode -mconsole -shared"
  set "TOOLLINK=%CLANGMSVC% -static -municode -mconsole"
)

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%CLLIB% /nologo /OUT:"
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all -Wl,/DEBUG -Wl,/INCREMENTAL:NO -Wl,/SUBSYSTEM:CONSOLE -Wl,/Entry:ExtDllMain -Wl,/NODEFAULTLIB:libcmt -Wl,/DEFAULTLIB:libcmtd -Wl,/DEFAULTLIB:kernel32 -Wl,/DEFAULTLIB:libvcruntimed -Wl,/DEFAULTLIB:libucrtd"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all"
) else (
  set "LIBABSTRACTLIB=%LLVMAR% -csr "
  set "GAWKLINK=%GAWKLINK% -flto -fuse-ld=lld"
  set "SHLIBLINK=%SHLIBLINK% -flto -fuse-ld=lld"
  set "TOOLLINK=%TOOLLINK% -flto -fuse-ld=lld"
)

set "UNICODE_CTYPECC=%CLANGMSVC% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%CLANGMSVC% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%CLANGMSVC% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%CLANGMSVC% -c %CMNOPTS% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%CLANGMSVC% -c %CMNOPTS% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%CLANGMSVC% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"

set CC_DEFINE=-D
set CC_INCLUDE=-I

goto :do_build

:clangmsvc_awk_ld
call :execq "%GAWKLINK:^^=^% -o dist\%~1 %~2 -lws2_32"
exit /b

:clangmsvc_shlib_ld
call :execq "%SHLIBLINK:^^=^% -o dist\%~1 %~2"
exit /b

:clangmsvc_tool_ld
call :execq "%TOOLLINK:^^=^% -o helpers\%~1 %~2"
exit /b

:::::::::::::::::: CLANG-MSVC END :::::::::::::::

:toolchain4
:: add more toolchains here

::::: BUILDING/TESTING :::::
:do_build

set CALL_STAT=0

if defined DO_BUILD_AWK (
  if not exist dist call :exec md dist || goto :build_err
  call :unicode_ctype || goto :build_err
  call :libutf16      || goto :build_err
  call :mscrtx        || goto :build_err
  call :support       || goto :build_err
  call :gawk          || goto :build_err
)
if defined DO_BUILD_SHLIB (
  if not exist dist call :exec md dist || goto :build_err
  call :shlibs  || goto :build_err
)
if defined DO_BUILD_HELPERS (
  call :unicode_ctype || goto :build_err
  call :libutf16      || goto :build_err
  call :mscrtx        || goto :build_err
  call :helpers       || goto :build_err
)
if defined DO_TEST (
  call :tests   || goto :test_err
)

echo CALL_STAT=%CALL_STAT%
echo OK!
exit /b 0

:::::: COMPILATION :::::
:unicode_ctype

call :cc UNICODE_CTYPECC "%UNICODE_CTYPE%\src\unicode_ctype.c"     || exit /b
call :cc UNICODE_CTYPECC "%UNICODE_CTYPE%\src\unicode_toupper.c"   || exit /b

call :lib "unicode_ctype.a" ^
 "%UNICODE_CTYPE%\src\unicode_ctype.obj"   ^
 "%UNICODE_CTYPE%\src\unicode_toupper.obj"  || exit /b

exit /b 0

:libutf16

call :cc LIBUTF16CC "%LIBUTF16%\src\utf16_to_utf8.c"     || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf8_to_utf16.c"     || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf16_to_utf8_one.c" || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf8_to_utf16_one.c" || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf8_cstd.c"         || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf16_to_utf32.c"    || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf32_to_utf16.c"    || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf8_to_utf32.c"     || exit /b
call :cc LIBUTF16CC "%LIBUTF16%\src\utf32_to_utf8.c"     || exit /b

call :lib "libutf16.a" ^
 "%LIBUTF16%\src\utf16_to_utf8.obj"     ^
 "%LIBUTF16%\src\utf8_to_utf16.obj"     ^
 "%LIBUTF16%\src\utf16_to_utf8_one.obj" ^
 "%LIBUTF16%\src\utf8_to_utf16_one.obj" ^
 "%LIBUTF16%\src\utf8_cstd.obj"         ^
 "%LIBUTF16%\src\utf16_to_utf32.obj"    ^
 "%LIBUTF16%\src\utf32_to_utf16.obj"    ^
 "%LIBUTF16%\src\utf8_to_utf32.obj"     ^
 "%LIBUTF16%\src\utf32_to_utf8.obj"  || exit /b

exit /b 0

:mscrtx

call :cc MSCRTXCC "%MSCRTX%\src\arg_parser.c"     || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\socket_fd.c"      || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\socket_file.c"    || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\wreaddir.c"       || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\wreadlink.c"      || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\xstat.c"          || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\locale_helpers.c" || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\localerpl.c"      || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\utf16cvt.c"       || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\utf8env.c"        || exit /b
call :cc MSCRTXCC "%MSCRTX%\src\utf8rpl.c"        || exit /b

call :lib "mscrtx.a" ^
 "%MSCRTX%\src\arg_parser.obj"     ^
 "%MSCRTX%\src\socket_fd.obj"      ^
 "%MSCRTX%\src\socket_file.obj"    ^
 "%MSCRTX%\src\wreaddir.obj"       ^
 "%MSCRTX%\src\wreadlink.obj"      ^
 "%MSCRTX%\src\xstat.obj"          ^
 "%MSCRTX%\src\locale_helpers.obj" ^
 "%MSCRTX%\src\localerpl.obj"      ^
 "%MSCRTX%\src\utf16cvt.obj"       ^
 "%MSCRTX%\src\utf8env.obj"        ^
 "%MSCRTX%\src\utf8rpl.obj"          || exit /b

exit /b 0

:support

call :cc GAWKCC support\getopt.c     || exit /b
call :cc GAWKCC support\getopt1.c    || exit /b
call :cc GAWKCC support\random.c     || exit /b
call :cc GAWKCC support\dfa.c        || exit /b
call :cc GAWKCC support\localeinfo.c || exit /b
call :cc GAWKCC support\regex.c      || exit /b

call :lib support\libsupport.a ^
 support\getopt.obj     ^
 support\getopt1.obj    ^
 support\random.obj     ^
 support\dfa.obj        ^
 support\localeinfo.obj ^
 support\regex.obj         || exit /b

exit /b 0

:gawk

call :cc GAWKCC array.c             || exit /b
call :cc GAWKCC awkgram.c           || exit /b
call :cc GAWKCC builtin.c           || exit /b
call :cc GAWKCC pc\popen.c          || exit /b
call :cc GAWKCC pc\getid.c          || exit /b
call :cc GAWKCC cint_array.c        || exit /b
call :cc GAWKCC command.c           || exit /b
call :cc GAWKCC debug.c             || exit /b
call :cc GAWKCC eval.c              || exit /b
call :cc GAWKCC ext.c               || exit /b
call :cc GAWKCC field.c             || exit /b
call :cc GAWKCC floatcomp.c         || exit /b
call :cc GAWKCC gawkapi.c           || exit /b
call :cc GAWKCC gawkmisc.c          || exit /b
call :cc GAWKCC int_array.c         || exit /b
call :cc GAWKCC io.c                || exit /b
call :cc GAWKCC main.c              || exit /b
call :cc GAWKCC mpfr.c              || exit /b
call :cc GAWKCC msg.c               || exit /b
call :cc GAWKCC node.c              || exit /b
call :cc GAWKCC profile.c           || exit /b
call :cc GAWKCC re.c                || exit /b
call :cc GAWKCC replace.c           || exit /b
call :cc GAWKCC str_array.c         || exit /b
call :cc GAWKCC symbol.c            || exit /b
call :cc GAWKCC version.c           || exit /b

call :ld awk gawk.exe ^
 array.obj             ^
 awkgram.obj           ^
 builtin.obj           ^
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
 support\libsupport.a mscrtx.a libutf16.a unicode_ctype.a || exit /b

exit /b 0

:shlibs

call :shlib testext        || exit /b
call :shlib readdir        || exit /b
call :shlib readdir_test %CC_DEFINE%READDIR_TEST readdir.c || exit /b
call :shlib inplace        || exit /b
call :shlib time           || exit /b

call :shlib_cc stack       || exit /b
call :shlib_cc gawkfts "%CC_DEFINE%FTS_ALIGN_BY=sizeof^(void*^)" || exit /b
call :shlib_cc filefuncs   || exit /b

call :shlib_ld filefuncs ^
 extension\stack.obj     ^
 extension\gawkfts.obj   ^
 extension\filefuncs.obj || exit /b

call :shlib rwarray        || exit /b
call :shlib rwarray0       || exit /b
call :shlib revoutput      || exit /b
call :shlib revtwoway      || exit /b
call :shlib readfile       || exit /b
call :shlib ordchr         || exit /b
call :shlib intdiv         || exit /b
call :shlib fnmatch %CC_INCLUDE%missing_d || exit /b

:: not ported yet...
::call :shlib fork         || exit /b

exit /b 0

:shlib
:: %1 - extension DLL name
:: %2 - (optional) additional compiler options
:: %3 - (optional) name of source file
call :shlib_cc "%~1" "%~2" "%~3"         || exit /b
call :shlib_ld "%~1" "extension\%~1.obj" || exit /b
exit /b 0

:shlib_cc
:: %1 - object file base name
:: %2 - (optional) additional compiler options
:: %3 - (optional) source file name
setlocal & set CALL_STAT=0
:: %~2 doubles ^ in the options, un-double it
set "OPTIONS=%~2"
if defined OPTIONS set "OPTIONS=%OPTIONS:^^=^%"
if ""=="%~3" (
  call :cc SHLIBCC "extension\%~1.c" "%OPTIONS%"
) else (
  call :cc SHLIBCC "extension\%~3" "%OPTIONS%" "extension\%~1.obj"
)
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:shlib_ld
:: %1 - dll name
:: %2, %3, ... - object file(s) (pathnames)
call :ld shlib %~1.dll %~2 %~3 %~4 %~5 %~6 %~7 %~8 %~9
exit /b

:helpers
call :cc TOOLCC helpers\tst_ls.c || exit /b
call :ld tool tst_ls.exe ^
 helpers\tst_ls.obj      ^
 mscrtx.a libutf16.a unicode_ctype.a || exit /b
exit /b 0

:::::: TESTSUITE :::::
:tests

::NOTE: use 3 chars {"\"} for escaping a double quote:
::1) MSVC version of gawk.exe allows escaping either by two double quotes or a backslash, e.g.:
::  gawk.exe "BEGIN{print""a""}"
::  gawk.exe "BEGIN{print\"a\"}"
::2) But MINGW version, allows escaping only by a backslash.
::3) Cmd.exe counts double quotes and text between them interprets as commands, e.g.:
::  gawk.exe "BEGIN{print\"&echo \"}"  -> will print \"}"
::4) So a double-quote must be escaped by two double quotes: "" or "\":
::  gawk.exe "BEGIN{print"\"&echo "\"}"  -> will print \"}"

echo.
echo.===============================================================
echo.Any output from FC is bad news, although some differences
echo.in floating point values are probably benign -- in particular,
echo.some systems may omit a leading zero and the floating point
echo.precision may lead to slightly different output in a few cases.
echo.===============================================================

setlocal & set CALL_STAT=0

set TESTS_UNSUPPORTED=0
set TESTS_PARTIAL=0
set TESTS_SKIPPED=0

:: Use only standard windows utilities
call :execq "set ""Path=%WINDIR%\system32;%WINDIR%;%WINDIR%\System32\Wbem"""

:: don't call abort() in gawk.exe - it shows a message box window
call :exec set GAWKTESTING=1 || goto :exit_local

call :change_locale C || goto :exit_local
dist\gawk.exe -f "test\printlang.awk" || ((echo.failed to execute: dist\gawk.exe -f "test\printlang.awk") & goto :exit_local)

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
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="shlib" goto :try_extra_tests)
echo.======== Starting shared library tests ========
call :shlib_tests           || exit /b
echo.======== Done with shared library tests ========

:try_extra_tests
if defined DO_TEST_ONLY (if not "%DO_TEST_ONLY%"=="extra" goto :end_of_tests)
echo.======== Starting extra tests ========
call :extra_tests           || exit /b
echo.======== Done with extra tests ========

:end_of_tests

if defined DO_TEST_ONLY (
  if not "%DO_TEST_ONLY%"=="basic" (
  if not "%DO_TEST_ONLY%"=="unix" (
  if not "%DO_TEST_ONLY%"=="ext" (
  if not "%DO_TEST_ONLY%"=="cpu" (
  if not "%DO_TEST_ONLY%"=="locale" (
  if not "%DO_TEST_ONLY%"=="shlib" (
  if not "%DO_TEST_ONLY%"=="extra" (
    echo.ERROR: unknown sub-test name: "%DO_TEST_ONLY%"
    exit /b 1
))))))))

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

setlocal & set CALL_STAT=0
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :runtest         posix_compare --posix                   || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

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
call :runtest_in      strfieldnum                             || exit /b
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
call :execq "wmic process call create CommandLine=""\""%CD%\%GAWK%\"" -f .\winpid.awk"" CurrentDirectory=""%CD%"" 2>&1 | %FIND% ""ProcessId"" > _winpid" || exit /b
call :waitfor winpid.done                                     || exit /b
call :cmpdel winpid                                           || exit /b
call :exec del /q winpid.ok winpid.done                       || exit /b

call :runtest pipeio1 && call :exec del /q test1 test2        || exit /b

setlocal & set CALL_STAT=0
set OK_SUFFIX=_win
call :runtest pipeio2 "-vSRCDIR=."                            || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT%

:: UNSUPPORTED test: poundbang - cannot be ported to windows
set /A TESTS_UNSUPPORTED+=1

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
call :execq "%GAWK% -f 2>&1 | %FIND% /v ""patchlevel"" | %FIND% /v ""--parsedebug"" > _badargs" && ^
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
call :execq "fc /b _charasbytes1 _charasbytes2 | %FIND% ""0000"" > _charasbytes"           || goto :exit_local
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

call :execq_fail "%GAWK% -f clos1way5.awk > clos1way5.tmp 2>&1" clos1way5.tmp && ^
call :execq "type clos1way5.tmp | %FIND% /V ""The process tried to write to a nonexistent pipe."" > _clos1way5" && ^
call :cmpdel clos1way5 && call :exec del clos1way5.tmp        || exit /b

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
set /A TESTS_UNSUPPORTED+=3

call :runtest_in      dfacheck1                               || exit /b

call :execq "%GAWK% --dump-variables 1 < dumpvars.in > NUL" && ^
call :execq "%FIND% /v ""ENVIRON"" < awkvars.out | %FIND% /v ""PROCINFO"" > _dumpvars" && ^
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
call :runtest         lintplus   --lint                       || exit /b
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
set /A TESTS_UNSUPPORTED+=2

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
call :execq "type __symtab8 | %FIND% /v ""ENVIRON"" | %FIND% /v ""PROCINFO"" | %FIND% /v ""FILENAME"" >> _symtab8" || exit /b
call :cmpdel symtab8 && call :exec del /q __symtab8           || exit /b

call :runtest           symtab9                               || exit /b
call :exec del /q testit.txt                                  || exit /b

call :runtest_fail_in   symtab10 --debug                      || exit /b
call :runtest           symtab11                              || exit /b

:: UNSUPPORTED test: reading files with timeout is not supported yet on windows
::call :runtest           timeout                               || exit /b
set /A TESTS_UNSUPPORTED+=1

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

(echo.%GAWK_DEFINES%) | %FIND% "ARRAYDEBUG" > NUL || ^
rem. && (
  set /A TESTS_SKIPPED+=1
  (echo gawk was not compiled to support the array debug tests)
  goto :skip_arr_test
)

call :runtest           arrdbg -v "okfile=arrdbg.ok" "| %FIND% ""array_f""" && ^
call :exec del /q arrdbg.ok                                   || exit /b

:skip_arr_test
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
  set /A TESTS_SKIPPED+=1
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
  set /A TESTS_SKIPPED+=1
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
call :exec set GAWKWINLOCALE=UTF8
call :runtest           fnmatch_glibc "--locale=en_US.UTF-8"  || goto :exit_local
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
set /A TESTS_UNSUPPORTED+=2

call :execq "%GAWK% -f fts.awk" && ^
call :cmpdel fts && call :exec del /q fts.ok                  || exit /b

call :runtest           functab4                              || exit /b

call :execq "%GAWK% -f getfile.awk -v ""TESTEXT_QUIET=1"" -ltestext < getfile.awk > _getfile 2>&1" || exit /b
call :execq "type _getfile   | %FIND% /v ""get_file:""  > ap-getfile.out" || exit /b
call :execq "type _getfile   | %FIND%    ""get_file:"" >> ap-getfile.out" || exit /b
call :execq "type getfile.ok | %FIND% /v ""get_file:""  > getfile.ok1"    || exit /b
call :execq "type getfile.ok | %FIND%    ""get_file:"" >> getfile.ok1"    || exit /b
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

setlocal & set CALL_STAT=0 & set TESTS_PARTIAL=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -f readdir.awk ""%top_srcdir%"" > _readdir" || goto :exit_local
call :execq "..\helpers\tst_ls -afi ""%top_srcdir%"" > _dirlist"   || goto :exit_local
call :execq "..\helpers\tst_ls -lna ""%top_srcdir%"" > _longlist1" || goto :exit_local
call :execq "more +1 _longlist1 > _longlist"                  || goto :exit_local
call :execq "%GAWK% -f readdir0.awk -v extout=_readdir -v dirlist=_dirlist -v longlist=_longlist > readdir.ok" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir && call :exec del /q readdir.ok _dirlist _longlist1 _longlist || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT% & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

setlocal & set CALL_STAT=0 & set TESTS_PARTIAL=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -lreaddir -F/ ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%top_srcdir%"" > readdir_test.ok" || goto :exit_local
call :execq "%GAWK% -lreaddir_test ""{printf """"[%%%%s] [%%%%s] [%%%%s] [%%%%s]\n"""", $1, $2, $3, $4}"" ""%top_srcdir%"" > _readdir_test" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir_test && call :exec del /q readdir_test.ok || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT% & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

setlocal & set CALL_STAT=0 & set TESTS_PARTIAL=0
call :get_top_srcdir
call :change_locale "en_US.UTF-8"                             || goto :exit_local
call :execq "mklink ""%top_srcdir%%\FFF"" f1\f2\f3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic file link, ignoring)
call :execq "mklink ""%top_srcdir%%\DDD"" /D d1\d2\d3 2>NUL" || (set /A TESTS_PARTIAL+=1 & echo can't create symbolic directory link, ignoring)
call :execq "%GAWK% -lreaddir -F/ -f readdir_retest.awk ""%top_srcdir%"" > readdir_retest.ok" || goto :exit_local
call :execq "%GAWK% -lreaddir_test -F/ -f readdir_retest.awk ""%top_srcdir%"" > _readdir_retest" || goto :exit_local
call :execq "del /q ""%top_srcdir%%\FFF"" 2>NUL"
call :execq "rd /q ""%top_srcdir%%\DDD"" 2>NUL"
call :cmpdel readdir_retest && call :exec del /q readdir_retest.ok || goto :exit_local
endlocal & set /A CALL_STAT+=%CALL_STAT% & set /A TESTS_PARTIAL+=%TESTS_PARTIAL%

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

:::::: EXTRA TESTS :::::
:extra_tests

call :execq "%GAWK% -f inftest.awk > _inftest"                || exit /b
call :execq "more inftest.ok > _inftest.ok"                   || exit /b
call :cmpdel_ _inftest.ok _inftest /c && call :exec del _inftest.ok || exit /b

::regtest
call :execq "%GAWK% -f reg\exp-eq.awk < reg\exp-eq.in > reg\_exp-eq 2>&1" && ^
call :cmpdel_ reg\exp-eq.good reg\_exp-eq                     || exit /b

call :execq "(for /f ""tokens=1* delims=/"" %%%%a in (reg\func.good) do (echo %%%%a\%%%%b)) > reg\_func.good" || exit /b
call :execq "%GAWK% -f reg\func.awk < reg\func.in > reg\_func 2>&1 && cmd /c ""exit /b 1"" || cmd /c ""exit /b 0""" && ^
call :cmpdel_ reg\_func.good reg\_func && call :exec del reg\_func.good || exit /b

call :execq "cmd /s /c ""for /f ""tokens=1* delims=/"" %%%%a in ^(reg\func2.good^) do @^(^(echo %%%%a\%%%%b^)^&exit /b^)"" > reg\_func2.good" || exit /b
call :execq "more +1 reg\func2.good >> reg\_func2.good" || exit /b
call :execq "%GAWK% -f reg\func2.awk < reg\func2.in > reg\_func2 2>&1 && cmd /c ""exit /b 1"" || cmd /c ""exit /b 0""" && ^
call :cmpdel_ reg\_func2.good reg\_func2 && call :exec del reg\_func2.good || exit /b

:: UNSUPPORTED
:: this test fails under Windows - lower case greek letter 0xf2 (Greek_Greece.28597),
:: when converting to upper case, do not maps to 0xd3 (Greek_Greece.28597)
::setlocal & set CALL_STAT=0
::call :change_locale "Greek_Greece.28597"                      || goto :exit_local
::call :runtest ignrcas3                                        || goto :exit_local
::endlocal & set /A CALL_STAT+=%CALL_STAT%
set /A TESTS_UNSUPPORTED+=1

call :inet_test  9 DISCARD inetdis || exit /b
call :inet_test 13 DAYTIME inetday || exit /b
call :inet_test  7 DAYTIME inetech || exit /b

exit /b 0

:inet_test
:: %1 - 7, 9, 13
:: %2 - ECHO, DISCARD, DAYTIME
:: %3 - inetdis, inetday, inetech
call :checksvc udp 4 "0.0.0.0" %1 %2 && (call :%3 udp "127.0.0.1" || exit /b)
call :checksvc udp 6 "[::]"    %1 %2 && (call :%3 udp "[::1]" 6   || exit /b)
call :checksvc tcp 4 "0.0.0.0" %1 %2 && (call :%3 tcp "127.0.0.1" || exit /b)
call :checksvc tcp 6 "[::]"    %1 %2 && (call :%3 tcp "[::1]" 6   || exit /b)
exit /b 0

:inetdis
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - <empty> or 6
call :execq "%GAWK% ""BEGIN { print """"%1:%~2"""" ^|^& """"/inet%3/%1/0/%~2/9""""}"" > _inetdis 2>&1" || exit /b
call :execq "rem.>inetdis.ok" && call :cmpdel inetdis && call :exec del /q inetdis.ok || exit /b
exit /b

:inetday
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - <empty> or 6
call :execq "%GAWK% ""BEGIN { print """""""" ^|^& """"/inet%3/%1/0/%~2/13""""; """"/inet%3/%1/0/%~2/13"""" ^|^& getline; print $0}"" > _inetday" || exit /b
call :execq "type _inetday | %FIND% /v """"" && call :exec del /q _inetday || exit /b
exit /b

:inetech
:: %1 - udp, tcp
:: %2 - "127.0.0.1" or "[::1]"
:: %3 - <empty> or 6
call :execq "%GAWK% ""BEGIN { print """"[%1 %~2 %3]"""" ^|^& """"/inet%3/%1/0/%~2/7""""; """"/inet%3/%1/0/%~2/7"""" ^|^& getline; print $0}"" > _inetech" || exit /b
call :execq "(echo.[%1 %~2 %3]) > inetech.ok" && call :cmpdel inetech && call :exec del /q inetech.ok || exit /b
exit /b

:checksvc
:: %1 - udp, tcp
:: %2 - 4 or 6
:: %3 - "0.0.0.0" or "[::]"
:: %4 - 7, 9, 13
:: %5 - ECHO, DISCARD, DAYTIME
(for /f "tokens=1,2" %%a in ('netstat -na') do (echo [%%a:%%b])) | %FIND% /i "[%1:%~3:%4]" >NUL && exit /b
echo no %1 %5 service on local IPv%2 port %4, skipping test
set /A TESTS_SKIPPED+=1
exit /b 1

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
for /f "tokens=* delims=" %%a in ("%CD%") do set "top_srcdir=%%~dpa"
set "top_srcdir=%top_srcdir:~0,-1%"
exit /b

:cc
:: compiler
:: %1 - GAWKCC, SHLIBCC, TOOLCC ...
:: %2 - support\getopt.c
:: %3 - (optional) additional compiler options
:: %4 - (optional) support\getopt.obj - name of object file
setlocal & set CALL_STAT=0
set "BASENAME=%~2"
call set "CC=%%%~1%%"
:: %~3 doubles ^ in the options, un-double it
set "OPTIONS=%~3"
if defined OPTIONS set "OPTIONS=""%OPTIONS:^^=^%"""
if ""=="%~4" (
  call :execq "%CC:^^=^%""%BASENAME:.c=.obj%"" ""%~2"" %OPTIONS%"
) else (
  call :execq "%CC:^^=^%""%~4"" ""%~2"" %OPTIONS%"
)
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:lib
:: static library linker
:: %* - support\libsupport.a support\getopt.obj support\getopt1.obj ...
setlocal & set CALL_STAT=0
set "LIBOBJS=|%*"
call set "LIBOBJS=%%LIBOBJS:|%1 =%%"
call :execq "%LIBABSTRACTLIB:^^=^%""%~1"" %LIBOBJS:"=""%"
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:ld
:: exe/shared library linker
:: %* - awk gawk.exe array.obj ...
:: %* - shlib testext.dll extension\testext.obj ...
setlocal & set CALL_STAT=0
set "LDOBJS=|%*"
call set "LDOBJS=%%LDOBJS:|%1 =|%%"
call set "LDOBJS=%%LDOBJS:|%2 =%%"
:: call cl_awk_ld, cl_shlib_ld, cl_tool_ld, gcc_awk_ld, gcc_shlib_ld, gcc_tool_ld ...
call :%TOOLCHAIN%_%~1_ld "%~2" "%LDOBJS:"=""%"
endlocal & set /A CALL_STAT+=%CALL_STAT% & exit /b

:checkoptsuniq
setlocal
set COUNT1=0
set COUNT2=0
for %%a in (%*) do (
  set /a COUNT1+=1
  for %%b in (%*) do (
    if "%%a"=="%%b" set /a COUNT2+=1
  )
)
if not %COUNT1%==%COUNT2% (endlocal & exit /b 1)
endlocal & exit /b 0

:checkbuildopt
if "%~1"==""           exit /b
if "%~1"=="cl"         exit /b
if "%~1"=="gcc"        exit /b
if "%~1"=="clang"      exit /b
if "%~1"=="clang-msvc" exit /b
if "%~1"=="c++"        exit /b
if "%~1"=="pedantic"   exit /b
if "%~1"=="debug"      exit /b
if "%~1"=="analyze"    exit /b
if "%~1"=="32"         exit /b
if "%~1"=="64"         exit /b
:badbuildopts
echo ERROR: bad build options
echo.
exit /b 1

:isoptionset
if "%~3"=="%~1" exit /b 0
if "%~4"=="%~1" exit /b 0
if "%~5"=="%~1" exit /b 0
if "%~6"=="%~1" exit /b 0
if "%~7"=="%~1" exit /b 0
if "%~8"=="%~1" exit /b 0
exit /b 1

:escape_cc
:: %1 - CLANGMSVC, CLANGMSVCXX, LLVMAR
:: %2 - (optional) "%BUILDFORCPU%"
:: replace one double-quote with two double-quotes
call set "%1=%%%1:"=""%% %~2"
:: special characters in double quotes must be escaped by ^ (and ^ must not be at end of the line)
call set "%1=%%%1:(=^(%%"
call set "%1=%%%1:)=^)%%"
exit /b

:combine_ver
:: 9.3 -> 9003
setlocal
call set "VER=%%%1%%"
if not defined VER (endlocal & exit /b 1)
for /f "tokens=1,2 delims=." %%a in ("%VER%") do (
  set "VER=%%a"
  set "VERMINOR=%%b"
)
if not defined VERMINOR set VERMINOR=0
set "VERMINOR=00%VERMINOR%"
set "VER=%VER%%VERMINOR:~-3,3%"
endlocal & set "%1=%VER%" & exit /b 0

:extract_ver2
:: %1 - prefix string
:: %2 - some text
set "TEXT=%~2"
call set "TEXT=%%TEXT:%~1=%%"
if "%TEXT%"=="%~2" exit /b 1
for /f "tokens=1" %%b in ("%TEXT%") do (set "VER=%%b")
call :combine_ver VER
exit /b

:extract_ver1
:: %1 - "%CLANGMSVC% --version 2>&1"
:: %2 - "clang version "
set "CCCMD=%~1"
:: %~1 doubles ^ in the options, un-double it
set "CCCMD=%CCCMD:^^^^^^^^=^%"
set "CCCMD=%CCCMD:<=^<%"
set "CCCMD=%CCCMD:>=^>%"
set "CCCMD=%CCCMD:&=^&%"
set "CCCMD=%CCCMD:(=^(%"
set "CCCMD=%CCCMD:)=^)%"
set "CCCMD=%CCCMD:^^=%"
for /f "tokens=*" %%a in ('%CCCMD:""="%') do (call :extract_ver2 %2 "%%a" && exit /b 0)
exit /b 1

:extract_ver
:: %1 - CLANGMSVCVER
:: %2 - "%CLANGMSVC% --version 2>&1"
:: %3 - "clang version "
setlocal
call :extract_ver1 %2 %3 || exit /b 1
endlocal & set "%1=%VER%" & exit /b 0

:getdllstartup
:: %1 - GCC, CLANG
call :getdllstartup1 %1
if exist aaa.dll (del /f aaa.dll && exit /b)
echo failed to determine dll startup function
exit /b 1

:getdllstartup1
set "SHLIBDLLMAIN=_ExtDllMain@12"
call set "COMMAND=%%%1:""="%%"
for /f "tokens=* delims=" %%a in ('%%COMMAND%% --verbose -mdll -x c -o aaa.dll - ^<NUL 2^>^&1') do (call :checkdllmaincrt %%a && exit /b)
set "SHLIBDLLMAIN=ExtDllMain"
exit /b

:checkdllmaincrt
set "COMMAND=%*"
if not defined COMMAND exit /b 1
set "COMMAND=%COMMAND:"=""%"
if "%COMMAND:_DllMainCRTStartup@12=%"=="%COMMAND%" exit /b 1
exit /b 0
