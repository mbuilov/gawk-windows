#include <stdio.h>
#include <locale.h>
#include <langinfo.h>

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	printf("D_T_FMT: %s\n", nl_langinfo(D_T_FMT));
#ifdef _DATE_FMT
	printf("_DATE_FMT: %s\n", nl_langinfo(_DATE_FMT));
#endif /* _DATE_FMT */
	return 0;
}
