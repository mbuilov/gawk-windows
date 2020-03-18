@echo off
setlocal
for /F "tokens=*" %%a in ('more') do (
  call :echo_var "%%a"
)
endlocal
exit /b

:echo_var
set "V=%~1"
set "V=%V:0=.%"
set "V=%V:1=.%"
set "V=%V:2=.%"
set "V=%V:3=.%"
set "V=%V:4=.%"
set "V=%V:5=.%"
set "V=%V:6=.%"
set "V=%V:7=.%"
set "V=%V:8=.%"
set "V=%V:9=.%"
echo %V%
exit /b
