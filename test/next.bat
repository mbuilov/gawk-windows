@echo off
setlocal

rem next.sh ported to windows

set "AWK=%~1"

if not defined AWK ((echo Usage: %0 ^<path-to-gawk^>)>&2 & exit /b 1)

rem non-fatal 
%AWK% "{next}" NUL
%AWK% "function f() { next}; {f()}" NUL
rem fatal
%AWK% "function f() { next}; BEGIN{f()}"
%AWK% "function f() { next}; {f()}; END{f()}" NUL
%AWK% "function f() { next}; BEGINFILE{f()}"
%AWK% "function f() { next}; {f()}; ENDFILE{f()}" NUL

exit /b 0
