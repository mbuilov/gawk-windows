REM Simple brute force command file for building gawk under msdos
REM
REM *** This has only been tested using MSC 5.1 and MSC 6.00A ***
REM
REM Written by Arnold Robbins, May 1991
REM Modified by Scott Deifik, July, 1992, Sep 1993
REM Based on earlier makefile for dos
REM
REM Copyright (C) 1986, 1988, 1989, 1991 the Free Software Foundation, Inc.
REM 
REM This file is part of GAWK, the GNU implementation of the
REM AWK Progamming Language.
REM 
REM GAWK is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 2 of the License, or
REM (at your option) any later version.
REM 
REM GAWK is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM 
REM You should have received a copy of the GNU General Public License
REM along with GAWK; see the file COPYING.  If not, write to
REM the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
REM
REM  compile debug flags: -DDEBUG -DFUNC_TRACE -DMEMDEBUG -Zi -Od
REM			  

set CFLAGS=-D_MSC_VER

rem MSC 5.1 should use:
rem set CFLAGS=-D_MSC_VER=510

rem MSC 6.00A predefines _MSC_VER
rem set CFLAGS=

rem -Za sets ANSI flag so that __STDC__ is defined in MSC 6.00A 
rem (MSC 5.1 sets __STDC__=0 regardless of ANSI switch)

cl -Za -c -AL %CFLAGS% -DGAWK array.c
cl -Za -c -AL %CFLAGS% -DGAWK awktab.c
cl -Za -c -AL %CFLAGS% -DGAWK builtin.c
cl -Za -c -AL %CFLAGS% -DGAWK dfa.c
cl -Za -c -AL %CFLAGS% -DGAWK eval.c
cl -Za -c -AL %CFLAGS% -DGAWK field.c
cl -Za -c -AL %CFLAGS% -DGAWK getid.c
cl -Za -c -AL %CFLAGS% -DGAWK getopt.c
cl -Za -c -AL %CFLAGS% -DGAWK getopt1.c
cl -Za -c -AL %CFLAGS% -DGAWK io.c
cl -Za -c -AL %CFLAGS% -DGAWK iop.c
cl -Za -c -AL %CFLAGS% -DGAWK main.c
cl -Za -c -AL %CFLAGS% -DGAWK missing.c
cl -Za -c -AL %CFLAGS% -DGAWK msg.c
cl -Za -c -AL %CFLAGS% -DGAWK node.c
cl -Za -c -AL %CFLAGS% -DGAWK popen.c
cl -Za -c -AL %CFLAGS% -DGAWK re.c
REM You can ignore the warnings you will get
cl -Za -c -AL %CFLAGS% -DGAWK regex.c
cl -Za -c -AL %CFLAGS% -DGAWK version.c
REM
REM link debug flags: /CO /NOE /NOI /st:30000
REM
link @names.lnk,gawk.exe /NOE /NOI /st:30000;
