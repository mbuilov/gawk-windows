/*
 * strtod.c
 *
 * Stupid version of System V strtod(3) library routine.
 * Does no overflow/underflow checking.
 *
 * A real number is defined to be
 *	optional leading white space
 *	optional sign
 *	string of digits with optional decimal point
 *	optional 'e' or 'E'
 *		followed by optional sign or space
 *		followed by an integer
 *
 * if ptr is not NULL a pointer to the character terminating the
 * scan is returned in *ptr.  If no number formed, *ptr is set to str
 * and 0 is returned.
 *
 * For speed, we don't do the conversion ourselves.  Instead, we find
 * the end of the number and then call atof() to do the dirty work.
 * This bought us a 10% speedup on a sample program at uunet.uu.net.
 *
 * Fall 2000: Changed to enforce C89 semantics, so that 0x... returns 0.
 * C99 has hexadecimal floating point numbers.
 *
 * Summer 2001. Try to make it smarter, so that a string like "0000"
 * doesn't look like we failed. Sigh.
 */

#if 0
#include <ctype.h>
#endif

extern double atof();

double
gawk_strtod(s, ptr)
register const char *s;
register const char **ptr;
{
	double ret = 0.0;
	const char *start = s;		/* save original start of string */
	const char *begin = NULL;	/* where the number really begins */
	int success = 0;

	/* optional white space */
	while (isspace(*s))
		s++;

	/* optional sign */
	if (*s == '+' || *s == '-') {
		s++;
		if (*(s-1) == '-')
			begin = s - 1;
		else
			begin = s;
	}

	/* string of digits with optional decimal point */
	if (isdigit(*s) && ! begin)
		begin = s;

	while (isdigit(*s)) {
		/* don't succeed on 0x... */
		if (*s > '0')
			success++;
		s++;
	}

	if (*s == '.') {
		if (! begin)
			begin = s;
		s++;
		while (isdigit(*s))
			s++;
		success++;
	}

	if (s == start || success == 0)		/* nothing there */
		goto out;

	/*
 	 *	optional 'e' or 'E'
	 *		followed by optional sign or space
	 *		followed by an integer
	 */

	if ((*s == 'e' || *s == 'E')
	    && (isdigit(s[1])
		|| ((s[1] == '-' || s[1] == '+') && isdigit(s[2])))) {
		s++;

		if (*s == '+' || *s == '-')
			s++;

		while (isdigit(*s))
			s++;
	}

	/* go for it */
	ret = atof(begin);

out:
	if (! success && s == begin)
		s = start;	/* in case all we did was skip whitespace */

	if (ptr)
		*ptr = s;

	return ret;
}

#ifdef TEST
int
main(argc, argv)
int argc;
char **argv;
{
	double d;
	char *p;

	for (argc--, argv++; argc; argc--, argv++) {
		d = strtod (*argv, & p);
		printf ("%lf [%s]\n", d, p);
	}

	return 0;
}
#endif
