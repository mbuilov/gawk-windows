/* hard-locale.h -- Same as hard-locale.c.
 *
 * For gawk, put this in a header file, provides source code
 * compatibility with GNU grep for dfa.c, so that dfa.c need
 * not be continually modified by hand.
 */
/* hard-locale.c -- Determine whether a locale is hard.
   Copyright 1997, 1998, 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */


/* Return nonzero if the current CATEGORY locale is hard, i.e. if you
   can't get away with assuming traditional C or POSIX behavior.  */
static int
hard_locale (int category)
{
#if ! (defined ENABLE_NLS && HAVE_SETLOCALE)
  return 0;
#else

  int hard = 1;
  char const *p = setlocale (category, 0);

  if (p)
    {
# if defined __GLIBC__ && __GLIBC__ >= 2
      if (strcmp (p, "C") == 0 || strcmp (p, "POSIX") == 0)
	hard = 0;
# else
      static ptr_t xmalloc PARAMS ((size_t n));

      char *locale = xmalloc (strlen (p) + 1);
      strcpy (locale, p);

      /* Temporarily set the locale to the "C" and "POSIX" locales to
	 find their names, so that we can determine whether one or the
	 other is the caller's locale.  */
      if (((p = setlocale (category, "C")) && strcmp (p, locale) == 0)
	  || ((p = setlocale (category, "POSIX")) && strcmp (p, locale) == 0))
	hard = 0;

      /* Restore the caller's locale.  */
      setlocale (category, locale);
      free(locale);
# endif
    }

  return hard;

#endif
}
