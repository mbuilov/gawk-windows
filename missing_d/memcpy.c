/*
 * memcpy --- copy strings.
 *
 * We supply this routine for those systems that aren't standard yet.
 */

char *
memcpy (dest, src, l)
char *dest, *src;
int l;
{
	char *ret = dest;

	while (l--)
		*dest++ = *src++;

	return ret;
}
