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

#include <sys/sysmacros.h>

int plugin_is_GPL_compatible;

/*  do_chdir --- provide dynamically loaded chdir() builtin for gawk */

static NODE *
do_chdir(int nargs)
{
	NODE *newdir;
	int ret = -1;

	if (do_lint && get_curfunc_arg_count() != 1)
		lintwarn("chdir: called with incorrect number of arguments");

	newdir = get_scalar_argument(0, FALSE);
	(void) force_string(newdir);
	ret = chdir(newdir->stptr);
	if (ret < 0)
		update_ERRNO();

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

/* do_stat --- provide a stat() function for gawk */

static NODE *
do_stat(int nargs)
{
	NODE *file, *array, *tmp;
	struct stat sbuf;
	int ret;
	NODE **aptr;
	char *pmode;	/* printable mode */
	char *type = "unknown";

	if (do_lint && get_curfunc_arg_count() > 2)
		lintwarn("stat: called with too many arguments");

	/* file is first arg, array to hold results is second */
	file = get_scalar_argument(0, FALSE);
	array = get_array_argument(1, FALSE);

	/* empty out the array */
	assoc_clear(array);

	/* lstat the file, if error, set ERRNO and return */
	(void) force_string(file);
	ret = lstat(file->stptr, & sbuf);
	if (ret < 0) {
		update_ERRNO();
		return make_number((AWKNUM) ret);
	}

	/* fill in the array */
	aptr = assoc_lookup(array, tmp = make_string("name", 4), FALSE);
	*aptr = dupnode(file);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("dev", 3), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_dev);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("ino", 3), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_ino);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("mode", 4), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_mode);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("nlink", 5), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_nlink);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("uid", 3), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_uid);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("gid", 3), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_gid);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("size", 4), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_size);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("blocks", 6), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_blocks);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("atime", 5), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_atime);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("mtime", 5), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_mtime);
	unref(tmp);

	aptr = assoc_lookup(array, tmp = make_string("ctime", 5), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_ctime);
	unref(tmp);

	/* for block and character devices, add rdev, major and minor numbers */
	if (S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode)) {
		aptr = assoc_lookup(array, tmp = make_string("rdev", 4), FALSE);
		*aptr = make_number((AWKNUM) sbuf.st_rdev);
		unref(tmp);

		aptr = assoc_lookup(array, tmp = make_string("major", 5), FALSE);
		*aptr = make_number((AWKNUM) major(sbuf.st_rdev));
		unref(tmp);

		aptr = assoc_lookup(array, tmp = make_string("minor", 5), FALSE);
		*aptr = make_number((AWKNUM) minor(sbuf.st_rdev));
		unref(tmp);
	}

#ifdef HAVE_ST_BLKSIZE
	aptr = assoc_lookup(array, tmp = make_string("blksize", 7), FALSE);
	*aptr = make_number((AWKNUM) sbuf.st_blksize);
	unref(tmp);
#endif /* HAVE_ST_BLKSIZE */

	aptr = assoc_lookup(array, tmp = make_string("pmode", 5), FALSE);
	pmode = format_mode(sbuf.st_mode);
	*aptr = make_string(pmode, strlen(pmode));
	unref(tmp);

	/* for symbolic links, add a linkval field */
	if (S_ISLNK(sbuf.st_mode)) {
		char *buf;
		int linksize;

		emalloc(buf, char *, sbuf.st_size + 2, "do_stat");
		if (((linksize = readlink(file->stptr, buf,
					  sbuf.st_size + 2)) >= 0) &&
		    (linksize <= sbuf.st_size)) {
			/*
			 * set the linkval field only if we are able to
			 * retrieve the entire link value successfully.
			 */
			buf[linksize] = '\0';

			aptr = assoc_lookup(array, tmp = make_string("linkval", 7), FALSE);
			*aptr = make_str_node(buf, linksize, ALREADY_MALLOCED);
			unref(tmp);
		}
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

	aptr = assoc_lookup(array, tmp = make_string("type", 4), FALSE);
	*aptr = make_string(type, strlen(type));
	unref(tmp);

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
