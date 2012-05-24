/*
 * filefuncs.c - Builtin functions that provide initial minimal iterface
 *		 to the file system.
 *
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 * Arnold Robbins and John Haque, update for 3.1.4, applied Mon Jun 14 13:55:30 IDT 2004
 */

/*
 * Copyright (C) 2001, 2004, 2005, 2010, 2011, 2012
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "config.h"
#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

/*  do_chdir --- provide dynamically loaded chdir() builtin for gawk */

static awk_value_t *
do_chdir(int nargs, awk_value_t *result)
{
	awk_value_t newdir;
	int ret = -1;

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "chdir: called with incorrect number of arguments");

	if (get_curfunc_param(0, AWK_STRING, &newdir) != NULL) {
		ret = chdir(newdir.str_value.str);
		if (ret < 0)
			update_ERRNO_int(errno);
	}

	return make_number(ret, result);
}

/* format_mode --- turn a stat mode field into something readable */

static char *
format_mode(unsigned long fmode)
{
	static char outbuf[12];
	static struct mode_map {
		int mask;
		int rep;
	} map[] = {
		{ S_IRUSR, 'r' }, { S_IWUSR, 'w' }, { S_IXUSR, 'x' },
		{ S_IRGRP, 'r' }, { S_IWGRP, 'w' }, { S_IXGRP, 'x' },
		{ S_IROTH, 'r' }, { S_IWOTH, 'w' }, { S_IXOTH, 'x' },
	};
	int i, j, k;

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

	for (j = 0, k = sizeof(map)/sizeof(map[0]); j < k; j++) {
		i++;
		if ((fmode & map[j].mask) != 0)
			outbuf[i] = map[j].rep;
	}

	i++;
	outbuf[i] = '\0';

	/* setuid bit */
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

	/* the so-called "sticky" bit */
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
   for example, on GNU/Linux in the /proc filesystem, where the symbolic link
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
		bufsize = BUFSIZ * 2;

	/* Make sure that bufsize >= 2 and within range */
	if (bufsize > MAXSIZE || bufsize < 2)
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
array_set(awk_array_t array, const char *sub, awk_value_t *value)
{
	awk_element_t element;
	awk_value_t tmp;

	memset(& element, 0, sizeof(element));

	element.index = dup_string(sub, strlen(sub), & tmp)->str_value;
	element.value = *value;

	set_array_element(array, & element);
}

static void
array_set_numeric(awk_array_t array, const char *sub, double num)
{
	awk_value_t tmp;
	return array_set(array, sub, make_number(num, & tmp));
}

/* do_stat --- provide a stat() function for gawk */

static awk_value_t *
do_stat(int nargs, awk_value_t *result)
{
	awk_value_t file_param, array_param;
	char *name;
	awk_array_t array;
	struct stat sbuf;
	int ret;
	char *pmode;	/* printable mode */
	char *type = "unknown";
	awk_value_t tmp;

	if (do_lint && nargs != 2) {
		lintwarn(ext_id, "stat: called with wrong number of arguments");
		/* XXX previous version returned 0; why? */
		return make_number(-1, result);
	}

	/* file is first arg, array to hold results is second */
	if (get_curfunc_param(0, AWK_STRING, &file_param) == NULL ||
	    get_curfunc_param(1, AWK_ARRAY, &array_param) == NULL) {
		warning(ext_id, "stat: bad parameters");
		/* XXX previous version returned 0; why? */
		return make_number(-1, result);
	}

	name = file_param.str_value.str;
	array = array_param.array_cookie;

	/* empty out the array */
	clear_array(array);

	/* lstat the file, if error, set ERRNO and return */
	ret = lstat(name, & sbuf);
	if (ret < 0) {
		update_ERRNO_int(errno);
		/* XXX previous version returned 0; why? */
		return make_number(-1, result);
	}

	/* fill in the array */
	array_set(array, "name", make_string(name, file_param.str_value.len, &tmp));
	array_set_numeric(array, "dev", sbuf.st_dev);
	array_set_numeric(array, "ino", sbuf.st_ino);
	array_set_numeric(array, "mode", sbuf.st_mode);
	array_set_numeric(array, "nlink", sbuf.st_nlink);
	array_set_numeric(array, "uid", sbuf.st_uid);
	array_set_numeric(array, "gid", sbuf.st_gid);
	array_set_numeric(array, "size", sbuf.st_size);
	array_set_numeric(array, "blocks", sbuf.st_blocks);
	array_set_numeric(array, "atime", sbuf.st_atime);
	array_set_numeric(array, "mtime", sbuf.st_mtime);
	array_set_numeric(array, "ctime", sbuf.st_ctime);

	/* for block and character devices, add rdev, major and minor numbers */
	if (S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode)) {
		array_set_numeric(array, "rdev", sbuf.st_rdev);
		array_set_numeric(array, "major", major(sbuf.st_rdev));
		array_set_numeric(array, "minor", minor(sbuf.st_rdev));
	}

#ifdef HAVE_ST_BLKSIZE
	array_set_numeric(array, "blksize", sbuf.st_blksize);
#endif /* HAVE_ST_BLKSIZE */

	pmode = format_mode(sbuf.st_mode);
	array_set(array, "pmode", make_string(pmode, strlen(pmode), &tmp));

	/* for symbolic links, add a linkval field */
	if (S_ISLNK(sbuf.st_mode)) {
		char *buf;
		ssize_t linksize;

		if ((buf = read_symlink(name, sbuf.st_size,
					&linksize)) != NULL)
			array_set(array, "linkval", make_string(buf, linksize, &tmp));
		else
			warning(ext_id, "unable to read symbolic link `%s'", name);
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

	array_set(array, "type", make_string(type, strlen(type), &tmp));

	ret = 1;	/* success */

	return make_number(ret, result);
}

static awk_ext_func_t func_table[] = {
	{ "chdir", do_chdir, 1 },
	{ "stat", do_stat, 2 },
};


/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, filefuncs, "")
