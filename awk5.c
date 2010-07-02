/*
 * routines for error messages
 *
 * $Log:	awk5.c,v $
 * Revision 1.12  89/03/21  18:31:46  david
 * changed defines for system without vprintf()
 * 
 * Revision 1.11  89/03/21  10:52:53  david
 * cleanup
 * 
 * Revision 1.10  88/12/08  11:00:07  david
 * add $Log:	awk5.c,v $
 * Revision 1.12  89/03/21  18:31:46  david
 * changed defines for system without vprintf()
 * 
 * Revision 1.11  89/03/21  10:52:53  david
 * cleanup
 * 
 * 
 */

/*
 * GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY.  No author or distributor accepts responsibility to anyone for
 * the consequences of using it or for whether it serves any particular
 * purpose or works at all, unless he says so in writing. Refer to the GAWK
 * General Public License for full details. 
 *
 * Everyone is granted permission to copy, modify and redistribute GAWK, but
 * only under the conditions described in the GAWK General Public License.  A
 * copy of this license is supposed to have been given to you along with GAWK
 * so you can know your rights and responsibilities.  It should be in a file
 * named COPYING.  Among other things, the copyright notice and this notice
 * must be preserved on all copies. 
 *
 * In other words, go ahead and share GAWK, but don't try to stop anyone else
 * from sharing it farther.  Help stamp out software hoarding! 
 */

#include "awk.h"

int sourceline = 0;
char *source = NULL;

/* VARARGS2 */
static void
err(s, msg, argp)
char *s;
char *msg;
va_list *argp;
{
	int line;
	char *file;

	(void) fprintf(stderr, "%s: %s ", myname, s);
	vfprintf(stderr, msg, *argp);
	(void) fprintf(stderr, "\n");
	line = (int) FNR_node->var_value->numbr;
	if (line)
		(void) fprintf(stderr, " input line number %d", line);
	file = FILENAME_node->var_value->stptr;
	if (file && !STREQ(file, "-"))
		(void) fprintf(stderr, ", file `%s'", file);
	(void) fprintf(stderr, "\n");
	if (sourceline)
		(void) fprintf(stderr, " source line number %d", sourceline);
	if (source)
		(void) fprintf(stderr, ", file `%s'", source);
	(void) fprintf(stderr, "\n");
}

/*VARARGS0*/
void
msg(msg, va_alist)
char *msg;
va_dcl
{
	va_list args;

	va_start(args);
	err("", msg, &args);
	va_end(args);
}

/*VARARGS0*/
void
warning(msg, va_alist)
char *msg;
va_dcl
{
	va_list args;

	va_start(args);
	err("warning:", msg, &args);
	va_end(args);
}

/*VARARGS0*/
void
fatal(msg, va_alist)
char *msg;
va_dcl
{
	va_list args;

	va_start(args);
	err("fatal error:", msg, &args);
	va_end(args);
#ifdef DEBUG
	abort();
#endif
	exit(1);
}

#if defined(HASDOPRNT) && defined(NOVPRINTF)
int
vsprintf(str, fmt, ap)
	char *str, *fmt;
	va_list ap;
{
	FILE f;
	int len;

	f._flag = _IOWRT+_IOSTRG;
	f._ptr = (unsigned char *)str;
	f._cnt = 32767;
	len = _doprnt(fmt, ap, &f);
	*f._ptr = 0;
	return (len);
}

int
vfprintf(iop, fmt, ap)
	FILE *iop;
	char *fmt;
	va_list ap;
{
	int len;

	len = _doprnt(fmt, ap, iop);
	return (ferror(iop) ? EOF : len);
}

int
vprintf(fmt, ap)
	char *fmt;
	va_list ap;
{
	int len;

	len = _doprnt(fmt, ap, stdout);
	return (ferror(stdout) ? EOF : len);
}
#endif
