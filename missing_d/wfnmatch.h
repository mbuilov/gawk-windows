/* Copyright (C) 1991, 1992, 1993, 1996, 1997, 1998, 1999 Free Software
Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to the Free
Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA.  */

/*
 * Wide-character version of fnmatch.
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Reworked 4/2020
 */

#ifndef	WFNMATCH_H_INCLUDED
#define	WFNMATCH_H_INCLUDED

#include <wchar.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Match NAME against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
/* flags/return values are defined in "fnmatch.h" */
extern int wfnmatch(const wchar_t *pattern, const wchar_t *name, int flags);

#ifdef	__cplusplus
}
#endif

#endif /* wfnmatch.h */
