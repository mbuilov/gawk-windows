#!/bin/bash

# eventuali file di lavoro da elaborazioni precedenti
rm -f gawk-it-17x24.pdf gawk-it.toc gawk-it.aux gawk-it.cp gawk-it.texi 2>/dev/null
	
sed '{
   s/@example/@smallexample/g
   s/@end example/@end smallexample/g
  }' gawktexi.in > gawktexi-tmp.in

awk -f sidebar.awk < gawktexi-tmp.in > gawk-it.texi

texi2dvi -t "@set SMALLPRINT" -t "@set FOR_PRINT" -t @manuale -t "@include margini.texi" gawk-it.texi -o gawk-it-17x24.dvi

dvipdfmx --dvipdfm -p 170mm,240mm  gawk-it-17x24.dvi

awk -f sidebar.awk < gawktexi.in > gawk-it.texi

rm gawktexi-tmp.in
