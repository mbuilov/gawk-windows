/* vms_misc.c -- sustitute code for missing/different run-time library routines.

   Copyright (C) 1991-1993, 1996-1997, 2001, 2003, 2009, 2010, 2011, 2014
   the Free Software Foundation, Inc.

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

#define creat creat_dummy	/* one of gcc-vms's headers has bad prototype */
#include "awk.h"
#include "vms.h"
#undef creat
#include <fab.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include <rmsdef.h>
#include <ssdef.h>
#include <stsdef.h>

    /*
     * In VMS's VAXCRTL, strerror() takes an optional second argument.
     *  #define strerror(errnum) strerror(errnum,vaxc$errno)
     * is all that's needed, but VAXC can't handle that (gcc can).
     * [The 2nd arg is used iff errnum == EVMSERR.]
     */
#ifdef strerror
# undef strerror
#endif
extern char *strerror(int,...);

/* vms_strerror() -- convert numeric error code into text string */
char *
vms_strerror( int errnum )
{
    return ( errnum != EVMSERR ? strerror(errnum)
			       : strerror(EVMSERR, vaxc$errno) );
}
# define strerror(v) vms_strerror(v)

    /*
     * Miscellaneous utility routine, not part of the run-time library.
     */
/* vms_strdup() - allocate some new memory and copy a string into it */
char *
vms_strdup( const char *str )
{
    char *result;
    int len = strlen(str);

    emalloc(result, char *, len+1, "strdup");
    return strcpy(result, str);
}

    /*
     * VAXCRTL does not contain unlink().  This replacement has limited
     * functionality which is sufficient for GAWK's needs.  It works as
     * desired even when we have the file open.
     */
/* unlink(file) -- delete a file (ignore soft links) */
int
unlink( const char *file_spec ) {
    char tmp[255+1];			/*(should use alloca(len+2+1)) */
    extern int delete(const char *);

    strcpy(tmp, file_spec);		/* copy file name */
    if (strchr(tmp, ';') == NULL)
	strcat(tmp, ";0");		/* append version number */
    return delete(tmp);
}

    /*
     * Work-around an open(O_CREAT+O_TRUNC) bug (screwed up modification
     * and creation dates when new version is created), and also use some
     * VMS-specific file options.  Note:  optional 'prot' arg is completely
     * ignored; gawk doesn't need it.
     */
#ifdef open
# undef open
#endif
extern int creat(const char *,int,...);
extern int open(const char *,int,unsigned,...);

/* vms_open() - open a file, possibly creating it */
int
vms_open( const char *name, int mode, ... )
{
    int result;

    if (strncmp(name, "/dev/", 5) == 0) {
	/* (this used to be handled in vms_devopen(), but that is only
	   called when opening files for output; we want it for input too) */
	if (strcmp(name + 5, "null") == 0)	/* /dev/null -> NL: */
	    name = "NL:";
	else if (strcmp(name + 5, "tty") == 0)	/* /dev/tty -> TT: */
	    name = "TT:";
    }

    if (mode == (O_WRONLY|O_CREAT|O_TRUNC)) {
	/* explicitly force stream_lf record format to override DECC$SHR's
	   defaulting of RFM to earlier file version's when one is present */
	/* 3.1.7 fix: letting record attibutes default resulted in DECC$SHR's
	   creat() failing with "invalid record attributes" when trying to
	   make a new version of an existing file which had rfm=vfc,rat=prn
	   format, so add explicit "rat=cr" to go with rfm=stmlf to force
	   the usual "carriage return carriage control" setting */
	result = creat(name, 0, "rfm=stmlf", "rat=cr", "shr=nil", "mbc=32");
    } else {
	struct stat stb;
        int stat_result;
	const char *mbc, *shr = "shr=get", *ctx = "ctx=stm";

	stat_result = stat((char *)name, &stb);
	if ( stat_result < 0) {	/* assume DECnet */
	    mbc = "mbc=8";
	} else {    /* ordinary file; allow full sharing iff record format */
	    mbc = "mbc=32";
	    if ((stb.st_fab_rfm & 0x0F) < FAB$C_STM) shr = "shr=get,put,upd";
	}
	result = open(name, mode, 0, shr, mbc, "mbf=2");
	if ((stat_result >= 0) && (result < 0) && (errno == ENOENT)) {
	    /* ENOENT not possible because stat succeeded */
	    errno = EMFILE;
	    if (S_ISDIR(stb.st_mode)) {
		errno = EISDIR; /* Bug seen in VMS 8.3 */
	    }
        }
    }

    /* This is only approximate; the ACP -> RMS -> VAXCRTL interface
       discards too much potentially useful status information...  */
    if (result < 0 && errno == EVMSERR
		   && (vaxc$errno == RMS$_ACC || vaxc$errno == RMS$_CRE))
	errno = EMFILE;	/* redirect() should close 1 file & try again */

    return result;
}

    /*
     * Check for attempt to (re-)open known file.
     */
/* vms_devopen() - check for "SYS$INPUT" or "SYS$OUTPUT" or "SYS$ERROR" */
int
vms_devopen( const char *name, int mode )
{
    FILE *file = NULL;

    if (strncasecmp(name, "SYS$", 4) == 0) {
	name += 4;		/* skip "SYS$" */
	if (strncasecmp(name, "INPUT", 5) == 0 && (mode & O_WRONLY) == 0)
	    file = stdin,  name += 5;
	else if (strncasecmp(name, "OUTPUT", 6) == 0 && (mode & O_WRONLY) != 0)
	    file = stdout,  name += 6;
	else if (strncasecmp(name, "ERROR", 5) == 0 && (mode & O_WRONLY) != 0)
	    file = stderr,  name += 5;
	if (*name == ':')  name++;	/* treat trailing colon as optional */
    }
    /* note: VAXCRTL stdio has extra level of indirection (*file) */
    return (file && *file && *name == '\0') ? fileno(file) : -1;
}


#define VMS_UNITS_PER_SECOND 10000000L	/* hundreds of nanoseconds, 1e-7 */
#define UNIX_EPOCH "01-JAN-1970 00:00:00.00"

extern U_Long SYS$BINTIM(), SYS$GETTIM();
extern U_Long LIB$SUBX(), LIB$EDIV();

    /*
     * Get current time in microsecond precision.
     */
/* vms_gettimeofday() - get current time in `struct timeval' format */
int
vms_gettimeofday(struct timeval *tv, void *timezone__not_used)
{
    /*
	Emulate unix's gettimeofday call; timezone argument is ignored.
    */
    static const struct dsc$descriptor_s epoch_dsc =
       { sizeof UNIX_EPOCH - sizeof "",
         DSC$K_DTYPE_T, DSC$K_CLASS_S, UNIX_EPOCH };
    static long epoch[2] = {0L,0L};	/* needs one time initialization */
    const long  thunk = VMS_UNITS_PER_SECOND;
    long        now[2], quad[2];

    if (!epoch[0])  SYS$BINTIM(&epoch_dsc, epoch);	/* 1 Jan 0:0:0 1970 */
    /* get current time, as VMS quadword time */
    SYS$GETTIM(now);
    /* convert the quadword time so that it's relative to Unix epoch */
    LIB$SUBX(now, epoch, quad); /* quad = now - epoch; */
    /* convert 1e-7 units into seconds and fraction of seconds */
    LIB$EDIV(&thunk, quad, &tv->tv_sec, &tv->tv_usec);
    /* convert fraction of seconds into microseconds */
    tv->tv_usec /= (VMS_UNITS_PER_SECOND / 1000000);

    return 0;           /* success */
}


#ifndef VMS_V7
    /*
     * VMS prior to V7.x has no timezone support unless DECnet/OSI is used.
     */
/* these are global for use by missing/strftime.c */
char   *tzname[2] = { "local", "" };
int     daylight = 0, timezone = 0, altzone = 0;

/* tzset() -- dummy to satisfy linker */
void tzset(void)
{
    return;
}
#endif	/*VMS_V7*/


#ifndef CRTL_VER_V731
/* getpgrp() -- there's no such thing as process group under VMS;
 *		job tree might be close enough to be useful though.
 */
int getpgrp(void)
{
    return 0;
}
#endif

#ifndef __GNUC__
void vms_bcopy( const char *src, char *dst, int len )
{
    (void) memcpy(dst, src, len);
}
#endif /*!__GNUC__*/


/*----------------------------------------------------------------------*/
#ifdef NO_VMS_ARGS      /* real code is in "vms/vms_args.c" */
void vms_arg_fixup( int *argc, char ***argv ) { return; }	/* dummy */
#endif

#ifdef NO_VMS_PIPES     /* real code is in "vms/vms_popen.c" */
FILE *popen( const char *command, const char *mode ) {
    fatal(" Cannot open pipe `%s' (not implemented)", command);
    return NULL;
}
int pclose( FILE *current ) {
    fatal(" Cannot close pipe #%d (not implemented)", fileno(current));
    return -1;
}
int fork( void ) {
    fatal(" Cannot fork process (not implemented)");
    return -1;
}
#endif /*NO_VMS_PIPES*/
/*----------------------------------------------------------------------*/


/*
 *	The following code is taken from the GNU C preprocessor (cccp.c,
 *	2.8.1 vintage) where it was used #if VMS.  It is only needed for
 *	VAX C and GNU C on VAX configurations; DEC C's run-time library
 *	doesn't have the problem described.
 *
 *	VMS_fstat() and VMS_stat() were static in cccp.c but need to be
 *	accessible to the whole program here.  Also, the special handling
 *	for the null device has been introduced for gawk's benefit, to
 *	prevent --lint mode from giving spurious warnings about /dev/null
 *	being empty if it's used as an input file.
 */

#if defined(VAXC) || (defined(__GNUC__) && !defined(__alpha))

/* more VMS hackery */
#include <fab.h>
#include <nam.h>

extern unsigned long SYS$PARSE(), SYS$SEARCH();

/* Work around a VAXCRTL bug.  If a file is located via a searchlist,
   and if the device it's on is not the same device as the one specified
   in the first element of that searchlist, then both stat() and fstat()
   will fail to return info about it.  `errno' will be set to EVMSERR, and
   `vaxc$errno' will be set to SS$_NORMAL due yet another bug in stat()!
   We can get around this by fully parsing the filename and then passing
   that absolute name to stat().

   Without this fix, we can end up failing to find header files, which is
   bad enough, but then compounding the problem by reporting the reason for
   failure as "normal successful completion."  */

#undef fstat	/* Get back to the library version.  */

int
VMS_fstat (fd, statbuf)
     int fd;
     struct stat *statbuf;
{
  int result = fstat (fd, statbuf);

  if (result < 0)
    {
      FILE *fp;
      char nambuf[NAM$C_MAXRSS+1];

      if ((fp = fdopen (fd, "r")) != 0 && fgetname (fp, nambuf) != 0)
	result = VMS_stat (nambuf, statbuf);
      /* No fclose(fp) here; that would close(fd) as well.  */
    }

  if (result == 0		/* GAWK addition; fixup /dev/null flags */
      && (statbuf->st_mode & S_IFREG)
      && strcmp(statbuf->st_dev, "_NLA0:") == 0)
    {
      statbuf->st_mode &= ~S_IFREG;
      statbuf->st_mode |= S_IFCHR;
    }

  return result;
}

int
VMS_stat (name, statbuf)
     const char *name;
     struct stat *statbuf;
{
  int result = stat (name, statbuf);

  if (result < 0)
    {
      struct FAB fab;
      struct NAM nam;
      char exp_nam[NAM$C_MAXRSS+1],  /* expanded name buffer for sys$parse */
	   res_nam[NAM$C_MAXRSS+1];  /* resultant name buffer for sys$search */

      fab = cc$rms_fab;
      fab.fab$l_fna = (char *) name;
      fab.fab$b_fns = (unsigned char) strlen (name);
      fab.fab$l_nam = (void *) &nam;
      nam = cc$rms_nam;
      nam.nam$l_esa = exp_nam,  nam.nam$b_ess = sizeof exp_nam - 1;
      nam.nam$l_rsa = res_nam,  nam.nam$b_rss = sizeof res_nam - 1;
      nam.nam$b_nop = NAM$M_PWD | NAM$M_NOCONCEAL;
      if (sys$parse (&fab) & 1)
	{
	  if (sys$search (&fab) & 1)
	    {
	      res_nam[nam.nam$b_rsl] = '\0';
	      result = stat (res_nam, statbuf);
	    }
	  /* Clean up searchlist context cached by the system.  */
	  nam.nam$b_nop = NAM$M_SYNCHK;
	  fab.fab$l_fna = 0,  fab.fab$b_fns = 0;
	  (void) sys$parse (&fab);
	}
    }

  if (result == 0		/* GAWK addition; fixup /dev/null flags */
      && (statbuf->st_mode & S_IFREG)
      && strcmp(statbuf->st_dev, "_NLA0:") == 0)
    {
      statbuf->st_mode &= ~S_IFREG;
      statbuf->st_mode |= S_IFCHR;
    }

  return result;
}
#endif	/* VAXC || (__GNUC__ && !__alpha) */
