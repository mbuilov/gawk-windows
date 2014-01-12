#! /bin/sh

# bootstrap.sh --- touch relevant files to avoid out-of-date issues in
#		   Git sandboxes

# Copyright (C) 2007, 2009-2014 the Free Software Foundation, Inc.
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

touch aclocal.m4
touch extension/aclocal.m4
find awklib -type f -print | xargs touch
sleep 1
touch configure
touch extension/configure
sleep 2
touch configh.in
touch extension/configh.in
sleep 1
touch test/Maketests
find . -name Makefile.in -print | xargs touch
touch doc/gawk.texi	# make later than gawktexi.in
sleep 1
touch doc/*.info
touch po/*.gmo
touch po/stamp-po
touch awkgram.c
touch command.c
