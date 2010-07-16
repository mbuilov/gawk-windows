/*
 * snprintf.c - Implement snprintf and vsnprintf on platforms that need them.
 */

/* 
 * Copyright (C) 2006 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
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


/* If using in a multi-threaded context, then SNPRINTF_REENTRANT must be
   defined.   But in that case, performance will be much worse, since a
   temporary file is created and closed for each call to snprintf. */

#if defined(HAVE_MKSTEMP)
/* If mkstemp is available, use it instead of tmpfile(), since some older
   implementations of tmpfile() were not secure. */

static FILE *
safe_tmpfile (void)
{
	static const char template[] = "/tmp/snprintfXXXXXX";
	FILE *f;
	int fd;
	char t[sizeof (template)];

	strcpy (t, template);
	if ((fd = mkstemp (t)) < 0)
		return NULL;
	unlink (t);
	if ((f = fdopen (fd, "w+b")) == NULL) {
		close (fd);
		return NULL;
	}
	/* setvbuf(f,NULL,_IOFBF,4*BUFSIZ); */
	return f;
}

#elif defined(HAVE_TMPFILE)
#define safe_tmpfile tmpfile
#else
#error Neither mkstemp() nor tmpfile() is available on this platform.
#endif

int
vsnprintf (char *restrict buf, size_t len,
		const char *restrict fmt, va_list args)
{
	int actual;
	int nread;
	size_t cnt = 0;
#ifndef SNPRINTF_REENTRANT
	static
#endif
	FILE *fp;

	if ((buf == NULL) || (len < 1))
		return -1;

	buf[0] = '\0';	/* in case the caller does not check the return code! */

#ifdef SNPRINTF_REENTRANT
	if ((fp = safe_tmpfile ()) == NULL)
		return -1;
#else
	if ((fp == NULL) && ((fp = safe_tmpfile ()) == NULL))
		return -1;
	rewind (fp);
#endif
	actual = vfprintf (fp, fmt, args);
	rewind (fp);
	if (actual < 0) {
#ifdef SNPRINTF_REENTRANT
		fclose (fp);
#endif
		return -1;
	}
	else if ((size_t) actual < len)
		len = actual;
	else
		--len;
	while (cnt < len && (nread = fread (buf + cnt, 1, len - cnt, fp)) > 0)
		cnt += nread;
	buf[cnt] = '\0';
#ifdef SNPRINTF_REENTRANT
	fclose (fp);
#endif
	if (cnt < len)
		return -1;

	return actual;
}

int
#if defined(HAVE_STDARG_H) && defined(__STDC__) && __STDC__
snprintf (char *restrict buf, size_t len, const char *restrict fmt, ...)
#else
snprintf (va_alist)
     va_dcl
#endif
{
	int rv;
	va_list args;

#if defined(HAVE_STDARG_H) && defined(__STDC__) && __STDC__
	va_start (args, fmt);
#else
	char *buf;
	size_t len;
	char *fmt;

	va_start (args);
	buf = va_arg (args, char *);
	len = va_arg (args, size_t);
	fmt = va_arg (args, char *);
#endif
	rv = vsnprintf (buf, len, fmt, args);
	va_end (args);
	return rv;
}
