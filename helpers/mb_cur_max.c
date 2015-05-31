#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

int main()
{
	setlocale(LC_ALL, "");
	printf("MB_CUR_MAX = %lu\n", MB_CUR_MAX);
	return 0;
}
