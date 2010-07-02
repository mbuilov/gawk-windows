/*
 * routines for error messages
 *
 * $Log:	awk5.c,v $
 * Revision 1.15  89/03/31  13:26:11  david
 * GNU license
 * 
 * Revision 1.14  89/03/30  10:22:23  david
 * fixed up varargs usage
 * 
 * Revision 1.13  89/03/29  14:14:37  david
 * delinting
 * 
 * Revision 1.12  89/03/21  18:31:46  david
 * changed defines for system without vprintf()
 * 
 * Revision 1.11  89/03/21  10:52:53  david
 * cleanup
 * 
 * Revision 1.10  88/12/08  11:00:07  david
 * add $Log:	awk5.c,v $
 * Revision 1.15  89/03/31  13:26:11  david
 * GNU license
 * 
 * Revision 1.14  89/03/30  10:22:23  david
 * fixed up varargs usage
 * 
 * Revision 1.13  89/03/29  14:14:37  david
 * delinting
 * 
 * Revision 1.12  89/03/21  18:31:46  david
 * changed defines for system without vprintf()
 * 
 * Revision 1.11  89/03/21  10:52:53  david
 * cleanup
 * 
 * 
 */

/* 
 * Copyright (C) 1986, 1988, 1989 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
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
msg(va_alist)
va_dcl
{
	va_list args;
	char *mesg;

	va_start(args);
	mesg = va_arg(args, char *);
	err("", mesg, &args);
	va_end(args);
}

/*VARARGS0*/
void
warning(va_alist)
va_dcl
{
	va_list args;
	char *mesg;

	va_start(args);
	mesg = va_arg(args, char *);
	err("warning:", mesg, &args);
	va_end(args);
}

/*VARARGS0*/
void
fatal(va_alist)
va_dcl
{
	va_list args;
	char *mesg;

	va_start(args);
	mesg = va_arg(args, char *);
	err("fatal error:", mesg, &args);
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
