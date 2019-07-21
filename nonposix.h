/*
 * nonposix.h --- definitions needed on non-POSIX systems.
 */

/*
 * Copyright (C) 2012, 2013, 2016, 2017, 2018, 2019
 * the Free Software Foundation, Inc.
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

/*
 * This is for fake directory file descriptors on systems that don't
 * allow to open() a directory.
 */

#define FAKE_FD_VALUE 42

#ifdef __MINGW32__
/* Replacements for sys/wait.h macros.  */
# define WEXITSTATUS(stv) (((unsigned)(stv)) & ~0xC0000000)
/* MS-Windows programs that crash due to a fatal exception exit with
   an exit code whose 2 MSB bits are set.  */
# define WIFEXITED(stv)   ((((unsigned)(stv)) & 0xC0000000) == 0)
# define WIFSIGNALED(stv) ((((unsigned)(stv)) & 0xC0000000) == 0xC0000000)
# define WTERMSIG(stv)    w32_status_to_termsig ((unsigned)stv)
# define WIFSTOPPED(stv)  (0)
# define WSTOPSIG(stv)    (0)

int w32_status_to_termsig (unsigned);

/* Prototypes of for Posix functions for which we define replacements
   in pc/ files.  */

/* getid.c */
unsigned int getuid (void);
unsigned int geteuid (void);
unsigned int getgid (void);
unsigned int getegid (void);

/* gawkmisc.pc */
int unsetenv (const char *);
int setenv (const char *, const char *, int);
void w32_maybe_set_errno (void);
char *w32_setlocale (int, const char *);
/* libintl.h from GNU gettext defines setlocale to redirect that to
   its own function.  Note: this will have to be revisited if MinGW
   Gawk will support ENABLE_NLS at some point.  */
#ifdef setlocale
# undef setlocale
#endif
#define setlocale(c,v) w32_setlocale(c,v)

#endif	/* __MINGW32__ */

#if defined(VMS) || defined(__DJGPP__) || defined(__MINGW32__)
int getpgrp(void);
#endif

#if defined(__DJGPP__) || defined(__MINGW32__)
int getppid(void);
#endif

#ifdef __DJGPP__
/* Prototypes of for Posix functions for which we define replacements
   in pc/ files.  */
wint_t btowc (int c);
wint_t putwc (wchar_t wc, FILE *stream);
#endif

#ifdef __EMX__

char *os2_fixdllname(char *dst, const char *src, size_t n);

#ifdef __KLIBC__
#include <dlfcn.h>

#define dlopen(f, m) os2_dlopen(f, m)
void *os2_dlopen(const char *file, int mode);

#define dlsym(h, n) os2_dlsym(h, n)
void *os2_dlsym(void *handle, const char *name);
#endif /* __KLIBC__ */

#endif /* __EMX__ */
