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

#ifndef mp_bitcnt_t
#define	mp_bitcnt_t unsigned long
#endif

#define POP_TWO_SCALARS(s1, s2) \
s2 = POP_SCALAR(); \
s1 = POP(); \
do { if (s1->type == Node_var_array) { \
DEREF(s2); \
fatal(_("attempt to use array `%s' in a scalar context"), array_vname(s1)); \
}} while (FALSE)

mpz_t mpzval;	/* GMP integer type; used as temporary in many places */

static mpfr_rnd_t mpfr_rnd_mode(const char *mode, size_t mode_len);


/* init_mpfr --- set up MPFR related variables */

void
init_mpfr(const char *rnd_mode)
{
	mpfr_set_default_prec(PRECISION);
	RND_MODE = mpfr_rnd_mode(rnd_mode, strlen(rnd_mode));
	mpfr_set_default_rounding_mode(RND_MODE);
	make_number = make_mpfr_number;
	m_force_number = force_mpfr_number;
	mpz_init(mpzval);
}

/* mpfr_node --- allocate a node to store a MPFR number */

NODE *
mpfr_node()
{
	NODE *r;
	getnode(r);
	r->type = Node_val;
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

NODE *
make_mpfr_number(double x)
{
	NODE *r;
	r = mpfr_node();
	mpfr_set_d(r->mpfr_numbr, x, RND_MODE);
	return r;
}

/* mpfr_force_number --- force a value to be a MPFR number */

AWKNUM
force_mpfr_number(NODE *n)
{
	char *cp, *cpend, *ptr;
	char save;
	int base = 10;
	unsigned int newflags = 0;

	if ((n->flags & (MPFN|NUMCUR)) == (MPFN|NUMCUR))
		return 0;

	if (n->flags & MAYBE_NUM) {
		n->flags &= ~MAYBE_NUM;
		newflags = NUMBER;
	}

	if ((n->flags & MPFN) == 0) {
		n->flags |= MPFN;
		mpfr_init(n->mpfr_numbr);
	}

	mpfr_set_d(n->mpfr_numbr, 0.0, RND_MODE);	/* initialize to 0.0 */

	if (n->stlen == 0)
		return 0;

	cp = n->stptr;
	cpend = n->stptr + n->stlen;
	while (cp < cpend && isspace((unsigned char) *cp))
		cp++;
	if (cp == cpend)	/* only spaces */
		return 0;
	
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
	return 0;
}

/* set_PREC --- update MPFR PRECISION related variables when PREC assigned to */

void
set_PREC()
{
	if (do_mpfr) {
		long l;
		NODE *val = PREC_node->var_value;

		l = (long) force_number(val);
		if ((val->flags & MPFN) != 0)
			l = mpfr_get_si(val->mpfr_numbr, RND_MODE);		

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


/* do_and_mpfr --- perform an & operation */

NODE *
do_and_mpfr(int nargs)
{
	NODE *t1, *t2;

	POP_TWO_SCALARS(t1, t2);

	DEREF(t1);
	DEREF(t2);
	return dupnode(Nnull_string);
}

/* do_atan2 --- do the atan2 function */

NODE *
do_atan2_mpfr(int nargs)
{
	NODE *t1, *t2;

	POP_TWO_SCALARS(t1, t2);

	DEREF(t1);
	DEREF(t2);
	return dupnode(Nnull_string);
}


/* do_compl --- perform a ~ operation */

NODE *
do_compl_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_cos --- do the cos function */

NODE *
do_cos_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_exp --- exponential function */

NODE *
do_exp_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_int --- convert double to int for awk */

NODE *
do_int_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_log --- the log function */

NODE *
do_log_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
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
do_lshift_mpfr(int nargs)
{
	NODE *t1, *t2, *res;
	mpfr_ptr left, right;
	mp_bitcnt_t shift;

	POP_TWO_SCALARS(t1, t2);
	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric first argument"));
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("lshift: received non-numeric second argument"));
	}

	(void) force_number(t1);
	(void) force_number(t2);

	assert((t1->flags & MPFN) != 0);
	assert((t2->flags & MPFN) != 0);

	left = t1->mpfr_numbr;
	right = t2->mpfr_numbr;		/* shift */

	if (! mpfr_number_p(left)) {
		/* [+-]inf or NaN */
		res = dupnode(t1);
		goto finish;
	}

	if (! mpfr_number_p(right)) {
		/* [+-]inf or NaN */
		res = dupnode(t2);
		goto finish;
	}

	if (do_lint) {
		char *tmp = NULL;
		if (mpfr_signbit(left) || mpfr_signbit(right)) {
			(void) mpfr_asprintf(& tmp,
				_("lshift(%Rg, %Rg): negative values will give strange results"), left, right);
			if (tmp != NULL) {
				lintwarn("%s", tmp);
				mpfr_free_str(tmp);
				tmp = NULL;
			}
		}
		if (! mpfr_integer_p(left) || ! mpfr_integer_p(right)) {
			(void) mpfr_asprintf(& tmp,
				_("lshift(%Rg, %Rg): fractional values will be truncated"), left, right);
			if (tmp != NULL) {
				lintwarn("%s", tmp);
				mpfr_free_str(tmp);
			}
		}
	}

	(void) mpfr_get_z(mpzval, left, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(right, MPFR_RNDZ);		/* mpfr_t (float) => unsigned long conversion */
	mpz_mul_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval * 2^shift */

	res = mpfr_node();
	(void) mpfr_set_z(res->mpfr_numbr, mpzval, RND_MODE);	/* mpz_t => mpfr_t conversion */

finish:
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_or --- perform an | operation */

NODE *
do_or_mpfr(int nargs)
{
	NODE *s1, *s2;

	POP_TWO_SCALARS(s1, s2);

	DEREF(s1);
	DEREF(s2);
	return dupnode(Nnull_string);
}

/* do_rand --- do the rand function */

NODE *
do_rand_mpfr(int nargs ATTRIBUTE_UNUSED)
{
	return dupnode(Nnull_string);
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
do_rhift_mpfr(int nargs)
{
	NODE *t1, *t2, *res;
	mpfr_ptr left, right;
	mp_bitcnt_t shift;

	POP_TWO_SCALARS(t1, t2);
	if (do_lint) {
		if ((t1->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric first argument"));
		if ((t2->flags & (NUMCUR|NUMBER)) == 0)
			lintwarn(_("rshift: received non-numeric second argument"));
	}

	(void) force_number(t1);
	(void) force_number(t2);

	assert((t1->flags & MPFN) != 0);
	assert((t2->flags & MPFN) != 0);

	left = t1->mpfr_numbr;
	right = t2->mpfr_numbr;		/* shift */

	if (! mpfr_number_p(left)) {
		/* [+-]inf or NaN */
		res = dupnode(t1);
		goto finish;
	}

	if (! mpfr_number_p(right)) {
		/* [+-]inf or NaN */
		res = dupnode(t2);
		goto finish;
	}

	if (do_lint) {
		char *tmp = NULL;
		if (mpfr_signbit(left) || mpfr_signbit(right)) {
			(void) mpfr_asprintf(& tmp,
				_("rshift(%Rg, %Rg): negative values will give strange results"), left, right);
			if (tmp != NULL) {
				lintwarn("%s", tmp);
				mpfr_free_str(tmp);
				tmp = NULL;
			}
		}

		if (! mpfr_integer_p(left) || ! mpfr_integer_p(right)) {
			(void) mpfr_asprintf(& tmp,
				_("rshift(%Rg, %Rg): fractional values will be truncated"), left, right);
			if (tmp != NULL) {
				lintwarn("%s", tmp);
				mpfr_free_str(tmp);
			}
		}
	}

	(void) mpfr_get_z(mpzval, left, MPFR_RNDZ);	/* mpfr_t (float) => mpz_t (integer) conversion */
	shift = mpfr_get_ui(right, MPFR_RNDZ);		/* mpfr_t (float) => unsigned long conversion */
	mpz_fdiv_q_2exp(mpzval, mpzval, shift);		/* mpzval = mpzval / 2^shift, round towards −inf */

	res = mpfr_node();
	(void) mpfr_set_z(res->mpfr_numbr, mpzval, RND_MODE);	/* mpz_t => mpfr_t conversion */

finish:
	DEREF(t1);
	DEREF(t2);
	return res;
}


/* do_sin --- do the sin function */

NODE *
do_sin_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_sqrt --- do the sqrt function */

NODE *
do_sqrt_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();

	DEREF(tmp);
	return dupnode(Nnull_string);
}

/* do_srand --- seed the random number generator */

NODE *
do_srand_mpfr(int nargs)
{
	NODE *tmp;

	if (nargs == 0)
		;
	else {
		tmp = POP_SCALAR();
		DEREF(tmp);
	}

	return dupnode(Nnull_string);
}

/* do_strtonum --- the strtonum function */

NODE *
do_strtonum_mpfr(int nargs)
{
	NODE *tmp;

	tmp = POP_SCALAR();
	DEREF(tmp);

	return dupnode(Nnull_string);
}


/* do_xor --- perform an ^ operation */

NODE *
do_xor_mpfr(int nargs)
{
	NODE *s1, *s2;

	POP_TWO_SCALARS(s1, s2);

	DEREF(s1);
	DEREF(s2);
	return dupnode(Nnull_string);
}

#endif

