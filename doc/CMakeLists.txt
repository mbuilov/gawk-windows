#
# doc/CMakeLists.txt --- CMake input file for gawk
#
# Copyright (C) 2013
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
#

## process this file with CMake to produce Makefile

add_custom_target(doc)
add_custom_command(
  TARGET doc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND texi2dvi  --clean gawkinet.texi gawk.texi
  COMMAND groff -man gawk.1 > gawk.1.ps
  COMMAND groff -man igawk.1 > igawk.1.ps
)

if (DVIPS_CONVERTER)
  add_custom_command(
    TARGET doc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND dvips -o gawk.ps gawk.dvi
    COMMAND dvips -o gawkinet.ps gawkinet.dvi
  )
  if (PS2PDF_CONVERTER)
    add_custom_command(
      TARGET doc
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND ps2pdf gawk.1.ps gawk.1.pdf
      COMMAND ps2pdf igawk.1.ps igawk.1.pdf
      COMMAND ps2pdf gawk.1.ps gawk.1.pdf
      COMMAND ps2pdf gawkinet.ps gawkinet.pdf
    )
  endif()
endif()

