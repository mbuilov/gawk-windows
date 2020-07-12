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
 * Reworked version of original "missing_d/fnmatch.h":
 * 1) Assume compiler supports at least ANSI C.
 * 2) Added wide-character version: wfnmatch (include "wfnmatch.h")
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Reworked 4/2020
 */

#ifndef	FNMATCH_H_INCLUDED
#define	FNMATCH_H_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif

/* Bits set in the FLAGS argument to `fnmatch'.  */
#define	FNM_PATHNAME	(1 << 0) /* No wildcard can ever match `/'.  */
#define	FNM_NOESCAPE	(1 << 1) /* Backslashes don't quote special chars.  */
#define	FNM_PERIOD	(1 << 2) /* Leading `.' is matched only explicitly.  */

#if !defined _POSIX_C_SOURCE || _POSIX_C_SOURCE < 2 || defined _GNU_SOURCE
# define FNM_FILE_NAME	 FNM_PATHNAME	/* Preferred GNU name.  */
# define FNM_LEADING_DIR (1 << 3)	/* Ignore `/...' after a match.  */
# define FNM_CASEFOLD	 (1 << 4)	/* Compare without regard to case.  */
#endif

/* Value returned by `fnmatch' if STRING does not match PATTERN.  */
#define	FNM_NOMATCH	1

/* This value is returned if the implementation does not support
   `fnmatch'.  Since this is not the case here it will never be
   returned but the conformance test suites still require the symbol
   to be defined.  */
#ifdef _XOPEN_SOURCE
# define FNM_NOSYS	(-1)
#endif

/* Match NAME against the filename pattern PATTERN,
   returning zero if it matches, FNM_NOMATCH if not.  */
extern int fnmatch(const char *pattern, const char *name, int flags);

#ifdef	__cplusplus
}
#endif

#endif /* fnmatch.h */
