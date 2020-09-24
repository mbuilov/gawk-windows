:: Windows Gawk build script.
:: Written by Michael M. Builov (mbuilov@gmail.com)
:: Public domain.

:: Note: line ends _must_ be CRLF, otherwise cmd.exe may interpret script incorrectly.

@echo off
setlocal

:: Note: these environment variables are taken in account (default value in parents):
::
:: BLD_DIST     (dist)       - directory for build artifacts
:: BLD_OBJ      (obj)        - directory for intermediate build files
::
:: CLCC         (cl)         - how to call cl, this may be for example "cl"
:: CLVER        {auto}       - cl version, e.g. for cl version 19.26.28806 CLVER must be set to 19026
:: CLLIB        (lib)        - how to call lib, this may be for example "lib"
:: CLLINK       (link)       - how to call link, this may be for example "link"
:: CLRC         (rc)         - how to call rc, this may be for example "rc"
::
:: GCC          (gcc)        - how to call gcc, this may be for example "gcc -m32"
:: GXX          (g++)        - how to call g++, this may be for example "g++ -m32"
:: GCCVER       {auto}       - gcc version, e.g. for gcc version 9.13.31 GCCVER must be set to 9013
:: AR           (ar)         - how to call ar, this may be for example "ar"
:: GCCAR        (gcc-ar)     - how to call gcc-ar, this may be for example "gcc-ar"
:: WINDRES      (windres)    - how to call windres, this may be for example "windres"
:: GCC_IS_MINGW_ORG {auto}   - non-empty if using gcc/g++ from MinGW.org project
::
:: CLANG        (clang)      - how to call clang, this may be for example "clang -m32"
:: CLANGXX      (clang++)    - how to call clang++, this may be for example "clang++ -m32"
:: CLANGVER     {auto}       - clang version, e.g. for clang version 9.13.31 CLANGVER must be set to 9013
:: AR           (ar)         - how to call ar, this may be for example "ar"
:: LLVMAR       (llvm-ar)    - how to call llvm-ar, this may be for example "llvm-ar"
:: WINDRES      (windres)    - how to call windres, this may be for example "windres"
::
:: CLANGMSVC    (clang)      - how to call clang-msvc c compiler, this may be for example "clang"
:: CLANGMSVCXX  (clang++)    - how to call clang-msvc c++ compiler, this may be for example "clang++"
:: CLANGMSVCVER {auto}       - clang-msvc compiler version, e.g. for version 11.23.6 CLANGMSVCVER must be set to 11023
:: CLLIB        (lib)        - how to call lib, this may be for example "lib"
:: LLVMAR       (llvm-ar)    - how to call llvm-ar, this may be for example "llvm-ar"
:: CLRC         (rc)         - how to call rc, this may be for example "rc"

:: re-execute batch file to avoid pollution of the environment variables space of the caller process
if not defined BUILDBATRECURSIVE (
  set TERM_PROGRAM=
  set "BUILDBATRECURSIVE=%CMDCMDLINE:"=%"
  %SYSTEMROOT%\System32\cmd.exe /s /c "call "%~dpnx0" "%*""
  exit /b
)

:: run commands from the directory of this batch file
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
  echo %~nx0 clean      - cleanup intermediate build results in "%BLD_OBJ%" directory
  echo %~nx0 distclean  - cleanup intermediate build results in "%BLD_OBJ%" directory and contents of "%BLD_DIST%" directory
  if not "%BUILDBATRECURSIVE:build.bat=%"=="%BUILDBATRECURSIVE%" ((echo.)&set /p DUMMY="Hit ENTER to close...")
  exit /b 1
)

set "FIND=%SYSTEMROOT%\System32\Find.exe"

:: remove double quotes
if defined UNICODE_CTYPE set "UNICODE_CTYPE=%UNICODE_CTYPE:"=%"
if defined LIBUTF16      set "LIBUTF16=%LIBUTF16:"=%"
if defined MSCRTX        set "MSCRTX=%MSCRTX:"=%"
if defined SAL_DEFS_H    set "SAL_DEFS_H=%SAL_DEFS_H:"=%"

if not defined BLD_DIST (set BLD_DIST=dist) else set "BLD_DIST=%BLD_DIST:"=%"
if not defined BLD_OBJ  (set BLD_OBJ=obj)   else set "BLD_OBJ=%BLD_OBJ:"=%"

set A1=
set A2=
set A3=
set A4=
set A5=
set A6=
set A7=
set A8=
for /f "tokens=1,2,3,4,5,6,7,8" %%a in (%1) do (
  set "A1=%%a"
  set "A2=%%b"
  set "A3=%%c"
  set "A4=%%d"
  set "A5=%%e"
  set "A6=%%f"
  set "A7=%%g"
  set "A8=%%h"
)

:: "all" by default
set TOOLCHAIN=
set DO_BUILD_AWK=x
set DO_BUILD_SHLIB=x
set DO_BUILD_HELPERS=x

if "awk"=="%A1%" (
  set DO_BUILD_SHLIB=
  set DO_BUILD_HELPERS=
) else (if "shlib"=="%A1%" (
  set DO_BUILD_AWK=
  set DO_BUILD_HELPERS=
) else (if "helpers"=="%A1%" (
  set DO_BUILD_AWK=
  set DO_BUILD_SHLIB=
) else (
  if "clean"=="%A1%" goto :do_clean
  if "distclean"=="%A1%" (
    echo rd /q /s "%BLD_DIST%"
    rd /q /s "%BLD_DIST%" > NUL 2>&1
:do_clean
    echo rd /q /s "%BLD_OBJ%"
    rd /q /s "%BLD_OBJ%" > NUL 2>&1
    exit /b
  )
  if not "all"=="%A1%" (
    echo ERROR: unknown build target: "%A1%"
    echo.
    goto :usage
  )
)))

call :checkbuildopt "%A2%" || goto :usage
call :checkbuildopt "%A3%" || goto :usage
call :checkbuildopt "%A4%" || goto :usage
call :checkbuildopt "%A5%" || goto :usage
call :checkbuildopt "%A6%" || goto :usage
call :checkbuildopt "%A7%" || goto :usage
if not ""=="%A8%" call :badbuildopts || goto :usage

call :checkoptsuniq %~1 || (echo ERROR: There are duplicate options) && exit /b 1

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
(call :isoptionset "cl"         %~1) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%cl"
(call :isoptionset "gcc"        %~1) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%gcc"
(call :isoptionset "clang"      %~1) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%clang"
(call :isoptionset "clang-msvc" %~1) && set "SPECTOOLCHAIN=%SPECTOOLCHAIN%clangmsvc"
if defined SPECTOOLCHAIN (
if not "%SPECTOOLCHAIN%"=="cl" (
if not "%SPECTOOLCHAIN%"=="gcc" (
if not "%SPECTOOLCHAIN%"=="clang" (
if not "%SPECTOOLCHAIN%"=="clangmsvc" (
  (echo ERROR: Duplicate toolchain) && exit /b 1
)))))
endlocal & set "TOOLCHAIN=%SPECTOOLCHAIN%"

(call :isoptionset "c++"      %~1) && set COMPILE_AS_CXX=x
(call :isoptionset "pedantic" %~1) && set BUILD_PEDANTIC=x
(call :isoptionset "debug"    %~1) && set DEBUG_BUILD=x
(call :isoptionset "analyze"  %~1) && set DO_ANALYZE=x
(call :isoptionset "32"       %~1) && set BUILDFORCPU=32
(call :isoptionset "64"       %~1) && set "BUILDFORCPU=%BUILDFORCPU%64"

if "%BUILDFORCPU%"=="3264" ((echo ERROR: Duplicate target CPU architecture) && exit /b 1)

:: choose toolchain
if not defined TOOLCHAIN cl > NUL 2>&1 && set TOOLCHAIN=cl
if not defined TOOLCHAIN gcc --version > NUL 2>&1 && set TOOLCHAIN=gcc
if not defined TOOLCHAIN clang --version > NUL 2>&1 && (
  set TOOLCHAIN=clang
  clang --version | %FIND% /i "msvc" > NUL && set TOOLCHAIN=clangmsvc
)

if not defined TOOLCHAIN (
  echo Can't determine toolchan.
  echo Supported toolchains: CL^(MSVC^), GCC^(MinGW.org/mingw-w64^), CLANG^(mingw-w64/MSVC^).
  echo.
  echo To setup CL^(MSVC^) toolchain, run:
  echo  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
  echo.
  echo To setup GCC^(mingw-w64^) toolchain, first run:
  echo  "C:\msys64\mingw64.exe"
  echo then make sure mingw-w64-x86_64-gcc is installed:
  echo  pacman -S mingw-w64-x86_64-gcc
  echo then in MSYS window, run cmd.exe ^(with proper environment^):
  echo  start "" cmd.exe
  echo then close MSYS window and in cmd.exe console change directory:
  echo  cd "%~dp0"
  echo.
  echo mingw-w64 notes:
  echo For GCC-i686     - install mingw-w64-i686-gcc
  echo For CLANG-i686   - install mingw-w64-i686-clang & mingw-w64-i686-lld
  echo For CLANG-x86_64 - install mingw-w64-x86_64-clang & mingw-w64-x86_64-lld
  exit /b 1
)

:: prerequisites
if not defined LIBUTF16 (
  echo Please define LIBUTF16 - path to the libutf16 library ^(https://github.com/mbuilov/libutf16^)
  exit /b 1
)
if not defined UNICODE_CTYPE (
  echo Please define UNICODE_CTYPE - path to the unicode_ctype library ^(https://github.com/mbuilov/unicode_ctype^)
  exit /b 1
)
if not defined MSCRTX (
  echo Please define MSCRTX - path to the mscrtx library ^(https://github.com/mbuilov/mscrtx^)
  exit /b 1
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

:: build for WindowsXP and later
set "CMN_DEFINES=/D_WIN32_WINNT=0x501"

:: include sal_defs.h, if available
if defined SAL_DEFS_H set "CMN_DEFINES=%CMN_DEFINES% /FI""%SAL_DEFS_H%"""

:: define GAWK_VER_DEFINES - set of variables for generating version info resource file(s)
call :getgawkver || (
  echo failed to determine building gawk version info
  exit /b 1
)

if not "%TOOLCHAIN%"=="cl" goto :toolchain1

:::::::::::::::::::::: CL :::::::::::::::::::::::

if not defined CLCC   (set CLCC=cl)     else call :escape_cc CLCC
if not defined CLLIB  (set CLLIB=lib)   else call :escape_cc CLLIB
if not defined CLLINK (set CLLINK=link) else call :escape_cc CLLINK
if not defined CLRC   (set CLRC=rc)     else call :escape_cc CLRC

if not defined CLVER (
  call :extract_ver CLVER "%CLCC% <NUL 2>&1" "Microsoft (R) C/C++ Optimizing Compiler Version " || (
    echo Couldn't determine cl compiler version, please specify CLVER explicitly ^(e.g. CLVER=19026 for 19.26.28806^)
    exit /b 1
  )
)

if not defined BUILDFORCPU (echo Please specify target CPU architecture: 32 or 64) & exit /b 1

:: define _Noreturn for pc/config.h & support/cdefs.h
:: define __inline for gettext.h
set "CMN_DEFINES=%CMN_DEFINES% /DNEED_C99_FEATURES /D_Noreturn=__declspec(noreturn) /D__inline=__inline"

:: _CRT_DECLARE_NONSTDC_NAMES=0 - do not define non-STDC names sush as isascii, environ, etc.
set "CMN_DEFINES=%CMN_DEFINES% /D_CRT_DECLARE_NONSTDC_NAMES=0"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% /analyze"
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

set "SUBSYSTEM=/SUBSYSTEM:CONSOLE"

:: WindowsXP support
if "%BUILDFORCPU%"=="32" set "SUBSYSTEM=%SUBSYSTEM%,5.01%"
if "%BUILDFORCPU%"=="64" set "SUBSYSTEM=%SUBSYSTEM%,5.02%"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=/nologo %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Od /Z7 /EHsc /MTd"
  set "LIBABSTRACTLIB=%CLLIB% /nologo /OUT:"
  set "GAWKLINK=%CLLINK% /nologo %SUBSYSTEM% /DEBUG /INCREMENTAL:NO"
  set "SHLIBLINK=%CLLINK% /nologo %SUBSYSTEM% /DEBUG /INCREMENTAL:NO /DLL /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntimed /DEFAULTLIB:libucrtd"
  set "TOOLLINK=%CLLINK% /nologo %SUBSYSTEM% /DEBUG /INCREMENTAL:NO"
) else (
  :: release options
  set "CMNOPTS=/nologo %NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% /Ox /GF /Gy /GS- /GL /EHsc /DNDEBUG /MT"
  set "LIBABSTRACTLIB=%CLLIB% /nologo /LTCG /OUT:"
  set "GAWKLINK=%CLLINK% /nologo %SUBSYSTEM% /LTCG"
  set "SHLIBLINK=%CLLINK% /nologo %SUBSYSTEM% /LTCG /DLL /Entry:ExtDllMain /DEFAULTLIB:kernel32 /DEFAULTLIB:libvcruntime /DEFAULTLIB:libucrt"
  set "TOOLLINK=%CLLINK% /nologo %SUBSYSTEM% /LTCG"
)
set SUBSYSTEM=

set "UNICODE_CTYPECC=%CLCC% /c %CMNOPTS% /I""%UNICODE_CTYPE%"" /Fo"
set "LIBUTF16CC=%CLCC% /c %CMNOPTS% /I""%LIBUTF16%"" /Fo"
set "MSCRTXCC=%CLCC% /c %CMNOPTS% /I""%MSCRTX%"" /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /Fo"
set "GAWKCC=%CLCC% /c %CMNOPTS% %GAWK_DEFINES% /DGAWK_STATIC_CRT /DHAVE_CONFIG_H /DLOCALEDIR=\"\" /DDEFPATH=\".\" /DDEFLIBPATH=\"\" /DSHLIBEXT=\"dll\" /Isupport /Ipc /I. /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /I""%MSCRTX%"" /Fo"
set "SHLIBCC=%CLCC% /c %CMNOPTS% /DGAWK_STATIC_CRT /DHAVE_CONFIG_H /Iextension /Ipc /I. /I""%MSCRTX%"" /Fo"
set "TOOLCC=%CLCC% /c %CMNOPTS% /I""%MSCRTX%"" /I""%LIBUTF16%"" /I""%UNICODE_CTYPE%"" /Fo"
set "GAWKRES=%CLRC% /nologo %GAWK_VER_DEFINES% /fo"

set CC_DEFINE=/D
set CC_INCLUDE=/I

goto :do_build

:cl_awk_ld
call :execq "%GAWKLINK:^^=^% /DEFAULTLIB:WS2_32.lib /OUT:""%BLD_DIST%\%~1.exe"" %~2"
exit /b

:cl_shlib_ld
call :execq "%SHLIBLINK:^^=^% /IMPLIB:""%BLD_OBJ%\extension\%~1\%~1.lib"" /OUT:""%BLD_DIST%\%~1.dll"" %~2"
exit /b

:cl_tool_ld
call :execq "%TOOLLINK:^^=^% /OUT:""%BLD_DIST%\helpers\%~1.exe"" %~2"
exit /b

:::::::::::::::::::::: CL END :::::::::::::::::::

:toolchain1
if not "%TOOLCHAIN%"=="gcc" goto :toolchain2

:::::::::::: GCC (MinGW.org/mingw-w64) :::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined GCC    (set "GCC=gcc %BUILDFORCPU%") else call :escape_cc GCC "%BUILDFORCPU%"
if not defined GXX    (set "GXX=g++ %BUILDFORCPU%") else call :escape_cc GXX "%BUILDFORCPU%"
if not defined AR     (set AR=ar)                   else call :escape_cc AR
if not defined GCCAR  (set GCCAR=gcc-ar)            else call :escape_cc GCCAR
if defined BUILDFORCPU set "BUILDFORCPU=%BUILDFORCPU:-m=%"

if not defined GCCVER (
  :: also define GCC_IS_MINGW_ORG if using ming32 build environment by the MinGW.org project
  call :extract_ver GCCVER "%GCC% --verbose 2>&1" "gcc version " || (
    echo Couldn't determine gcc compiler version, please specify GCCVER explicitly ^(e.g. GCCVER=9003 for 9.3^)
    exit /b 1
  )
)

if not defined BUILDFORCPU (echo Please specify target CPU architecture: 32 or 64) & exit /b 1

set WINDRES_ARCH=
if "%BUILDFORCPU%"=="32" set "WINDRES_ARCH=-F pe-i386"
if "%BUILDFORCPU%"=="64" set "WINDRES_ARCH=-F pe-x86-64"
if not defined WINDRES (set "WINDRES=windres %WINDRES_ARCH%") else call :escape_cc WINDRES "%WINDRES_ARCH%"

set "GAWK_VER_DEFINES=%GAWK_VER_DEFINES:/D=-D%"

set "CMN_DEFINES=%CMN_DEFINES:/FI=-include %"
set "CMN_DEFINES=%CMN_DEFINES:/D=-D% -DNEED_C99_FEATURES -fstrict-aliasing"

:: do not use old names - non-STDC names sush as isascii, environ, etc.
set "CMN_DEFINES=%CMN_DEFINES% -D_NO_OLDNAMES -DNO_OLDNAMES"

if defined DO_ANALYZE (
  if %GCCVER% GEQ 10000 (
    :: analyze sources
    set "CMN_DEFINES=%CMN_DEFINES% -fanalyzer"
  )
)

:: use Mingw's implementation of printf-functions, which support "%ll"/"%zu" format specifiers
set "CMN_DEFINES=%CMN_DEFINES% -D_POSIX_C_SOURCE"

set WARNING_OPTIONS=-Wall

:: MinGW.org: mingw-printf-format recognition is broken for C++
if defined GCC_IS_MINGW_ORG if defined COMPILE_AS_CXX set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-format"

if not defined BUILD_PEDANTIC goto :gcc_no_pedantic

:: common C/C++ warnings
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
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wfloat-equal"

:: MinGW.org: mingw-printf-format recognition is broken for C++
if defined GCC_IS_MINGW_ORG if defined COMPILE_AS_CXX goto :no_wformat

set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-contains-nul"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-extra-args"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-nonliteral"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-security"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-y2k"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-zero-length"

:no_wformat

set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-overflow=2"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-signedness"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wformat-truncation=2"
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
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-overflow=2"
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wstrict-overflow=5"
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
::set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wswitch-default"
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
::set "CMN_DEFINES=-x c++ -std=c++11 %CMN_DEFINES%"
set "CMN_DEFINES=-x c++ %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :gcc_do_build

:: C++-specific warnings
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

:: MinGW.org: mingw-printf-format recognition is broken for C++
if defined GCC_IS_MINGW_ORG set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-suggest-attribute=format"

:: disable some C++ warnings for gawk/shlib
set "GAWK_NO_WARN=-Wno-vla"

goto :gcc_do_build

:gcc_no_cxx

:: compile as c99
::set "CMN_DEFINES=-std=c99 %CMN_DEFINES%"

if not defined BUILD_PEDANTIC goto :gcc_do_build

:: C-specific warnings
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wabsolute-value"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wbad-function-cast"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc++-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc11-c2x-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wc99-c11-compat"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wdeclaration-after-statement"
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

:: disable some C warnings for gawk/shlib
set "GAWK_NO_WARN=-Wno-vla -Wno-c99-c11-compat -Wno-declaration-after-statement"

:gcc_do_build

:: gawk-specific defines
set "GAWK_DEFINES=%GAWK_DEFINES:/D=-D%"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -ggdb3 -fstack-protector-all"
) else (
  :: release options
  if %GCCVER% GEQ 10000 (
    set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
  ) else (
    :: Seems lto is broken under MinGW.org with gcc 9.2.0
    set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2"
  )
)

if defined COMPILE_AS_CXX (set "LINKER=%GXX%") else set "LINKER=%GCC%"
set "LINKER=%LINKER% -mconsole"
:: WindowsXP support
if "%BUILDFORCPU%"=="32" set "LINKER=%LINKER% -Wl,--major-subsystem-version=5 -Wl,--minor-subsystem-version=1"
if "%BUILDFORCPU%"=="64" set "LINKER=%LINKER% -Wl,--major-subsystem-version=5 -Wl,--minor-subsystem-version=2"
set "GAWKLINK=%LINKER% -static"
set "SHLIBLINK=%LINKER% -static -shared"
set "TOOLLINK=%LINKER% -static"
set LINKER=

:: GCC from MinGW.org do not supports '-municode'
if not defined GCC_IS_MINGW_ORG (
  set "GAWKLINK=%GAWKLINK% -municode"
  set "SHLIBLINK=%SHLIBLINK% -municode"
  set "TOOLLINK=%TOOLLINK% -municode"
)

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%AR% -csr "
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all"
) else (
  :: release options
  if %GCCVER% GEQ 10000 (
    set "LIBABSTRACTLIB=%GCCAR% -csr "
    set "GAWKLINK=%GAWKLINK% -flto"
    set "SHLIBLINK=%SHLIBLINK% -flto"
    set "TOOLLINK=%TOOLLINK% -flto"
  ) else (
    :: Seems lto is broken under MinGW.org with gcc 9.2.0
    set "LIBABSTRACTLIB=%AR% -csr "
  )
)

if defined COMPILE_AS_CXX (set "COMPILER=%GXX%") else set "COMPILER=%GCC%"
set "UNICODE_CTYPECC=%COMPILER% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%COMPILER% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%COMPILER% -c %CMNOPTS% %GAWK_NO_WARN% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%COMPILER% -c %CMNOPTS% %GAWK_NO_WARN% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKRES=%WINDRES% %GAWK_VER_DEFINES% -o"
set COMPILER=

set CC_DEFINE=-D
set CC_INCLUDE=-I

if defined DO_BUILD_SHLIB call :getdllstartup GCC || exit /b

goto :do_build

:gcc_awk_ld
call :execq "%GAWKLINK:^^=^% -o ""%BLD_DIST%\%~1.exe"" %~2 -lws2_32 -lmpfr -lgmp"
exit /b

:gcc_shlib_ld
call :execq "%SHLIBLINK:^^=^% -Wl,-e%SHLIBDLLMAIN% -o ""%BLD_DIST%\%~1.dll"" %~2"
exit /b

:gcc_tool_ld
call :execq "%TOOLLINK:^^=^% -o ""%BLD_DIST%\helpers\%~1.exe"" %~2"
exit /b

::::::::: GCC (MinGW.org/mingw-w64) END ::::::::::

:toolchain2
if not "%TOOLCHAIN%"=="clang" goto :toolchain3

:::::::::::::::: CLANG (mingw-w64) :::::::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined CLANG   (set "CLANG=clang %BUILDFORCPU%")     else call :escape_cc CLANG "%BUILDFORCPU%"
if not defined CLANGXX (set "CLANGXX=clang++ %BUILDFORCPU%") else call :escape_cc CLANGXX "%BUILDFORCPU%"
if not defined AR      (set AR=ar)                           else call :escape_cc AR
if not defined LLVMAR  (set LLVMAR=llvm-ar)                  else call :escape_cc LLVMAR
if defined BUILDFORCPU set "BUILDFORCPU=%BUILDFORCPU:-m=%"

if not defined CLANGVER (
  call :extract_ver CLANGVER "%CLANG% --version 2>&1" "clang version " || (
    echo Couldn't determine clang compiler version, please specify CLANGVER explicitly ^(e.g. CLANGVER=10001 for 10.1^)
    exit /b 1
  )
)

if not defined BUILDFORCPU (echo Please specify target CPU architecture: 32 or 64) & exit /b 1

set WINDRES_ARCH=
if "%BUILDFORCPU%"=="32" set "WINDRES_ARCH=-F pe-i386"
if "%BUILDFORCPU%"=="64" set "WINDRES_ARCH=-F pe-x86-64"
if not defined WINDRES (set "WINDRES=windres %WINDRES_ARCH%") else call :escape_cc WINDRES "%WINDRES_ARCH%"

set "GAWK_VER_DEFINES=%GAWK_VER_DEFINES:/D=-D%"

set "CMN_DEFINES=%CMN_DEFINES:/FI=-include %"
set "CMN_DEFINES=%CMN_DEFINES:/D=-D% -DNEED_C99_FEATURES -fstrict-aliasing"

:: do not use old names - non-STDC names sush as isascii, environ, etc.
set "CMN_DEFINES=%CMN_DEFINES% -D_NO_OLDNAMES -DNO_OLDNAMES"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% --analyze"
)

:: use Mingw's implementation of printf-functions, which support "%ll"/"%zu" format specifiers
set "CMN_DEFINES=%CMN_DEFINES% -D_POSIX_C_SOURCE"

if not defined BUILD_PEDANTIC (
  set WARNING_OPTIONS=-Wall
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
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-float-equal"

:clang_no_pedantic

if not defined COMPILE_AS_CXX goto :clang_no_cxx

:: compile as c++11
set "CMN_DEFINES=-std=c++11 %CMN_DEFINES%"

:: avoid warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated
set "CMN_DEFINES=-x c++ %CMN_DEFINES%"

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
set "GAWK_DEFINES=%GAWK_DEFINES:/D=-D%"

:: define HAVE___INLINE for support/libc-config.h
:: define HAVE___RESTRICT for support/libc-config.h
if defined COMPILE_AS_CXX set "GAWK_DEFINES=%GAWK_DEFINES% -DHAVE___INLINE -DHAVE___RESTRICT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -g -gcodeview -fstack-protector-all"
) else (
  :: release options
  set "CMNOPTS=%WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
)

set "GAWKLINK=%LINKER% -static"
set "SHLIBLINK=%LINKER% -static -shared"
set "TOOLLINK=%LINKER% -static"
set LINKER=

if defined COMPILE_AS_CXX (set "LINKER=%CLANGXX%") else set "LINKER=%CLANG%"
set "LINKER=%LINKER% -mconsole -municode"
:: WindowsXP support
if "%BUILDFORCPU%"=="32" set "LINKER=%LINKER% -Wl,--major-subsystem-version=5 -Wl,--minor-subsystem-version=1"
if "%BUILDFORCPU%"=="64" set "LINKER=%LINKER% -Wl,--major-subsystem-version=5 -Wl,--minor-subsystem-version=2"
set "GAWKLINK=%LINKER% -static"
set "SHLIBLINK=%LINKER% -static -shared"
set "TOOLLINK=%LINKER% -static"
set LINKER=

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%AR% -csr "
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all -fuse-ld=lld -Wl,-pdb"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all -fuse-ld=lld -Wl,-pdb"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all -fuse-ld=lld -Wl,-pdb"
) else (
  set "LIBABSTRACTLIB=%LLVMAR% -csr "
  set "GAWKLINK=%GAWKLINK% -flto -fuse-ld=lld"
  set "SHLIBLINK=%SHLIBLINK% -flto -fuse-ld=lld"
  set "TOOLLINK=%TOOLLINK% -flto -fuse-ld=lld"
)

if defined COMPILE_AS_CXX (set "COMPILER=%CLANGXX%") else set "COMPILER=%CLANG%"
set "UNICODE_CTYPECC=%COMPILER% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%COMPILER% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%COMPILER% -c %CMNOPTS% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%COMPILER% -c %CMNOPTS% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKRES=%WINDRES% %GAWK_VER_DEFINES% -o"
set COMPILER=

set CC_DEFINE=-D
set CC_INCLUDE=-I

if defined DO_BUILD_SHLIB call :getdllstartup CLANG || exit /b

goto :do_build

:clang_awk_ld
if defined DEBUG_BUILD (
  call :execq "%GAWKLINK:^^=^% -Wl,""%BLD_DIST%\%~1.pdb"" -o ""%BLD_DIST%\%~1.exe"" %~2 -lws2_32 -lmpfr -lgmp"
) else (
  call :execq "%GAWKLINK:^^=^% -o ""%BLD_DIST%\%~1.exe"" %~2 -lws2_32 -lmpfr -lgmp"
)
exit /b

:clang_shlib_ld
if defined DEBUG_BUILD (
  call :execq "%SHLIBLINK:^^=^% -Wl,""%BLD_DIST%\%~1.pdb"" -Wl,-e%SHLIBDLLMAIN% -o ""%BLD_DIST%\%~1.dll"" %~2"
) else (
  call :execq "%SHLIBLINK:^^=^% -Wl,-e%SHLIBDLLMAIN% -o ""%BLD_DIST%\%~1.dll"" %~2"
)
exit /b

:clang_tool_ld
if defined DEBUG_BUILD (
  call :execq "%TOOLLINK:^^=^% -Wl,""%BLD_DIST%\helpers\%~1.pdb"" -o ""%BLD_DIST%\helpers\%~1.exe"" %~2"
) else (
  call :execq "%TOOLLINK:^^=^% -o ""%BLD_DIST%\helpers\%~1.exe"" %~2"
)
exit /b

:::::::::::::: CLANG (mingw-w64) END ::::::::::::

:toolchain3
if not "%TOOLCHAIN%"=="clangmsvc" goto :toolchain4

:::::::::::::::::: CLANG-MSVC :::::::::::::::::::

if defined BUILDFORCPU set "BUILDFORCPU=-m%BUILDFORCPU%"
if not defined CLANGMSVC   (set "CLANGMSVC=clang %BUILDFORCPU%")     else call :escape_cc CLANGMSVC "%BUILDFORCPU%%"
if not defined CLANGMSVCXX (set "CLANGMSVCXX=clang++ %BUILDFORCPU%") else call :escape_cc CLANGMSVCXX "%BUILDFORCPU%"
if not defined CLLIB       (set CLLIB=lib)                           else call :escape_cc CLLIB
if not defined LLVMAR      (set LLVMAR=llvm-ar)                      else call :escape_cc LLVMAR
if not defined CLRC        (set CLRC=rc)                             else call :escape_cc CLRC
if defined BUILDFORCPU set "BUILDFORCPU=%BUILDFORCPU:-m=%"

if not defined CLANGMSVCVER (
  call :extract_ver CLANGMSVCVER "%CLANGMSVC% --version 2>&1" "clang version " || (
    echo Couldn't determine clang-msvc compiler version, please specify CLANGMSVCVER explicitly ^(e.g. CLANGMSVCVER=11023 for 11.23.6^)
    exit /b 1
  )
)

if not defined BUILDFORCPU (echo Please specify target CPU architecture: 32 or 64) & exit /b 1

set "GAWK_VER_DEFINES=%GAWK_VER_DEFINES:/D=-D%"

set "CMN_DEFINES=%CMN_DEFINES:/FI=-include %"

:: define _Noreturn for pc/config.h & support/cdefs.h
:: define __inline for gettext.h
set "CMN_DEFINES=%CMN_DEFINES:/D=-D% -DNEED_C99_FEATURES -fstrict-aliasing -D_Noreturn=_Noreturn -D__inline=__inline"

:: _CRT_DECLARE_NONSTDC_NAMES=0 - do not define non-STDC names sush as isascii, environ, etc.
set "CMN_DEFINES=%CMN_DEFINES% -D_CRT_DECLARE_NONSTDC_NAMES=0"

if defined DO_ANALYZE (
  :: analyze sources
  set "CMN_DEFINES=%CMN_DEFINES% --analyze"
)

:: disable MSVC deprecation warnings
set "NODEPRECATE=-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE -D_WINSOCK_DEPRECATED_NO_WARNINGS"

if not defined BUILD_PEDANTIC (
  set WARNING_OPTIONS=-Wall
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
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-float-equal"
set "WARNING_OPTIONS=%WARNING_OPTIONS% -Wno-nonportable-system-include-path"

:clangmsvc_no_pedantic

if not defined COMPILE_AS_CXX goto :clangmsvc_no_cxx

:: compile as c++11
set "CMN_DEFINES=-std=c++11 %CMN_DEFINES%"

:: avoid warning: treating 'c' input as 'c++' when in C++ mode, this behavior is deprecated
set "CMN_DEFINES=-x c++ %CMN_DEFINES%"

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
set "GAWK_DEFINES=%GAWK_DEFINES:/D=-D%"

:: define _Restrict_ for support/regex.h
:: define HAVE___INLINE for support/libc-config.h
:: define HAVE___RESTRICT for support/libc-config.h
if defined COMPILE_AS_CXX set "GAWK_DEFINES=%GAWK_DEFINES% -D_Restrict_=__restrict -DHAVE___INLINE -DHAVE___RESTRICT"

if defined DEBUG_BUILD (
  :: debugging options
  set "CMNOPTS=%NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% -pipe -O0 -gcodeview -fstack-protector-all"
) else (
  :: release options
  set "CMNOPTS=%NODEPRECATE% %WARNING_OPTIONS% %CMN_DEFINES% -pipe -DNDEBUG -O2 -flto"
)

if defined COMPILE_AS_CXX (set "LINKER=%CLANGMSVCXX%") else set "LINKER=%CLANGMSVC%"
set "LINKER=%LINKER% -mconsole -municode"
set "GAWKLINK=%LINKER% -static"
set "SHLIBLINK=%LINKER% -static -shared"
set "TOOLLINK=%LINKER% -static"
set LINKER=

set "SUBSYSTEM=/SUBSYSTEM:CONSOLE"

:: WindowsXP support
if "%BUILDFORCPU%"=="32" set "SUBSYSTEM=%SUBSYSTEM%,5.01%"
if "%BUILDFORCPU%"=="64" set "SUBSYSTEM=%SUBSYSTEM%,5.02%"

:: cannot pass /SUBSYSTEM:CONSOLE,5.01 via -Wl,<option> - use linker response file
if not exist "%BLD_OBJ%" call :execq "md ""%BLD_OBJ%""" || (echo Failed to create directory: "%BLD_OBJ%") && exit /b 1
call :execq "(echo %SUBSYSTEM%) > ""%BLD_OBJ%\subsys.resp""" || (echo Failed to create linker response file: "%BLD_OBJ%\subsys.resp") && exit /b 1

if defined DEBUG_BUILD (
  set "LIBABSTRACTLIB=%CLLIB% /nologo /OUT:"
  set "GAWKLINK=%GAWKLINK% -fstack-protector-all -Wl,@""%BLD_OBJ%\subsys.resp"" -Wl,/DEBUG -Wl,/INCREMENTAL:NO -Wl,/NODEFAULTLIB:libcmt -Wl,/DEFAULTLIB:libcmtd -Wl,/DEFAULTLIB:kernel32 -Wl,/DEFAULTLIB:libvcruntimed -Wl,/DEFAULTLIB:libucrtd"
  set "SHLIBLINK=%SHLIBLINK% -fstack-protector-all -Wl,@""%BLD_OBJ%\subsys.resp"" -Wl,/DEBUG -Wl,/INCREMENTAL:NO -Wl,/Entry:ExtDllMain -Wl,/NODEFAULTLIB:libcmt -Wl,/DEFAULTLIB:libcmtd -Wl,/DEFAULTLIB:kernel32 -Wl,/DEFAULTLIB:libvcruntimed -Wl,/DEFAULTLIB:libucrtd"
  set "TOOLLINK=%TOOLLINK% -fstack-protector-all -Wl,@""%BLD_OBJ%\subsys.resp"" -Wl,/DEBUG -Wl,/INCREMENTAL:NO -Wl,/NODEFAULTLIB:libcmt -Wl,/DEFAULTLIB:libcmtd -Wl,/DEFAULTLIB:kernel32 -Wl,/DEFAULTLIB:libvcruntimed -Wl,/DEFAULTLIB:libucrtd"
) else (
  set "LIBABSTRACTLIB=%LLVMAR% -csr "
  set "GAWKLINK=%GAWKLINK% -flto -fuse-ld=lld -Wl,@""%BLD_OBJ%\subsys.resp"""
  set "SHLIBLINK=%SHLIBLINK% -flto -fuse-ld=lld -Wl,@""%BLD_OBJ%\subsys.resp"""
  set "TOOLLINK=%TOOLLINK% -flto -fuse-ld=lld -Wl,@""%BLD_OBJ%\subsys.resp"""
)
set SUBSYSTEM=

if defined COMPILE_AS_CXX (set "COMPILER=%CLANGMSVCXX%") else set "COMPILER=%CLANGMSVC%"
set "UNICODE_CTYPECC=%COMPILER% -c %CMNOPTS% -I""%UNICODE_CTYPE%"" -o"
set "LIBUTF16CC=%COMPILER% -c %CMNOPTS% -I""%LIBUTF16%"" -o"
set "MSCRTXCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKCC=%COMPILER% -c %CMNOPTS% %GAWK_DEFINES% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -DLOCALEDIR=\"\" -DDEFPATH=\".\" -DDEFLIBPATH=\"\" -DSHLIBEXT=\"dll\" -Isupport -Ipc -I. -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -I""%MSCRTX%"" -o"
set "SHLIBCC=%COMPILER% -c %CMNOPTS% -DGAWK_STATIC_CRT -DHAVE_CONFIG_H -Iextension -Ipc -I. -I""%MSCRTX%"" -o"
set "TOOLCC=%COMPILER% -c %CMNOPTS% -I""%MSCRTX%"" -I""%LIBUTF16%"" -I""%UNICODE_CTYPE%"" -o"
set "GAWKRES=%CLRC% /nologo %GAWK_VER_DEFINES% /fo"
set COMPILER=

set CC_DEFINE=-D
set CC_INCLUDE=-I

goto :do_build

:clangmsvc_awk_ld
call :execq "%GAWKLINK:^^=^% -o ""%BLD_DIST%\%~1.exe"" %~2 -lws2_32"
exit /b

:clangmsvc_shlib_ld
call :execq "%SHLIBLINK:^^=^% -o ""%BLD_DIST%\%~1.dll"" %~2"
exit /b

:clangmsvc_tool_ld
call :execq "%TOOLLINK:^^=^% -o ""%BLD_DIST%\helpers\%~1.exe"" %~2"
exit /b

:::::::::::::::::: CLANG-MSVC END :::::::::::::::

:toolchain4
:: add more toolchains here

::::: BUILDING :::::
:do_build

if not exist "%BLD_DIST%" call :execq "md ""%BLD_DIST%""" || goto :build_err

if defined DO_BUILD_AWK (
  call :unicode_ctype || goto :build_err
  call :libutf16      || goto :build_err
  call :mscrtx        || goto :build_err
  call :support       || goto :build_err
  call :gawk          || goto :build_err
)
if defined DO_BUILD_SHLIB (
  call :shlibs  || goto :build_err
)
if defined DO_BUILD_HELPERS (
  call :unicode_ctype || goto :build_err
  call :libutf16      || goto :build_err
  call :mscrtx        || goto :build_err
  call :helpers       || goto :build_err
)

echo OK!
exit /b 0

:build_err
echo *** Compilation failed! ***
exit /b 1

:::::: COMPILATION :::::
:unicode_ctype

if not exist "%BLD_OBJ%\unicode_ctype" call :execq "md ""%BLD_OBJ%\unicode_ctype""" || exit /b
call :cc UNICODE_CTYPECC "%BLD_OBJ%\unicode_ctype" "%UNICODE_CTYPE%\src\unicode_ctype.c"   || exit /b
call :cc UNICODE_CTYPECC "%BLD_OBJ%\unicode_ctype" "%UNICODE_CTYPE%\src\unicode_toupper.c" || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :lib "%BLD_OBJ%\unicode_ctype\unicode_ctype.a" ^
 "%BLD_OBJ%\unicode_ctype\unicode_ctype.obj"   ^
 "%BLD_OBJ%\unicode_ctype\unicode_toupper.obj" || exit /b

exit /b 0

:libutf16

if not exist "%BLD_OBJ%\libutf16" call :execq "md ""%BLD_OBJ%\libutf16""" || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf16_to_utf8.c"     || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf8_to_utf16.c"     || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf16_to_utf8_one.c" || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf8_to_utf16_one.c" || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf8_cstd.c"         || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf16_to_utf32.c"    || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf32_to_utf16.c"    || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf8_to_utf32.c"     || exit /b
call :cc LIBUTF16CC "%BLD_OBJ%\libutf16" "%LIBUTF16%\src\utf32_to_utf8.c"     || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :lib "%BLD_OBJ%\libutf16\libutf16.a" ^
 "%BLD_OBJ%\libutf16\utf16_to_utf8.obj"     ^
 "%BLD_OBJ%\libutf16\utf8_to_utf16.obj"     ^
 "%BLD_OBJ%\libutf16\utf16_to_utf8_one.obj" ^
 "%BLD_OBJ%\libutf16\utf8_to_utf16_one.obj" ^
 "%BLD_OBJ%\libutf16\utf8_cstd.obj"         ^
 "%BLD_OBJ%\libutf16\utf16_to_utf32.obj"    ^
 "%BLD_OBJ%\libutf16\utf32_to_utf16.obj"    ^
 "%BLD_OBJ%\libutf16\utf8_to_utf32.obj"     ^
 "%BLD_OBJ%\libutf16\utf32_to_utf8.obj"  || exit /b

exit /b 0

:mscrtx

if not exist "%BLD_OBJ%\mscrtx" call :execq "md ""%BLD_OBJ%\mscrtx""" || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\arg_parser.c"      || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\socket_fd.c"       || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\socket_file.c"     || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\wreaddir.c"        || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\wreadlink.c"       || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\xstat.c"           || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\locale_helpers.c"  || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\localerpl.c"       || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\utf16cvt.c"        || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\utf8env.c"         || exit /b
call :cc MSCRTXCC "%BLD_OBJ%\mscrtx" "%MSCRTX%\src\consoleio.c"       || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :lib "%BLD_OBJ%\mscrtx\mscrtx.a" ^
 "%BLD_OBJ%\mscrtx\arg_parser.obj"      ^
 "%BLD_OBJ%\mscrtx\socket_fd.obj"       ^
 "%BLD_OBJ%\mscrtx\socket_file.obj"     ^
 "%BLD_OBJ%\mscrtx\wreaddir.obj"        ^
 "%BLD_OBJ%\mscrtx\wreadlink.obj"       ^
 "%BLD_OBJ%\mscrtx\xstat.obj"           ^
 "%BLD_OBJ%\mscrtx\locale_helpers.obj"  ^
 "%BLD_OBJ%\mscrtx\localerpl.obj"       ^
 "%BLD_OBJ%\mscrtx\utf16cvt.obj"        ^
 "%BLD_OBJ%\mscrtx\utf8env.obj"         ^
 "%BLD_OBJ%\mscrtx\consoleio.obj" || exit /b

exit /b 0

:support

if not exist "%BLD_OBJ%\support" call :execq "md ""%BLD_OBJ%\support""" || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\getopt.c     || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\getopt1.c    || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\random.c     || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\dfa.c        || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\localeinfo.c || exit /b
call :cc GAWKCC "%BLD_OBJ%\support" support\regex.c      || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :lib "%BLD_OBJ%\support\libsupport.a" ^
 "%BLD_OBJ%\support\getopt.obj"     ^
 "%BLD_OBJ%\support\getopt1.obj"    ^
 "%BLD_OBJ%\support\random.obj"     ^
 "%BLD_OBJ%\support\dfa.obj"        ^
 "%BLD_OBJ%\support\localeinfo.obj" ^
 "%BLD_OBJ%\support\regex.obj"         || exit /b

exit /b 0

:gawk

if not exist "%BLD_OBJ%\gawk" call :execq "md ""%BLD_OBJ%\gawk""" || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" array.c             || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" awkgram.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" builtin.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" pc\getid.c          || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" cint_array.c        || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" command.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" debug.c             || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" eval.c              || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" ext.c               || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" field.c             || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" floatcomp.c         || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" gawkapi.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" gawkcrtapi.c        || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" gawkmisc.c          || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" int_array.c         || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" io.c                || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" main.c              || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" mpfr.c              || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" msg.c               || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" node.c              || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" profile.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" re.c                || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" replace.c           || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" str_array.c         || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" symbol.c            || exit /b
call :cc GAWKCC "%BLD_OBJ%\gawk" version.c           || exit /b

call :res GAWKRES "%BLD_OBJ%\gawk" pc\version.rc "gawk" "gawk.exe" || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :ld awk gawk           ^
 "%BLD_OBJ%\gawk\array.obj"        ^
 "%BLD_OBJ%\gawk\awkgram.obj"      ^
 "%BLD_OBJ%\gawk\builtin.obj"      ^
 "%BLD_OBJ%\gawk\getid.obj"        ^
 "%BLD_OBJ%\gawk\cint_array.obj"   ^
 "%BLD_OBJ%\gawk\command.obj"      ^
 "%BLD_OBJ%\gawk\debug.obj"        ^
 "%BLD_OBJ%\gawk\eval.obj"         ^
 "%BLD_OBJ%\gawk\ext.obj"          ^
 "%BLD_OBJ%\gawk\field.obj"        ^
 "%BLD_OBJ%\gawk\floatcomp.obj"    ^
 "%BLD_OBJ%\gawk\gawkapi.obj"      ^
 "%BLD_OBJ%\gawk\gawkcrtapi.obj"   ^
 "%BLD_OBJ%\gawk\gawkmisc.obj"     ^
 "%BLD_OBJ%\gawk\int_array.obj"    ^
 "%BLD_OBJ%\gawk\io.obj"           ^
 "%BLD_OBJ%\gawk\main.obj"         ^
 "%BLD_OBJ%\gawk\mpfr.obj"         ^
 "%BLD_OBJ%\gawk\msg.obj"          ^
 "%BLD_OBJ%\gawk\node.obj"         ^
 "%BLD_OBJ%\gawk\profile.obj"      ^
 "%BLD_OBJ%\gawk\re.obj"           ^
 "%BLD_OBJ%\gawk\replace.obj"      ^
 "%BLD_OBJ%\gawk\str_array.obj"    ^
 "%BLD_OBJ%\gawk\symbol.obj"       ^
 "%BLD_OBJ%\gawk\version.obj"      ^
 "%BLD_OBJ%\gawk\version.res.obj"  ^
 "%BLD_OBJ%\support\libsupport.a"  ^
 "%BLD_OBJ%\mscrtx\mscrtx.a"       ^
 "%BLD_OBJ%\libutf16\libutf16.a"   ^
 "%BLD_OBJ%\unicode_ctype\unicode_ctype.a" || exit /b

exit /b 0

:shlibs

call :shlib testext        || exit /b
call :shlib readdir        || exit /b
call :shlib readdir_test %CC_DEFINE%READDIR_TEST readdir.c || exit /b
call :shlib inplace        || exit /b
call :shlib time           || exit /b

if not exist "%BLD_OBJ%\extension\filefuncs" call :execq "md ""%BLD_OBJ%\extension\filefuncs""" || exit /b
call :shlib_cc "%BLD_OBJ%\extension\filefuncs" stack       || exit /b
call :shlib_cc "%BLD_OBJ%\extension\filefuncs" gawkfts "%CC_DEFINE%FTS_ALIGN_BY=sizeof^(void*^)" || exit /b
call :shlib_cc "%BLD_OBJ%\extension\filefuncs" filefuncs   || exit /b

:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :shlib_ld filefuncs ^
 "%BLD_OBJ%\extension\filefuncs\stack.obj"     ^
 "%BLD_OBJ%\extension\filefuncs\gawkfts.obj"   ^
 "%BLD_OBJ%\extension\filefuncs\filefuncs.obj" || exit /b

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
if not exist "%BLD_OBJ%\extension\%~1" call :execq "md ""%BLD_OBJ%\extension\%~1""" || exit /b
call :shlib_cc "%BLD_OBJ%\extension\%~1" "%~1" "%~2" "%~3"                          || exit /b
call :shlib_ld "%~1" "%BLD_OBJ%\extension\%~1\%~1.obj"                              || exit /b
exit /b 0

:shlib_cc
:: %1 - object file directory
:: %2 - object file base name
:: %3 - (optional) additional compiler options
:: %4 - (optional) source file name
setlocal
:: %~3 doubles ^ in the options, un-double it
set "OPTIONS=%~3"
if defined OPTIONS set "OPTIONS=%OPTIONS:^^=^%"
if ""=="%~4" (
  call :cc SHLIBCC "%~1" "extension\%~2.c" "%OPTIONS%"
) else (
  call :cc SHLIBCC "%~1" "extension\%~4" "%OPTIONS%" "%~2.obj"
)
endlocal & exit /b

:shlib_ld
:: %1 - dll name
:: %2, %3, ... - object file(s) (pathnames)
call :res GAWKRES "%BLD_OBJ%\extension\%~1" pc\version.rc "%~1" "%~1.dll" GAWK_EXTENSION_DLL "%~1.res.obj" || exit /b
call :ld shlib %1 %2 %3 %4 %5 %6 %7 %8 %9 "%BLD_OBJ%\extension\%~1\%~1.res.obj"
exit /b

:helpers
if not exist "%BLD_DIST%\helpers" call :execq "md ""%BLD_DIST%\helpers""" || exit /b
if not exist "%BLD_OBJ%\helpers" call :execq "md ""%BLD_OBJ%\helpers""" || exit /b
call :cc TOOLCC "%BLD_OBJ%\helpers" helpers\tst_ls.c || exit /b
call :res GAWKRES "%BLD_OBJ%\helpers" pc\version.rc "tst_ls" "tst_ls.exe" GAWK_TEST_HELPER "tst_ls.res.obj" || exit /b
:: note: next lines after ^ _must_ begin with a space - because they are not commands
call :ld tool tst_ls ^
 "%BLD_OBJ%\helpers\tst_ls.obj"     ^
 "%BLD_OBJ%\helpers\tst_ls.res.obj" ^
 "%BLD_OBJ%\mscrtx\mscrtx.a"        ^
 "%BLD_OBJ%\libutf16\libutf16.a"    ^
 "%BLD_OBJ%\unicode_ctype\unicode_ctype.a" || exit /b
exit /b 0

::::::::::::::::: support routines :::::::::::::

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
:: call :execq "%GAWK% ""BEGIN { for ^(i = 1; i ^<= 1030; i++^) print i, i}"" >_manyfiles"
 
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

:entryname
set "ENTRYNAME=%~nx1"
exit /b

:cc
:: compiler
:: %1 - GAWKCC, SHLIBCC, TOOLCC ...
:: %2 - objdir name
:: %3 - support\getopt.c
:: %4 - (optional) additional compiler options
:: %5 - (optional) getopt.obj - name of object file
setlocal
call :entryname "%~3"
call set "CC=%%%~1%%"
:: %~4 doubles ^ in the options, un-double it
set "OPTIONS=%~4"
if defined OPTIONS set "OPTIONS=""%OPTIONS:^^=^%"""
if ""=="%~5" (
  call :execq "%CC:^^=^%""%~2\%ENTRYNAME:.c=.obj%"" ""%~3"" %OPTIONS%"
) else (
  call :execq "%CC:^^=^%""%~2\%~5"" ""%~3"" %OPTIONS%"
)
endlocal & exit /b

:res
:: resource compiler
:: %1 - GAWKRES ...
:: %2 - objdir name
:: %3 - resource.rc
:: %4 - module name (gawk)
:: %5 - module file (gawk.exe)
:: %6 - (optional) GAWK_EXTENSION_DLL/GAWK_TEST_HELPER
:: %7 - (optional) resource.res.obj
setlocal
call :entryname "%~3"
call set "RES=%%%~1%%"
set "OPTIONS=%~6"
if defined OPTIONS set "OPTIONS=%CC_DEFINE%%OPTIONS%"
set "OPTIONS=%OPTIONS% %CC_DEFINE%GAWK_MODULE_NAME=""%~4"" %CC_DEFINE%GAWK_MODULE_FILE_NAME=""%~5"""
call set "RES=%%RES:%CC_DEFINE%GAWK_VER_MAJOR=%OPTIONS% %CC_DEFINE%GAWK_VER_MAJOR%%"
if ""=="%~7" (
  call :execq "%RES:^^=^%""%~2\%ENTRYNAME:.rc=.res.obj%"" ""%~3"""
) else (
  call :execq "%RES:^^=^%""%~2\%~7"" ""%~3"""
)
endlocal & exit /b

:lib
:: static library linker
:: %* - support\libsupport.a support\getopt.obj support\getopt1.obj ...
setlocal
set "LIBOBJS=|%*"
call set "LIBOBJS=%%LIBOBJS:|%1 =%%"
call :execq "%LIBABSTRACTLIB:^^=^%""%~1"" %LIBOBJS:"=""%"
endlocal & exit /b

:ld
:: exe/shared library linker
:: %* - awk gawk array.obj ...
:: %* - shlib testext.dll extension\testext.obj ...
setlocal
set "LDOBJS=|%*"
call set "LDOBJS=%%LDOBJS:|%1 =|%%"
call set "LDOBJS=%%LDOBJS:|%2 =%%"
:: call cl_awk_ld, cl_shlib_ld, cl_tool_ld, gcc_awk_ld, gcc_shlib_ld, gcc_tool_ld ...
call :%TOOLCHAIN%_%~1_ld "%~2" "%LDOBJS:"=""%"
endlocal & exit /b

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

:extract_ver
:: %1 - CLANGMSVCVER
:: %2 - "%CLANGMSVC% --version 2>&1"
:: %3 - "clang version "
setlocal
set LANG=C
call :extract_ver1 %2 %3 || exit /b 1
endlocal & set "%1=%VER%" & set "GCC_IS_MINGW_ORG=%GCC_IS_MINGW_ORG%" & set "BUILDFORCPU=%BUILDFORCPU%" & exit /b 0

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
if not defined VER exit /b 1
if not defined BUILDFORCPU exit /b 1
exit /b 0

:extract_ver2
:: %1 - prefix string
:: %2 - some text
set "TEXT=%~2"
if not defined BUILDFORCPU if not "%TEXT:Target: x86_64=%"=="%TEXT%" set BUILDFORCPU=64
if not defined BUILDFORCPU if not "%TEXT:Target: i686=%"=="%TEXT%"   set BUILDFORCPU=32
if not defined BUILDFORCPU if not "%TEXT: for x64=%"=="%TEXT%"       set BUILDFORCPU=64
if not defined BUILDFORCPU if not "%TEXT: for x86=%"=="%TEXT%"       set BUILDFORCPU=32
if not defined BUILDFORCPU if "%TEXT%"=="Target: mingw32"            set BUILDFORCPU=32
call set "TEXT=%%TEXT:%~1=%%"
if "%TEXT%"=="%~2" exit /b 1
if not "%TEXT:MinGW.org=%"=="%TEXT%" set GCC_IS_MINGW_ORG=1
for /f "tokens=1" %%b in ("%TEXT%") do (set "VER=%%b")
call :combine_ver VER || exit /b
:: fast-exit if both VER & BUILDFORCPU are defined
if defined BUILDFORCPU exit /b
exit /b 1

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

:getdllstartup
:: %1 - GCC, CLANG
call :getdllstartup1 %1
if exist "%BLD_OBJ%\aaa.dll" del /f "%BLD_OBJ%\aaa.dll" && exit /b
echo failed to determine dll startup function
exit /b 1

:getdllstartup1
set "SHLIBDLLMAIN=_ExtDllMain@12"
call set "COMMAND=%%%1:""="%%"
if not exist "%BLD_OBJ%" md "%BLD_OBJ%" || exit /b
for /f "tokens=* delims=" %%a in ('%%COMMAND%% --verbose -shared -x c -o "%BLD_OBJ%\aaa.dll" - ^<NUL 2^>^&1') do (call :checkdllmaincrt %%a && exit /b)
set "SHLIBDLLMAIN=ExtDllMain"
exit /b

:checkdllmaincrt
set "COMMAND=%*"
if not defined COMMAND exit /b 1
set "COMMAND=%COMMAND:"=""%"
if "%COMMAND:_DllMainCRTStartup@12=%"=="%COMMAND%" exit /b 1
exit /b 0

:getgawkver
setlocal
set PACKAGE_VERSION=
for /f "tokens=2,3" %%a in (pc\config.h) do (
  if "%%a"=="PACKAGE_VERSION" set PACKAGE_VERSION=%%b
)
if not defined PACKAGE_VERSION exit /b 1
set GAWK_VER_MAJOR=
set GAWK_VER_MINOR=
set GAWK_VER_PATCH=
for /f "tokens=1,2,3 delims=." %%a in (%PACKAGE_VERSION%) do (
  set "GAWK_VER_MAJOR=%%a"
  set "GAWK_VER_MINOR=%%b"
  set "GAWK_VER_PATCH=%%c"
)
if not defined GAWK_VER_MAJOR exit /b 1
if not defined GAWK_VER_MINOR exit /b 1
if not defined GAWK_VER_PATCH exit /b 1
set GAWK_YEAR=
set GAWK_MONTH=
set GAWK_WEEK=
set GAWK_DOW=
set GAWK_DAY=
set GAWK_HOUR=
set GAWK_MINUTE=
:: current date & time, in UTC
for /f "tokens=1,2 delims==" %%a in ('%%SYSTEMROOT%%\System32\Wbem\wmic.exe path Win32_UTCTime get /value') do (
  if "Year"=="%%a"        call set "GAWK_YEAR=%%b"
  if "Month"=="%%a"       call set "GAWK_MONTH=%%b"
  if "WeekInMonth"=="%%a" call set "GAWK_WEEK=%%b"
  if "DayOfWeek"=="%%a"   call set "GAWK_DOW=%%b"
  if "Day"=="%%a"         call set "GAWK_DAY=%%b"
  if "Hour"=="%%a"        call set "GAWK_HOUR=%%b"
  if "Minute"=="%%a"      call set "GAWK_MINUTE=%%b"
)
if not defined GAWK_YEAR   exit /b 1
if not defined GAWK_MONTH  exit /b 1
if not defined GAWK_WEEK   exit /b 1
if not defined GAWK_DOW    exit /b 1
if not defined GAWK_DAY    exit /b 1
if not defined GAWK_HOUR   exit /b 1
if not defined GAWK_MINUTE exit /b 1
:: date format: DD.MM.YYYY/hh:mm
set GAWK_DAY=0%GAWK_DAY%
set GAWK_BUILD_DATE=%GAWK_DAY:~-2%
set GAWK_DAY=0%GAWK_MONTH%
set GAWK_BUILD_DATE=%GAWK_BUILD_DATE%.%GAWK_DAY:~-2%.%GAWK_YEAR%
set GAWK_DAY=0%GAWK_HOUR%
set GAWK_BUILD_TIME=%GAWK_DAY:~-2%
set GAWK_DAY=0%GAWK_MINUTE%
set GAWK_BUILD_TIME=%GAWK_BUILD_TIME%:%GAWK_DAY:~-2%
set /A GAWK_MONTH-=1
set /A GAWK_MONTH*=4
set /A GAWK_WEEK+=%GAWK_MONTH%
set GAWK_BUILD_NUM=%GAWK_YEAR:~-2%
:: 7 bits for year (0-99), 6 bits for week-in-year (1-48), 3 bits for day-in-week (1-7)
set /A GAWK_BUILD_NUM*=512
set /A GAWK_WEEK*=8
set /A GAWK_BUILD_NUM+=%GAWK_WEEK%
set /A GAWK_BUILD_NUM+=%GAWK_DOW%
endlocal & set "GAWK_VER_DEFINES=/DGAWK_VER_MAJOR=%GAWK_VER_MAJOR% /DGAWK_VER_MINOR=%GAWK_VER_MINOR% /DGAWK_VER_PATCH=%GAWK_VER_PATCH% /DGAWK_BUILD_NUM=%GAWK_BUILD_NUM% /DGAWK_YEAR=%GAWK_YEAR% /DGAWK_BUILD_DATE=%GAWK_BUILD_DATE% /DGAWK_BUILD_TIME=%GAWK_BUILD_TIME%" & exit /b 0
