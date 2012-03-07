/*
 * mpfr.c - routines for MPFR number support in gawk.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2012 the Free Software Foundation, Inc.
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

#ifdef HAVE_MPFR

#if !defined(__GNU_MP_VERSION) ||  __GNU_MP_VERSION < 5                 
typedef unsigned long int mp_bitcnt_t;
#endif

#if !defined(MPFR_VERSION_MAJOR) || MPFR_VERSION_MAJOR < 3
typedef mp_exp_t mpfr_exp_t;
#endif

extern NODE **fmt_list;          /* declared in eval.c */

mpz_t mpzval;	/* GMP integer type; used as temporary in many places */
mpfr_t MNR;
mpfr_t MFNR;
int do_ieee_fmt;	/* IEEE-754 floating-point emulation */

static mpfr_rnd_t get_rnd_mode(const char rmode);
static NODE *get_bit_ops(NODE **p1, NODE **p2, const char *op);
static NODE *mpg_force_number(NODE *n);
static NODE *mpg_make_number(double);
static NODE *mpg_format_val(const char *format, int index, NODE *s);
static int mpg_interpret(INSTRUCTION **cp);
static mpfr_exp_t min_exp = MPFR_EMIN_DEFAULT;
static mpfr_exp_t max_exp = MPFR_EMAX_DEFAULT;


/* init_mpfr --- set up MPFR related variables */

void
init_mpfr(const char *rmode)
{
	mpfr_set_default_prec(PRECISION);
	RND_MODE = get_rnd_mode(rmode[0]);
	mpfr_set_default_rounding_mode(RND_MODE);
	make_number = mpg_make_number;
	str2number = mpg_force_number;
	format_val = mpg_format_val;
	mpz_init(mpzval);
	mpfr_init(MNR);
	mpfr_set_d(MNR, 0.0, RND_MODE);
	mpfr_init(MFNR);
	mpfr_set_d(MFNR, 0.0, RND_MODE);
	do_ieee_fmt = FALSE;
	register_exec_hook(mpg_interpret, 0);
}

/* mpg_node --- allocate a node to store a MPFR number */

NODE *
mpg_node()
{
	NODE *r;
	getnode(r);
	r->type = Node_val;

	/* Initialize, set precision to the default precision, and value to NaN */
	mpfr_init(r->mpg_numbr);

	r->valref = 1;
	r->flags = MALLOC|MPFN|NUMBER|NUMCUR;
	r->stptr = NULL;
	r->stlen = 0;
#if MBS_SUPPORT
	r->wstptr = NULL;
	r->wstlen = 0;
#endif /* defined MBS_SUPPORT */
	return r;
}

/* mpg_make_number --- make a MPFR number node and initialize with a double */

static NODE *
mpg_make_number(double x)
{
	NODE *r;
	int tval;

	r = mpg_node();
	tval = mpfr_set_d(r->mpg_numbr, x, RND_MODE);
	IEEE_FMT(r->mpg_numbr, tval);
	return r;
}

/* mpg_force_number --- force a value to be a MPFR number */

static NODE *
mpg_force_number(NODE *n)
{
	char *cp, *cpend, *ptr;
	char save;
	int base = 10;
	unsigned int newflags = 0;
	int tval;

	if ((n->flags & (MPFN|NUMCUR)) == (MPFN|NUMCUR))
		return n;

	if (n->flags & MAYBE_NUM) {
		n->flags &= ~MAYBE_NUM;
		newflags = NUMBER;
	}

	if ((n->flags & MPFN) == 0) {
		n->flags |= MPFN;
		mpfr_init(n->mpg_numbr);
	}
	mpfr_set_d(n->mpg_numbr, 0.0, RND_MODE);

	if (n->stlen == 0)
		return n;

	cp = n->stptr;
	cpend = n->stptr + n->stlen;
	while (cp < cpend && isspace((unsigned char) *cp))
		cp++;
	if (cp == cpend)	/* only spaces */
		return n;
	
	save = *cpend;
	*cpend = '\0';

	if (do_non_decimal_data && ! do_traditional)
		base = get_numbase(cp, TRUE);

	errno = 0;
	tval = mpfr_strtofr(n->mpg_numbr, cp, & ptr, base, RND_MODE);
	IEEE_FMT(n->mpg_numbr, tval);

	/* trailing space is OK for NUMBER */
	while (isspace((unsigned char) *ptr))
		ptr++;
	*cpend = save;
	if (errno == 0 && ptr == cpend) {
		n->flags |= newflags;
		n->flags |= NUMCUR;
	}
	errno = 0;
	return n;
}


/* mpg_format_val --- format a numeric value based on format */

static NODE *
mpg_format_val(const char *format, int index, NODE *s)
{
	NODE *dummy[2], *r;
	unsigned int oflags;

	/* create dummy node for a sole use of format_tree */
	dummy[1] = s;
	oflags = s->flags;

	if (mpfr_integer_p(s->mpg_numbr)) {
		/* integral value, use %d */
		r = format_tree("%d", 2, dummy, 2);
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
 
	s->flags |= STRCUR;
	free_wstr(s);
	return s;
}


/*
 * mpg_update_var --- update NR or FNR. 
 *	NR_node->var_value(mpfr_t) = MNR(mpfr_t) * LONG_MAX + NR(long) 
 */

void
mpg_update_var(NODE *n)
{
	NODE *val = n->var_value;
	long nl;
	mpfr_ptr nm;

	if (n == NR_node) {
		nl = NR;
		nm = MNR;
	} else if (n == FNR_node) {
		nl = FNR;
		nm = MFNR;
	} else
		cant_happen();

	if (mpfr_zero_p(nm)) {
		double d;

		/* Efficiency hack for NR < LONG_MAX */
		d = mpfr_get_d(val->mpg_numbr, RND_MODE);
		if (d != nl) {
			unref(n->var_value);
			n->var_value = make_number(nl);
		}
	} else {
		unref(n->var_value);
		val = n->var_value = mpg_node();
		mpfr_mul_si(val->mpg_numbr, nm, LONG_MAX, RND_MODE);
		mpfr_add_si(val->mpg_numbr, val->mpg_numbr, nl, RND_MODE);
	}
}


/* mpg_set_var --- set NR or FNR */

long
mpg_set_var(NODE *n)
{
	long l;
	mpfr_ptr nm;
	mpfr_ptr p = n->var_value->mpg_numbr;
	int neg = FALSE;

	if (n == NR_node)
		nm = MNR;
	else if (n == FNR_node)
		nm = MFNR;
	else
		cant_happen();

	mpfr_get_z(mpzval, p, MPFR_RNDZ);
	if (mpfr_signbit(p)) {
		/* It is a negative number ! */
		neg = TRUE;
		mpz_neg(mpzval, mpzval);
	}		
	l = mpz_fdiv_q_ui(mpzval, mpzval, LONG_MAX);
	if (neg) {
		mpz_neg(mpzval, mpzval);
		l = -l;
	}
	
	mpfr_set_z(nm, mpzval, RND_MODE);	/* quotient (MNR) */
	return l;	/* remainder (NR) */
}


/* set_PREC --- update MPFR PRECISION related variables when PREC assigned to */

void
set_PREC()
{
	long prec = 0;
	NODE *val;
	static const struct ieee_fmt {
		const char *name;
		mpfr_prec_t precision;
		mpfr_exp_t emax;
		mpfr_exp_t emin;
	} ieee_fmts[] = {
{ "half",	11,	16,	-23	},	/* binary16 */
{ "single",	24,	128,	-148	},	/* binary32 */
{ "double",	53,	1024,	-1073	},	/* binary64 */
{ "quad",	113,	16384,	-16493	},	/* binary128 */
{ "oct",	237,	262144,	-262377	},	/* binary256, not in the IEEE 754-2008 standard */

		/*
 		 * For any bitwidth = 32 * k ( k >= 4),
 		 * precision = 13 + bitwidth - int(4 * log2(bitwidth))
		 * emax = 1 << bitwidth - precision - 1
		 * emin = 4 - emax - precision
 		 */
	};

	if (! do_mpfr)
		return;

	val = PREC_node->var_value;
	if (val->flags & MAYBE_NUM)
		force_number(val);

	if ((val->flags & (STRING|NUMBER)) == STRING) {
		int i, j;

		/* emulate IEEE-754 binary format */

		for (i = 0, j = sizeof(ieee_fmts)/sizeof(ieee_fmts[0]); i < j; i++) {
			if (strcasecmp(ieee_fmts[i].name, val->stptr) == 0)
				break;
		}

		if (i < j) {
			prec = ieee_fmts[i].precision;

			/*
			 * We *DO NOT* change the MPFR exponent range using
			 * mpfr_set_{emin, emax} here. See format_ieee() for details.
			 */
			max_exp = ieee_fmts[i].emax;
			min_exp = ieee_fmts[i].emin;

			do_ieee_fmt = TRUE;
		}
	}

	if (prec <= 0) {
		force_number(val);
		prec = get_number_si(val);		
		if (prec < MPFR_PREC_MIN || prec > MPFR_PREC_MAX) {
			force_string(val);
			warning(_("PREC value `%.*s' is invalid"), (int) val->stlen, val->stptr);
			prec = 0;
		} else
			do_ieee_fmt = FALSE;
	}

	if (prec > 0) {
		mpfr_set_default_prec(prec);
		PRECISION = mpfr_get_default_prec();
	}
}


/* get_rnd_mode --- convert string to MPFR rounding mode */

static mpfr_rnd_t
get_rnd_mode(const char rmode)
{
	switch (rmode) {
	case 'N':
	case 'n':
		return MPFR_RNDN;	/* round to nearest */
	case 'Z':
	case 'z':
		return MPFR_RNDZ;	/* round toward zero */
	case 'U':
	case 'u':
		return MPFR_RNDU;	/* round toward plus infinity */
	case 'D':
	case 'd':
		return MPFR_RNDD;	/* round toward minus infinity */
#if defined(MPFR_VERSION_MAJOR) && MPFR_VERSION_MAJOR > 2
	case 'A':
	case 'a':
		return MPFR_RNDA;	/* round away from zero */
#endif
	default:
		break;
	}
	return -1;
}

/* set_RNDMODE --- update MPFR rounding mode related variables when RNDMODE assigned to */

void
set_RNDMODE()
{
	if (do_mpfr) {
		mpfr_rnd_t rnd = -1;
		NODE *n;
		n = force_string(RNDMODE_node->var_value);
		if (n->stlen == 1)
			rnd = get_rnd_mode(n->stptr[0]);
		if (rnd != -1) {
			mpfr_set_default_rounding_mode(rnd);
			RND_MODE = rnd;
		} else
			warning(_("RNDMODE value `%.*s' is invalid"), (int) n->stlen, n->stptr);
	}
}


/* format_ieee --- make sure a number follows IEEE-754 floating-point standard */

int
format_ieee(mpfr_ptr x, int tval)
{
	/*
	 * The MPFR doc says that it's our responsibility to make sure all numbers
	 * including those previously created are in range after we've changed the
	 * exponent range. Most MPFR operations and functions  requires
	 * the input arguments to have exponents within the current exponent range.
	 * Any argument outside the range results in a MPFR assertion failure
	 * like this:
	 *
	 *   $] gawk -M 'BEGIN { x=1.0e-10000; print x+0; PREC="double"; print x+0}'
	 *   1e-10000
	 *   init2.c:52: MPFR assertion failed ....
	 *
	 * A "naive" approach would be to keep track of the ternary state and
	 * the rounding mode for each number, and make sure it is in the current
	 * exponent range (using mpfr_check_range) before using it in an
	 * operation or function. Instead, we adopt the following strategy.
	 *
	 * When gawk starts, the exponent range is the MPFR default
	 * [MPFR_EMIN_DEFAULT, MPFR_EMAX_DEFAULT]. Any number that gawk
	 * creates must have exponent in this range (excluding infinities, NANs and zeros).
	 * Each MPFR operation or function is performed with this default exponent
	 * range.
	 *
	 * When emulating IEEE-754 format, the exponents are *temporarily* changed,
	 * mpfr_check_range is called to make sure the number is in the new range,
	 * and mpfr_subnormalize is used to round following the rules of subnormal
	 * arithmetic. The exponent range is then *restored* to the original value
	 * [MPFR_EMIN_DEFAULT, MPFR_EMAX_DEFAULT].
	 */

	(void) mpfr_set_emin(min_exp);
	(void) mpfr_set_emax(max_exp);
	tval = mpfr_check_range(x, tval, RND_MODE);
	tval = mpfr_subnormalize(x, tval, RND_MODE);
	(void) mpfr_set_emin(MPFR_EMIN_DEFAULT);
	(void) mpfr_set_emax(MPFR_EMAX_DEFAULT);
	return tval;
}


/* get_bit_ops --- get the numeric operands of a binary function */

static NODE *
get_bit_ops(NODE **p1, NODE **p2, const char *op)
{
	NODE *t1, *t2;
	mpfr_ptr left, right;

	*p2 = t2 = POP_SCALAR();
	*p1 = t1 = POP_SCALAR();

	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("%s: received non-numeric first argument"), op);
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("%s: received non-numeric second argument"), op);
	}

	left = force_number(t1)->mpg_numbr;
	right = force_number(t2)->mpg_numbr;

	if (! mpfr_number_p(left)) {
		/* [+-]inf or NaN */
		DEREF(t2);
		return t1;
	}

	if (! mpfr_number_p(right)) {
		/* [+-]inf or NaN */
		DEREF(t1);
		return t2;
	}

	if (do_lint) {
		if (mpfr_signbit(left) || mpfr_signbit(right))
			lintwarn("%s",
		mpg_fmt(_("%s(%Rg, %Rg): negative values will give strange results"),
					op, left, right)
				);
		if (! mpfr_integer_p(left) || ! mpfr_integer_p(right))
			lintwarn("%s",
		mpg_fmt(_("%s(%Rg, %Rg): fractional values will be truncated"),
					op, left, right)
				);
	}
	return NULL;
}


/* do_mpfr_and --- perform an & operation */

NODE *
do_mpfr_and(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "and")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpg_numbr, MPFR_RNDZ);	/* float to integer conversion */
	mpfr_get_z(z, t2->mpg_numbr, MPFR_RNDZ);	/* Same */	
	mpz_and(z, mpzval, z);

	res = mpg_node();
	mpfr_set_z(res->mpg_numbr, z, RND_MODE);	/* integer to float conversion */
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}

/* do_mpfr_atan2 --- do the atan2 function */

NODE *
do_mpfr_atan2(int nargs)
{
	NODE *t1, *t2, *res;
	int tval;

	t2 = POP_SCALAR();
	t1 = POP_SCALAR();

	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric first argument"));
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("atan2: received non-numeric second argument"));
	}
	force_number(t1);
	force_number(t2);

	res = mpg_node();
	/* See MPFR documentation for handling of special values like +inf as an argument */ 
	tval = mpfr_atan2(res->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
	IEEE_FMT(res->mpg_numbr, tval);

	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_mpfr_compl --- perform a ~ operation */

NODE *
do_mpfr_compl(int nargs)
{
	NODE *tmp, *r;
	mpfr_ptr p;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("compl: received non-numeric argument"));

	p = force_number(tmp)->mpg_numbr;
	if (! mpfr_number_p(p)) {
		/* [+-]inf or NaN */
		return tmp;
	}

	if (do_lint) {
		if (mpfr_signbit(p))
			lintwarn("%s",
		mpg_fmt(_("compl(%Rg): negative value will give strange results"), p)
				);
		if (! mpfr_integer_p(p))
			lintwarn("%s",
		mpg_fmt(_("comp(%Rg): fractional value will be truncated"), p)
				);
	}
	mpfr_get_z(mpzval, p, MPFR_RNDZ);
	mpz_com(mpzval, mpzval);
	r = mpg_node();
	mpfr_set_z(r->mpg_numbr, mpzval, RND_MODE);
	DEREF(tmp);
	return r;
}

#define SPEC_MATH(X)	\
NODE *tmp, *res; \
int tval; \
tmp = POP_SCALAR(); \
if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0) \
	lintwarn(_("%s: received non-numeric argument"), #X); \
force_number(tmp); \
res = mpg_node(); \
tval = mpfr_##X(res->mpg_numbr, tmp->mpg_numbr, RND_MODE); \
IEEE_FMT(res->mpg_numbr, tval); \
DEREF(tmp); \
return res


/* do_mpfr_sin --- do the sin function */

NODE *
do_mpfr_sin(int nargs)
{
	SPEC_MATH(sin);
}

/* do_mpfr_cos --- do the cos function */

NODE *
do_mpfr_cos(int nargs)
{
	SPEC_MATH(cos);
}

/* do_mpfr_exp --- exponential function */

NODE *
do_mpfr_exp(int nargs)
{
	SPEC_MATH(exp);
}

/* do_mpfr_log --- the log function */

NODE *
do_mpfr_log(int nargs)
{
	SPEC_MATH(log);
}

/* do_mpfr_sqrt --- do the sqrt function */

NODE *
do_mpfr_sqrt(int nargs)
{
	SPEC_MATH(sqrt);
}

/* do_mpfr_int --- convert double to int for awk */

NODE *
do_mpfr_int(int nargs)
{
	NODE *tmp, *r;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("int: received non-numeric argument"));
	force_number(tmp);
	if (! mpfr_number_p(tmp->mpg_numbr)) {
		/* [+-]inf or NaN */
		return tmp;
	}
	mpfr_get_z(mpzval, tmp->mpg_numbr, MPFR_RNDZ);
	r = mpg_node();
	mpfr_set_z(r->mpg_numbr, mpzval, RND_MODE);
	DEREF(tmp);
	return r;
}


/* do_mpfr_lshift --- perform a << operation */

NODE *
do_mpfr_lshift(int nargs)
{
	NODE *t1, *t2, *res;
	mp_bitcnt_t shift;

	if ((res = get_bit_ops(& t1, & t2, "lshift")) != NULL)
		return res;

	mpfr_get_z(mpzval, t1->mpg_numbr, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(t2->mpg_numbr, MPFR_RNDZ);	/* mpfr_t (float) => unsigned long conversion */
	mpz_mul_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval * 2^shift */

	res = mpg_node();
	mpfr_set_z(res->mpg_numbr, mpzval, RND_MODE);	/* integer to float conversion */
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_mpfr_or --- perform an | operation */

NODE *
do_mpfr_or(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "or")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpg_numbr, MPFR_RNDZ);
	mpfr_get_z(z, t2->mpg_numbr, MPFR_RNDZ);	
	mpz_ior(z, mpzval, z);

	res = mpg_node();
	mpfr_set_z(res->mpg_numbr, z, RND_MODE);
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_mpfr_rshift --- perform a >> operation */

NODE *
do_mpfr_rhift(int nargs)
{
	NODE *t1, *t2, *res;
	mp_bitcnt_t shift;

	if ((res = get_bit_ops(& t1, & t2, "rshift")) != NULL)
		return res;

	mpfr_get_z(mpzval, t1->mpg_numbr, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(t2->mpg_numbr, MPFR_RNDZ);	/* mpfr_t (float) => unsigned long conversion */
	mpz_fdiv_q_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval / 2^shift, round towards −inf */

	res = mpg_node();
	mpfr_set_z(res->mpg_numbr, mpzval, RND_MODE);	/* integer to float conversion */
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_mpfr_strtonum --- the strtonum function */

NODE *
do_mpfr_strtonum(int nargs)
{
	NODE *tmp, *r;
	int base, tval;

	tmp = POP_SCALAR();
	r = mpg_node();
	if ((tmp->flags & (NUMBER|NUMCUR)) != 0)
		tval = mpfr_set(r->mpg_numbr, tmp->mpg_numbr, RND_MODE);
	else if ((base = get_numbase(tmp->stptr, use_lc_numeric)) != 10) {
		tval = mpfr_strtofr(r->mpg_numbr, tmp->stptr, NULL, base, RND_MODE);
		errno = 0;
	} else {
		(void) force_number(tmp);
		tval = mpfr_set(r->mpg_numbr, tmp->mpg_numbr, RND_MODE);
	}

	IEEE_FMT(r->mpg_numbr, tval);
	DEREF(tmp);
	return r;
}


/* do_mpfr_xor --- perform an ^ operation */

NODE *
do_mpfr_xor(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "xor")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpg_numbr, MPFR_RNDZ);
	mpfr_get_z(z, t2->mpg_numbr, MPFR_RNDZ);	
	mpz_xor(z, mpzval, z);

	res = mpg_node();
	mpfr_set_z(res->mpg_numbr, z, RND_MODE);
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}


static int firstrand = TRUE;
static gmp_randstate_t state;
static mpz_t seed;	/* current seed */

/* do_mpfr_rand --- do the rand function */

NODE *
do_mpfr_rand(int nargs ATTRIBUTE_UNUSED)
{
	NODE *res;
	int tval;

	if (firstrand) {
		/* Choose the default algorithm */
		gmp_randinit_default(state);
		mpz_init(seed);
		mpz_set_ui(seed, 1L);
		/* seed state */
		gmp_randseed(state, seed);
		firstrand = FALSE;
	}
	res = mpg_node();
	tval = mpfr_urandomb(res->mpg_numbr, state);
	IEEE_FMT(res->mpg_numbr, tval);
	return res;
}


/* do_mpfr_srand --- seed the random number generator */

NODE *
do_mpfr_srand(int nargs)
{
	NODE *res;
	int tval;

	if (firstrand) {
		/* Choose the default algorithm */
		gmp_randinit_default(state);
		mpz_init(seed);
		mpz_set_ui(seed, 1L);
		/* No need to seed state, will change it below */
		firstrand = FALSE;
	}

	res = mpg_node();
	tval = mpfr_set_z(res->mpg_numbr, seed, RND_MODE);	/* previous seed */
	IEEE_FMT(res->mpg_numbr, tval);

	if (nargs == 0)
		mpz_set_ui(seed, (unsigned long) time((time_t *) 0));
	else {
		NODE *tmp;
		tmp = POP_SCALAR();
		if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("srand: received non-numeric argument"));
		force_number(tmp);
		mpfr_get_z(seed, tmp->mpg_numbr, MPFR_RNDZ);
		DEREF(tmp);
	}

	gmp_randseed(state, seed);
	return res;
}

/*
 * mpg_interpret --- pre-exec hook in the interpreter. Handles
 *	arithmetic operations with MPFR numbers.
 */ 

static int
mpg_interpret(INSTRUCTION **cp)
{
	INSTRUCTION *pc = *cp;	/* current instruction */
	OPCODE op;	/* current opcode */
	NODE *r = NULL;
	NODE *t1, *t2;
	NODE **lhs;
	AWKNUM x;
	mpfr_ptr p1, p2;
	int tval;

	switch ((op = pc->opcode)) {
	case Op_plus_i:
		t2 = force_number(pc->memory);
		goto plus;
	case Op_plus:
		t2 = POP_NUMBER();
plus:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_add(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_plus)
			DEREF(t2);
		REPLACE(r);
		break;

	case Op_minus_i:
		t2 = force_number(pc->memory);
		goto minus;
	case Op_minus:
		t2 = POP_NUMBER();
minus:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_sub(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_minus)
			DEREF(t2);
		REPLACE(r);
		break;

	case Op_times_i:
		t2 = force_number(pc->memory);
		goto times;
	case Op_times:
		t2 = POP_NUMBER();
times:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_mul(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_times)
			DEREF(t2);
		REPLACE(r);
		break;

	case Op_exp_i:
		t2 = force_number(pc->memory);
		goto exp;
	case Op_exp:
		t2 = POP_NUMBER();
exp:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_pow(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_exp)
			DEREF(t2);
		REPLACE(r);
		break;

	case Op_quotient_i:
		t2 = force_number(pc->memory);
		goto quotient;
	case Op_quotient:
		t2 = POP_NUMBER();
quotient:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_div(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_quotient)
			DEREF(t2);
		REPLACE(r);
		break;		

	case Op_mod_i:
		t2 = force_number(pc->memory);
		goto mod;
	case Op_mod:
		t2 = POP_NUMBER();
mod:
		t1 = TOP_NUMBER();
		r = mpg_node();
		tval = mpfr_fmod(r->mpg_numbr, t1->mpg_numbr, t2->mpg_numbr, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		if (op == Op_mod)
			DEREF(t2);
		REPLACE(r);
		break;

	case Op_preincrement:
	case Op_predecrement:
		x = op == Op_preincrement ? 1.0 : -1.0;
		lhs = TOP_ADDRESS();
		t1 = *lhs;
		force_number(t1);

#if 0
		/*
		 * The optimizations for fixed precision do not always
		 * work the same way in arbitrary precision. With this optimization on,
		 *   gawk -M 'BEGIN { PREC=53; i=2^53; PREC=113; ++i; print i}'
		 * prints 2^53 instead of 2^53+1.
		 */

		if (t1->valref == 1 && t1->flags == (MALLOC|MPFN|NUMCUR|NUMBER)) {
			/* optimization */
			tval = mpfr_add_d(t1->mpg_numbr, t1->mpg_numbr, x, RND_MODE);
			IEEE_FMT(t1->mpg_numbr, tval);
			r = t1;
		}
#endif
		r = *lhs = mpg_node();
		tval = mpfr_add_d(r->mpg_numbr, t1->mpg_numbr, x, RND_MODE);
		IEEE_FMT(r->mpg_numbr, tval);
		unref(t1);
		UPREF(r);
		REPLACE(r);
		break;

	case Op_postincrement:
	case Op_postdecrement:
		x = op == Op_postincrement ? 1.0 : -1.0;
		lhs = TOP_ADDRESS();
		t1 = *lhs;
		force_number(t1);
		r = mpg_node();
		tval = mpfr_set(r->mpg_numbr, t1->mpg_numbr, RND_MODE);	/* r = t1 */
		IEEE_FMT(r->mpg_numbr, tval);
		t2 = *lhs = mpg_node();
		tval = mpfr_add_d(t2->mpg_numbr, t1->mpg_numbr, x, RND_MODE);
		IEEE_FMT(t2->mpg_numbr, tval);
		unref(t1);
		REPLACE(r);
		break;

	case Op_unary_minus:
		t1 = TOP_NUMBER();
		r = mpg_node();
		mpfr_set(r->mpg_numbr, t1->mpg_numbr, RND_MODE);	/* r = t1 */
		tval = mpfr_neg(r->mpg_numbr, r->mpg_numbr, RND_MODE);	/* change sign */
		IEEE_FMT(r->mpg_numbr, tval);
		DEREF(t1);
		REPLACE(r);
		break;

	case Op_assign_plus:
	case Op_assign_minus:
	case Op_assign_times:
	case Op_assign_quotient:
	case Op_assign_mod:
	case Op_assign_exp:
		lhs = POP_ADDRESS();
		t1 = *lhs;
		p1 = force_number(t1)->mpg_numbr;

		t2 = TOP_NUMBER();
		p2 = t2->mpg_numbr;

		r = mpg_node();
		switch (op) {
		case Op_assign_plus:
			tval = mpfr_add(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		case Op_assign_minus:
			tval = mpfr_sub(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		case Op_assign_times:
			tval = mpfr_mul(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		case Op_assign_quotient:
			tval = mpfr_div(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		case Op_assign_mod:
			tval = mpfr_fmod(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		case Op_assign_exp:
			tval = mpfr_pow(r->mpg_numbr, p1, p2, RND_MODE);
			break;
		default:
			cant_happen();
		}

		IEEE_FMT(r->mpg_numbr, tval);

		DEREF(t2);
		unref(*lhs);
		*lhs = r;

		UPREF(r);
		REPLACE(r);
		break;

	default:
		return TRUE;	/* unhandled */
	}

	*cp = pc->nexti;	/* next instruction to execute */
	return FALSE;
}


/* mpg_fmt --- output formatted string with special MPFR/GMP conversion specifiers */

const char *
mpg_fmt(const char *mesg, ...)
{
	static char *tmp = NULL;
	int ret;
	va_list args;

	if (tmp != NULL) {
		mpfr_free_str(tmp);
		tmp = NULL;
	}
	va_start(args, mesg);
	ret = mpfr_vasprintf(& tmp, mesg, args);
	va_end(args);
	if (ret >= 0 && tmp != NULL)
		return tmp;
	return mesg;
}

#else

void
set_PREC()
{
	/* dummy function */
}

void
set_RNDMODE()
{
	/* dummy function */
}

#endif
