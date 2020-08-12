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

/* To avoid linking with OLDNAMES.LIB, compile with _CRT_DECLARE_NONSTDC_NAMES=0/
  _NO_OLDNAMES/NO_OLDNAMES defined, so some (POSIX) names marked by Microsoft as "old"
  are not defined in system headers.  Define them here.  */

#ifdef fileno
#undef fileno
#endif
#define fileno		_fileno

#ifdef close
#undef close
#endif
#define close		_close

#ifdef dup
#undef dup
#endif
#define dup		_dup

#ifdef dup2
#undef dup2
#endif
#define dup2		_dup2

#ifdef fdopen
#undef fdopen
#endif
#define fdopen		_fdopen

#ifdef pclose
#undef pclose
#endif
#define pclose		_pclose

#ifdef fdopen
#undef fdopen
#endif
#define fdopen		_fdopen

#ifdef lseek
#undef lseek
#endif
#define lseek		_lseek

#ifdef getpid
#undef getpid
#endif
#define getpid		_getpid

#ifdef strdup
#undef strdup
#endif
#define strdup		_strdup

#ifdef setmode
#undef setmode
#endif
#define setmode		_setmode

#ifdef isatty
#undef isatty
#endif
#define isatty		_isatty

#ifdef O_RDONLY
#undef O_RDONLY
#endif
#define O_RDONLY	_O_RDONLY

#ifdef O_WRONLY
#undef O_WRONLY
#endif
#define O_WRONLY	_O_WRONLY

#ifdef O_RDWR
#undef O_RDWR
#endif
#define O_RDWR		_O_RDWR

#ifdef O_CREAT
#undef O_CREAT
#endif
#define O_CREAT		_O_CREAT

#ifdef O_TRUNC
#undef O_TRUNC
#endif
#define O_TRUNC		_O_TRUNC

#ifdef O_APPEND
#undef O_APPEND
#endif
#define O_APPEND	_O_APPEND

#ifdef O_NOINHERIT
#undef O_NOINHERIT
#endif
#define O_NOINHERIT	_O_NOINHERIT

#ifdef S_IFREG
#undef S_IFREG
#endif
#define S_IFREG		_S_IFREG

#ifdef S_IFDIR
#undef S_IFDIR
#endif
#define S_IFDIR		_S_IFDIR

#ifdef S_IFCHR
#undef S_IFCHR
#endif
#define S_IFCHR		_S_IFCHR

#ifdef S_IFMT
#undef S_IFMT
#endif
#define S_IFMT		_S_IFMT

#ifdef S_ISREG
#undef S_ISREG
#endif
#define S_ISREG(m)	(((m) & _S_IFMT) == _S_IFREG)

#ifdef S_ISDIR
#undef S_ISDIR
#endif
#define S_ISDIR(m)	(((m) & _S_IFMT) == _S_IFDIR)

#ifdef S_ISCHR
#undef S_ISCHR
#endif
#define S_ISCHR(m)	(((m) & _S_IFMT) == _S_IFCHR)

#ifdef S_ISFIFO
#undef S_ISFIFO
#endif
#define S_ISFIFO(m)	(((m) & _S_IFMT) == _S_IFIFO)
