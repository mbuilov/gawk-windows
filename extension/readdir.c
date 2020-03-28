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
 * Michael M. Builov: ported to _MSC_VER.
 * mbuilov@gmail.com
 * Ported 3/2020
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

#include "gawkapi.h"

#include "gawkdirfd.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#ifndef PATH_MAX
#define PATH_MAX	1024	/* a good guess */
#endif

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "readdir extension: version 2.0";

static awk_bool_t init_readdir(void);
static awk_bool_t (*init_func)(void) = init_readdir;

GAWK_PLUGIN_GPL_COMPATIBLE

/* data type for the opaque pointer: */

typedef struct open_directory {
#ifndef _MSC_VER
	DIR *dp;
#endif
	char buf[1];
} open_directory_t;

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

/* ftype --- return type of file as a single character string */

static const char *
ftype(struct dirent *entry, const char *dirname)
{
#ifdef DT_BLK
	(void) dirname;		/* silence warnings */
	switch (entry->d_type) {
	case DT_BLK:	return "b";
	case DT_CHR:	return "c";
	case DT_DIR:	return "d";
	case DT_FIFO:	return "p";
	case DT_LNK:	return "l";
	case DT_REG:	return "f";
	case DT_SOCK:	return "s";
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
			return "b";
		if (S_ISCHR(sbuf.st_mode))
			return "c";
		if (S_ISDIR(sbuf.st_mode))
			return "d";
		if (S_ISFIFO(sbuf.st_mode))
			return "p";
		if (S_ISREG(sbuf.st_mode))
			return "f";
#ifdef S_ISLNK
		if (S_ISLNK(sbuf.st_mode))
			return "l";
#endif
#ifdef S_ISSOCK
		if (S_ISSOCK(sbuf.st_mode))
			return "s";
#endif
	}
	return "u";
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
static const char *
ftype_and_ino(
	const WIN32_FIND_DATA *ffd,
	const char *dirname,
	unsigned long long *inode/*out*/)
{
	HANDLE fh;
	char fname[2*MAX_PATH];

	unsigned long long ino = 0; /* zero - in case of error */
	const char *ftstr = "u"; /* unknown - in case of error */

	if (ffd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		ftstr = "l";
	else if (ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		ftstr = "d";

	sprintf(fname, "%s\\%s", dirname, ffd->cFileName);

	fh = CreateFile(fname, 0, 0, NULL, OPEN_EXISTING,
			((ffd->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				? FILE_FLAG_OPEN_REPARSE_POINT : 0u) |
			FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (fh != INVALID_HANDLE_VALUE) {

		BY_HANDLE_FILE_INFORMATION info;

		if (GetFileInformationByHandle(fh, &info))
			ino = file_info_get_inode(&info);

		if ('u' == *ftstr) {
			const DWORD ft = GetFileType(fh);

			switch (ft) {
				case FILE_TYPE_DISK:
					ftstr = "f";
					break;
				case FILE_TYPE_CHAR:
					ftstr = "c";
					break;
				case FILE_TYPE_PIPE:
					if (GetNamedPipeInfo(fh, NULL, NULL, NULL, NULL))
						ftstr = "p";
					else
						ftstr = "s";
					break;
				default:
					break;
			}
		}

		CloseHandle(fh);
	}

	*inode = ino;
	return ftstr;
}

#endif /* _MSC_VER */

/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len,
		const awk_fieldwidth_info_t **unused)
{
#ifndef _MSC_VER
	DIR *dp;
	struct dirent *dirent;
#else
	WIN32_FIND_DATA *ffd;
#endif
	char *out_buf;
	int len;
	open_directory_t *the_dir;
	const char *ftstr;
	unsigned long long ino;

	(void) unused;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	the_dir = (open_directory_t *) iobuf->opaque;
#ifndef _MSC_VER
	dp = the_dir->dp;
#else
	ffd = (WIN32_FIND_DATA*) the_dir->buf;
#endif

#ifndef _MSC_VER

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
	ftstr = ftype(dirent, iobuf->name);
	out_buf = the_dir->buf;

#ifdef __MINGW32__
	len = sprintf(out_buf, "%I64u/%s", ino, dirent->d_name);
#else
	len = sprintf(out_buf, "%llu/%s", ino, dirent->d_name);
#endif

#else /* _MSC_VER */

	if (ffd->dwFileAttributes == ~(DWORD)0) {
		/* no cached entry */
		if (!FindNextFile((HANDLE)(intptr_t)iobuf->fd, ffd)) {
			const DWORD err = GetLastError();
			*errcode = ERROR_NO_MORE_FILES == err ? 0 : err ? (int)err : -1;
			return EOF;
		}
	}

	ftstr = ftype_and_ino(ffd, iobuf->name, &ino);

	/* mark that there is no cached entry */
	ffd->dwFileAttributes = ~(DWORD)0;

	/* dwFileAttributes must be the first member */
	(void) sizeof(int[1-2*!!(
		(char*)&((WIN32_FIND_DATA*)NULL)->dwFileAttributes - (char*)NULL)]);

	/* there must be at least 21 chars between ffd->dwFileAttributes and ffd->cFileName */
	(void) sizeof(int[1-2*!(
		(size_t)((char*)&((WIN32_FIND_DATA*)NULL)->cFileName - (char*)NULL) >=
			sizeof(((WIN32_FIND_DATA*)NULL)->dwFileAttributes) + 21)]);

	/* print max 21 chars, including terminating '\0' */
	len = sprintf((char*)(&ffd->dwFileAttributes + 1), "%I64u", ino);

	ffd->cFileName[-1] = '/';
	out_buf = ffd->cFileName - 1 - len;
	memmove(out_buf, &ffd->dwFileAttributes + 1, (unsigned) len);
	len += 1 + (int)strlen(ffd->cFileName);

#endif /* _MSC_VER */

	len += sprintf(out_buf + len, "/%s", ftstr);

	*out = out_buf;

	*rt_start = NULL;
	*rt_len = 0;	/* set RT to "" */
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
	FindClose((HANDLE) (intptr_t) iobuf->fd);
#endif
	gawk_free(the_dir);

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
#ifdef HAVE_FDOPENDIR
	dp = fdopendir(iobuf->fd);
#else
	dp = opendir(iobuf->name);
	if (dp != NULL)
		iobuf->fd = dirfd(dp);
#endif
	if (dp == NULL) {
		warning(ext_id, _("dir_take_control_of: opendir/fdopendir failed: %s"),
				strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

	/* ensure that offsetof(struct open_directory, buf) == sizeof(DIR*) */
	(void) sizeof(int[1-2*!(sizeof(DIR*) ==
		((char*)&((struct open_directory*)NULL)->buf - (char*)NULL))]);

	size = sizeof(DIR*)
		+ sizeof(struct dirent)
		+ 20 /* max digits in inode */
		+ 2 /* slashes */
		+ 1 /* ftype */;

	emalloc(the_dir, open_directory_t *, size, "dir_take_control_of");

#else /* _MSC_VER */

	HANDLE h;
	char path_buf[MAX_PATH];
	size_t dir_len = strlen(iobuf->name);

	if (dir_len > sizeof(path_buf) - sizeof("\\*")) {
		warning(ext_id, _("dir_take_control_of: too long directory name: %s"),
				iobuf->name);
		update_ERRNO_int(ENAMETOOLONG);
		return awk_false;
	}

	memcpy(path_buf, iobuf->name, dir_len);
	memcpy(path_buf + dir_len, "\\*", sizeof("\\*"));

	size = sizeof(WIN32_FIND_DATA) + 1 /* slash */ + 1 /* ftype */;
	emalloc(the_dir, open_directory_t *, size, "dir_take_control_of");

	/* cache found file in the_dir */
	h = FindFirstFile(path_buf, (WIN32_FIND_DATA*) the_dir->buf);

	/* must find at least '.', otherwise directory path is wrong */
	if (h == INVALID_HANDLE_VALUE ||
		((WIN32_FIND_DATA*) the_dir->buf)->dwFileAttributes == ~(DWORD)0)
	{
		int err = report_opendir_error(iobuf->name);
		gawk_free(the_dir);
		update_ERRNO_int(err);
		return awk_false;
	}

	iobuf->fd = (fd_t) (intptr_t) h; /* only 32 bits of HANDLE are meaningful */

#endif /* _MSC_VER */

#ifndef _MSC_VER
	the_dir->dp = dp;
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
