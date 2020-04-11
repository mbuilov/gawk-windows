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
 * Reworked 3/2020
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
#elif ! defined _MSC_VER
#error Cannot compile the readdir extension on this system!
#endif

#if defined(__MINGW32__) || defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* Include <locale.h> before "gawkapi.h" redefines setlocale().
  "gettext.h" will include <locale.h> anyway */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "gawkapi.h"

#include "gawkdirfd.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#ifndef PATH_MAX
#define PATH_MAX	1024	/* a good guess */
#endif

GAWK_PLUGIN_GPL_COMPATIBLE

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;

#ifndef READDIR_TEST
static const char *ext_version = "readdir extension: version 2.0";
#else
static const char *ext_version = "readdir extension: version 3.0";
#endif

static awk_bool_t init_readdir(void);
static awk_bool_t (*init_func)(void) = init_readdir;

/* data type for the opaque pointer: */

union u_field_info {
	awk_fieldwidth_info_t fw;
	char buf[awk_fieldwidth_info_size(3)];
};

typedef struct open_directory {
#ifdef READDIR_TEST
	union u_field_info u;
#endif
#ifdef _MSC_VER
	HANDLE ffd_handle;
	unsigned dir_len;
	wchar_t dirname[1];
#else
	DIR *dp;
	char buf[1];
#endif
} open_directory_t;
#define fw u.fw

#if defined(__MINGW32__) || defined(_MSC_VER)
static unsigned long long
file_info_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}
#endif

#ifndef _MSC_VER

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
#ifdef __MINGW32__
	char fname[PATH_MAX];
	HANDLE fh;
	BY_HANDLE_FILE_INFORMATION info;
	unsigned long long ino = 0;

	sprintf(fname, "%s\\%s", dirname, entry->d_name);
	fh = CreateFile(fname, 0, 0, NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return 0;
	if (GetFileInformationByHandle(fh, &info))
		ino = file_info_get_inode(&info);
	CloseHandle(fh);
	return ino;
#else
	(void) dirname;		/* silence warnings */
	return (unsigned long long) entry->d_ino;
#endif
}

#else /* _MSC_VER */

/* ftype_and_ino: use file handle to get both inode number and the file type
  --- return type of file as a single character string,
  --- get the inode of the file */
static int
ftype_and_ino(
	const WIN32_FIND_DATAW *ffd,
	const wchar_t dirname[],
	unsigned dir_len,
	unsigned long long *inode/*out*/)
{
	HANDLE fh;
	wchar_t fname[2*MAX_PATH];

	unsigned long long ino = 0; /* zero - in case of error */
	int ftype = 'u'; /* unknown - in case of error */

	if (ffd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		ftype = 'l'; /* link */
	else if (ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		ftype = 'd'; /* directory */

	memcpy(fname, dirname, dir_len*sizeof(dirname[0]));
	fname[dir_len] = L'\\';
	wcscpy(fname + dir_len + 1, ffd->cFileName);

	fh = CreateFileW(fname, 0, 0, NULL, OPEN_EXISTING,
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
					if (GetNamedPipeInfo(fh, NULL, NULL, NULL, NULL))
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

	*inode = ino;
	return ftype;
}

#endif /* _MSC_VER */

#ifdef _MSC_VER
static unsigned ffd_offset(unsigned dir_len)
{
	unsigned offs;

# ifndef READDIR_TEST

	/* ensure that offsetof(struct open_directory, dir_len) == sizeof(HANDLE) */
	(void) sizeof(int[1-2*!(sizeof(HANDLE) ==
		((char*)&((struct open_directory*)NULL)->dir_len - (char*)NULL))]);

	/* ensure that offsetof(struct open_directory, dirname) ==
		sizeof(HANDLE) + sizeof(unsigned) */
	(void) sizeof(int[1-2*!(sizeof(HANDLE) + sizeof(unsigned) ==
		((char*)&((struct open_directory*)NULL)->dirname - (char*)NULL))]);

	offs = sizeof(HANDLE) + sizeof(unsigned);

# else /* READDIR_TEST */

	/* ensure that offsetof(struct open_directory, ffd_handle)
		== sizeof(union u_field_info) */
	(void) sizeof(int[1-2*!(sizeof(union u_field_info) ==
		((char*)&((struct open_directory*)NULL)->ffd_handle - (char*)NULL))]);

	/* ensure that offsetof(struct open_directory, dir_len)
		== sizeof(union u_field_info) + sizeof(HANDLE) */
	(void) sizeof(int[1-2*!(sizeof(union u_field_info) + sizeof(HANDLE) ==
		((char*)&((struct open_directory*)NULL)->dir_len - (char*)NULL))]);

	/* ensure that offsetof(struct open_directory, dirname)
		== sizeof(union u_field_info) + sizeof(HANDLE) + sizeof(unsigned) */
	(void) sizeof(int[1-2*!(sizeof(union u_field_info) + sizeof(HANDLE) +
			sizeof(unsigned) ==
		((char*)&((struct open_directory*)NULL)->dirname - (char*)NULL))]);

	offs = sizeof(union u_field_info) + sizeof(HANDLE) + sizeof(unsigned);

# endif /* READDIR_TEST */

	offs += dir_len*sizeof(wchar_t) + sizeof(void*) - 1;

	offs &= ~(sizeof(void*) - 1);

	return offs;
}
#endif /* _MSC_VER */

/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len,
		const awk_fieldwidth_info_t **field_width)
{
#ifndef _MSC_VER
	DIR *dp;
	struct dirent *dirent;
#else
	WIN32_FIND_DATAW *ffd;
	size_t converted;
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

#ifndef _MSC_VER

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

#  ifdef __MINGW32__
	len = sprintf(out_buf, "%I64u/%s", ino, dirent->d_name);
#  else
	len = sprintf(out_buf, "%llu/%s", ino, dirent->d_name);
#  endif

# else /* READDIR_TEST */

#  ifdef __MINGW32__
	len = sprintf(out_buf, "%I64u", ino);
#  else
	len = sprintf(out_buf, "%llu", ino);
#  endif
	the_dir->fw.fields[0].len = (unsigned) len;
	len += (flen = sprintf(out_buf + len, "/%s", dirent->d_name));
	the_dir->fw.fields[1].len = (unsigned) flen - 1;

# endif /* READDIR_TEST */

#else /* _MSC_VER */

	ffd = (WIN32_FIND_DATAW*) ((char*) the_dir + ffd_offset(the_dir->dir_len));

	if (ffd->dwFileAttributes == ~(DWORD)0) {
		/* no cached entry */
		if (!FindNextFileW(the_dir->ffd_handle, ffd)) {
			const DWORD err = GetLastError();
			*errcode = ERROR_NO_MORE_FILES == err ? 0 : err ? (int)err : -1;
			return EOF;
		}
	}

	ftype = ftype_and_ino(ffd, the_dir->dirname, the_dir->dir_len, &ino);

	/* mark that there is no cached entry */
	ffd->dwFileAttributes = ~(DWORD)0;

	/* dwFileAttributes must be the first member */
	(void) sizeof(int[1-2*!!(
		(char*)&((WIN32_FIND_DATAW*)NULL)->dwFileAttributes - (char*)NULL)]);

	/* layout of the_dir:
	  <dir_len><dirname><padding><WIN32_FIND_DATAW><MAX_PATH*MB_CUR_MAX> */
	converted = wcstombs((char*)(ffd + 1), ffd->cFileName, MAX_PATH);

	if ((size_t)-1 == converted) {
		*errcode = errno;
		return EOF;
	}

	/* there must be a space for at least 21 characters between
	  ffd->dwfileattributes and ffd->cfilename */
	(void) sizeof(int[1-2*!(
		(size_t)((char*)&((WIN32_FIND_DATAW*)NULL)->cFileName - (char*)NULL) >=
			sizeof(((WIN32_FIND_DATAW*)NULL)->dwFileAttributes) + 21)]);

	/* print max 21 characters, including terminating '\0' */
	out_buf = (char*)(&ffd->dwFileAttributes + 1);
	len = sprintf(out_buf, "%llu", ino);
# ifdef READDIR_TEST
	the_dir->fw.fields[0].len = (unsigned) len;
# endif

	/* there must be a space for 3 more characters: two forward slashes and ftype */
	(void) sizeof(int[1-2*!(sizeof((WIN32_FIND_DATAW*)NULL)->cFileName >= 3)]);

	out_buf[len++] = '/';
	memcpy(out_buf + len, ffd + 1, converted);
	len += (int) converted;
# ifdef READDIR_TEST
	the_dir->fw.fields[1].len = (unsigned) converted;
# endif

#endif /* _MSC_VER */

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

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	the_dir = (open_directory_t *) iobuf->opaque;

#ifndef _MSC_VER
	closedir(the_dir->dp);
#else
	FindClose(the_dir->ffd_handle);
#endif
	free(the_dir);

	iobuf->fd = INVALID_HANDLE;
}

/* dir_can_take_file --- return true if we want the file */

static awk_bool_t
dir_can_take_file(const awk_input_buf_t *iobuf)
{
	if (iobuf == NULL)
		return awk_false;

	return (awk_bool_t) (iobuf->fd != INVALID_HANDLE &&
		S_ISDIR(iobuf->sbuf.st_mode));
}

#ifdef _MSC_VER
static int
report_opendir_error(const char *dirname)
{
	DWORD n_chars;
	char *msg_buf = NULL;
	DWORD last_err = GetLastError();

	if (!last_err)
		last_err = ERROR_INVALID_DATA;

	n_chars = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, last_err, 0, (LPSTR)&msg_buf, 0, NULL);

	/* trim "\r\n" at end of msg */
	if (n_chars && '\n' == msg_buf[n_chars - 1]) {
		if (--n_chars && '\r' == msg_buf[n_chars - 1])
			n_chars--;
		msg_buf[n_chars] = '\0';
	}

	if (n_chars)
		warning(ext_id, _("dir_take_control_of: "
			"couldn't open directory: %s, system error: 0x%x (%s)"),
			dirname, last_err, msg_buf);
	else
		warning(ext_id, _("dir_take_control_of: "
			"couldn't open directory: %s, system error: 0x%x"),
			dirname, last_err);

	LocalFree(msg_buf);
	return (int) last_err;
}
#endif /* _MSC_VER */

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

#ifndef _MSC_VER

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
		warning(ext_id, _("dir_take_control_of: opendir/fdopendir failed: %s"),
				strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

# ifndef READDIR_TEST

	/* ensure that offsetof(struct open_directory, buf) == sizeof(DIR*) */
	(void) sizeof(int[1-2*!(sizeof(DIR*) ==
		((char*)&((struct open_directory*)NULL)->buf - (char*)NULL))]);

	size = sizeof(DIR*);

# else /* READDIR_TEST */

	/* ensure that offsetof(struct open_directory, dp)
		== sizeof(union u_field_info) */
	(void) sizeof(int[1-2*!(sizeof(union u_field_info) ==
		((char*)&((struct open_directory*)NULL)->dp - (char*)NULL))]);

	/* ensure that offsetof(struct open_directory, buf)
		== sizeof(union u_field_info) + sizeof(DIR*) */
	(void) sizeof(int[1-2*!(sizeof(union u_field_info) + sizeof(DIR*) ==
		((char*)&((struct open_directory*)NULL)->buf - (char*)NULL))]);

	size = sizeof(union u_field_info) + sizeof(DIR*);

# endif /* READDIR_TEST */

	size += sizeof(struct dirent)
		+ 20 /* max digits in inode */
		+ 2 /* slashes */
		+ 1 /* ftype */;

	emalloc(the_dir, open_directory_t *, size, "dir_take_control_of");

	the_dir->dp = dp;

#else /* _MSC_VER */

	HANDLE h;
	wchar_t path_buf[MAX_PATH];
	unsigned offset;
	WIN32_FIND_DATAW *ffd;

	size_t converted = mbstowcs(NULL, iobuf->name, 0);
	if ((size_t)-1 == converted) {
		warning(ext_id, _("dir_take_control_of: opendir/fdopendir failed: %s"),
				strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

	if (converted > (sizeof(path_buf) - sizeof(L"\\*"))/sizeof(path_buf[0])) {
		warning(ext_id, _("dir_take_control_of: too long directory name: %s"),
				iobuf->name);
		update_ERRNO_int(ENAMETOOLONG);
		return awk_false;
	}

	(void) mbstowcs(path_buf, iobuf->name, converted);
	memcpy(path_buf + converted, L"\\*", sizeof(L"\\*"));

	offset = ffd_offset((unsigned) converted);
	size = offset
		+ sizeof(WIN32_FIND_DATAW)
		+ MAX_PATH*MB_CUR_MAX;

	emalloc(the_dir, open_directory_t *, size, "dir_take_control_of");

	/* cache found file in the_dir */
	ffd = (WIN32_FIND_DATAW*) ((char*) the_dir + offset);
	h = FindFirstFileW(path_buf, ffd);

	/* must find at least '.', otherwise directory path is wrong */
	if (h == INVALID_HANDLE_VALUE || ffd->dwFileAttributes == ~(DWORD)0) {
		int err = report_opendir_error(iobuf->name);
		free(the_dir);
		update_ERRNO_int(err);
		return awk_false;
	}

	the_dir->ffd_handle = h;
	the_dir->dir_len = (unsigned) converted;
	memcpy(the_dir->dirname, path_buf, converted*sizeof(path_buf[0]));

#endif /* _MSC_VER */

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

dl_load_func(func_table, readdir, "")
