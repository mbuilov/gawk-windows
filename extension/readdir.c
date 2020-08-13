/*
 * readdir.c --- Provide an input parser to read directories
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2012
 *
 * Andrew Schorr and Arnold Robbins: further fixes 8/2012.
 * Simplified 11/2012.
 * Improved 3/2019.
 *
 * Michael M. Builov: reworked, ported to _MSC_VER.
 * mbuilov@gmail.com
 * Reworked 4/2020
 *
 * Michael M. Builov: reworked, make __MINGW32__ version the same as _MSC_VER.
 * mbuilov@gmail.com
 * Reworked 5/2020
 */

/*
 * Copyright (C) 2012-2014, 2018, 2019 the Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _BSD_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <inttypes.h>
#include <wchar.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#elif defined WINDOWS_NATIVE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mscrtx/xstat.h"
#else
#error Cannot compile the readdir extension on this system!
#endif

/* Include <locale.h> before "gawkapi.h" redefines setlocale().
  "gettext.h" will include <locale.h> anyway */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifndef WINDOWS_NATIVE
#include "gawkdirfd.h"
#endif

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#ifndef S_ISDIR
# ifdef S_IFDIR
#  define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
# elif defined _S_IFDIR
#  define S_ISDIR(m)	(((m) & _S_IFMT) == _S_IFDIR)
# endif
#endif

#ifndef PATH_MAX
#define PATH_MAX	1024	/* a good guess */
#endif

GAWK_PLUGIN_GPL_COMPATIBLE

#ifndef READDIR_TEST
GAWK_PLUGIN("readdir extension: version 2.0");
#else
GAWK_PLUGIN("readdir extension: version 3.0");
#endif

#ifndef OFFSETOF
# if defined _MSC_VER && !defined __cplusplus
#  define OFFSETOF(type, member) \
     ((size_t) ((const char*)&((type*)NULL)->member - (const char*)NULL))
# else
#  define OFFSETOF(type, member) offsetof(type, member)
# endif
#endif

/* data type for the opaque pointer: */

union u_field_info {
	awk_fieldwidth_info_t fwi;
	char buf[awk_fieldwidth_info_size(3)];
};

typedef struct open_directory {
#ifdef READDIR_TEST
	union u_field_info u;
#define fw u.fwi
#endif
#ifdef WINDOWS_NATIVE
	/* HANDLE ffd_handle        */
	/* wchar_t dirname[dir_len] */
	/* ...optional padding...   */
	unsigned dir_len;
	unsigned space;
	WIN32_FIND_DATAW ffd;
#else
	DIR *dp;
#endif
	char buf[1];
} open_directory_t;

/* Get head size of struct open_directory.  */
static size_t
dir_head_size(void)
{
	return OFFSETOF(struct open_directory, buf);
}

#ifdef WINDOWS_NATIVE
/* Get size of memory before the head of struct open_directory.  */
static size_t
head_offset(size_t dir_len)
{
	/* Align offset on size of pointer.  */
	size_t offset = sizeof(HANDLE) + dir_len*sizeof(wchar_t);
	offset = (offset + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
	return offset;
}
#endif /* WINDOWS_NATIVE */

#ifdef WINDOWS_NATIVE
static unsigned long long
file_info_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}
#endif

#ifndef WINDOWS_NATIVE

/* file_type --- return type of file as a single character */

static int
file_type(struct dirent *entry, const char *dirname)
{
#ifdef DT_BLK
	(void) dirname;		/* silence warnings */
	switch (entry->d_type) {
	case DT_BLK:	return 'b';
	case DT_CHR:	return 'c';
	case DT_DIR:	return 'd';
	case DT_FIFO:	return 'p';
	case DT_LNK:	return 'l';
	case DT_REG:	return 'f';
	case DT_SOCK:	return 's';
	default:
	case DT_UNKNOWN: break;	// JFS returns 'u', so fall through to stat
	}
#endif
	char fname[PATH_MAX];
	struct stat sbuf;

	strcpy(fname, dirname);
	strcat(fname, "/");
	strcat(fname, entry->d_name);
	if (stat(fname, &sbuf) == 0) {
		if (S_ISBLK(sbuf.st_mode))
			return 'b';
		if (S_ISCHR(sbuf.st_mode))
			return 'c';
		if (S_ISDIR(sbuf.st_mode))
			return 'd';
		if (S_ISFIFO(sbuf.st_mode))
			return 'p';
		if (S_ISREG(sbuf.st_mode))
			return 'f';
#ifdef S_ISLNK
		if (S_ISLNK(sbuf.st_mode))
			return 'l';
#endif
#ifdef S_ISSOCK
		if (S_ISSOCK(sbuf.st_mode))
			return 's';
#endif
	}
	return 'u';
}

/* get_inode --- get the inode of a file */

static unsigned long long
get_inode(struct dirent *entry, const char *dirname)
{
	(void) dirname;		/* silence warnings */
	return (unsigned long long) entry->d_ino;
}

#else /* WINDOWS_NATIVE */

/* ftype_and_ino: use file handle to get both inode number and the file type
  --- return type of file as a single character string,
  --- get the inode of the file */
static int
ftype_and_ino(
	const WIN32_FIND_DATAW *ffd,
	const wchar_t dirname[],
	const unsigned dir_len,
	unsigned long long *inode/*out*/)
{
	HANDLE fh;
	wchar_t path_buf[2*MAX_PATH], *path = path_buf;
	size_t fname_len = wcslen(ffd->cFileName);

	unsigned long long ino = 0; /* zero - in case of error */
	int ftype = 'u'; /* unknown - in case of error */

	if (ffd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		ftype = 'l'; /* link */
	else if (ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		ftype = 'd'; /* directory */

	if (dir_len + fname_len > sizeof(path_buf)/sizeof(path_buf[0]) - 2) {
		size_t full_len = dir_len + fname_len + 2;
		path = (wchar_t*) malloc(sizeof(wchar_t)*(full_len));
		if (path == NULL)
			goto err;
	}

	memcpy(path, dirname, dir_len*sizeof(dirname[0]));
	path[dir_len] = L'\\';
	memcpy(path + dir_len + 1, ffd->cFileName,
		sizeof(wchar_t)*(fname_len + 1));

	fh = CreateFileW(path, FILE_READ_ATTRIBUTES,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING,
			((ffd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				? FILE_FLAG_OPEN_REPARSE_POINT : 0u) |
			FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (fh != INVALID_HANDLE_VALUE) {

		BY_HANDLE_FILE_INFORMATION info;

		if (GetFileInformationByHandle(fh, &info))
			ino = file_info_get_inode(&info);

		if ('u' == ftype) {
			const DWORD ft = GetFileType(fh);

			switch (ft) {
				case FILE_TYPE_DISK:
					ftype = 'f'; /* file */
					break;
				case FILE_TYPE_CHAR:
					ftype = 'c'; /* character device */
					break;
				case FILE_TYPE_PIPE:
					if (GetNamedPipeInfo(fh, NULL, NULL,
								NULL, NULL))
						ftype = 'p'; /* pipe */
					else
						ftype = 's'; /* socket */
					break;
				default:
					break;
			}
		}

		CloseHandle(fh);
	}

	if (path != path_buf)
		free(path);
err:
	*inode = ino;
	return ftype;
}

#endif /* WINDOWS_NATIVE */

/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len,
		const awk_fieldwidth_info_t **field_width)
{
#ifndef WINDOWS_NATIVE
	DIR *dp;
	struct dirent *dirent;
#else
	size_t converted;
	HANDLE *ffd_handle;
	const wchar_t *dirname;
	void *p;
#endif
	char *out_buf;
	int len, flen;
	open_directory_t *the_dir;
	int ftype;
	unsigned long long ino;

	(void) field_width;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	the_dir = (open_directory_t *) iobuf->opaque;

#ifndef WINDOWS_NATIVE

	dp = the_dir->dp;

	/*
	 * Initialize errno, since readdir does not set it to zero on EOF.
	 */
	errno = 0;
	dirent = readdir(dp);
	if (dirent == NULL) {
		*errcode = errno;	/* in case there was an error */
		return EOF;
	}

	ino = get_inode(dirent, iobuf->name);
	ftype = file_type(dirent, iobuf->name);
	out_buf = the_dir->buf;

# ifndef READDIR_TEST

	len = sprintf(out_buf, "%" LLUFMT "/%s", ino, dirent->d_name);

# else /* READDIR_TEST */

	len = sprintf(out_buf, "%" LLUFMT "", ino);
	the_dir->fw.fields[0].len = (unsigned) len;
	len += (flen = sprintf(out_buf + len, "/%s", dirent->d_name));
	the_dir->fw.fields[1].len = (unsigned) flen - 1;

# endif /* READDIR_TEST */

#else /* WINDOWS_NATIVE */

	p = (char*) the_dir - head_offset(the_dir->dir_len);
	ffd_handle = (HANDLE*) p;

	if (the_dir->ffd.dwFileAttributes == ~(DWORD)0) {
		/* no cached entry */
		if (!FindNextFileW(*ffd_handle, &the_dir->ffd)) {
			const DWORD err = GetLastError();
			*errcode = ERROR_NO_MORE_FILES == err ? 0 : EACCES;
			return EOF;
		}
	}

	p = ffd_handle + 1;
	dirname = (const wchar_t*) p;
	ftype = ftype_and_ino(&the_dir->ffd, dirname, the_dir->dir_len, &ino);

	/* mark that there is no cached entry */
	the_dir->ffd.dwFileAttributes = ~(DWORD)0;

	converted = wcstombs(the_dir->buf, the_dir->ffd.cFileName,
		the_dir->space);

	if ((size_t)-1 == converted) {
		*errcode = EILSEQ;
		return EOF;
	}

	/* Make sure that conversion wasn't stopped due to small buffer.  */
	/* Note: the_dir->space > 10 */
	if (converted > the_dir->space - 10/*MB_CUR_MAX*/) {
		const size_t n = wcstombs(NULL, the_dir->ffd.cFileName, 0);
		if (n != converted) {
			*errcode = (size_t)-1 == n ? EILSEQ : ENAMETOOLONG;
			return EOF;
		}
	}

	/* there must be a space for at least 21 characters between
	  ffd->dwfileattributes and ffd->cfilename */
	(void) sizeof(int[1-2*!(OFFSETOF(WIN32_FIND_DATAW, cFileName)
		- OFFSETOF(WIN32_FIND_DATAW, dwFileAttributes) >=
		sizeof(the_dir->ffd.dwFileAttributes) + 21)]);

	/* print max 21 characters, including terminating '\0' */
	out_buf = (char*)(&the_dir->ffd.dwFileAttributes + 1);
	len = sprintf(out_buf, "%" LLUFMT "", ino);
# ifdef READDIR_TEST
	the_dir->fw.fields[0].len = (unsigned) len;
# endif

	/* There must be a space for 3 more characters: 2 slashes and ftype.  */
	(void) sizeof(int[1-2*!(sizeof(the_dir->ffd.cFileName) >= 3)]);

	out_buf[len++] = '/';
	memmove(out_buf + len, the_dir->buf, converted);
	len += (int) converted;
# ifdef READDIR_TEST
	the_dir->fw.fields[1].len = (unsigned) converted;
# endif

#endif /* WINDOWS_NATIVE */

	flen = sprintf(out_buf + len, "/%c", (char) ftype);
	len += flen;
#ifdef READDIR_TEST
	the_dir->fw.fields[2].len = (unsigned) flen - 1;
#endif

	*out = out_buf;

	*rt_start = NULL;
	*rt_len = 0;	/* set RT to "" */
#ifdef READDIR_TEST
	if (field_width)
		*field_width = & the_dir->fw;
#endif
	return len;
}

/* dir_close --- close up when done */

static void
dir_close(awk_input_buf_t *iobuf)
{
	open_directory_t *the_dir;
	void *mem;

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	the_dir = (open_directory_t *) iobuf->opaque;

#ifndef WINDOWS_NATIVE
	mem = the_dir;
	closedir(the_dir->dp);
#else
	mem = (char*) the_dir - head_offset(the_dir->dir_len);
	FindClose(*(HANDLE*)mem);
#endif

	free(mem);
	iobuf->fd = INVALID_HANDLE;
}

/* dir_can_take_file --- return true if we want the file */

static awk_bool_t
dir_can_take_file(const awk_input_buf_t *iobuf)
{
	if (iobuf == NULL)
		return awk_false;

	return (awk_bool_t) (iobuf->fd != INVALID_HANDLE &&
		S_ISDIR(awk_input_buf_get_stat(iobuf)->st_mode));
}

#ifdef WINDOWS_NATIVE
static int
report_opendir_error(const char *dirname, const DWORD last_err)
{
	char *msg_buf = NULL;
	WCHAR *wbuf = NULL;
	void *const pwbuf = &wbuf;
	DWORD n_chars;

	n_chars = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, last_err, 0, (LPWSTR)pwbuf, 0, NULL);

	/* trim "\r\n" at end of msg */
	if (n_chars && L'\n' == wbuf[n_chars - 1]) {
		if (--n_chars && L'\r' == wbuf[n_chars - 1])
			n_chars--;
		wbuf[n_chars] = L'\0';
	}

	if (n_chars) {
		/* e.g. convert to UTF-8 */
		size_t n = wcstombs(NULL, wbuf, 0);
		if (n != (size_t)-1) {
			msg_buf = (char*)malloc(n + 1);
			if (msg_buf != NULL)
				(void)wcstombs(msg_buf, wbuf, n + 1);
		}
	}

	if (wbuf != NULL)
		LocalFree(wbuf);

	if (msg_buf != NULL)
		warning(_("dir_take_control_of: "
			"couldn't open directory: %s, system error: 0x%lx (%s)"),
			dirname, last_err, msg_buf);
	else
		warning(_("dir_take_control_of: "
			"couldn't open directory: %s, system error: 0x%lx"),
			dirname, last_err);

	if (msg_buf != NULL)
		free(msg_buf);
	return
		ERROR_FILE_NOT_FOUND == last_err ? ENOENT :
		ERROR_ACCESS_DENIED == last_err ? EACCES :
		EFAULT;
}
#endif /* WINDOWS_NATIVE */

/*
 * dir_take_control_of --- set up input parser.
 * We can assume that dir_can_take_file just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
dir_take_control_of(awk_input_buf_t *iobuf)
{
	open_directory_t *the_dir;
	size_t size;

#ifndef WINDOWS_NATIVE

	DIR *dp;

	errno = 0;
# ifdef HAVE_FDOPENDIR
	dp = fdopendir(iobuf->fd);
# else
	dp = opendir(iobuf->name);
	if (dp != NULL)
		iobuf->fd = dirfd(dp);
# endif
	if (dp == NULL) {
		warning(_("dir_take_control_of: opendir/fdopendir failed: %s"),
			strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

	size = dir_head_size()
		+ sizeof(struct dirent)
		+ 20 /* max digits in inode */
		+ 2 /* slashes */
		+ 1 /* ftype */;

	emalloc(the_dir, open_directory_t *, size, "dir_take_control_of");

	the_dir->dp = dp;

#else /* WINDOWS_NATIVE */

	HANDLE *ffd_handle;
	wchar_t *dirname;
	size_t offset;
	void *p;

	size_t dir_len = mbstowcs(NULL, iobuf->name, 0);
	if ((size_t)-1 == dir_len) {
		errno = EILSEQ;
		warning(_("dir_take_control_of: opendir/fdopendir failed: %s"),
			strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

	/* Path should not exceed the limit of 32767 wide characters.  */
	if (dir_len > 32767 - 3) {
		warning(_("dir_take_control_of: too long directory path: \"%s\""),
			iobuf->name);
		update_ERRNO_int(ENAMETOOLONG);
		return awk_false;
	}

	offset = head_offset(dir_len);
	size = offset
		+ dir_head_size()
		+ MAX_PATH*((unsigned) MB_CUR_MAX) + 10;

	emalloc(ffd_handle, HANDLE *, size, "dir_take_control_of");

	p = (char*) ffd_handle + offset;
	the_dir = (open_directory_t*) p;

	p = ffd_handle + 1;
	dirname = (wchar_t*) p;

	(void) mbstowcs(dirname, iobuf->name, dir_len);

	/* Note: write past dirname.  This memory will be initialized later.  */
	memcpy(dirname + dir_len, L"\\*", 3*sizeof(wchar_t));

	/* Cache found file in the_dir.  */
	*ffd_handle = FindFirstFileW(dirname, &the_dir->ffd);

	/* Must find at least '.', otherwise directory path is wrong.  */
	if (*ffd_handle == INVALID_HANDLE_VALUE ||
		the_dir->ffd.dwFileAttributes == ~(DWORD)0)
	{
		const int err = report_opendir_error(iobuf->name,
			*ffd_handle == INVALID_HANDLE_VALUE
			? GetLastError() : ERROR_INVALID_DATA);

		if (*ffd_handle != INVALID_HANDLE_VALUE)
			FindClose(*ffd_handle);
		free(ffd_handle);
		update_ERRNO_int(err);
		return awk_false;
	}

	/* Trim the directory name.  */
	dirname[dir_len] = L'\0';

	the_dir->dir_len = (unsigned) dir_len;
	the_dir->space = (unsigned) (size - offset - dir_head_size());

#endif /* WINDOWS_NATIVE */

#ifdef READDIR_TEST
	/* pre-populate the field_width struct with constant values: */
	the_dir->fw.use_chars = awk_false;
	the_dir->fw.nf = 3;
	the_dir->fw.fields[0].skip = 0;	/* no leading space */
	the_dir->fw.fields[1].skip = 1;	/* single '/' separator */
	the_dir->fw.fields[2].skip = 1;	/* single '/' separator */
#endif

	iobuf->opaque = the_dir;
	iobuf->get_record = dir_get_record;
	iobuf->close_func = dir_close;

	return awk_true;
}

static awk_input_parser_t readdir_parser = {
	"readdir",
	dir_can_take_file,
	dir_take_control_of,
	NULL
};

#ifdef TEST_DUPLICATE
static awk_input_parser_t readdir_parser2 = {
	"readdir2",
	dir_can_take_file,
	dir_take_control_of,
	NULL
};
#endif

/* init_readdir --- set things ups */

static awk_bool_t
init_readdir(void)
{
	register_input_parser(& readdir_parser);
#ifdef TEST_DUPLICATE
	register_input_parser(& readdir_parser2);
#endif

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0, 0, awk_false, NULL }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(init_readdir, func_table, readdir, "")
