/* wreadlink.c */

/*
 * Copyright (C) 2020 Michael. M. Builov (mbuilov@gmail.com).
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <sys/stat.h>
#include "xstat.h" /* xpathwc */

#include "wreadlink.h"
#include "gawkapi.h"

#ifndef INT_MAX
#define INT_MAX ((unsigned)-1/2)
#endif

/* Ntifs.h may be not available in application WDK,
   define structure according to
   https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer */
typedef struct reparse_buf {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	} u;
} REPARSE_DATA_BUFFER;

static WCHAR *
readlink_ioctl(HANDLE h, REPARSE_DATA_BUFFER *rdb, DWORD rdb_size,
	REPARSE_DATA_BUFFER **dyn_rdb, USHORT *link_len)
{
	WCHAR *pbuf;
	USHORT offs, plen;	/* in bytes */
	DWORD filled;

	/* Assume path is less than 32768 wide characters.  */
	const DWORD add_by = 32768*sizeof(WCHAR);

	while (!DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0,
		rdb, rdb_size, &filled, NULL))
	{
		const DWORD err = GetLastError();
		if (err != ERROR_MORE_DATA) {
			errno = EINVAL;
			return NULL;
		}
		if (rdb_size > (DWORD)-1 - add_by) {
			errno = ENOMEM;
			return NULL;
		}
		if (*dyn_rdb != NULL)
			free(*dyn_rdb);
		rdb_size += add_by;
		*dyn_rdb = (REPARSE_DATA_BUFFER*) malloc(rdb_size);
		if (*dyn_rdb == NULL)
			return NULL;
		rdb = *dyn_rdb;
	}

	if (filled < 8)
		goto notsup;

	if (!IsReparseTagMicrosoft(rdb->ReparseTag))
		goto notsup;

	switch (rdb->ReparseTag) {
	case IO_REPARSE_TAG_SYMLINK:
		/* fields of rdb->SymbolicLinkReparseBuffer must be filled.  */
		if (rdb->ReparseDataLength < 12)
			goto notsup;
		offs = rdb->u.SymbolicLinkReparseBuffer.SubstituteNameOffset;
		plen = rdb->u.SymbolicLinkReparseBuffer.SubstituteNameLength;
		if (rdb->ReparseDataLength < 12 + offs + plen + sizeof(L'\0'))
			goto notsup;
		pbuf = rdb->u.SymbolicLinkReparseBuffer.PathBuffer;
		break;
	case IO_REPARSE_TAG_MOUNT_POINT:
		/* fields of rdb->MountPointReparseBuffer must be filled.  */
		if (rdb->ReparseDataLength < 8)
			goto notsup;
		offs = rdb->u.MountPointReparseBuffer.SubstituteNameOffset;
		plen = rdb->u.MountPointReparseBuffer.SubstituteNameLength;
		if (rdb->ReparseDataLength < 8 + offs + plen + sizeof(L'\0'))
			goto notsup;
		pbuf = rdb->u.MountPointReparseBuffer.PathBuffer;
		break;
	default:
		goto notsup;
	}

	*link_len = plen/sizeof(WCHAR);
	return &pbuf[offs/sizeof(WCHAR)];

notsup:
	errno = ENOTSUP;
	return NULL;
}

int
wreadlinkfd(HANDLE h, wchar_t *buf, size_t bufsiz)
{
	int ret = -1;

	union {
		REPARSE_DATA_BUFFER buf;
		char mem[sizeof(REPARSE_DATA_BUFFER) +
			MAX_PATH*sizeof(WCHAR)];
	} u;
	REPARSE_DATA_BUFFER *dyn_rdb = NULL;

	USHORT link_len;
	const WCHAR *link = readlink_ioctl(h, &u.buf, sizeof(u),
		&dyn_rdb, &link_len);

	if (link == NULL)
		goto err;

	if (link_len > bufsiz) {
		errno = ERANGE;
		goto err;
	}
	memcpy(buf, link, link_len*sizeof(*link));

	/* Ensure link_len <= INT_MAX.  */
	(void) sizeof(int[1-2*((USHORT)-1 > INT_MAX)]);

	ret = (int) link_len;
err:
	if (dyn_rdb != NULL)
		free(dyn_rdb);
	return ret;
}

int
readlinkfd(HANDLE h, char *buf, size_t bufsiz)
{
	int ret = -1;
	size_t converted;

	union {
		REPARSE_DATA_BUFFER buf;
		char mem[sizeof(REPARSE_DATA_BUFFER) +
			MAX_PATH*sizeof(wchar_t)];
	} u;
	REPARSE_DATA_BUFFER *dyn_rdb = NULL;

	USHORT link_len;
	const WCHAR *link = readlink_ioctl(h, &u.buf, sizeof(u),
		&dyn_rdb, &link_len);

	if (link == NULL)
		goto err;

	/* Convert wide-character path to multibyte string.  */
	converted = wcstombs(buf, link, bufsiz);
	if ((size_t)-1 == converted) {
		errno = EILSEQ;
		goto err;
	}

	/* Make sure that conversion wasn't stopped due to small buffer.  */
	if (bufsiz < 10 || converted > bufsiz - 10/*MB_CUR_MAX*/) {
		const size_t n = wcstombs(NULL, link, 0);
		if (n != converted) {
			errno = (size_t)-1 == n ? EILSEQ : ERANGE;
			goto err;
		}
	}

	if (converted > INT_MAX) {
		errno = ENAMETOOLONG;
		goto err;
	}

	ret = (int) converted;
err:
	if (dyn_rdb != NULL)
		free(dyn_rdb);
	return ret;
}

int
readlink(const char *path, char *buf, size_t bufsiz)
{
	int r;
	wchar_t *wp, wpath_buf[MAX_PATH];
	BY_HANDLE_FILE_INFORMATION info;
	HANDLE h;

	wp = xpathwc(path, wpath_buf, sizeof(wpath_buf)/sizeof(wpath_buf[0]));
	if (wp == NULL)
		return -1;

	h = xstat_open(wp, /*dont_follow:*/1);

	if (wp != wpath_buf)
		free(wp);

	if (h == INVALID_HANDLE_VALUE) {
		const DWORD err = GetLastError();
		errno = ERROR_ACCESS_DENIED == err ? EACCES : ENOENT;
		return -1;
	}

	if (!GetFileInformationByHandle(h, &info)) {
		CloseHandle(h);
		errno = EACCES;
		return -1;
	}

	if (!(info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
		CloseHandle(h);
		errno = EINVAL;
		return -1;
	}

	r = readlinkfd(h, buf, bufsiz);
	CloseHandle(h);
	return r;
}

int
wreadlink(const wchar_t *path, wchar_t *buf, size_t bufsiz)
{
	int r;
	BY_HANDLE_FILE_INFORMATION info;
	HANDLE h = xstat_open(path, /*dont_follow:*/1);

	if (h == INVALID_HANDLE_VALUE) {
		const DWORD err = GetLastError();
		errno = ERROR_ACCESS_DENIED == err ? EACCES : ENOENT;
		return -1;
	}

	if (!GetFileInformationByHandle(h, &info)) {
		CloseHandle(h);
		errno = EACCES;
		return -1;
	}

	if (!(info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
		CloseHandle(h);
		errno = EINVAL;
		return -1;
	}

	r = wreadlinkfd(h, buf, bufsiz);
	CloseHandle(h);
	return r;
}
