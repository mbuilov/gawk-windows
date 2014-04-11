/*
 * Test out ' flag in different locales.
 * Michal Jaegermann
 * March, 2014
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
int
main(int argc, char **argv)
{
	const char *fmt;
	if (argc == 1)
		fmt = "%'.2f";
	else
		fmt = argv[1];

        setlocale(LC_ALL, getenv("LC_ALL"));
       	printf(fmt, 12456789.01);
	printf("\n");
	return 0;
}
