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

#define HAVE_STRING_H 1 /* have <string.h> */
#ifdef  HAVE_STRING_H
#undef	NEED_MEMORY_H	/* need <memory.h> to declare memcpy() et al. */
#endif

#define STDC_HEADERS 1 /* have the usual ANSI header files */
#undef	HAVE_UNISTD_H	/* have <unistd.h> */
#undef	HAVE_ALLOCA_H	/* have <alloca.h> -- only used if bison is used */
#undef	HAVE_SIGNUM_H	/* have <signum.h> */
#undef	REGEX_MALLOC	/* don't use alloca in regex.c */

#define HAVE_VPRINTF 1 /* have vprintf() */

#define HAVE_RANDOM 1 /* have random(), or using missing/random.c */

#define HAVE_STRCHR 1   /* have strchr() and strrchr() */
#ifndef	HAVE_STRCHR
#ifdef	HAVE_RINDEX	/* use index() and rindex() if present */
#define	strchr	index
#define	strrchr	rindex
#endif
#endif

#define HAVE_FMOD 1     /* have fmod(), otherwise use modf() */

#define HAVE_MEMCPY 1   /* have memcpy() et al. */

#define HAVE_ST_BLKSIZE 1 /* have st_blksize member in the stat(2) structure */

#define HAVE_STRFTIME 1

#define HAVE_STRINGIZE 1 /* have ANSI "stringizing" capability */

#undef	__CHAR_UNSIGNED__	/* default char is signed */

#define	RETSIGTYPE	void    /* type used in signal() */
#define SPRINTF_RET int         /* type returned by sprintf() */

#undef	_ALL_SOURCE	/* on AIX, used to get some BSD functions */
/*
 * srandom already has a prototype defined - don't redefine it
 */
#define SRANDOM_PROTO	1

#undef	_POSIX_SOURCE	/* on Minix, used to get Posix functions */
#undef	_MINIX		/* on Minix, used to get Posix functions */
#undef	_POSIX_1_SOURCE	/* on Minix, define to 2 */



