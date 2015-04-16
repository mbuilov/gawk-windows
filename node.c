/*
 * node.c -- routines for node management
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2001, 2003-2015,
 * the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "awk.h"
#include "math.h"
#include "floatmagic.h"	/* definition of isnan */

static int is_ieee_magic_val(const char *val);
static NODE *r_make_number(double x);
static AWKNUM get_ieee_magic_val(const char *val);
extern NODE **fmt_list;          /* declared in eval.c */

NODE *(*make_number)(double) = r_make_number;
NODE *(*str2number)(NODE *) = r_force_number;
NODE *(*format_val)(const char *, int, NODE *) = r_format_val;
int (*cmp_numbers)(const NODE *, const NODE *) = cmp_awknums;

/* force_number --- force a value to be numeric */

NODE *
r_force_number(NODE *n)
{
	char *cp;
	char *cpend;
	char save;
	char *ptr;
	unsigned int newflags;
	extern double strtod();

	if ((n->flags & NUMCUR) != 0)
		return n;

	/* all the conditionals are an attempt to avoid the expensive strtod */

	/* Note: only set NUMCUR if we actually convert some digits */

	n->numbr = 0.0;

	if (n->stlen == 0) {
		return n;
	}

	cp = n->stptr;
	/*
	 * 2/2007:
	 * POSIX, by way of severe language lawyering, seems to
	 * allow things like "inf" and "nan" to mean something.
	 * So if do_posix, the user gets what he deserves.
	 * This also allows hexadecimal floating point. Ugh.
	 */
	if (! do_posix) {
		if (is_alpha((unsigned char) *cp)) {
			return n;
		} else if (n->stlen == 4 && is_ieee_magic_val(n->stptr)) {
			if ((n->flags & MAYBE_NUM) != 0)
				n->flags &= ~MAYBE_NUM;
			n->flags |= NUMBER|NUMCUR;
			n->numbr = get_ieee_magic_val(n->stptr);

			return n;
		}
		/* else
			fall through */
	}
	/* else not POSIX, so
		fall through */

	cpend = cp + n->stlen;
	while (cp < cpend && isspace((unsigned char) *cp))
		cp++;

	if (   cp == cpend		/* only spaces, or */
	    || (! do_posix		/* not POSIXLY paranoid and */
	        && (is_alpha((unsigned char) *cp)	/* letter, or */
					/* CANNOT do non-decimal and saw 0x */
		    || (! do_non_decimal_data && cp[0] == '0'
		        && (cp[1] == 'x' || cp[1] == 'X'))))) {
		return n;
	}

	if ((n->flags & MAYBE_NUM) != 0) {
		newflags = NUMBER;
		n->flags &= ~MAYBE_NUM;
	} else
		newflags = 0;

	if (cpend - cp == 1) {		/* only one character */
		if (isdigit((unsigned char) *cp)) {	/* it's a digit! */
			n->numbr = (AWKNUM)(*cp - '0');
			n->flags |= newflags;
			n->flags |= NUMCUR;
			if (cp == n->stptr)		/* no leading spaces */
				n->flags |= NUMINT;
		}
		return n;
	}

	if (do_non_decimal_data) {	/* main.c assures false if do_posix */
		errno = 0;
		if (! do_traditional && get_numbase(cp, true) != 10) {
			n->numbr = nondec2awknum(cp, cpend - cp);
			n->flags |= NUMCUR;
			ptr = cpend;
			goto finish;
		}
	}

	errno = 0;
	save = *cpend;
	*cpend = '\0';
	n->numbr = (AWKNUM) strtod((const char *) cp, &ptr);

	/* POSIX says trailing space is OK for NUMBER */
	while (isspace((unsigned char) *ptr))
		ptr++;
	*cpend = save;
finish:
	if (errno == 0) {
		if (ptr == cpend) {
			n->flags |= newflags;
			n->flags |= NUMCUR;
		}
		/* else keep the leading numeric value without updating flags */
	} else {
		errno = 0;
		/*
		 * N.B. For subnormal values, strtod may return the
		 * floating-point representation while setting errno to ERANGE.
		 * We force the numeric value to 0 in such cases.
		 */
		n->numbr = 0;
	}

	return n;
}


/*
 * The following lookup table is used as an optimization in force_string;
 * (more complicated) variations on this theme didn't seem to pay off, but 
 * systematic testing might be in order at some point.
 */
static const char *values[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
};
#define	NVAL	(sizeof(values)/sizeof(values[0]))

/* r_format_val --- format a numeric value based on format */

NODE *
r_format_val(const char *format, int index, NODE *s)
{
	char buf[BUFSIZ];
	char *sp = buf;
	double val;

	/*
	 * 2/2007: Simplify our lives here. Instead of worrying about
	 * whether or not the value will fit into a long just so we
	 * can use sprintf("%ld", val) on it, always format it ourselves.
	 * The only thing to worry about is that integral values always
	 * format as integers. %.0f does that very well.
	 *
	 * 6/2008: Would that things were so simple. Always using %.0f
	 * imposes a notable performance penalty for applications that
	 * do a lot of conversion of integers to strings. So, we reinstate
	 * the old code, but use %.0f for integral values that are outside
	 * the range of a long.  This seems a reasonable compromise.
	 *
	 * 12/2009: Use <= and >= in the comparisons with LONG_xxx instead of
	 * < and > so that things work correctly on systems with 64 bit integers.
	 */

	/* not an integral value, or out of range */
	if ((val = double_to_int(s->numbr)) != s->numbr
			|| val <= LONG_MIN || val >= LONG_MAX
	) {
		/*
		 * Once upon a time, we just blindly did this:
		 *	sprintf(sp, format, s->numbr);
		 *	s->stlen = strlen(sp);
		 *	s->stfmt = (char) index;
		 * but that's no good if, e.g., OFMT is %s. So we punt,
		 * and just always format the value ourselves.
		 */

		NODE *dummy[2], *r;
		unsigned int oflags;

		/* create dummy node for a sole use of format_tree */
		dummy[1] = s;
		oflags = s->flags;

		if (val == s->numbr) {
			/* integral value, but outside range of %ld, use %.0f */
			r = format_tree("%.0f", 4, dummy, 2);
			s->stfmt = -1;
		} else {
			r = format_tree(format, fmt_list[index]->stlen, dummy, 2);
			assert(r != NULL);
			s->stfmt = (char) index;
		}
		s->flags = oflags;
		s->stlen = r->stlen;
		if ((s->flags & STRCUR) != 0)
			efree(s->stptr);
		s->stptr = r->stptr;
		freenode(r);	/* Do not unref(r)! We want to keep s->stptr == r->stpr.  */

		goto no_malloc;
	} else {
		/*
		 * integral value; force conversion to long only once.
		 */
		long num = (long) val;

		if (num < NVAL && num >= 0) {
			sp = (char *) values[num];
			s->stlen = 1;
		} else {
			(void) sprintf(sp, "%ld", num);
			s->stlen = strlen(sp);
		}
		s->stfmt = -1;
		if ((s->flags & INTIND) != 0) {
			s->flags &= ~(INTIND|NUMBER);
			s->flags |= STRING;
		}
	}
	if (s->stptr != NULL)
		efree(s->stptr);
	emalloc(s->stptr, char *, s->stlen + 2, "format_val");
	memcpy(s->stptr, sp, s->stlen + 1);
no_malloc:
	s->flags |= STRCUR;
	free_wstr(s);
	return s;
}

/* r_dupnode --- duplicate a node */

NODE *
r_dupnode(NODE *n)
{
	NODE *r;

	assert(n->type == Node_val);

#ifdef GAWKDEBUG
	if ((n->flags & MALLOC) != 0) {
		n->valref++;
		return n;
	}
#endif

	getnode(r);
	*r = *n;
	r->flags &= ~FIELD;
	r->flags |= MALLOC;
	r->valref = 1;
	/*
	 * DON'T call free_wstr(r) here!
	 * r->wstptr still points at n->wstptr's value, and we
	 * don't want to free it!
	 */
	r->wstptr = NULL;
	r->wstlen = 0;

	if ((n->flags & STRCUR) != 0) {
		emalloc(r->stptr, char *, n->stlen + 2, "r_dupnode");
		memcpy(r->stptr, n->stptr, n->stlen);
		r->stptr[n->stlen] = '\0';
		if ((n->flags & WSTRCUR) != 0) {
			r->wstlen = n->wstlen;
			emalloc(r->wstptr, wchar_t *, sizeof(wchar_t) * (n->wstlen + 2), "r_dupnode");
			memcpy(r->wstptr, n->wstptr, n->wstlen * sizeof(wchar_t));
			r->wstptr[n->wstlen] = L'\0';
			r->flags |= WSTRCUR;
		}
	}
	
	return r;
}

/* r_make_number --- allocate a node with defined number */

static NODE *
r_make_number(double x)
{
	NODE *r;
	getnode(r);
	r->type = Node_val;
	r->numbr = x;
	r->flags = MALLOC|NUMBER|NUMCUR;
	r->valref = 1;
	r->stptr = NULL;
	r->stlen = 0;
	r->wstptr = NULL;
	r->wstlen = 0;
	return r;
}

/* cmp_awknums --- compare two AWKNUMs */

int
cmp_awknums(const NODE *t1, const NODE *t2)
{
	/*
	 * This routine is also used to sort numeric array indices or values.
	 * For the purposes of sorting, NaN is considered greater than
	 * any other value, and all NaN values are considered equivalent and equal.
	 * This isn't in compliance with IEEE standard, but compliance w.r.t. NaN
	 * comparison at the awk level is a different issue, and needs to be dealt
	 * with in the interpreter for each opcode seperately.
	 */

	if (isnan(t1->numbr))
		return ! isnan(t2->numbr);
	if (isnan(t2->numbr))
		return -1;
	/* don't subtract, in case one or both are infinite */
	if (t1->numbr == t2->numbr)
		return 0;
	if (t1->numbr < t2->numbr)
		return -1;
	return 1;
}


/* make_str_node --- make a string node */

NODE *
make_str_node(const char *s, size_t len, int flags)
{
	NODE *r;
	getnode(r);
	r->type = Node_val;
	r->numbr = 0;
	r->flags = (MALLOC|STRING|STRCUR);
	r->valref = 1;
	r->stfmt = -1;
	r->wstptr = NULL;
	r->wstlen = 0;

	if ((flags & ALREADY_MALLOCED) != 0)
		r->stptr = (char *) s;
	else {
		emalloc(r->stptr, char *, len + 2, "make_str_node");
		memcpy(r->stptr, s, len);
	}
	r->stptr[len] = '\0';
       
	if ((flags & SCAN) != 0) {	/* scan for escape sequences */
		const char *pf;
		char *ptm;
		int c;
		const char *end;
		mbstate_t cur_state;

		memset(& cur_state, 0, sizeof(cur_state));

		end = &(r->stptr[len]);
		for (pf = ptm = r->stptr; pf < end;) {
			/*
			 * Keep multibyte characters together. This avoids
			 * problems if a subsequent byte of a multibyte
			 * character happens to be a backslash.
			 */
			if (gawk_mb_cur_max > 1) {
				int mblen = mbrlen(pf, end-pf, &cur_state);

				if (mblen > 1) {
					int i;

					for (i = 0; i < mblen; i++)
						*ptm++ = *pf++;
					continue;
				}
			}

			c = *pf++;
			if (c == '\\') {
				c = parse_escape(&pf);
				if (c < 0) {
					if (do_lint)
						lintwarn(_("backslash at end of string"));
					c = '\\';
				}
				*ptm++ = c;
			} else
				*ptm++ = c;
		}
		len = ptm - r->stptr;
		erealloc(r->stptr, char *, len + 1, "make_str_node");
		r->stptr[len] = '\0';
	}
	r->stlen = len;

	return r;
}


/* unref --- remove reference to a particular node */

void
r_unref(NODE *tmp)
{
#ifdef GAWKDEBUG
	if (tmp == NULL)
		return;
	if ((tmp->flags & MALLOC) != 0) {
		if (tmp->valref > 1) {
			tmp->valref--;
			return;
		}
		if ((tmp->flags & STRCUR) != 0)
			efree(tmp->stptr);
	}
#else
	if ((tmp->flags & (MALLOC|STRCUR)) == (MALLOC|STRCUR))
		efree(tmp->stptr);
#endif

	mpfr_unset(tmp);

	free_wstr(tmp);
	freenode(tmp);
}


/*
 * parse_escape:
 *
 * Parse a C escape sequence.  STRING_PTR points to a variable containing a
 * pointer to the string to parse.  That pointer is updated past the
 * characters we use.  The value of the escape sequence is returned. 
 *
 * A negative value means the sequence \ newline was seen, which is supposed to
 * be equivalent to nothing at all. 
 *
 * If \ is followed by a null character, we return a negative value and leave
 * the string pointer pointing at the null character. 
 *
 * If \ is followed by 000, we return 0 and leave the string pointer after the
 * zeros.  A value of 0 does not mean end of string.  
 *
 * POSIX doesn't allow \x.
 */

int
parse_escape(const char **string_ptr)
{
	int c = *(*string_ptr)++;
	int i;
	int count;
	int j;
	const char *start;

	if (do_lint_old) {
		switch (c) {
		case 'a':
		case 'b':
		case 'f':
		case 'r':
			warning(_("old awk does not support the `\\%c' escape sequence"), c);
			break;
		}
	}

	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\n':
		return -2;
	case 0:
		(*string_ptr)--;
		return -1;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		i = c - '0';
		count = 0;
		while (++count < 3) {
			if ((c = *(*string_ptr)++) >= '0' && c <= '7') {
				i *= 8;
				i += c - '0';
			} else {
				(*string_ptr)--;
				break;
			}
		}
		return i;
	case 'x':
		if (do_lint) {
			static bool warned = false;

			if (! warned) {
				warned = true;
				lintwarn(_("POSIX does not allow `\\x' escapes"));
			}
		}
		if (do_posix)
			return ('x');
		if (! isxdigit((unsigned char) (*string_ptr)[0])) {
			warning(_("no hex digits in `\\x' escape sequence"));
			return ('x');
		}
		i = j = 0;
		start = *string_ptr;
		for (;; j++) {
			/* do outside test to avoid multiple side effects */
			c = *(*string_ptr)++;
			if (isxdigit(c)) {
				i *= 16;
				if (isdigit(c))
					i += c - '0';
				else if (isupper(c))
					i += c - 'A' + 10;
				else
					i += c - 'a' + 10;
			} else {
				(*string_ptr)--;
				break;
			}
		}
		if (do_lint && j > 2)
			lintwarn(_("hex escape \\x%.*s of %d characters probably not interpreted the way you expect"), j, start, j);
		return i;
	case '\\':
	case '"':
		return c;
	default:
	{
		static bool warned[256];
		unsigned char uc = (unsigned char) c;

		/* N.B.: use unsigned char here to avoid Latin-1 problems */

		if (! warned[uc]) {
			warned[uc] = true;

			warning(_("escape sequence `\\%c' treated as plain `%c'"), uc, uc);
		}
	}
		return c;
	}
}

/* get_numbase --- return the base to use for the number in 's' */

int
get_numbase(const char *s, bool use_locale)
{
	int dec_point = '.';
	const char *str = s;

#if defined(HAVE_LOCALE_H)
	/*
	 * loc.decimal_point may not have been initialized yet,
	 * so double check it before using it.
	 */
	if (use_locale && loc.decimal_point != NULL && loc.decimal_point[0] != '\0')
		dec_point = loc.decimal_point[0];	/* XXX --- assumes one char */
#endif

	if (str[0] != '0')
		return 10;

	/* leading 0x or 0X */
	if (str[1] == 'x' || str[1] == 'X')
		return 16;

	/*
	 * Numbers with '.', 'e', or 'E' are decimal.
	 * Have to check so that things like 00.34 are handled right.
	 *
	 * These beasts can have trailing whitespace. Deal with that too.
	 */
	for (; *str != '\0'; str++) {
		if (*str == 'e' || *str == 'E' || *str == dec_point)
			return 10;
		else if (! isdigit((unsigned char) *str))
			break;
	}

	if (! isdigit((unsigned char) s[1])
			|| s[1] == '8' || s[1] == '9'
	)
		return 10;
	return 8;
}

/* str2wstr --- convert a multibyte string to a wide string */

NODE *
str2wstr(NODE *n, size_t **ptr)
{
	size_t i, count, src_count;
	char *sp;
	mbstate_t mbs;
	wchar_t wc, *wsp;
	static bool warned = false;

	assert((n->flags & (STRING|STRCUR)) != 0);

	/*
	 * Don't convert global null string or global null field
	 * variables to a wide string. They are both zero-length anyway.
	 * This also avoids future double-free errors while releasing
	 * shallow copies, eg. *tmp = *Null_field; free_wstr(tmp);
	 */
	if (n == Nnull_string || n == Null_field)
		return n;

	if ((n->flags & WSTRCUR) != 0) {
		if (ptr == NULL)
			return n;
		/* otherwise
			fall through and recompute to fill in the array */
		free_wstr(n);
	}

	/*
	 * After consideration and consultation, this
	 * code trades space for time. We allocate
	 * an array of wchar_t that is n->stlen long.
	 * This is needed in the worst case anyway, where
	 * each input byte maps to one wchar_t.  The
	 * advantage is that we only have to convert the string
	 * once, instead of twice, once to find out how many
	 * wide characters, and then again to actually fill in
	 * the info.  If there's a lot left over, we can
	 * realloc the wide string down in size.
	 */

	emalloc(n->wstptr, wchar_t *, sizeof(wchar_t) * (n->stlen + 2), "str2wstr");
	wsp = n->wstptr;

	/*
	 * For use by do_match, create and fill in an array.
	 * For each byte `i' in n->stptr (the original string),
	 * a[i] is equal to `j', where `j' is the corresponding wchar_t
	 * in the converted wide string.
	 *
	 * Create the array.
	 */
	if (ptr != NULL) {
		emalloc(*ptr, size_t *, sizeof(size_t) * n->stlen, "str2wstr");
		memset(*ptr, 0, sizeof(size_t) * n->stlen);
	}

	sp = n->stptr;
	src_count = n->stlen;
	memset(& mbs, 0, sizeof(mbs));
	for (i = 0; src_count > 0; i++) {
		/*
		 * 9/2010: Check the current byte; if it's a valid character,
		 * then it doesn't start a multibyte sequence. This brings a
		 * big speed up. Thanks to Ulrich Drepper for the tip.
		 * 11/2010: Thanks to Paolo Bonzini for some even faster code.
		 */
		if (is_valid_character(*sp)) {
			count = 1;
			wc = btowc_cache(*sp);
		} else
			count = mbrtowc(& wc, sp, src_count, & mbs);
		switch (count) {
		case (size_t) -2:
		case (size_t) -1:
			/*
			 * Just skip the bad byte and keep going, so that
			 * we get a more-or-less full string, instead of
			 * stopping early. This is particularly important
			 * for match() where we need to build the indices.
			 */
			sp++;
			src_count--;
			/*
			 * mbrtowc(3) says the state of mbs becomes undefined
			 * after a bad character, so reset it.
			 */
			memset(& mbs, 0, sizeof(mbs));
			/* And warn the user something's wrong */
			if (do_lint && ! warned) {
				warned = true;
				lintwarn(_("Invalid multibyte data detected. There may be a mismatch between your data and your locale."));
			}
			break;

		case 0:
			count = 1;
			/* fall through */
		default:
			*wsp++ = wc;
			src_count -= count;
			while (count--)  {
				if (ptr != NULL)
					(*ptr)[sp - n->stptr] = i;
				sp++;
			}
			break;
		}
	}

	*wsp = L'\0';
	n->wstlen = wsp - n->wstptr;
	n->flags |= WSTRCUR;
#define ARBITRARY_AMOUNT_TO_GIVE_BACK 100
	if (n->stlen - n->wstlen > ARBITRARY_AMOUNT_TO_GIVE_BACK)
		erealloc(n->wstptr, wchar_t *, sizeof(wchar_t) * (n->wstlen + 2), "str2wstr");

	return n;
}

/* wstr2str --- convert a wide string back into multibyte one */

NODE *
wstr2str(NODE *n)
{
	size_t result;
	size_t length;
	wchar_t *wp;
	mbstate_t mbs;
	char *newval, *cp;

	assert(n->valref == 1);
	assert((n->flags & WSTRCUR) != 0);

	/*
	 * Convert the wide chars in t1->wstptr back into m.b. chars.
	 * This is pretty grotty, but it's the most straightforward
	 * way to do things.
	 */
	memset(& mbs, 0, sizeof(mbs));

	length = n->wstlen;
	emalloc(newval, char *, (length * gawk_mb_cur_max) + 2, "wstr2str");

	wp = n->wstptr;
	for (cp = newval; length > 0; length--) {
		result = wcrtomb(cp, *wp, & mbs);
		if (result == (size_t) -1)	/* what to do? break seems best */
			break;
		cp += result;
		wp++;
	}
	*cp = '\0';

	efree(n->stptr);
	n->stptr = newval;
	n->stlen = cp - newval;

	return n;
}

/* free_wstr --- release the wide string part of a node */

void
r_free_wstr(NODE *n)
{
	assert(n->type == Node_val);

	if ((n->flags & WSTRCUR) != 0) {
		assert(n->wstptr != NULL);
		efree(n->wstptr);
	}
	n->wstptr = NULL;
	n->wstlen = 0;
	n->flags &= ~WSTRCUR;
}

static void __attribute__ ((unused))
dump_wstr(FILE *fp, const wchar_t *str, size_t len)
{
	if (str == NULL || len == 0)
		return;

	for (; len--; str++)
		putwc(*str, fp);
}

/* wstrstr --- walk haystack, looking for needle, wide char version */

const wchar_t *
wstrstr(const wchar_t *haystack, size_t hs_len,
	const wchar_t *needle, size_t needle_len)
{
	size_t i;

	if (haystack == NULL || needle == NULL || needle_len > hs_len)
		return NULL;

	for (i = 0; i < hs_len; i++) {
		if (haystack[i] == needle[0]
		    && i+needle_len-1 < hs_len
		    && haystack[i+needle_len-1] == needle[needle_len-1]) {
			/* first & last chars match, check string */
			if (memcmp(haystack+i, needle, sizeof(wchar_t) * needle_len) == 0) {
				return haystack + i;
			}
		}
	}

	return NULL;
}

/* wcasestrstr --- walk haystack, nocase look for needle, wide char version */

const wchar_t *
wcasestrstr(const wchar_t *haystack, size_t hs_len,
	const wchar_t *needle, size_t needle_len)
{
	size_t i, j;

	if (haystack == NULL || needle == NULL || needle_len > hs_len)
		return NULL;

	for (i = 0; i < hs_len; i++) {
		if (towlower(haystack[i]) == towlower(needle[0])
		    && i+needle_len-1 < hs_len
		    && towlower(haystack[i+needle_len-1]) == towlower(needle[needle_len-1])) {
			/* first & last chars match, check string */
			const wchar_t *start;

			start = haystack+i;
			for (j = 0; j < needle_len; j++, start++) {
				wchar_t h, n;

				h = towlower(*start);
				n = towlower(needle[j]);
				if (h != n)
					goto out;
			}
			return haystack + i;
		}
out:	;
	}

	return NULL;
}

/* is_ieee_magic_val --- return true for +inf, -inf, +nan, -nan */

static int
is_ieee_magic_val(const char *val)
{
	/*
	 * Avoid strncasecmp: it mishandles ASCII bytes in some locales.
	 * Assume the length is 4, as the caller checks this.
	 */
	return (   (val[0] == '+' || val[0] == '-')
		&& (   (   (val[1] == 'i' || val[1] == 'I')
			&& (val[2] == 'n' || val[2] == 'N')
			&& (val[3] == 'f' || val[3] == 'F'))
		    || (   (val[1] == 'n' || val[1] == 'N')
			&& (val[2] == 'a' || val[2] == 'A')
			&& (val[3] == 'n' || val[3] == 'N'))));
}

/* get_ieee_magic_val --- return magic value for string */

static AWKNUM
get_ieee_magic_val(const char *val)
{
	static bool first = true;
	static AWKNUM inf;
	static AWKNUM nan;

	char *ptr;
	AWKNUM v = strtod(val, &ptr);

	if (val == ptr) { /* Older strtod implementations don't support inf or nan. */
		if (first) {
			first = false;
			nan = sqrt(-1.0);
			inf = -log(0.0);
		}

		v = ((val[1] == 'i' || val[1] == 'I') ? inf : nan);
		if (val[0] == '-')
			v = -v;
	}

	return v;
}

wint_t btowc_cache[256];

/* init_btowc_cache --- initialize the cache */

void init_btowc_cache()
{
	int i;

	for (i = 0; i < 255; i++) {
		btowc_cache[i] = btowc(i);
	}
}

#define BLOCKCHUNK 100

BLOCK nextfree[BLOCK_MAX] = {
	{ 0, NULL},	/* invalid */	
	{ sizeof(NODE), NULL },
	{ sizeof(BUCKET), NULL },
};


/* more_blocks --- get more blocks of memory and add to the free list;
	size of a block must be >= sizeof(BLOCK)
 */

void *
more_blocks(int id)
{
	BLOCK *freep, *np, *next;
	char *p, *endp;
	size_t size;

	size = nextfree[id].size;

	emalloc(freep, BLOCK *, BLOCKCHUNK * size, "more_blocks");
	p = (char *) freep;
	endp = p + BLOCKCHUNK * size;

	for (np = freep; ; np = next) {
		next = (BLOCK *) (p += size);
		if (p >= endp) {
			np->freep = NULL;
			break;
		}
		np->freep = next;
	}
	nextfree[id].freep = freep->freep;
	return freep;
}
