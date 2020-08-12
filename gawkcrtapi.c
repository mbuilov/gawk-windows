/*
 * gawkcrtapi.c -- Implement the functions defined for gawkcrtapi.h
 */

/*
 * Copyright (C) 2020 Michael M. Builov, mbuilov@gmail.com
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

#define NO_LOCALE_RPL /* avoid including "mscrtx/localerpl.h" */
#include "awk.h"

#ifdef WINDOWS_NATIVE
#include <direct.h>	/* mkdir/rmdir */
#include "mscrtx/socket_file.h"	/* socket_file_clearerr */
#include "mscrtx/wreaddir.h"	/* opendirfd/closedirfd/fchdir/opendir/closedir/readdir */
#include "mscrtx/wreadlink.h"	/* readlinkfd/readlink */
#include "mscrtx/localerpl.h"
#endif

#ifdef WINDOWS_NATIVE
#include "oldnames.h"
#endif

#ifdef GAWK_USE_CRT_API

ATTRIBUTE_PRINTF_PTR(format, 1, 2)
static int
crt_printf(const char *format, ...)
{
	va_list args;
	int r;
	va_start(args, format);
	r = vprintf(format, args);
	va_end(args);
	return r;
}

static int
crt_open(const char *name, int flags, ...)
{
	va_list args;
	int r;
	va_start(args, flags);
	r = open(name, flags, args);
	va_end(args);
	return r;
}

static int
crt_putchar(int c)
{
	return putchar(c);
}

static int
crt_stat(const char *path, gawk_stat_t *buf)
{
	return stat(path, buf);
}

ATTRIBUTE_PRINTF_PTR(format, 2, 3)
static int
crt_fprintf(FILE *stream, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}

ATTRIBUTE_PRINTF_PTR(format, 1, 0)
static int
crt_vprintf(const char *format, va_list ap)
{
	return vprintf(format, ap);
}


ATTRIBUTE_PRINTF_PTR(format, 2, 0)
static int
crt_vfprintf(FILE *stream, const char *format, va_list ap)
{
	return vfprintf(stream, format, ap);
}

static char **
crt_environ(void)
{
	return environ;
}

/* crt_assert_failed --- print error message and abort the program */

ATTRIBUTE_NORETURN
static void
crt_assert_failed(const char *sexpr, const char *file, unsigned line)
{
	set_loc(file, line);
	r_fatal("Assert failed: \"%s\" at %s:%u\n", sexpr, file, line);
}

#ifdef WINDOWS_NATIVE
static ssize_t crt_read(int fd, void *buf, size_t count)
{
	ssize_t ret = 0;
	/* Don't overflow ssize_t.  */
	if (count > (size_t)-1/2) {
		errno = EINVAL;
		return -1;
	}
	while (count > 0) {
		/* Cannot read more than INT_MAX at once.  */
		unsigned n = count <= INT_MAX ? (unsigned) count : INT_MAX;
		int x = read(fd, buf, n);
		if (x < 0)
			return -1;
		ret += x;
		if ((unsigned) x < n)
			break;
		buf = (char*) buf + n;
		count -= n;
	}
	return ret;
}

static ssize_t crt_write(int fd, const void *buf, size_t count)
{
	ssize_t ret;
	/* Don't overflow ssize_t.  */
	if (count > (size_t)-1/2) {
		errno = EINVAL;
		return -1;
	}
	ret = (ssize_t) count;
	while (count > 0) {
		/* Cannot write more than INT_MAX at once.  */
		unsigned n = count <= INT_MAX ? (unsigned) count : INT_MAX;
		int x = write(fd, buf, n);
		if (x != (int) n)
			return -1;
		buf = (const char*) buf + n;
		count -= n;
	}
	return ret;
}
#endif /* WINDOWS_NATIVE */

#ifndef WINDOWS_NATIVE
static long long crt_lseek(int fd, long long offset, int whence)
{
	/* Note:
	  _FILE_OFFSET_BITS=64 should be defined when compiling for a 32-bit OS
	  to support 64-bit offsets.  */
	off_t ret = lseek(fd, (off_t) offset, whence);
	return (long long) ret;
}

static long long crt_tell(int fd)
{
	/* Note:
	  _FILE_OFFSET_BITS=64 should be defined when compiling for a 32-bit OS
	  to support 64-bit offsets.  */
	off_t ret = tell(fd);
	return (long long) ret;
}
#endif /* !WINDOWS_NATIVE */

#ifdef WINDOWS_NATIVE
static wchar_t *crt_xpathwc(const char *path, wchar_t buf[], size_t buf_size)
{
	return xpathwc(path, buf, buf_size); /* this could be macro */
}
#endif /* !WINDOWS_NATIVE */

static int crt_mb_cur_max(void)
{
	return MB_CUR_MAX;
}

#ifndef WINDOWS_NATIVE
static int *crt_errno_p(void)
{
	return &errno;
}
#endif

#ifdef WINDOWS_NATIVE
static wctype_t crt_c32ctype(const char *name)
{
	c32ctype_t t = c32ctype(name);
	return (wctype_t) t;
}

static int crt_c32isctype(uint32_t c, wctype_t desc)
{
	c32ctype_t t = (c32ctype_t) desc;
	return c32isctype(c, t);
}
#endif

/* referenced from api_impl */
gawk_crt_api_t crt_api_impl = {
	GAWK_CRT_MAJOR_VERSION,	/* major and minor versions */
	GAWK_CRT_MINOR_VERSION,

	malloc,
	calloc,
	realloc,
	free,

	/* print a message to stdout */
	crt_printf,

	/* process failed assertion */
	crt_assert_failed,

	/* Standard streams */
	NULL/*stdin*/,
	NULL/*stdout*/,
	NULL/*stderr*/,

	/* File IO */
	crt_open,
	close,
	dup,
	dup2,

#ifdef WINDOWS_NATIVE
	crt_read,
	crt_write,
	_lseeki64,
	_telli64,
	_commit,
#else
	read,
	write,
	crt_lseek,
	crt_tell,
	fsync,
#endif

	fflush,
	fgetpos,
	fsetpos,
	rewind,
	fseek,
	ftell,

	fopen,
	fclose,
	fread,
	fwrite,

	clearerr,
	feof,
	ferror,
	fileno,

	crt_putchar,
	fputc,
	getchar,
	fgetc,
	puts,
	fputs,

	fgets,
	ungetc,

	mkdir,
	rmdir,
	remove,
	unlink,
	rename,
	chdir,

#ifdef WINDOWS_NATIVE
	crt_xpathwc,
	xfstat,
	xwstat,
	xstat,
	xlstat,
	xstat_root,
#endif

	crt_stat,
#ifdef WINDOWS_NATIVE
	_fstat64,
#else
	fstat,
#endif
	chmod,

#ifdef WINDOWS_NATIVE
	opendirfd,
	closedirfd,
	fchdir,
	opendir,
	closedir,
	readdir,
	wreadlinkfd,
	wreadlink,
	readlinkfd,
	readlink,
#endif

	crt_fprintf,
	sprintf,
	snprintf,

	crt_vprintf,
	crt_vfprintf,
	vsprintf,
	vsnprintf,

#ifdef WINDOWS_NATIVE
	_errno,
#else
	crt_errno_p,
#endif
	strerror,

	mkstemp,

	crt_environ,
	getenv,
	setenv,
	unsetenv,
	clearenv,

	setlocale,
	localeconv,

	crt_mb_cur_max,

	btowc,

	mblen,
	mbrlen,

	mbstowcs,
	wcstombs,

	strcoll,
	tolower,
	toupper,
	isascii,
	isalnum,
	isalpha,
	isblank,
	iscntrl,
	isdigit,
	isgraph,
	islower,
	isprint,
	ispunct,
	isspace,
	isupper,
	isxdigit,

#ifndef WINDOWS_NATIVE
	wcscoll,
	towlower,
	towupper,
	iswascii,
	iswalnum,
	iswalpha,
	iswblank,
	iswcntrl,
	iswdigit,
	iswgraph,
	iswlower,
	iswprint,
	iswpunct,
	iswspace,
	iswupper,
	iswxdigit,
	wctype,
	iswctype,
	mbtowc,
	mbrtowc,
	wctomb,
	wcrtomb,
#else /* WINDOWS_NATIVE */
	c32scoll,
	c32tolower,
	c32toupper,
	c32isascii,
	c32isalnum,
	c32isalpha,
	c32isblank,
	c32iscntrl,
	c32isdigit,
	c32isgraph,
	c32islower,
	c32isprint,
	c32ispunct,
	c32isspace,
	c32isupper,
	c32isxdigit,
	crt_c32ctype,
	crt_c32isctype,
	mbrtoc16,
	mbrtoc32,
	c16rtomb,
	c32rtomb,
	mbstoc32s,
	c32stombs,
	wcstoc32s,
	c32stowcs,
	c32slen,
	c32schr,
	c32srchr,
	c32schrnul,
#endif /* WINDOWS_NATIVE */

	/* Add more CRT replacements here.  */
};

void init_crt_api(void)
{
	crt_api_impl.crt_stdin  = stdin;
	crt_api_impl.crt_stdout = stdout;
	crt_api_impl.crt_stderr = stderr;
}

#endif /* GAWK_USE_CRT_API */
