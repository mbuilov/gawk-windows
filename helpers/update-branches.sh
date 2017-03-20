#! /bin/sh

if [ ! -d .git ]
then
	echo you must run this script from the top level directory
	exit 1
fi

doit () {
	echo "
	Running: $@"
	"$@" || {
		echo "Oops: command [$@] failed with status $?"
		return 1
	}
}

doit git checkout master || exit

features=$(git branch -a | grep /origin/feature/ | sed 's;.*/origin/;;')
others="porting"

for i in $others $features
do
	echo "
	Updating branch $i"
	(doit git checkout $i && doit git pull && doit git merge master && doit git push) || {
		echo "
Error encountered updating branch $i.
Please resolve the conflict and push it manually in a separate window.
Please hit enter when you are done so we may continue to merge into
the other branches."
		read x
	}
done

doit git checkout master
