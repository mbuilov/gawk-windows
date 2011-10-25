@echo off
sed -n -f configpk.sed < ..\configure.ac > tmp.sed
sed -f config.sed < ..\configh.in > config.tmp
sed -f tmp.sed < config.tmp > config.h
del tmp.sed
del config.tmp
