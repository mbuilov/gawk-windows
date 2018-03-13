/*
 * snprintf.c - Implement snprintf and vsnprintf on platforms that need them.
 */

/*
 * Copyright (C) 2006, 2007, 2018 the Free Software Foundation, Inc.
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

static char *tmpfilename = NULL;
static FILE *safe_f = NULL;

#ifdef HAVE_ATEXIT
static void close_safe_f()
{
	if (safe_f != NULL) {
		fclose(safe_f);
		safe_f = NULL;
	}
	if (tmpfilename != NULL) {
		unlink(tmpfilename);
		free(tmpfilename);
		tmpfilename = NULL;
	}
}
#endif

static FILE *
safe_tmpfile (void)
{
	static bool first = true;
	static const char template[] = "snprintfXXXXXX";
	int fd;
	static char *tmpdir = NULL;
	static int len = 0;

	if (first) {
		first = false;
		/*
		 * First try Unix stanadard env var, then Windows var,
		 * then fall back to /tmp.
		 */
		if ((tmpdir = getenv("TMPDIR")) != NULL && *tmpdir != '\0')
			;	/* got it */
		else if ((tmpdir = getenv("TEMP")) != NULL && *tmpdir != '\0')
			;	/* got it */
		else
			tmpdir = "/tmp";

		len = strlen(tmpdir) + 1 + strlen(template) + 1;
#ifdef HAVE_ATEXIT
		atexit(close_safe_f);
#endif /* HAVE_ATEXIT */
	}

	if ((tmpfilename = (char *) malloc(len)) == NULL)
		return NULL;
	else
		sprintf(tmpfilename, "%s/%s", tmpdir, template);

	if ((fd = mkstemp (tmpfilename)) < 0)
		return NULL;

#if ! defined(__DJGPP__) && ! defined(MSDOS) && ! defined(_MSC_VER) \
	&& ! defined(_WIN32) && ! defined(__CRTRSXNT__) && ! defined(__EMX__) \
	&& ! defined(__MINGW32__) && ! defined(__WIN32__)
	/* If not MS or OS/2, unlink after opening. */
	unlink (tmpfilename);
	free(tmpfilename);
	tmpfilename = NULL;
#endif

	if ((safe_f = fdopen (fd, "w+b")) == NULL) {
		close (fd);
		return NULL;
	}
	/* setvbuf(f,NULL,_IOFBF,4*BUFSIZ); */
	return safe_f;
}

#elif defined(HAVE_TMPFILE)
#define safe_tmpfile tmpfile
#else
#error Neither mkstemp() nor tmpfile() is available on this platform.
#endif

#if (__STDC_VERSION__ + 0) < 199901
#undef restrict	/* force it! */
#define restrict
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
		if (tmpfilename != NULL) {
			unlink(tmpfilename);
			free(tmpfilename);
			tmpfilename = NULL;
		}
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
	if (tmpfilename != NULL) {
		unlink(tmpfilename);
		free(tmpfilename);
		tmpfilename = NULL;
	}
#endif
	if (cnt < len)
		return -1;

	return actual;
}

int
snprintf (char *restrict buf, size_t len, const char *restrict fmt, ...)
{
	int rv;
	va_list args;

	va_start (args, fmt);
	rv = vsnprintf (buf, len, fmt, args);
	va_end (args);
	return rv;
}
