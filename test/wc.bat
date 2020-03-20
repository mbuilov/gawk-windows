@echo off

setlocal

set "where=%~1"

if not defined where goto :stdin

rem emulate "wc -l junk/*"

set total=0
for %%a in ("%where%") do (
  call :count_lines "%%a"
)
echo %total% total
exit /b

:count_lines
set lines=0
for /f "usebackq" %%b in ("%~1") do (
  set /a lines+=1
)
echo %lines% %~1
set /a total+=%lines%
exit /b

:stdin
rem emulate "wc -l"

set lines=0
for /f %%b in ('more') do (
  set /a lines+=1
)
echo %lines%
