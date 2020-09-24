/*
 * tst_ls.c --- Generate output similar to "ls -afi" or "ls -lna"
 *  for the readdir test on Windows
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Written 4-9/2020
 */

/*
 * Copyright (C) 2020 Michael M. Builov (mbuilov@gmail.com)
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

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#error for _MSC_VER or __MINGW32__ only!
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>

#include "libutf16/utf16_to_utf8.h"
#include "mscrtx/arg_parser.h"
#include "mscrtx/utf8env.h"
#include "mscrtx/locale_helpers.h"
#include "mscrtx/localerpl.h"
#include "mscrtx/consoleio.h"

/* wprintf-format specifier for unsigned long long.
  Note: under WindowsXP, msvcrt.dll do not supports "%llu".  */
#ifndef LLUWFMT
# define LLUWFMT L"I64u"
#endif

/* exact copy of the same function in extension/readdir.c */
static unsigned long long
file_info_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}

/* exact copy of the same function in extension/readdir.c */
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

static HANDLE open_dir(
	const wchar_t prog[],
	const wchar_t dirname[],
	size_t dir_len,
	WIN32_FIND_DATAW *ffd/*out*/)
{
	HANDLE h;
	wchar_t path_buf[MAX_PATH], *path = path_buf;

	/* Path should not exceed the limit of 32767 wide characters.  */
	if (dir_len > 32767 - 3) {
		fwprintfmb(stderr, L"%ls: too long directory path: \"%ls\"\n",
			prog, dirname);
		return INVALID_HANDLE_VALUE;
	}

	if (dir_len > sizeof(path_buf)/sizeof(path_buf[0]) - 3) {
		size_t sz = sizeof(wchar_t)*(dir_len + 3);
		path = (wchar_t*) malloc(sz);
		if (path == NULL) {
			fwprintfmb(stderr, L"%ls: can't allocate memory of %u bytes\n",
				prog, 0u + (unsigned) sz);
			return INVALID_HANDLE_VALUE;
		}
	}

	memcpy(path, dirname, dir_len*sizeof(dirname[0]));
	memcpy(path + dir_len, L"\\*", 3*sizeof(wchar_t));

	h = FindFirstFileExW(path, FindExInfoStandard, ffd, FindExSearchNameMatch, NULL, 0);
	if (path != path_buf)
		free(path);
	if (INVALID_HANDLE_VALUE == h) {
		fwprintfmb(stderr, L"%ls: can't open directory \"%ls\"\n",
			prog, dirname);
		return INVALID_HANDLE_VALUE;
	}

	return h;
}

static int process_dir(const wchar_t prog[], const wchar_t dirname[], const int afi)
{
	WIN32_FIND_DATAW ffd;

	const size_t dir_len = wcslen(dirname);
	const HANDLE h = open_dir(prog, dirname, dir_len, &ffd);
	if (INVALID_HANDLE_VALUE == h)
		return 2;

	/* print fake total */
	if (!afi)
		puts("total 555");

	do {
		unsigned long long ino;

		int ftype = ftype_and_ino(&ffd, dirname, (unsigned) dir_len, &ino);

		if (afi) {
			/* ls -afi */
			wprintfmb(L"%" LLUWFMT L" %ls\n", ino, ffd.cFileName);
		}
		else {
			/* ls -lna */
			unsigned name_len = (unsigned) wcslen(ffd.cFileName);
			wprintfmb(L"%lc%ls %2.u 1000 1000 %8.u Mar 27 2016 %ls%ls\n",
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
			fwprintfmb(stderr, L"%ls: FindNextFileW() failed with error: %lx\n",
				prog, (unsigned long) err);
			return 2;
		}
	}
	return 0;
}

static int usage(const wchar_t prog[])
{
	const wchar_t *name = wcsrchr(prog, L'\\');
	name = name != NULL ? name + 1 : prog;
	fwprintfmb(stderr, L"%ls - Generate output similar to \"ls -afi\" or \"ls -lna\" for the gawk readdir test on Windows\n", name);
	fwprintfmb(stderr, L"Usage: %ls [--locale=<locale>] (-afi|-lna) <dir>\n", name);
	return 2;
}

#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
# if defined(__GNUC__) || defined(__clang__)
/* gcc/clang complains about missing protope of wmain().  Provide one.  */
int wmain(int argc, wchar_t *argv[]);
# endif
int wmain(int argc, wchar_t *argv[])
#else
/* mingw32 from MinGW.org (not 32-bit variant of mingw-w64) do not supports wmain(),
  so the CRT startup code will convert wide-character program arguments to multibyte ones,
  but this is just a waste of time, since we ignore the arguments completely. */
int main(int argc, char *argv[])
#endif
{
	struct wide_arg *const wargs = arg_parse_command_line(&argc);

	if (wargs == NULL) {
		fputs("Failed to parse command-line arguments list\n", stderr);
		return 2;
	}

	/* Not used, since MinGW version do not handles escaping of
	   double quote via two double-quotes, like: "1""2".  */
	(void)argv;

	{
		int afi;
		const wchar_t *locale = NULL;	/* optional */
		const wchar_t *const prog = wargs->value;
		struct wide_arg *w = wargs->next;

		if (argc < 3 || argc > 4)
			return usage(prog);

		for (;; w = w->next) {
			if (!wcsncmp(w->value, L"--locale=", wcslen(L"--locale="))) {
				if (locale)
					return usage(prog);
				locale = w->value + wcslen(L"--locale=");
				continue;
			}
			if (!wcscmp(w->value, L"-afi")) {
				afi = 1;
				break;
			}
			if (!wcscmp(w->value, L"-lna")) {
				afi = 0;
				break;
			}
			return usage(prog);
		}

		w = w->next;
		if (!w || w->next)
			return usage(prog);

		if (locale) {
			if (wset_locale_helper(LC_ALL, locale)) {
				fwprintfmb(stderr, L"%ls: bad locale: %ls\n", prog, locale);
				return 2;
			}
		}
		else {
			struct set_locale_err err;
			if (set_locale_from_env("", &err)) {
				if (err.lc)
					fwprintfmb(stderr, L"%ls: bad locale: %ls=%ls\n", prog, err.cat, err.lc);
				else
					fwprintfmb(stderr, L"%ls: failed to set default locale\n", prog);
				return 2;
			}
		}

		if (process_dir(prog, w->value, afi))
			return 2;
	}

	arg_free_wide_args(wargs);
	console_buffers_reset();
	utf8_env_shadow_reset();
	return 0;
}

void utf8_env_fatal(void)
{
	fputs("failed to create environment variables table\n", stderr);
	exit(3);
}
