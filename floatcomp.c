/*
 * floatcomp.c - Isolate floating point details.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2011, 2016
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
#include <math.h>

#ifdef HAVE_UINTMAX_T

/* Assume IEEE-754 arithmetic on pre-C89 hosts.  */
#ifndef FLT_RADIX
#define FLT_RADIX 2
#endif
#ifndef FLT_MANT_DIG
#define FLT_MANT_DIG 24
#endif
#ifndef DBL_MANT_DIG
#define DBL_MANT_DIG 53
#endif

/*
 * The number of base-FLT_RADIX digits in an AWKNUM fraction, assuming
 * that AWKNUM is not long double.
 */
#define AWKSMALL_MANT_DIG \
  (sizeof (AWKNUM) == sizeof (double) ? DBL_MANT_DIG : FLT_MANT_DIG)

/*
 * The number of base-FLT_DIGIT digits in an AWKNUM fraction, even if
 * AWKNUM is long double.  Don't mention 'long double' unless
 * LDBL_MANT_DIG is defined, for the sake of ancient compilers that
 * lack 'long double'.
 */
#ifdef LDBL_MANT_DIG
#define AWKNUM_MANT_DIG \
  (sizeof (AWKNUM) == sizeof (long double) ? LDBL_MANT_DIG : AWKSMALL_MANT_DIG)
#else
#define AWKNUM_MANT_DIG AWKSMALL_MANT_DIG
#endif

/*
 * The number of bits in an AWKNUM fraction, assuming FLT_RADIX is
 * either 2 or 16.  IEEE and VAX formats use radix 2, and IBM
 * mainframe format uses radix 16; we know of no other radices in
 * practical use.
 */
#if FLT_RADIX != 2 && FLT_RADIX != 16
Please port the following code to your weird host;
#endif
#define AWKNUM_FRACTION_BITS (AWKNUM_MANT_DIG * (FLT_RADIX == 2 ? 1 : 4))
#define DBL_FRACTION_BITS (DBL_MANT_DIG * (FLT_RADIX == 2 ? 1 : 4))

/* Return the number of trailing zeros in N.  N must be nonzero.  */
static int
count_trailing_zeros(uintmax_t n)
{
#if 3 < (__GNUC__ + (4 <= __GNUC_MINOR__)) && UINTMAX_MAX <= ULLONG_MAX
	return __builtin_ctzll(n);
#else
	int i = 0;
	for (; (n & 3) == 0; n >>= 2)
		i += 2;
	return i + (1 & ~n);
#endif
}

/* adjust_uint --- fiddle with values, ask Paul Eggert to explain */

uintmax_t
adjust_uint(uintmax_t n)
{
	/*
	 * If uintmax_t is so wide that AWKNUM cannot represent all its
	 * values, strip leading nonzero bits of integers that are so large
	 * that they cannot be represented exactly as AWKNUMs, so that their
	 * low order bits are represented exactly, without rounding errors.
	 * This is more desirable in practice, since it means the user sees
	 * integers that are the same width as the AWKNUM fractions.
	 */
	int wordbits = CHAR_BIT * sizeof n;
	if (AWKNUM_FRACTION_BITS < wordbits) {
		uintmax_t one = 1;
		uintmax_t sentinel = one << (wordbits - AWKNUM_FRACTION_BITS);
		int shift = count_trailing_zeros(n | sentinel);
		uintmax_t mask = (one << AWKNUM_FRACTION_BITS) - 1;

		n &= mask << shift;
	}

	return n;
}
#endif /* HAVE_UINTMAX_T */
