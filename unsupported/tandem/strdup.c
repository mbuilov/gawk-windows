/*
 * strdup --- duplicate a string
 *
 * We supply this routine for those systems that aren't standard yet.
 */

char *
strdup (str)
register const char *str;
{
	char *p;

	p=(char *)malloc(strlen(str)+1);
	return strcpy(p,str);
}
