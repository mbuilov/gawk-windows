setlocal

rem windows version of exit.sh

set "AWK=%~1"
if not defined AWK (
	echo Usage: %0 ^<path-to-gawk.exe^> >&2
	exit /b 1
)

rem Use awk to print the string instead of echo or printf.

set "x=BEGIN{print 1; exit; print 2}; NR>1{print}; END{print 3; exit; print 4}"
"%AWK%" "BEGIN { print ""a\nb"" }" | "%AWK%" "%x%"
echo -- 1

set "x=function f(){ exit}; END{print NR;f();print NR}"
"%AWK%" "BEGIN { print ""a\nb"" }" | "%AWK%" "%x%"
echo -- 2

set "x=function f(){ exit}; NR>1 {f()}; END{print NR; f();print NR}"
"%AWK%" "BEGIN { print ""a\nb"" }" | "%AWK%" "%x%"
echo -- 3

set "x=function f(){ exit}; NR>1{ f()}; END{print NR;print NR}"
"%AWK%" "BEGIN { print ""a\nb"" }" | "%AWK%" "%x%"
echo -- 4

set "x=function f(){ exit}; BEGINFILE {f()}; NR>1{ f()}; END{print NR}"
"%AWK%" "BEGIN { print ""a\nb"" }" | "%AWK%" "%x%"
echo -- 5

set "y=function strip(val) { sub(/.*\//, """", val); return val };"

set "x=BEGINFILE{if(++i==1) exit;}; END{print i, strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 6

set "x=BEGINFILE{if(++i==1) exit;}; ENDFILE{print i++}; END{print i, strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 7

set "x=function f(){ exit}; BEGINFILE{i++ && f()}; END{print NR,strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 8

set "x=function f(){ exit}; BEGINFILE{i++ && f()}; ENDFILE{ print i}; END{print NR,strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 9

set "x=function f(){ exit}; BEGINFILE{i++}; ENDFILE{ f(); print i}; END{print NR,strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 10

set "x=function f(){ exit}; BEGINFILE{i++}; ENDFILE{ i>1 && f(); print i, strip(FILENAME)}"
"%AWK%" "%y%%x%" /dev/null "%0"
echo -- 11
