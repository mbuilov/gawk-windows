/* wreaddir.h */

/* Emulation of fchdir(2), opendir(3), readdir(3) using native Windows API.  */

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

#ifndef WREADDIR_H_INCLUDED
#define WREADDIR_H_INCLUDED

/* Directory descriptor.  */
typedef struct dir_fd_ {
	HANDLE handle;
	wchar_t abs_path[1];
} *dir_fd_t;

/* Open directory or file.
   If path is NULL, open current directory.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   EILSEQ       - can't convert path to wide-character string,
   ENOMEM       - memory allocation failure,
   ENAMETOOLONG - path is too long,
   EAGAIN       - can't make path absolute,
   ENOENT       - path does not point to directory or file,
   EACCES       - access to directory or file is denied.  */
int opendirfd(const char *path/*NULL?*/, dir_fd_t *dfd/*out*/);

/* Open current directory.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   ENOMEM       - memory allocation failure,
   ENAMETOOLONG - path to current directory is too long,
   EAGAIN       - can't get path to current directory,
   ENOENT       - path does not point to directory or file,
   EACCES       - access to directory or file is denied.  */
#define opencurdirfd(dfd) opendirfd(NULL, dfd)

/* Close directory.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to;
   EINVAL       - directory descriptor is not valid.  */
int closedirfd(dir_fd_t dfd);

/* Change current directory.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   ENOENT       - path does not point to directory or file,
   EACCES       - access to directory or file is denied.  */
int fchdir(dir_fd_t dfd);

/* Directory search structure.  */
struct win_find {
	HANDLE search_handle;
	WIN32_FIND_DATAW ffd;
	struct dir_fd_ dfd;
};

/* Get directory descriptor from the directory search structure.  */
#define dirfd(dirp)	(&(dirp)->dfd)

/* Open directory to enumerate files/directories in it.
   Returns NULL if failed.
   On failure errno can be set to:
   EILSEQ       - can't convert path to wide-character string,
   ENOMEM       - memory allocation failure,
   ENAMETOOLONG - path is too long,
   EAGAIN       - can't make path absolute,
   ENOENT       - path does not point to directory or file,
   EACCES       - access to directory or file is denied,
   EFAULT       - search failed.  */
struct win_find *opendir(const char *path);

/* Close directory opened for the search.
   Returns 0 on success, -1 if failed.
   On failure errno can be set to:
   EINVAL       - directory search structure is not valid.  */
int closedir(struct win_find *dirp);

/* Get next entry of the directory.
   Returns NULL if no more entries or on failure.
   Returned entry must be released via dp_release().
   To be able to detect a failure, set errno to 0 before the call.
   On failure errno can be set to:
   EACCES       - access to directory is denied.  */
WIN32_FIND_DATAW *readdir(struct win_find *dirp);

/* Release entry returned by readdir().  */
#define dp_release(dp) \
	/* mark that there is no cached entry */ \
	((void) ((dp)->dwFileAttributes = ~(DWORD)0))

#endif /* WREADDIR_H_INCLUDED */
