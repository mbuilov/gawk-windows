#
# CMakeLists.txt --- CMake input file for gawk's support library
#
# Copyright (C) 2016
# the Free Software Foundation, Inc.
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

add_definitions(-DGAWK)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-D_GNU_SOURCE)
add_library (support STATIC
  dfa.c
  getopt1.c
  getopt.c
  localeinfo.c
  random.c
  regex.c
)
