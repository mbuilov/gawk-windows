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
 * 10) Use collating order when maching against ranges in bracket expressions.
 * 11) Do not match file name beginning with dot after bracket expression:
 *  fnmatch("*[a]/??", "ca/.b", PATHNAME | PERIOD) should _not_ match.
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

#else /* FNMATCH_WIDE_CHAR */

# define FNMISBLANK(c) iswblank (c)
# define FNMISGRAPH(c) iswgraph (c)
# define FNMISPRINT(c) iswprint (c)
# define FNMISDIGIT(c) iswdigit (c)
# define FNMISALNUM(c) iswalnum (c)
# define FNMISALPHA(c) iswalpha (c)
# define FNMISCNTRL(c) iswcntrl (c)
# define FNMISLOWER(c) iswlower (c)
# define FNMISPUNCT(c) iswpunct (c)
# define FNMISSPACE(c) iswspace (c)
# define FNMISUPPER(c) iswupper (c)
# define FNMISXDIGIT(c) iswxdigit (c)

#endif /* FNMATCH_WIDE_CHAR */

#ifndef FNMATCH_WIDE_CHAR
# define FNMCOLL(s1, s2) strcoll(s1, s2)
#else
# define FNMCOLL(s1, s2) wcscoll(s1, s2)
#endif

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
# define FNMISWCTYPE(c, wt)     iswctype (btowc (c), wt)
#else
# define FNM_CHAR               wchar_t
# define FNM_UCHAR              wchar_t
# define FNM_INT                wint_t
# define CH(c)                  L##c
# define FNMSTRCHR(str, c)      wcschr (str, c)
# define FNMSTRCHRNUL(str, c)   wcschrnul (str, c)
# define FNMISWCTYPE(c, wt)     iswctype (c, wt)
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
     int flags, const int do_coll)
{
  const FNM_UCHAR *p = pattern, *n = string;
  FNM_INT c;

  enum {
    NO_WILDCARD      = 0,
    WILDCARD         = 1,
    WILDCARD_BRACKET = 2
  } state = NO_WILDCARD;

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
        case CH('\\'):
          if (!(flags & FNM_NOESCAPE))
            {
              c = *p++;
              if (c == CH('\0'))
                /* Trailing \ loses.  */
                return FNM_NOMATCH;
              c = FOLD (c);
            }
          goto as_normal;

        case CH('?'):
          /* A ? needs to match one character.  */
          if (*n == CH('\0') || (*n == CH('/') && (flags & FNM_FILE_NAME)))
            /* A slash does not match a wildcard under FNM_FILE_NAME.  */
            return FNM_NOMATCH;
          if (*n == CH('.') && (flags & FNM_PERIOD)
              && (n == string
                  || ((flags & FNM_FILE_NAME) && n[-1] == CH('/'))))
            /* A leading dot does not match a wildcard under FNM_PERIOD.  */
            return FNM_NOMATCH;
          break;

        case CH('*'):
          /* Previous wildcard (if any) have consumed part of the string.
             Reset state.  */
          state = NO_WILDCARD;

          if (*n == CH('.') && (flags & FNM_PERIOD)
              && (n == string
                  || ((flags & FNM_FILE_NAME) && n[-1] == CH('/'))))
            /* A leading dot does not match a wildcard under FNM_PERIOD.  */
            return FNM_NOMATCH;

          /* Read * and ? wildcards.  They are can be mixed arbitrary.  */
          for (;;)
            {
              c = *p++;
              if (c == CH('*'))
                continue;
              if (c != CH('?'))
                break;
              /* A ? needs to match one character.  */
              if (*n == CH('\0') || (*n == CH('/') && (flags & FNM_FILE_NAME)))
                /* A slash does not match a wildcard under FNM_FILE_NAME.  */
                return FNM_NOMATCH;
              /* One character of the string is consumed in matching
                 this ? wildcard, so *??? won't match if there are
                 less than three characters.  */
              ++n;
            }

          if (c == CH('\0'))
            /* The wildcard(s) is/are the last element of the pattern.
               If the name is a file name and contains another slash
               this does mean it cannot match.  */
            /* The FNM_LEADING_DIR flag allows slash(es) after a name.  */
            return (flags & FNM_FILE_NAME) && !(flags & FNM_LEADING_DIR) &&
                   FNMSTRCHR (n, CH('/')) != NULL ? FNM_NOMATCH : 0;

          if (c == CH('/') && (flags & FNM_FILE_NAME))
            {
              /* Simple case:
                 a slash does not match a wildcard under FNM_FILE_NAME,
                 so can't consume `/' by a wildcard and re-match from the
                 next char.  */
              n = (const FNM_UCHAR*) FNMSTRCHR (n, CH('/'));
              if (n != NULL)
                {
                  ++n;
                  continue;
                }
              return FNM_NOMATCH;
            }

          /* Prepare for re-matching tail of the name while consuming name
             characters by a wildcard.  endp points to the name end.  */
          if (endp == NULL || (flags & FNM_FILE_NAME))
            endp = (const FNM_UCHAR*) FNMSTRCHRNUL (n,
                                                    (flags & FNM_FILE_NAME)
                                                    ? CH('/') : CH('\0'));

          /* Have already checked for a leading dot, may remove FNM_PERIOD
             from flags.  */
          flags = (flags & FNM_FILE_NAME) ? flags : (flags & ~FNM_PERIOD);

          /* Position of the first pattern character after wildcard.  */
          saved_p = p;

          if (c == CH('['))
            {
              /* Try to match by bracket expression.  If not matched, consume
                 one name character and retry a match.  */
              state = WILDCARD_BRACKET;
              saved_n = n;
              goto bracket;

            wildcard_bracket_not_matched:
              if (n >= endp)
                /* Full name was matched, but remaining part of the string
                   was not.  Wildcard cannot consume more than the name.  */
                return FNM_NOMATCH;

              /* Consume one name character by a wildcard and retry a match.  */
              n = ++saved_n;
              p = saved_p;
              goto bracket;
            }

          if (c == CH('\\') && !(flags & FNM_NOESCAPE))
            {
              c = *p++;
              if (c == CH('\0'))
                /* Trailing \ loses.  */
                return FNM_NOMATCH;
            }

          state = WILDCARD;
          saved_c = FOLD (c);

        wildcard_rematch:
          /* Consume characters of the name by a wildcard.  */
          while (FOLD (*n) != saved_c)
            if (++n >= endp)
              return FNM_NOMATCH;

          /* Try to match tail of the name from the next character.
             If cannot match, consume some characters of the name by
             a wildcard and retry a match.  */
          saved_n = ++n;
          continue;

        wildcard_not_matched:
          if (n >= endp)
            /* Full name was matched, but remaining part of the string
               was not.  Wildcard cannot consume more than the name.  */
            return FNM_NOMATCH;

          /* Consume matched name character by a wildcard, then repeat.  */
          n = saved_n;
          p = saved_p;
          goto wildcard_rematch;

        case CH('['):
        bracket:
          {
            /* Nonzero if the sense of the character class is inverted.  */
            int not_;
            FNM_INT cold;
            FNM_INT fn;
            const FNM_UCHAR *const bracket = p;
            static int posixly_correct;

            if (*n == CH('\0') || (*n == CH('/') && (flags & FNM_FILE_NAME)))
              /* `/' cannot be matched.  */
              goto not_matched;

            if (*n == CH('.') && (flags & FNM_PERIOD)
                && (n == string
                    || ((flags & FNM_FILE_NAME) && n[-1] == CH('/'))))
              /* leading `.' cannot be matched.  */
              goto not_matched;

            if (posixly_correct == 0)
              posixly_correct = getenv ("POSIXLY_CORRECT") != NULL ? 1 : -1;

            not_ = (*p == CH('!') || (posixly_correct < 0 && *p == CH('^')));
            if (not_)
              ++p;

            fn = FOLD (*n);

            /* Scan bracket expression until `]'  */
            c = *p++;
            do
              {
                if (c == CH('\\') && !(flags & FNM_NOESCAPE))
                  {
                    c = *p++;
                    if (c == CH('\0'))
                      /* Trailing \ loses.  */
                      return FNM_NOMATCH;
                    goto pat_match;
                  }

                if (c == CH('\0'))
                  {
                    /* [ (unterminated) loses.
                      Try to match [ as a non-special char.  */
                    c = CH('[');
                    p = bracket;
                    goto as_normal;
                  }

                if (c == CH('[') && *p == CH(':'))
                  {
                    /* Leave room for the null.  */
                    char str[CHAR_CLASS_MAX_LENGTH + 1];
                    size_t c1 = 0;
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
                            goto pat_match;
                          }
                        str[c1++] = (char) ('a' + ((FNM_CHAR) c - CH('a')));
                      }
                    str[c1] = '\0';

#if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H
                    {
                      wctype_t wt = wctype (str);
                      if (wt == 0)
                        /* Invalid character class name.  */
                        return FNM_NOMATCH;
                      if (FNMISWCTYPE(*n, wt))
                        goto matched;
                    }
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
                else
                  {
                  pat_match:
                    cold = FOLD (c);

                    if (*p == CH('-') && p[1] != CH(']'))
                      {
                        /* It is a range.  */
                        FNM_INT cend = *++p;
                        if (cend == CH('\\') && !(flags & FNM_NOESCAPE))
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
                            fns[1] = CH('\0');
                            coll[0] = (FNM_CHAR) cold;
                            coll[1] = CH('\0');
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

                /* Try next character of the pattern.  */
                c = *p++;
              }
            while (c != CH(']'));

            if (!not_)
              goto not_matched;
            break;

          matched:
            if (not_)
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

                if (c == CH('\\') && !(flags & FNM_NOESCAPE))
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

  if (*n == CH('/') && (flags & FNM_LEADING_DIR))
    /* The FNM_LEADING_DIR flag says that "foo*" matches "foobar/frobozz".  */
    return 0;

  not_matched:
  if (state == WILDCARD_BRACKET)
    goto wildcard_bracket_not_matched;
  if (state == WILDCARD)
    goto wildcard_not_matched;

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
                           flags, do_coll);
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
#undef FNMISWCTYPE

#undef INTERNAL_FNMATCH
#undef FNMATCH
