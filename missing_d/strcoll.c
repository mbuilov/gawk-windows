/* replacement strcoll.c */

int
strcoll(const char *s1, const char *s2)
{
	return strcmp(s1, s2);	/* nyah, nyah, so there */
}
