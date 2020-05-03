@echo off
setlocal
rem print current date and time in GMT
rem like Unix command "LC_ALL=C TZ=GMT0 date"
rem i.g.: Tue Mar 24 13:29:07 GMT 2020

for /f "tokens=1,2 delims==" %%a in ('wmic path Win32_UTCTime get /value') do (
  if "%%a"=="Day"       (set "day=0%%b")         else (
  if "%%a"=="Hour"      (set "hour=0%%b")        else (
  if "%%a"=="Minute"    (set "minute=0%%b")      else (
  if "%%a"=="Second"    (set "second=0%%b")      else (
  if "%%a"=="Year"      (set "year=%%b")         else (
  if "%%a"=="DayOfWeek" (call :choose_day "%%b") else (
  if "%%a"=="Month"     (call :choose_month "%%b")))))))
)

rem strip excessive leading zero and trailing unprintable symbol
set "day=%day:~-3,-1%"
set "hour=%hour:~-3,-1%"
set "minute=%minute:~-3,-1%"
set "second=%second:~-3,-1%"
set "year=%year:~0,-1%"

rem replace leading 0 of 'day' with space
if "%day:~-2,1%"=="0" set "day= %day:~-1%"

(echo.%dow% %month% %day% %hour%:%minute%:%second% GMT %year%)
exit /b

:choose_day
set "v=%~1"
if "%v%"=="0"  (set dow=Sun) else (
if "%v%"=="1"  (set dow=Mon) else (
if "%v%"=="2"  (set dow=Tue) else (
if "%v%"=="3"  (set dow=Wed) else (
if "%v%"=="4"  (set dow=Thu) else (
if "%v%"=="5"  (set dow=Fri) else (
if "%v%"=="6"  (set dow=Sat)))))))
exit /b

:choose_month
set "v=%~1"
if "%v%"=="1"  (set month=Jan) else (
if "%v%"=="2"  (set month=Feb) else (
if "%v%"=="3"  (set month=Mar) else (
if "%v%"=="4"  (set month=Apr) else (
if "%v%"=="5"  (set month=May) else (
if "%v%"=="6"  (set month=Jun) else (
if "%v%"=="7"  (set month=Jul) else (
if "%v%"=="8"  (set month=Aug) else (
if "%v%"=="9"  (set month=Sep) else (
if "%v%"=="10" (set month=Oct) else (
if "%v%"=="11" (set month=Nov) else (
if "%v%"=="12" (set month=Dec))))))))))))
exit /b
