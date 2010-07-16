/*
 * Very simple implementation of strtoul() for gawk,
 * for old systems.  Descriptive prose from the Linux man page.
 *
 * May 2004
 */

/* #define TEST 1 */

#ifdef TEST
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#define TRUE 1
#define FALSE 0
#define strtoul mystrtoul
#endif

#ifndef ULONG_MAX
#define ULONG_MAX (~ 0UL)
#endif

unsigned long int
strtoul(nptr, endptr, base)
const char *nptr;
char **endptr;
int base;
{
	static char lower[] = "abcdefghijklmnopqrstuvwxyz";

	unsigned long result = 0UL;
	char *nptr_orig = (char *) nptr;
	int neg = FALSE;
	char *cp, c;
	int val;
	int sawdigs = FALSE;

	/*
	 * The strtoul() function converts the initial part of the
	 * string in nptr to an unsigned long integer value according
	 * to the given base, which must be between 2 and 36 inclusive,
	 * or be the special value 0.
	 */

	if ((base != 0 && (base < 2 || base > 36)) || nptr == NULL) {
		if (endptr != NULL)
			*endptr = nptr_orig;
		errno = EINVAL;
		return 0;
	}

	/*
	 * The string must [sic] begin with an arbitrary amount of white space
	 * (as determined by isspace(3)) followed by a single optional
	 * `+' or `-' sign.
         */
	while (isspace(*nptr))
		nptr++;

	if (*nptr == '+')
		nptr++;
	else if (*nptr == '-') {
		nptr++;
		neg = TRUE;
	}

       /*
	* If base is zero or 16, the string may then include a `0x' prefix,
	* and the number will be read in base 16; otherwise, a zero base is
	* taken as 10 (decimal) unless the next character is `0', in which
	* case it is taken as 8 (octal).
	*/
       if ((base == 0 || base == 16)
           && nptr[0] == '0'
	   && (nptr[1] == 'x' || nptr[1] == 'X')) {
		base = 16;	/* force it */
		nptr += 2;	/* skip 0x */
	} else if ((base == 0 || base == 8) && nptr[0] == '0') {
		base = 8;
		nptr++;
	} else if (base == 0)
		base = 10;

	/*
	 * The remainder of the string is converted to an unsigned long int
	 * value in the obvious manner, stopping at the first character
	 * which is not a valid digit in the given base. (In bases above 10,
	 * the letter `A' in either upper or lower case represents 10,
	 * `B' represents 11, and so forth, with `Z' representing 35.)
	 */
	for (; *nptr != '\0'; nptr++) {
		c = *nptr;
#if defined(HAVE_LOCALE_H)
		if (base == 10
		    && loc.thousands_sep != NULL
		    && loc.thousands_sep[0] != '\0'
		    && c == loc.thousands_sep[0])
			continue;
#endif
		switch (c) {
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':
			val = c  - '0';
			if (val >= base)  /* even base 2 allowed ... */
				goto out;
			result *= base;
			result += val;
			sawdigs = TRUE;
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
			c += 'a' - 'A';	/* downcase */
			/* fall through */
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y':
		case 'z':
			cp = strchr(lower, c);
			val = cp - lower;
			val += 10;	/* 'a' == 10 */
			if (val >= base)
				goto out;
			result *= base;
			result += val;
			sawdigs = TRUE;
			break;
		default:
			goto out;
		}
	}
out:
	/*
	 * If endptr is not NULL, strtoul() stores the address of the
	 * first invalid character in *endptr. If there were no digits
	 * at all, strtoul() stores the original value of nptr in *endptr
	 * (and returns 0).  In particular, if *nptr is not `\0' but
	 * **endptr is `\0' on return, the entire string is valid.
	 */
	if (endptr != NULL) {
		if (! sawdigs) {
			*endptr = nptr_orig;
			return 0;
		} else
			*endptr = (char *) nptr;
	}

	/*
	 * RETURN VALUE
	 * The strtoul() function returns either the result of the
	 * conversion or, if there was a leading minus sign, the
	 * negation of the result of the conversion, unless the original
	 * (non-negated) value would overflow; in the latter case,
	 * strtoul() returns ULONG_MAX and sets the global variable errno
	 * to ERANGE.
	 */

	/*
	 * ADR: This computation is probably bogus.  If it's a
	 * problem, upgrade to a modern system.
	 */
	if (neg && result == ULONG_MAX) {
		errno = ERANGE;
		return ULONG_MAX;
	} else if (neg)
		result = -result;

	return result;
}

#ifdef TEST
#undef strtoul
int main(void)
{
	char *endptr;
	unsigned long res1, res2;

	res1 = strtoul("0xdeadBeeF", & endptr, 0),
	res2 = mystrtoul("0xdeadBeeF", & endptr, 0),
printf("(real,my)strtoul(\"0xdeadBeeF\", & endptr, 0) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("0101101", & endptr, 2),
	res2 = mystrtoul("0101101", & endptr, 2),
printf("(real,my)strtoul(\"0101101\", & endptr, 2) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("01011012", & endptr, 2),
	res2 = mystrtoul("01011012", & endptr, 2),
printf("(real,my)strtoul(\"01011012\", & endptr, 2) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("  +42a", & endptr, 0),
	res2 = mystrtoul("  +42a", & endptr, 0),
printf("(real,my)strtoul(\"  +42a\", & endptr, 0) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("0377", & endptr, 0),
	res2 = mystrtoul("0377", & endptr, 0),
printf("(real,my)strtoul(\"0377\", & endptr, 0) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("Z", & endptr, 36),
	res2 = mystrtoul("Z", & endptr, 36),
printf("(real,my)strtoul(\"Z\", & endptr, 36) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);

	res1 = strtoul("qZ*", & endptr, 36),
	res2 = mystrtoul("qZ*", & endptr, 36),
printf("(real,my)strtoul(\"qZ*\", & endptr, 36) is %lu, %lu *endptr = %d\n",
		res1, res2, *endptr);
}
#endif
