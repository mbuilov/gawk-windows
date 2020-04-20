/* wreadlink.h */

/* Emulation of readlink(2) using native Windows API.  */

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

#ifndef WREADLINK_H_INCLUDED
#define WREADLINK_H_INCLUDED

/* Get the path symbolic link points to by file handle.
   File must exist, must be opened with FILE_FLAG_OPEN_REPARSE_POINT flag.
   Assume bufsiz is the buf size in wide-characters.
   Returns number of wide characters placed in buf, -1 if failed.
   On failure errno can be set to:
   EINVAL       - ReparsePoint IOCTL on file handle failed,
   ENOMEM       - memory allocation failure,
   ENOTSUP      - ReparsePoint file has unsupported format,
   ERANGE       - output buffer size is too small.  */
/* Note: on success, buf is not L'\0'-terminated.  */
int wreadlinkfd(HANDLE h, wchar_t *buf, size_t bufsiz);

/* Get the path symbolic link points to.
   Assume bufsiz is the buf size in wide-characters.
   Returns number of wide characters placed in buf, -1 if failed.
   On failure errno can be set to:
   EACCES       - access to directory or file is denied,
   ENOENT       - path does not point to directory or file,
   EINVAL       - path does not point to ReparsePoint file,
   ENOMEM       - memory allocation failure,
   ENOTSUP      - ReparsePoint file has unsupported format,
   ERANGE       - output buffer size is too small.  */
/* Note: on success, buf is not L'\0'-terminated.  */
int wreadlink(const wchar_t *path, wchar_t *buf, size_t bufsiz);

/* Get the path symbolic link points to by file handle.
   File must exist, must be opened with FILE_FLAG_OPEN_REPARSE_POINT flag.
   Returns number of bytes placed in buf, -1 if failed.
   On failure errno can be set to:
   EINVAL       - ReparsePoint IOCTL on file handle failed,
   ENOMEM       - memory allocation failure,
   ENOTSUP      - ReparsePoint file has unsupported format,
   EILSEQ       - can't convert resulting path to wide-character string,
   ERANGE       - output buffer size is too small,
   ENAMETOOLONG - resulting path length > INT_MAX.  */
/* Note: on success, buf is not '\0'-terminated.  */
int readlinkfd(HANDLE h, char *buf, size_t bufsiz);

/* Get the path symbolic link points to.
   Returns number of bytes placed in buf, -1 if failed.
   On failure errno can be set to:
   EACCES       - access to directory or file is denied,
   ENOENT       - path does not point to directory or file,
   EINVAL       - path does not point to ReparsePoint file,
   ENOMEM       - memory allocation failure,
   ENOTSUP      - ReparsePoint file has unsupported format,
   EILSEQ       - failed to convert input path to wide-character string,
   EILSEQ       - can't convert resulting path to wide-character string,
   ERANGE       - output buffer size is too small,
   ENAMETOOLONG - input path exceeds the limit of 32767 wide characters,
   ENAMETOOLONG - resulting path length > INT_MAX.  */
/* Note: on success, buf is not '\0'-terminated.  */
int readlink(const char *path, char *buf, size_t bufsiz);

#endif /* WREADLINK_H_INCLUDED */
