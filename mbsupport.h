/*
 * mbsupport.h --- Localize determination of whether we have multibyte stuff.
 */

/* 
 * Copyright (C) 2004, 2005 the Free Software Foundation, Inc.
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

/*
 * This file is needed because we test for i18n support in 3 different
 * places, and we want a consistent definition in all of them.  Following
 * the ``Don't Repeat Yourself'' principle from "The Pragmatic Programmer",
 * we centralize the tests here.
 *
 * This test is the union of all the current tests.
 */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if    defined(HAVE_ISWCTYPE) \
    && defined(HAVE_LOCALE_H) \
    && defined(HAVE_MBRLEN) \
    && defined(HAVE_MBRTOWC) \
    && defined(HAVE_WCHAR_H) \
    && defined(HAVE_WCRTOMB) \
    && defined(HAVE_WCSCOLL) \
    && defined(HAVE_WCTYPE) \
    && defined(HAVE_WCTYPE_H) \
    && defined(HAVE_WCTYPE_T) \
    && defined(HAVE_WINT_T) \
    && defined(HAVE_ISWLOWER) \
    && defined(HAVE_ISWUPPER) \
    && defined(HAVE_TOWLOWER) \
    && defined(HAVE_TOWUPPER) \
    && (defined(HAVE_STDLIB_H) && defined(MB_CUR_MAX)) \
/* We can handle multibyte strings.  */
# define MBS_SUPPORT 1
#else
# undef MBS_SUPPORT
#endif
