/*
 * Test out ' flag in different locales.
 * Michal Jaegermann
 * March, 2014
 */

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
int
main(int argc, char **argv)
{
        double t;

        if (argc == 1)
                return 1;

	setlocale(LC_ALL, getenv("LC_ALL"));
	sscanf(argv[1], "%lf", &t);
        printf("%.2f\n", t);
        return 0;
}
