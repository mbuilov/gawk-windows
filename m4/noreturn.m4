dnl Date: Sun, 26 Feb 2012 11:31:50 -0800
dnl From: Paul Eggert <eggert@cs.ucla.edu>
dnl To: arnold@skeeve.com
dnl CC: bug-grep@gnu.org
dnl Subject: Re: avoid gcc 4.6.2 'may be used before set' warnings in dfa.c
dnl 
dnl On 02/26/2012 01:18 AM, arnold@skeeve.com wrote:
dnl > It looks like I can just use the code as it is now in grep. I have asked
dnl > for compile failures and haven't gotten any.
dnl 
dnl Sure, but the gnulib support for this is better
dnl than what's in the dfa code.  It could be that
dnl your correspondents aren't using the less-common hosts
dnl or compiler warning options that gnulib is ported to.
dnl 
dnl In the long run stdnoreturn.h or _Noreturn is the way to go,
dnl since they're part of the C standard.
dnl 
dnl If you'd rather not create a separate file, how about if
dnl we change the dfa code to use _Noreturn instead of
dnl <stdnoreturn.h> and noreturn, and you can put the
dnl following into your configure.ac so that config.h
dnl defines _Noreturn the same way gnulib does:

AC_DEFUN([GAWK_AC_NORETURN],[
AH_VERBATIM([_Noreturn],
[/* The _Noreturn keyword of C11.  */
#ifndef _Noreturn
# if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__) \
      || 0x5110 <= __SUNPRO_C)
#  define _Noreturn __attribute__ ((__noreturn__))
# elif defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif
])])
