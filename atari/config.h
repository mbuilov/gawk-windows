/* 
 * config.h for Atari ST.
 * Assumes gcc compiler and TOS libraries.
 * Edited by hand from a config.h generated automatically by configure.
 */

/* Default path for Awk library */
#define DEFPATH	".,c:\\lib\\awk,c:\\gnu\\lib\\awk"
/* Path separator in use */
#define ENVSEP	','
#define SZTC (size_t)
#define INTC (int)

#define STDC_HEADERS 1  /* have the usual ANSI header files */
#undef	REGEX_MALLOC	/* use alloca in regex.c */

#undef	__CHAR_UNSIGNED__	/* default char is signed */

/*
 * srandom already has a prototype defined - don't redefine it
 */
#define SRANDOM_PROTO	1

#undef	_POSIX_SOURCE	/* on Minix, used to get Posix functions */
#undef	_MINIX		/* on Minix, used to get Posix functions */
#undef	_POSIX_1_SOURCE	/* on Minix, define to 2 */

/*
 * define const to nothing if not __STDC__
 */
#ifndef __STDC__
#define const
#endif
