
/* os_close_on_exec --- set close on exec flag, print warning if fails */

void
os_close_on_exec(fd, name, what, dir)
int fd;
char *name, *what, *dir;
{
	/* no-op */
}

/* os_isdir --- is this an fd on a directory? */

#if ! defined(S_ISDIR) && defined(S_IFDIR)
#define	S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

int
os_isdir(fd)
int fd;
{
	struct stat sbuf;

	return (fstat(fd, &sbuf) == 0 && S_ISDIR(sbuf.st_mode));
}
/*
 * gawkmisc.c --- miscellanious gawk routines that are OS specific.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991 - 95 the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

char quote = '"';
char envsep  = ';';
char *defpath = "";

/* gawk_name --- pull out the "gawk" part from how the OS called us */

char *
gawk_name(filespec)
const char *filespec;
{
        char *p, *q;

        p = (char *) filespec;  /* Sloppy... */

        if ((q = strrchr(p, '.')) != NULL)
                p = q + 1;
        return p;
}

/* os_arg_fixup --- fixup the command line */

void
os_arg_fixup(argcp, argvp)
int *argcp;
char ***argvp;
{
        return;
}

/* os_devopen --- open special per-OS devices */

int
os_devopen(name, flag)
const char *name;
int flag;
{
        /* no-op */
        return -1;
}

/* optimal_bufsize --- determine optimal buffer size */

size_t
optimal_bufsize(fd, stb)
int fd;
struct stat *stb;
{
        /*
         * TANDEM  doesn't have a stat function.
         * So we just return 4096 which is the Tandem disk block size.
         */

        /* set all members to zero. */

        memset(stb, '\0', sizeof(struct stat));

        /* set file size to arbitrary non-zero value. */
        stb->st_size = 1;

        return 4096;
}

/* ispath --- return true if path has directory components */

int
ispath(file)
const char *file;
{
        for (; *file; file++) {
                switch (*file) {
                case '.':
                        return 1;
                }
        }
        return 0;
}

/* isdirpunct --- return true if char is a directory separator */

int
isdirpunct(c)
int c;
{
        return (strchr(".\\", c) != NULL);
}

void
initstate(i, j, k)
unsigned i;
char * j;
int k;
{
}

void setstate(i)
char * i;
{
}

/* os_close_on_exec --- set close on exec flag, print warning if fails */

void
os_close_on_exec(fd, name, what, dir)
int fd;
char *name, *what, *dir;
{
	/* no-op */
}

/* os_isdir --- is this an fd on a directory? */

/* can't do this on tandem, so just assume it's not a directory */

int
os_isdir(fd)
int fd;
{
	return 0;
}

/* os_is_setuid --- true if running setuid root */

int
os_is_setuid()
{
	return 0;
}

/* os_setbinmode --- set binary mode on file */

int
os_setbinmode (fd, mode)
int fd, mode;
{
	return 0;
}

/* os_restore_mode --- restore the original mode of the console device */

void
os_restore_mode (fd)
int fd;
{
	/* no-op */
	return;
}
