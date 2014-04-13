/* dirfd.c -- return the file descriptor associated with an open DIR*

   Copyright (C) 2001, 2006, 2008-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Jim Meyering. */

/* Modified for gawk */

#include <config.h>

#ifndef ENOTSUP
# define ENOTSUP ENOSYS
#endif

/*
 * This is for fake directory file descriptors on systems that don't
 * allow to open() a directory.
 *
 * Including a header from the main gawk source to share the definition
 * of FAKE_FD_VALUE is the least of all evils that I can see.
 *
 * Unlike the main gawk code base, this include is NOT dependant
 * upon MinGW or EMX.
 */
#ifndef __VMS
#include "../nonposix.h"
#else
#include "nonposix.h"
#endif

#ifndef DIR_TO_FD
# define DIR_TO_FD(d) (FAKE_FD_VALUE)
#endif

#if !defined(HAVE_DIRFD) && (!defined(HAVE_DECL_DIRFD) || HAVE_DECL_DIRFD == 0)
int
dirfd (DIR *dir_p)
{
  int fd = DIR_TO_FD (dir_p);
  if (fd == -1)
    errno = ENOTSUP;
  return fd;
}
#endif /* HAVE_DIRFD */
