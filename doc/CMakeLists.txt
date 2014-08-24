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

MACRO(DocDependency outfile)
  add_dependencies(doc ${outfile})
  add_custom_target(
    ${outfile}
    DEPENDS ${ARGN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_SOURCE_DIR}/cmake/docmaker ${outfile} ${ARGN}
  )
ENDMACRO(DocDependency)

find_program(TEXI2DVI_CONVERTER texi2dvi)
if (TEXI2DVI_CONVERTER)
  add_custom_target(doc)
  DocDependency(gawk.texi gawktexi.in rflashlight.eps api-figure1.fig api-figure2.fig api-figure3.fig general-program.fig process-flow.fig)
  DocDependency(rflashlight.eps)
  DocDependency(api-figure1.fig)
  DocDependency(api-figure2.fig)
  DocDependency(api-figure3.fig)
  DocDependency(general-program.fig)
  DocDependency(process-flow.fig)
  DocDependency(gawk.dvi gawk.texi)
  DocDependency(gawk.info gawk.texi)
  DocDependency(gawkinet.dvi gawkinet.texi)
  DocDependency(gawkinet.info gawkinet.texi)
  DocDependency(gawkinet.texi statist.eps)
  DocDependency(gawk.1.ps gawk.1)
  DocDependency(igawk.1.ps igawk.1)
  find_program(DVIPS_CONVERTER dvips)
  if (DVIPS_CONVERTER)
    DocDependency(gawk.ps gawk.dvi)
    DocDependency(gawkinet.ps gawkinet.dvi)
    find_program(PS2PDF_CONVERTER ps2pdf)
    if (PS2PDF_CONVERTER)
      DocDependency(gawk.1.pdf gawk.1.ps)
      DocDependency(igawk.1.pdf igawk.1.ps)
      DocDependency(gawk.pdf gawk.ps)
      DocDependency(gawkinet.pdf gawkinet.ps)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawk.1.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/igawk.1.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawk.info DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawk.pdf DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawkinet.info DESTINATION doc)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gawkinet.pdf DESTINATION doc)

      set(CARDSRC macros cardfonts colors awkcard.tr)
      set(CARDSRC_N macros cardfonts no.colors awkcard.tr)
      set(CARDFILES  ${CARDSRC} ad.block awkcard.in setter.outline)
      DocDependency(awkcard.tr awkcard.in)
      DocDependency(awkcard.nc ${CARDFILES})
      DocDependency(awkcard.ps ${CARDFILES})
      DocDependency(awkcard.pdf awkcard.ps)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/awkcard.pdf DESTINATION doc)

    else()
      message(WARNING "Found no ps2pdf tool; no doc will be generated")
      install(CODE "MESSAGE(\"doc generated only in .ps files\")")
    endif()
  else()
    message(WARNING "Found no dvips tool; no doc will be generated")
    install(CODE "MESSAGE(\"doc generated only in .dvi files and man pages in .ps files\")")
  endif()
else()
  message(WARNING "Found no texi2dvi tool; no doc will be generated")
  add_custom_command(
    TARGET doc
    COMMAND echo no doc generated because of missing texi2dvi
  )
endif()

