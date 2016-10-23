/*
 * random.h - redefine name of random lib routines to avoid conflicts
 */

/*
 * Copyright (C) 1996, 2001, 2004, 2005, 2013 the Free Software Foundation, Inc.
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

#define initstate gawk_initstate
#define setstate gawk_setstate
#define random gawk_random
#define srandom gawk_srandom

#if SIZEOF_UNSIGNED_INT == 4
typedef unsigned int gawk_uint32_t;
typedef          int gawk_int32_t;
#else
#if SIZEOF_UNSIGNED_LONG == 4
typedef unsigned long gawk_uint32_t;
typedef          long gawk_int32_t;
#endif
#endif
#define uint32_t gawk_uint32_t
#define int32_t  gawk_int32_t

extern long random (void);
