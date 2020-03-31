/*
 * tst_ls.c --- Generate output similar to "ls -afi" or "ls -lna"
 *  for the readdir test on Windows
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Written 3/2020
 */

/*
 * Copyright (C) 2020 the Free Software Foundation, Inc.
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

#ifndef _MSC_VER
#error for _MSC_VER only!
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

static unsigned long long
file_info_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}

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

static int
set_lcat_from_env(const wchar_t prog[], int cat, const wchar_t name[])
{
	const wchar_t *lc = _wgetenv(name);
	if (lc && lc[0] && !_wsetlocale(cat, lc)) {
		fwprintf(stderr, L"%s: bad locale: %s=%s\n", prog, name, lc);
		return -1;
	}
	return 0;
}

/* Set locale based on the values of environment variables.  */
static int
set_locale_from_env(const wchar_t prog[], const wchar_t def[])
{
	const wchar_t *lc = _wgetenv(L"LC_ALL");
	if (lc && lc[0]) {
		if (!_wsetlocale(LC_ALL, lc)) {
			fwprintf(stderr, L"%s: bad locale: LC_ALL=%s\n",
				prog, lc);
			return -1;
		}
		return 0;
	}

	/* LANG is the default for all LC_...  */
	lc = _wgetenv(L"LANG");

	/* if LANG is not defined - set the system default.
	   (If def is "", locale may be be set to e.g. Russian_Russia.1251).  */
	if (!lc || !lc[0])
		lc = def;

	if (!_wsetlocale(LC_ALL, lc)) {
		fwprintf(stderr, L"%s: bad locale: LANG=%s\n", prog, lc);
		return -1;
	}

	/* set all locale categories from the environment:
	   specific LC_... take precedence over LANG.  */
	if (set_lcat_from_env(prog, LC_CTYPE,    L"LC_CTYPE") ||
		set_lcat_from_env(prog, LC_COLLATE,  L"LC_COLLATE") ||
		set_lcat_from_env(prog, LC_MONETARY, L"LC_MONETARY") ||
		set_lcat_from_env(prog, LC_NUMERIC,  L"LC_NUMERIC") ||
		set_lcat_from_env(prog, LC_TIME,     L"LC_TIME"))
		return -1;

	return 0;
}

static HANDLE open_dir(
	const wchar_t prog[],
	const wchar_t dirname[],
	size_t dir_len,
	WIN32_FIND_DATAW *ffd/*out*/)
{
	HANDLE h;
	wchar_t path_buf[MAX_PATH];

	if (dir_len > (sizeof(path_buf) - sizeof(L"\\*"))/sizeof(wchar_t)) {
		fwprintf(stderr, L"%s: too long directory path: \"%s\"\n",
			prog, dirname);
		return INVALID_HANDLE_VALUE;
	}

	memcpy(path_buf, dirname, dir_len*sizeof(dirname[0]));
	memcpy(path_buf + dir_len, L"\\*", sizeof(L"\\*"));

	h = FindFirstFileExW(path_buf, FindExInfoStandard, ffd, FindExSearchNameMatch, NULL, 0);
	if (INVALID_HANDLE_VALUE == h) {
		fwprintf(stderr, L"%s: can't open directory \"%s\"\n",
			prog, dirname);
		return INVALID_HANDLE_VALUE;
	}

	return h;
}

static int usage(const wchar_t *prog)
{
	fwprintf(stderr, L"Usage: %s [--locale=<locale>] (-afi|-lna) <dir>\n", prog);
	return 2;
}

int wmain(int argc, wchar_t *wargv[])
{
	WIN32_FIND_DATAW ffd;
	HANDLE h;

	int afi;
	const wchar_t *dirname;
	const wchar_t *locale = NULL;	/* optional */
	wchar_t **w = wargv + 1;
	size_t dir_len;

	if (argc < 3 || argc > 4)
		return usage(wargv[0]);

	for (;; w++) {
		if (!wcsncmp(*w, L"--locale=", wcslen(L"--locale="))) {
			if (locale)
				return usage(wargv[0]);
			locale = *w + wcslen(L"--locale=");
			continue;
		}
		if (!wcscmp(*w, L"-afi")) {
			afi = 1;
			break;
		}
		if (!wcscmp(*w, L"-lna")) {
			afi = 0;
			break;
		}
		return usage(wargv[0]);
	}

	dirname = *++w;
	if (!dirname || w[1])
		return usage(wargv[0]);

	if (locale) {
		if (!_wsetlocale(LC_ALL, locale)) {
			fwprintf(stderr, L" %s: bad locale: %s\n", wargv[0], locale);
			return 2;
		}
	}
	else if (set_locale_from_env(wargv[0], L""))
		return 2;

	dir_len = wcslen(dirname);
	h = open_dir(wargv[0], dirname, dir_len, &ffd);
	if (INVALID_HANDLE_VALUE == h)
		return 2;

	/* print fake total */
	if (!afi)
		fwprintf(stdout, L"total 555\n");

	do {
		unsigned long long ino;

		int ftype = ftype_and_ino(&ffd, dirname, (unsigned) dir_len, &ino);

		if (afi) {
			/* ls -afi */
			fwprintf(stdout, L"%llu %s\n", ino, ffd.cFileName);
		}
		else {
			/* ls -lna */
			unsigned name_len = (unsigned) wcslen(ffd.cFileName);
			fwprintf(stdout, L"%c%s %2.u 1000 1000 %8.u Mar 27 2016 %s%s\n",
				'd' == ftype ? L'd' :
				'l' == ftype ? L'l' :
				L'-',
				'd' == ftype ? L"rwxr-xr-x" :
				'l' == ftype ? L"rwxrwxrwx" :
				L"rwxrw-r--",
				name_len /* fake */,
				256*name_len /* fake */,
				ffd.cFileName,
				'l' == ftype ? L" -> xxx" : L"" /* fake */);
		}

	} while (FindNextFileW(h, &ffd));

	{
		const DWORD err = GetLastError();
		FindClose(h);

		if (ERROR_NO_MORE_FILES != err) {
			fwprintf(stderr, L"%s: FindNextFileW() failed with error: %lx\n",
				wargv[0], err);
			return 2;
		}
	}

	return 0;
}
