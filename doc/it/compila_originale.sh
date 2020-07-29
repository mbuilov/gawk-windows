
#
# builds the PDF version of the Gawk manual,
# in the current directory
#
echo "Building the pdf version of the gawk manual"
echo "in directory:"
pwd
echo "Beware, it can take a long time!"
if [ -f "gawktexi.in" ]
then
	gawk -f sidebar.awk gawktexi.in  >gawk.texi
fi
# just in case, drop previous Index
rm -f gawk.cps gawk.cp gawk.aux gawk.fn gawk.ky gawk.log gawk.pg
texi2pdf  gawk.texi 
