/*
 * builtin.c - Builtin functions and various utility procedures.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2015 the Free Software Foundation, Inc.
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
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif
#include <math.h>
#include "random.h"
#include "floatmagic.h"

#if defined(HAVE_POPEN_H)
#include "popen.h"
#endif

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

/* The extra casts work around common compiler bugs.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
/* The outer cast is needed to work around a bug in Cray C 5.0.3.0.
   It is necessary at least when t == time_t.  */
#define TYPE_MINIMUM(t) ((t) (TYPE_SIGNED (t) \
			      ? ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1) : (t) 0))
#define TYPE_MAXIMUM(t) ((t) (~ (t) 0 - TYPE_MINIMUM (t)))

#ifndef INTMAX_MIN
# define INTMAX_MIN TYPE_MINIMUM (intmax_t)
#endif
#ifndef UINTMAX_MAX
# define UINTMAX_MAX TYPE_MAXIMUM (uintmax_t)
#endif

#ifndef SIZE_MAX	/* C99 constant, can't rely on it everywhere */
#define SIZE_MAX ((size_t) -1)
#endif

#define DEFAULT_G_PRECISION 6

static size_t mbc_byte_count(const char *ptr, size_t numchars);
static size_t mbc_char_count(const char *ptr, size_t numbytes);

/* Can declare these, since we always use the random shipped with gawk */
extern char *initstate(unsigned long seed, char *state, long n);
extern char *setstate(char *state);
extern long random(void);
extern void srandom(unsigned long seed);

extern NODE **args_array;
extern int max_args;
extern NODE **fields_arr;
extern bool output_is_tty;
extern FILE *output_fp;


#define POP_TWO_SCALARS(s1, s2) \
s2 = POP_SCALAR(); \
s1 = POP(); \
do { if (s1->type == Node_var_array) { \
DEREF(s2); \
fatal(_("attempt to use array `%s' in a scalar context"), array_vname(s1)); \
}} while (false)


/*
 * Since we supply the version of random(), we know what
 * value to use here.
 */
#define GAWK_RANDOM_MAX 0x7fffffffL

/* efwrite --- like fwrite, but with error checking */

static void
efwrite(const void *ptr,
	size_t size,
	size_t count,
	FILE *fp,
	const char *from,
	struct redirect *rp,
	bool flush)
{
	errno = 0;
	if (rp != NULL) {
		if (rp->output.gawk_fwrite(ptr, size, count, fp, rp->output.opaque) != count)
			goto wrerror;
	} else if (fwrite(ptr, size, count, fp) != count)
		goto wrerror;
	if (flush
	  && ((fp == stdout && output_is_tty)
	      || (rp != NULL && (rp->flag & RED_NOBUF) != 0))) {
		if (rp != NULL) {
			rp->output.gawk_fflush(fp, rp->output.opaque);
			if (rp->output.gawk_ferror(fp, rp->output.opaque))
				goto wrerror;
		} else {
			fflush(fp);
			if (ferror(fp))
				goto wrerror;
		}
	}
	return;

wrerror:
	/* die silently on EPIPE to stdout */
	if (fp == stdout && errno == EPIPE)
		gawk_exit(EXIT_FATAL);

	/* otherwise die verbosely */
	fatal(_("%s to \"%s\" failed (%s)"), from,
		rp ? rp->value : _("standard output"),
		errno ? strerror(errno) : _("reason unknown"));
}

/* do_exp --- exponential function */

NODE *
do_exp(int nargs)
{
	NODE *tmp;
	double d, res;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("exp: received non-numeric argument"));
	d = force_number(tmp)->numbr;
	DEREF(tmp);
	errno = 0;
	res = exp(d);
	if (errno == ERANGE)
		warning(_("exp: argument %g is out of range"), d);
	return make_number((AWKNUM) res);
}

/* stdfile --- return fp for a standard file */

/*
 * This function allows `fflush("/dev/stdout")' to work.
 * The other files will be available via getredirect().
 * /dev/stdin is not included, since fflush is only for output.
 */

static FILE *
stdfile(const char *name, size_t len)
{
	if (len == 11) {
		if (strncmp(name, "/dev/stderr", 11) == 0)
			return stderr;
		else if (strncmp(name, "/dev/stdout", 11) == 0)
			return stdout;
	}

	return NULL;
}

/* do_fflush --- flush output, either named file or pipe or everything */

NODE *
do_fflush(int nargs)
{
	struct redirect *rp;
	NODE *tmp;
	FILE *fp;
	int status = 0;
	const char *file;

	/*
	 * November, 2012.
	 * It turns out that circa 2002, when BWK 
	 * added fflush() and fflush("") to his awk, he made both of
	 * them flush everything.  
	 *
	 * Now, with our inside agent getting ready to try to get fflush()
	 * standardized in POSIX, we are going to make our awk consistent
	 * with his.  This should not really affect anyone, as flushing
	 * everything also flushes stdout.
	 *
	 * So. Once upon a time:
	 * 	fflush()	--- flush stdout
	 * 	fflush("")	--- flush everything
	 * Now, both calls flush everything.
	 */

	/* fflush() */
	if (nargs == 0) {
		status = flush_io();
		return make_number((AWKNUM) status);
	}

	tmp = POP_STRING();
	file = tmp->stptr;

	/* fflush("") */
	if (tmp->stlen == 0) {
		status = flush_io();
		DEREF(tmp);
		return make_number((AWKNUM) status);
	}

	/* fflush("/some/path") */
	rp = getredirect(tmp->stptr, tmp->stlen);
	status = -1;
	if (rp != NULL) {
		if ((rp->flag & (RED_WRITE|RED_APPEND)) == 0) {
			if ((rp->flag & RED_PIPE) != 0)
				warning(_("fflush: cannot flush: pipe `%s' opened for reading, not writing"),
					file);
			else
				warning(_("fflush: cannot flush: file `%s' opened for reading, not writing"),
					file);
			DEREF(tmp);
			return make_number((AWKNUM) status);
		}
		fp = rp->output.fp;
		if (fp != NULL)
			status = rp->output.gawk_fflush(fp, rp->output.opaque);
	} else if ((fp = stdfile(tmp->stptr, tmp->stlen)) != NULL) {
		status = fflush(fp);
	} else {
		status = -1;
		warning(_("fflush: `%s' is not an open file, pipe or co-process"), file);
	}
	DEREF(tmp);
	return make_number((AWKNUM) status);
}

/* strncasecmpmbs --- like strncasecmp (multibyte string version)  */

int
strncasecmpmbs(const unsigned char *s1, const unsigned char *s2, size_t n)
{
	size_t i1, i2, mbclen1, mbclen2, gap;
	wchar_t wc1, wc2;
	mbstate_t mbs1, mbs2;

	memset(& mbs1, 0, sizeof(mbs1));
	memset(& mbs2, 0, sizeof(mbs2));

	for (i1 = i2 = 0 ; i1 < n && i2 < n ;i1 += mbclen1, i2 += mbclen2) {
		if (is_valid_character(s1[i1])) {
			mbclen1 = 1;
			wc1 = btowc_cache(s1[i1]);
		} else {
			mbclen1 = mbrtowc(& wc1, (const char *)s1 + i1,
					  n - i1, & mbs1);
			if (mbclen1 == (size_t) -1 || mbclen1 == (size_t) -2 || mbclen1 == 0) {
				/* We treat it as a singlebyte character. */
				mbclen1 = 1;
				wc1 = btowc_cache(s1[i1]);
			}
		}
		if (is_valid_character(s2[i2])) {
			mbclen2 = 1;
			wc2 = btowc_cache(s2[i2]);
		} else {
			mbclen2 = mbrtowc(& wc2, (const char *)s2 + i2,
					  n - i2, & mbs2);
			if (mbclen2 == (size_t) -1 || mbclen2 == (size_t) -2 || mbclen2 == 0) {
				/* We treat it as a singlebyte character. */
				mbclen2 = 1;
				wc2 = btowc_cache(s2[i2]);
			}
		}
		if ((gap = towlower(wc1) - towlower(wc2)) != 0)
			/* s1 and s2 are not equivalent. */
			return gap;
	}
	/* s1 and s2 are equivalent. */
	return 0;
}

/* Inspect the buffer `src' and write the index of each byte to `dest'.
   Caller must allocate `dest'.
   e.g. str = <mb1(1)>, <mb1(2)>, a, b, <mb2(1)>, <mb2(2)>, <mb2(3)>, c
        where mb(i) means the `i'-th byte of a multibyte character.
		dest =       1,        2, 1, 1,        1,        2,        3. 1
*/
static void
index_multibyte_buffer(char* src, char* dest, int len)
{
	int idx, prev_idx;
	mbstate_t mbs, prevs;

	memset(& prevs, 0, sizeof(mbstate_t));
	for (idx = prev_idx = 0 ; idx < len ; idx++) {
		size_t mbclen;
		mbs = prevs;
		mbclen = mbrlen(src + prev_idx, idx - prev_idx + 1, & mbs);
		if (mbclen == (size_t) -1 || mbclen == 1 || mbclen == 0) {
			/* singlebyte character.  */
			mbclen = 1;
			prev_idx = idx + 1;
		} else if (mbclen == (size_t) -2) {
			/* a part of a multibyte character.  */
			mbclen = idx - prev_idx + 1;
		} else if (mbclen > 1) {
			/* the end of a multibyte character.  */
			prev_idx = idx + 1;
			prevs = mbs;
		} else {
			/* Can't reach.  */
		}
		dest[idx] = mbclen;
    }
}

/* do_index --- find index of a string */

NODE *
do_index(int nargs)
{
	NODE *s1, *s2;
	const char *p1, *p2;
	size_t l1, l2;
	long ret;
	bool do_single_byte = false;
	mbstate_t mbs1, mbs2;

	if (gawk_mb_cur_max > 1) {
		memset(& mbs1, 0, sizeof(mbstate_t));
		memset(& mbs2, 0, sizeof(mbstate_t));
	}

	POP_TWO_SCALARS(s1, s2);

	if (do_lint) {
		if ((s1->flags & (STRING|STRCUR)) == 0)
			lintwarn(_("index: received non-string first argument"));
		if ((s2->flags & (STRING|STRCUR)) == 0)
			lintwarn(_("index: received non-string second argument"));
	}

	s1 = force_string(s1);
	s2 = force_string(s2);

	p1 = s1->stptr;
	p2 = s2->stptr;
	l1 = s1->stlen;
	l2 = s2->stlen;
	ret = 0;

	/*
	 * Icky special case, index(foo, "") should return 1,
	 * since both bwk awk and mawk do, and since match("foo", "")
	 * returns 1. This makes index("", "") work, too, fwiw.
	 */
	if (l2 == 0) {
		ret = 1;
		goto out;
	}

	if (gawk_mb_cur_max > 1) {
		s1 = force_wstring(s1);
		s2 = force_wstring(s2);
		/*
		 * If we don't have valid wide character strings, use
		 * the real bytes.
		 */
		do_single_byte = ((s1->wstlen == 0 && s1->stlen > 0) 
					|| (s2->wstlen == 0 && s2->stlen > 0));
	}

	/* IGNORECASE will already be false if posix */
	if (IGNORECASE) {
		while (l1 > 0) {
			if (l2 > l1)
				break;
			if (! do_single_byte && gawk_mb_cur_max > 1) {
				const wchar_t *pos;

				pos = wcasestrstr(s1->wstptr, s1->wstlen, s2->wstptr, s2->wstlen);
				if (pos == NULL)
					ret = 0;
				else
					ret = pos - s1->wstptr + 1;	/* 1-based */
				goto out;
			} else {
				/*
				 * Could use tolower(*p1) == tolower(*p2) here.
				 * See discussion in eval.c as to why not.
				 */
				if (casetable[(unsigned char)*p1] == casetable[(unsigned char)*p2]
				    && (l2 == 1 || strncasecmp(p1, p2, l2) == 0)) {
					ret = 1 + s1->stlen - l1;
					break;
				}
				l1--;
				p1++;
			}
		}
	} else {
		while (l1 > 0) {
			if (l2 > l1)
				break;
			if (*p1 == *p2
			    && (l2 == 1 || (l2 > 0 && memcmp(p1, p2, l2) == 0))) {
				ret = 1 + s1->stlen - l1;
				break;
			}
			if (! do_single_byte && gawk_mb_cur_max > 1) {
				const wchar_t *pos;

				pos = wstrstr(s1->wstptr, s1->wstlen, s2->wstptr, s2->wstlen);
				if (pos == NULL)
					ret = 0;
				else
					ret = pos - s1->wstptr + 1;	/* 1-based */
				goto out;
			} else {
				l1--;
				p1++;
			}
		}
	}
out:
	DEREF(s1);
	DEREF(s2);
	return make_number((AWKNUM) ret);
}

/* double_to_int --- convert double to int, used several places */

double
double_to_int(double d)
{
	if (d >= 0)
		d = floor(d);
	else
		d = ceil(d);
	return d;
}

/* do_int --- convert double to int for awk */

NODE *
do_int(int nargs)
{
	NODE *tmp;
	double d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("int: received non-numeric argument"));
	d = force_number(tmp)->numbr;
	d = double_to_int(d);
	DEREF(tmp);
	return make_number((AWKNUM) d);
}

/* do_isarray --- check if argument is array */

NODE *
do_isarray(int nargs)
{
	NODE *tmp;
	int ret = 1;

	tmp = POP();
	if (tmp->type != Node_var_array) {
		ret = 0;
		DEREF(tmp);
	}
	return make_number((AWKNUM) ret);
}

/* do_length --- length of a string, array or $0 */

NODE *
do_length(int nargs)
{
	NODE *tmp;
	size_t len;

	tmp = POP();
	if (tmp->type == Node_var_array) {
		static bool warned = false;
		unsigned long size;

		if (do_posix)
			fatal(_("length: received array argument"));
   		if (do_lint && ! warned) {
			warned = true;
			lintwarn(_("`length(array)' is a gawk extension"));
		}

		/*
		 * Support for deferred loading of array elements requires that
		 * we use the array length interface even though it isn't 
		 * necessary for the built-in array types.
		 *
		 * 1/2015: The deferred arrays are gone, but this is probably
		 * still a good idea.
		 */

		size = assoc_length(tmp);
		return make_number(size);
	}

	assert(tmp->type == Node_val);

	if (do_lint && (tmp->flags & (STRING|STRCUR)) == 0)
		lintwarn(_("length: received non-string argument"));
	tmp = force_string(tmp);

	if (gawk_mb_cur_max > 1) {
		tmp = force_wstring(tmp);
		len = tmp->wstlen;
		/*
		 * If the bytes don't make a valid wide character
		 * string, fall back to the bytes themselves.
		 */
		 if (len == 0 && tmp->stlen > 0)
			 len = tmp->stlen;
	} else
		len = tmp->stlen;

	DEREF(tmp);
	return make_number((AWKNUM) len);
}

/* do_log --- the log function */

NODE *
do_log(int nargs)
{
	NODE *tmp;
	double d, arg;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("log: received non-numeric argument"));
	arg = force_number(tmp)->numbr;
	if (arg < 0.0)
		warning(_("log: received negative argument %g"), arg);
	d = log(arg);
	DEREF(tmp);
	return make_number((AWKNUM) d);
}


#ifdef HAVE_MPFR

/*
 * mpz2mpfr --- convert an arbitrary-precision integer to a float
 *	without any loss of precision. The returned value is only
 * 	good for temporary use.
 */


static mpfr_ptr
mpz2mpfr(mpz_ptr zi)
{
	size_t prec;
	static mpfr_t mpfrval;
	static bool inited = false;
	int tval;

	/* estimate minimum precision for exact conversion */
	prec = mpz_sizeinbase(zi, 2);	/* most significant 1 bit position starting at 1 */
	prec -= (size_t) mpz_scan1(zi, 0);	/* least significant 1 bit index starting at 0 */
	if (prec < MPFR_PREC_MIN)
		prec = MPFR_PREC_MIN;
	else if (prec > MPFR_PREC_MAX)
		prec = MPFR_PREC_MAX;

	if (! inited) {
		mpfr_init2(mpfrval, prec);
		inited = true;
	} else
		mpfr_set_prec(mpfrval, prec);
	tval = mpfr_set_z(mpfrval, zi, ROUND_MODE);
	IEEE_FMT(mpfrval, tval);
	return mpfrval;
}
#endif

/*
 * format_tree() formats arguments of sprintf,
 * and accordingly to a fmt_string providing a format like in
 * printf family from C library.  Returns a string node which value
 * is a formatted string.  Called by  sprintf function.
 *
 * It is one of the uglier parts of gawk.  Thanks to Michal Jaegermann
 * for taming this beast and making it compatible with ANSI C.
 */

NODE *
format_tree(
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
	int i, nc;
	bool toofew = false;
	char *obuf, *obufout;
	size_t osiz, ofre;
	const char *chbuf;
	const char *s0, *s1;
	int cs1;
	NODE *arg;
	long fw, prec, argnum;
	bool used_dollar;
	bool lj, alt, big_flag, bigbig_flag, small_flag, have_prec, need_format;
	long *cur = NULL;
	uintmax_t uval;
	bool sgn;
	int base;
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
	AWKNUM tmpval = 0.0;
	char signchar = '\0';
	size_t len;
	bool zero_flag = false;
	bool quote_flag = false;
	int ii, jj;
	char *chp;
	size_t copy_count, char_count;
#ifdef HAVE_MPFR
	mpz_ptr zi;
	mpfr_ptr mf;
#endif
	enum { MP_NONE = 0, MP_INT_WITH_PREC = 1, MP_INT_WITHOUT_PREC, MP_FLOAT } fmt_type;

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
			msg(_("fatal: must use `count$' on all formats or none")); \
			goto out; \
		} \
		arg = the_args[argnum]; \
	} else if (used_dollar) { \
		msg(_("fatal: must use `count$' on all formats or none")); \
		arg = 0; /* shutup the compiler */ \
		goto out; \
	} else if (cur_arg >= num_args) { \
		arg = 0; /* shutup the compiler */ \
		toofew = true; \
		break; \
	} else { \
		arg = the_args[cur_arg]; \
		cur_arg++; \
	} \
}

	need_format = false;
	used_dollar = false;

	s0 = s1 = fmt_string;
	while (n0-- > 0) {
		if (*s1 != '%') {
			s1++;
			continue;
		}
		need_format = true;
		bchunk(s0, s1 - s0);
		s0 = s1;
		cur = &fw;
		fw = 0;
		prec = 0;
		base = 0;
		argnum = 0;
		base = 0;
		have_prec = false;
		signchar = '\0';
		zero_flag = false;
		quote_flag = false;
#ifdef HAVE_MPFR
		mf = NULL;
		zi = NULL;
#endif
		fmt_type = MP_NONE;

		lj = alt = big_flag = bigbig_flag = small_flag = false;
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
			need_format = false;
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
				zero_flag = true;
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
				have_prec = false;
			if (cur == &prec)
				cur = NULL;
			if (n0 == 0)	/* badly formatted control string */
				continue;
			goto retry;
		case '$':
			if (do_traditional) {
				msg(_("fatal: `$' is not permitted in awk formats"));
				goto out;
			}

			if (cur == &fw) {
				argnum = fw;
				fw = 0;
				used_dollar = true;
				if (argnum <= 0) {
					msg(_("fatal: arg count with `$' must be > 0"));
					goto out;
				}
				if (argnum >= num_args) {
					msg(_("fatal: arg count %ld greater than total number of supplied arguments"), argnum);
					goto out;
				}
			} else {
				msg(_("fatal: `$' not permitted after period in format"));
				goto out;
			}

			goto retry;
		case '*':
			if (cur == NULL)
				break;
			if (! do_traditional && used_dollar && ! isdigit((unsigned char) *s1)) {
				fatal(_("fatal: must use `count$' on all formats or none"));
				break;	/* silence warnings */
			} else if (! do_traditional && isdigit((unsigned char) *s1)) {
				int val = 0;

				for (; n0 > 0 && *s1 && isdigit((unsigned char) *s1); s1++, n0--) {
					val *= 10;
					val += *s1 - '0';
				}
				if (*s1 != '$') {
					msg(_("fatal: no `$' supplied for positional field width or precision"));
					goto out;
				} else {
					s1++;
					n0--;
				}
				if (val >= num_args) {
					toofew = true;
					break;
				}
				arg = the_args[val];
			} else {
				parse_next_arg();
			}
			(void) force_number(arg);
			*cur = get_number_si(arg);
			if (*cur < 0 && cur == &fw) {
				*cur = -*cur;
				lj++;
			}
			if (cur == &prec) {
				if (*cur >= 0)
					have_prec = true;
				else
					have_prec = false;
				cur = NULL;
			}
			goto retry;
		case ' ':		/* print ' ' or '-' */
					/* 'space' flag is ignored */
					/* if '+' already present  */
			if (signchar != false) 
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
			have_prec = true;
			goto retry;
		case '#':
			alt = true;
			goto check_pos;
		case '\'':
#if defined(HAVE_LOCALE_H)       
			quote_flag = true;
			goto check_pos;
#else
			goto retry;  
#endif
		case 'l':
			if (big_flag)
				break;
			else {
				static bool warned = false;
				
				if (do_lint && ! warned) {
					lintwarn(_("`l' is meaningless in awk formats; ignored"));
					warned = true;
				}
				if (do_posix) {
					msg(_("fatal: `l' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			big_flag = true;
			goto retry;
		case 'L':
			if (bigbig_flag)
				break;
			else {
				static bool warned = false;
				
				if (do_lint && ! warned) {
					lintwarn(_("`L' is meaningless in awk formats; ignored"));
					warned = true;
				}
				if (do_posix) {
					msg(_("fatal: `L' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			bigbig_flag = true;
			goto retry;
		case 'h':
			if (small_flag)
				break;
			else {
				static bool warned = false;
				
				if (do_lint && ! warned) {
					lintwarn(_("`h' is meaningless in awk formats; ignored"));
					warned = true;
				}
				if (do_posix) {
					msg(_("fatal: `h' is not permitted in POSIX awk formats"));
					goto out;
				}
			}
			small_flag = true;
			goto retry;
		case 'c':
			need_format = false;
			parse_next_arg();
			/* user input that looks numeric is numeric */
			if ((arg->flags & (MAYBE_NUM|NUMBER)) == MAYBE_NUM)
				(void) force_number(arg);
			if ((arg->flags & NUMBER) != 0) {
				uval = get_number_uj(arg);
				if (gawk_mb_cur_max > 1) {
					char buf[100];
					wchar_t wc;
					mbstate_t mbs;
					size_t count;

					memset(& mbs, 0, sizeof(mbs));

					/* handle systems with too small wchar_t */
					if (sizeof(wchar_t) < 4 && uval > 0xffff) {
						if (do_lint)
							lintwarn(
						_("[s]printf: value %g is too big for %%c format"),
									arg->numbr);

						goto out0;
					}

					wc = uval;

					count = wcrtomb(buf, wc, & mbs);
					if (count == 0
					    || count == (size_t) -1) {
						if (do_lint)
							lintwarn(
						_("[s]printf: value %g is not a valid wide character"),
									arg->numbr);

						goto out0;
					}

					memcpy(cpbuf, buf, count);
					prec = count;
					cp = cpbuf;
					goto pr_tail;
				}
out0:
				;
				/* else,
					fall through */

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
			prec = 1;
			/*
			 * First character can be multiple bytes if
			 * it's a multibyte character. Grr.
			 */
			if (gawk_mb_cur_max > 1) {
				mbstate_t state;
				size_t count;

				memset(& state, 0, sizeof(state));
				count = mbrlen(cp, arg->stlen, & state);
				if (count != (size_t) -1 && count != (size_t) -2 && count > 0) {
					prec = count;
					/* may need to increase fw so that padding happens, see pr_tail code */
					if (fw > 0)
						fw += count - 1;
				}
			}
			goto pr_tail;
		case 's':
			need_format = false;
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
			need_format = false;
			parse_next_arg();
			(void) force_number(arg);
#ifdef HAVE_MPFR
			if (is_mpg_float(arg))
				goto mpf0;
			else if (is_mpg_integer(arg))
				goto mpz0;
			else
#endif
			tmpval = arg->numbr;

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
				sgn = true;
			} else {
				if (tmpval == -0.0)
					/* avoid printing -0 */
					tmpval = 0.0;
				sgn = false;
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
#if defined(HAVE_LOCALE_H)
			quote_flag = (quote_flag && loc.thousands_sep[0] != 0);
#endif
			chp = &cpbufs[1].buf[i-1];
			ii = jj = 0;
			do {
				PREPEND(*chp);
				chp--; i--;
#if defined(HAVE_LOCALE_H)
				if (quote_flag && loc.grouping[ii] && ++jj == loc.grouping[ii]) {
					if (i) {	/* only add if more digits coming */
						int k;
						const char *ts = loc.thousands_sep;

						for (k = strlen(ts) - 1; k >= 0; k--) {
							PREPEND(ts[k]);
						}
					}
					if (loc.grouping[ii+1] == 0)
						jj = 0;		/* keep using current val in loc.grouping[ii] */
					else if (loc.grouping[ii+1] == CHAR_MAX)
						quote_flag = false;
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
			need_format = false;
			parse_next_arg();
			(void) force_number(arg);
#ifdef HAVE_MPFR
			if (is_mpg_integer(arg)) {
mpz0:
				zi = arg->mpg_i;

				if (cs1 != 'd' && cs1 != 'i') {
					if (mpz_sgn(zi) <= 0) {
						/*
						 * Negative value or 0 requires special handling.
						 * Unlike MPFR, GMP does not allow conversion
						 * to (u)intmax_t. So we first convert GMP type to
						 * a MPFR type.
						 */
						mf = mpz2mpfr(zi);
						goto mpf1;
					}
					signchar = '\0';	/* Don't print '+' */
				}

				/* See comments above about when to fill with zeros */
				zero_flag = (! lj
						    && ((zero_flag && ! have_prec)
							 || (fw == 0 && have_prec)));

 				fmt_type = have_prec ? MP_INT_WITH_PREC : MP_INT_WITHOUT_PREC;
				goto fmt0;

			} else if (is_mpg_float(arg)) {
mpf0:
				mf = arg->mpg_numbr;
				if (! mpfr_number_p(mf)) {
					/* inf or NaN */
					cs1 = 'g';
					fmt_type = MP_FLOAT;
					goto fmt1;
				}

				if (cs1 != 'd' && cs1 != 'i') {
mpf1:
					/*
					 * The output of printf("%#.0x", 0) is 0 instead of 0x, hence <= in
					 * the comparison below.
					 */
					if (mpfr_sgn(mf) <= 0) {
						if (! mpfr_fits_intmax_p(mf, ROUND_MODE)) {
							/* -ve number is too large */
							cs1 = 'g';
							fmt_type = MP_FLOAT;
							goto fmt1;
						}

						tmpval = uval = (uintmax_t) mpfr_get_sj(mf, ROUND_MODE);
						if (! alt && have_prec && prec == 0 && tmpval == 0)
							goto pr_tail;	/* printf("%.0x", 0) is no characters */
						goto int0;
					}
					signchar = '\0';	/* Don't print '+' */
				}

				/* See comments above about when to fill with zeros */
				zero_flag = (! lj
						    && ((zero_flag && ! have_prec)
							 || (fw == 0 && have_prec)));
				
				(void) mpfr_get_z(mpzval, mf, MPFR_RNDZ);	/* convert to GMP integer */
 				fmt_type = have_prec ? MP_INT_WITH_PREC : MP_INT_WITHOUT_PREC;
				zi = mpzval;
				goto fmt0;
			} else
#endif
				tmpval = arg->numbr;

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
				if ((AWKNUM)(intmax_t)uval != double_to_int(tmpval))
					goto out_of_range;
			} else {
				uval = (uintmax_t) tmpval;
				if ((AWKNUM)uval != double_to_int(tmpval))
					goto out_of_range;
			}
#ifdef HAVE_MPFR
	int0:
#endif
#if defined(HAVE_LOCALE_H)
			quote_flag = (quote_flag && loc.thousands_sep[0] != 0);
#endif
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
					if (uval) {	/* only add if more digits coming */
						int k;
						const char *ts = loc.thousands_sep;

						for (k = strlen(ts) - 1; k >= 0; k--) {
							PREPEND(ts[k]);
						}
					}
					if (loc.grouping[ii+1] == 0)                                          
						jj = 0;     /* keep using current val in loc.grouping[ii] */
					else if (loc.grouping[ii+1] == CHAR_MAX)                        
						quote_flag = false;
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
			else if (gawk_mb_cur_max > 1) {
				if (cs1 == 's') {
					assert(cp == arg->stptr || cp == cpbuf);
					copy_count = mbc_byte_count(arg->stptr, prec);
				}
				/* prec was set by code for %c */
				/* else
					copy_count = prec; */
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
			goto fmt1;

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
			need_format = false;
			parse_next_arg();
			(void) force_number(arg);

			if (! is_mpg_number(arg))
				tmpval = arg->numbr;
#ifdef HAVE_MPFR
			else if (is_mpg_float(arg)) {
				mf = arg->mpg_numbr;
				fmt_type = MP_FLOAT;
			} else {
				/* arbitrary-precision integer, convert to MPFR float */
				assert(mf == NULL);
				mf = mpz2mpfr(arg->mpg_i);
				fmt_type = MP_FLOAT;
			}
#endif
     fmt1:
			if (! have_prec)
				prec = DEFAULT_G_PRECISION;
#ifdef HAVE_MPFR
     fmt0:
#endif
			chksize(fw + prec + 11);	/* 11 == slop */
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

#if defined(LC_NUMERIC)
			if (quote_flag && ! use_lc_numeric)
				setlocale(LC_NUMERIC, "");
#endif

			switch (fmt_type) {
#ifdef HAVE_MPFR
			case MP_INT_WITH_PREC:
				sprintf(cp, "*.*Z%c", cs1);
				while ((nc = mpfr_snprintf(obufout, ofre, cpbuf,
					     (int) fw, (int) prec, zi)) >= ofre)
					chksize(nc)
				break;
			case MP_INT_WITHOUT_PREC:
				sprintf(cp, "*Z%c", cs1);
				while ((nc = mpfr_snprintf(obufout, ofre, cpbuf,
					     (int) fw, zi)) >= ofre)
					chksize(nc)
				break;
			case MP_FLOAT:
				sprintf(cp, "*.*R*%c", cs1);
				while ((nc = mpfr_snprintf(obufout, ofre, cpbuf,
					     (int) fw, (int) prec, ROUND_MODE, mf)) >= ofre)
					chksize(nc)
				break;
#endif
			default:
				sprintf(cp, "*.*%c", cs1);
				while ((nc = snprintf(obufout, ofre, cpbuf,
					     (int) fw, (int) prec,
					     (double) tmpval)) >= ofre)
					chksize(nc)
			}

#if defined(LC_NUMERIC)
			if (quote_flag && ! use_lc_numeric)
				setlocale(LC_NUMERIC, "C");
#endif

			len = strlen(obufout);
			ofre -= len;
			obufout += len;
			s0 = s1;
			break;
		default:
			if (do_lint && is_alpha(cs1))
				lintwarn(_("ignoring unknown format specifier character `%c': no argument converted"), cs1);
			break;
		}
		if (toofew) {
			msg("%s\n\t`%s'\n\t%*s%s",
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

	if (r == NULL)
		gawk_exit(EXIT_FATAL);
	return r;
}


/* printf_common --- common code for sprintf and printf */

static NODE *
printf_common(int nargs)
{
	int i;
	NODE *r, *tmp;

	assert(nargs > 0 && nargs <= max_args);
	for (i = 1; i <= nargs; i++) {
		tmp = args_array[nargs - i] = POP();
		if (tmp->type == Node_var_array) {
			while (--i > 0)
				DEREF(args_array[nargs - i]);
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(tmp));
		}
	}

	args_array[0] = force_string(args_array[0]);
	r = format_tree(args_array[0]->stptr, args_array[0]->stlen, args_array, nargs);
	for (i = 0; i < nargs; i++)
		DEREF(args_array[i]);
	return r;
}

/* do_sprintf --- perform sprintf */

NODE *
do_sprintf(int nargs)
{
	NODE *r;

	if (nargs == 0)
		fatal(_("sprintf: no arguments"));

	r = printf_common(nargs);
	if (r == NULL)
		gawk_exit(EXIT_FATAL);
	return r;
}


/* do_printf --- perform printf, including redirection */

void
do_printf(int nargs, int redirtype)
{
	FILE *fp = NULL;
	NODE *tmp;
	struct redirect *rp = NULL;
	int errflg;	/* not used, sigh */
	NODE *redir_exp = NULL;

	if (nargs == 0) {
		if (do_traditional) {
			if (do_lint)
				lintwarn(_("printf: no arguments"));
			if (redirtype != 0) {
				redir_exp = TOP();
				if (redir_exp->type != Node_val)
					fatal(_("attempt to use array `%s' in a scalar context"), array_vname(redir_exp));
				rp = redirect(redir_exp, redirtype, & errflg);
				DEREF(redir_exp);
				decr_sp();
			}
			return;	/* bwk accepts it silently */
		}
		fatal(_("printf: no arguments"));
	}

	if (redirtype != 0) {
		redir_exp = PEEK(nargs);
		if (redir_exp->type != Node_val)
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(redir_exp));
		rp = redirect(redir_exp, redirtype, & errflg);
		if (rp != NULL)
			fp = rp->output.fp;
	} else if (do_debug)	/* only the debugger can change the default output */
		fp = output_fp;
	else
		fp = stdout;

	tmp = printf_common(nargs);
	if (redir_exp != NULL) {
		DEREF(redir_exp);
		decr_sp();
	}
	if (tmp != NULL) {
		if (fp == NULL) {
			DEREF(tmp);
			return;
		}
		efwrite(tmp->stptr, sizeof(char), tmp->stlen, fp, "printf", rp, true);
		if (rp != NULL && (rp->flag & RED_TWOWAY) != 0)
			rp->output.gawk_fflush(rp->output.fp, rp->output.opaque);
		DEREF(tmp);
	} else
		gawk_exit(EXIT_FATAL);
}

/* do_sqrt --- do the sqrt function */

NODE *
do_sqrt(int nargs)
{
	NODE *tmp;
	double arg;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("sqrt: received non-numeric argument"));
	arg = (double) force_number(tmp)->numbr;
	DEREF(tmp);
	if (arg < 0.0)
		warning(_("sqrt: called with negative argument %g"), arg);
	return make_number((AWKNUM) sqrt(arg));
}

/* do_substr --- do the substr function */

NODE *
do_substr(int nargs)
{
	NODE *t1;
	NODE *r;
	size_t indx;
	size_t length = 0;
	double d_index = 0, d_length = 0;
	size_t src_len;

	if (nargs == 3) {
		t1 = POP_NUMBER();
		d_length = get_number_d(t1);
		DEREF(t1);
	}

	t1 = POP_NUMBER();
	d_index = get_number_d(t1);
	DEREF(t1);

	t1 = POP_STRING();

	if (nargs == 3) {
		if (! (d_length >= 1)) {
			if (do_lint == DO_LINT_ALL)
				lintwarn(_("substr: length %g is not >= 1"), d_length);
			else if (do_lint == DO_LINT_INVALID && ! (d_length >= 0))
				lintwarn(_("substr: length %g is not >= 0"), d_length);
			DEREF(t1);
			/*
			 * Return explicit null string instead of doing
			 * dupnode(Nnull_string) so that if the result
			 * is checked with the combination of length()
			 * and lint, no error is reported about using
			 * an uninitialized value. Same thing later, too.
			 */
			return make_string("", 0);
		}
		if (do_lint) {
			if (double_to_int(d_length) != d_length)
				lintwarn(
			_("substr: non-integer length %g will be truncated"),
					d_length);

			if (d_length > SIZE_MAX)
				lintwarn(
			_("substr: length %g too big for string indexing, truncating to %g"),
					d_length, (double) SIZE_MAX);
		}
		if (d_length < SIZE_MAX)
			length = d_length;
		else
			length = SIZE_MAX;
	}

	/* the weird `! (foo)' tests help catch NaN values. */
	if (! (d_index >= 1)) {
		if (do_lint)
			lintwarn(_("substr: start index %g is invalid, using 1"),
				 d_index);
		d_index = 1;
	}
	if (do_lint && double_to_int(d_index) != d_index)
		lintwarn(_("substr: non-integer start index %g will be truncated"),
			 d_index);

	/* awk indices are from 1, C's are from 0 */
	if (d_index <= SIZE_MAX)
		indx = d_index - 1;
	else
		indx = SIZE_MAX;

	if (nargs == 2) {	/* third arg. missing */
		/* use remainder of string */
		length = t1->stlen - indx;	/* default to bytes */
		if (gawk_mb_cur_max > 1) {
			t1 = force_wstring(t1);
			if (t1->wstlen > 0)	/* use length of wide char string if we have one */
				length = t1->wstlen - indx;
		}
		d_length = length;	/* set here in case used in diagnostics, below */
	}

	if (t1->stlen == 0) {
		/* substr("", 1, 0) produces a warning only if LINT_ALL */
		if (do_lint && (do_lint == DO_LINT_ALL || ((indx | length) != 0)))
			lintwarn(_("substr: source string is zero length"));
		DEREF(t1);
		return make_string("", 0);
	}

	/* get total len of input string, for following checks */
	if (gawk_mb_cur_max > 1) {
		t1 = force_wstring(t1);
		src_len = t1->wstlen;
	} else
		src_len = t1->stlen;

	if (indx >= src_len) {
		if (do_lint)
			lintwarn(_("substr: start index %g is past end of string"),
				d_index);
		DEREF(t1);
		return make_string("", 0);
	}
	if (length > src_len - indx) {
		if (do_lint)
			lintwarn(
	_("substr: length %g at start index %g exceeds length of first argument (%lu)"),
			d_length, d_index, (unsigned long int) src_len);
		length = src_len - indx;
	}

	/* force_wstring() already called */
	if (gawk_mb_cur_max == 1 || t1->wstlen == t1->stlen)
		/* single byte case */
		r = make_string(t1->stptr + indx, length);
	else {
		/* multibyte case, more work */
		size_t result;
		wchar_t *wp;
		mbstate_t mbs;
		char *substr, *cp;

		/*
		 * Convert the wide chars in t1->wstptr back into m.b. chars.
		 * This is pretty grotty, but it's the most straightforward
		 * way to do things.
		 */
		memset(& mbs, 0, sizeof(mbs));
		emalloc(substr, char *, (length * gawk_mb_cur_max) + 2, "do_substr");
		wp = t1->wstptr + indx;
		for (cp = substr; length > 0; length--) {
			result = wcrtomb(cp, *wp, & mbs);
			if (result == (size_t) -1)	/* what to do? break seems best */
				break;
			cp += result;
			wp++;
		}
		*cp = '\0';
		r = make_str_node(substr, cp - substr, ALREADY_MALLOCED);
	}

	DEREF(t1);
	return r;
}

/* do_strftime --- format a time stamp */

NODE *
do_strftime(int nargs)
{
	NODE *t1, *t2, *t3, *ret;
	struct tm *tm;
	time_t fclock;
	double clock_val;
	char *bufp;
	size_t buflen, bufsize;
	char buf[BUFSIZ];
	const char *format;
	int formatlen;
	int do_gmt;
	NODE *val = NULL;
	NODE *sub = NULL;
	static const time_t time_t_min = TYPE_MINIMUM(time_t);
	static const time_t time_t_max = TYPE_MAXIMUM(time_t);

	/* set defaults first */
	format = def_strftime_format;	/* traditional date format */
	formatlen = strlen(format);
	(void) time(& fclock);	/* current time of day */
	do_gmt = false;

	if (PROCINFO_node != NULL) {
		sub = make_string("strftime", 8);
		val = in_array(PROCINFO_node, sub);
		unref(sub);

		if (val != NULL) {
			if (do_lint && (val->flags & STRING) == 0)
				lintwarn(_("strftime: format value in PROCINFO[\"strftime\"] has numeric type"));
			val = force_string(val);
			format = val->stptr;
			formatlen = val->stlen;
		}
	}

	t1 = t2 = t3 = NULL;
	if (nargs > 0) {	/* have args */
		NODE *tmp;

		if (nargs == 3) {
			t3 = POP_SCALAR();
			if ((t3->flags & (NUMCUR|NUMBER)) != 0)
				do_gmt = (t3->numbr != 0);
			else
				do_gmt = (t3->stlen > 0);
			DEREF(t3);
		}

		if (nargs >= 2) {
			t2 = POP_SCALAR();
			if (do_lint && (t2->flags & (NUMCUR|NUMBER)) == 0)
				lintwarn(_("strftime: received non-numeric second argument"));
			(void) force_number(t2);
			clock_val = get_number_d(t2);
			fclock = (time_t) clock_val;
			/*
			 * Protect against negative value being assigned
			 * to unsigned time_t.
			 */
			if (clock_val < 0 && fclock > 0) {
				if (do_lint)
					lintwarn(_("strftime: second argument less than 0 or too big for time_t"));
				return make_string("", 0);
			}

			/* And check that the value is in range */
			if (clock_val < time_t_min || clock_val > time_t_max) {
				if (do_lint)
					lintwarn(_("strftime: second argument out of range for time_t"));
				return make_string("", 0);
			}

			DEREF(t2);
		}

		tmp = POP_SCALAR();
		if (do_lint && (tmp->flags & (STRING|STRCUR)) == 0)
			lintwarn(_("strftime: received non-string first argument"));
	
		t1 = force_string(tmp);
		format = t1->stptr;
		formatlen = t1->stlen;
		if (formatlen == 0) {
			if (do_lint)
				lintwarn(_("strftime: received empty format string"));
			DEREF(t1);
			return make_string("", 0);
		}
	}

	if (do_gmt)
		tm = gmtime(& fclock);
	else
		tm = localtime(& fclock);

	if (tm == NULL)
		return make_string("", 0);

	bufp = buf;
	bufsize = sizeof(buf);
	for (;;) {
		*bufp = '\0';
		buflen = strftime(bufp, bufsize, format, tm);
		/*
		 * buflen can be zero EITHER because there's not enough
		 * room in the string, or because the control command
		 * goes to the empty string. Make a reasonable guess that
		 * if the buffer is 1024 times bigger than the length of the
		 * format string, it's not failing for lack of room.
		 * Thanks to Paul Eggert for pointing out this issue.
		 */
		if (buflen > 0 || bufsize >= 1024 * formatlen)
			break;
		bufsize *= 2;
		if (bufp == buf)
			emalloc(bufp, char *, bufsize, "do_strftime");
		else
			erealloc(bufp, char *, bufsize, "do_strftime");
	}
	ret = make_string(bufp, buflen);
	if (bufp != buf)
		efree(bufp);
	if (t1)
		DEREF(t1);
	return ret;
}

/* do_systime --- get the time of day */

NODE *
do_systime(int nargs ATTRIBUTE_UNUSED)
{
	time_t lclock;

	(void) time(& lclock);
	return make_number((AWKNUM) lclock);
}

/* do_mktime --- turn a time string into a timestamp */

NODE *
do_mktime(int nargs)
{
	NODE *t1;
	struct tm then;
	long year;
	int month, day, hour, minute, second, count;
	int dst = -1; /* default is unknown */
	time_t then_stamp;
	char save;

	t1 = POP_SCALAR();
	if (do_lint && (t1->flags & (STRING|STRCUR)) == 0)
		lintwarn(_("mktime: received non-string argument"));
	t1 = force_string(t1);

	save = t1->stptr[t1->stlen];
	t1->stptr[t1->stlen] = '\0';

	count = sscanf(t1->stptr, "%ld %d %d %d %d %d %d",
		        & year, & month, & day,
			& hour, & minute, & second,
		        & dst);

	if (   do_lint /* Ready? Set! Go: */
	    && (   (second < 0 || second > 60)
		|| (minute < 0 || minute > 59)
		|| (hour < 0 || hour > 23) /* FIXME ISO 8601 allows 24 ? */
		|| (day < 1 || day > 31)
		|| (month < 1 || month > 12) ))
			lintwarn(_("mktime: at least one of the values is out of the default range"));

	t1->stptr[t1->stlen] = save;
	DEREF(t1);

	if (count < 6
	    || month == INT_MIN
	    || year < INT_MIN + 1900
	    || year - 1900 > INT_MAX)
		return make_number((AWKNUM) -1);

	memset(& then, '\0', sizeof(then));
	then.tm_sec = second;
	then.tm_min = minute;
	then.tm_hour = hour;
	then.tm_mday = day;
	then.tm_mon = month - 1;
	then.tm_year = year - 1900;
	then.tm_isdst = dst;

	then_stamp = mktime(& then);
	return make_number((AWKNUM) then_stamp);
}

/* do_system --- run an external command */

NODE *
do_system(int nargs)
{
	NODE *tmp;
	int ret = 0;
	char *cmd;
	char save;

	if (do_sandbox)
		fatal(_("'system' function not allowed in sandbox mode"));

	(void) flush_io();     /* so output is synchronous with gawk's */
	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (STRING|STRCUR)) == 0)
		lintwarn(_("system: received non-string argument"));
	cmd = force_string(tmp)->stptr;

	if (cmd && *cmd) {
		/* insure arg to system is zero-terminated */
		save = cmd[tmp->stlen];
		cmd[tmp->stlen] = '\0';

		os_restore_mode(fileno(stdin));
		ret = system(cmd);
		if (ret != -1)
			ret = WEXITSTATUS(ret);
		if ((BINMODE & BINMODE_INPUT) != 0)
			os_setbinmode(fileno(stdin), O_BINARY);

		cmd[tmp->stlen] = save;
	}
	DEREF(tmp);
	return make_number((AWKNUM) ret);
}

/* do_print --- print items, separated by OFS, terminated with ORS */

void
do_print(int nargs, int redirtype)
{
	struct redirect *rp = NULL;
	int errflg;	/* not used, sigh */
	FILE *fp = NULL;
	int i;
	NODE *redir_exp = NULL;
	NODE *tmp = NULL;

	assert(nargs <= max_args);

	if (redirtype != 0) {
		redir_exp = PEEK(nargs);
		if (redir_exp->type != Node_val)
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(redir_exp));
		rp = redirect(redir_exp, redirtype, & errflg);
		if (rp != NULL)
			fp = rp->output.fp;
	} else if (do_debug)	/* only the debugger can change the default output */
		fp = output_fp;
	else
		fp = stdout;

	for (i = 1; i <= nargs; i++) {
		tmp = args_array[i] = POP();
		if (tmp->type == Node_var_array) {
			while (--i > 0)
				DEREF(args_array[i]);
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(tmp));
		}

		if ((tmp->flags & (NUMBER|STRING)) == NUMBER) {
			if (OFMTidx == CONVFMTidx)
				args_array[i] = force_string(tmp);
			else
				args_array[i] = format_val(OFMT, OFMTidx, tmp);
		}
	}

	if (redir_exp != NULL) {
		DEREF(redir_exp);
		decr_sp();
	}

	if (fp == NULL) {
		for (i = nargs; i > 0; i--)
			DEREF(args_array[i]);
		return;
	}

	for (i = nargs; i > 0; i--) {
		efwrite(args_array[i]->stptr, sizeof(char), args_array[i]->stlen, fp, "print", rp, false);
		DEREF(args_array[i]);
		if (i != 1 && OFSlen > 0)
			efwrite(OFS, sizeof(char), (size_t) OFSlen,
				fp, "print", rp, false);

	}
	if (ORSlen > 0)
		efwrite(ORS, sizeof(char), (size_t) ORSlen, fp, "print", rp, true);

	if (rp != NULL && (rp->flag & RED_TWOWAY) != 0)
		rp->output.gawk_fflush(rp->output.fp, rp->output.opaque);
}

/* do_print_rec --- special case printing of $0, for speed */

void 
do_print_rec(int nargs, int redirtype)
{
	FILE *fp = NULL;
	NODE *f0;
	struct redirect *rp = NULL;
	int errflg;	/* not used, sigh */
	NODE *redir_exp = NULL;

	assert(nargs == 0);
	if (redirtype != 0) {
		redir_exp = TOP();
		rp = redirect(redir_exp, redirtype, & errflg);
		if (rp != NULL)
			fp = rp->output.fp;
		DEREF(redir_exp);
		decr_sp();
	} else
		fp = output_fp;

	if (fp == NULL)
		return;

	if (! field0_valid)
		(void) get_field(0L, NULL);	/* rebuild record */

	f0 = fields_arr[0];

	if (do_lint && (f0->flags & NULL_FIELD) != 0)
		lintwarn(_("reference to uninitialized field `$%d'"), 0);

	efwrite(f0->stptr, sizeof(char), f0->stlen, fp, "print", rp, false);

	if (ORSlen > 0)
		efwrite(ORS, sizeof(char), (size_t) ORSlen, fp, "print", rp, true);

	if (rp != NULL && (rp->flag & RED_TWOWAY) != 0)
		rp->output.gawk_fflush(rp->output.fp, rp->output.opaque);
}


/* is_wupper --- function version of iswupper for passing function pointers */

static int
is_wupper(wchar_t c)
{
	return iswupper(c);
}

/* is_wlower --- function version of iswlower for passing function pointers */

static int
is_wlower(wchar_t c)
{
	return iswlower(c);
}

/* to_wupper --- function version of towupper for passing function pointers */

static int
to_wlower(wchar_t c)
{
	return towlower(c);
}

/* to_wlower --- function version of towlower for passing function pointers */

static int
to_wupper(wchar_t c)
{
	return towupper(c);
}

/* wide_change_case --- generic case converter for wide characters */

static void
wide_change_case(wchar_t *wstr,
			size_t wlen,
			int (*is_x)(wchar_t c),
			int (*to_y)(wchar_t c))
{
	size_t i;
	wchar_t *wcp;

	for (i = 0, wcp = wstr; i < wlen; i++, wcp++)
		if (is_x(*wcp))
			*wcp = to_y(*wcp);
}

/* wide_toupper --- map a wide string to upper case */

static void
wide_toupper(wchar_t *wstr, size_t wlen)
{
	wide_change_case(wstr, wlen, is_wlower, to_wupper);
}

/* wide_tolower --- map a wide string to lower case */

static void
wide_tolower(wchar_t *wstr, size_t wlen)
{
	wide_change_case(wstr, wlen, is_wupper, to_wlower);
}

/* do_tolower --- lower case a string */

NODE *
do_tolower(int nargs)
{
	NODE *t1, *t2;

	t1 = POP_SCALAR();
	if (do_lint && (t1->flags & (STRING|STRCUR)) == 0)
		lintwarn(_("tolower: received non-string argument"));
	t1 = force_string(t1);
	t2 = make_string(t1->stptr, t1->stlen);

	if (gawk_mb_cur_max == 1) {
		unsigned char *cp, *cp2;

		for (cp = (unsigned char *)t2->stptr,
		     cp2 = (unsigned char *)(t2->stptr + t2->stlen);
			cp < cp2; cp++)
			if (isupper(*cp))
				*cp = tolower(*cp);
	} else {
		force_wstring(t2);
		wide_tolower(t2->wstptr, t2->wstlen);
		wstr2str(t2);
	}

	DEREF(t1);
	return t2;
}

/* do_toupper --- upper case a string */

NODE *
do_toupper(int nargs)
{
	NODE *t1, *t2;

	t1 = POP_SCALAR();
	if (do_lint && (t1->flags & (STRING|STRCUR)) == 0)
		lintwarn(_("toupper: received non-string argument"));
	t1 = force_string(t1);
	t2 = make_string(t1->stptr, t1->stlen);

	if (gawk_mb_cur_max == 1) {
		unsigned char *cp, *cp2;

		for (cp = (unsigned char *)t2->stptr,
		     cp2 = (unsigned char *)(t2->stptr + t2->stlen);
			cp < cp2; cp++)
			if (islower(*cp))
				*cp = toupper(*cp);
	} else {
		force_wstring(t2);
		wide_toupper(t2->wstptr, t2->wstlen);
		wstr2str(t2);
	}

	DEREF(t1);
	return t2;
}

/* do_atan2 --- do the atan2 function */

NODE *
do_atan2(int nargs)
{
	NODE *t1, *t2;
	double d1, d2;

	POP_TWO_SCALARS(t1, t2);
	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric first argument"));
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric second argument"));
	}
	d1 = force_number(t1)->numbr;
	d2 = force_number(t2)->numbr;
	DEREF(t1);
	DEREF(t2);
	return make_number((AWKNUM) atan2(d1, d2));
}

/* do_sin --- do the sin function */

NODE *
do_sin(int nargs)
{
	NODE *tmp;
	double d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("sin: received non-numeric argument"));
	d = sin((double) force_number(tmp)->numbr);
	DEREF(tmp);
	return make_number((AWKNUM) d);
}

/* do_cos --- do the cos function */

NODE *
do_cos(int nargs)
{
	NODE *tmp;
	double d;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("cos: received non-numeric argument"));
	d = cos((double) force_number(tmp)->numbr);
	DEREF(tmp);
	return make_number((AWKNUM) d);
}

/* do_rand --- do the rand function */

static bool firstrand = true;
/* Some systems require this array to be integer aligned. Sigh. */
#define SIZEOF_STATE 256
static uint32_t istate[SIZEOF_STATE/sizeof(uint32_t)];
static char *const state = (char *const) istate;

/* ARGSUSED */
NODE *
do_rand(int nargs ATTRIBUTE_UNUSED)
{
	if (firstrand) {
		(void) initstate((unsigned) 1, state, SIZEOF_STATE);
		/* don't need to srandom(1), initstate() does it for us. */
		firstrand = false;
		setstate(state);
	}
	/*
	 * Per historical practice and POSIX, return value N is
	 *
	 * 	0 <= n < 1
	 */
	return make_number((AWKNUM) (random() % GAWK_RANDOM_MAX) / GAWK_RANDOM_MAX);
}

/* do_srand --- seed the random number generator */

NODE *
do_srand(int nargs)
{
	NODE *tmp;
	static long save_seed = 1;
	long ret = save_seed;	/* SVR4 awk srand returns previous seed */

	if (firstrand) {
		(void) initstate((unsigned) 1, state, SIZEOF_STATE);
		/* don't need to srandom(1), we're changing the seed below */
		firstrand = false;
		(void) setstate(state);
	}

	if (nargs == 0)
		srandom((unsigned int) (save_seed = (long) time((time_t *) 0)));
	else {
		tmp = POP_SCALAR();
		if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("srand: received non-numeric argument"));
		srandom((unsigned int) (save_seed = (long) force_number(tmp)->numbr));
		DEREF(tmp);
	}
	return make_number((AWKNUM) ret);
}

/* do_match --- match a regexp, set RSTART and RLENGTH,
 * 	optional third arg is array filled with text of
 * 	subpatterns enclosed in parens and start and len info.
 */

NODE *
do_match(int nargs)
{
	NODE *tre, *t1, *dest, *it;
	int rstart, len, ii;
	int rlength;
	Regexp *rp;
	regoff_t s;
	char *start;
	char *buf = NULL;
	char buff[100];
	size_t amt, oldamt = 0, ilen, slen;
	char *subsepstr;
	size_t subseplen;

	dest = NULL;
	if (nargs == 3) {	/* 3rd optional arg for the subpatterns */
		dest = POP_PARAM();
		if (dest->type != Node_var_array)
			fatal(_("match: third argument is not an array"));
		assoc_clear(dest);
	}
	tre = POP();
	rp = re_update(tre);
	t1 = POP_STRING();
	
	rstart = research(rp, t1->stptr, 0, t1->stlen, RE_NEED_START);
	if (rstart >= 0) {	/* match succeded */
		size_t *wc_indices = NULL;

		rlength = REEND(rp, t1->stptr) - RESTART(rp, t1->stptr);	/* byte length */
		if (rlength > 0 && gawk_mb_cur_max > 1) {
			t1 = str2wstr(t1, & wc_indices);
			rlength = wc_indices[rstart + rlength - 1] - wc_indices[rstart] + 1;
			rstart = wc_indices[rstart];
		}

		rstart++;	/* now it's 1-based indexing */
	
		/* Build the array only if the caller wants the optional subpatterns */
		if (dest != NULL) {
			subsepstr = SUBSEP_node->var_value->stptr;
			subseplen = SUBSEP_node->var_value->stlen;

			for (ii = 0; ii < NUMSUBPATS(rp, t1->stptr); ii++) {
				/*
				 * Loop over all the subpats; some of them may have
				 * matched even if all of them did not.
				 */
				if ((s = SUBPATSTART(rp, t1->stptr, ii)) != -1) {
					size_t subpat_start;
					size_t subpat_len;
					NODE **lhs;
					NODE *sub;
					
					start = t1->stptr + s;
					subpat_start = s;
					subpat_len = len = SUBPATEND(rp, t1->stptr, ii) - s;
					if (len > 0 && gawk_mb_cur_max > 1) {
						subpat_start = wc_indices[s];
						subpat_len = wc_indices[s + len - 1] - subpat_start + 1;
					}
	
					it = make_string(start, len);
					it->flags |= MAYBE_NUM;	/* user input */

					sub = make_number((AWKNUM) (ii));
					lhs = assoc_lookup(dest, sub);
					unref(*lhs);
					*lhs = it;
					/* execute post-assignment routine if any */
					if (dest->astore != NULL)
						(*dest->astore)(dest, sub);
					unref(sub);

					sprintf(buff, "%d", ii);
					ilen = strlen(buff);
					amt = ilen + subseplen + strlen("length") + 2;
	
					if (oldamt == 0) {
						emalloc(buf, char *, amt, "do_match");
					} else if (amt > oldamt) {
						erealloc(buf, char *, amt, "do_match");
					}
					oldamt = amt;
					memcpy(buf, buff, ilen);
					memcpy(buf + ilen, subsepstr, subseplen);
					memcpy(buf + ilen + subseplen, "start", 6);
	
					slen = ilen + subseplen + 5;
	
					it = make_number((AWKNUM) subpat_start + 1);
					sub = make_string(buf, slen);
					lhs = assoc_lookup(dest, sub);
					unref(*lhs);
					*lhs = it;
					if (dest->astore != NULL)
						(*dest->astore)(dest, sub);
					unref(sub);
	
					memcpy(buf, buff, ilen);
					memcpy(buf + ilen, subsepstr, subseplen);
					memcpy(buf + ilen + subseplen, "length", 7);
	
					slen = ilen + subseplen + 6;
	
					it = make_number((AWKNUM) subpat_len);
					sub = make_string(buf, slen);
					lhs = assoc_lookup(dest, sub);
					unref(*lhs);
					*lhs = it;
					if (dest->astore != NULL)
						(*dest->astore)(dest, sub);
					unref(sub);
				}
			}

			efree(buf);
		}
		if (wc_indices != NULL)
			efree(wc_indices);
	} else {		/* match failed */
		rstart = 0;
		rlength = -1;
	}

	DEREF(t1);
	unref(RSTART_node->var_value);
	RSTART_node->var_value = make_number((AWKNUM) rstart);
	unref(RLENGTH_node->var_value);
	RLENGTH_node->var_value = make_number((AWKNUM) rlength);
	return make_number((AWKNUM) rstart);
}

/* do_sub --- do the work for sub, gsub, and gensub */

/*
 * Gsub can be tricksy; particularly when handling the case of null strings.
 * The following awk code was useful in debugging problems.  It is too bad
 * that it does not readily translate directly into the C code, below.
 * 
 * #! /usr/local/bin/mawk -f
 * 
 * BEGIN {
 * 	true = 1; false = 0
 * 	print "--->", mygsub("abc", "b+", "FOO")
 * 	print "--->", mygsub("abc", "x*", "X")
 * 	print "--->", mygsub("abc", "b*", "X")
 * 	print "--->", mygsub("abc", "c", "X")
 * 	print "--->", mygsub("abc", "c+", "X")
 * 	print "--->", mygsub("abc", "x*$", "X")
 * }
 * 
 * function mygsub(str, regex, replace,	origstr, newstr, eosflag, nonzeroflag)
 * {
 * 	origstr = str;
 * 	eosflag = nonzeroflag = false
 * 	while (match(str, regex)) {
 * 		if (RLENGTH > 0) {	# easy case
 * 			nonzeroflag = true
 * 			if (RSTART == 1) {	# match at front of string
 * 				newstr = newstr replace
 * 			} else {
 * 				newstr = newstr substr(str, 1, RSTART-1) replace
 * 			}
 * 			str = substr(str, RSTART+RLENGTH)
 * 		} else if (nonzeroflag) {
 * 			# last match was non-zero in length, and at the
 * 			# current character, we get a zero length match,
 * 			# which we don't really want, so skip over it
 * 			newstr = newstr substr(str, 1, 1)
 * 			str = substr(str, 2)
 * 			nonzeroflag = false
 * 		} else {
 * 			# 0-length match
 * 			if (RSTART == 1) {
 * 				newstr = newstr replace substr(str, 1, 1)
 * 				str = substr(str, 2)
 * 			} else {
 * 				return newstr str replace
 * 			}
 * 		}
 * 		if (length(str) == 0)
 * 			if (eosflag)
 * 				break
 * 			else
 * 				eosflag = true
 * 	}
 * 	if (length(str) > 0)
 * 		newstr = newstr str	# rest of string
 * 
 * 	return newstr
 * }
 */

/*
 * 1/2004:  The gawk sub/gsub behavior dates from 1996, when we proposed it
 * for POSIX.  The proposal fell through the cracks, and the 2001 POSIX
 * standard chose a more simple behavior.
 *
 * The relevant text is to be found on lines 6394-6407 (pages 166, 167) of the
 * 2001 standard:
 * 
 * sub(ere, repl[, in ])
 *  Substitute the string repl in place of the first instance of the
 *  extended regular expression ERE in string in and return the number of
 *  substitutions. An ampersand ('&') appearing in the string repl shall
 *  be replaced by the string from in that matches the ERE. An ampersand
 *  preceded with a backslash ('\') shall be interpreted as the literal
 *  ampersand character. An occurrence of two consecutive backslashes shall
 *  be interpreted as just a single literal backslash character. Any other
 *  occurrence of a backslash (for example, preceding any other character)
 *  shall be treated as a literal backslash character. Note that if repl is a
 *  string literal (the lexical token STRING; see Grammar (on page 170)), the
 *  handling of the ampersand character occurs after any lexical processing,
 *  including any lexical backslash escape sequence processing. If in is
 *  specified and it is not an lvalue (see Expressions in awk (on page 156)),
 *  the behavior is undefined. If in is omitted, awk shall use the current
 *  record ($0) in its place.
 *
 * 11/2010: The text in the 2008 standard is the same as just quoted.
 * However, POSIX behavior is now the default.  This can change the behavior
 * of awk programs.  The old behavior is not available.
 *
 * 7/2011: Reverted backslash handling to what it used to be. It was in
 * gawk for too long. Should have known better.
 */

/*
 * NB: `howmany' conflicts with a SunOS 4.x macro in <sys/param.h>.
 */

NODE *
do_sub(int nargs, unsigned int flags)
{
	char *scan;
	char *bp, *cp;
	char *buf = NULL;
	size_t buflen;
	char *matchend;
	size_t len;
	char *matchstart;
	char *text;
	size_t textlen = 0;
	char *repl;
	char *replend;
	size_t repllen;
	int sofar;
	int ampersands;
	int matches = 0;
	Regexp *rp;
	NODE *s;		/* subst. pattern */
	NODE *t;		/* string to make sub. in; $0 if none given */
	NODE *tmp;
	NODE **lhs = NULL;
	long how_many = 1;	/* one substitution for sub, also gensub default */
	int global;
	long current;
	bool lastmatchnonzero;
	char *mb_indices = NULL;
	
	if ((flags & GENSUB) != 0) {
		double d;
		NODE *t1;

		tmp = PEEK(3);
		rp = re_update(tmp);

		t = POP_STRING();	/* original string */

		t1 = POP_SCALAR();	/* value of global flag */
		if ((t1->flags & (STRCUR|STRING)) != 0) {
			if (t1->stlen > 0 && (t1->stptr[0] == 'g' || t1->stptr[0] == 'G'))
				how_many = -1;
			else {
				(void) force_number(t1);
				d = get_number_d(t1);
				if ((t1->flags & NUMCUR) != 0)
					goto set_how_many;

				warning(_("gensub: third argument `%.*s' treated as 1"),
						(int) t1->stlen, t1->stptr);
				how_many = 1;
			}
		} else {
			(void) force_number(t1);
			d = get_number_d(t1);
set_how_many:
			if (d < 1)
				how_many = 1;
			else if (d < LONG_MAX)
				how_many = d;
			else
				how_many = LONG_MAX;
			if (d <= 0)
				warning(_("gensub: third argument %g treated as 1"), d);
		}
		DEREF(t1);

	} else {

		/* take care of regexp early, in case re_update is fatal */

		tmp = PEEK(2);
		rp = re_update(tmp);

		if ((flags & GSUB) != 0)
			how_many = -1;

		/* original string */

		if ((flags & LITERAL) != 0)
			t = POP_STRING();
		else {
			lhs = POP_ADDRESS();
			t = force_string(*lhs);
		}
	}

	global = (how_many == -1);

	s = POP_STRING();	/* replacement text */
	decr_sp();		/* regexp, already updated above */

	/* do the search early to avoid work on non-match */
	if (research(rp, t->stptr, 0, t->stlen, RE_NEED_START) == -1 ||
			RESTART(rp, t->stptr) > t->stlen)
		goto done;

	t->flags |= STRING;

	text = t->stptr;
	textlen = t->stlen;

	repl = s->stptr;
	replend = repl + s->stlen;
	repllen = replend - repl;

	ampersands = 0;

	/*
	 * Some systems' malloc() can't handle being called with an
	 * argument of zero.  Thus we have to have some special case
	 * code to check for `repllen == 0'.  This can occur for
	 * something like:
	 * 	sub(/foo/, "", mystring)
	 * for example.
	 */
	if (gawk_mb_cur_max > 1 && repllen > 0) {
		emalloc(mb_indices, char *, repllen * sizeof(char), "do_sub");
		index_multibyte_buffer(repl, mb_indices, repllen);
	}

	for (scan = repl; scan < replend; scan++) {
		if ((gawk_mb_cur_max == 1 || (repllen > 0 && mb_indices[scan - repl] == 1))
		    && (*scan == '&')) {
			repllen--;
			ampersands++;
		} else if (*scan == '\\') {
			if ((flags & GENSUB) != 0) {	/* gensub, behave sanely */
				if (isdigit((unsigned char) scan[1])) {
					ampersands++;
					scan++;
				} else {	/* \q for any q --> q */
					repllen--;
					scan++;
				}
			} else if (do_posix) {
				/* \& --> &, \\ --> \ */
				if (scan[1] == '&' || scan[1] == '\\') {
					repllen--;
					scan++;
				} /* else
					leave alone, it goes into the output */
			} else {
				/* gawk default behavior since 1996 */
				if (strncmp(scan, "\\\\\\&", 4) == 0) {
					/* \\\& --> \& */
					repllen -= 2;
					scan += 3;
				} else if (strncmp(scan, "\\\\&", 3) == 0) {
					/* \\& --> \<string> */
					ampersands++;
					repllen--;
					scan += 2;
				} else if (scan[1] == '&') {
					/* \& --> & */
					repllen--;
					scan++;
				} /* else
					leave alone, it goes into the output */
			}
		}
	}

	lastmatchnonzero = false;

	/* guesstimate how much room to allocate; +2 forces > 0 */
	buflen = textlen + (ampersands + 1) * repllen + 2;
	emalloc(buf, char *, buflen + 2, "do_sub");
	buf[buflen] = '\0';
	buf[buflen + 1] = '\0';

	bp = buf;
	for (current = 1;; current++) {
		matches++;
		matchstart = t->stptr + RESTART(rp, t->stptr);
		matchend = t->stptr + REEND(rp, t->stptr);

		/*
		 * create the result, copying in parts of the original
		 * string 
		 */
		len = matchstart - text + repllen
		      + ampersands * (matchend - matchstart);
		sofar = bp - buf;
		while (buflen < (sofar + len + 1)) {
			buflen *= 2;
			erealloc(buf, char *, buflen, "sub_common");
			bp = buf + sofar;
		}
		for (scan = text; scan < matchstart; scan++)
			*bp++ = *scan;
		if (global || current == how_many) {
			/*
			 * If the current match matched the null string,
			 * and the last match didn't and did a replacement,
			 * and the match of the null string is at the front of
			 * the text (meaning right after end of the previous
			 * replacement), then skip this one.
			 */
			if (matchstart == matchend
			    && lastmatchnonzero
			    && matchstart == text) {
				lastmatchnonzero = false;
				matches--;
				goto empty;
			}
			/*
			 * If replacing all occurrences, or this is the
			 * match we want, copy in the replacement text,
			 * making substitutions as we go.
			 */
			for (scan = repl; scan < replend; scan++)
				if (*scan == '&'
					/*
					 * Don't test repllen here. A simple "&" could
					 * end up with repllen == 0.
					 */
					&& (gawk_mb_cur_max == 1
						|| mb_indices[scan - repl] == 1)
				) {
						for (cp = matchstart; cp < matchend; cp++)
								*bp++ = *cp;
				} else if (*scan == '\\'
					&& (gawk_mb_cur_max == 1
						|| (repllen > 0 && mb_indices[scan - repl] == 1))
				) {
					if (flags & GENSUB) {	/* gensub, behave sanely */
						if (isdigit((unsigned char) scan[1])) {
							int dig = scan[1] - '0';
							if (dig < NUMSUBPATS(rp, t->stptr) && SUBPATSTART(rp, tp->stptr, dig) != -1) {
								char *start, *end;
		
								start = t->stptr
								      + SUBPATSTART(rp, t->stptr, dig);
								end = t->stptr
								      + SUBPATEND(rp, t->stptr, dig);

								for (cp = start; cp < end; cp++)
									*bp++ = *cp;
							}
							scan++;
						} else	/* \q for any q --> q */
							*bp++ = *++scan;
					} else if (do_posix) {
						/* \& --> &, \\ --> \ */
						if (scan[1] == '&' || scan[1] == '\\')
							scan++;
						*bp++ = *scan;
					} else {
						/* gawk default behavior since 1996 */
						if (strncmp(scan, "\\\\\\&", 4) == 0) {
							/* \\\& --> \& */
							*bp++ = '\\';
							*bp++ = '&';
							scan += 3;
						} else if (strncmp(scan, "\\\\&", 3) == 0) {
							/* \\& --> \<string> */
							*bp++ = '\\';
							for (cp = matchstart; cp < matchend; cp++)
								*bp++ = *cp;
							scan += 2;
						} else if (scan[1] == '&') {
							/* \& --> & */
							*bp++ = '&';
							scan++;
						} else
							*bp++ = *scan;
					}
				} else
					*bp++ = *scan;
			if (matchstart != matchend)
				lastmatchnonzero = true;
		} else {
			/*
			 * don't want this match, skip over it by copying
			 * in current text.
			 */
			for (cp = matchstart; cp < matchend; cp++)
				*bp++ = *cp;
		}
	empty:
		/* catch the case of gsub(//, "blah", whatever), i.e. empty regexp */
		if (matchstart == matchend && matchend < text + textlen) {
			*bp++ = *matchend;
			matchend++;
		}
		textlen = text + textlen - matchend;
		text = matchend;

		if ((current >= how_many && ! global)
		    || ((long) textlen <= 0 && matchstart == matchend)
		    || research(rp, t->stptr, text - t->stptr, textlen, RE_NEED_START) == -1)
			break;

	}
	sofar = bp - buf;
	if (buflen - sofar - textlen - 1) {
		buflen = sofar + textlen + 2;
		erealloc(buf, char *, buflen, "do_sub");
		bp = buf + sofar;
	}
	for (scan = matchend; scan < text + textlen; scan++)
		*bp++ = *scan;
	*bp = '\0';
	textlen = bp - buf;

	if (mb_indices != NULL)
		efree(mb_indices);

done:
	DEREF(s);

	if ((matches == 0 || (flags & LITERAL) != 0) && buf != NULL) {
		efree(buf); 
		buf = NULL;
	}

	if (flags & GENSUB) {
		if (matches > 0) {
			/* return the result string */
			DEREF(t);
			assert(buf != NULL);
			return make_str_node(buf, textlen, ALREADY_MALLOCED);	
		}

		/* return the original string */
		return t;
	}

	/* For a string literal, must not change the original string. */
	if ((flags & LITERAL) != 0)
		DEREF(t);
	else if (matches > 0) {
		unref(*lhs);
		*lhs = make_str_node(buf, textlen, ALREADY_MALLOCED);	
	}

	return make_number((AWKNUM) matches);
}

/* call_sub --- call do_sub indirectly */

NODE *
call_sub(const char *name, int nargs)
{
	unsigned int flags = 0;
	NODE *regex, *replace, *glob_flag;
	NODE **lhs, *rhs;
	NODE *zero = make_number(0.0);
	NODE *result;

	if (name[0] == 'g') {
		if (name[1] == 'e')
			flags = GENSUB;
		else
			flags = GSUB;
	}

	if (flags == 0 || flags == GSUB) {
		/* sub or gsub */
		if (nargs != 2)
			fatal(_("%s: can be called indirectly only with two arguments"), name);

		replace = POP_STRING();
		regex = POP();	/* the regex */
		/*
		 * push regex
		 * push replace
		 * push $0
		 */
		regex = make_regnode(Node_regex, regex);
		PUSH(regex);
		PUSH(replace);
		lhs = r_get_field(zero, (Func_ptr *) 0, true);
		nargs++;
		PUSH_ADDRESS(lhs);
	} else {
		/* gensub */
		if (nargs == 4)
			rhs = POP();
		else
			rhs = NULL;
		glob_flag = POP_STRING();
		replace = POP_STRING();
		regex = POP();	/* the regex */
		/*
		 * push regex
		 * push replace
		 * push glob_flag
		 * if (nargs = 3) {
		 *	 push $0
		 *	 nargs++
		 * }
		 */
		regex = make_regnode(Node_regex, regex);
		PUSH(regex);
		PUSH(replace);
		PUSH(glob_flag);
		if (rhs == NULL) {
			lhs = r_get_field(zero, (Func_ptr *) 0, true);
			rhs = *lhs;
			UPREF(rhs);
			PUSH(rhs);
			nargs++;
		}
		else
			PUSH(rhs);
	}

	unref(zero);
	result = do_sub(nargs, flags);
	if (flags != GENSUB)
		reset_record();
	return result;
}

/* call_match --- call do_match indirectly */

NODE *
call_match(int nargs)
{
	NODE *regex, *text, *array;
	NODE *result;

	regex = text = array = NULL;
	if (nargs == 3)
		array = POP();
	regex = POP();

	/* Don't need to pop the string just to push it back ... */

	regex = make_regnode(Node_regex, regex);
	PUSH(regex);

	if (array)
		PUSH(array);

	result = do_match(nargs);
	return result;
}

/* call_split_func --- call do_split or do_pat_split indirectly */

NODE *
call_split_func(const char *name, int nargs)
{
	NODE *regex, *seps;
	NODE *result;

	regex = seps = NULL;
	if (nargs < 2)
		fatal(_("indirect call to %s requires at least two arguments"),
				name);

	if (nargs == 4)
		seps = POP();

	if (nargs >= 3) {
		regex = POP_STRING();
		regex = make_regnode(Node_regex, regex);
	} else {
		if (name[0] == 's') {
			regex = make_regnode(Node_regex, FS_node->var_value);
			regex->re_flags |= FS_DFLT;
		} else
			regex = make_regnode(Node_regex, FPAT_node->var_value);
		nargs++;
	}

	/* Don't need to pop the string or the data array */

	PUSH(regex);

	if (seps)
		PUSH(seps);

	result = (name[0] == 's') ? do_split(nargs) : do_patsplit(nargs);

	return result;
}

/* make_integer - Convert an integer to a number node.  */

static NODE *
make_integer(uintmax_t n)
{
	n = adjust_uint(n);

	return make_number((AWKNUM) n);
}

/* do_lshift --- perform a << operation */

NODE *
do_lshift(int nargs)
{
	NODE *s1, *s2;
	uintmax_t uval, ushift, res;
	AWKNUM val, shift;

	POP_TWO_SCALARS(s1, s2);
	if (do_lint) {
		if ((s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric first argument"));
		if ((s2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric second argument"));
	}
	val = force_number(s1)->numbr;
	shift = force_number(s2)->numbr;
	if (do_lint) {
		if (val < 0 || shift < 0)
			lintwarn(_("lshift(%f, %f): negative values will give strange results"), val, shift);
		if (double_to_int(val) != val || double_to_int(shift) != shift)
			lintwarn(_("lshift(%f, %f): fractional values will be truncated"), val, shift);
		if (shift >= sizeof(uintmax_t) * CHAR_BIT)
			lintwarn(_("lshift(%f, %f): too large shift value will give strange results"), val, shift);
	}

	DEREF(s1);
	DEREF(s2);

	uval = (uintmax_t) val;
	ushift = (uintmax_t) shift;

	res = uval << ushift;
	return make_integer(res);
}

/* do_rshift --- perform a >> operation */

NODE *
do_rshift(int nargs)
{
	NODE *s1, *s2;
	uintmax_t uval, ushift, res;
	AWKNUM val, shift;

	POP_TWO_SCALARS(s1, s2);
	if (do_lint) {
		if ((s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric first argument"));
		if ((s2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric second argument"));
	}
	val = force_number(s1)->numbr;
	shift = force_number(s2)->numbr;
	if (do_lint) {
		if (val < 0 || shift < 0)
			lintwarn(_("rshift(%f, %f): negative values will give strange results"), val, shift);
		if (double_to_int(val) != val || double_to_int(shift) != shift)
			lintwarn(_("rshift(%f, %f): fractional values will be truncated"), val, shift);
		if (shift >= sizeof(uintmax_t) * CHAR_BIT)
			lintwarn(_("rshift(%f, %f): too large shift value will give strange results"), val, shift);
	}

	DEREF(s1);
	DEREF(s2);

	uval = (uintmax_t) val;
	ushift = (uintmax_t) shift;

	res = uval >> ushift;
	return make_integer(res);
}

/* do_and --- perform an & operation */

NODE *
do_and(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKNUM val;
	int i;

	res = ~0;	/* start off with all ones */
	if (nargs < 2)
		fatal(_("and: called with less than two arguments"));

	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("and: argument %d is non-numeric"), i);

		val = force_number(s1)->numbr;
		if (do_lint && val < 0)
			lintwarn(_("and: argument %d negative value %g will give strange results"), i, val);

		uval = (uintmax_t) val;
		res &= uval;

		DEREF(s1);
	}

	return make_integer(res);
}

/* do_or --- perform an | operation */

NODE *
do_or(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKNUM val;
	int i;

	res = 0;
	if (nargs < 2)
		fatal(_("or: called with less than two arguments"));

	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("or: argument %d is non-numeric"), i);

		val = force_number(s1)->numbr;
		if (do_lint && val < 0)
			lintwarn(_("or: argument %d negative value %g will give strange results"), i, val);

		uval = (uintmax_t) val;
		res |= uval;

		DEREF(s1);
	}

	return make_integer(res);
}

/* do_xor --- perform an ^ operation */

NODE *
do_xor(int nargs)
{
	NODE *s1;
	uintmax_t res, uval;
	AWKNUM val;
	int i;

	if (nargs < 2)
		fatal(_("xor: called with less than two arguments"));

	res = 0;	/* silence compiler warning */
	for (i = 1; nargs > 0; nargs--, i++) {
		s1 = POP_SCALAR();
		if (do_lint && (s1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("xor: argument %d is non-numeric"), i);

		val = force_number(s1)->numbr;
		if (do_lint && val < 0)
			lintwarn(_("xor: argument %d negative value %g will give strange results"), i, val);

		uval = (uintmax_t) val;
		if (i == 1)
			res = uval;
		else
			res ^= uval;

		DEREF(s1);
	}

	return make_integer(res);
}

/* do_compl --- perform a ~ operation */

NODE *
do_compl(int nargs)
{
	NODE *tmp;
	double d;
	uintmax_t uval;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("compl: received non-numeric argument"));
	d = force_number(tmp)->numbr;
	DEREF(tmp);

	if (do_lint) {
		if (d < 0)
			lintwarn(_("compl(%f): negative value will give strange results"), d);
		if (double_to_int(d) != d)
			lintwarn(_("compl(%f): fractional value will be truncated"), d);
	}

	uval = (uintmax_t) d;
	uval = ~ uval;
	return make_integer(uval);
}

/* do_strtonum --- the strtonum function */

NODE *
do_strtonum(int nargs)
{
	NODE *tmp;
	AWKNUM d;

	tmp = POP_SCALAR();
	if ((tmp->flags & (NUMBER|NUMCUR)) != 0)
		d = (AWKNUM) force_number(tmp)->numbr;
	else if (get_numbase(tmp->stptr, use_lc_numeric) != 10)
		d = nondec2awknum(tmp->stptr, tmp->stlen);
	else
		d = (AWKNUM) force_number(tmp)->numbr;

	DEREF(tmp);
	return make_number((AWKNUM) d);
}

/* nondec2awknum --- convert octal or hex value to double */

/*
 * Because of awk's concatenation rules and the way awk.y:yylex()
 * collects a number, this routine has to be willing to stop on the
 * first invalid character.
 */

AWKNUM
nondec2awknum(char *str, size_t len)
{
	AWKNUM retval = 0.0;
	char save;
	short val;
	char *start = str;

	if (*str == '0' && (str[1] == 'x' || str[1] == 'X')) {
		/*
		 * User called strtonum("0x") or some such,
		 * so just quit early.
		 */
		if (len <= 2)
			return (AWKNUM) 0.0;

		for (str += 2, len -= 2; len > 0; len--, str++) {
			switch (*str) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				val = *str - '0';
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				val = *str - 'a' + 10;
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				val = *str - 'A' + 10;
				break;
			default:
				goto done;
			}
			retval = (retval * 16) + val;
		}
	} else if (*str == '0') {
		for (; len > 0; len--) {
			if (! isdigit((unsigned char) *str))
				goto done;
			else if (*str == '8' || *str == '9') {
				str = start;
				goto decimal;
			}
			retval = (retval * 8) + (*str - '0');
			str++;
		}
	} else {
decimal:
		save = str[len];
		retval = strtod(str, NULL);
		str[len] = save;
	}
done:
	return retval;
}

/* do_dcgettext, do_dcngettext --- handle i18n translations */

#if ENABLE_NLS && defined(LC_MESSAGES) && HAVE_DCGETTEXT

static int
localecategory_from_argument(NODE *t)
{
	static const struct category_table {
		int val;
		const char *name;
	} cat_tab[] = {
#ifdef LC_ALL
		{ LC_ALL,	"LC_ALL" },
#endif /* LC_ALL */
#ifdef LC_COLLATE
		{ LC_COLLATE,	"LC_COLLATE" },
#endif /* LC_COLLATE */
#ifdef LC_CTYPE
		{ LC_CTYPE,	"LC_CTYPE" },
#endif /* LC_CTYPE */
#ifdef LC_MESSAGES
		{ LC_MESSAGES,	"LC_MESSAGES" },
#endif /* LC_MESSAGES */
#ifdef LC_MONETARY
		{ LC_MONETARY,	"LC_MONETARY" },
#endif /* LC_MONETARY */
#ifdef LC_NUMERIC
		{ LC_NUMERIC,	"LC_NUMERIC" },
#endif /* LC_NUMERIC */
#ifdef LC_RESPONSE
		{ LC_RESPONSE,	"LC_RESPONSE" },
#endif /* LC_RESPONSE */
#ifdef LC_TIME
		{ LC_TIME,	"LC_TIME" },
#endif /* LC_TIME */
	};

	if (t != NULL) {
		int low, high, i, mid;
		char *category;
		int lc_cat = -1;

		category = t->stptr;

		/* binary search the table */
		low = 0;
		high = (sizeof(cat_tab) / sizeof(cat_tab[0])) - 1;
		while (low <= high) {
			mid = (low + high) / 2;
			i = strcmp(category, cat_tab[mid].name);

			if (i < 0)		/* category < mid */
				high = mid - 1;
			else if (i > 0)		/* category > mid */
				low = mid + 1;
			else {
				lc_cat = cat_tab[mid].val;
				break;
			}
		}
		if (lc_cat == -1)	/* not there */
			fatal(_("dcgettext: `%s' is not a valid locale category"), category);

		return lc_cat;
	} else
		return LC_MESSAGES;
}

#endif

/*
 * awk usage is
 *
 * 	str = dcgettext(string [, domain [, category]])
 * 	str = dcngettext(string1, string2, number [, domain [, category]])
 *
 * Default domain is TEXTDOMAIN, default category is LC_MESSAGES.
 */

NODE *
do_dcgettext(int nargs)
{
	NODE *tmp, *t1, *t2 = NULL;
	char *string;
	char *the_result;
#if ENABLE_NLS && defined(LC_MESSAGES) && HAVE_DCGETTEXT
	int lc_cat;
	char *domain;

	if (nargs == 3) {	/* third argument */
		tmp = POP_STRING();
		lc_cat = localecategory_from_argument(tmp);
		DEREF(tmp);
	} else
		lc_cat = LC_MESSAGES;

	if (nargs >= 2) {  /* second argument */
		t2 = POP_STRING();
		domain = t2->stptr;
	} else
		domain = TEXTDOMAIN;
#else
	if (nargs == 3) {
		tmp = POP_STRING();
		DEREF(tmp);
	}
	if (nargs >= 2) {
		t2 = POP_STRING();
		DEREF(t2);
	}
#endif

	t1 = POP_STRING();	/* first argument */
	string = t1->stptr;

#if ENABLE_NLS && defined(LC_MESSAGES) && HAVE_DCGETTEXT
	the_result = dcgettext(domain, string, lc_cat);
	if (t2 != NULL)
		DEREF(t2);
#else
	the_result = string;
#endif
	DEREF(t1);
	return make_string(the_result, strlen(the_result));
}


NODE *
do_dcngettext(int nargs)
{
	NODE *tmp, *t1, *t2, *t3;
	char *string1, *string2;
	unsigned long number;
	AWKNUM d;
	char *the_result;

#if ENABLE_NLS && defined(LC_MESSAGES) && HAVE_DCGETTEXT
	int lc_cat;
	char *domain;

	if (nargs == 5) {	/* fifth argument */
		tmp = POP_STRING();
		lc_cat = localecategory_from_argument(tmp);
		DEREF(tmp);
	} else
		lc_cat = LC_MESSAGES;

	t3 = NULL;
	if (nargs >= 4) {	/* fourth argument */
		t3 = POP_STRING();
		domain = t3->stptr;
	} else
		domain = TEXTDOMAIN;
#else
	if (nargs == 5) {
		tmp = POP_STRING();
		DEREF(tmp);
	}
	if (nargs >= 4) {
		t3 = POP_STRING();
		DEREF(t3);
	}
#endif

	t2 = POP_NUMBER();	/* third argument */
	d = get_number_d(t2);
	DEREF(t2);

	number = (unsigned long) double_to_int(d);
	t2 = POP_STRING();	/* second argument */
	string2 = t2->stptr;
	t1 = POP_STRING();	/* first argument */
	string1 = t1->stptr;

#if ENABLE_NLS && defined(LC_MESSAGES) && HAVE_DCGETTEXT

	the_result = dcngettext(domain, string1, string2, number, lc_cat);
	if (t3 != NULL)
		DEREF(t3);
#else
	the_result = (number == 1 ? string1 : string2);
#endif
	DEREF(t1);
	DEREF(t2);
	return make_string(the_result, strlen(the_result));
}

/* do_bindtextdomain --- set the directory for a text domain */

/*
 * awk usage is
 *
 * 	binding = bindtextdomain(dir [, domain])
 *
 * If dir is "", pass NULL to C version.
 * Default domain is TEXTDOMAIN.
 */

NODE *
do_bindtextdomain(int nargs)
{
	NODE *t1, *t2;
	const char *directory, *domain;
	const char *the_result;

	t1 = t2 = NULL;
	/* set defaults */
	directory = NULL;
	domain = TEXTDOMAIN;

	if (nargs == 2) {	/* second argument */
		t2 = POP_STRING();
		domain = (const char *) t2->stptr;
	}

	/* first argument */
	t1 = POP_STRING();
	if (t1->stlen > 0)
		directory = (const char *) t1->stptr;

	the_result = bindtextdomain(domain, directory);

	DEREF(t1);
	if (t2 != NULL)
		DEREF(t2);

	return make_string(the_result, strlen(the_result));
}


/* mbc_byte_count --- return number of bytes for corresponding numchars multibyte characters */

static size_t
mbc_byte_count(const char *ptr, size_t numchars)
{
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
}

/* mbc_char_count --- return number of m.b. chars in string, up to numbytes bytes */

static size_t
mbc_char_count(const char *ptr, size_t numbytes)
{
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
}
