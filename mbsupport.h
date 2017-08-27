/*
 * mbsupport.h --- Localize determination of whether we have multibyte stuff.
 */

/*
 * Copyright (C) 2004, 2005, 2011, 2012, 2015, 2016
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


#ifdef __DJGPP__
# undef MB_CUR_MAX
# define MB_CUR_MAX 1

/* All this glop is for DGJPP */

#define towupper	toupper
#define towlower	tolower
#define iswalnum	isalnum
#define iswalpha	isalpha
#define iswupper	isupper
#define iswlower	islower

#define mbrtowc(wcp, s, e, mbs)	(-1)
#define mbrlen(s, e, mbs)	strlen(s)
#define wcrtomb(wc, b, mbs)	(-1)
#define wcslen		strlen
#define wctob(wc)	(EOF)

#if (__DJGPP__ > 2 || __DJGPP_MINOR__ >= 3)
# include <ctype.h>
# include <wchar.h>
# include <wctype.h>
#else
# define mbstate_t	int
#endif

extern wctype_t wctype(const char *name);
extern int iswctype(wint_t wc, wctype_t desc);
extern int wcscoll(const wchar_t *ws1, const wchar_t *ws2);
#endif
