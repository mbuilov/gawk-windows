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
if (LATEX_COMPILER)
  add_custom_command(
    TARGET doc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND echo Compiling to .dvi for gawk.texi and gawkinet.texi
    COMMAND texi2dvi  -q --clean gawkinet.texi gawk.texi
    COMMAND echo Compiling to .ps for gawk.1 and igawk.1
    COMMAND groff -z -man gawk.1 > gawk.1.ps
    COMMAND groff -z -man igawk.1 > igawk.1.ps
  )

  if (DVIPS_CONVERTER)
    add_custom_command(
      TARGET doc
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND echo Compiling to .ps for gawk.dvi and gawkinet.dvi
      COMMAND dvips -q -o gawk.ps gawk.dvi
      COMMAND dvips -q -o gawkinet.ps gawkinet.dvi
    )
    if (PS2PDF_CONVERTER)
      add_custom_command(
        TARGET doc
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND echo Compiling to .pdf for gawk.1.ps, igawk.1.ps, gawk.ps and gawkinet.ps
        COMMAND ps2pdf -q gawk.1.ps gawk.1.pdf
        COMMAND ps2pdf -q igawk.1.ps igawk.1.pdf
        COMMAND ps2pdf -q gawk.ps gawk.pdf
        COMMAND ps2pdf -q gawkinet.ps gawkinet.pdf
      )
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawk.1.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/igawk.1.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawk.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawkinet.pdf DESTINATION doc)
    else()
      message(WARNING "Found no PS2PDF_CONVERTER; no doc will be generated")
      install(CODE "MESSAGE(\"doc generated only in .ps files\")")
    endif()
  else()
    message(WARNING "Found no DVIPS_CONVERTER; no doc will be generated")
    install(CODE "MESSAGE(\"doc generated only in .dvi files and man pages in .ps files\")")
  endif()
else()
  message(WARNING "Found no LATEX_COMPILER; no doc will be generated")
  add_custom_command(
    TARGET doc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND echo no doc generated because of missing LATEX_COMPILER
  )
endif()

