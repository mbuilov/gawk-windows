/*
 * awkprintf.h -- Formatting code for gawk, used in debug.c and builtin.c.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2010 the Free Software Foundation, Inc.
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

#include "floatmagic.h"

#define DEFAULT_G_PRECISION 6

#ifdef GFMT_WORKAROUND
/* semi-temporary hack, mostly to gracefully handle VMS */
static void sgfmt(char *buf, const char *format, int alt,
		     int fwidth, int precision, double value);
#endif /* GFMT_WORKAROUND */
static size_t mbc_byte_count(const char *ptr, size_t numchars);
static size_t mbc_char_count(const char *ptr, size_t numbytes);

/*
 * r_format_arg() formats arguments of sprintf,
 * and accordingly to a fmt_string providing a format like in
 * printf family from C library.  Returns a string node which value
 * is a formatted string.  Called by  sprintf function.
 *
 * It is one of the uglier parts of gawk.  Thanks to Michal Jaegermann
 * for taming this beast and making it compatible with ANSI C.
 */

r_format_arg(
	const char *fmt_string,
	size_t n0,
	NODE **the_args,
	long num_args)
{
/* copy 'l' bytes from 's' to 'obufout' checking for space in the process */
/* difference of pointers should be of ptrdiff_t type, but let us be kind */
#define bchunk(s, l) if (l) { \
	while ((l) > ofre) { \
		size_t olen = obufout - obuf; \
		erealloc(obuf, char *, osiz * 2, "format_tree"); \
		ofre += osiz; \
		osiz *= 2; \
		obufout = obuf + olen; \
	} \
	memcpy(obufout, s, (size_t) (l)); \
	obufout += (l); \
	ofre -= (l); \
}

/* copy one byte from 's' to 'obufout' checking for space in the process */
#define bchunk_one(s) { \
	if (ofre < 1) { \
		size_t olen = obufout - obuf; \
		erealloc(obuf, char *, osiz * 2, "format_tree"); \
		ofre += osiz; \
		osiz *= 2; \
		obufout = obuf + olen; \
	} \
	*obufout++ = *s; \
	--ofre; \
}

/* Is there space for something L big in the buffer? */
#define chksize(l)  if ((l) >= ofre) { \
	size_t olen = obufout - obuf; \
	size_t delta = osiz+l-ofre; \
	erealloc(obuf, char *, osiz + delta, "format_tree"); \
	obufout = obuf + olen; \
	ofre += delta; \
	osiz += delta; \
}

	size_t cur_arg = 0;
	NODE *r = NULL;
	int i;
	int toofew = FALSE;
	char *obuf, *obufout;
	size_t osiz, ofre;
	const char *chbuf;
	const char *s0, *s1;
	int cs1;
	NODE *arg;
	long fw, prec, argnum;
	int used_dollar;
	int lj, alt, big, bigbig, small, have_prec, need_format;
	long *cur = NULL;
	uintmax_t uval;
	int sgn;
	int base = 0;
	/*
	 * Although this is an array, the elements serve two different
	 * purposes. The first element is the general buffer meant
	 * to hold the entire result string.  The second one is a
	 * temporary buffer for large floating point values. They
	 * could just as easily be separate variables, and the
	 * code might arguably be clearer.
	 */
	struct {
		char *buf;
		size_t bufsize;
		char stackbuf[30];
	} cpbufs[2];
#define cpbuf	cpbufs[0].buf
	char *cend = &cpbufs[0].stackbuf[sizeof(cpbufs[0].stackbuf)];
	char *cp;
	const char *fill;
	AWKNUM tmpval;
	char signchar = FALSE;
	size_t len;
	int zero_flag = FALSE;
	int quote_flag = FALSE;
	int ii, jj;
	char *chp;
	size_t copy_count, char_count;
	static const char sp[] = " ";
	static const char zero_string[] = "0";
	static const char lchbuf[] = "0123456789abcdef";
	static const char Uchbuf[] = "0123456789ABCDEF";

#define INITIAL_OUT_SIZE	512
	emalloc(obuf, char *, INITIAL_OUT_SIZE, "format_tree");
	obufout = obuf;
	osiz = INITIAL_OUT_SIZE;
	ofre = osiz - 2;

	cur_arg = 1;

	{
		size_t k;
		for (k = 0; k < sizeof(cpbufs)/sizeof(cpbufs[0]); k++) {
			cpbufs[k].bufsize = sizeof(cpbufs[k].stackbuf);
			cpbufs[k].buf = cpbufs[k].stackbuf;
		}
	}

	/*
	 * The point of this goop is to grow the buffer
	 * holding the converted number, so that large
	 * values don't overflow a fixed length buffer.
	 */
#define PREPEND(CH) do {	\
	if (cp == cpbufs[0].buf) {	\
		char *prev = cpbufs[0].buf;	\
		emalloc(cpbufs[0].buf, char *, 2*cpbufs[0].bufsize, \
		 	"format_tree");	\
		memcpy((cp = cpbufs[0].buf+cpbufs[0].bufsize), prev,	\
		       cpbufs[0].bufsize);	\
		cpbufs[0].bufsize *= 2;	\
		if (prev != cpbufs[0].stackbuf)	\
			efree(prev);	\
		cend = cpbufs[0].buf+cpbufs[0].bufsize;	\
	}	\
	*--cp = (CH);	\
} while(0)

	/*
	 * Check first for use of `count$'.
	 * If plain argument retrieval was used earlier, choke.
	 *	Otherwise, return the requested argument.
	 * If not `count$' now, but it was used earlier, choke.
	 * If this format is more than total number of args, choke.
	 * Otherwise, return the current argument.
	 */
#define parse_next_arg() { \
	if (argnum > 0) { \
		if (cur_arg > 1) { \
			fmt_msg(_("fatal: must use `count$' on all formats or none")); \
			goto out; \
		} \
		arg = the_args[argnum]; \
	} else if (used_dollar) { \
		fmt_msg(_("fatal: must use `count$' on all formats or none")); \
		arg = 0; /* shutup the compiler */ \
		goto out; \
	} else if (cur_arg >= num_args) { \
		arg = 0; /* shutup the compiler */ \
		toofew = TRUE; \
		break; \
	} else { \
		arg = the_args[cur_arg]; \
		cur_arg++; \
	} \
}

	need_format = FALSE;
	used_dollar = FALSE;

	s0 = s1 = fmt_string;
	while (n0-- > 0) {
		if (*s1 != '%') {
			s1++;
			continue;
		}
		need_format = TRUE;
		bchunk(s0, s1 - s0);
		s0 = s1;
		cur = &fw;
		fw = 0;
		prec = 0;
		argnum = 0;
		have_prec = FALSE;
		signchar = FALSE;
		zero_flag = FALSE;
		quote_flag = FALSE;
		lj = alt = big = bigbig = small = FALSE;
		fill = sp;
		cp = cend;
		chbuf = lchbuf;
		s1++;

retry:
		if (n0-- == 0)	/* ran out early! */
			break;

		switch (cs1 = *s1++) {
		case (-1):	/* dummy case to allow for checking */
check_pos:
			if (cur != &fw)
				break;		/* reject as a valid format */
			goto retry;
		case '%':
			need_format = FALSE;
			/*
			 * 29 Oct. 2002:
			 * The C99 standard pages 274 and 279 seem to imply that
			 * since there's no arg converted, the field width doesn't
			 * apply.  The code already was that way, but this
			 * comment documents it, at least in the code.
			 */
			if (do_lint) {
				const char *msg = NULL;

				if (fw && ! have_prec)
					msg = _("field width is ignored for `%%' specifier");
				else if (fw == 0 && have_prec)
					msg = _("precision is ignored for `%%' specifier");
				else if (fw && have_prec)
					msg = _("field width and precision are ignored for `%%' specifier");

				if (msg != NULL)
					lintwarn("%s", msg);
			}
			bchunk_one("%");
			s0 = s1;
			break;

		case '0':
			/*
			 * Only turn on zero_flag if we haven't seen
			 * the field width or precision yet.  Otherwise,
			 * screws up floating point formatting.
			 */
			if (cur == & fw)
				zero_flag = TRUE;
			if (lj)
				goto retry;
			/* FALL through */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (cur == NULL)
				break;
			if (prec >= 0)
				*cur = cs1 - '0';
			/*
			 * with a negative precision *cur is already set
			 * to -1, so it will remain negative, but we have
			 * to "eat" precision digits in any case
			 */
			while (n0 > 0 && *s1 >= '0' && *s1 <= '9') {
				--n0;
				*cur = *cur * 10 + *s1++ - '0';
			}
			if (prec < 0) 	/* negative precision is discarded */
				have_prec = FALSE;
			if (cur == &prec)
				cur = NULL;
			if (n0 == 0)	/* badly formatted control string */
				continue;
			goto retry;
		case '$':
			if (do_traditional) {
				fmt_msg(_("fatal: `$' is not permitted in awk formats"));
				goto out;
			}

			if (cur == &fw) {
				argnum = fw;
				fw = 0;
				used_dollar = TRUE;
				if (argnum <= 0) {
					fmt_msg(_("fatal: arg count with `$' must be > 0"));
					goto out;
				}
				if (argnum >= num_args) {
					fmt_msg(_("fatal: arg count %ld greater than total number of supplied arguments"), argnum);
					goto out;
				}
			} else {
				fmt_msg(_("fatal: `$' not permitted after period in format"));
				goto out;
			}

			goto retry;
		case '*':
			if (cur == NULL)
				break;
			if (! do_traditional && isdigit(*s1)) {
				int val = 0;

				for (; n0 > 0 && *s1 && isdigit(*s1); s1++, n0--) {
					val *= 10;
					val += *s1 - '0';
				}
				if (*s1 != '$') {
					fmt_msg(_("fatal: no `$' supplied for positional field width or precision"));
					goto out;
				} else {
					s1++;
					n0--;
				}
				if (val >= num_args) {
					toofew = TRUE;
					break;
				}
				arg = the_args[val];
			} else {
				parse_next_arg();
			}
			*cur = force_number(arg);
			if (*cur < 0 && cur == &fw) {
				*cur = -*cur;
				lj++;
			}
			if (cur == &prec) {
				if (*cur >= 0)
					have_prec = TRUE;
				else
					have_prec = FALSE;
				cur = NULL;
			}
			goto retry;
		case ' ':		/* print ' ' or '-' */
					/* 'space' flag is ignored */
					/* if '+' already present  */
			if (signchar != FALSE) 
				goto check_pos;
			/* FALL THROUGH */
		case '+':		/* print '+' or '-' */
			signchar = cs1;
			goto check_pos;
		case '-':
			if (prec < 0)
				break;
			if (cur == &prec) {
				prec = -1;
				goto retry;
			}
			fill = sp;      /* if left justified then other */
			lj++; 		/* filling is ignored */
			goto check_pos;
		case '.':
			if (cur != &fw)
				break;
			cur = &prec;
			have_prec = TRUE;
			goto retry;
		case '#':
			alt = TRUE;
			goto check_pos;
		case '\'':
#if defined(HAVE_LOCALE_H)       
			/* allow quote_flag if there is a thousands separator. */
			if (loc.thousands_sep[0] != '\0')
				quote_flag = TRUE;
			goto check_pos;
#else
			goto retry;  
#endif
		case 'l':
			if (big)
				break;
			else {
				static short warned = FALSE;
				
				if (do_lint && ! warned) {
					lintwarn(_("`l' is meaningless in awk formats; ignored"));
					warned = TRUE;
				}
				if (do_posix) {
					fmt_msg(_("fatal: `l' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			big = TRUE;
			goto retry;
		case 'L':
			if (bigbig)
				break;
			else {
				static short warned = FALSE;
				
				if (do_lint && ! warned) {
					lintwarn(_("`L' is meaningless in awk formats; ignored"));
					warned = TRUE;
				}
				if (do_posix) {
					fmt_msg(_("fatal: `L' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			bigbig = TRUE;
			goto retry;
		case 'h':
			if (small)
				break;
			else {
				static short warned = FALSE;
				
				if (do_lint && ! warned) {
					lintwarn(_("`h' is meaningless in awk formats; ignored"));
					warned = TRUE;
				}
				if (do_posix) {
					fmt_msg(_("fatal: `h' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			small = TRUE;
			goto retry;
		case 'c':
			need_format = FALSE;
			parse_next_arg();
			/* user input that looks numeric is numeric */
			if ((arg->flags & (MAYBE_NUM|NUMBER)) == MAYBE_NUM)
				(void) force_number(arg);
			if (arg->flags & NUMBER) {
				uval = (uintmax_t) arg->numbr;
#if MBS_SUPPORT
				if (gawk_mb_cur_max > 1) {
					char buf[100];
					wchar_t wc;
					mbstate_t mbs;
					size_t count;

					memset(& mbs, 0, sizeof(mbs));
					wc = uval;

					count = wcrtomb(buf, wc, & mbs);
					if (count == 0
					    || count == (size_t)-1
					    || count == (size_t)-2)
						goto out0;

					memcpy(cpbuf, buf, count);
					prec = count;
					cp = cpbuf;
					goto pr_tail;
				}
out0:
				;
				/* else,
					fall through */
#endif
				if (do_lint && uval > 255) {
					lintwarn("[s]printf: value %g is too big for %%c format",
							arg->numbr);
				}
				cpbuf[0] = uval;
				prec = 1;
				cp = cpbuf;
				goto pr_tail;
			}
			/*
			 * As per POSIX, only output first character of a
			 * string value.  Thus, we ignore any provided
			 * precision, forcing it to 1.  (Didn't this
			 * used to work? 6/2003.)
			 */
			cp = arg->stptr;
#ifdef MBS_SUPPORT
			/*
			 * First character can be multiple bytes if
			 * it's a multibyte character. Grr.
			 */
			if (gawk_mb_cur_max > 1) {
				mbstate_t state;
				size_t count;

				memset(& state, 0, sizeof(state));
				count = mbrlen(cp, arg->stlen, & state);
				if (count == 0
				    || count == (size_t)-1
				    || count == (size_t)-2)
					goto out2;
				prec = count;
				goto pr_tail;
			}
out2:
			;
#endif
			prec = 1;
			goto pr_tail;
		case 's':
			need_format = FALSE;
			parse_next_arg();
			arg = force_string(arg);
			if (fw == 0 && ! have_prec)
				prec = arg->stlen;
			else {
				char_count = mbc_char_count(arg->stptr, arg->stlen);
				if (! have_prec || prec > char_count)
					prec = char_count;
			}
			cp = arg->stptr;
			goto pr_tail;
		case 'd':
		case 'i':
			need_format = FALSE;
			parse_next_arg();
			tmpval = force_number(arg);
			/*
			 * Check for Nan or Inf.
			 */
			if (isnan(tmpval) || isinf(tmpval))
				goto out_of_range;
			else
				tmpval = double_to_int(tmpval);

			/*
			 * ``The result of converting a zero value with a
			 * precision of zero is no characters.''
			 */
			if (have_prec && prec == 0 && tmpval == 0)
				goto pr_tail;

			if (tmpval < 0) {
				tmpval = -tmpval;
				sgn = TRUE;
			} else {
				if (tmpval == -0.0)
					/* avoid printing -0 */
					tmpval = 0.0;
				sgn = FALSE;
			}
			/*
			 * Use snprintf return value to tell if there
			 * is enough room in the buffer or not.
			 */
			while ((i = snprintf(cpbufs[1].buf,
					     cpbufs[1].bufsize, "%.0f",
					     tmpval)) >=
			       cpbufs[1].bufsize) {
				if (cpbufs[1].buf == cpbufs[1].stackbuf)
					cpbufs[1].buf = NULL;
				if (i > 0) {
					cpbufs[1].bufsize += ((i > cpbufs[1].bufsize) ?
							      i : cpbufs[1].bufsize);
				}
				else
					cpbufs[1].bufsize *= 2;
				assert(cpbufs[1].bufsize > 0);
				erealloc(cpbufs[1].buf, char *,
					 cpbufs[1].bufsize, "format_tree");
			}
			if (i < 1)
				goto out_of_range;
			chp = &cpbufs[1].buf[i-1];
			ii = jj = 0;
			do {
				PREPEND(*chp);
				chp--; i--;
#if defined(HAVE_LOCALE_H)
				if (quote_flag && loc.grouping[ii] && ++jj == loc.grouping[ii]) {
					if (i)	/* only add if more digits coming */
						PREPEND(loc.thousands_sep[0]);	/* XXX - assumption it's one char */
					if (loc.grouping[ii+1] == 0)
						jj = 0;		/* keep using current val in loc.grouping[ii] */
					else if (loc.grouping[ii+1] == CHAR_MAX)
						quote_flag = FALSE;
					else {                 
						ii++;
						jj = 0;
					}
				}
#endif
			} while (i > 0);

			/* add more output digits to match the precision */
			if (have_prec) {
				while (cend - cp < prec)
					PREPEND('0');
			}

			if (sgn)
				PREPEND('-');
			else if (signchar)
				PREPEND(signchar);
			/*
			 * When to fill with zeroes is of course not simple.
			 * First: No zero fill if left-justifying.
			 * Next: There seem to be two cases:
			 * 	A '0' without a precision, e.g. %06d
			 * 	A precision with no field width, e.g. %.10d
			 * Any other case, we don't want to fill with zeroes.
			 */
			if (! lj
			    && ((zero_flag && ! have_prec)
				 || (fw == 0 && have_prec)))
				fill = zero_string;
			if (prec > fw)
				fw = prec;
			prec = cend - cp;
			if (fw > prec && ! lj && fill != sp
			    && (*cp == '-' || signchar)) {
				bchunk_one(cp);
				cp++;
				prec--;
				fw--;
			}
			goto pr_tail;
		case 'X':
			chbuf = Uchbuf;	/* FALL THROUGH */
		case 'x':
			base += 6;	/* FALL THROUGH */
		case 'u':
			base += 2;	/* FALL THROUGH */
		case 'o':
			base += 8;
			need_format = FALSE;
			parse_next_arg();
			tmpval = force_number(arg);

			/*
			 * ``The result of converting a zero value with a
			 * precision of zero is no characters.''
			 *
			 * If I remember the ANSI C standard, though,
			 * it says that for octal conversions
			 * the precision is artificially increased
			 * to add an extra 0 if # is supplied.
			 * Indeed, in C,
			 * 	printf("%#.0o\n", 0);
			 * prints a single 0.
			 */
			if (! alt && have_prec && prec == 0 && tmpval == 0)
				goto pr_tail;

			if (tmpval < 0) {
				uval = (uintmax_t) (intmax_t) tmpval;
				if ((AWKNUM)(intmax_t)uval !=
				    double_to_int(tmpval))
					goto out_of_range;
			} else {
				uval = (uintmax_t) tmpval;
				if ((AWKNUM)uval != double_to_int(tmpval))
					goto out_of_range;
			}
			/*
			 * When to fill with zeroes is of course not simple.
			 * First: No zero fill if left-justifying.
			 * Next: There seem to be two cases:
			 * 	A '0' without a precision, e.g. %06d
			 * 	A precision with no field width, e.g. %.10d
			 * Any other case, we don't want to fill with zeroes.
			 */
			if (! lj
			    && ((zero_flag && ! have_prec)
				 || (fw == 0 && have_prec)))
				fill = zero_string;
			ii = jj = 0;
			do {
				PREPEND(chbuf[uval % base]);
				uval /= base;
#if defined(HAVE_LOCALE_H)
				if (base == 10 && quote_flag && loc.grouping[ii] && ++jj == loc.grouping[ii]) {
					if (uval)	/* only add if more digits coming */
						PREPEND(loc.thousands_sep[0]);	/* XXX --- assumption it's one char */
					if (loc.grouping[ii+1] == 0)                                          
						jj = 0;     /* keep using current val in loc.grouping[ii] */
					else if (loc.grouping[ii+1] == CHAR_MAX)                        
						quote_flag = FALSE;
					else {                 
						ii++;
						jj = 0;
					}
				}
#endif
			} while (uval > 0);

			/* add more output digits to match the precision */
			if (have_prec) {
				while (cend - cp < prec)
					PREPEND('0');
			}

			if (alt && tmpval != 0) {
				if (base == 16) {
					PREPEND(cs1);
					PREPEND('0');
					if (fill != sp) {
						bchunk(cp, 2);
						cp += 2;
						fw -= 2;
					}
				} else if (base == 8)
					PREPEND('0');
			}
			base = 0;
			if (prec > fw)
				fw = prec;
			prec = cend - cp;
	pr_tail:
			if (! lj) {
				while (fw > prec) {
			    		bchunk_one(fill);
					fw--;
				}
			}
			copy_count = prec;
			if (fw == 0 && ! have_prec)
				;
			else if (gawk_mb_cur_max > 1 && (cs1 == 's' || cs1 == 'c')) {
				assert(cp == arg->stptr || cp == cpbuf);
				copy_count = mbc_byte_count(arg->stptr, prec);
			}
			bchunk(cp, copy_count);
			while (fw > prec) {
				bchunk_one(fill);
				fw--;
			}
			s0 = s1;
			break;

     out_of_range:
			/* out of range - emergency use of %g format */
			if (do_lint)
				lintwarn(_("[s]printf: value %g is out of range for `%%%c' format"),
							(double) tmpval, cs1);
			cs1 = 'g';
			goto format_float;

		case 'F':
#if ! defined(PRINTF_HAS_F_FORMAT) || PRINTF_HAS_F_FORMAT != 1
			cs1 = 'f';
			/* FALL THROUGH */
#endif
		case 'g':
		case 'G':
		case 'e':
		case 'f':
		case 'E':
			need_format = FALSE;
			parse_next_arg();
			tmpval = force_number(arg);
     format_float:
			if (! have_prec)
				prec = DEFAULT_G_PRECISION;
			chksize(fw + prec + 9);	/* 9 == slop */
#ifdef VAXCRTL
			/* pre-ANSI library doesn't handle '0' flag
			   correctly in many cases; reject it */
			if (zero_flag
			    && (lj || (signchar && signchar != '+')))
				zero_flag = FALSE;
#endif
			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (signchar)
				*cp++ = signchar;
			if (alt)
				*cp++ = '#';
			if (zero_flag)
				*cp++ = '0';
			if (quote_flag)
				*cp++ = '\'';
			strcpy(cp, "*.*");
			cp += 3;
			*cp++ = cs1;
			*cp = '\0';
#ifndef GFMT_WORKAROUND
#if defined(LC_NUMERIC)
			if (quote_flag && ! use_lc_numeric)
				setlocale(LC_NUMERIC, "");
#endif
			{
				int n;
				while ((n = snprintf(obufout, ofre, cpbuf,
						     (int) fw, (int) prec,
						     (double) tmpval)) >= ofre)
					chksize(n)
			}
#if defined(LC_NUMERIC)
			if (quote_flag && ! use_lc_numeric)
				setlocale(LC_NUMERIC, "C");
#endif
#else	/* GFMT_WORKAROUND */
			if (cs1 == 'g' || cs1 == 'G')
				sgfmt(obufout, cpbuf, (int) alt,
				       (int) fw, (int) prec, (double) tmpval);
			else {
				int n;
				while ((n = snprintf(obufout, ofre, cpbuf,
						     (int) fw, (int) prec,
						     (double) tmpval)) >= ofre)
					chksize(n)
			}
#endif	/* GFMT_WORKAROUND */
			len = strlen(obufout);
			ofre -= len;
			obufout += len;
			s0 = s1;
			break;
		default:
			if (do_lint && isalpha(cs1))
				lintwarn(_("ignoring unknown format specifier character `%c': no argument converted"), cs1);
			break;
		}
		if (toofew) {
			fmt_msg("%s\n\t`%s'\n\t%*s%s",
			      _("fatal: not enough arguments to satisfy format string"),
			      fmt_string, (int) (s1 - fmt_string - 1), "",
			      _("^ ran out for this one"));
			goto out;
		}
	}
	if (do_lint) {
		if (need_format)
			lintwarn(
			_("[s]printf: format specifier does not have control letter"));
		if (cur_arg < num_args)
			lintwarn(
			_("too many arguments supplied for format string"));
	}
	bchunk(s0, s1 - s0);
	r = make_str_node(obuf, obufout - obuf, ALREADY_MALLOCED);
	obuf = NULL;
out:
	{
		size_t k;
		size_t count = sizeof(cpbufs)/sizeof(cpbufs[0]);
		for (k = 0; k < count; k++) {
			if (cpbufs[k].buf != cpbufs[k].stackbuf)
				efree(cpbufs[k].buf);
		}
		if (obuf != NULL)
			efree(obuf);
	}
	return r;
}


#ifdef GFMT_WORKAROUND
/*
 * printf's %g format [can't rely on gcvt()]
 *	caveat: don't use as argument to *printf()!
 * 'format' string HAS to be of "<flags>*.*g" kind, or we bomb!
 */
static void
sgfmt(char *buf,	/* return buffer; assumed big enough to hold result */
	const char *format,
	int alt,	/* use alternate form flag */
	int fwidth,	/* field width in a format */
	int prec,	/* indicates desired significant digits, not decimal places */
	double g)	/* value to format */
{
	char dform[40];
	char *gpos;
	char *d, *e, *p;
	int again = FALSE;

	strncpy(dform, format, sizeof dform - 1);
	dform[sizeof dform - 1] = '\0';
	gpos = strrchr(dform, '.');

	if (g == 0.0 && ! alt) {	/* easy special case */
		*gpos++ = 'd';
		*gpos = '\0';
		(void) sprintf(buf, dform, fwidth, 0);
		return;
	}

	/* advance to location of 'g' in the format */
	while (*gpos && *gpos != 'g' && *gpos != 'G')
		gpos++;

	if (prec <= 0)	      /* negative precision is ignored */
		prec = (prec < 0 ?  DEFAULT_G_PRECISION : 1);

	if (*gpos == 'G')
		again = TRUE;
	/* start with 'e' format (it'll provide nice exponent) */
	*gpos = 'e';
	prec--;
	(void) sprintf(buf, dform, fwidth, prec, g);
	if ((e = strrchr(buf, 'e')) != NULL) {	/* find exponent  */
		int expn = atoi(e+1);		/* fetch exponent */
		if (expn >= -4 && expn <= prec) {	/* per K&R2, B1.2 */
			/* switch to 'f' format and re-do */
			*gpos = 'f';
			prec -= expn;		/* decimal precision */
			(void) sprintf(buf, dform, fwidth, prec, g);
			e = buf + strlen(buf);
			while (*--e == ' ')
				continue;
			e++;
		}
		else if (again)
			*gpos = 'E';

		/* if 'alt' in force, then trailing zeros are not removed */
		if (! alt && (d = strrchr(buf, '.')) != NULL) {
			/* throw away an excess of precision */
			for (p = e; p > d && *--p == '0'; )
				prec--;
			if (d == p)
				prec--;
			if (prec < 0)
				prec = 0;
			/* and do that once again */
			again = TRUE;
		}
		if (again)
			(void) sprintf(buf, dform, fwidth, prec, g);
	}
}
#endif	/* GFMT_WORKAROUND */


/* mbc_byte_count --- return number of bytes for corresponding numchars multibyte characters */

static size_t
mbc_byte_count(const char *ptr, size_t numchars)
{
#ifdef MBS_SUPPORT
	mbstate_t cur_state;
	size_t sum = 0;
	int mb_len;

	memset(& cur_state, 0, sizeof(cur_state));

	assert(gawk_mb_cur_max > 1);
	mb_len = mbrlen(ptr, numchars * gawk_mb_cur_max, &cur_state);
	if (mb_len <= 0)
		return numchars;	/* no valid m.b. char */

	for (; numchars > 0; numchars--) {
		mb_len = mbrlen(ptr, numchars * gawk_mb_cur_max, &cur_state);
		if (mb_len <= 0)
			break;
		sum += mb_len;
		ptr += mb_len;
	}

	return sum;
#else
	return numchars;
#endif
}

/* mbc_char_count --- return number of m.b. chars in string, up to numbytes bytes */

static size_t
mbc_char_count(const char *ptr, size_t numbytes)
{
#ifdef MBS_SUPPORT
	mbstate_t cur_state;
	size_t sum = 0;
	int mb_len;

	if (gawk_mb_cur_max == 1)
		return numbytes;

	memset(& cur_state, 0, sizeof(cur_state));

	mb_len = mbrlen(ptr, numbytes * gawk_mb_cur_max, &cur_state);
	if (mb_len <= 0)
		return numbytes;	/* no valid m.b. char */

	for (; numbytes > 0; numbytes--) {
		mb_len = mbrlen(ptr, numbytes * gawk_mb_cur_max, &cur_state);
		if (mb_len <= 0)
			break;
		sum++;
		ptr += mb_len;
	}

	return sum;
#else
	return numbytes;
#endif
}
