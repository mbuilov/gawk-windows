/*
 * gawkcrtapi.h -- Replacements of CRT library functions.
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

/* Current version of the GAWK CRT API.  */
enum {
	GAWK_CRT_MAJOR_VERSION = 1,
	GAWK_CRT_MINOR_VERSION = 0
};

#if defined(_MSC_VER) || defined(__MINGW32__)

/* declared in mscrtx/xstat.h */
struct xstat;

/* declared in mscrtx/wreaddir.h */
struct dir_fd_;
struct win_find;

/* from windows.h */
struct _WIN32_FIND_DATAW;

#endif /* _MSC_VER || __MINGW32__ */

struct lconv;

/*
 * Replacements of CRT library functions.
 *
 * !!! If you make any changes to this structure, please remember to bump !!!
 * !!! GAWK_CRT_MAJOR_VERSION and/or GAWK_CRT_MINOR_VERSION.              !!!
 */
typedef struct gawk_crt_api {

	/* CRT api version */
	const int crt_major_version;
	const int crt_minor_version;

	/*
	 * Hooks to provide access to gawk's memory allocation functions.
	 * This ensures that memory passed between gawk and the extension
	 * is allocated and released by the same library.
	 */
	void *(*crt_malloc)(size_t size);
	void *(*crt_calloc)(size_t nmemb, size_t size);
	void *(*crt_realloc)(void *ptr, size_t size);
	void (*crt_free)(void *ptr);

	ATTRIBUTE_PRINTF_PTR(format, 1, 2)
	int (*crt_printf)(const char *format, ...);

	/* Assert failed: print message and abort the program */
	void (*crt_assert_failed)(const char *sexpr, const char *file,
			unsigned line);

	FILE *crt_stdin;
	FILE *crt_stdout;
	FILE *crt_stderr;

	int (*crt_open)(const char *name, int flags, ...);
	int (*crt_close)(int fd);
	int (*crt_dup)(int oldfd);
	int (*crt_dup2)(int oldfd, int newfd);

	ssize_t (*crt_read)(int fd, void *buf, size_t count);
	ssize_t (*crt_write)(int fd, const void *buf, size_t count);
	long long (*crt_lseek)(int fd, long long offset, int whence);
	long long (*crt_tell)(int fd);
	int (*crt_fsync)(int fd);

	int (*crt_fflush)(FILE *stream);
	int (*crt_fgetpos)(FILE *stream, fpos_t *pos);
	int (*crt_fsetpos)(FILE *stream, const fpos_t *pos);
	void (*crt_rewind)(FILE *stream);
	int (*crt_fseek)(FILE *stream, long offset, int whence);
	long (*crt_ftell)(FILE *stream);

	FILE *(*crt_fopen)(const char *filename, const char *mode);
	int (*crt_fclose)(FILE *stream);
	size_t (*crt_fread)(void *buf, size_t size, size_t nmemb, FILE *stream);
	size_t (*crt_fwrite)(const void *buf, size_t size, size_t nmemb,
			FILE *stream);

	void (*crt_clearerr)(FILE *stream);
	int (*crt_feof)(FILE *stream);
	int (*crt_ferror)(FILE *stream);
	int (*crt_fileno)(FILE *stream);

	int (*crt_putchar)(int c);
	int (*crt_fputc)(int c, FILE *stream);
	int (*crt_getchar)(void);
	int (*crt_fgetc)(FILE *stream);
	int (*crt_puts)(const char *s);
	int (*crt_fputs)(const char *s, FILE *stream);

	char *(*crt_fgets)(char *s, int size, FILE *stream);
	int (*crt_ungetc)(int c, FILE *stream);

	int (*crt_mkdir)(const char *dirname);
	int (*crt_rmdir)(const char *dirname);
	int (*crt_remove)(const char *pathname);
	int (*crt_unlink)(const char *pathname);
	int (*crt_rename)(const char *old_name, const char *new_name);
	int (*crt_chdir)(const char *path);

#if defined _MSC_VER || defined(__MINGW32__)
	wchar_t *(*crt_xpathwc)(const char *path, wchar_t buf[],
		size_t buf_size);
	int (*crt_xfstat)(void *h, const wchar_t *path, struct xstat *buf);
	int (*crt_xwstat)(const wchar_t *path, struct xstat *buf,
			int dont_follow);
	int (*crt_xstat)(const char *path, struct xstat *buf);
	int (*crt_xlstat)(const char *path, struct xstat *buf);
	int (*crt_xstat_root)(const wchar_t *path, struct xstat *buf);
#endif

	int (*crt_stat)(const char *path, gawk_stat_t *buf);
	int (*crt_fstat)(int fd, gawk_stat_t *buf);
	int (*crt_chmod)(const char *path, int mode);

#if defined _MSC_VER || defined(__MINGW32__)
	int (*crt_opendirfd)(const char path[], struct dir_fd_ **const dfd);
	int (*crt_closedirfd)(struct dir_fd_ *dfd);
	int (*crt_fchdir)(const struct dir_fd_ *dfd);
	struct win_find *(*crt_opendir)(const char path[]);
	int (*crt_closedir)(struct win_find *const dirp);
	struct _WIN32_FIND_DATAW *(*crt_readdir)(struct win_find *const dirp);
	int (*crt_wreadlinkfd)(void *h, wchar_t buf[], const size_t bufsiz);
	int (*crt_wreadlink)(const wchar_t path[], wchar_t buf[], const size_t bufsiz);
	int (*crt_readlinkfd)(void *h, char buf[], const size_t bufsiz);
	int (*crt_readlink)(const char path[], char buf[], const size_t bufsiz);
#endif

	ATTRIBUTE_PRINTF_PTR(format, 2, 3)
	int (*crt_fprintf)(FILE *stream, const char *format, ...);

	ATTRIBUTE_PRINTF_PTR(format, 2, 3)
	int (*crt_sprintf)(char *str, const char *format, ...);

	ATTRIBUTE_PRINTF_PTR(format, 3, 4)
	int (*crt_snprintf)(char *str, size_t size, const char *format, ...);

	ATTRIBUTE_PRINTF_PTR(format, 1, 0)
	int (*crt_vprintf)(const char *format, va_list ap);

	ATTRIBUTE_PRINTF_PTR(format, 2, 0)
	int (*crt_vfprintf)(FILE *stream, const char *format, va_list ap);

	ATTRIBUTE_PRINTF_PTR(format, 2, 0)
	int (*crt_vsprintf)(char *str, const char *format, va_list ap);

	ATTRIBUTE_PRINTF_PTR(format, 3, 0)
	int (*crt_vsnprintf)(char *str, size_t size, const char *format,
			va_list ap);

	int *(*crt_errno_p)(void);
	char *(*crt_strerror)(int error_number);

	int (*crt_mkstemp)(char *templ);

	char **(*crt_environ)(void);
	char *(*crt_getenv)(const char *name);
	int (*crt_setenv)(const char *name, const char *value, int overwrite);
	int (*crt_unsetenv)(const char *name);
	int (*crt_clearenv)(void);

	char *(*crt_setlocale)(int category, const char *locale);
	struct lconv *(*crt_localeconv)(void);

	int (*crt_mb_cur_max)(void);

	wint_t (*crt_btowc)(int c);

	int (*crt_mblen)(const char *s, size_t n);
	size_t (*crt_mbrlen)(const char *s, size_t n, mbstate_t *ps);

	size_t (*crt_mbstowcs)(wchar_t *wcstr, const char *mbstr, size_t count);
	size_t (*crt_wcstombs)(char *mbstr, const wchar_t *wcstr, size_t count);

	int (*crt_strcoll)(const char *s1, const char *s2);
	int (*crt_tolower)(int c);
	int (*crt_toupper)(int c);
	int (*crt_isascii)(int c);
	int (*crt_isalnum)(int c);
	int (*crt_isalpha)(int c);
	int (*crt_isblank)(int c);
	int (*crt_iscntrl)(int c);
	int (*crt_isdigit)(int c);
	int (*crt_isgraph)(int c);
	int (*crt_islower)(int c);
	int (*crt_isprint)(int c);
	int (*crt_ispunct)(int c);
	int (*crt_isspace)(int c);
	int (*crt_isupper)(int c);
	int (*crt_isxdigit)(int c);

#if !defined _MSC_VER && !defined __MINGW32__

	int (*crt_wcscoll)(const wchar_t *s1, const wchar_t *s2);
	wint_t (*crt_towlower)(wint_t c);
	wint_t (*crt_towupper)(wint_t c);
	int (*crt_iswascii)(wint_t c);
	int (*crt_iswalnum)(wint_t c);
	int (*crt_iswalpha)(wint_t c);
	int (*crt_iswblank)(wint_t c);
	int (*crt_iswcntrl)(wint_t c);
	int (*crt_iswdigit)(wint_t c);
	int (*crt_iswgraph)(wint_t c);
	int (*crt_iswlower)(wint_t c);
	int (*crt_iswprint)(wint_t c);
	int (*crt_iswpunct)(wint_t c);
	int (*crt_iswspace)(wint_t c);
	int (*crt_iswupper)(wint_t c);
	int (*crt_iswxdigit)(wint_t c);
	wctype_t (*crt_wctype)(const char *name);
	int (*crt_iswctype)(wint_t c, wctype_t desc);

	int (*crt_mbtowc)(wchar_t *pwc, const char *s, size_t n);
	size_t (*crt_mbrtowc)(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
	int (*crt_wctomb)(char *s, wchar_t wc);
	size_t (*crt_wcrtomb)(char *s, wchar_t wc, mbstate_t *ps);

#else /* _MSC_VER || __MINGW32__ */

	int (*crt_c32scoll)(const uint32_t *s1, const uint32_t *s2);
	uint32_t (*crt_c32tolower)(uint32_t c);
	uint32_t (*crt_c32toupper)(uint32_t c);
	int (*crt_c32isascii)(uint32_t c);
	int (*crt_c32isalnum)(uint32_t c);
	int (*crt_c32isalpha)(uint32_t c);
	int (*crt_c32isblank)(uint32_t c);
	int (*crt_c32iscntrl)(uint32_t c);
	int (*crt_c32isdigit)(uint32_t c);
	int (*crt_c32isgraph)(uint32_t c);
	int (*crt_c32islower)(uint32_t c);
	int (*crt_c32isprint)(uint32_t c);
	int (*crt_c32ispunct)(uint32_t c);
	int (*crt_c32isspace)(uint32_t c);
	int (*crt_c32isupper)(uint32_t c);
	int (*crt_c32isxdigit)(uint32_t c);
	wctype_t (*crt_c32ctype)(const char *name);
	int (*crt_c32isctype)(uint32_t c, wctype_t desc);

	size_t (*crt_mbrtoc16)(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps);
	size_t (*crt_mbrtoc32)(uint32_t *pwi, const char *s, size_t n, mbstate_t *ps);
	size_t (*crt_c16rtomb)(char *s, wchar_t wc, mbstate_t *ps);
	size_t (*crt_c32rtomb)(char *s, uint32_t wi, mbstate_t *ps);

	size_t (*crt_mbstoc32s)(uint32_t *dst, const char *src, size_t n);
	size_t (*crt_c32stombs)(char *dst, const uint32_t *src, size_t n);
	size_t (*crt_wcstoc32s)(uint32_t *dst, const wchar_t *src, size_t n);
	size_t (*crt_c32stowcs)(wchar_t *dst, const uint32_t *src, size_t n);

	size_t (*crt_c32slen)(const uint32_t *s);
	uint32_t *(*crt_c32schr)(const uint32_t *s, uint32_t c);
	uint32_t *(*crt_c32srchr)(const uint32_t *s, uint32_t c);
	uint32_t *(*crt_c32schrnul)(const uint32_t *s, uint32_t c);

#endif /* _MSC_VER || __MINGW32__ */

	/* 1) Add more CRT replacements here.  */
} gawk_crt_api_t;

#ifdef GAWK	/* Only for GAWK code! */
extern gawk_crt_api_t crt_api_impl;
void init_crt_api(void);
#endif

#ifndef GAWK	/* these are not for the gawk code itself! */

extern const gawk_crt_api_t *gawk_crt(void);

#ifndef NDEBUG
#define gawk_crt_assert(expr) ((void)((expr) ? 1 : \
	(gawk_crt()->crt_assert_failed(#expr, __FILE__, __LINE__), 0)))
#else
#define gawk_crt_assert(expr) ASSUME(expr)
#endif

#define gawk_crt_malloc(size)		(gawk_crt()->crt_malloc(size))
#define gawk_crt_calloc(nmemb, size)	(gawk_crt()->crt_calloc(nmemb, size))
#define gawk_crt_realloc(ptr, size)	(gawk_crt()->crt_realloc(ptr, size))
#define gawk_crt_free(ptr)		(gawk_crt()->crt_free(ptr))

#if !defined(_MSC_VER) || !defined(_PREFAST_)
#define gawk_crt_printf			gawk_crt()->crt_printf
#define gawk_crt_fprintf		gawk_crt()->crt_fprintf
#define gawk_crt_sprintf		gawk_crt()->crt_sprintf
#define gawk_crt_snprintf		gawk_crt()->crt_snprintf
#else
/* Annotate printf-like format string so compiler will check passed args.  */
int gawk_crt_printf(_Printf_format_string_ const char *format, ...);
int gawk_crt_fprintf(FILE *file, _Printf_format_string_ const char *format, ...);
int gawk_crt_sprintf(char *str, _Printf_format_string_ const char *format, ...);
int gawk_crt_snprintf(char *str, size_t size, _Printf_format_string_ const char *format, ...);
#endif

#define gawk_crt_stdin()		(gawk_crt()->crt_stdin)
#define gawk_crt_stdout()		(gawk_crt()->crt_stdout)
#define gawk_crt_stderr()		(gawk_crt()->crt_stderr)

static inline int
r_gawk_close(const gawk_crt_api_t *crt,
	     fd_t fd)
{
	return crt->crt_close(fd);
}

static inline fd_t
r_gawk_dup(const gawk_crt_api_t *crt,
	   fd_t oldfd)
{
	return crt->crt_dup(oldfd);
}

static inline fd_t
r_gawk_dup2(const gawk_crt_api_t *crt,
	    fd_t oldfd, fd_t newfd)
{
	return crt->crt_dup2(oldfd, newfd);
}

static inline ssize_t
r_gawk_read(const gawk_crt_api_t *crt,
	    fd_t fd, void *buf, size_t count)
{
	return crt->crt_read(fd, buf, count);
}

static inline ssize_t
r_gawk_write(const gawk_crt_api_t *crt,
	     fd_t fd, const void *buf, size_t count)
{
	return crt->crt_write(fd, buf, count);
}

static inline long long
r_gawk_lseek(const gawk_crt_api_t *crt,
	      fd_t fd, long long offset, int whence)
{
	return crt->crt_lseek(fd, offset, whence);
}

static inline long long
r_gawk_tell(const gawk_crt_api_t *crt,
	    fd_t fd)
{
	return crt->crt_tell(fd);
}

static inline int
r_gawk_fsync(const gawk_crt_api_t *crt,
	     fd_t fd)
{
	return crt->crt_fsync(fd);
}

static inline int
r_gawk_fstat(const gawk_crt_api_t *crt,
	     fd_t fd, gawk_stat_t *buf)
{
	return crt->crt_fstat(fd, buf);
}

#define gawk_crt_open				(fd_t) gawk_crt()->crt_open
#define gawk_crt_close(fd)			r_gawk_close(gawk_crt(), fd)
#define gawk_crt_dup(oldfd)			r_gawk_dup(gawk_crt(), oldfd)
#define gawk_crt_dup2(oldfd, newfd)		r_gawk_dup2(gawk_crt(), oldfd, newfd)

#define gawk_crt_read(fd, buf, count)		r_gawk_read(gawk_crt(), fd, buf, count)
#define gawk_crt_write(fd, buf, count)		r_gawk_write(gawk_crt(), fd, buf, count)
#define gawk_crt_lseek(fd, offset, whence)	r_gawk_lseek(gawk_crt(), fd, offset, whence)
#define gawk_crt_tell(fd)			r_gawk_tell(gawk_crt(), fd)
#define gawk_crt_fsync(fd)			r_gawk_fsync(gawk_crt(), fd)

#define gawk_crt_fflush(stream)			(gawk_crt()->crt_fflush(stream))
#define gawk_crt_fgetpos(stream, pos)		(gawk_crt()->crt_fgetpos(stream, pos))
#define gawk_crt_fsetpos(stream, pos)		(gawk_crt()->crt_fsetpos(stream, pos))
#define gawk_crt_rewind(stream)			(gawk_crt()->crt_rewind(stream))
#define gawk_crt_fseek(stream, offset, whence)	(gawk_crt()->crt_fseek(stream, offset, whence))
#define gawk_crt_ftell(stream)			(gawk_crt()->crt_ftell(stream))

#define gawk_crt_fopen(filename, mode)		(gawk_crt()->crt_fopen(filename, mode))
#define gawk_crt_fclose(stream)			(gawk_crt()->crt_fclose(stream))

#define gawk_crt_fread(buf, size, nmemb, stream) \
	(gawk_crt()->crt_fread(buf, size, nmemb, stream))
#define gawk_crt_fwrite(buf, size, nmemb, stream) \
	(gawk_crt()->crt_fwrite(buf, size, nmemb, stream))

#define gawk_crt_clearerr(stream)		(gawk_crt()->crt_clearerr(stream))
#define gawk_crt_feof(stream)			(gawk_crt()->crt_feof(stream))
#define gawk_crt_ferror(stream)			(gawk_crt()->crt_ferror(stream))
#define gawk_crt_fileno(stream)			(gawk_crt()->crt_fileno(stream))

#define gawk_crt_putchar(c)			(gawk_crt()->crt_putchar(c))
#define gawk_crt_fputc(c, stream)		(gawk_crt()->crt_fputc(c, stream))
/* same as fputc */
#define gawk_crt_putc(c, stream)		(gawk_crt()->crt_fputc(c, stream))
#define gawk_crt_getchar()			(gawk_crt()->crt_getchar())
#define gawk_crt_fgetc(stream)			(gawk_crt()->crt_fgetc(stream))
/* same as fgetc */
#define gawk_crt_getc(stream)			(gawk_crt()->crt_fgetc(stream))
#define gawk_crt_puts(s)			(gawk_crt()->crt_puts(s))
#define gawk_crt_fputs(s, stream)		(gawk_crt()->crt_fputs(s, stream))

#define gawk_crt_fgets(s, size, stream)		(gawk_crt()->crt_fgets(s, size, stream))
#define gawk_crt_ungetc(c, stream)		(gawk_crt()->crt_ungetc(s, stream))

#define gawk_crt_mkdir(dirname)			(gawk_crt()->crt_mkdir(dirname))
#define gawk_crt_rmdir(dirname)			(gawk_crt()->crt_rmdir(dirname))
#define gawk_crt_remove(pathname)		(gawk_crt()->crt_remove(pathname))
#define gawk_crt_unlink(pathname)		(gawk_crt()->crt_unlink(pathname))
#define gawk_crt_rename(old_name, new_name)	(gawk_crt()->crt_rename(old_name, new_name))
#define gawk_crt_chdir(path)			(gawk_crt()->crt_chdir(path))

#if defined(_MSC_VER) || defined(__MINGW32__)
#define gawk_crt_xpathwc(path, buf, buf_size)	(gawk_crt()->crt_xpathwc(path, buf, buf_size))
#define gawk_crt_xfstat(h, path, buf)		(gawk_crt()->crt_xfstat(h, path, buf))
#define gawk_crt_xwstat(path, buf, dont_follow)	(gawk_crt()->crt_xwstat(path, buf, dont_follow))
#define gawk_crt_xstat(path, buf)		(gawk_crt()->crt_xstat(path, buf))
#define gawk_crt_xlstat(path, buf)		(gawk_crt()->crt_xlstat(path, buf))
#define gawk_crt_xstat_root(path, buf)		(gawk_crt()->crt_xstat_root(path, buf))
#else
#define gawk_crt_xstat(path, buf)		(gawk_crt()->crt_stat(path, buf))
#endif

#define gawk_crt_stat(path, buf)		(gawk_crt()->crt_stat(path, buf))
#define gawk_crt_fstat(fd, buf)			r_gawk_fstat(gawk_crt(), fd, buf)
#define gawk_crt_chmod(path, mode)		(gawk_crt()->crt_chmod(path, mode))

#if defined(_MSC_VER) || defined(__MINGW32__)
#define gawk_crt_opendirfd(path, dfd)		(gawk_crt()->crt_opendirfd(path, dfd))
#define gawk_crt_closedirfd(dfd)		(gawk_crt()->crt_closedirfd(dfd))
#define gawk_crt_fchdir(dfd)			(gawk_crt()->crt_fchdir(dfd))
#define gawk_crt_opendir(path)			(gawk_crt()->crt_opendir(path))
#define gawk_crt_closedir(dirp)			(gawk_crt()->crt_closedir(dirp))
#define gawk_crt_readdir(dirp)			(gawk_crt()->crt_readdir(dirp))
#define gawk_crt_wreadlinkfd(h, buf, bufsiz)	(gawk_crt()->crt_wreadlinkfd(h, buf, bufsiz))
#define gawk_crt_wreadlink(path, buf, bufsiz)	(gawk_crt()->crt_wreadlink(path, buf, bufsiz))
#define gawk_crt_readlinkfd(h, buf, bufsiz)	(gawk_crt()->crt_readlinkfd(h, buf, bufsiz))
#define gawk_crt_readlink(path, buf, bufsiz)	(gawk_crt()->crt_readlink(path, buf, bufsiz))
#endif

#define gawk_crt_setlocale(category, locale)	(gawk_crt()->crt_setlocale(category, locale))
#define gawk_crt_localeconv()			(gawk_crt()->crt_localeconv())

#define gawk_crt_mb_cur_max()			(gawk_crt()->crt_mb_cur_max())

#define gawk_crt_btowc(c)			(gawk_crt()->crt_btowc(c))

#define gawk_crt_mblen(s, n)			(gawk_crt()->crt_mblen(s, n))
#define gawk_crt_mbrlen(s, n, ps)		(gawk_crt()->crt_mbrlen(s, n, ps))

#define gawk_crt_mbstowcs(wcstr, mbstr, count)	(gawk_crt()->crt_mbstowcs(wcstr, mbstr, count))
#define gawk_crt_wcstombs(mbstr, wcstr, count)	(gawk_crt()->crt_wcstombs(mbstr, wcstr, count))

#define gawk_crt_vprintf(format, ap)		(gawk_crt()->crt_vprintf(format, ap))
#define gawk_crt_vfprintf(stream, format, ap)	(gawk_crt()->crt_vfprintf(stream, format, ap))
#define gawk_crt_vsprintf(str, format, ap)	(gawk_crt()->crt_vsprintf(str, format, ap))

#define gawk_crt_vsnprintf(str, size, format, ap) \
	(gawk_crt()->crt_vsnprintf(str, size, format, ap))

#define gawk_crt_errno_p()			(gawk_crt()->crt_errno_p())
#define gawk_crt_strerror(error_number)		(gawk_crt()->crt_strerror(error_number))

static inline fd_t
r_gawk_mkstemp(const gawk_crt_api_t *crt,
	       char *templ)
{
	return crt->crt_mkstemp(templ);
}

#define gawk_crt_mkstemp(templ)			r_gawk_mkstemp(gawk_crt(), templ)

#define gawk_crt_environ()        		(gawk_crt()->crt_environ())
#define gawk_crt_getenv(name)        		(gawk_crt()->crt_getenv(name))
#define gawk_crt_setenv(name, value, overwrite) (gawk_crt()->crt_setenv(name, value, overwrite))
#define gawk_crt_unsetenv(name)			(gawk_crt()->crt_unsetenv(name))
#define gawk_crt_clearenv()        		(gawk_crt()->crt_clearenv())

#define gawk_crt_strcoll(s1, s2)		(gawk_crt()->crt_strcoll(s1, s2))
#define gawk_crt_tolower(c)			(gawk_crt()->crt_tolower(c))
#define gawk_crt_toupper(c)			(gawk_crt()->crt_toupper(c))
#define gawk_crt_isascii(c)			(gawk_crt()->crt_isascii(c))
#define gawk_crt_isalnum(c)			(gawk_crt()->crt_isalnum(c))
#define gawk_crt_isalpha(c)			(gawk_crt()->crt_isalpha(c))
#define gawk_crt_isblank(c)			(gawk_crt()->crt_isblank(c))
#define gawk_crt_iscntrl(c)			(gawk_crt()->crt_iscntrl(c))
#define gawk_crt_isdigit(c)			(gawk_crt()->crt_isdigit(c))
#define gawk_crt_isgraph(c)			(gawk_crt()->crt_isgraph(c))
#define gawk_crt_islower(c)			(gawk_crt()->crt_islower(c))
#define gawk_crt_isprint(c)			(gawk_crt()->crt_isprint(c))
#define gawk_crt_ispunct(c)			(gawk_crt()->crt_ispunct(c))
#define gawk_crt_isspace(c)			(gawk_crt()->crt_isspace(c))
#define gawk_crt_isupper(c)			(gawk_crt()->crt_isupper(c))
#define gawk_crt_isxdigit(c)			(gawk_crt()->crt_isxdigit(c))

#if !defined _MSC_VER && !defined __MINGW32__

#define gawk_crt_wcscoll(s1, s2)		(gawk_crt()->crt_wcscoll(s1, s2))
#define gawk_crt_towlower(c)			(gawk_crt()->crt_towlower(c))
#define gawk_crt_towupper(c)			(gawk_crt()->crt_towupper(c))
#define gawk_crt_iswascii(c)			(gawk_crt()->crt_iswascii(c))
#define gawk_crt_iswalnum(c)			(gawk_crt()->crt_iswalnum(c))
#define gawk_crt_iswalpha(c)			(gawk_crt()->crt_iswalpha(c))
#define gawk_crt_iswblank(c)			(gawk_crt()->crt_iswblank(c))
#define gawk_crt_iswcntrl(c)			(gawk_crt()->crt_iswcntrl(c))
#define gawk_crt_iswdigit(c)			(gawk_crt()->crt_iswdigit(c))
#define gawk_crt_iswgraph(c)			(gawk_crt()->crt_iswgraph(c))
#define gawk_crt_iswlower(c)			(gawk_crt()->crt_iswlower(c))
#define gawk_crt_iswprint(c)			(gawk_crt()->crt_iswprint(c))
#define gawk_crt_iswpunct(c)			(gawk_crt()->crt_iswpunct(c))
#define gawk_crt_iswspace(c)			(gawk_crt()->crt_iswspace(c))
#define gawk_crt_iswupper(c)			(gawk_crt()->crt_iswupper(c))
#define gawk_crt_iswxdigit(c)			(gawk_crt()->crt_iswxdigit(c))
#define gawk_crt_wctype(name)			(gawk_crt()->crt_wctype(name))
#define gawk_crt_iswctype(c, desc)		(gawk_crt()->crt_iswctype(c, desc))

#define gawk_crt_mbtowc(pwc, s, n)		(gawk_crt()->crt_mbtowc(pwc, s, n))
#define gawk_crt_mbrtowc(pwc, s, n, ps)		(gawk_crt()->crt_mbrtowc(pwc, s, n, ps))
#define gawk_crt_wctomb(s, wc)			(gawk_crt()->crt_wctomb(s, wc))
#define gawk_crt_wcrtomb(s, wc, ps)		(gawk_crt()->crt_wcrtomb(s, wc, ps))

#else /* _MSC_VER || __MINGW32__ */

#define gawk_crt_c32scoll(s1, s2)		(gawk_crt()->crt_c32scoll(s1, s2))
#define gawk_crt_c32tolower(c)			(gawk_crt()->crt_c32tolower(c))
#define gawk_crt_c32toupper(c)			(gawk_crt()->crt_c32toupper(c))
#define gawk_crt_c32isascii(c)			(gawk_crt()->crt_c32isascii(c))
#define gawk_crt_c32isalnum(c)                  (gawk_crt()->crt_c32isalnum(c))
#define gawk_crt_c32isalpha(c)                  (gawk_crt()->crt_c32isalpha(c))
#define gawk_crt_c32isblank(c)                  (gawk_crt()->crt_c32isblank(c))
#define gawk_crt_c32iscntrl(c)                  (gawk_crt()->crt_c32iscntrl(c))
#define gawk_crt_c32isdigit(c)                  (gawk_crt()->crt_c32isdigit(c))
#define gawk_crt_c32isgraph(c)                  (gawk_crt()->crt_c32isgraph(c))
#define gawk_crt_c32islower(c)                  (gawk_crt()->crt_c32islower(c))
#define gawk_crt_c32isprint(c)                  (gawk_crt()->crt_c32isprint(c))
#define gawk_crt_c32ispunct(c)                  (gawk_crt()->crt_c32ispunct(c))
#define gawk_crt_c32isspace(c)                  (gawk_crt()->crt_c32isspace(c))
#define gawk_crt_c32isupper(c)                  (gawk_crt()->crt_c32isupper(c))
#define gawk_crt_c32isxdigit(c)                 (gawk_crt()->crt_c32isxdigit(c))
#define gawk_crt_c32ctype(name)			(gawk_crt()->crt_c32ctype(name))
#define gawk_crt_c32isctype(c, desc)		(gawk_crt()->crt_c32isctype(c, desc))

#define gawk_crt_mbrtoc16(pwc, s, n, ps)	(gawk_crt()->crt_mbrtoc16(pwc, s, n, ps))
#define gawk_crt_mbrtoc32(pwi, s, c, ps)	(gawk_crt()->crt_mbrtoc32(pwi, s, n, ps))
#define gawk_crt_c16rtomb(s, wc, ps)		(gawk_crt()->crt_c16rtomb(s, wc, ps))
#define gawk_crt_c32rtomb(s, wi, ps)		(gawk_crt()->crt_c32rtomb(s, wi, ps))

#define gawk_crt_mbstoc32s(dst, src, n)		(gawk_crt()->crt_mbstoc32s(dst, src, n))
#define gawk_crt_c32stombs(dst, src, n)		(gawk_crt()->crt_c32stombs(dst, src, n))
#define gawk_crt_wcstoc32s(dst, src, n)		(gawk_crt()->crt_wcstoc32s(dst, src, n))
#define gawk_crt_c32stowcs(dst, src, n)		(gawk_crt()->crt_c32stowcs(dst, src, n))

#define gawk_crt_c32slen(s)			(gawk_crt()->crt_c32slen(s))
#define gawk_crt_c32schr(s, c)			(gawk_crt()->crt_c32schr(s, c))
#define gawk_crt_c32srchr(s, c)			(gawk_crt()->crt_c32srchr(s, c))
#define gawk_crt_c32schrnul(s, c)		(gawk_crt()->crt_c32schrnul(s, c))

#endif /* _MSC_VER || __MINGW32__ */

/* 2) Add more CRT replacements here.  */

#ifndef GAWK_DONT_REDEFINE_CRT

#ifdef malloc
#undef malloc
#endif
#define malloc gawk_crt_malloc

#ifdef calloc
#undef calloc
#endif
#define calloc gawk_crt_calloc

#ifdef realloc
#undef realloc
#endif
#define realloc gawk_crt_realloc

#ifdef free
#undef free
#endif
#define free gawk_crt_free

#ifdef printf
#undef printf
#endif
#define printf gawk_crt_printf

#ifdef assert
#undef assert
#endif
#define assert(expr) gawk_crt_assert(expr)

#ifdef stdin
#undef stdin
#endif
#define stdin gawk_crt_stdin()

#ifdef stdout
#undef stdout
#endif
#define stdout gawk_crt_stdout()

#ifdef stderr
#undef stderr
#endif
#define stderr gawk_crt_stderr()

#ifdef open
#undef open
#endif
#define open gawk_crt_open

#ifdef close
#undef close
#endif
#define close gawk_crt_close

#ifdef dup
#undef dup
#endif
#define dup gawk_crt_dup

#ifdef dup2
#undef dup2
#endif
#define dup2 gawk_crt_dup2

#ifdef read
#undef read
#endif
#define read gawk_crt_read

#ifdef write
#undef write
#endif
#define write gawk_crt_write

#ifdef lseek
#undef lseek
#endif
#define lseek gawk_crt_lseek

#ifdef tell
#undef tell
#endif
#define tell gawk_crt_tell

#ifdef fsync
#undef fsync
#endif
#define fsync gawk_crt_fsync

#ifdef fflush
#undef fflush
#endif
#define fflush gawk_crt_fflush

#ifdef fgetpos
#undef fgetpos
#endif
#define fgetpos gawk_crt_fgetpos

#ifdef fsetpos
#undef fsetpos
#endif
#define fsetpos gawk_crt_fsetpos

#ifdef rewind
#undef rewind
#endif
#define rewind gawk_crt_rewind

#ifdef fseek
#undef fseek
#endif
#define fseek gawk_crt_fseek

#ifdef ftell
#undef ftell
#endif
#define ftell gawk_crt_ftell

#ifdef fopen
#undef fopen
#endif
#define fopen gawk_crt_fopen

#ifdef fclose
#undef fclose
#endif
#define fclose gawk_crt_fclose

#ifdef fread
#undef fread
#endif
#define fread gawk_crt_fread

#ifdef fwrite
#undef fwrite
#endif
#define fwrite gawk_crt_fwrite

#ifdef clearerr
#undef clearerr
#endif
#define clearerr gawk_crt_clearerr

#ifdef feof
#undef feof
#endif
#define feof gawk_crt_feof

#ifdef ferror
#undef ferror
#endif
#define ferror gawk_crt_ferror

#ifdef fileno
#undef fileno
#endif
#define fileno gawk_crt_fileno

#ifdef putchar
#undef putchar
#endif
#define putchar gawk_crt_putchar

#ifdef fputc
#undef fputc
#endif
#define fputc gawk_crt_fputc

#ifdef putc
#undef putc
#endif
#define putc gawk_crt_putc

#ifdef getchar
#undef getchar
#endif
#define getchar gawk_crt_getchar

#ifdef fgetc
#undef fgetc
#endif
#define fgetc gawk_crt_fgetc

#ifdef getc
#undef getc
#endif
#define getc gawk_crt_getc

#ifdef puts
#undef puts
#endif
#define puts gawk_crt_puts

#ifdef fputs
#undef fputs
#endif
#define fputs gawk_crt_fputs

#ifdef fgets
#undef fgets
#endif
#define fgets gawk_crt_fgets

#ifdef ungetc
#undef ungetc
#endif
#define ungetc gawk_crt_ungetc

#ifdef mkdir
#undef mkdir
#endif
#define mkdir gawk_crt_mkdir

#ifdef rmdir
#undef rmdir
#endif
#define rmdir gawk_crt_rmdir

#ifdef remove
#undef remove
#endif
#define remove gawk_crt_remove

#ifdef unlink
#undef unlink
#endif
#define unlink gawk_crt_unlink

#ifdef rename
#undef rename
#endif
#define rename gawk_crt_rename

#ifdef chdir
#undef chdir
#endif
#define chdir gawk_crt_chdir

#if defined(_MSC_VER) || defined(__MINGW32__)

#ifdef xpathwc
#undef xpathwc
#endif
#define xpathwc gawk_crt_xpathwc

#ifdef xfstat
#undef xfstat
#endif
#define xfstat gawk_crt_xfstat

#ifdef xwstat
#undef xwstat
#endif
#define xwstat gawk_crt_xwstat

#ifdef xlstat
#undef xlstat
#endif
#define xlstat gawk_crt_xlstat

#ifdef xstat_root
#undef xstat_root
#endif
#define xstat_root gawk_crt_xstat_root

#endif /* _MSC_VER || __MINGW32__ */

#ifdef xstat
#undef xstat
#endif
#define xstat(name, buf) gawk_crt_xstat(name, buf)

#ifdef stat
#undef stat
#endif
#define stat(name, buf) gawk_crt_stat(name, buf)

#ifdef fstat
#undef fstat
#endif
#define fstat gawk_crt_fstat

#ifdef chmod
#undef chmod
#endif
#define chmod gawk_crt_chmod

#ifdef opendirfd
#undef opendirfd
#endif
#define opendirfd gawk_crt_opendirfd

#ifdef closedirfd
#undef closedirfd
#endif
#define closedirfd gawk_crt_closedirfd

#ifdef fchdir
#undef fchdir
#endif
#define fchdir gawk_crt_fchdir

#ifdef opendir
#undef opendir
#endif
#define opendir gawk_crt_opendir

#ifdef closedir
#undef closedir
#endif
#define closedir gawk_crt_closedir

#ifdef readdir
#undef readdir
#endif
#define readdir gawk_crt_readdir

#ifdef wreadlinkfd
#undef wreadlinkfd
#endif
#define wreadlinkfd gawk_crt_wreadlinkfd

#ifdef wreadlink
#undef wreadlink
#endif
#define wreadlink gawk_crt_wreadlink

#ifdef readlinkfd
#undef readlinkfd
#endif
#define readlinkfd gawk_crt_readlinkfd

#ifdef readlink
#undef readlink
#endif
#define readlink gawk_crt_readlink

#ifdef setlocale
#undef setlocale
#endif
#define setlocale gawk_crt_setlocale

#ifdef localeconv
#undef localeconv
#endif
#define localeconv gawk_crt_localeconv

#ifdef MB_CUR_MAX
#undef MB_CUR_MAX
#endif
#define MB_CUR_MAX gawk_crt_mb_cur_max()

#ifdef btowc
#undef btowc
#endif
#define btowc gawk_crt_btowc

#ifdef mblen
#undef mblen
#endif
#define mblen gawk_crt_mblen

#ifdef mbrlen
#undef mbrlen
#endif
#define mbrlen gawk_crt_mbrlen

#ifdef mbstowcs
#undef mbstowcs
#endif
#define mbstowcs gawk_crt_mbstowcs

#ifdef wcstombs
#undef wcstombs
#endif
#define wcstombs gawk_crt_wcstombs

#ifdef fprintf
#undef fprintf
#endif
#define fprintf gawk_crt_fprintf

#ifdef sprintf
#undef sprintf
#endif
#define sprintf gawk_crt_sprintf

#ifdef snprintf
#undef snprintf
#endif
#define snprintf gawk_crt_snprintf

#ifdef vprintf
#undef vprintf
#endif
#define vprintf gawk_crt_vprintf

#ifdef vfprintf
#undef vfprintf
#endif
#define vfprintf gawk_crt_vfprintf

#ifdef vsprintf
#undef vsprintf
#endif
#define vsprintf gawk_crt_vsprintf

#ifdef vsnprintf
#undef vsnprintf
#endif
#define vsnprintf gawk_crt_vsnprintf

#ifdef errno
#undef errno
#endif
#define errno (*gawk_crt_errno_p())

#ifdef strerror
#undef strerror
#endif
#define strerror gawk_crt_strerror

#ifdef mkstemp
#undef mkstemp
#endif
#define mkstemp gawk_crt_mkstemp

#ifdef environ
#undef environ
#endif
#define environ gawk_crt_environ()

#ifdef getenv
#undef getenv
#endif
#define getenv gawk_crt_getenv

#ifdef setenv
#undef setenv
#endif
#define setenv gawk_crt_setenv

#ifdef unsetenv
#undef unsetenv
#endif
#define unsetenv gawk_crt_unsetenv

#ifdef clearenv
#undef clearenv
#endif
#define clearenv gawk_crt_clearenv

#ifdef strcoll
#undef strcoll
#endif
#define strcoll gawk_crt_strcoll

#ifdef tolower
#undef tolower
#endif
#define tolower gawk_crt_tolower

#ifdef toupper
#undef toupper
#endif
#define toupper gawk_crt_toupper

#ifdef isascii
#undef isascii
#endif
#define isascii gawk_crt_isascii

#ifdef isalnum
#undef isalnum
#endif
#define isalnum gawk_crt_isalnum

#ifdef isalpha
#undef isalpha
#endif
#define isalpha gawk_crt_isalpha

#ifdef isblank
#undef isblank
#endif
#define isblank gawk_crt_isblank

#ifdef iscntrl
#undef iscntrl
#endif
#define iscntrl gawk_crt_iscntrl

#ifdef isdigit
#undef isdigit
#endif
#define isdigit gawk_crt_isdigit

#ifdef isgraph
#undef isgraph
#endif
#define isgraph gawk_crt_isgraph

#ifdef islower
#undef islower
#endif
#define islower gawk_crt_islower

#ifdef isprint
#undef isprint
#endif
#define isprint gawk_crt_isprint

#ifdef ispunct
#undef ispunct
#endif
#define ispunct gawk_crt_ispunct

#ifdef isspace
#undef isspace
#endif
#define isspace gawk_crt_isspace

#ifdef isupper
#undef isupper
#endif
#define isupper gawk_crt_isupper

#ifdef isxdigit
#undef isxdigit
#endif
#define isxdigit gawk_crt_isxdigit

#ifdef wcscoll
#undef wcscoll
#endif
#ifdef towlower
#undef towlower
#endif
#ifdef towupper
#undef towupper
#endif
#ifdef iswascii
#undef iswascii
#endif
#ifdef iswalnum
#undef iswalnum
#endif
#ifdef iswalpha
#undef iswalpha
#endif
#ifdef iswblank
#undef iswblank
#endif
#ifdef iswcntrl
#undef iswcntrl
#endif
#ifdef iswdigit
#undef iswdigit
#endif
#ifdef iswgraph
#undef iswgraph
#endif
#ifdef iswlower
#undef iswlower
#endif
#ifdef iswprint
#undef iswprint
#endif
#ifdef iswpunct
#undef iswpunct
#endif
#ifdef iswspace
#undef iswspace
#endif
#ifdef iswupper
#undef iswupper
#endif
#ifdef iswxdigit
#undef iswxdigit
#endif
#ifdef wctype
#undef wctype
#endif
#ifdef iswctype
#undef iswctype
#endif

#ifdef mbtowc
#undef mbtowc
#endif
#ifdef mbrtowc
#undef mbrtowc
#endif
#ifdef wctomb
#undef wctomb
#endif
#ifdef wcrtomb
#undef wcrtomb
#endif

#if !defined _MSC_VER && !defined __MINGW32__

#define wcscoll gawk_crt_wcscoll
#define towlower gawk_crt_towlower
#define towupper gawk_crt_towupper
#define iswascii gawk_crt_iswascii
#define iswalnum gawk_crt_iswalnum
#define iswalpha gawk_crt_iswalpha
#define iswblank gawk_crt_iswblank
#define iswcntrl gawk_crt_iswcntrl
#define iswdigit gawk_crt_iswdigit
#define iswgraph gawk_crt_iswgraph
#define iswlower gawk_crt_iswlower
#define iswprint gawk_crt_iswprint
#define iswpunct gawk_crt_iswpunct
#define iswspace gawk_crt_iswspace
#define iswupper gawk_crt_iswupper
#define iswxdigit gawk_crt_iswxdigit
#define wctype gawk_crt_wctype
#define iswctype gawk_crt_iswctype

#define mbtowc gawk_crt_mbtowc
#define mbrtowc gawk_crt_mbrtowc
#define wctomb gawk_crt_wctomb
#define wcrtomb gawk_crt_wcrtomb

#else /* _MSC_VER || __MINGW32__ */

#define wcscoll use_c32scoll_instead
#define towlower use_c32tolower_instead
#define towupper use_c32toupper_instead
#define iswascii use_c32isascii_instead
#define iswalnum use_c32isalnum_instead
#define iswalpha use_c32isalpha_instead
#define iswblank use_c32isblank_instead
#define iswcntrl use_c32iscntrl_instead
#define iswdigit use_c32isdigit_instead
#define iswgraph use_c32isgraph_instead
#define iswlower use_c32islower_instead
#define iswprint use_c32isprint_instead
#define iswpunct use_c32ispunct_instead
#define iswspace use_c32isspace_instead
#define iswupper use_c32isupper_instead
#define iswxdigit use_c32isxdigit_instead
#define wctype use_c32ctype_instead
#define iswctype use_c32isctype_instead

#define mbtowc use_mbrtoc16_or_mbrtoc32_instead
#define mbrtowc use_mbrtoc16_or_mbrtoc32_instead
#define wctomb use_c16rtomb_or_c32rtomb_intead
#define wcrtomb use_c16rtomb_or_c32rtomb_intead

#ifdef c32scoll
#undef c32scoll
#endif
#define c32scoll gawk_crt_c32scoll

#ifdef c32tolower
#undef c32tolower
#endif
#define c32tolower gawk_crt_c32tolower

#ifdef c32toupper
#undef c32toupper
#endif
#define c32toupper gawk_crt_c32toupper

#ifdef c32isascii
#undef c32isascii
#endif
#define c32isascii gawk_crt_c32isascii

#ifdef c32isalnum
#undef c32isalnum
#endif
#define c32isalnum gawk_crt_c32isalnum

#ifdef c32isalpha
#undef c32isalpha
#endif
#define c32isalpha gawk_crt_c32isalpha

#ifdef c32isblank
#undef c32isblank
#endif
#define c32isblank gawk_crt_c32isblank

#ifdef c32iscntrl
#undef c32iscntrl
#endif
#define c32iscntrl gawk_crt_c32iscntrl

#ifdef c32isdigit
#undef c32isdigit
#endif
#define c32isdigit gawk_crt_c32isdigit

#ifdef c32isgraph
#undef c32isgraph
#endif
#define c32isgraph gawk_crt_c32isgraph

#ifdef c32islower
#undef c32islower
#endif
#define c32islower gawk_crt_c32islower

#ifdef c32isprint
#undef c32isprint
#endif
#define c32isprint gawk_crt_c32isprint

#ifdef c32ispunct
#undef c32ispunct
#endif
#define c32ispunct gawk_crt_c32ispunct

#ifdef c32isspace
#undef c32isspace
#endif
#define c32isspace gawk_crt_c32isspace

#ifdef c32isupper
#undef c32isupper
#endif
#define c32isupper gawk_crt_c32isupper

#ifdef c32isxdigit
#undef c32isxdigit
#endif
#define c32isxdigit gawk_crt_c32isxdigit

#ifdef c32ctype
#undef c32ctype
#endif
#define c32ctype gawk_crt_c32ctype

#ifdef c32isctype
#undef c32isctype
#endif
#define c32isctype gawk_crt_c32isctype

#ifdef mbrtoc16
#undef mbrtoc16
#endif
#define mbrtoc16 gawk_crt_mbrtoc16

#ifdef mbrtoc32
#undef mbrtoc32
#endif
#define mbrtoc32 gawk_crt_mbrtoc32

#ifdef c16rtomb
#undef c16rtomb
#endif
#define c16rtomb gawk_crt_c16rtomb

#ifdef c32rtomb
#undef c32rtomb
#endif
#define c32rtomb gawk_crt_c32rtomb

#ifdef mbstoc32s
#undef mbstoc32s
#endif
#define mbstoc32s gawk_crt_mbstoc32s

#ifdef c32stombs
#undef c32stombs
#endif
#define c32stombs gawk_crt_c32stombs

#ifdef wcstoc32s
#undef wcstoc32s
#endif
#define wcstoc32s gawk_crt_wcstoc32s

#ifdef c32stowcs
#undef c32stowcs
#endif
#define c32stowcs gawk_crt_c32stowcs

#ifdef c32slen
#undef c32slen
#endif
#define c32slen gawk_crt_c32slen

#ifdef c32schr
#undef c32schr
#endif
#define c32schr gawk_crt_c32schr

#ifdef c32srchr
#undef c32srchr
#endif
#define c32srchr gawk_crt_c32srchr

#ifdef c32schrnul
#undef c32schrnul
#endif
#define c32schrnul gawk_crt_c32schrnul

#endif /* _MSC_VER || __MINGW32__ */

/* 3) Add more CRT replacements here.  */

#endif /* !GAWK_DONT_REDEFINE_CRT */

#endif /* !GAWK */
