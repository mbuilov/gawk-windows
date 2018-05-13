# Makefile for GNU Awk test suite.
#
# Copyright (C) 1988-2018 the Free Software Foundation, Inc.
# 
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
# 
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

# ============================================================================
# MS-DOS & OS/2 Notes: READ THEM!
# ============================================================================

# As of version 2.91, efforts to make this makefile run in MS-DOS and OS/2
# have started in earnest.  The following steps need to be followed in order 
# to run this makefile:
#
# 1. The first thing that you will need to do is to convert all of the 
#    files ending in ".ok" in the test directory, all of the files ending 
#    in ".good" (or ".goo") in the test/reg directory, and mmap8k.in from
#    having a linefeed to having carriage return/linefeed at the end of each
#    line. There are various public domain UNIX to DOS converters and any 
#    should work.  Alternatively, you can use diff instead of cmp--most 
#    versions of diff don't care about how the lines end.
#
# 2. You will need an sh-compatible shell.  Please refer to the "README.pc"
#    file in the README_d directory for information about obtaining a copy.
#    You will also need various UNIX utilities.  At a minimum, you will 
#    need: rm, tr, cmp (or diff, see above), cat, wc, and sh.  
#    You should also have a UNIX-compatible date program.
#
# The makefile has only been tested with dmake 3.8 and DJGPP Make 3.74 or
# later.  After making all of these changes, typing "dmake check extra"
# or "make check extra" (with DJGPP Make) should run successfully.

# The Bash shell (compiled with djgpp) works very well with the
# djgpp-compiled gawk.  It is currently the recommended shell to use
# for testing, along with DJGPP make.  See README.pc for 
# more information on OS/2 and DOS shells.

# You will almost certainly need to change some of the values (MACROS) 
# defined on the next few lines.  

# This won't work unless you have "sh" and set SHELL equal to it (Make 3.74
# or later which comes with DJGPP will work with SHELL=/bin/sh if you have
# sh.exe anywhere on your PATH).
#SHELL = e:\bin\sh.exe
SHELL = /bin/sh

# Point to gawk
AWK = AWKLIBPATH=../extension $(AWKPROG)
AWKPROG = ../gawk.exe
# Point $(LS) to a version of ls.exe that reports true Windows file
# index numbers, because this is what the readdir test expects.
# Otherwise, the readdir test will fail.  (The MSYS ls.exe doesn't
# report full index values.)
LS = ls.exe

# Set your cmp command here (you can use most versions of diff instead of cmp
# if you don't want to convert the .ok files to the DOS CR/LF format).
# This is also an issue for the "mmap8k" test.  If it fails, make sure that
# mmap8k.in has CR/LFs or that you've used diff.
#
# The following comment is for users of OSs which support long file names
# (such as Windows 95) for all versions of gawk (both 16 & 32-bit).
# If you use a shell which doesn't support long filenames, temporary files
# created by this makefile will be truncated by your shell.  "_argarra" is an
# example of this.  If $(CMP) is a DJGPP-compiled program, then it will fail
# because it looks for the long filename (eg. _argarray).  To fix this, you
# need to set LFN=n in your shell's environment.
# NOTE: Setting LFN in the makefile most probably won't help you because LFN
# needs to be an environment variable.
#CMP = cmp
# See the comment above for why you might want to set CMP to "env LFN=n diff"
#CMP = env LFN=n diff
#CMP = diff
CMP = diff -u
#CMP = gcmp

# cmp replacement program for PC where the error messages aren't
# exactly the same.  Should run even on old awk.
TESTOUTCMP = $(AWK) -f ../testoutcmp.awk

# Set your "cp," "mv," and "mkdir" commands here.  Note: DOS's copy must take
# forward slashes.
CP = cp
#CP = : && command -c copy
#CP  = command.com /c copy

#MV = cmd.exe /c ren
MV = mv

MKDIR = mkdir
#MKDIR = gmkdir
#MKDIR = : && command -c mkdir
#MKDIR  = command.com /c mkdir

# Set your unix-style date function here
#DATE = date
DATE = gdate

# Set sort command
SORT = sort

# MS-DOS and OS/2 use ; as a PATH delimiter
PATH_SEPARATOR = ;

# Argument to -F to produce -F/, should be // for MinGW, / otherwise,
# because MSYS Bash converts a single slash to the equivalent Windows
# directory.
SLASH = /
#SLASH = //

# Non-default GREP_OPTIONS might fail the badargs test
export GREP_OPTIONS=

# ============================================================================
# You shouldn't need to modify anything below this line.
# ============================================================================

srcdir = .
abs_srcdir = .
abs_builddir = .
top_srcdir = "$(srcdir)"/..
abs_top_builddir = "$(top_srcdir)"

