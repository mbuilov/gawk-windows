#
# doc/Makefile.am --- automake input file for gawk
#
# Copyright (C) 2000, 2001, 2002, 2004, 2005, 2007, 2010, 2011
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

## process this file with automake to produce Makefile.in

info_TEXINFOS = gawk.texi gawkinet.texi

man_MANS = gawk.1 igawk.1

EXTRA_DIST = ChangeLog ChangeLog.0 README.card ad.block setter.outline \
	awkcard.in awkforai.txt texinfo.tex cardfonts \
	general-program.eps general-program.fig general-program.pdf \
	process-flow.eps process-flow.fig process-flow.pdf \
	macros colors no.colors $(man_MANS) \
	lflashlight-small.xpic lflashlight.eps lflashlight.pdf \
	rflashlight-small.xpic rflashlight.eps rflashlight.pdf \
	statist.jpg statist.eps statist.pdf \
	bc_notes

# Get rid of generated files when cleaning
CLEANFILES = *.ps *.html *.dvi *~ awkcard.nc awkcard.tr gawk.pdf gawkinet.pdf awkcard.pdf gawk.1.pdf igawk.1.pdf

MAKEINFO = @MAKEINFO@ --no-split --force

TROFF = groff -t -Tps -U
SEDME = sed -e "s/^level0 restore/level0 restore flashme 100 72 moveto (Copyright `date '+%m-%d-%y %T'`, FSF, Inc. (all)) show/" \
		-e "s/^\/level0 save def/\/level0 save def 30 -48 translate/"
SEDME2 = sed '/%%Page: 10 10/,/0 Cg EP/d'

CARDSRC = $(srcdir)/macros $(srcdir)/cardfonts $(srcdir)/colors awkcard.tr
CARDSRC_N = $(srcdir)/macros $(srcdir)/cardfonts $(srcdir)/no.colors awkcard.tr
CARDFILES= $(CARDSRC) ad.block awkcard.in setter.outline

# Use this if your troff can correctly handle macros from 'colors' file
AWKCARD = awkcard.ps

# Uncomment the following definition of AWKCARD if your troff can produce
# Postscript but still has troubles with macros from 'colors'.  As this
# is not groff you will have to change TROFF macro as well.  Do not forget
# to ensure that awkcard.tr is processed by tbl.
#AWKCARD = awkcard.nc

# The following is patterned after the main Makefile.am.  The point is to
# make pgawk.1 a link to gawk.1 in the installed man directory.

# We want hard links for install-data-hook, below
LN= ln -f

# Link gawk.1 to pgawk.1
install-data-hook:
	(cd $(DESTDIR)$(man1dir); \
	$(LN) gawk.1 pgawk.1 2>/dev/null ; \
	exit 0)

# Undo the above when uninstalling
uninstall-hook:
	cd $(DESTDIR)$(man1dir); rm -f pgawk.1 ; exit 0

postscript: gawk.ps gawkinet.ps gawk.1.ps igawk.1.ps $(AWKCARD)

pdf: postscript gawk.pdf gawkinet.pdf awkcard.pdf gawk.1.pdf igawk.1.pdf

gawk.ps: gawk.dvi
	dvips -o gawk.ps gawk.dvi

gawkinet.ps: gawkinet.dvi
	dvips -o gawkinet.ps gawkinet.dvi

gawk.1.ps: gawk.1
	-groff -man $(srcdir)/gawk.1 > gawk.1.ps

gawk.1.pdf: gawk.1.ps
	ps2pdf gawk.1.ps gawk.1.pdf

igawk.1.ps: igawk.1
	-groff -man $(srcdir)/igawk.1 > igawk.1.ps

igawk.1.pdf: igawk.1.ps
	ps2pdf igawk.1.ps igawk.1.pdf

awkcard.tr: awkcard.in
	sed 's:SRCDIR:$(srcdir):' < $(srcdir)/awkcard.in > awkcard.tr

awkcard.ps: $(CARDFILES)
	$(TROFF) $(CARDSRC) | $(SEDME) | cat $(srcdir)/setter.outline - | $(SEDME2) > awkcard.ps

awkcard.nc: $(CARDFILES)
	$(TROFF) $(CARDSRC_N) | $(SEDME) | cat $(srcdir)/setter.outline - | $(SEDME2) > awkcard.ps && touch awkcard.nc

awkcard.pdf: awkcard.ps
	ps2pdf awkcard.ps awkcard.pdf
