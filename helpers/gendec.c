#include <stdio.h>

int main()
{
	int c;
	int count = 0;

	while ((c = getchar()) != EOF) {
		printf("%d ", (c & 0xFF));
		if (++count > 30) {
			putchar('\n');
			count = 0;
		}
	}
	putchar('\n');

	return 0;
}
