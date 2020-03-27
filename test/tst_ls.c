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

#include <stdio.h>

static unsigned long long
file_info_get_inode(const BY_HANDLE_FILE_INFORMATION *info)
{
	unsigned long long inode = info->nFileIndexHigh;
	inode <<= 32;
	inode += info->nFileIndexLow;
	return inode;
}

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

static HANDLE open_dir(
	const char prog[],
	const char dir[],
	WIN32_FIND_DATA *ffd/*out*/)
{
	HANDLE h;
	char path_buf[MAX_PATH];
	size_t dir_len = strlen(dir);

	if (dir_len > sizeof(path_buf) - sizeof("\\*")) {
		fprintf(stderr, "%s: too long directory path: \"%s\"\n",
			prog, dir);
		return INVALID_HANDLE_VALUE;
	}

	memcpy(path_buf, dir, dir_len);
	memcpy(path_buf + dir_len, "\\*", sizeof("\\*"));

	h = FindFirstFile(path_buf, ffd);
	if (INVALID_HANDLE_VALUE == h) {
		fprintf(stderr, "%s: can't open directory \"%s\"\n",
			prog, dir);
		return INVALID_HANDLE_VALUE;
	}

	return h;
}

int main(int argc, char *argv[])
{
	WIN32_FIND_DATA ffd;
	HANDLE h;

	const char *const opts = argv[1];

	if (argc != 3 || (strcmp("-afi", opts) && strcmp("-lna", opts))) {
		fprintf(stderr, "usage: %s [-afi|-lna] <dir>\n", argv[0]);
		return 2;
	}

	h = open_dir(argv[0], argv[2], &ffd);
	if (INVALID_HANDLE_VALUE == h)
		return 2;

	{
		const int afi = 0 == strcmp("-afi", opts);

		/* print fake total */
		if (!afi)
			fprintf(stdout, "total 555\n");

		do {
			unsigned long long ino;

			const char *ftype = ftype_and_ino(
				&ffd, argv[2], &ino);

			if (afi) {
				/* ls -afi */
				fprintf(stdout, "%I64u %s\n",
					ino, ffd.cFileName);
			}
			else {
				/* ls -lna */
				unsigned name_len = (unsigned) strlen(ffd.cFileName);
				fprintf(stdout, "%c%s %2.u 1000 1000 %8.u Mar 27 2016 %s%s\n",
					'd' == *ftype ? 'd' :
					'l' == *ftype ? 'l' :
					'-',
					'd' == *ftype ? "rwxr-xr-x" :
					'l' == *ftype ? "rwxrwxrwx" :
					"rwxrw-r--",
					name_len /* fake */,
					256*name_len /* fake */,
					ffd.cFileName,
					'l' == *ftype ? " -> xxx" : "" /* fake */);
			}
		} while (FindNextFile(h, &ffd));

		{
			const DWORD err = GetLastError();
			FindClose(h);

			if (ERROR_NO_MORE_FILES != err) {
				fprintf(stderr, "%s: FindNextFile() failed with error: %lx\n",
					argv[0], err);
				return 2;
			}
		}
	}

	return 0;
}
