/* 
 * Copyright (C) 2020 Michael. M. Builov (mbuilov@gmail.com).
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
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

/* uni_char.h */

#ifndef UNI_CHAR_H_INCLUDED
#define UNI_CHAR_H_INCLUDED 1

/* Generally, it's a bad idea to use wchar_t/wint_t to hold a unicode
  code point: there are platforms (such as Windows) where wchar_t/wint_t
  is too small to be able to hold any unicode code point value.
  Use uni_int_t/uni_char_t for that.  */
#if defined(_MSC_VER) || defined(__MINGW32__)
typedef unsigned int uni_int_t;
typedef unsigned int uni_char_t;
#define uni_toupper c32toupper
#define uni_tolower c32tolower
#define mbrto_uni   mbrtoc32
#define uni_rtomb   c32rtomb
#else
typedef wint_t uni_int_t;
typedef wchar_t uni_char_t;
#define uni_toupper towupper
#define uni_tolower towlower
#define mbrto_uni   mbrtowc
#define uni_rtomb   wcrtomb
#endif

/* uni_char_t/uni_int_t must be at least of 32-bits.  */
typedef int check_uni_char_t[1-2*((uni_char_t)-1 < 0xFFFFFFFF)];
typedef int check_uni_int_t[1-2*((uni_int_t)-1 < 0xFFFFFFFF)];

#define UNI_EOF ((uni_int_t)-1)

#endif /* UNI_CHAR_H_INCLUDED */
