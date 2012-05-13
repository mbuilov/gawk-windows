/*
 * filefuncs.c - Builtin functions that provide initial minimal iterface
 *		 to the file system.
 *
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 * Arnold Robbins and John Haque, update for 3.1.4, applied Mon Jun 14 13:55:30 IDT 2004
 */

/*
 * Copyright (C) 2001, 2004, 2005, 2010, 2011 the Free Software Foundation, Inc.
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

#include "awk.h"

int plugin_is_GPL_compatible;

/*  do_chdir --- provide dynamically loaded chdir() builtin for gawk */

static NODE *
do_chdir(int nargs)
{
	NODE *newdir;
	int ret = -1;

	if (do_lint && nargs != 1)
		lintwarn("chdir: called with incorrect number of arguments");

	newdir = get_scalar_argument(0, false);
	(void) force_string(newdir);
	ret = chdir(newdir->stptr);
	if (ret < 0)
		update_ERRNO_int(errno);

	return make_number((AWKNUM) ret);
}

/* format_mode --- turn a stat mode field into something readable */

static char *
format_mode(unsigned long fmode)
{
	static char outbuf[12];
	int i;

	strcpy(outbuf, "----------");
	/* first, get the file type */
	i = 0;
	switch (fmode & S_IFMT) {
#ifdef S_IFSOCK
	case S_IFSOCK:
		outbuf[i] = 's';
		break;
#endif
#ifdef S_IFLNK
	case S_IFLNK:
		outbuf[i] = 'l';
		break;
#endif
	case S_IFREG:
		outbuf[i] = '-';	/* redundant */
		break;
	case S_IFBLK:
		outbuf[i] = 'b';
		break;
	case S_IFDIR:
		outbuf[i] = 'd';
		break;
#ifdef S_IFDOOR	/* Solaris weirdness */
	case S_IFDOOR:
		outbuf[i] = 'D';
		break;
#endif /* S_IFDOOR */
	case S_IFCHR:
		outbuf[i] = 'c';
		break;
#ifdef S_IFIFO
	case S_IFIFO:
		outbuf[i] = 'p';
		break;
#endif
	}

	i++;
	if ((fmode & S_IRUSR) != 0)
		outbuf[i] = 'r';
	i++;
	if ((fmode & S_IWUSR) != 0)
		outbuf[i] = 'w';
	i++;
	if ((fmode & S_IXUSR) != 0)
		outbuf[i] = 'x';
	i++;

	if ((fmode & S_IRGRP) != 0)
		outbuf[i] = 'r';
	i++;
	if ((fmode & S_IWGRP) != 0)
		outbuf[i] = 'w';
	i++;
	if ((fmode & S_IXGRP) != 0)
		outbuf[i] = 'x';
	i++;

	if ((fmode & S_IROTH) != 0)
		outbuf[i] = 'r';
	i++;
	if ((fmode & S_IWOTH) != 0)
		outbuf[i] = 'w';
	i++;
	if ((fmode & S_IXOTH) != 0)
		outbuf[i] = 'x';
	i++;

	outbuf[i] = '\0';

	if ((fmode & S_ISUID) != 0) {
		if (outbuf[3] == 'x')
			outbuf[3] = 's';
		else
			outbuf[3] = 'S';
	}

	/* setgid without execute == locking */
	if ((fmode & S_ISGID) != 0) {
		if (outbuf[6] == 'x')
			outbuf[6] = 's';
		else
			outbuf[6] = 'l';
	}

	if ((fmode & S_ISVTX) != 0) {
		if (outbuf[9] == 'x')
			outbuf[9] = 't';
		else
			outbuf[9] = 'T';
	}

	return outbuf;
}

/* read_symlink -- read a symbolic link into an allocated buffer.
   This is based on xreadlink; the basic problem is that lstat cannot be relied
   upon to return the proper size for a symbolic link.  This happens,
   for example, on linux in the /proc filesystem, where the symbolic link
   sizes are often 0. */

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif
#ifndef SSIZE_MAX
# define SSIZE_MAX ((ssize_t) (SIZE_MAX / 2))
#endif

#define MAXSIZE (SIZE_MAX < SSIZE_MAX ? SIZE_MAX : SSIZE_MAX)

static char *
read_symlink(const char *fname, size_t bufsize, ssize_t *linksize)
{
	if (bufsize)
		bufsize += 2;
	else
		bufsize = BUFSIZ*2;
	/* Make sure that bufsize >= 2 and within range */
	if ((bufsize > MAXSIZE) || (bufsize < 2))
		bufsize = MAXSIZE;
	while (1) {
		char *buf;

		emalloc(buf, char *, bufsize, "read_symlink");
		if ((*linksize = readlink(fname, buf, bufsize)) < 0) {
			/* On AIX 5L v5.3 and HP-UX 11i v2 04/09, readlink
			   returns -1 with errno == ERANGE if the buffer is
			   too small.  */
			if (errno != ERANGE) {
				free(buf);
				return NULL;
			}
		}
		/* N.B. This test is safe because bufsize must be >= 2 */
		else if ((size_t)*linksize <= bufsize-2) {
			buf[*linksize] = '\0';
			return buf;
		}
		free(buf);
		if (bufsize <= MAXSIZE/2)
			bufsize *= 2;
		else if (bufsize < MAXSIZE)
			bufsize = MAXSIZE;
		else
			return NULL;
	}
	return NULL;
}

/* array_set --- set an array element */

static void
array_set(NODE *array, const char *sub, NODE *value)
{
	NODE *tmp;
	NODE **aptr;

	tmp = make_string(sub, strlen(sub));
	aptr = assoc_lookup(array, tmp);
	unref(tmp);
	/* 
	 * Note: since we initialized with assoc_clear, we know that aptr
	 * has been initialized with Nnull_string.  Thus, the call to
	 * unref(*aptr) is not strictly necessary.  However, I think it is
	 * generally more correct to call unref to maintain the proper
	 * reference count.
	 */
	unref(*aptr);
	*aptr = value;
}

/* do_stat --- provide a stat() function for gawk */

static NODE *
do_stat(int nargs)
{
	NODE *file, *array;
	struct stat sbuf;
	int ret;
	char *pmode;	/* printable mode */
	char *type = "unknown";

	if (do_lint && nargs > 2)
		lintwarn("stat: called with too many arguments");

	/* file is first arg, array to hold results is second */
	file = get_scalar_argument(0, false);
	array = get_array_argument(1, false);

	/* empty out the array */
	assoc_clear(array);

	/* lstat the file, if error, set ERRNO and return */
	(void) force_string(file);
	ret = lstat(file->stptr, & sbuf);
	if (ret < 0) {
		update_ERRNO_int(errno);
		return make_number((AWKNUM) ret);
	}

	/* fill in the array */
	array_set(array, "name", dupnode(file));
	array_set(array, "dev", make_number((AWKNUM) sbuf.st_dev));
	array_set(array, "ino", make_number((AWKNUM) sbuf.st_ino));
	array_set(array, "mode", make_number((AWKNUM) sbuf.st_mode));
	array_set(array, "nlink", make_number((AWKNUM) sbuf.st_nlink));
	array_set(array, "uid", make_number((AWKNUM) sbuf.st_uid));
	array_set(array, "gid", make_number((AWKNUM) sbuf.st_gid));
	array_set(array, "size", make_number((AWKNUM) sbuf.st_size));
	array_set(array, "blocks", make_number((AWKNUM) sbuf.st_blocks));
	array_set(array, "atime", make_number((AWKNUM) sbuf.st_atime));
	array_set(array, "mtime", make_number((AWKNUM) sbuf.st_mtime));
	array_set(array, "ctime", make_number((AWKNUM) sbuf.st_ctime));

	/* for block and character devices, add rdev, major and minor numbers */
	if (S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode)) {
		array_set(array, "rdev", make_number((AWKNUM) sbuf.st_rdev));
		array_set(array, "major", make_number((AWKNUM) major(sbuf.st_rdev)));
		array_set(array, "minor", make_number((AWKNUM) minor(sbuf.st_rdev)));
	}

#ifdef HAVE_ST_BLKSIZE
	array_set(array, "blksize", make_number((AWKNUM) sbuf.st_blksize));
#endif /* HAVE_ST_BLKSIZE */

	pmode = format_mode(sbuf.st_mode);
	array_set(array, "pmode", make_string(pmode, strlen(pmode)));

	/* for symbolic links, add a linkval field */
	if (S_ISLNK(sbuf.st_mode)) {
		char *buf;
		ssize_t linksize;

		if ((buf = read_symlink(file->stptr, sbuf.st_size,
					&linksize)) != NULL)
			array_set(array, "linkval", make_str_node(buf, linksize, ALREADY_MALLOCED));
		else
			warning(_("unable to read symbolic link `%s'"),
				file->stptr);
	}

	/* add a type field */
	switch (sbuf.st_mode & S_IFMT) {
#ifdef S_IFSOCK
	case S_IFSOCK:
		type = "socket";
		break;
#endif
#ifdef S_IFLNK
	case S_IFLNK:
		type = "symlink";
		break;
#endif
	case S_IFREG:
		type = "file";
		break;
	case S_IFBLK:
		type = "blockdev";
		break;
	case S_IFDIR:
		type = "directory";
		break;
#ifdef S_IFDOOR
	case S_IFDOOR:
		type = "door";
		break;
#endif
	case S_IFCHR:
		type = "chardev";
		break;
#ifdef S_IFIFO
	case S_IFIFO:
		type = "fifo";
		break;
#endif
	}

	array_set(array, "type", make_string(type, strlen(type)));

	return make_number((AWKNUM) ret);
}

/* dlload --- load new builtins in this library */

NODE *
dlload(NODE *tree, void *dl)
{
	make_builtin("chdir", do_chdir, 1);
	make_builtin("stat", do_stat, 2);

	return make_number((AWKNUM) 0);
}
