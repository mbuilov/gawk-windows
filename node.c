/*
 * node.c -- routines for node management
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2001, 2003-2007 the Free Software Foundation, Inc.
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

static int is_ieee_magic_val P((const char *val));
static AWKNUM get_ieee_magic_val P((const char *val));

/* r_force_number --- force a value to be numeric */

AWKNUM
r_force_number(register NODE *n)
{
	register char *cp;
	register char *cpend;
	char save;
	char *ptr;
	unsigned int newflags;
	extern double strtod();

#ifdef GAWKDEBUG
	if (n == NULL)
		cant_happen();
	if (n->type != Node_val)
		cant_happen();
	if (n->flags == 0)
		cant_happen();
	if (n->flags & NUMCUR)
		return n->numbr;
#endif

	/* all the conditionals are an attempt to avoid the expensive strtod */

	/* Note: only set NUMCUR if we actually convert some digits */

	n->numbr = 0.0;

	if (n->stlen == 0) {
		if (0 && do_lint)
			lintwarn(_("can't convert string to float"));
		return 0.0;
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
		if (ISALPHA(*cp)) {
			if (0 && do_lint)
				lintwarn(_("can't convert string to float"));
			return 0.0;
		} else if (n->stlen == 4 && is_ieee_magic_val(n->stptr)) {
			if (n->flags & MAYBE_NUM)
				n->flags &= ~MAYBE_NUM;
			n->flags |= NUMBER|NUMCUR;
			n->numbr = get_ieee_magic_val(n->stptr);

			return n->numbr;
		}
		/* else
			fall through */
	}
	/* else not POSIX, so
		fall through */

	cpend = cp + n->stlen;
	while (cp < cpend && ISSPACE(*cp))
		cp++;

	/* FIXME: Simplify this condition! */
	if (   cp == cpend
	    || (! do_posix
	        && (ISALPHA(*cp)
		    || (! do_non_decimal_data && cp[0] == '0'
		        && (cp[1] == 'x' || cp[1] == 'X'))))) {
		if (0 && do_lint)
			lintwarn(_("can't convert string to float"));
		return 0.0;
	}

	if (n->flags & MAYBE_NUM) {
		newflags = NUMBER;
		n->flags &= ~MAYBE_NUM;
	} else
		newflags = 0;

	if (cpend - cp == 1) {
		if (ISDIGIT(*cp)) {
			n->numbr = (AWKNUM)(*cp - '0');
			n->flags |= newflags;
			n->flags |= NUMCUR;
		} else if (0 && do_lint)
			lintwarn(_("can't convert string to float"));
		return n->numbr;
	}

	if (do_non_decimal_data) {	/* main.c assures false if do_posix */
		errno = 0;
		if (! do_traditional && isnondecimal(cp, TRUE)) {
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
	while (ISSPACE(*ptr))
		ptr++;
	*cpend = save;
finish:
	/* the >= should be ==, but for SunOS 3.5 strtod() */
	if (errno == 0 && ptr >= cpend) {
		n->flags |= newflags;
		n->flags |= NUMCUR;
	} else {
		if (0 && do_lint && ptr < cpend)
			lintwarn(_("can't convert string to float"));
		errno = 0;
	}

	return n->numbr;
}


/* format_val --- format a numeric value based on format */

NODE *
format_val(const char *format, int index, register NODE *s)
{
	double val;
	char *orig, *trans, save;

	NODE *dummy, *r;
	unsigned short oflags;
	extern NODE **fmt_list;          /* declared in eval.c */

	if (! do_traditional && (s->flags & INTLSTR) != 0) {
		save = s->stptr[s->stlen];
		s->stptr[s->stlen] = '\0';

		orig = s->stptr;
		trans = dgettext(TEXTDOMAIN, orig);

		s->stptr[s->stlen] = save;
		return tmp_string(trans, strlen(trans));
	}

	/*
	 * 2/2007: Simplify our lives here. Instead of worrying about
	 * whether or not the value will fit into a long just so we
	 * can use sprintf("%ld", val) on it, always format it ourselves.
	 * The only thing to worry about is that integral values always
	 * format as integers. %.0f does that very well.
	 */

	val = double_to_int(s->numbr);

	/* create dummy node for a sole use of format_tree */
	getnode(dummy);
	dummy->type = Node_expression_list;
	dummy->lnode = s;
	dummy->rnode = NULL;
	oflags = s->flags;
	s->flags |= PERM; /* prevent from freeing by format_tree() */
	if (val == s->numbr) {
		r = format_tree("%.0f", 4, dummy, 2);
		s->stfmt = -1;
	} else {
		r = format_tree(format, fmt_list[index]->stlen, dummy, 2);
		s->stfmt = (char) index;
	}
	s->flags = oflags;
	s->stlen = r->stlen;
	if ((s->flags & STRCUR) != 0)
		free(s->stptr);
	s->stptr = r->stptr;
	freenode(r);		/* Do not free_temp(r)!  We want */
	freenode(dummy);	/* to keep s->stptr == r->stpr.  */

	s->stref = 1;
	s->flags |= STRCUR;
	free_wstr(s);
	return s;
}

/* r_force_string --- force a value to be a string */

NODE *
r_force_string(register NODE *s)
{
	NODE *ret;
#ifdef GAWKDEBUG
	if (s == NULL)
		cant_happen();
	if (s->type != Node_val)
		cant_happen();
	if (s->stref <= 0)
		cant_happen();
	if ((s->flags & STRCUR) != 0
	    && (s->stfmt == -1 || s->stfmt == CONVFMTidx))
		return s;
#endif

	ret = format_val(CONVFMT, CONVFMTidx, s);
	return ret;
}

/*
 * dupnode:
 * Duplicate a node.  (For strings, "duplicate" means crank up the
 * reference count.)
 */

NODE *
r_dupnode(NODE *n)
{
	register NODE *r;

#ifndef DUPNODE_MACRO
	if ((n->flags & TEMP) != 0) {
		n->flags &= ~TEMP;
		n->flags |= MALLOC;
		return n;
	}
	if ((n->flags & PERM) != 0)
		return n;
#endif
	if ((n->flags & (MALLOC|STRCUR)) == (MALLOC|STRCUR)) {
		if (n->stref < LONG_MAX)
			n->stref++;
		else
			n->flags |= PERM;
		return n;
	} else if ((n->flags & MALLOC) != 0 && n->type == Node_ahash) {
		if (n->ahname_ref < LONG_MAX)
			n->ahname_ref++;
		else
			n->flags |= PERM;
		return n;
	}
	getnode(r);
	*r = *n;
	r->flags &= ~(PERM|TEMP|FIELD);
	r->flags |= MALLOC;
#ifdef MBS_SUPPORT
	/*
	 * DON'T call free_wstr(r) here!
	 * r->wstptr still points at n->wstptr's value, and we
	 * don't want to free it!
	 */
	r->wstptr = NULL;
	r->wstlen = 0;
#endif /* defined MBS_SUPPORT */
	if (n->type == Node_val && (n->flags & STRCUR) != 0) {
		r->stref = 1;
		emalloc(r->stptr, char *, r->stlen + 2, "dupnode");
		memcpy(r->stptr, n->stptr, r->stlen);
		r->stptr[r->stlen] = '\0';
#if defined MBS_SUPPORT
		if ((n->flags & WSTRCUR) != 0) {
			r->wstlen = n->wstlen;
			emalloc(r->wstptr, wchar_t *, sizeof(wchar_t) * (r->wstlen + 2), "dupnode");
			memcpy(r->wstptr, n->wstptr, r->wstlen * sizeof(wchar_t));
			r->wstptr[r->wstlen] = L'\0';
			r->flags |= WSTRCUR;
		}
#endif /* defined MBS_SUPPORT */
	} else if (n->type == Node_ahash && (n->flags & MALLOC) != 0) {
		r->ahname_ref = 1;
		emalloc(r->ahname_str, char *, r->ahname_len + 2, "dupnode");
		memcpy(r->ahname_str, n->ahname_str, r->ahname_len);
		r->ahname_str[r->ahname_len] = '\0';
	}
	return r;
}

/* copy_node --- force a brand new copy of a node to be allocated */

NODE *
copynode(NODE *old)
{
	NODE *new;
	int saveflags;

	assert(old != NULL);
	saveflags = old->flags;
	old->flags &= ~(MALLOC|PERM);
	new = dupnode(old);
	old->flags = saveflags;
	return new;
}

/* mk_number --- allocate a node with defined number */

NODE *
mk_number(AWKNUM x, unsigned int flags)
{
	register NODE *r;

	getnode(r);
	r->type = Node_val;
	r->numbr = x;
	r->flags = flags;
#ifdef GAWKDEBUG
	r->stref = 1;
	r->stptr = NULL;
	r->stlen = 0;
	free_wstr(r);
#endif /* GAWKDEBUG */
	return r;
}

/* make_str_node --- make a string node */

NODE *
make_str_node(char *s, unsigned long len, int flags)
{
	register NODE *r;

	getnode(r);
	r->type = Node_val;
	r->flags = (STRING|STRCUR|MALLOC);
#ifdef MBS_SUPPORT
	r->wstptr = NULL;
	r->wstlen = 0;
#endif /* defined MBS_SUPPORT */

	if (flags & ALREADY_MALLOCED)
		r->stptr = s;
	else {
		emalloc(r->stptr, char *, len + 2, s);
		memcpy(r->stptr, s, len);
	}
	r->stptr[len] = '\0';
	       
	if ((flags & SCAN) != 0) {	/* scan for escape sequences */
		const char *pf;
		register char *ptm;
		register int c;
		register const char *end;
#ifdef MBS_SUPPORT
		mbstate_t cur_state;

		memset(& cur_state, 0, sizeof(cur_state));
#endif

		end = &(r->stptr[len]);
		for (pf = ptm = r->stptr; pf < end;) {
#ifdef MBS_SUPPORT
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
#endif
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
		r->flags |= PERM;
	}
	r->stlen = len;
	r->stref = 1;
	r->stfmt = -1;

	return r;
}

/* tmp_string --- allocate a temporary string */

NODE *
tmp_string(char *s, size_t len)
{
	register NODE *r;

	r = make_string(s, len);
	r->flags |= TEMP;
	return r;
}

/* more_nodes --- allocate more nodes */

#define NODECHUNK	100

NODE *nextfree = NULL;

NODE *
more_nodes()
{
	register NODE *np;

	/* get more nodes and initialize list */
	emalloc(nextfree, NODE *, NODECHUNK * sizeof(NODE), "more_nodes");
	memset(nextfree, 0, NODECHUNK * sizeof(NODE));
	for (np = nextfree; np <= &nextfree[NODECHUNK - 1]; np++) {
		np->nextp = np + 1;
	}
	--np;
	np->nextp = NULL;
	np = nextfree;
	nextfree = nextfree->nextp;
	return np;
}

#ifdef MEMDEBUG
#undef freenode
/* freenode --- release a node back to the pool */

void
freenode(NODE *it)
{
#ifdef MPROF
	it->stref = 0;
	free((char *) it);
#else	/* not MPROF */
#ifndef NO_PROFILING
	it->exec_count = 0;
#endif
	/* add it to head of freelist */
	it->nextp = nextfree;
	nextfree = it;
#endif	/* not MPROF */
}
#endif	/* GAWKDEBUG */

/* unref --- remove reference to a particular node */

void
unref(register NODE *tmp)
{
	if (tmp == NULL)
		return;
	if ((tmp->flags & PERM) != 0)
		return;
	tmp->flags &= ~TEMP;
	if ((tmp->flags & MALLOC) != 0) {
		if (tmp->type == Node_ahash) {
			if (tmp->ahname_ref > 1) {
				tmp->ahname_ref--;
				return;
			}
			free(tmp->ahname_str);
		} else if ((tmp->flags & STRCUR) != 0) {
			if (tmp->stref > 1) {
				tmp->stref--;
				return;
			}
			free(tmp->stptr);
			free_wstr(tmp);
		}
		freenode(tmp);
		return;
	}
	if ((tmp->flags & FIELD) != 0) {
		free_wstr(tmp);
		freenode(tmp);
		return;
	}
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
 * Posix doesn't allow \x.
 */

int
parse_escape(const char **string_ptr)
{
	register int c = *(*string_ptr)++;
	register int i;
	register int count;

	if (do_lint_old) {
		switch (c) {
		case 'b':
		case 'f':
		case 'r':
			warning(_("old awk does not support the `\\%c' escape sequence"), c);
			break;
		}
	}

	switch (c) {
	case 'a':
		return BELL;
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
			static short warned = FALSE;

			if (! warned) {
				warned = TRUE;
				lintwarn(_("POSIX does not allow `\\x' escapes"));
			}
		}
		if (do_posix)
			return ('x');
		if (! ISXDIGIT((*string_ptr)[0])) {
			warning(_("no hex digits in `\\x' escape sequence"));
			return ('x');
		}
		i = 0;
		for (;;) {
			/* do outside test to avoid multiple side effects */
			c = *(*string_ptr)++;
			if (ISXDIGIT(c)) {
				i *= 16;
				if (ISDIGIT(c))
					i += c - '0';
				else if (ISUPPER(c))
					i += c - 'A' + 10;
				else
					i += c - 'a' + 10;
			} else {
				(*string_ptr)--;
				break;
			}
		}
		return i;
	case '\\':
	case '"':
		return c;
	default:
	{
		static short warned[256];
		unsigned char uc = (unsigned char) c;

		/* N.B.: use unsigned char here to avoid Latin-1 problems */

		if (! warned[uc]) {
			warned[uc] = TRUE;

			warning(_("escape sequence `\\%c' treated as plain `%c'"), uc, uc);
		}
	}
		return c;
	}
}

/* isnondecimal --- return true if number is not a decimal number */

int
isnondecimal(const char *str, int use_locale)
{
	int dec_point = '.';
#if defined(HAVE_LOCALE_H)
	/*
	 * loc.decimal_point may not have been initialized yet,
	 * so double check it before using it.
	 */
	if (use_locale && loc.decimal_point != NULL && loc.decimal_point[0] != '\0')
		dec_point = loc.decimal_point[0];	/* XXX --- assumes one char */
#endif

	if (str[0] != '0')
		return FALSE;

	/* leading 0x or 0X */
	if (str[1] == 'x' || str[1] == 'X')
		return TRUE;

	/*
	 * Numbers with '.', 'e', or 'E' are decimal.
	 * Have to check so that things like 00.34 are handled right.
	 *
	 * These beasts can have trailing whitespace. Deal with that too.
	 */
	for (; *str != '\0'; str++) {
		if (*str == 'e' || *str == 'E' || *str == dec_point)
			return FALSE;
		else if (! ISDIGIT(*str))
			break;
	}

	return TRUE;
}

#if defined MBS_SUPPORT
/* str2wstr --- convert a multibyte string to a wide string */

NODE *
str2wstr(NODE *n, size_t **ptr)
{
	size_t i, count, src_count;
	char *sp;
	mbstate_t mbs;
	wchar_t wc, *wsp;

	assert((n->flags & (STRING|STRCUR)) != 0);

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
	 * each input bytes maps to one wchar_t.  The
	 * advantage is that we only have to convert the string
	 * once, instead of twice, once to find out how many
	 * wide characters, and then again to actually fill
	 * the info in.  If there's a lot left over, we can
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
		count = mbrtowc(& wc, sp, src_count, & mbs);
		switch (count) {
		case (size_t) -2:
		case (size_t) -1:
			goto done;

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

done:
	*wsp = L'\0';
	n->wstlen = i;
	n->flags |= WSTRCUR;
#define ARBITRARY_AMOUNT_TO_GIVE_BACK 100
	if (n->stlen - n->wstlen > ARBITRARY_AMOUNT_TO_GIVE_BACK)
		erealloc(n->wstptr, wchar_t *, sizeof(wchar_t) * (n->wstlen + 2), "str2wstr");

	return n;
}

/* free_wstr --- release the wide string part of a node */

void
free_wstr(NODE *n)
{
	assert(n->type == Node_val);

	if ((n->flags & WSTRCUR) != 0) {
		assert(n->wstptr != NULL);
		free(n->wstptr);
	}
	n->wstptr = NULL;
	n->wstlen = 0;
	n->flags &= ~WSTRCUR;
}

static void
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
#endif /* defined MBS_SUPPORT */

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
	static short first = TRUE;
	static AWKNUM inf;
	static AWKNUM nan;

	char *ptr;
	AWKNUM v = strtod(val, &ptr);

	if (val == ptr) { /* Older strtod implementations don't support inf or nan. */
		if (first) {
			first = FALSE;
			nan = sqrt(-1.0);
			inf = -log(0.0);
		}

		v = ((val[1] == 'i' || val[1] == 'I') ? inf : nan);
		if (val[0] == '-')
			v = -v;
	}

	return v;
}
