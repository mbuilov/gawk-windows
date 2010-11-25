/*
 * memset --- initialize memory
 *
 * We supply this routine for those systems that aren't standard yet.
 */

void *
memset(dest, val, l)
void *dest;
int val;
size_t l;
{
	char *ret = dest;
	char *d = dest;

	while (l--)
		*d++ = val;

	return ((void *) ret);
}
