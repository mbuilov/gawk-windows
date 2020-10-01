@echo off
for /f "delims=" %%f in ('find /v ""') do (echo.%%f)
