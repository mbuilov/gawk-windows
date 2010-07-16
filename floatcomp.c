/*
 * floatcomp.c - Isolate floating point details.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2007 the Free Software Foundation, Inc.
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

/*
 * Floor and Ceil --- Work around a problem in conversion of
 * doubles to exact integers.
 */

/* Floor --- do floor(), also for Cray */

AWKNUM
Floor(AWKNUM n)
{
	return floor(n
#if 0
#ifdef _CRAY
		* (1.0 + DBL_EPSILON)
#endif
#endif
	);
}

/* Ceil --- do ceil(), also for Cray */

AWKNUM
Ceil(AWKNUM n)
{
	return ceil(n
#if 0
#ifdef _CRAY
		* (1.0 + DBL_EPSILON)
#endif
#endif
	);
}

#ifdef HAVE_UINTMAX_T
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
	if (AWKNUM_FRACTION_BITS < CHAR_BIT * sizeof n)
		n &= ((uintmax_t) 1 << AWKNUM_FRACTION_BITS) - 1;

	return n;
}
#endif /* HAVE_UINTMAX_T */
