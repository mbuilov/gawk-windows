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

#ifndef HAVE_MPFR

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

#else

#ifndef mp_bitcnt_t
#define	mp_bitcnt_t unsigned long
#endif

extern NODE **fmt_list;          /* declared in eval.c */

mpz_t mpzval;	/* GMP integer type; used as temporary in many places */
mpfr_t MNR;
mpfr_t MFNR;

static mpfr_rnd_t mpfr_rnd_mode(const char *mode, size_t mode_len);
static NODE *get_bit_ops(NODE **p1, NODE **p2, const char *op);
static NODE *mpfr_force_number(NODE *n);
static NODE *mpfr_make_number(double);
static NODE *mpfr_format_val(const char *format, int index, NODE *s);


/* init_mpfr --- set up MPFR related variables */

void
init_mpfr(const char *rnd_mode)
{
	mpfr_set_default_prec(PRECISION);
	RND_MODE = mpfr_rnd_mode(rnd_mode, strlen(rnd_mode));
	mpfr_set_default_rounding_mode(RND_MODE);
	make_number = mpfr_make_number;
	m_force_number = mpfr_force_number;
	format_val = mpfr_format_val;
	mpz_init(mpzval);
	mpfr_init(MNR);
	mpfr_set_d(MNR, 0.0, RND_MODE);
	mpfr_init(MFNR);
	mpfr_set_d(MFNR, 0.0, RND_MODE);
}

/* mpfr_node --- allocate a node to store a MPFR number */

NODE *
mpfr_node()
{
	NODE *r;
	getnode(r);
	r->type = Node_val;

	/* Initialize, set precision to the default precision, and value to NaN */
	mpfr_init(r->mpfr_numbr);

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

/* mpfr_make_number --- make a MPFR number node and initialize with a double */

static NODE *
mpfr_make_number(double x)
{
	NODE *r;
	r = mpfr_node();
	mpfr_set_d(r->mpfr_numbr, x, RND_MODE);
	return r;
}

/* mpfr_force_number --- force a value to be a MPFR number */

static NODE *
mpfr_force_number(NODE *n)
{
	char *cp, *cpend, *ptr;
	char save;
	int base = 10;
	unsigned int newflags = 0;

	if ((n->flags & (MPFN|NUMCUR)) == (MPFN|NUMCUR))
		return n;

	if (n->flags & MAYBE_NUM) {
		n->flags &= ~MAYBE_NUM;
		newflags = NUMBER;
	}

	if ((n->flags & MPFN) == 0) {
		n->flags |= MPFN;
		mpfr_init(n->mpfr_numbr);
	}
	mpfr_set_d(n->mpfr_numbr, 0.0, RND_MODE);

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
	(void) mpfr_strtofr(n->mpfr_numbr, cp, & ptr, base, RND_MODE);

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


/* mpfr_format_val --- format a numeric value based on format */

static NODE *
mpfr_format_val(const char *format, int index, NODE *s)
{
	NODE *dummy[2], *r;
	unsigned int oflags;

	/* create dummy node for a sole use of format_tree */
	dummy[1] = s;
	oflags = s->flags;

	if (mpfr_integer_p(s->mpfr_numbr)) {
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
 * mpfr_update_var --- update NR or FNR. 
 *	NR_node(mpfr_t)  = MNR(mpfr_t) * LONG_MAX + NR(long) 
 */

/*
 * Test:
 * $ ./gawk -M 'BEGIN{NR=0x7FFFFFFFL; print NR} END{ print NR, NR-0x7FFFFFFFL, FNR}' awk.h
 */

void
mpfr_update_var(NODE *n)
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
		d = mpfr_get_d(val->mpfr_numbr, RND_MODE);
		if (d != nl) {
			unref(n->var_value);
			n->var_value = make_number((AWKNUM) nl);
		}
	} else {
		unref(n->var_value);
		val = n->var_value = mpfr_node();
		mpfr_mul_si(val->mpfr_numbr, nm, LONG_MAX, RND_MODE);
		mpfr_add_si(val->mpfr_numbr, val->mpfr_numbr, nl, RND_MODE);
	}
}


/* mpfr_set_var --- set NR or FNR */

long
mpfr_set_var(NODE *n)
{
	long l;
	mpfr_ptr nm;
	mpfr_ptr p = n->var_value->mpfr_numbr;
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
	/* TODO: "DOUBLE", "QUAD", "OCT", .. */

	if (do_mpfr) {
		long l;
		NODE *val = PREC_node->var_value;

		(void) force_number(val);
		l = get_number_si(val);		

		if (l >= MPFR_PREC_MIN && l <= MPFR_PREC_MAX) {
			mpfr_set_default_prec(l);
			PRECISION = mpfr_get_default_prec();
		} else
			warning(_("Invalid PREC value: %ld"), l);
	}
}

/* mpfr_rnd_mode --- convert string to MPFR rounding mode */

static mpfr_rnd_t
mpfr_rnd_mode(const char *mode, size_t mode_len)
{
	if (mode_len != 4 || strncmp(mode, "RND", 3) != 0)
		return -1;

	switch (mode[3]) {
	case 'N':
		return MPFR_RNDN;
	case 'Z':
		return MPFR_RNDZ;
	case 'U':
		return MPFR_RNDU;
	case 'D':
		return MPFR_RNDD;
#ifdef MPFR_RNDA
	case 'A':
		return MPFR_RNDA;
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
		mpfr_rnd_t rnd;
		NODE *n;
		n = force_string( RNDMODE_node->var_value);
		rnd = mpfr_rnd_mode(n->stptr, n->stlen);
		if (rnd != -1) {
			mpfr_set_default_rounding_mode(rnd);
			RND_MODE = rnd;
		} else
			warning(_("Invalid value for RNDMODE: `%s'"), n->stptr);
	}
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

	left = force_number(t1)->mpfr_numbr;
	right = force_number(t2)->mpfr_numbr;

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
		mpfr_fmt(_("%s(%Rg, %Rg): negative values will give strange results"),
					op, left, right)
				);
		if (! mpfr_integer_p(left) || ! mpfr_integer_p(right))
			lintwarn("%s",
		mpfr_fmt(_("%s(%Rg, %Rg): fractional values will be truncated"),
					op, left, right)
				);
	}
	return NULL;
}


/* do_and --- perform an & operation */

NODE *
do_mpfr_and(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "and")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpfr_numbr, MPFR_RNDZ);	/* float to integer conversion */
	mpfr_get_z(z, t2->mpfr_numbr, MPFR_RNDZ);	/* Same */	
	mpz_and(z, mpzval, z);

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, z, RND_MODE);	/* integer to float conversion */
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}

/* do_atan2 --- do the atan2 function */

NODE *
do_mpfr_atan2(int nargs)
{
	NODE *t1, *t2, *res;

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

	res = mpfr_node();
	/* See MPFR documentation for handling of special values like +inf as an argument */ 
	mpfr_atan2(res->mpfr_numbr, t1->mpfr_numbr, t2->mpfr_numbr, RND_MODE);

	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_compl --- perform a ~ operation */

NODE *
do_mpfr_compl(int nargs)
{
	NODE *tmp, *r;
	mpfr_ptr p;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("compl: received non-numeric argument"));

	p = force_number(tmp)->mpfr_numbr;
	if (! mpfr_number_p(p)) {
		/* [+-]inf or NaN */
		return tmp;
	}

	if (do_lint) {
		if (mpfr_signbit(p))
			lintwarn("%s",
		mpfr_fmt(_("compl(%Rg): negative value will give strange results"), p)
				);
		if (! mpfr_integer_p(p))
			lintwarn("%s",
		mpfr_fmt(_("comp(%Rg): fractional value will be truncated"), p)
				);
	}
	mpfr_get_z(mpzval, p, MPFR_RNDZ);
	mpz_com(mpzval, mpzval);
	r = mpfr_node();
	mpfr_set_z(r->mpfr_numbr, mpzval, RND_MODE);
	DEREF(tmp);
	return r;
}

#define SPEC_MATH(X)	\
NODE *tmp, *res; \
tmp = POP_SCALAR(); \
if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0) \
	lintwarn(_("%s: received non-numeric argument"), #X); \
force_number(tmp); \
res = mpfr_node(); \
mpfr_##X(res->mpfr_numbr, tmp->mpfr_numbr, RND_MODE); \
DEREF(tmp); \
return res


/* do_sin --- do the sin function */

NODE *
do_mpfr_sin(int nargs)
{
	SPEC_MATH(sin);
}

/* do_cos --- do the cos function */

NODE *
do_mpfr_cos(int nargs)
{
	SPEC_MATH(cos);
}

/* do_exp --- exponential function */

NODE *
do_mpfr_exp(int nargs)
{
	SPEC_MATH(exp);
}

/* do_log --- the log function */

NODE *
do_mpfr_log(int nargs)
{
	SPEC_MATH(log);
}

/* do_sqrt --- do the sqrt function */

NODE *
do_mpfr_sqrt(int nargs)
{
	SPEC_MATH(sqrt);
}


/* do_int --- convert double to int for awk */

NODE *
do_mpfr_int(int nargs)
{
	NODE *tmp, *r;

	tmp = POP_SCALAR();
	if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
		lintwarn(_("int: received non-numeric argument"));
	force_number(tmp);
	if (! mpfr_number_p(tmp->mpfr_numbr)) {
		/* [+-]inf or NaN */
		return tmp;
	}
	mpfr_get_z(mpzval, tmp->mpfr_numbr, MPFR_RNDZ);
	r = mpfr_node();
	mpfr_set_z(r->mpfr_numbr, mpzval, RND_MODE);
	DEREF(tmp);
	return r;
}


/* do_lshift --- perform a << operation */
/*
 * Test:
 * $ ./gawk 'BEGIN { print lshift(1, 52) }'
 * 4503599627370496
 * $ ./gawk 'BEGIN { print lshift(1, 53) }'
 * 0
 * $ ./gawk -M 'BEGIN { print lshift(1, 53) }'
 * 9007199254740992
 */

NODE *
do_mpfr_lshift(int nargs)
{
	NODE *t1, *t2, *res;
	mp_bitcnt_t shift;

	if ((res = get_bit_ops(& t1, & t2, "lshift")) != NULL)
		return res;

	mpfr_get_z(mpzval, t1->mpfr_numbr, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(t2->mpfr_numbr, MPFR_RNDZ);		/* mpfr_t (float) => unsigned long conversion */
	mpz_mul_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval * 2^shift */

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, mpzval, RND_MODE);	/* integer to float conversion */
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_or --- perform an | operation */

NODE *
do_mpfr_or(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "or")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpfr_numbr, MPFR_RNDZ);
	mpfr_get_z(z, t2->mpfr_numbr, MPFR_RNDZ);	
	mpz_ior(z, mpzval, z);

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, z, RND_MODE);
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_rshift --- perform a >> operation */

/*
 * $ ./gawk 'BEGIN { print rshift(0xFFFF, 1)}'
 * 32767
 * $ ./gawk -M 'BEGIN { print rshift(0xFFFF, 1)}'
 * 32767
 * $ ./gawk 'BEGIN { print rshift(-0xFFFF, 1)}'
 * 9007199254708224
 * $ ./gawk -M 'BEGIN { print rshift(-0xFFFF, 1) }'
 * -32768
 *
 * $ ./gawk 'BEGIN { print rshift(lshift(123456789012345678, 45), 45)}'
 * 80
 * $ ./gawk -M 'BEGIN { print rshift(lshift(123456789012345678, 45), 45)}'
 * 123456789012345680
 * $ ./gawk -M -vPREC=80 'BEGIN { print rshift(lshift(123456789012345678, 45), 45)}'
 * 123456789012345678
 *
 * $ ./gawk -M 'BEGIN { print rshift(lshift(1, 999999999), 999999999)}'
 * 1
 * $ ./gawk -M 'BEGIN { print rshift(lshift(1, 9999999999), 9999999999)}'
 * gmp: overflow in mpz type
 * Aborted
 */

NODE *
do_mpfr_rhift(int nargs)
{
	NODE *t1, *t2, *res;
	mp_bitcnt_t shift;

	if ((res = get_bit_ops(& t1, & t2, "rshift")) != NULL)
		return res;

	mpfr_get_z(mpzval, t1->mpfr_numbr, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(t2->mpfr_numbr, MPFR_RNDZ);		/* mpfr_t (float) => unsigned long conversion */
	mpz_fdiv_q_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval / 2^shift, round towards −inf */

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, mpzval, RND_MODE);	/* integer to float conversion */
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_strtonum --- the strtonum function */

NODE *
do_mpfr_strtonum(int nargs)
{
	NODE *tmp, *r;
	int base;

	tmp = POP_SCALAR();
	r = mpfr_node();
	if ((tmp->flags & (NUMBER|NUMCUR)) != 0)
		mpfr_set(r->mpfr_numbr, tmp->mpfr_numbr, RND_MODE);
	else if ((base = get_numbase(tmp->stptr, use_lc_numeric)) != 10) {
		mpfr_strtofr(r->mpfr_numbr, tmp->stptr, NULL, base, RND_MODE);
		errno = 0;
	} else {
		(void) force_number(tmp);
		mpfr_set(r->mpfr_numbr, tmp->mpfr_numbr, RND_MODE);
	}

	DEREF(tmp);
	return r;
}


/* do_xor --- perform an ^ operation */

NODE *
do_mpfr_xor(int nargs)
{
	NODE *t1, *t2, *res;
	mpz_t z;

	if ((res = get_bit_ops(& t1, & t2, "xor")) != NULL)
		return res;

	mpz_init(z);
	mpfr_get_z(mpzval, t1->mpfr_numbr, MPFR_RNDZ);
	mpfr_get_z(z, t2->mpfr_numbr, MPFR_RNDZ);	
	mpz_xor(z, mpzval, z);

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, z, RND_MODE);
	mpz_clear(z);

	DEREF(t1);
	DEREF(t2);
	return res;
}

static int firstrand = TRUE;
static gmp_randstate_t state;
static mpz_t seed;	/* current seed */

/* do_rand --- do the rand function */

NODE *
do_mpfr_rand(int nargs ATTRIBUTE_UNUSED)
{
	NODE *res;

	if (firstrand) {
		/* Choose the default algorithm */
		gmp_randinit_default(state);
		mpz_init(seed);
		mpz_set_ui(seed, 1L);
		/* seed state */
		gmp_randseed(state, seed);
		firstrand = FALSE;
	}
	res = mpfr_node();
	mpfr_urandomb(res->mpfr_numbr, state);
	return res;
}


/* do_srand --- seed the random number generator */

NODE *
do_mpfr_srand(int nargs)
{
	NODE *tmp, *res;

	if (firstrand) {
		/* Choose the default algorithm */
		gmp_randinit_default(state);
		mpz_init(seed);
		mpz_set_ui(seed, 1L);
		/* No need to seed state, will change it below */
		firstrand = FALSE;
	}

	res = mpfr_node();
	mpfr_set_z(res->mpfr_numbr, seed, RND_MODE);	/* previous seed */

	if (nargs == 0)
		mpz_set_ui(seed, (unsigned long) time((time_t *) 0));
	else {
		tmp = POP_SCALAR();
		if (do_lint && (tmp->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("srand: received non-numeric argument"));
		force_number(tmp);
		mpfr_get_z(seed, tmp->mpfr_numbr, MPFR_RNDZ);
		DEREF(tmp);
	}

	gmp_randseed(state, seed);
	return res;
}


/* op_mpfr_assign --- assignment operators excluding = */
 
void
op_mpfr_assign(OPCODE op)
{
	NODE **lhs;
	NODE *t1, *t2, *r;
	mpfr_ptr p1, p2;

	lhs = POP_ADDRESS();
	t1 = *lhs;
	p1 = force_number(t1)->mpfr_numbr;

	t2 = TOP_SCALAR();
	p2 = force_number(t2)->mpfr_numbr;

	r = mpfr_node();
	switch (op) {
	case Op_assign_plus:
		mpfr_add(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	case Op_assign_minus:
		mpfr_sub(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	case Op_assign_times:
		mpfr_mul(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	case Op_assign_quotient:
		mpfr_div(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	case Op_assign_mod:
		mpfr_fmod(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	case Op_assign_exp:
		mpfr_pow(r->mpfr_numbr, p1, p2, RND_MODE);
		break;
	default:
		break;
	}

	DEREF(t2);
	unref(*lhs);
	*lhs = r;

	UPREF(r);
	REPLACE(r);
}


/* mpfr_fmt --- output formatted string with special MPFR/GMP conversion specifiers */

const char *
mpfr_fmt(const char *mesg, ...)
{
	static char *tmp = NULL;
	int ret;
	va_list args;

	if (tmp != NULL)
		mpfr_free_str(tmp);
	va_start(args, mesg);
	ret = mpfr_vasprintf(& tmp, mesg, args);
	va_end(args);
	if (ret >= 0 && tmp != NULL)
		return tmp;
	return mesg;
}

#endif
