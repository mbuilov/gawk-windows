/* Copyright (C) 1991, 1992, 1993, 1996, 1997, 1998, 1999 Free Software
Foundation, Inc.
This file is part of the GNU C Library.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to the Free
Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA.  */

/*
 * Reworked version of original "missing_d/fnmatch.c":
 * 1) Do not assume this source can be a part of LIBC.
 *
 * Bug fixes:
 * 1) Continue checking other characters in bracket expression after unmatched
 *  escaped ']', e.g.: "[\]xyz]" now matches "x".
 * 2) Escaped ']' in bracket expression now may begin a range, e.g.: "[\]-_]"
 *  will match "^".
 * 3) If wctype is not available:
 *  return FNM_NOMATCH if character class is not known - be consistent with
 *  the case when wctype _is_ available.
 * 4) In bracket expression, after unmatched character class, ']' now do not
 *  starts a range, e.g. "[[:alpha:]-?]" will _not_ match "=".
 * 5) Allow letter 'z' in a character class name.
 * 6) FOLD start character of the range in bracket expression, so "^" will not
 *  be incorrectly matched by "[A-Z]".
 * 7) Correctly skip character classes in the bracket expressions, e.g.:
 *  match "XaZ" by "X[a[:digit:]]Z".
 * 8) Firstly, check for the range in bracket expression and do not match if
 *  the range is invalid, i.e.: do _not_ match "z" by "[z-a]".
 * 9) Try to match unpaired '[' as a non-special char, e.g.:
 *  "a[b" will match "a[b".
 *
 * Improvements:
 * 1) Add wide-character support.
 * 2) Make internal_fnmatch() non-recurive.
 * 3) FOLD name character only once before matching against bracket expression.
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Reworked 4/2020
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Enable GNU extensions in fnmatch.h.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE   1
#endif

#include <errno.h>
#include <fnmatch.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#ifdef FNMATCH_WIDE_CHAR
# include <wchar.h>
#endif

/* For platform which support the ISO C amendement 1 functionality we
   support user defined character classes.  */
#if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H
/* Solaris 2.5 has a bug: <wchar.h> must be included before <wctype.h>.  */
# ifndef FNMATCH_WIDE_CHAR
#  include <wchar.h>
# endif
# include <wctype.h>
#endif

#ifndef FNMATCH_WIDE_CHAR

# ifdef isblank
#  define FNMISBLANK(c) isblank (c)
# else
#  define FNMISBLANK(c) ((c) == ' ' || (c) == '\t')
# endif
# ifdef isgraph
#  define FNMISGRAPH(c) isgraph (c)
# else
#  define FNMISGRAPH(c) (isprint (c) && !isspace (c))
# endif

# define FNMISPRINT(c) isprint (c)
# define FNMISDIGIT(c) isdigit (c)
# define FNMISALNUM(c) isalnum (c)
# define FNMISALPHA(c) isalpha (c)
# define FNMISCNTRL(c) iscntrl (c)
# define FNMISLOWER(c) islower (c)
# define FNMISPUNCT(c) ispunct (c)
# define FNMISSPACE(c) isspace (c)
# define FNMISUPPER(c) isupper (c)
# define FNMISXDIGIT(c) isxdigit (c)

# define FNMCOLL(s1, s2) strcoll(s1, s2)

#else /* FNMATCH_WIDE_CHAR */

# define FNMISBLANK(c) wisblank (c)
# define FNMISGRAPH(c) wisgraph (c)
# define FNMISPRINT(c) wisprint (c)
# define FNMISDIGIT(c) wisdigit (c)
# define FNMISALNUM(c) wisalnum (c)
# define FNMISALPHA(c) wisalpha (c)
# define FNMISCNTRL(c) wiscntrl (c)
# define FNMISLOWER(c) wislower (c)
# define FNMISPUNCT(c) wispunct (c)
# define FNMISSPACE(c) wisspace (c)
# define FNMISUPPER(c) wisupper (c)
# define FNMISXDIGIT(c) wisxdigit (c)

# define FNMCOLL(s1, s2) wcscoll(s1, s2)

#endif /* FNMATCH_WIDE_CHAR */

#ifndef STREQ
# define STREQ(s1, s2) ((strcmp (s1, s2) == 0))
#endif

#ifndef CHAR_CLASS_MAX_LENGTH
# if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H
/* The GNU C library provides support for user-defined character classes
   and the functions from ISO C amendement 1.  */
#  ifdef CHARCLASS_NAME_MAX
#   define CHAR_CLASS_MAX_LENGTH CHARCLASS_NAME_MAX
#  else
/* This shouldn't happen but some implementation might still have this
   problem.  Use a reasonable default value.  */
#   define CHAR_CLASS_MAX_LENGTH 256
#  endif
# else
#  define CHAR_CLASS_MAX_LENGTH  6 /* Namely, `xdigit'.  */
# endif
#endif

/* This function doesn't exist on most systems.  */

#ifndef FNMATCH_WIDE_CHAR

# ifndef HAVE___STRCHRNUL
static char *
__strchrnul (
     const char *s,
     int c)
{
  const char *result = strchr (s, c);
  if (result == NULL)
    result = strchr (s, '\0');
  return (char*) result;
}
# endif

#else /* FNMATCH_WIDE_CHAR */

# ifndef HAVE_WCSCHRNUL
static wchar_t *
wcschrnul (
     const wchar_t *s,
     wchar_t c)
{
  const wchar_t *result = wcschr (s, c);
  if (result == NULL)
    result = wcschr(s, L'\0');
  return (wchar_t*) result;
}
# endif

#endif /* FNMATCH_WIDE_CHAR */

#ifndef FNMATCH_WIDE_CHAR
# define FNM_CHAR               char
# define FNM_UCHAR              unsigned FNM_CHAR
# define FNM_INT                int
# define CH(c)                  c
# define FNMSTRCHR(str, c)      strchr ((const char*)(str), c)
# define FNMSTRCHRNUL(str, c)   __strchrnul ((const char*)(str), c)
#else
# define FNM_CHAR               wchar_t
# define FNM_UCHAR              wchar_t
# define FNM_INT                wint_t
# define CH(c)                  L##c
# define FNMSTRCHR(str, c)      wcschr (str, c)
# define FNMSTRCHRNUL(str, c)   wcschrnul (str, c)
#endif

#ifndef FNMATCH_WIDE_CHAR
# define INTERNAL_FNMATCH internal_fnmatch
#else
# define INTERNAL_FNMATCH internal_wfnmatch
#endif

/* Match STRING against the filename pattern PATTERN, returning zero if
   it matches, nonzero if not.  */
static int
INTERNAL_FNMATCH (
     const FNM_UCHAR *const pattern,
     const FNM_UCHAR *const string,
     int no_leading_period, int flags, const int do_coll)
{
  const FNM_UCHAR *p = pattern, *n = string;
  FNM_INT c;

  enum {
    NO_ASTERISK      = 0,
    ASTERISK         = 1,
    ASTERISK_BRACKET = 2
  } state = NO_ASTERISK;

  const FNM_UCHAR *endp = NULL;

  /* Initialize variables to suppress compiler warnings.  */
  const FNM_UCHAR *saved_p = NULL, *saved_n = NULL;
  FNM_INT saved_c = 0;

/* Note that this evaluates C many times.  */
#ifndef FNMATCH_WIDE_CHAR
# define FOLD(c) ((flags & FNM_CASEFOLD) ? tolower (c) : (FNM_INT) (c))
#else
# define FOLD(c) ((flags & FNM_CASEFOLD) ? towlower (c) : (FNM_INT) (c))
#endif

  while ((c = *p++) != CH('\0'))
    {
      c = FOLD (c);

      switch (c)
        {
        case CH('?'):
          /* A ? needs to match one character.  */
          if (*n == CH('\0'))
            /* There isn't another character; no match.  */
            return FNM_NOMATCH;
          if (*n == CH('/') && (flags & FNM_FILE_NAME))
            /* A slash does not match a wildcard under FNM_FILE_NAME.  */
            goto not_matched;
          if (*n == CH('.') && no_leading_period
              && (n == string
                  || (n[-1] == CH('/') && (flags & FNM_FILE_NAME))))
            goto not_matched;
          break;

        case CH('\\'):
          if (!(flags & FNM_NOESCAPE))
            {
              c = *p++;
              if (c == CH('\0'))
                /* Trailing \ loses.  */
                return FNM_NOMATCH;
              c = FOLD (c);
            }
          if (FOLD (*n) != c)
            goto not_matched;
          break;

        case CH('*'):
          /* Previous asterisk have eaten part of the name.  */
          state = NO_ASTERISK;

          if (*n == CH('.') && no_leading_period
              && (n == string
                  || (n[-1] == CH('/') && (flags & FNM_FILE_NAME))))
            return FNM_NOMATCH;

          for (c = *p++; c == CH('?') || c == CH('*'); c = *p++)
            {
              if (*n == CH('/') && (flags & FNM_FILE_NAME))
                /* A slash does not match a wildcard under FNM_FILE_NAME.  */
                return FNM_NOMATCH;
              if (c == CH('?'))
                {
                  /* A ? needs to match one character.  */
                  if (*n == CH('\0'))
                    /* There isn't another character; no match.  */
                    return FNM_NOMATCH;
                  /* One character of the string is consumed in matching
                     this ? wildcard, so *??? won't match if there are
                     less than three characters.  */
                  ++n;
                }
            }

          if (c == CH('\0'))
            /* The wildcard(s) is/are the last element of the pattern.
               If the name is a file name and contains another slash
               this does mean it cannot match.  */
            /* The FNM_LEADING_DIR flag allows to match a string ended by
               slash */
            return (flags & FNM_FILE_NAME) && !(flags & FNM_LEADING_DIR) &&
                   FNMSTRCHR (n, CH('/')) != NULL ? FNM_NOMATCH : 0;

          else if (c == CH('/') && (flags & FNM_FILE_NAME))
            {
              while (*n != CH('\0') && *n != CH('/'))
                ++n;
              if (*n == CH('/'))
                {
                  ++n;
                  no_leading_period = flags & FNM_PERIOD;
                  continue;
                }
            }
          else
            {
              if (endp == NULL || (flags & FNM_FILE_NAME))
                endp = (const FNM_UCHAR*) FNMSTRCHRNUL (n,
                                                        (flags & FNM_FILE_NAME)
                                                        ? CH('/') : CH('\0'));
              if (n < endp)
                {
                  no_leading_period = no_leading_period
                                      && (n == string
                                          || ((flags & FNM_FILE_NAME)
                                              && n[-1] == CH('/')));

                  flags = (flags & FNM_FILE_NAME)
                          ? flags : (flags & ~FNM_PERIOD);

                  /* Prepare for a recursive call.  */
                  saved_p = --p;

                  if (c == CH('['))
                    {
                      state = ASTERISK_BRACKET;
                      saved_n = n;
                      continue;

                    asterisk_bracket_return:
                      /* Restart scan moving n by 1.  */
                      n = ++saved_n;
                      if (n < endp)
                        {
                          p = saved_p;
                          no_leading_period = no_leading_period
                                              && (flags & FNM_FILE_NAME)
                                              && n[-1] == CH('/');
                          continue;
                        }
                    }

                  if (c == CH('\\') && !(flags & FNM_NOESCAPE))
                    c = p[1];
                  saved_c = FOLD (c);

                  do
                    {
                      if (FOLD (*n) == saved_c)
                        break;
                    }
                  while (++n < endp);

                  if (n < endp)
                    {
                      state = ASTERISK;
                      saved_n = n;
                      continue;

                    asterisk_return:
                      /* Restart scan moving n by 1.  */
                      n = saved_n;
                      while (++n < endp)
                        {
                          if (FOLD (*n) == saved_c)
                            break;
                        }

                      if (n < endp)
                        {
                          saved_n = n;
                          p = saved_p;
                          no_leading_period = no_leading_period
                                              && (flags & FNM_FILE_NAME)
                                              && n[-1] == CH('/');
                          continue;
                        }
                    }
                }
            }

          /* If we come here no match is possible with the wildcard.  */
          return FNM_NOMATCH;

        case CH('['):
          {
            /* Nonzero if the sense of the character class is inverted.  */
            static int posixly_correct;
            int not;
            FNM_INT cold;
            FNM_INT fn;
            const FNM_UCHAR *const bracket = p;

            if (posixly_correct == 0)
              posixly_correct = getenv ("POSIXLY_CORRECT") != NULL ? 1 : -1;

            if (*n == CH('\0'))
              goto not_matched;

            if (*n == CH('.') && no_leading_period
                && (n == string || (n[-1] == CH('/')
                                    && (flags & FNM_FILE_NAME))))
              goto not_matched;

            if (*n == CH('/') && (flags & FNM_FILE_NAME))
              /* `/' cannot be matched.  */
              goto not_matched;

            not = (*p == CH('!') || (posixly_correct < 0 && *p == CH('^')));
            if (not)
              ++p;

            fn = FOLD (*n);

            c = *p++;
            do
              {
                if (!(flags & FNM_NOESCAPE) && c == CH('\\'))
                  {
                    if (*p == CH('\0'))
                      /* Trailing \ loses.  */
                      return FNM_NOMATCH;
                    c = *p++;
                    goto do_match;
                  }
                else if (c == CH('[') && *p == CH(':'))
                  {
                    /* Leave room for the null.  */
                    char str[CHAR_CLASS_MAX_LENGTH + 1];
                    size_t c1 = 0;
#if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H
                    wctype_t wt;
#endif
                    const FNM_UCHAR *const startp = p;

                    for (;;)
                      {
                        if (c1 == CHAR_CLASS_MAX_LENGTH)
                          /* The name is too long and therefore the pattern
                             is ill-formed.  */
                          return FNM_NOMATCH;

                        c = *++p;
                        if (c == CH(':') && p[1] == CH(']'))
                          {
                            p += 2;
                            break;
                          }
                        if ((FNM_CHAR) c < CH('a') || (FNM_CHAR) c > CH('z'))
                          {
                            /* This cannot possibly be a character class name.
                               Match it as a normal range.  */
                            p = startp;
                            c = CH('[');
                            goto do_match;
                          }
                        str[c1++] = (char) ('a' + ((FNM_CHAR) c - CH('a')));
                      }
                    str[c1] = '\0';

#if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H
                    wt = wctype (str);
                    if (wt == 0)
                      /* Invalid character class name.  */
                      return FNM_NOMATCH;

# ifndef FNMATCH_WIDE_CHAR
                    if (iswctype (btowc (*n), wt))
                      goto matched;
# else
                    if (iswctype (*n, wt))
                      goto matched;
# endif
#else
                    if STREQ (str, "alnum") {if FNMISALNUM (*n) goto matched;}
                    else if STREQ (str, "alpha") {if FNMISALPHA (*n) goto matched;}
                    else if STREQ (str, "blank") {if FNMISBLANK (*n) goto matched;}
                    else if STREQ (str, "cntrl") {if FNMISCNTRL (*n) goto matched;}
                    else if STREQ (str, "digit") {if FNMISDIGIT (*n) goto matched;}
                    else if STREQ (str, "graph") {if FNMISGRAPH (*n) goto matched;}
                    else if STREQ (str, "lower") {if FNMISLOWER (*n) goto matched;}
                    else if STREQ (str, "print") {if FNMISPRINT (*n) goto matched;}
                    else if STREQ (str, "punct") {if FNMISPUNCT (*n) goto matched;}
                    else if STREQ (str, "space") {if FNMISSPACE (*n) goto matched;}
                    else if STREQ (str, "upper") {if FNMISUPPER (*n) goto matched;}
                    else if STREQ (str, "xdigit") {if FNMISXDIGIT (*n) goto matched;}
                    else
                      /* Invalid character class name.  */
                      return FNM_NOMATCH;
#endif
                  }
                else if (c == CH('\0'))
                  {
                    /* [ (unterminated) loses.
                      Try to match [ as a non-special char.  */
                    c = CH('[');
                    p = bracket;
                    goto as_normal;
                  }
                else
                  {
                  do_match:

                    cold = FOLD (c);

                    if (*p == CH('-') && p[1] != CH(']'))
                      {
                        /* It is a range.  */
                        FNM_INT cend = *++p;
                        if (!(flags & FNM_NOESCAPE) && cend == CH('\\'))
                          cend = *++p;
                        if (cend == CH('\0'))
                          /* Trailing \ loses.  */
                          return FNM_NOMATCH;
                        p++;

                        if (!do_coll)
                          {
                            if ((FNM_CHAR) cold <= (FNM_CHAR) fn
                                && (FNM_CHAR) fn <= (FNM_CHAR) FOLD (cend))
                              goto matched;
                          }
                        else
                          {
                            FNM_CHAR fns[2];
                            FNM_CHAR coll[2];
                            fns[0] = (FNM_CHAR) fn;
                            fns[1] = '\0';
                            coll[0] = (FNM_CHAR) cold;
                            coll[1] = '\0';
                            if (FNMCOLL (coll, fns) <= 0)
                              {
                                coll[0] = (FNM_CHAR) cend;
                                if (FNMCOLL (fns, coll) <= 0)
                                  goto matched;
                              }
                          }
                      }
                    else if (cold == fn)
                      goto matched;
                  }

                c = *p++;
              }
            while (c != CH(']'));

            if (!not)
              goto not_matched;
            break;

          matched:
            if (not)
              goto not_matched;

            /* Skip the rest of the [...] that already matched.  */
            for (;;)
              {
                c = *p++;
                if (c == CH(']'))
                  break;

                if (c == CH('\0'))
                  /* [... (unterminated) loses.  */
                  goto not_matched;

                if (!(flags & FNM_NOESCAPE) && c == CH('\\'))
                  {
                    if (*p == CH('\0'))
                      /* Trailing \ loses.  */
                      return FNM_NOMATCH;
                    /* XXX 1003.2d11 is unclear if this is right.  */
                    ++p;
                  }
                else if (c == CH('[') && *p == CH(':'))
                  {
                    /* Skip character class name.  */
                    for (;;)
                      {
                        c = *++p;
                        if (c == CH(':') && p[1] == CH(']'))
                          {
                            p += 2;
                            break;
                          }
                        if ((FNM_CHAR) c < CH('a') || (FNM_CHAR) c > CH('z'))
                          {
                            /* This cannot possibly be a character class
                              name. */
                            break;
                          }
                      }
                  }
              }

          }
          break;

        default:
as_normal:
          if (c != FOLD (*n))
            goto not_matched;
        }

      ++n;
    }

  if (*n == CH('\0'))
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == CH('/'))
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  not_matched:
  if (state == ASTERISK_BRACKET)
    goto asterisk_bracket_return;
  if (state == ASTERISK)
    goto asterisk_return;

  return FNM_NOMATCH;
#undef FOLD
}

#ifndef FNMATCH_WIDE_CHAR
# define FNMATCH fnmatch
#else
# define FNMATCH wfnmatch
#endif

int
FNMATCH (
     const FNM_CHAR *pattern,
     const FNM_CHAR *string,
     int flags)
{
  const char *const coll = setlocale (LC_COLLATE, NULL);

  const int do_coll = coll != NULL
                      && !(*coll == 'C' && coll[1] == '\0')
                      && strcmp ("POSIX", coll) != 0;

  return INTERNAL_FNMATCH ((const FNM_UCHAR*) pattern,
                           (const FNM_UCHAR*) string,
                           flags & FNM_PERIOD, flags, do_coll);
}

#undef FNMCOLL

#undef FNMISBLANK
#undef FNMISGRAPH
#undef FNMISPRINT
#undef FNMISDIGIT
#undef FNMISALNUM
#undef FNMISALPHA
#undef FNMISCNTRL
#undef FNMISLOWER
#undef FNMISPUNCT
#undef FNMISSPACE
#undef FNMISUPPER
#undef FNMISXDIGIT

#undef FNM_CHAR
#undef FNM_UCHAR
#undef FNM_INT
#undef CH
#undef FNMSTRCHR
#undef FNMSTRCHRNUL

#undef INTERNAL_FNMATCH
#undef FNMATCH
