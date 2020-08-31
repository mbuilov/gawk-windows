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

struct tm;
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
	void (*crt_assert_failed)(const char *sexpr, const char *file, unsigned line);

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
	size_t (*crt_fwrite)(const void *buf, size_t size, size_t nmemb, FILE *stream);

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
	wchar_t *(*crt_xpathwc)(const char *path, wchar_t buf[], size_t buf_size);
	int (*crt_xfstat)(void *h, const wchar_t *path, struct xstat *buf);
	int (*crt_xwstat)(const wchar_t *path, struct xstat *buf, int dont_follow);
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
	int (*crt_vsnprintf)(char *str, size_t size, const char *format, va_list ap);

	int *(*crt_errno_p)(void);
	char *(*crt_strerror)(int error_number);

	ATTRIBUTE_PRINTF_PTR(format, 3, 0)
	size_t (*crt_strftime)(char *s, size_t mx, const char *fmt, const struct tm *t);

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

#if defined _MSC_VER || defined(__MINGW32__)
	long long (*crt_xtimegm)(
		unsigned year/*since 1900,>70*/,
		unsigned month/*0..11*/,
		unsigned day/*1..31*/,
		unsigned hour/*0..23*/,
		unsigned minute/*0..59*/,
		unsigned second/*0..60*/,
		unsigned *yday/*NULL?,out:0..365*/);
#endif

	void (*crt_mpz_init)(void *);
	void (*crt_mpz_clear)(void *);
	void (*crt_mpz_set_d)(void *, double);
	void (*crt_mpz_tdiv_qr)(void *, void *, const void *, const void *);
	int (*crt_mpfr_number_p)(const void *);
	int (*crt_mpfr_get_z)(void *, const void *, int);

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

static inline void *
gawk_crt_malloc(size_t size)
{
	return gawk_crt()->crt_malloc(size);
}

static inline void *
gawk_crt_calloc(size_t nmemb, size_t size)
{
	return gawk_crt()->crt_calloc(nmemb, size);
}

static inline void *
gawk_crt_realloc(void *ptr, size_t size)
{
	return gawk_crt()->crt_realloc(ptr, size);
}

static inline void
gawk_crt_free(void *ptr)
{
	gawk_crt()->crt_free(ptr);
}

#ifdef _PREFAST_
/* Annotate printf-like format string so compiler will check passed args.  */
int gawk_crt_printf(_Printf_format_string_ const char *format, ...);
int gawk_crt_fprintf(FILE *file, _Printf_format_string_ const char *format, ...);
int gawk_crt_sprintf(char *str, _Printf_format_string_ const char *format, ...);
int gawk_crt_snprintf(char *str, size_t size, _Printf_format_string_ const char *format, ...);
#else
#define gawk_crt_printf		gawk_crt()->crt_printf
#define gawk_crt_fprintf	gawk_crt()->crt_fprintf
#define gawk_crt_sprintf	gawk_crt()->crt_sprintf
#define gawk_crt_snprintf	gawk_crt()->crt_snprintf
#endif

static inline FILE *
gawk_crt_stdin(void)
{
	return gawk_crt()->crt_stdin;
}

static inline FILE *
gawk_crt_stdout(void)
{
	return gawk_crt()->crt_stdout;
}

static inline FILE *
gawk_crt_stderr(void)
{
	return gawk_crt()->crt_stderr;
}

#define gawk_crt_open		(fd_t) gawk_crt()->crt_open

static inline int
gawk_crt_close(fd_t fd)
{
	return gawk_crt()->crt_close(fd);
}

static inline fd_t
gawk_crt_dup(fd_t oldfd)
{
	return gawk_crt()->crt_dup(oldfd);
}

static inline fd_t
gawk_crt_dup2(fd_t oldfd, fd_t newfd)
{
	return gawk_crt()->crt_dup2(oldfd, newfd);
}

static inline ssize_t
gawk_crt_read(fd_t fd, void *buf, size_t count)
{
	return gawk_crt()->crt_read(fd, buf, count);
}

static inline ssize_t
gawk_crt_write(fd_t fd, const void *buf, size_t count)
{
	return gawk_crt()->crt_write(fd, buf, count);
}

static inline long long
gawk_crt_lseek(fd_t fd, long long offset, int whence)
{
	return gawk_crt()->crt_lseek(fd, offset, whence);
}

static inline long long
gawk_crt_tell(fd_t fd)
{
	return gawk_crt()->crt_tell(fd);
}

static inline int
gawk_crt_fsync(fd_t fd)
{
	return gawk_crt()->crt_fsync(fd);
}

static inline int
gawk_crt_fflush(FILE *stream)
{
	return gawk_crt()->crt_fflush(stream);
}

static inline int
gawk_crt_fgetpos(FILE *stream, fpos_t *pos)
{
	return gawk_crt()->crt_fgetpos(stream, pos);
}

static inline int
gawk_crt_fsetpos(FILE *stream, const fpos_t *pos)
{
	return gawk_crt()->crt_fsetpos(stream, pos);
}

static inline void
gawk_crt_rewind(FILE *stream)
{
	gawk_crt()->crt_rewind(stream);
}

static inline int
gawk_crt_fseek(FILE *stream, long offset, int whence)
{
	return gawk_crt()->crt_fseek(stream, offset, whence);
}

static inline long
gawk_crt_ftell(FILE *stream)
{
	return gawk_crt()->crt_ftell(stream);
}

static inline FILE *
gawk_crt_fopen(const char *filename, const char *mode)
{
	return gawk_crt()->crt_fopen(filename, mode);
}

static inline int
gawk_crt_fclose(FILE *stream)
{
	return gawk_crt()->crt_fclose(stream);
}

static inline size_t
gawk_crt_fread(void *buf, size_t size, size_t nmemb, FILE *stream)
{
	return gawk_crt()->crt_fread(buf, size, nmemb, stream);
}

static inline size_t
gawk_crt_fwrite(const void *buf, size_t size, size_t nmemb, FILE *stream)
{
	return gawk_crt()->crt_fwrite(buf, size, nmemb, stream);
}

static inline void
crt_clearerr(FILE *stream)
{
	gawk_crt()->crt_clearerr(stream);
}

static inline int
gawk_crt_feof(FILE *stream)
{
	return gawk_crt()->crt_feof(stream);
}

static inline int
gawk_crt_ferror(FILE *stream)
{
	return gawk_crt()->crt_ferror(stream);
}

static inline fd_t
gawk_crt_fileno(FILE *stream)
{
	return gawk_crt()->crt_fileno(stream);
}

static inline int
gawk_crt_putchar(int c)
{
	return gawk_crt()->crt_putchar(c);
}

static inline int
gawk_crt_fputc(int c, FILE *stream)
{
	return gawk_crt()->crt_fputc(c, stream);
}

/* same as fputc */
static inline int
gawk_crt_putc(int c, FILE *stream)
{
	return gawk_crt()->crt_fputc(c, stream);
}

static inline int
gawk_crt_getchar(void)
{
	return gawk_crt()->crt_getchar();
}

static inline int
gawk_crt_fgetc(FILE *stream)
{
	return gawk_crt()->crt_fgetc(stream);
}

/* same as fgetc */
static inline int
gawk_crt_getc(FILE *stream)
{
	return gawk_crt()->crt_fgetc(stream);
}

static inline int
gawk_crt_puts(const char *s)
{
	return gawk_crt()->crt_puts(s);
}

static inline int
gawk_crt_fputs(const char *s, FILE *stream)
{
	return gawk_crt()->crt_fputs(s, stream);
}

static inline char *
gawk_crt_fgets(char *s, int size, FILE *stream)
{
	return gawk_crt()->crt_fgets(s, size, stream);
}

static inline int
gawk_crt_ungetc(int c, FILE *stream)
{
	return gawk_crt()->crt_ungetc(c, stream);
}

static inline int
gawk_crt_mkdir(const char *dirname)
{
	return gawk_crt()->crt_mkdir(dirname);
}

static inline int
gawk_crt_rmdir(const char *dirname)
{
	return gawk_crt()->crt_rmdir(dirname);
}

static inline int
gawk_crt_remove(const char *pathname)
{
	return gawk_crt()->crt_remove(pathname);
}

static inline int
gawk_crt_unlink(const char *pathname)
{
	return gawk_crt()->crt_unlink(pathname);
}

static inline int
gawk_crt_rename(const char *old_name, const char *new_name)
{
	return gawk_crt()->crt_rename(old_name, new_name);
}

static inline int
gawk_crt_chdir(const char *path)
{
	return gawk_crt()->crt_chdir(path);
}

#if defined(_MSC_VER) || defined(__MINGW32__)

static inline wchar_t *
gawk_crt_xpathwc(const char *path, wchar_t buf[], size_t buf_size)
{
	return gawk_crt()->crt_xpathwc(path, buf, buf_size);
}

static inline int
gawk_crt_xfstat(void *h, const wchar_t *path, struct xstat *buf)
{
	return gawk_crt()->crt_xfstat(h, path, buf);
}

static inline int
gawk_crt_xwstat(const wchar_t *path, struct xstat *buf, int dont_follow)
{
	return gawk_crt()->crt_xwstat(path, buf, dont_follow);
}

static inline int
gawk_crt_xstat(const char *path, struct xstat *buf)
{
	return gawk_crt()->crt_xstat(path, buf);
}

static inline int
gawk_crt_xlstat(const char *path, struct xstat *buf)
{
	return gawk_crt()->crt_xlstat(path, buf);
}

static inline int
gawk_crt_xstat_root(const wchar_t *path, struct xstat *buf)
{
	return gawk_crt()->crt_xstat_root(path, buf);
}

#else /* !_MSC_VER && !__MINGW32__ */

static inline int
gawk_crt_xstat(const char *path, gawk_stat_t *buf)
{
	return gawk_crt()->crt_stat(path, buf);
}

#endif /* !_MSC_VER && !__MINGW32__ */

static inline int
gawk_crt_stat(const char *path, gawk_stat_t *buf)
{
	return gawk_crt()->crt_stat(path, buf);
}

static inline int
gawk_crt_fstat(fd_t fd, gawk_stat_t *buf)
{
	return gawk_crt()->crt_fstat(fd, buf);
}

static inline int
gawk_crt_chmod(const char *path, int mode)
{
	return gawk_crt()->crt_chmod(path, mode);
}

#if defined(_MSC_VER) || defined(__MINGW32__)

static inline int
gawk_crt_opendirfd(const char path[], struct dir_fd_ **const dfd)
{
	return gawk_crt()->crt_opendirfd(path, dfd);
}

static inline int
gawk_crt_closedirfd(struct dir_fd_ *dfd)
{
	return gawk_crt()->crt_closedirfd(dfd);
}

static inline int
gawk_crt_fchdir(const struct dir_fd_ *dfd)
{
	return gawk_crt()->crt_fchdir(dfd);
}

static inline struct win_find *
gawk_crt_opendir(const char path[])
{
	return gawk_crt()->crt_opendir(path);
}

static inline int
gawk_crt_closedir(struct win_find *const dirp)
{
	return gawk_crt()->crt_closedir(dirp);
}

static inline struct _WIN32_FIND_DATAW *
gawk_crt_readdir(struct win_find *const dirp)
{
	return gawk_crt()->crt_readdir(dirp);
}

static inline int
gawk_crt_wreadlinkfd(void *h, wchar_t buf[], const size_t bufsiz)
{
	return gawk_crt()->crt_wreadlinkfd(h, buf, bufsiz);
}

static inline int
gawk_crt_wreadlink(const wchar_t path[], wchar_t buf[], const size_t bufsiz)
{
	return gawk_crt()->crt_wreadlink(path, buf, bufsiz);
}

static inline int
gawk_crt_readlinkfd(void *h, char buf[], const size_t bufsiz)
{
	return gawk_crt()->crt_readlinkfd(h, buf, bufsiz);
}

static inline int
gawk_crt_readlink(const char path[], char buf[], const size_t bufsiz)
{
	return gawk_crt()->crt_readlink(path, buf, bufsiz);
}

#endif /* _MSC_VER || __MINGW32__ */

static inline char *
gawk_crt_setlocale(int category, const char *locale)
{
	return gawk_crt()->crt_setlocale(category, locale);
}

static inline struct lconv *
gawk_crt_localeconv(void)
{
	return gawk_crt()->crt_localeconv();
}

static inline int
gawk_crt_mb_cur_max(void)
{
	return gawk_crt()->crt_mb_cur_max();
}

static inline wint_t
gawk_crt_btowc(int c)
{
	return gawk_crt()->crt_btowc(c);
}

static inline int
gawk_crt_mblen(const char *s, size_t n)
{
	return gawk_crt()->crt_mblen(s, n);
}

static inline size_t
gawk_crt_mbrlen(const char *s, size_t n, mbstate_t *ps)
{
	return gawk_crt()->crt_mbrlen(s, n, ps);
}

static inline size_t
gawk_crt_mbstowcs(wchar_t *wcstr, const char *mbstr, size_t count)
{
	return gawk_crt()->crt_mbstowcs(wcstr, mbstr, count);
}

static inline size_t
gawk_crt_wcstombs(char *mbstr, const wchar_t *wcstr, size_t count)
{
	return gawk_crt()->crt_wcstombs(mbstr, wcstr, count);
}

ATTRIBUTE_PRINTF_PTR(format, 1, 0)
static inline int
gawk_crt_vprintf(const char *format, va_list ap)
{
	return gawk_crt()->crt_vprintf(format, ap);
}

ATTRIBUTE_PRINTF_PTR(format, 2, 0)
static inline int
gawk_crt_vfprintf(FILE *stream, const char *format, va_list ap)
{
	return gawk_crt()->crt_vfprintf(stream, format, ap);
}

ATTRIBUTE_PRINTF_PTR(format, 2, 0)
static inline int
gawk_crt_vsprintf(char *str, const char *format, va_list ap)
{
	return gawk_crt()->crt_vsprintf(str, format, ap);
}

ATTRIBUTE_PRINTF_PTR(format, 3, 0)
static inline int
gawk_crt_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return gawk_crt()->crt_vsnprintf(str, size, format, ap);
}

static inline int *
gawk_crt_errno_p(void)
{
	return gawk_crt()->crt_errno_p();
}

static inline char *
gawk_crt_strerror(int error_number)
{
	return gawk_crt()->crt_strerror(error_number);
}

ATTRIBUTE_PRINTF_PTR(format, 3, 0)
static inline size_t
gawk_crt_strftime(char *s, size_t mx, const char *fmt, const struct tm *t)
{
	return gawk_crt()->crt_strftime(s, mx, fmt, t);
}

static inline fd_t
gawk_crt_mkstemp(char *templ)
{
	return gawk_crt()->crt_mkstemp(templ);
}

static inline char **
gawk_crt_environ(void)
{
	return gawk_crt()->crt_environ();
}

static inline char *
gawk_crt_getenv(const char *name)
{
	return gawk_crt()->crt_getenv(name);
}

static inline int
gawk_crt_setenv(const char *name, const char *value, int overwrite)
{
	return gawk_crt()->crt_setenv(name, value, overwrite);
}

static inline int
gawk_crt_unsetenv(const char *name)
{
	return gawk_crt()->crt_unsetenv(name);
}

static inline int
gawk_crt_clearenv(void)
{
	return gawk_crt()->crt_clearenv();
}

static inline int
gawk_crt_strcoll(const char *s1, const char *s2)
{
	return gawk_crt()->crt_strcoll(s1, s2);
}

static inline int
gawk_crt_tolower(int c)
{
	return gawk_crt()->crt_tolower(c);
}

static inline int
gawk_crt_toupper(int c)
{
	return gawk_crt()->crt_toupper(c);
}

static inline int
gawk_crt_isascii(int c)
{
	return gawk_crt()->crt_isascii(c);
}

static inline int
gawk_crt_isalnum(int c)
{
	return gawk_crt()->crt_isalnum(c);
}

static inline int
gawk_crt_isalpha(int c)
{
	return gawk_crt()->crt_isalpha(c);
}

static inline int
gawk_crt_isblank(int c)
{
	return gawk_crt()->crt_isblank(c);
}

static inline int
gawk_crt_iscntrl(int c)
{
	return gawk_crt()->crt_iscntrl(c);
}

static inline int
gawk_crt_isdigit(int c)
{
	return gawk_crt()->crt_isdigit(c);
}

static inline int
gawk_crt_isgraph(int c)
{
	return gawk_crt()->crt_isgraph(c);
}

static inline int
gawk_crt_islower(int c)
{
	return gawk_crt()->crt_islower(c);
}

static inline int
gawk_crt_isprint(int c)
{
	return gawk_crt()->crt_isprint(c);
}

static inline int
gawk_crt_ispunct(int c)
{
	return gawk_crt()->crt_ispunct(c);
}

static inline int
gawk_crt_isspace(int c)
{
	return gawk_crt()->crt_isspace(c);
}

static inline int
gawk_crt_isupper(int c)
{
	return gawk_crt()->crt_isupper(c);
}

static inline int
gawk_crt_isxdigit(int c)
{
	return gawk_crt()->crt_isxdigit(c);
}

#if !defined _MSC_VER && !defined __MINGW32__

static inline int
gawk_crt_wcscoll(const wchar_t *s1, const wchar_t *s2)
{
	return gawk_crt()->crt_wcscoll(s1, s2);
}

static inline wint_t
gawk_crt_towlower(wint_t c)
{
	return gawk_crt()->crt_towlower(c);
}

static inline wint_t
gawk_crt_towupper(wint_t c)
{
	return gawk_crt()->crt_towupper(c);
}

static inline int
gawk_crt_iswascii(wint_t c)
{
	return gawk_crt()->crt_iswascii(c);
}

static inline int
gawk_crt_iswalnum(wint_t c)
{
	return gawk_crt()->crt_iswalnum(c);
}

static inline int
gawk_crt_iswalpha(wint_t c)
{
	return gawk_crt()->crt_iswalpha(c);
}

static inline int
gawk_crt_iswblank(wint_t c)
{
	return gawk_crt()->crt_iswblank(c);
}

static inline int
gawk_crt_iswcntrl(wint_t c)
{
	return gawk_crt()->crt_iswcntrl(c);
}

static inline int
gawk_crt_iswdigit(wint_t c)
{
	return gawk_crt()->crt_iswdigit(c);
}

static inline int
gawk_crt_iswgraph(wint_t c)
{
	return gawk_crt()->crt_iswgraph(c);
}

static inline int
gawk_crt_iswlower(wint_t c)
{
	return gawk_crt()->crt_iswlower(c);
}

static inline int
gawk_crt_iswprint(wint_t c)
{
	return gawk_crt()->crt_iswprint(c);
}

static inline int
gawk_crt_iswpunct(wint_t c)
{
	return gawk_crt()->crt_iswpunct(c);
}

static inline int
gawk_crt_iswspace(wint_t c)
{
	return gawk_crt()->crt_iswspace(c);
}

static inline int
gawk_crt_iswupper(wint_t c)
{
	return gawk_crt()->crt_iswupper(c);
}

static inline int
gawk_crt_iswxdigit(wint_t c)
{
	return gawk_crt()->crt_iswxdigit(c);
}

static inline wctype_t
gawk_crt_wctype(const char *name)
{
	return gawk_crt()->crt_wctype(name);
}

static inline int
gawk_crt_iswctype(wint_t c, wctype_t desc)
{
	return gawk_crt()->crt_iswctype(c, desc);
}

static inline int
gawk_crt_mbtowc(wchar_t *pwc, const char *s, size_t n)
{
	return gawk_crt()->crt_mbtowc(pwc, s, n);
}

static inline size_t
gawk_crt_mbrtowc(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
	return gawk_crt()->crt_mbrtowc(pwc, s, n, ps);
}

static inline int
gawk_crt_wctomb(char *s, wchar_t wc)
{
	return gawk_crt()->crt_wctomb(s, wc);
}

static inline size_t
gawk_crt_wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
	return gawk_crt()->crt_wcrtomb(s, wc, ps);
}

#else /* _MSC_VER || __MINGW32__ */

static inline int
gawk_crt_c32scoll(const uint32_t *s1, const uint32_t *s2)
{
	return gawk_crt()->crt_c32scoll(s1, s2);
}

static inline uint32_t
gawk_crt_c32tolower(uint32_t c)
{
	return gawk_crt()->crt_c32tolower(c);
}

static inline uint32_t
gawk_crt_c32toupper(uint32_t c)
{
	return gawk_crt()->crt_c32toupper(c);
}

static inline int
gawk_crt_c32isascii(uint32_t c)
{
	return gawk_crt()->crt_c32isascii(c);
}

static inline int
gawk_crt_c32isalnum(uint32_t c)
{
	return gawk_crt()->crt_c32isalnum(c);
}

static inline int
gawk_crt_c32isalpha(uint32_t c)
{
	return gawk_crt()->crt_c32isalpha(c);
}

static inline int
gawk_crt_c32isblank(uint32_t c)
{
	return gawk_crt()->crt_c32isblank(c);
}

static inline int
gawk_crt_c32iscntrl(uint32_t c)
{
	return gawk_crt()->crt_c32iscntrl(c);
}

static inline int
gawk_crt_c32isdigit(uint32_t c)
{
	return gawk_crt()->crt_c32isdigit(c);
}

static inline int
gawk_crt_c32isgraph(uint32_t c)
{
	return gawk_crt()->crt_c32isgraph(c);
}

static inline int
gawk_crt_c32islower(uint32_t c)
{
	return gawk_crt()->crt_c32islower(c);
}

static inline int
gawk_crt_c32isprint(uint32_t c)
{
	return gawk_crt()->crt_c32isprint(c);
}

static inline int
gawk_crt_c32ispunct(uint32_t c)
{
	return gawk_crt()->crt_c32ispunct(c);
}

static inline int
gawk_crt_c32isspace(uint32_t c)
{
	return gawk_crt()->crt_c32isspace(c);
}

static inline int
gawk_crt_c32isupper(uint32_t c)
{
	return gawk_crt()->crt_c32isupper(c);
}

static inline int
gawk_crt_c32isxdigit(uint32_t c)
{
	return gawk_crt()->crt_c32isxdigit(c);
}

static inline wctype_t
gawk_crt_c32ctype(const char *name)
{
	return gawk_crt()->crt_c32ctype(name);
}

static inline int
gawk_crt_c32isctype(uint32_t c, wctype_t desc)
{
	return gawk_crt()->crt_c32isctype(c, desc);
}

static inline size_t
gawk_crt_mbrtoc16(wchar_t *pwc, const char *s, size_t n, mbstate_t *ps)
{
	return gawk_crt()->crt_mbrtoc16(pwc, s, n, ps);
}

static inline size_t
gawk_crt_mbrtoc32(uint32_t *pwi, const char *s, size_t n, mbstate_t *ps)
{
	return gawk_crt()->crt_mbrtoc32(pwi, s, n, ps);
}

static inline size_t
gawk_crt_c16rtomb(char *s, wchar_t wc, mbstate_t *ps)
{
	return gawk_crt()->crt_c16rtomb(s, wc, ps);
}

static inline size_t
gawk_crt_c32rtomb(char *s, uint32_t wi, mbstate_t *ps)
{
	return gawk_crt()->crt_c32rtomb(s, wi, ps);
}

static inline size_t
gawk_crt_mbstoc32s(uint32_t *dst, const char *src, size_t n)
{
	return gawk_crt()->crt_mbstoc32s(dst, src, n);
}

static inline size_t
gawk_crt_c32stombs(char *dst, const uint32_t *src, size_t n)
{
	return gawk_crt()->crt_c32stombs(dst, src, n);
}

static inline size_t
gawk_crt_wcstoc32s(uint32_t *dst, const wchar_t *src, size_t n)
{
	return gawk_crt()->crt_wcstoc32s(dst, src, n);
}

static inline size_t
gawk_crt_c32stowcs(wchar_t *dst, const uint32_t *src, size_t n)
{
	return gawk_crt()->crt_c32stowcs(dst, src, n);
}

static inline size_t
gawk_crt_c32slen(const uint32_t *s)
{
	return gawk_crt()->crt_c32slen(s);
}

static inline uint32_t *
gawk_crt_c32schr(const uint32_t *s, uint32_t c)
{
	return gawk_crt()->crt_c32schr(s, c);
}

static inline uint32_t *
gawk_crt_c32srchr(const uint32_t *s, uint32_t c)
{
	return gawk_crt()->crt_c32srchr(s, c);
}

static inline uint32_t *
gawk_crt_c32schrnul(const uint32_t *s, uint32_t c)
{
	return gawk_crt()->crt_c32schrnul(s, c);
}

#endif /* _MSC_VER || __MINGW32__ */

#if defined _MSC_VER || defined __MINGW32__

static inline long long
gawk_crt_xtimegm(
	unsigned year/*since 1900,>70*/,
	unsigned month/*0..11*/,
	unsigned day/*1..31*/,
	unsigned hour/*0..23*/,
	unsigned minute/*0..59*/,
	unsigned second/*0..60*/,
	unsigned *yday/*NULL?,out:0..365*/)
{
	return gawk_crt()->crt_xtimegm(year, month, day, hour, minute, second, yday);
}

#endif /* _MSC_VER || __MINGW32__ */

#if defined __GNU_MP_VERSION && defined MPFR_VERSION_MAJOR

static inline void
gawk_crt_mpz_init(mpz_ptr p)
{
	gawk_crt()->crt_mpz_init(p);
}

static inline void
gawk_crt_mpz_clear(mpz_ptr p)
{
	gawk_crt()->crt_mpz_clear(p);
}

static inline void
gawk_crt_mpz_set_d(mpz_ptr p, double d)
{
	gawk_crt()->crt_mpz_set_d(p, d);
}

static inline void
gawk_crt_mpz_tdiv_qr(mpz_ptr quot, mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
	gawk_crt()->crt_mpz_tdiv_qr(quot, rem, dividend, divisor);
}

static inline int
gawk_crt_mpfr_number_p(mpfr_srcptr p)
{
	return gawk_crt()->crt_mpfr_number_p(p);
}

static inline int
gawk_crt_mpfr_get_z(mpz_ptr z, mpfr_srcptr f, mpfr_rnd_t r)
{
	return gawk_crt()->crt_mpfr_get_z(z, f, r);
}

#endif /* __GNU_MP_VERSION && MPFR_VERSION_MAJOR */

/* 2) Add more CRT replacements here.  */

#ifndef GAWK_DONT_REDEFINE_CRT

#ifndef gawk_crt_do_not_redefine_malloc
#ifdef malloc
#undef malloc
#endif
#define malloc gawk_crt_malloc
#endif

#ifndef gawk_crt_do_not_redefine_calloc
#ifdef calloc
#undef calloc
#endif
#define calloc gawk_crt_calloc
#endif

#ifndef gawk_crt_do_not_redefine_realloc
#ifdef realloc
#undef realloc
#endif
#define realloc gawk_crt_realloc
#endif

#ifndef gawk_crt_do_not_redefine_free
#ifdef free
#undef free
#endif
#define free gawk_crt_free
#endif

#ifndef gawk_crt_do_not_redefine_printf
#ifdef printf
#undef printf
#endif
#define printf gawk_crt_printf
#endif

#ifndef gawk_crt_do_not_redefine_assert
#ifdef assert
#undef assert
#endif
#define assert(expr) gawk_crt_assert(expr)
#endif

#ifndef gawk_crt_do_not_redefine_stdin
#ifdef stdin
#undef stdin
#endif
#define stdin gawk_crt_stdin()
#endif

#ifndef gawk_crt_do_not_redefine_stdout
#ifdef stdout
#undef stdout
#endif
#define stdout gawk_crt_stdout()
#endif

#ifndef gawk_crt_do_not_redefine_stderr
#ifdef stderr
#undef stderr
#endif
#define stderr gawk_crt_stderr()
#endif

#ifndef gawk_crt_do_not_redefine_open
#ifdef open
#undef open
#endif
#define open gawk_crt_open
#endif

#ifndef gawk_crt_do_not_redefine_close
#ifdef close
#undef close
#endif
#define close gawk_crt_close
#endif

#ifndef gawk_crt_do_not_redefine_dup
#ifdef dup
#undef dup
#endif
#define dup gawk_crt_dup
#endif

#ifndef gawk_crt_do_not_redefine_dup2
#ifdef dup2
#undef dup2
#endif
#define dup2 gawk_crt_dup2
#endif

#ifndef gawk_crt_do_not_redefine_read
#ifdef read
#undef read
#endif
#define read gawk_crt_read
#endif

#ifndef gawk_crt_do_not_redefine_write
#ifdef write
#undef write
#endif
#define write gawk_crt_write
#endif

#ifndef gawk_crt_do_not_redefine_lseek
#ifdef lseek
#undef lseek
#endif
#define lseek gawk_crt_lseek
#endif

#ifndef gawk_crt_do_not_redefine_tell
#ifdef tell
#undef tell
#endif
#define tell gawk_crt_tell
#endif

#ifndef gawk_crt_do_not_redefine_fsync
#ifdef fsync
#undef fsync
#endif
#define fsync gawk_crt_fsync
#endif

#ifndef gawk_crt_do_not_redefine_fflush
#ifdef fflush
#undef fflush
#endif
#define fflush gawk_crt_fflush
#endif

#ifndef gawk_crt_do_not_redefine_fgetpos
#ifdef fgetpos
#undef fgetpos
#endif
#define fgetpos gawk_crt_fgetpos
#endif

#ifndef gawk_crt_do_not_redefine_fsetpos
#ifdef fsetpos
#undef fsetpos
#endif
#define fsetpos gawk_crt_fsetpos
#endif

#ifndef gawk_crt_do_not_redefine_rewind
#ifdef rewind
#undef rewind
#endif
#define rewind gawk_crt_rewind
#endif

#ifndef gawk_crt_do_not_redefine_fseek
#ifdef fseek
#undef fseek
#endif
#define fseek gawk_crt_fseek
#endif

#ifndef gawk_crt_do_not_redefine_ftell
#ifdef ftell
#undef ftell
#endif
#define ftell gawk_crt_ftell
#endif

#ifndef gawk_crt_do_not_redefine_fopen
#ifdef fopen
#undef fopen
#endif
#define fopen gawk_crt_fopen
#endif

#ifndef gawk_crt_do_not_redefine_fclose
#ifdef fclose
#undef fclose
#endif
#define fclose gawk_crt_fclose
#endif

#ifndef gawk_crt_do_not_redefine_fread
#ifdef fread
#undef fread
#endif
#define fread gawk_crt_fread
#endif

#ifndef gawk_crt_do_not_redefine_fwrite
#ifdef fwrite
#undef fwrite
#endif
#define fwrite gawk_crt_fwrite
#endif

#ifndef gawk_crt_do_not_redefine_clearerr
#ifdef clearerr
#undef clearerr
#endif
#define clearerr gawk_crt_clearerr
#endif

#ifndef gawk_crt_do_not_redefine_feof
#ifdef feof
#undef feof
#endif
#define feof gawk_crt_feof
#endif

#ifndef gawk_crt_do_not_redefine_ferror
#ifdef ferror
#undef ferror
#endif
#define ferror gawk_crt_ferror
#endif

#ifndef gawk_crt_do_not_redefine_fileno
#ifdef fileno
#undef fileno
#endif
#define fileno gawk_crt_fileno
#endif

#ifndef gawk_crt_do_not_redefine_putchar
#ifdef putchar
#undef putchar
#endif
#define putchar gawk_crt_putchar
#endif

#ifndef gawk_crt_do_not_redefine_fputc
#ifdef fputc
#undef fputc
#endif
#define fputc gawk_crt_fputc
#endif

#ifndef gawk_crt_do_not_redefine_putc
#ifdef putc
#undef putc
#endif
#define putc gawk_crt_putc
#endif

#ifndef gawk_crt_do_not_redefine_getchar
#ifdef getchar
#undef getchar
#endif
#define getchar gawk_crt_getchar
#endif

#ifndef gawk_crt_do_not_redefine_fgetc
#ifdef fgetc
#undef fgetc
#endif
#define fgetc gawk_crt_fgetc
#endif

#ifndef gawk_crt_do_not_redefine_getc
#ifdef getc
#undef getc
#endif
#define getc gawk_crt_getc
#endif

#ifndef gawk_crt_do_not_redefine_puts
#ifdef puts
#undef puts
#endif
#define puts gawk_crt_puts
#endif

#ifndef gawk_crt_do_not_redefine_fputs
#ifdef fputs
#undef fputs
#endif
#define fputs gawk_crt_fputs
#endif

#ifndef gawk_crt_do_not_redefine_fgets
#ifdef fgets
#undef fgets
#endif
#define fgets gawk_crt_fgets
#endif

#ifndef gawk_crt_do_not_redefine_ungetc
#ifdef ungetc
#undef ungetc
#endif
#define ungetc gawk_crt_ungetc
#endif

#ifndef gawk_crt_do_not_redefine_mkdir
#ifdef mkdir
#undef mkdir
#endif
#define mkdir gawk_crt_mkdir
#endif

#ifndef gawk_crt_do_not_redefine_rmdir
#ifdef rmdir
#undef rmdir
#endif
#define rmdir gawk_crt_rmdir
#endif

#ifndef gawk_crt_do_not_redefine_remove
#ifdef remove
#undef remove
#endif
#define remove gawk_crt_remove
#endif

#ifndef gawk_crt_do_not_redefine_unlink
#ifdef unlink
#undef unlink
#endif
#define unlink gawk_crt_unlink
#endif

#ifndef gawk_crt_do_not_redefine_rename
#ifdef rename
#undef rename
#endif
#define rename gawk_crt_rename
#endif

#ifndef gawk_crt_do_not_redefine_chdir
#ifdef chdir
#undef chdir
#endif
#define chdir gawk_crt_chdir
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)

#ifndef gawk_crt_do_not_redefine_xpathwc
#ifdef xpathwc
#undef xpathwc
#endif
#define xpathwc gawk_crt_xpathwc
#endif

#ifndef gawk_crt_do_not_redefine_xfstat
#ifdef xfstat
#undef xfstat
#endif
#define xfstat gawk_crt_xfstat
#endif

#ifndef gawk_crt_do_not_redefine_xwstat
#ifdef xwstat
#undef xwstat
#endif
#define xwstat gawk_crt_xwstat
#endif

#ifndef gawk_crt_do_not_redefine_xlstat
#ifdef xlstat
#undef xlstat
#endif
#define xlstat gawk_crt_xlstat
#endif

#ifndef gawk_crt_do_not_redefine_xstat_root
#ifdef xstat_root
#undef xstat_root
#endif
#define xstat_root gawk_crt_xstat_root
#endif

#endif /* _MSC_VER || __MINGW32__ */

#ifndef gawk_crt_do_not_redefine_xstat
#ifdef xstat
#undef xstat
#endif
#define xstat(name, buf) gawk_crt_xstat(name, buf)
#endif

#ifndef gawk_crt_do_not_redefine_stat
#ifdef stat
#undef stat
#endif
#define stat(name, buf) gawk_crt_stat(name, buf)
#endif

#ifndef gawk_crt_do_not_redefine_fstat
#ifdef fstat
#undef fstat
#endif
#define fstat gawk_crt_fstat
#endif

#ifndef gawk_crt_do_not_redefine_chmod
#ifdef chmod
#undef chmod
#endif
#define chmod gawk_crt_chmod
#endif

#ifndef gawk_crt_do_not_redefine_opendirfd
#ifdef opendirfd
#undef opendirfd
#endif
#define opendirfd gawk_crt_opendirfd
#endif

#ifndef gawk_crt_do_not_redefine_closedirfd
#ifdef closedirfd
#undef closedirfd
#endif
#define closedirfd gawk_crt_closedirfd
#endif

#ifndef gawk_crt_do_not_redefine_fchdir
#ifdef fchdir
#undef fchdir
#endif
#define fchdir gawk_crt_fchdir
#endif

#ifndef gawk_crt_do_not_redefine_opendir
#ifdef opendir
#undef opendir
#endif
#define opendir gawk_crt_opendir
#endif

#ifndef gawk_crt_do_not_redefine_closedir
#ifdef closedir
#undef closedir
#endif
#define closedir gawk_crt_closedir
#endif

#ifndef gawk_crt_do_not_redefine_readdir
#ifdef readdir
#undef readdir
#endif
#define readdir gawk_crt_readdir
#endif

#ifndef gawk_crt_do_not_redefine_wreadlinkfd
#ifdef wreadlinkfd
#undef wreadlinkfd
#endif
#define wreadlinkfd gawk_crt_wreadlinkfd
#endif

#ifndef gawk_crt_do_not_redefine_wreadlink
#ifdef wreadlink
#undef wreadlink
#endif
#define wreadlink gawk_crt_wreadlink
#endif

#ifndef gawk_crt_do_not_redefine_readlinkfd
#ifdef readlinkfd
#undef readlinkfd
#endif
#define readlinkfd gawk_crt_readlinkfd
#endif

#ifndef gawk_crt_do_not_redefine_readlink
#ifdef readlink
#undef readlink
#endif
#define readlink gawk_crt_readlink
#endif

#ifndef gawk_crt_do_not_redefine_setlocale
#ifdef setlocale
#undef setlocale
#endif
#define setlocale gawk_crt_setlocale
#endif

#ifndef gawk_crt_do_not_redefine_localeconv
#ifdef localeconv
#undef localeconv
#endif
#define localeconv gawk_crt_localeconv
#endif

#ifndef gawk_crt_do_not_redefine_MB_CUR_MAX
#ifdef MB_CUR_MAX
#undef MB_CUR_MAX
#endif
#define MB_CUR_MAX gawk_crt_mb_cur_max()
#endif

#ifndef gawk_crt_do_not_redefine_btowc
#ifdef btowc
#undef btowc
#endif
#define btowc gawk_crt_btowc
#endif

#ifndef gawk_crt_do_not_redefine_mblen
#ifdef mblen
#undef mblen
#endif
#define mblen gawk_crt_mblen
#endif

#ifndef gawk_crt_do_not_redefine_mbrlen
#ifdef mbrlen
#undef mbrlen
#endif
#define mbrlen gawk_crt_mbrlen
#endif

#ifndef gawk_crt_do_not_redefine_mbstowcs
#ifdef mbstowcs
#undef mbstowcs
#endif
#define mbstowcs gawk_crt_mbstowcs
#endif

#ifndef gawk_crt_do_not_redefine_wcstombs
#ifdef wcstombs
#undef wcstombs
#endif
#define wcstombs gawk_crt_wcstombs
#endif

#ifndef gawk_crt_do_not_redefine_fprintf
#ifdef fprintf
#undef fprintf
#endif
#define fprintf gawk_crt_fprintf
#endif

#ifndef gawk_crt_do_not_redefine_sprintf
#ifdef sprintf
#undef sprintf
#endif
#define sprintf gawk_crt_sprintf
#endif

#ifndef gawk_crt_do_not_redefine_snprintf
#ifdef snprintf
#undef snprintf
#endif
#define snprintf gawk_crt_snprintf
#endif

#ifndef gawk_crt_do_not_redefine_vprintf
#ifdef vprintf
#undef vprintf
#endif
#define vprintf gawk_crt_vprintf
#endif

#ifndef gawk_crt_do_not_redefine_vfprintf
#ifdef vfprintf
#undef vfprintf
#endif
#define vfprintf gawk_crt_vfprintf
#endif

#ifndef gawk_crt_do_not_redefine_vsprintf
#ifdef vsprintf
#undef vsprintf
#endif
#define vsprintf gawk_crt_vsprintf
#endif

#ifndef gawk_crt_do_not_redefine_vsnprintf
#ifdef vsnprintf
#undef vsnprintf
#endif
#define vsnprintf gawk_crt_vsnprintf
#endif

#ifndef gawk_crt_do_not_redefine_errno
#ifdef errno
#undef errno
#endif
#define errno (*gawk_crt_errno_p())
#endif

#ifndef gawk_crt_do_not_redefine_strerror
#ifdef strerror
#undef strerror
#endif
#define strerror gawk_crt_strerror
#endif

#ifndef gawk_crt_do_not_redefine_strftime
#ifdef strftime
#undef strftime
#endif
#define strftime gawk_crt_strftime
#endif

#ifndef gawk_crt_do_not_redefine_mkstemp
#ifdef mkstemp
#undef mkstemp
#endif
#define mkstemp gawk_crt_mkstemp
#endif

#ifndef gawk_crt_do_not_redefine_environ
#ifdef environ
#undef environ
#endif
#define environ gawk_crt_environ()
#endif

#ifndef gawk_crt_do_not_redefine_getenv
#ifdef getenv
#undef getenv
#endif
#define getenv gawk_crt_getenv
#endif

#ifndef gawk_crt_do_not_redefine_setenv
#ifdef setenv
#undef setenv
#endif
#define setenv gawk_crt_setenv
#endif

#ifndef gawk_crt_do_not_redefine_unsetenv
#ifdef unsetenv
#undef unsetenv
#endif
#define unsetenv gawk_crt_unsetenv
#endif

#ifndef gawk_crt_do_not_redefine_clearenv
#ifdef clearenv
#undef clearenv
#endif
#define clearenv gawk_crt_clearenv
#endif

#ifndef gawk_crt_do_not_redefine_strcoll
#ifdef strcoll
#undef strcoll
#endif
#define strcoll gawk_crt_strcoll
#endif

#ifndef gawk_crt_do_not_redefine_tolower
#ifdef tolower
#undef tolower
#endif
#define tolower gawk_crt_tolower
#endif

#ifndef gawk_crt_do_not_redefine_toupper
#ifdef toupper
#undef toupper
#endif
#define toupper gawk_crt_toupper
#endif

#ifndef gawk_crt_do_not_redefine_isascii
#ifdef isascii
#undef isascii
#endif
#define isascii gawk_crt_isascii
#endif

#ifndef gawk_crt_do_not_redefine_isalnum
#ifdef isalnum
#undef isalnum
#endif
#define isalnum gawk_crt_isalnum
#endif

#ifndef gawk_crt_do_not_redefine_isalpha
#ifdef isalpha
#undef isalpha
#endif
#define isalpha gawk_crt_isalpha
#endif

#ifndef gawk_crt_do_not_redefine_isblank
#ifdef isblank
#undef isblank
#endif
#define isblank gawk_crt_isblank
#endif

#ifndef gawk_crt_do_not_redefine_iscntrl
#ifdef iscntrl
#undef iscntrl
#endif
#define iscntrl gawk_crt_iscntrl
#endif

#ifndef gawk_crt_do_not_redefine_isdigit
#ifdef isdigit
#undef isdigit
#endif
#define isdigit gawk_crt_isdigit
#endif

#ifndef gawk_crt_do_not_redefine_isgraph
#ifdef isgraph
#undef isgraph
#endif
#define isgraph gawk_crt_isgraph
#endif

#ifndef gawk_crt_do_not_redefine_islower
#ifdef islower
#undef islower
#endif
#define islower gawk_crt_islower
#endif

#ifndef gawk_crt_do_not_redefine_isprint
#ifdef isprint
#undef isprint
#endif
#define isprint gawk_crt_isprint
#endif

#ifndef gawk_crt_do_not_redefine_ispunct
#ifdef ispunct
#undef ispunct
#endif
#define ispunct gawk_crt_ispunct
#endif

#ifndef gawk_crt_do_not_redefine_isspace
#ifdef isspace
#undef isspace
#endif
#define isspace gawk_crt_isspace
#endif

#ifndef gawk_crt_do_not_redefine_isupper
#ifdef isupper
#undef isupper
#endif
#define isupper gawk_crt_isupper
#endif

#ifndef gawk_crt_do_not_redefine_isxdigit
#ifdef isxdigit
#undef isxdigit
#endif
#define isxdigit gawk_crt_isxdigit
#endif

#if !defined gawk_crt_do_not_redefine_wcscoll || defined _MSC_VER || defined __MINGW32__
#ifdef wcscoll
#undef wcscoll
#endif
#endif
#if !defined gawk_crt_do_not_redefine_towlower || defined _MSC_VER || defined __MINGW32__
#ifdef towlower
#undef towlower
#endif
#endif
#if !defined gawk_crt_do_not_redefine_towupper || defined _MSC_VER || defined __MINGW32__
#ifdef towupper
#undef towupper
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswascii || defined _MSC_VER || defined __MINGW32__
#ifdef iswascii
#undef iswascii
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswalnum || defined _MSC_VER || defined __MINGW32__
#ifdef iswalnum
#undef iswalnum
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswalpha || defined _MSC_VER || defined __MINGW32__
#ifdef iswalpha
#undef iswalpha
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswblank || defined _MSC_VER || defined __MINGW32__
#ifdef iswblank
#undef iswblank
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswcntrl || defined _MSC_VER || defined __MINGW32__
#ifdef iswcntrl
#undef iswcntrl
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswdigit || defined _MSC_VER || defined __MINGW32__
#ifdef iswdigit
#undef iswdigit
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswgraph || defined _MSC_VER || defined __MINGW32__
#ifdef iswgraph
#undef iswgraph
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswlower || defined _MSC_VER || defined __MINGW32__
#ifdef iswlower
#undef iswlower
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswprint || defined _MSC_VER || defined __MINGW32__
#ifdef iswprint
#undef iswprint
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswpunct || defined _MSC_VER || defined __MINGW32__
#ifdef iswpunct
#undef iswpunct
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswspace || defined _MSC_VER || defined __MINGW32__
#ifdef iswspace
#undef iswspace
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswupper || defined _MSC_VER || defined __MINGW32__
#ifdef iswupper
#undef iswupper
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswxdigit || defined _MSC_VER || defined __MINGW32__
#ifdef iswxdigit
#undef iswxdigit
#endif
#endif
#if !defined gawk_crt_do_not_redefine_wctype || defined _MSC_VER || defined __MINGW32__
#ifdef wctype
#undef wctype
#endif
#endif
#if !defined gawk_crt_do_not_redefine_iswctype || defined _MSC_VER || defined __MINGW32__
#ifdef iswctype
#undef iswctype
#endif
#endif

#if !defined gawk_crt_do_not_redefine_mbtowc || defined _MSC_VER || defined __MINGW32__
#ifdef mbtowc
#undef mbtowc
#endif
#endif
#if !defined gawk_crt_do_not_redefine_mbrtowc || defined _MSC_VER || defined __MINGW32__
#ifdef mbrtowc
#undef mbrtowc
#endif
#endif
#if !defined gawk_crt_do_not_redefine_wctomb || defined _MSC_VER || defined __MINGW32__
#ifdef wctomb
#undef wctomb
#endif
#endif
#if !defined gawk_crt_do_not_redefine_wcrtomb || defined _MSC_VER || defined __MINGW32__
#ifdef wcrtomb
#undef wcrtomb
#endif
#endif

#if !defined _MSC_VER && !defined __MINGW32__

#ifndef gawk_crt_do_not_redefine_wcscoll
#define wcscoll gawk_crt_wcscoll
#endif
#ifndef gawk_crt_do_not_redefine_towlower
#define towlower gawk_crt_towlower
#endif
#ifndef gawk_crt_do_not_redefine_towupper
#define towupper gawk_crt_towupper
#endif
#ifndef gawk_crt_do_not_redefine_iswascii
#define iswascii gawk_crt_iswascii
#endif
#ifndef gawk_crt_do_not_redefine_iswalnum
#define iswalnum gawk_crt_iswalnum
#endif
#ifndef gawk_crt_do_not_redefine_iswalpha
#define iswalpha gawk_crt_iswalpha
#endif
#ifndef gawk_crt_do_not_redefine_iswblank
#define iswblank gawk_crt_iswblank
#endif
#ifndef gawk_crt_do_not_redefine_iswcntrl
#define iswcntrl gawk_crt_iswcntrl
#endif
#ifndef gawk_crt_do_not_redefine_iswdigit
#define iswdigit gawk_crt_iswdigit
#endif
#ifndef gawk_crt_do_not_redefine_iswgraph
#define iswgraph gawk_crt_iswgraph
#endif
#ifndef gawk_crt_do_not_redefine_iswlower
#define iswlower gawk_crt_iswlower
#endif
#ifndef gawk_crt_do_not_redefine_iswprint
#define iswprint gawk_crt_iswprint
#endif
#ifndef gawk_crt_do_not_redefine_iswpunct
#define iswpunct gawk_crt_iswpunct
#endif
#ifndef gawk_crt_do_not_redefine_iswspace
#define iswspace gawk_crt_iswspace
#endif
#ifndef gawk_crt_do_not_redefine_iswupper
#define iswupper gawk_crt_iswupper
#endif
#ifndef gawk_crt_do_not_redefine_iswxdigit
#define iswxdigit gawk_crt_iswxdigit
#endif
#ifndef gawk_crt_do_not_redefine_wctype
#define wctype gawk_crt_wctype
#endif
#ifndef gawk_crt_do_not_redefine_iswctype
#define iswctype gawk_crt_iswctype
#endif

#ifndef gawk_crt_do_not_redefine_mbtowc
#define mbtowc gawk_crt_mbtowc
#endif
#ifndef gawk_crt_do_not_redefine_mbrtowc
#define mbrtowc gawk_crt_mbrtowc
#endif
#ifndef gawk_crt_do_not_redefine_wctomb
#define wctomb gawk_crt_wctomb
#endif
#ifndef gawk_crt_do_not_redefine_wcrtomb
#define wcrtomb gawk_crt_wcrtomb
#endif

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

#ifndef gawk_crt_do_not_redefine_c32scoll
#ifdef c32scoll
#undef c32scoll
#endif
#define c32scoll gawk_crt_c32scoll
#endif

#ifndef gawk_crt_do_not_redefine_c32tolower
#ifdef c32tolower
#undef c32tolower
#endif
#define c32tolower gawk_crt_c32tolower
#endif

#ifndef gawk_crt_do_not_redefine_c32toupper
#ifdef c32toupper
#undef c32toupper
#endif
#define c32toupper gawk_crt_c32toupper
#endif

#ifndef gawk_crt_do_not_redefine_c32isascii
#ifdef c32isascii
#undef c32isascii
#endif
#define c32isascii gawk_crt_c32isascii
#endif

#ifndef gawk_crt_do_not_redefine_c32isalnum
#ifdef c32isalnum
#undef c32isalnum
#endif
#define c32isalnum gawk_crt_c32isalnum
#endif

#ifndef gawk_crt_do_not_redefine_c32isalpha
#ifdef c32isalpha
#undef c32isalpha
#endif
#define c32isalpha gawk_crt_c32isalpha
#endif

#ifndef gawk_crt_do_not_redefine_c32isblank
#ifdef c32isblank
#undef c32isblank
#endif
#define c32isblank gawk_crt_c32isblank
#endif

#ifndef gawk_crt_do_not_redefine_c32iscntrl
#ifdef c32iscntrl
#undef c32iscntrl
#endif
#define c32iscntrl gawk_crt_c32iscntrl
#endif

#ifndef gawk_crt_do_not_redefine_c32isdigit
#ifdef c32isdigit
#undef c32isdigit
#endif
#define c32isdigit gawk_crt_c32isdigit
#endif

#ifndef gawk_crt_do_not_redefine_c32isgraph
#ifdef c32isgraph
#undef c32isgraph
#endif
#define c32isgraph gawk_crt_c32isgraph
#endif

#ifndef gawk_crt_do_not_redefine_c32islower
#ifdef c32islower
#undef c32islower
#endif
#define c32islower gawk_crt_c32islower
#endif

#ifndef gawk_crt_do_not_redefine_c32isprint
#ifdef c32isprint
#undef c32isprint
#endif
#define c32isprint gawk_crt_c32isprint
#endif

#ifndef gawk_crt_do_not_redefine_c32ispunct
#ifdef c32ispunct
#undef c32ispunct
#endif
#define c32ispunct gawk_crt_c32ispunct
#endif

#ifndef gawk_crt_do_not_redefine_c32isspace
#ifdef c32isspace
#undef c32isspace
#endif
#define c32isspace gawk_crt_c32isspace
#endif

#ifndef gawk_crt_do_not_redefine_c32isupper
#ifdef c32isupper
#undef c32isupper
#endif
#define c32isupper gawk_crt_c32isupper
#endif

#ifndef gawk_crt_do_not_redefine_c32isxdigit
#ifdef c32isxdigit
#undef c32isxdigit
#endif
#define c32isxdigit gawk_crt_c32isxdigit
#endif

#ifndef gawk_crt_do_not_redefine_c32ctype
#ifdef c32ctype
#undef c32ctype
#endif
#define c32ctype gawk_crt_c32ctype
#endif

#ifndef gawk_crt_do_not_redefine_c32isctype
#ifdef c32isctype
#undef c32isctype
#endif
#define c32isctype gawk_crt_c32isctype
#endif

#ifndef gawk_crt_do_not_redefine_mbrtoc16
#ifdef mbrtoc16
#undef mbrtoc16
#endif
#define mbrtoc16 gawk_crt_mbrtoc16
#endif

#ifndef gawk_crt_do_not_redefine_mbrtoc32
#ifdef mbrtoc32
#undef mbrtoc32
#endif
#define mbrtoc32 gawk_crt_mbrtoc32
#endif

#ifndef gawk_crt_do_not_redefine_c16rtomb
#ifdef c16rtomb
#undef c16rtomb
#endif
#define c16rtomb gawk_crt_c16rtomb
#endif

#ifndef gawk_crt_do_not_redefine_c32rtomb
#ifdef c32rtomb
#undef c32rtomb
#endif
#define c32rtomb gawk_crt_c32rtomb
#endif

#ifndef gawk_crt_do_not_redefine_mbstoc32s
#ifdef mbstoc32s
#undef mbstoc32s
#endif
#define mbstoc32s gawk_crt_mbstoc32s
#endif

#ifndef gawk_crt_do_not_redefine_c32stombs
#ifdef c32stombs
#undef c32stombs
#endif
#define c32stombs gawk_crt_c32stombs
#endif

#ifndef gawk_crt_do_not_redefine_wcstoc32s
#ifdef wcstoc32s
#undef wcstoc32s
#endif
#define wcstoc32s gawk_crt_wcstoc32s
#endif

#ifndef gawk_crt_do_not_redefine_c32stowcs
#ifdef c32stowcs
#undef c32stowcs
#endif
#define c32stowcs gawk_crt_c32stowcs
#endif

#ifndef gawk_crt_do_not_redefine_c32slen
#ifdef c32slen
#undef c32slen
#endif
#define c32slen gawk_crt_c32slen
#endif

#ifndef gawk_crt_do_not_redefine_c32schr
#ifdef c32schr
#undef c32schr
#endif
#define c32schr gawk_crt_c32schr
#endif

#ifndef gawk_crt_do_not_redefine_c32srchr
#ifdef c32srchr
#undef c32srchr
#endif
#define c32srchr gawk_crt_c32srchr
#endif

#ifndef gawk_crt_do_not_redefine_c32schrnul
#ifdef c32schrnul
#undef c32schrnul
#endif
#define c32schrnul gawk_crt_c32schrnul
#endif

#endif /* _MSC_VER || __MINGW32__ */

#if defined _MSC_VER || defined __MINGW32__

#ifndef gawk_crt_do_not_redefine_xtimegm
#ifdef xtimegm
#undef xtimegm
#endif
#define xtimegm gawk_crt_xtimegm
#endif

#endif /* _MSC_VER || __MINGW32__ */

#if defined __GNU_MP_VERSION && defined MPFR_VERSION_MAJOR

#ifndef gawk_crt_do_not_redefine_mpz_init
#ifdef mpz_init
#undef mpz_init
#endif
#define mpz_init gawk_crt_mpz_init
#endif

#ifndef gawk_crt_do_not_redefine_mpz_clear
#ifdef mpz_clear
#undef mpz_clear
#endif
#define mpz_clear gawk_crt_mpz_clear
#endif

#ifndef gawk_crt_do_not_redefine_mpz_set_d
#ifdef mpz_set_d
#undef mpz_set_d
#endif
#define mpz_set_d gawk_crt_mpz_set_d
#endif

#ifndef gawk_crt_do_not_redefine_mpz_tdiv_qr
#ifdef mpz_tdiv_qr
#undef mpz_tdiv_qr
#endif
#define mpz_tdiv_qr gawk_crt_mpz_tdiv_qr
#endif

#ifndef gawk_crt_do_not_redefine_mpfr_number_p
#ifdef mpfr_number_p
#undef mpfr_number_p
#endif
#define mpfr_number_p gawk_crt_mpfr_number_p
#endif

#ifndef gawk_crt_do_not_redefine_mpfr_get_z
#ifdef mpfr_get_z
#undef mpfr_get_z
#endif
#define mpfr_get_z gawk_crt_mpfr_get_z
#endif

#endif /* __GNU_MP_VERSION && MPFR_VERSION_MAJOR */

/* 3) Add more CRT replacements here.  */

#endif /* !GAWK_DONT_REDEFINE_CRT */

#endif /* !GAWK */
