rem ported from localenl.sh
rem POSIX locale not supported by the Windows C-runtime library
rem use English_USA.28591 as alternative name of en_US.ISO-8859-1
setlocal
for %%a in (C English_USA.28591 en_US.UTF-8) do (
  set "LC_ALL=%%a"
  (
  ^(echo.line1^)&^
  ^(echo.line2^)&^
  ^(echo.line3^)&^
  ^(echo.line4^)&^
  ^(echo.line5^)&^
  ^(echo.line6^)&^
  ^(echo.line7^)&^
  ^(echo.line8^)&^
  ^(echo.line9^)
  ) | %* --posix -f localenl.awk || ((echo.LC_ALL=%%a FAILED) & endlocal & exit /b)
  (echo.LC_ALL=%%a passed)
)
endlocal
