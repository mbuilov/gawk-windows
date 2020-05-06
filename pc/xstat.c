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

/* xstat.c */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#include "xstat.h"

static int
is_wslash(const wchar_t w)
{
	return w == L'\\' || w == L'/';
}

static wchar_t
is_walpha(const wchar_t w)
{
	if (L'A' <= w && w <= L'Z')
		return L'A';
	if (L'a' <= w && w <= L'z')
		return L'a';
	return L'\0';
}

wchar_t *
xpathwc(const char *path, wchar_t buf[], size_t buf_size)
{
	wchar_t *wp = buf;
	const size_t wplen = mbstowcs(NULL, path, 0);

	if ((size_t)-1 == wplen) {
		errno = EILSEQ;
		return NULL;
	}

	/* Path should not exceed the limit of 32767 wide characters.  */
	if (wplen > 32767 - 1) {
		errno = ENAMETOOLONG;
		return NULL;
	}

	if (wplen + 1 > buf_size) {
		wp = (wchar_t*) malloc(sizeof(*wp)*(wplen + 1));
		if (wp == NULL)
			return NULL;
	}

	(void) mbstowcs(wp, path, wplen + 1);

	return wp;
}

static wchar_t *
get_full_path(const wchar_t *path, wchar_t abs_buf[], size_t abs_buf_size)
{
	wchar_t *ap = _wfullpath(abs_buf, path, abs_buf_size);
	if (ap != NULL)
		return ap;

	ap = _wfullpath(NULL, path, 0);
	return ap;
}

static int
is_unc_root(const wchar_t *ap)
{
	if (!is_wslash(ap[0]) || !is_wslash(ap[1]) || is_wslash(ap[2]))
		return 0;

	ap += 2;
	do {
		if (*ap == L'\0')
			return 0; /* "//" or "//abc" */
	} while (!is_wslash(*++ap));

	if (*++ap == L'\0' || is_wslash(*ap))
		return 0; /* "//x/" or "//x//" */

	do {
		if (*++ap == L'\0')
			return 1; /* "//x/y" */
	} while (!is_wslash(*ap));

	if (*++ap == L'\0')
		return 1; /* "//x/y/" */

	return 0; /* "//x/y/?" */
}

static int
is_root_path(const wchar_t *wp)
{
	/* Check for a path to the root drive directory, like "C:\".  */
	UINT type;
	wchar_t abs_buf[MAX_PATH];
	wchar_t *const ap = get_full_path(wp, abs_buf,
		sizeof(abs_buf)/sizeof(abs_buf[0]));

	if (ap == NULL)
		return 0;

	/* If longer than "C:\", check for a UNC root like "\\host\share\" */
	if (ap[0] && ap[1] && ap[2] && ap[3] && !is_unc_root(ap)) {
		if (ap != abs_buf)
			free(ap);
		return 0;
	}

	type = GetDriveTypeW(ap);

	if (ap != abs_buf)
		free(ap);

	if (DRIVE_UNKNOWN == type ||
		DRIVE_NO_ROOT_DIR == type)
		return 0;

	return 1;
}

static int
wpath_get_dev(const wchar_t *wp, unsigned short *dev)
{
	unsigned short drive;
	wchar_t r;

	/* Skip long path "\\?\" or "\\.\" prefixes.  */
	if (wp[0] == L'\\' && wp[1] == L'\\' &&
		(wp[2] == L'?' || wp[2] == L'.') && wp[3] == L'\\')
		wp += 4;

	r = is_walpha(wp[0]);

	if (r != L'\0' && wp[1] == L':') {

		/* Paths like "C:" are used to access devices,
		   not files/directories.  */
		if (wp[2] == L'\0')
			return -1;

		/* Path like "C:abc.txt" is a path to file "abc.txt"
		   relative to the current directory on disk "C:".  */
		*dev = (unsigned short) (wp[0] - r);
		return 0;
	}

	/* Note: drive will be 0 if current path is a UNC path.  */
	drive = (unsigned short) _getdrive();
	*dev = (unsigned short) (drive - 1);
	return 0;
}

static unsigned long long
hinfo_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}

static unsigned long long
hinfo_get_size(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long size = info->nFileSizeHigh;
	size <<= 32;
	size += info->nFileSizeLow;
	return size;
}

static time_t
hinfo_convert_time(const FILETIME file_time, const time_t fallback)
{
	SYSTEMTIME system_time;
	struct tm tm;
	time_t t;

	if (file_time.dwLowDateTime == 0 && file_time.dwHighDateTime == 0)
		return fallback; /* Not supported by the FS, use fallback.  */

	if (!FileTimeToSystemTime(&file_time, &system_time))
		return -1; /* _mkgmtime() also returns -1 if fails */

	tm.tm_year  = system_time.wYear - 1900;
	tm.tm_mon   = system_time.wMonth - 1;
	tm.tm_mday  = system_time.wDay;
	tm.tm_hour  = system_time.wHour;
	tm.tm_min   = system_time.wMinute;
	tm.tm_sec   = system_time.wSecond;

	t = _mkgmtime(&tm);
	return t;
}

static unsigned short dup_mode_bits(unsigned short mode)
{
	return mode
		| (unsigned short) ((mode & 0700) >> 3) /* group permissions */
		| (unsigned short) ((mode & 0700) >> 6) /* other permissions */
		;
}

int
xstat_root(const wchar_t *wp, struct xstat *buf)
{
	struct tm tm;

	if (!is_root_path(wp)) {
		errno = ENOENT;
		return -1;
	}

	if (wpath_get_dev(wp, &buf->st_dev)) {
		errno = ENOTSUP;
		return -1;
	}

	buf->st_mode = dup_mode_bits(_S_IFDIR | _S_IEXEC | _S_IREAD | _S_IWRITE);

	buf->st_nlink = 1;
	buf->st_ino   = 0;
	buf->st_size  = 0;

	tm.tm_year  = 80; /* 1980 */
	tm.tm_mon   = 0;
	tm.tm_mday  = 1;
	tm.tm_hour  = 0;
	tm.tm_min   = 0;
	tm.tm_sec   = 0;
	tm.tm_isdst = -1;

	buf->st_mtime = _mkgmtime(&tm);
	buf->st_atime = buf->st_mtime;
	buf->st_ctime = buf->st_mtime;

	return 0;
}

static int
is_exe(const wchar_t *wp)
{
	const wchar_t *dot = wcsrchr(wp, L'.');
	if (dot == NULL)
		return 0;

#define CASE_LET(a)	case a: case L'A' + (a - L'a')
#define CMP_DOT(i,a)	(dot[i] == a || dot[i] == L'A' + (a - L'a'))
	switch (dot[1]) {
		CASE_LET(L'b'):
			return
				CMP_DOT(2, L'a') &&
				CMP_DOT(3, L't') &&
				dot[4] == L'\0';
		CASE_LET(L'e'):
			return
				CMP_DOT(2, L'x') &&
				CMP_DOT(3, L'e') &&
				dot[4] == L'\0';
		CASE_LET(L'c'):
			return
				(
					(CMP_DOT(2, L'm') &&
					CMP_DOT(3, L'd'))
					||
					(CMP_DOT(2, L'o') &&
					CMP_DOT(3, L'm'))
				)
				&& dot[4] == L'\0';
		default:
			return 0;
	}
#undef CMP_DOT
#undef CASE_LET
}

static int
xstat_file(HANDLE h, const wchar_t *wp, struct xstat *buf)
{
	BY_HANDLE_FILE_INFORMATION info;

	if (!GetFileInformationByHandle(h, &info)) {
		errno = EACCES;
		return -1;
	}

	if (wp == NULL)
		buf->st_dev = 0;
	else if (wpath_get_dev(wp, &buf->st_dev)) {
		errno = ENOTSUP;
		return -1;
	}

	if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		buf->st_mode = S_IFLNK | _S_IREAD | _S_IWRITE | _S_IEXEC;
	else {
		buf->st_mode = (unsigned short) (
			(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			? _S_IFDIR | _S_IEXEC : wp != NULL && is_exe(wp)
			? _S_IFREG | _S_IEXEC : _S_IFREG);

		buf->st_mode |= (unsigned short) (
			(info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			? _S_IREAD : _S_IREAD | _S_IWRITE);
	}

	buf->st_mode = dup_mode_bits(buf->st_mode);

	buf->st_nlink = info.nNumberOfLinks;
	buf->st_ino   = hinfo_get_inode(&info);
	buf->st_size  = hinfo_get_size(&info);
	buf->st_mtime = hinfo_convert_time(info.ftLastWriteTime,  0);
	buf->st_atime = hinfo_convert_time(info.ftLastAccessTime, buf->st_mtime);
	buf->st_ctime = hinfo_convert_time(info.ftCreationTime,   buf->st_mtime);

	return 0;
}

int
xfstat(HANDLE h, const wchar_t *wp, struct xstat *buf)
{
	const DWORD file_type = GetFileType(h);

	if (file_type == FILE_TYPE_DISK)
		return xstat_file(h, wp, buf);

	if (file_type == FILE_TYPE_CHAR) {
		buf->st_dev   = (unsigned short) -1;
		buf->st_mode  = _S_IFCHR;
		buf->st_nlink = 1;
		buf->st_ino   = 0;
		buf->st_size  = 0;
		buf->st_atime = 0;
		buf->st_mtime = 0;
		buf->st_ctime = 0;
		return 0;
	}

	if (file_type == FILE_TYPE_PIPE) {
		DWORD avail;
		buf->st_dev   = (unsigned short) -1;
		buf->st_mode  = _S_IFIFO;
		buf->st_nlink = 1;
		buf->st_ino   = 0;
		buf->st_size  = PeekNamedPipe(h, NULL, 0, NULL, &avail, NULL)
			? avail : 0;
		buf->st_atime = 0;
		buf->st_mtime = 0;
		buf->st_ctime = 0;
		return 0;
	}

	errno = EBADF;
	return -1;
}

int
xwstat(const wchar_t *wp, struct xstat *buf, int dont_follow)
{
	int r;
	const HANDLE h = xstat_open(wp, dont_follow);

	if (h == INVALID_HANDLE_VALUE) {
		const DWORD open_err = GetLastError();
		r = xstat_root(wp, buf);
		if (r && errno == ENOENT) {
			if (ERROR_ACCESS_DENIED == open_err)
				errno = EACCES;
		}
		return r;
	}

	r = xfstat(h, wp, buf);
	CloseHandle(h);
	return r;
}

static int
xstat_internal(const char *path, struct xstat *buf, int dont_follow)
{
	int r;
	wchar_t *wp, wpath_buf[MAX_PATH];

	/* Empty path is not allowed.  */
	if (!*path) {
		errno = ENOENT;
		return -1;
	}

	wp = xpathwc(path, wpath_buf, sizeof(wpath_buf)/sizeof(wpath_buf[0]));
	if (wp == NULL)
		return -1;

	r = xwstat(wp, buf, dont_follow);
	if (wp != wpath_buf)
		free(wp);
	return r;
}

int
xstat(const char *path, struct xstat *buf)
{
	return xstat_internal(path, buf, /*folow:*/0);
}

int
xlstat(const char *path, struct xstat *buf)
{
	return xstat_internal(path, buf, /*folow:*/1);
}
