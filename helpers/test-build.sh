#! /bin/bash

MIXED_COMPILERS=${MIXED_COMPILERS:-gcc /usr/gcc/bin/gcc clang}
OTHER_COMPILERS=${OTHER_COMPILERS:-tcc pcc}

rm -f compile-results.txt

compile () {
	make -k
	if make check
	then
		echo success: $1 $2 >> compile-results.txt
	else
		echo failure: $1 $2 >> compile-results.txt
	fi
}

configure_and_compile () {
	if type -p $1 >> compile-results.txt 2>&1
	then
		for j in "" --disable-mpfr
		do
			./configure $j CC="$1"
			compile "$1" "$j"
			make distclean
		done
	fi
}

(make distclean)

for i in $OTHER_COMPILERS $MIXED_COMPILERS
do
	configure_and_compile $i
done

for i in $MIXED_COMPILERS
do
	configure_and_compile "$i -m32"
done

echo
echo ==========================================
echo
case $(grep failure compile-results.txt | wc -l) in
0)	echo No failures!
	# rm compile-results.txt
	;;
*)	echo The following combinations failed:
	grep failure compile-results.txt
	;;
esac
