#! /bin/sh

if [ ! -d .git ]
then
	echo you must run this script from the top level directory
	exit 1
fi

git checkout master || exit

features=$(git branch -a | grep /origin/feature/ | sed 's;.*/origin/;;')
others="porting"

for i in $others $features
do
	(git checkout $i && git pull && git merge master && git push) || break
done

git checkout master || exit
