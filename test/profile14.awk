#: 200810_ Prettyprint weirdness to show Arnold
BEGIN {	
	IGNORECASE = 1; 
	printf("\n");
	printf("test \"%s\"\tin FUNCTAB=%s (good!)\n","masterlib::libfunc","masterlib::libfunc" in FUNCTAB);
	exit;
#..1.1.1...............
} # e-o-begin
#..2.2.2.............
@namespace "nlib"
function tstlib(note){ printf("Tstv1=%s;	from @namespace nlib\n",Tstv1);
}# e-o-function tstlib()
#..3.3.3.............
@namespace "masterlib"
# masterlib is library of "default" user-defined functions
function libfunc(note){ printf("%s:\tHello World from @namespace masterlib\n",note);}
#..4.4.4.............
@namespace "awk"
# return to program's awk space
function overridefunc(note){ printf("%s:\tHello Lone Star state from @namespace awk\n",note);}
# --5-5-5-----------
