/*
 * filefuncs.c - Builtin functions that provide initial minimal iterface
 *		 to the file system.
 *
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 * Arnold Robbins and John Haque, update for 3.1.4, applied Mon Jun 14 13:55:30 IDT 2004
 * Arnold Robbins and Andrew Schorr, revised for new extension API, May 2012.
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
#include <assert.h>
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
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

/*  do_chdir --- provide dynamically loaded chdir() builtin for gawk */

static awk_value_t *
do_chdir(int nargs, awk_value_t *result)
{
	awk_value_t newdir;
	int ret = -1;

	assert(result != NULL);

	if (do_lint && nargs != 1)
		lintwarn(ext_id, "chdir: called with incorrect number of arguments, expecting 1");

	if (get_argument(0, AWK_STRING, & newdir)) {
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
	static struct ftype_map {
		unsigned int mask;
		int charval;
	} ftype_map[] = {
		{ S_IFREG, '-' },	/* redundant */
		{ S_IFBLK, 'b' },
		{ S_IFCHR, 'c' },
		{ S_IFDIR, 'd' },
#ifdef S_IFSOCK
		{ S_IFSOCK, 's' },
#endif
#ifdef S_IFIFO
		{ S_IFIFO, 'p' },
#endif
#ifdef S_IFLNK
		{ S_IFLNK, 'l' },
#endif
#ifdef S_IFDOOR	/* Solaris weirdness */
		{ S_IFDOOR, 'D' },
#endif /* S_IFDOOR */
	};
	static struct mode_map {
		unsigned int mask;
		int rep;
	} map[] = {
		{ S_IRUSR, 'r' }, { S_IWUSR, 'w' }, { S_IXUSR, 'x' },
		{ S_IRGRP, 'r' }, { S_IWGRP, 'w' }, { S_IXGRP, 'x' },
		{ S_IROTH, 'r' }, { S_IWOTH, 'w' }, { S_IXOTH, 'x' },
	};
	static struct setuid_map {
		unsigned int mask;
		int index;
		int small_rep;
		int big_rep;
	} setuid_map[] = {
		{ S_ISUID, 3, 's', 'S' }, /* setuid bit */
		{ S_ISGID, 6, 's', 'l' }, /* setgid without execute == locking */
		{ S_ISVTX, 9, 't', 'T' }, /* the so-called "sticky" bit */
	};
	int i, j, k;

	strcpy(outbuf, "----------");

	/* first, get the file type */
	i = 0;
	for (j = 0, k = sizeof(ftype_map)/sizeof(ftype_map[0]); j < k; j++) {
		if ((fmode & S_IFMT) == ftype_map[j].mask) {
			outbuf[i] = ftype_map[j].charval;
			break;
		}
	}

	/* now the permissions */
	for (j = 0, k = sizeof(map)/sizeof(map[0]); j < k; j++) {
		i++;
		if ((fmode & map[j].mask) != 0)
			outbuf[i] = map[j].rep;
	}

	i++;
	outbuf[i] = '\0';

	/* tweaks for the setuid / setgid / sticky bits */
	for (j = 0, k = sizeof(setuid_map)/sizeof(setuid_map[0]); j < k; j++) {
		if (fmode & setuid_map[j].mask) {
			if (outbuf[setuid_map[j].index] == 'x')
				outbuf[setuid_map[j].index] = setuid_map[j].small_rep;
			else
				outbuf[setuid_map[j].index] = setuid_map[j].big_rep;
		}
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
	awk_value_t index;

	set_array_element(array,
			make_const_string(sub, strlen(sub), & index),
			value);

}

/* array_set_numeric --- set an array element with a number */

static void
array_set_numeric(awk_array_t array, const char *sub, double num)
{
	awk_value_t tmp;

	array_set(array, sub, make_number(num, & tmp));
}

/* fill_stat_array --- do the work to fill an array with stat info */

static int
fill_stat_array(const char *name, awk_array_t array)
{
	char *pmode;	/* printable mode */
	const char *type = "unknown";
	struct stat sbuf;
	awk_value_t tmp;
	static struct ftype_map {
		unsigned int mask;
		const char *type;
	} ftype_map[] = {
		{ S_IFREG, "file" },
		{ S_IFBLK, "blockdev" },
		{ S_IFCHR, "chardev" },
		{ S_IFDIR, "directory" },
#ifdef S_IFSOCK
		{ S_IFSOCK, "socket" },
#endif
#ifdef S_IFIFO
		{ S_IFIFO, "fifo" },
#endif
#ifdef S_IFLNK
		{ S_IFLNK, "symlink" },
#endif
#ifdef S_IFDOOR	/* Solaris weirdness */
		{ S_IFDOOR, "door" },
#endif /* S_IFDOOR */
	};
	int ret, j, k;

	/* empty out the array */
	clear_array(array);

	/* lstat the file, if error, set ERRNO and return */
	ret = lstat(name, & sbuf);
	if (ret < 0) {
		update_ERRNO_int(errno);
		return -1;
	}

	/* fill in the array */
	array_set(array, "name", make_const_string(name, strlen(name), & tmp));
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
	array_set(array, "pmode", make_const_string(pmode, strlen(pmode), & tmp));

	/* for symbolic links, add a linkval field */
	if (S_ISLNK(sbuf.st_mode)) {
		char *buf;
		ssize_t linksize;

		if ((buf = read_symlink(name, sbuf.st_size,
					& linksize)) != NULL)
			array_set(array, "linkval", make_malloced_string(buf, linksize, & tmp));
		else
			warning(ext_id, "stat: unable to read symbolic link `%s'", name);
	}

	/* add a type field */
	type = "unknown";	/* shouldn't happen */
	for (j = 0, k = sizeof(ftype_map)/sizeof(ftype_map[0]); j < k; j++) {
		if ((sbuf.st_mode & S_IFMT) == ftype_map[j].mask) {
			type = ftype_map[j].type;
			break;
		}
	}

	array_set(array, "type", make_const_string(type, strlen(type), &tmp));

	return 0;
}

/* do_stat --- provide a stat() function for gawk */

static awk_value_t *
do_stat(int nargs, awk_value_t *result)
{
	awk_value_t file_param, array_param;
	char *name;
	awk_array_t array;
	int ret;

	assert(result != NULL);

	if (do_lint && nargs != 2) {
		lintwarn(ext_id, "stat: called with wrong number of arguments");
		return make_number(-1, result);
	}

	/* file is first arg, array to hold results is second */
	if (   ! get_argument(0, AWK_STRING, & file_param)
	    || ! get_argument(1, AWK_ARRAY, & array_param)) {
		warning(ext_id, "stat: bad parameters");
		return make_number(-1, result);
	}

	name = file_param.str_value.str;
	array = array_param.array_cookie;

	ret = fill_stat_array(name, array);

	return make_number(ret, result);
}

static awk_ext_func_t func_table[] = {
	{ "chdir", do_chdir, 1 },
	{ "stat", do_stat, 2 },
};


/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, filefuncs, "")
