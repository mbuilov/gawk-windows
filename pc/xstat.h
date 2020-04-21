/*
 * Copyright (C) 2020
 * Michael M. Builov (mbuilov@gmail.com)
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

/* xstat.h */

/* Emulation of stat(2)/lstat(2) for Windows:
  - unlike CRT's stat, set 64-bit inode values,
  - unlike CRT's stat, set real number of links,
  - xlstat do not follows re-parse points.  */

#ifndef XSTAT_H_INCLUDED
#define XSTAT_H_INCLUDED

typedef unsigned short     xdev_t;
typedef unsigned long long xino_t;

struct xstat {
	xdev_t             st_dev;
	unsigned short     st_mode;
	/* Number of links to this file.
	   Not useful for directories.  */
	unsigned int       st_nlink;
	xino_t             st_ino;
	unsigned long long st_size;
	long long          st_atime;
	long long          st_mtime;
	long long          st_ctime;
};

/* This is non-standard for Windows.  */
#define S_IFLNK    (_S_IFREG | _S_IFCHR)
#define	S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)

/* Open file or directory for xfstat.  */
#define xstat_open(wpath, dont_follow) \
	CreateFileW(wpath, FILE_READ_ATTRIBUTES, \
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, OPEN_EXISTING, \
		(DWORD) (((dont_follow) ? FILE_FLAG_OPEN_REPARSE_POINT : 0) | \
			FILE_FLAG_BACKUP_SEMANTICS), \
		NULL)

/* Get stat info by file handle.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   EBADF        - handle is not a file/directory/pipe/character device handle,
   EACCES       - access to directory or file is denied,
   ENOTSUP      - path points to a volume device, like "\\.\C:"  */
extern int xfstat(void *h, const wchar_t *path, struct xstat *buf);

/* Get stat info by file path.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   ENOENT       - path does not point to directory or file,
   EBADF        - path points not to a file/directory/pipe/character device,
   EACCES       - access to directory or file is denied,
   ENOTSUP      - path points to a volume device, like "\\.\C:"  */
extern int xwstat(const wchar_t *path, struct xstat *buf, int dont_follow);

/* Get stat info by file path.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   EILSEQ       - failed to convert path to wide-character string,
   ENAMETOOLONG - resulting path exceeds the limit of 32767 wide characters,
   ENOMEM       - memory allocation failure,
   ENOENT       - path does not point to directory or file,
   EBADF        - path points not to a file/directory/pipe/character device,
   EACCES       - access to directory or file is denied,
   ENOTSUP      - path points to a volume device, like "\\.\C:"  */
extern int xstat(const char *path, struct xstat *buf);

/* Same as xstat, but do not follow a symbolic link.  */
extern int xlstat(const char *path, struct xstat *buf);

/* Try to stat a root drive directory, like "C:\".
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   ENOENT       - path does not point to a root drive directory,
   ENOTSUP      - path points to a volume device, like "\\.\C:"  */
/* Note: xstat/xlstat/xwstat correctly stat root drive directories,
   this function may be usable if there is a need to use xfstat and
   xstat_open() fails - path may point to a root drive directory, but
   the drive may be not ready.  */
extern int xstat_root(const wchar_t *path, struct xstat *buf);

/* Convert given path to L'\0'-terminated wide-character string.
   May return buf if it's big enough, else returns dynamically allocated
    buffer, which must be free()'d after use.
   Assume buf_size is the buf size in wide-characters.
   Returns NULL on error, setting errno to one of:
   EILSEQ       - failed to convert path to wide-character string,
   ENAMETOOLONG - resulting path exceeds the limit of 32767 wide characters,
   ENOMEM       - memory allocation failure.  */
extern wchar_t *xpathwc(const char *path, wchar_t buf[], size_t buf_size);

#endif /* XSTAT_H_INCLUDED */
