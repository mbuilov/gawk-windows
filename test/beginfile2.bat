:: ported version of beginfile2.sh
:: beginfile2.bat --- test BEGINFILE/ENDFILE/getline/nextfile/exit combinations
:: note: --Test 10-- counts lines of this batch file
@echo off
setlocal
set "AWKPROG=beginfile2.in"
set "SCRIPT=%~nx0"

if not defined AWK (
        echo %0: You must set AWK >&2
        exit /b 1
)

echo --Test 1a--
"%AWK%" " /#TEST1#/, /#TEST2#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG%
echo --Test 1b--
"%AWK%" -f _beginfile2_prog %AWKPROG% file/does/not/exist

echo --Test 2--
"%AWK%" " /#TEST2#/, /#TEST3#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% file/does/not/exist

echo --Test 3--
"%AWK%" " /#TEST3#/, /#TEST4#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -vsrc=%SCRIPT% -f _beginfile2_prog %AWKPROG%

echo --Test 4--
"%AWK%" " /#TEST4#/, /#TEST5#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -vsrc=%SCRIPT% -f _beginfile2_prog %AWKPROG%

echo --Test 5--
"%AWK%" " /#TEST5#/, /#TEST6#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG%

echo --Test 6--
"%AWK%" " /#TEST6#/, /#TEST7#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG%

echo --Test 7--
"%AWK%" " /#TEST7#/, /#TEST8#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 8--
"%AWK%" " /#TEST8#/, /#TEST9#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG%

echo --Test 9a--
"%AWK%" " /#TEST9#/, /#TEST10#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog file/does/not/exist %AWKPROG%
echo --Test 9b--
"%AWK%" -vskip=1 -f _beginfile2_prog file/does/not/exist %AWKPROG%

echo --Test 10--
"%AWK%" " /#TEST10#/, /#TEST11#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 11--
"%AWK%" " /#TEST11#/, /#TEST12#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 12--
"%AWK%" " /#TEST12#/, /#TEST13#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 13--
"%AWK%" " /#TEST13#/, /#TEST14#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 14--
"%AWK%" " /#TEST14#/, /#TEST15#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 15--
"%AWK%" " /#TEST15#/, /#TEST16#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

echo --Test 16--
"%AWK%" " /#TEST16#/, /#TEST17#/" %AWKPROG% > _beginfile2_prog
"%AWK%" -f _beginfile2_prog %AWKPROG% %SCRIPT%

del /q _beginfile2_prog
