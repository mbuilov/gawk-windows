/* wreaddir.c */

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
#include <stdio.h>
#include <sys/stat.h>
#include "xstat.h" /* xpathwc */

#include "wreaddir.h"
#include "gawkapi.h"

static struct dir_fd_ *
alloc_dir_fd(const DWORD abs_sz)
{
	size_t sz;

	/* ensure offsetof(struct dir_fd_, abs_path) == sizeof(HANDLE) */
	(void) sizeof(int[1-2*(sizeof(HANDLE) !=
		(char*) &((struct dir_fd_*) NULL)->abs_path - (char*) NULL)]);

	if (abs_sz > ((size_t)-1 - sizeof(HANDLE))/sizeof(wchar_t)) {
		errno = ENAMETOOLONG;
		return NULL;
	}

	sz = sizeof(HANDLE) + (size_t) abs_sz*sizeof(wchar_t);
	return (struct dir_fd_*) malloc(sz);
}

static void
free_dir_fd(struct dir_fd_ *dfd)
{
	free(dfd);
}

static int
dir_fd_open_handle(struct dir_fd_ *buf)
{
	buf->handle = CreateFileW(buf->abs_path,
		FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

	if (buf->handle == INVALID_HANDLE_VALUE) {
		const DWORD err = GetLastError();
		errno = ERROR_FILE_NOT_FOUND == err ? ENOENT : EACCES;
		return -1;
	}
	return 0;
}

static struct dir_fd_ *
opencurdirfd_(void)
{
	struct dir_fd_ *buf;
	DWORD abs_len;
	const DWORD abs_sz = GetCurrentDirectoryW(0, NULL);

	if (abs_sz == 0) {
		errno = ENOMEM;
		return NULL;
	}

	buf = alloc_dir_fd(abs_sz);
	if (buf == NULL)
		return NULL;

	abs_len = GetCurrentDirectoryW(abs_sz, buf->abs_path);

	if (abs_len != abs_sz - 1) {
		free(buf);
		errno = EAGAIN;
		return NULL;
	}

	if (dir_fd_open_handle(buf)) {
		free(buf);
		return NULL;
	}

	return buf;
}

static struct dir_fd_ *
opendirfd_(const char *path, DWORD *p_abs_len,
	struct dir_fd_ *(*alloc)(DWORD abs_sz),
	void (*dealloc)(struct dir_fd_ *buf))
{
	wchar_t *wp, wpath_buf[MAX_PATH];
	struct dir_fd_ *buf;
	DWORD abs_sz, abs_len;

	wp = xpathwc(path, wpath_buf, sizeof(wpath_buf)/sizeof(wpath_buf[0]));
	if (wp == NULL)
		return NULL;

	abs_sz = GetFullPathNameW(wp, 0, NULL, NULL);
	if (abs_sz == 0) {
		if (wp != wpath_buf)
			free(wp);
		errno = ENOMEM;
		return NULL;
	}

	buf = (*alloc)(abs_sz);
	if (buf == NULL) {
		if (wp != wpath_buf)
			free(wp);
		return NULL;
	}

	abs_len = GetFullPathNameW(wp, abs_sz, buf->abs_path, NULL);

	if (wp != wpath_buf)
		free(wp);

	if (abs_len != abs_sz - 1) {
		(*dealloc)(buf);
		errno = EAGAIN;
		return NULL;
	}

	if (dir_fd_open_handle(buf)) {
		(*dealloc)(buf);
		return NULL;
	}

	*p_abs_len = abs_len;
	return buf;
}

int
opendirfd(const char *path/*NULL?*/, dir_fd_t *dfd/*out*/)
{
	DWORD abs_len;
	struct dir_fd_ *buf = path == NULL ? opencurdirfd_()
		: opendirfd_(path, &abs_len, alloc_dir_fd, free_dir_fd);

	if (buf == NULL)
		return -1;

	*dfd = buf;
	return 0;
}

int
closedirfd(dir_fd_t dfd)
{
	BOOL ok = CloseHandle(dfd->handle);
	free(dfd);
	if (ok)
		return 0;
	errno = EINVAL;
	return -1;
}

int
fchdir(dir_fd_t dfd)
{
	if (!SetCurrentDirectoryW(dfd->abs_path)) {
		const DWORD err = GetLastError();
		errno = ERROR_FILE_NOT_FOUND == err ? ENOENT : EACCES;
		return -1;
	}
	return 0;
}

static struct dir_fd_ *
alloc_win_find(DWORD abs_sz)
{
	struct win_find *f;
	size_t sz;

#define WIN_FIND_HEAD_SIZE	(2*sizeof(HANDLE) + sizeof(WIN32_FIND_DATAW))

	/* ensure offsetof(struct win_find, dfd.abs_path)
	   == WIN_FIND_HEAD_SIZE */
	(void) sizeof(int[1-2*(WIN_FIND_HEAD_SIZE !=
		(char*) &((struct win_find*) NULL)->dfd.abs_path
			- (char*) NULL)]);

	/* Reserve 2 wide-characters for "\*".  */
	if (abs_sz > ((size_t)-1 - WIN_FIND_HEAD_SIZE)/sizeof(wchar_t) - 2) {
		errno = ENAMETOOLONG;
		return NULL;
	}

	sz = WIN_FIND_HEAD_SIZE + ((size_t) abs_sz + 2)*sizeof(wchar_t);

#undef WIN_FIND_HEAD_SIZE

	f = (struct win_find*) malloc(sz);
	if (f == NULL)
		return NULL;

	return &f->dfd;
}

static struct win_find *
win_find_from_dfd(struct dir_fd_ *dfd)
{
	/* ensure offsetof(struct win_find, dfd)
	   == sizeof(HANDLE) + sizeof(WIN32_FIND_DATAW) */
	(void) sizeof(int[1-2*(sizeof(HANDLE) + sizeof(WIN32_FIND_DATAW) !=
		(char*) &((struct win_find*) NULL)->dfd - (char*) NULL)]);

	return (struct win_find*) ((char*)dfd - sizeof(HANDLE) -
		sizeof(WIN32_FIND_DATAW));
}

static void
free_win_find(struct dir_fd_ *dfd)
{
	struct win_find *f = win_find_from_dfd(dfd);
	free(f);
}

struct win_find *
opendir(const char *path)
{
	struct win_find *f;
	DWORD abs_len;
	struct dir_fd_ *dfd = opendirfd_(path, &abs_len,
		alloc_win_find, free_win_find);

	if (dfd == NULL)
		return NULL;

	f = win_find_from_dfd(dfd);

	/* Fill reserved 2 wide-characters for "\*".  */
	if (abs_len == 0 ||
		(f->dfd.abs_path[abs_len - 1] != L'\\' &&
		f->dfd.abs_path[abs_len - 1] != L'/'))
	{
		f->dfd.abs_path[abs_len++] = L'\\';
	}
	f->dfd.abs_path[abs_len++] = L'*';
	f->dfd.abs_path[abs_len] = L'\0';

	f->search_handle = FindFirstFileW(f->dfd.abs_path, &f->ffd);
	if (f->search_handle == INVALID_HANDLE_VALUE ||
		f->ffd.dwFileAttributes == ~(DWORD)0)
	{
		const DWORD last_err = GetLastError();
		const int err = f->search_handle != INVALID_HANDLE_VALUE
			? EFAULT : ERROR_FILE_NOT_FOUND == last_err
			? ENOENT : EACCES;

		if (f->search_handle != INVALID_HANDLE_VALUE)
			(void) FindClose(f->search_handle);
		(void) CloseHandle(f->dfd.handle);
		free(f);
		errno = err;
		return NULL;
	}

	/* Trim the path back.  */
	f->dfd.abs_path[abs_len - 1] = L'\0';

	return f;
}

int
closedir(struct win_find *dirp)
{
	BOOL ok1 = FindClose(dirp->search_handle);
	BOOL ok2 = CloseHandle(dirp->dfd.handle);
	free(dirp);
	if (ok1 && ok2)
		return 0;
	errno = EINVAL;
	return -1;
}

WIN32_FIND_DATAW *
readdir(struct win_find *dirp)
{
	if (dirp->ffd.dwFileAttributes != ~(DWORD)0)
		return &dirp->ffd;

	/* No cached entry.  */
	if (!FindNextFileW(dirp->search_handle, &dirp->ffd)) {
		const DWORD err = GetLastError();
		errno = ERROR_NO_MORE_FILES == err ? 0 : EACCES;
		return NULL;
	}

	return &dirp->ffd;
}
