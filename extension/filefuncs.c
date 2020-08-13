/*
 * filefuncs.c - Builtin functions that provide initial minimal iterface
 *		 to the file system.
 *
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 * Arnold Robbins and John Haque, update for 3.1.4, applied Mon Jun 14 13:55:30 IDT 2004
 * Arnold Robbins and Andrew Schorr, revised for new extension API, May 2012.
 * Arnold Robbins, add fts(), August 2012
 * Arnold Robbins, add statvfs(), November 2015
 */

/*
 * Michael M. Builov
 * mbuilov@gmail.com
 * Ported to _MSC_VER/__MINGW32__ 4-5/2020
 */

/*
 * Copyright (C) 2001, 2004, 2005, 2010-2020,
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _BSD_SOURCE

#ifdef __VMS
#if (__CRTL_VER >= 70200000) && !defined (__VAX)
#define _LARGEFILE 1
#endif

#ifndef __VAX
#ifdef __CRTL_VER
#if __CRTL_VER >= 80200000
#define _USE_STD_STAT 1
#endif
#endif
#endif
#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE 1
#include <stat.h>
#ifndef S_ISVTX
#define S_ISVTX (0)
#endif
#ifndef major
#define major(s) (s)
#endif
#ifndef minor
#define minor(s) (0)
#endif
#include <unixlib.h>
#endif


#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <wchar.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#elif defined HAVE_SYS_MKDEV_H
#include <sys/mkdev.h>
#endif /* HAVE_SYS_MKDEV_H */

#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
#include <sys/statvfs.h>
#endif

#ifdef WINDOWS_NATIVE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mscrtx/xstat.h"
#include "mscrtx/wreadlink.h"
#endif

/* Include <locale.h> before "gawkapi.h" redefines setlocale().
  "gettext.h" will include <locale.h> anyway */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#include "gawkfts.h"
#include "stack.h"

#ifdef WINDOWS_NATIVE
#define BAD_FILE_HANDLE		INVALID_HANDLE_VALUE
#else
#define BAD_FILE_HANDLE		NULL
#endif

#ifndef WINDOWS_NATIVE
#define xstat(path, buf)	stat(path, buf)
#define xlstat(path, buf)	lstat(path, buf)
#endif

#ifndef S_IFLNK
#undef xlstat
#define xlstat xstat
#define S_ISLNK(s) 0
#define readlink(f,b,bs) (-1)
#endif

#ifdef WINDOWS_NATIVE
# ifndef S_IRUSR
#  define S_IRUSR _S_IREAD
# endif
# ifndef S_IWUSR
#  define S_IWUSR _S_IWRITE
# endif
# ifndef S_IXUSR
#  define S_IXUSR _S_IEXEC
# endif
# ifndef S_IFIFO
#  define S_IFIFO _S_IFIFO
# endif
# ifndef S_ISCHR
#  define S_ISCHR(m) (((m)&_S_IFMT) == _S_IFCHR)
# endif
# define st_rdev st_dev
#endif

#ifdef WINDOWS_NATIVE
# ifndef S_IRGRP
#  define S_IRGRP S_IRUSR
# endif
# ifndef S_IWGRP
#  define S_IWGRP S_IWUSR
# endif
# ifndef S_IXGRP
#  define S_IXGRP S_IXUSR
# endif
# ifndef S_IROTH
#  define S_IROTH S_IRUSR
# endif
# ifndef S_IWOTH
#  define S_IWOTH S_IWUSR
# endif
# ifndef S_IXOTH
#  define S_IXOTH S_IXUSR
# endif
# ifndef S_ISUID
#  define S_ISUID 0
# endif
# ifndef S_ISGID
#  define S_ISGID 0
# endif
# ifndef S_ISVTX
#  define S_ISVTX 0
# endif
# define major(s) (s)
# define minor(s) (0)
#endif

#ifdef WINDOWS_NATIVE
# ifndef S_IFREG
#  define S_IFREG _S_IFREG
# endif
# ifndef S_IFCHR
#  define S_IFCHR _S_IFCHR
# endif
# ifndef S_IFDIR
#  define S_IFDIR _S_IFDIR
# endif
# ifndef S_IFMT
#  define S_IFMT _S_IFMT
# endif
#endif

GAWK_PLUGIN_GPL_COMPATIBLE
GAWK_PLUGIN("filefuncs extension: version 1.0");

/*  do_chdir --- provide dynamically loaded chdir() function for gawk */

static awk_value_t *
do_chdir(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t newdir;
	int ret = -1;

	(void) nargs, (void) unused;

	assert(result != NULL);

	if (get_argument(0, AWK_STRING, & newdir)) {
		ret = chdir(newdir.str_value.str);
		if (ret < 0)
			update_ERRNO_int(errno);
	}

	return make_number(ret, result);
}

/* format_mode --- turn a stat mode field into something readable */

static void
format_mode(unsigned long fmode, char outbuf[12])
{
	static const struct ftype_map {
		unsigned int mask;
		int charval;
	} ftype_map[] = {
		{ S_IFREG, '-' },	/* redundant */
#ifdef S_IFBLK
		{ S_IFBLK, 'b' },
#endif
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
	static const struct mode_map {
		unsigned int mask;
		int rep;
	} map[] = {
		{ S_IRUSR, 'r' }, { S_IWUSR, 'w' }, { S_IXUSR, 'x' },
		{ S_IRGRP, 'r' }, { S_IWGRP, 'w' }, { S_IXGRP, 'x' },
		{ S_IROTH, 'r' }, { S_IWOTH, 'w' }, { S_IXOTH, 'x' },
	};
	static const struct setuid_map {
		unsigned int mask;
		int index;
		int small_rep;
		int big_rep;
	} setuid_map[] = {
		{ S_ISUID, 3, 's', 'S' }, /* setuid bit */
		{ S_ISGID, 6, 's', 'l' }, /* setgid without execute == locking */
		{ S_ISVTX, 9, 't', 'T' }, /* the so-called "sticky" bit */
	};
	unsigned i, j, k;

	strcpy(outbuf, "----------");

	/* first, get the file type */
	i = 0;
	for (j = 0, k = sizeof(ftype_map)/sizeof(ftype_map[0]); j < k; j++) {
		if ((fmode & S_IFMT) == ftype_map[j].mask) {
			outbuf[i] = (char) ftype_map[j].charval;
			break;
		}
	}

	/* now the permissions */
	for (j = 0, k = sizeof(map)/sizeof(map[0]); j < k; j++) {
		i++;
		if (fmode & map[j].mask)
			outbuf[i] = (char) map[j].rep;
	}

	i++;
	outbuf[i] = '\0';

	/* tweaks for the setuid / setgid / sticky bits */
	for (j = 0, k = sizeof(setuid_map)/sizeof(setuid_map[0]); j < k; j++) {
		if (fmode & setuid_map[j].mask) {
			if (outbuf[setuid_map[j].index] == 'x')
				outbuf[setuid_map[j].index] = (char) setuid_map[j].small_rep;
			else
				outbuf[setuid_map[j].index] = (char) setuid_map[j].big_rep;
		}
	}
}

/* read_symlink --- read a symbolic link into an allocated buffer.
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
read_symlink(void *h, const char *fname, size_t bufsize, ssize_t *linksize)
{
#ifdef WINDOWS_NATIVE
	(void) fname;
# define READLINK(h, fname, buf, bufsize) \
	h != INVALID_HANDLE_VALUE \
	? readlinkfd(h, buf, bufsize) \
	: readlink(fname, buf, bufsize)
#else
	(void) h;
# define READLINK(h, fname, buf, bufsize) readlink(fname, buf, bufsize)
#endif

	if (bufsize)
		bufsize += 2;
	else
		bufsize = BUFSIZ * 2;

	/* Make sure that bufsize >= 2 and within range */
	if (bufsize > MAXSIZE || bufsize < 2)
		bufsize = MAXSIZE;

	for (;;) {
		char *buf = (char*) malloc(bufsize);
		if (buf == NULL)
			return NULL;

		if ((*linksize = READLINK(h, fname, buf, bufsize)) < 0) {
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
			break;
	}
#undef READLINK
	return NULL;
}


/* device_blocksize --- try to figure out units of st_blocks */

static unsigned int
device_blocksize(void)
{
	/* some of this derived from GNULIB stat-size.h */
#if defined(DEV_BSIZE)
	/* <sys/param.h>, most systems */
	return DEV_BSIZE;
#elif defined(S_BLKSIZE)
	/* <sys/stat.h>, BSD systems */
	return S_BLKSIZE;
#elif defined hpux || defined __hpux__ || defined __hpux
	return 1024;
#elif defined _AIX && defined _I386
	/* AIX PS/2 counts st_blocks in 4K units.  */
	return 4 * 1024;
#elif defined WINDOWS_NATIVE
	/*
	 * DOS doesn't have the file system block size in the
	 * stat structure. So we have to make some sort of reasonable
	 * guess. We use stdio's BUFSIZ, since that is what it was
	 * meant for in the first place.
	 */
	return BUFSIZ;
#else
	return 512;
#endif
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
fill_stat_array(void *h, const char *name, awk_array_t array, const fts_stat_t *sbuf)
{
	char pmode[12];	/* printable mode */
	const char *type = "unknown";
	unsigned int type_len = 7;
	awk_value_t tmp;
	static const struct ftype_map {
		unsigned int mask;
		unsigned int len;
		const char *type;
	} ftype_map[] = {
		{ S_IFREG, 4, "file" },
#ifdef S_IFBLK
		{ S_IFBLK, 8, "blockdev" },
#endif
		{ S_IFCHR, 7, "chardev" },
		{ S_IFDIR, 9, "directory" },
#ifdef S_IFSOCK
		{ S_IFSOCK, 6, "socket" },
#endif
#ifdef S_IFIFO
		{ S_IFIFO, 4, "fifo" },
#endif
#ifdef S_IFLNK
		{ S_IFLNK, 7, "symlink" },
#endif
#ifdef S_IFDOOR	/* Solaris weirdness */
		{ S_IFDOOR, 4, "door" },
#endif /* S_IFDOOR */
	};
	unsigned j, k;

	/* empty out the array */
	clear_array(array);

	/* fill in the array */
	array_set(array, "name", make_const_string(name, strlen(name), & tmp));
	array_set_numeric(array, "dev", sbuf->st_dev);
	array_set_numeric(array, "ino", (double)sbuf->st_ino);
	array_set_numeric(array, "mode", sbuf->st_mode);
	array_set_numeric(array, "nlink", sbuf->st_nlink);
#ifdef WINDOWS_NATIVE
	array_set_numeric(array, "uid", 0);
	array_set_numeric(array, "gid", 0);
#else
	array_set_numeric(array, "uid", sbuf->st_uid);
	array_set_numeric(array, "gid", sbuf->st_gid);
#endif
	array_set_numeric(array, "size", (double)sbuf->st_size);
#ifdef WINDOWS_NATIVE
	array_set_numeric(array, "blocks", (double)((sbuf->st_size +
		device_blocksize() - 1) / device_blocksize()));
#else
	array_set_numeric(array, "blocks", sbuf->st_blocks);
#endif
	array_set_numeric(array, "atime", (double)sbuf->st_atime);
	array_set_numeric(array, "mtime", (double)sbuf->st_mtime);
	array_set_numeric(array, "ctime", (double)sbuf->st_ctime);

	/* for block and character devices, add rdev, major and minor numbers */
	if (
#ifndef WINDOWS_NATIVE
		S_ISBLK(sbuf->st_mode) ||
#endif
		S_ISCHR(sbuf->st_mode))
	{
		array_set_numeric(array, "rdev", sbuf->st_rdev);
		array_set_numeric(array, "major", major(sbuf->st_rdev));
		array_set_numeric(array, "minor", minor(sbuf->st_rdev));
	}

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	array_set_numeric(array, "blksize", sbuf->st_blksize);
#elif defined WINDOWS_NATIVE
	array_set_numeric(array, "blksize", 4096);
#endif /* HAVE_STRUCT_STAT_ST_BLKSIZE */

	/* the size of a block for st_blocks */
	array_set_numeric(array, "devbsize", device_blocksize());

	format_mode(sbuf->st_mode, pmode);
	array_set(array, "pmode", make_const_string(pmode, strlen(pmode), & tmp));

	/* for symbolic links, add a linkval field */
	if (S_ISLNK(sbuf->st_mode)) {
		char *buf;
		ssize_t linksize;

		if ((buf = read_symlink(h, name, (size_t) sbuf->st_size, & linksize)) != NULL)
			array_set(array, "linkval",
				make_malloced_string(buf, (size_t) linksize, & tmp));
		else
			warning(_("stat: unable to read symbolic link `%s'"), name);
	}

	/* add a type field */
	for (j = 0, k = sizeof(ftype_map)/sizeof(ftype_map[0]); j < k; j++) {
		if ((unsigned) (sbuf->st_mode & S_IFMT) == ftype_map[j].mask) {
			type = ftype_map[j].type;
			type_len = ftype_map[j].len;
			break;
		}
	}

	array_set(array, "type", make_const_string(type, type_len, & tmp));

	return 0;
}

/* stat_and_fill_array --- stat a file and fill an array with stat info */

static int
stat_and_fill_array(const char *name, awk_array_t array, int do_lstat)
{
	void *h = NULL;
	int ret = -1;
	fts_stat_t sbuf;

#ifdef WINDOWS_NATIVE
	wchar_t buf[MAX_PATH];
	wchar_t *wp = xpathwc(name, buf, sizeof(buf)/sizeof(buf[0]));

	if (wp != NULL) {
		h = xstat_open(wp, do_lstat);
		if (h == INVALID_HANDLE_VALUE) {
			const DWORD open_err = GetLastError();
			/* name could be a root drive directory path, like "C:\",
			   if the drive is not ready - the open fails, but we can
			   still get some stat info (a drive number) */
			ret = xstat_root(wp, & sbuf);
			if (ret && errno == ENOENT) {
				if (ERROR_ACCESS_DENIED == open_err)
					errno = EACCES;
			}
		}
		else
			ret = xfstat(h, wp, & sbuf);

		if (wp != buf)
			free(wp);
	}
	else
		h = INVALID_HANDLE_VALUE;
#else
	ret = do_lstat ? lstat(name, & sbuf) : stat(name, & sbuf);
#endif

	if (ret == 0)
		ret = fill_stat_array(h, name, array, & sbuf);

#ifdef WINDOWS_NATIVE
	if (h != INVALID_HANDLE_VALUE)
		CloseHandle(h);
#endif
	return ret;
}

/* do_stat --- provide a stat() function for gawk */

static awk_value_t *
do_stat(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t file_param, array_param;
	const char *name;
	awk_array_t array;
	int ret, do_lstat = 1;	/* default */

	(void) unused;

	assert(result != NULL);

	/* file is first arg, array to hold results is second */
	if (! get_argument(0, AWK_STRING, & file_param)) {
		warning(_("stat: first argument is not a string"));
		return make_number(-1, result);
	}

	if (! get_argument(1, AWK_ARRAY, & array_param)) {
		warning(_("stat: second argument is not an array"));
		return make_number(-1, result);
	}

	if (nargs == 3)
		do_lstat = 0;

	name = file_param.str_value.str;
	array = array_param.array_cookie;

	/* always empty out the array */
	clear_array(array);

	/* stat the file; if error, set ERRNO and return */
	ret = stat_and_fill_array(name, array, do_lstat);
	if (ret < 0) {
		update_ERRNO_int(errno);
		return make_number(ret, result);
	}

	return make_number(ret, result);
}

#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)

/* do_statvfs --- provide a statvfs() function for gawk */

static awk_value_t *
do_statvfs(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t file_param, array_param;
	char *name;
	awk_array_t array;
	int ret;
	struct statvfs vfsbuf;

	assert(result != NULL);

	/* file is first arg, array to hold results is second */
	if (   ! get_argument(0, AWK_STRING, & file_param)
	    || ! get_argument(1, AWK_ARRAY, & array_param)) {
		warning(_("stat: bad parameters"));
		return make_number(-1, result);
	}

	name = file_param.str_value.str;
	array = array_param.array_cookie;

	/* always empty out the array */
	clear_array(array);

	/* statvfs the filesystem; if error, set ERRNO and return */
	ret = statvfs(name, & vfsbuf);
	if (ret < 0) {
		update_ERRNO_int(errno);
		return make_number(ret, result);
	}

	array_set_numeric(array, "bsize", vfsbuf.f_bsize);	/* filesystem block size */
	array_set_numeric(array, "frsize", vfsbuf.f_frsize);	/* fragment size */
	array_set_numeric(array, "blocks", vfsbuf.f_blocks);	/* size of fs in f_frsize units */
	array_set_numeric(array, "bfree", vfsbuf.f_bfree);	/* # free blocks */
	array_set_numeric(array, "bavail", vfsbuf.f_bavail);	/* # free blocks for unprivileged users */
	array_set_numeric(array, "files", vfsbuf.f_files);	/* # inodes */
	array_set_numeric(array, "ffree", vfsbuf.f_ffree);	/* # free inodes */
	array_set_numeric(array, "favail", vfsbuf.f_favail);	/* # free inodes for unprivileged users */
#ifndef _AIX
	array_set_numeric(array, "fsid", vfsbuf.f_fsid);	/* filesystem ID */
#endif
	array_set_numeric(array, "flag", vfsbuf.f_flag);	/* mount flags */
	array_set_numeric(array, "namemax", vfsbuf.f_namemax);	/* maximum filename length */


	return make_number(ret, result);
}
#endif

/* init_filefuncs --- initialization routine */

static awk_bool_t
init_filefuncs(void)
{
	int errors = 0;
	unsigned int i;
	awk_value_t value;

	/* at least right now, only FTS needs initializing */
#define FTS_NON_RECURSIVE	FTS_STOP	/* Don't step into directories.  */
	static const struct flagtab {
		const char *name;
		int value;
	} opentab[] = {
#define ENTRY(x)	{ #x, x }
		ENTRY(FTS_COMFOLLOW),
		ENTRY(FTS_LOGICAL),
		ENTRY(FTS_NOCHDIR),
		ENTRY(FTS_NOSTAT),
		ENTRY(FTS_PHYSICAL),
		ENTRY(FTS_SEEDOT),
		ENTRY(FTS_XDEV),
		{"FTS_SKIP", FTS_NON_RECURSIVE},
		{ NULL, 0 }
	};

	for (i = 0; opentab[i].name != NULL; i++) {
		(void) make_number(opentab[i].value, & value);
		if (! sym_update(opentab[i].name, & value)) {
			warning(_("fts init: could not create variable %s"),
					opentab[i].name);
			errors++;
		}
	}

	return (awk_bool_t) (errors == 0);
}

static int fts_errors = 0;

/* fill_stat_element --- fill in stat element of array */

static void
fill_stat_element(awk_array_t element_array, const char *name, const fts_stat_t *sbuf)
{
	awk_value_t index, value;
	awk_array_t stat_array;

	stat_array = create_array();
	if (stat_array == NULL) {
		warning(_("fill_stat_element: could not create array, out of memory"));
		fts_errors++;
		return;
	}
	fill_stat_array(BAD_FILE_HANDLE, name, stat_array, sbuf);
	(void) make_const_string("stat", 4, & index);
	value.val_type = AWK_ARRAY;
	value.array_cookie = stat_array;
	if (! set_array_element(element_array, & index, & value)) {
		warning(_("fill_stat_element: could not set element"));
		fts_errors++;
	}
}

/* fill_path_element --- fill in path element of array */

static void
fill_path_element(awk_array_t element_array, const char *path)
{
	awk_value_t index, value;

	(void) make_const_string("path", 4, & index);
	(void) make_const_string(path, strlen(path), & value);
	if (! set_array_element(element_array, & index, & value)) {
		warning(_("fill_path_element: could not set element"));
		fts_errors++;
	}
}

/* fill_error_element --- fill in error element of array */

static void
fill_error_element(awk_array_t element_array, const int errcode)
{
	awk_value_t index, value;
	const char *err = strerror(errcode);

	(void) make_const_string("error", 5, & index);
	(void) make_const_string(err, strlen(err), & value);
	if (! set_array_element(element_array, & index, & value)) {
		warning(_("fill_error_element: could not set element"));
		fts_errors++;
	}
}

/* fill_default_elements --- fill in stat and path elements */

static void
fill_default_elements(awk_array_t element_array, const FTSENT *const fentry, awk_bool_t bad_ret)
{
	/* full path */
	fill_path_element(element_array, fentry->fts_path);

	/* stat info */
	if (! bad_ret && fentry->fts_statp != NULL) {
		fill_stat_element(element_array,
				fentry->fts_name,
				fentry->fts_statp);
	}

	/* error info */
	if (bad_ret || fentry->fts_errno != 0) {
		fill_error_element(element_array, fentry->fts_errno);
	}
}

/* process --- process the hierarchy */

static void
process(FTS *hierarchy, awk_array_t destarray, int seedot, int nonrecursive)
{
	FTSENT *fentry;
	awk_value_t index, value;
	awk_array_t element_array, newdir_array, dot_array;
	awk_bool_t bad_ret = awk_false;

	/* path is full path,  pathlen is length thereof */
	/* name is name in directory, namelen is length thereof */
	while ((fentry = fts_read(hierarchy)) != NULL) {
		bad_ret = awk_false;

		switch (fentry->fts_info) {
		case FTS_D:
			/* directory */

			if (nonrecursive && fentry->fts_level == 0)
				fts_set(hierarchy, fentry, FTS_SKIP);

			/* create array to hold entries */
			/* this will be empty if doing FTS_SKIP */
			newdir_array = create_array();
			if (newdir_array == NULL) {
				warning(_("fts-process: could not create array"));
				fts_errors++;
				break;
			}

			/* store new directory in its parent directory */
			(void) make_const_string(fentry->fts_name, fentry->fts_namelen, & index);
			value.val_type = AWK_ARRAY;
			value.array_cookie = newdir_array;
			if (! set_array_element(destarray, & index, & value)) {
				warning(_("fts-process: could not set element"));
				fts_errors++;
				break;
			}
			newdir_array = value.array_cookie;

			/* push current directory */
			if (! stack_push(destarray))
				fatal(_("fts-process: could not push an entry"));

			/* new directory becomes current */
			destarray = newdir_array;
			break;

		case FTS_DNR:
		case FTS_DC:
		case FTS_ERR:
		case FTS_NS:
			/* error */
			bad_ret = awk_true;
			/* fall through */

		case FTS_DOT:
			/* if see dot, skip "." */
			if (seedot && strcmp(fentry->fts_name, ".") == 0)
				break;
			/* fall through */

		case FTS_NSOK:
		case FTS_SL:
		case FTS_SLNONE:
		case FTS_F:
			/*
			 * File case.
			 * destarray is the directory we're reading.
			 * step 1: create new empty array
			 */
			element_array = create_array();
			if (element_array == NULL) {
				warning(_("fts-process: could not create array"));
				fts_errors++;
				break;
			}

			/* step 2: add element array to parent array */
			(void) make_const_string(fentry->fts_name, fentry->fts_namelen, & index);
			value.val_type = AWK_ARRAY;
			value.array_cookie = element_array;
			if (! set_array_element(destarray, & index, & value)) {
				warning(_("fts-process: could not set element"));
				fts_errors++;
				break;
			}

			/* step 3: fill in path, stat, error elements */
			fill_default_elements(element_array, fentry, bad_ret);
			break;

		case FTS_DP:
			/* create "." subarray */
			dot_array = create_array();

			/* add it to parent */
			(void) make_const_string(".", 1, & index);
			value.val_type = AWK_ARRAY;
			value.array_cookie = dot_array;
			if (! set_array_element(destarray, & index, & value)) {
				warning(_("fts-process: could not set element"));
				fts_errors++;
				break;
			}

			/* fill it in with path, stat, error elements */
			fill_default_elements(dot_array, fentry, bad_ret);

			/* now pop the parent directory off the stack */
			if (! stack_empty()) {
				/* pop stack */
				destarray = stack_pop();
			}

			break;

		case FTS_DEFAULT:
			/* nothing to do */
			break;
		}
	}
}

/*  do_fts --- walk a hierarchy and fill in an array */

/*
 * Usage from awk:
 *	flags = or(FTS_PHYSICAL, ...)
 *	result = fts(pathlist, flags, filedata)
 */

static awk_value_t *
do_fts(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t pathlist, flagval, dest;
	awk_flat_array_t *path_array = NULL;
	char **pathvector = NULL;
	FTS *hierarchy;
	int flags;
	size_t i, count;
	awk_bool_t b;
	int ret = -1;
	static const int mask = (
		  FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR | FTS_NOSTAT
		| FTS_PHYSICAL | FTS_SEEDOT | FTS_XDEV | FTS_NON_RECURSIVE);

	(void) unused;

	assert(result != NULL);
	fts_errors = 0;		/* ensure a fresh start */

	if (nargs > 3)
		lintwarn(_("fts: called with incorrect number of arguments, expecting 3"));

	if (! get_argument(0, AWK_ARRAY, & pathlist)) {
		warning(_("fts: first argument is not an array"));
		update_ERRNO_int(EINVAL);
		goto out;
	}

	if (! get_argument(1, AWK_NUMBER, & flagval)) {
		warning(_("fts: second argument is not a number"));
		update_ERRNO_int(EINVAL);
		goto out;
	}

	if (! get_argument(2, AWK_ARRAY, & dest)) {
		warning(_("fts: third argument is not an array"));
		update_ERRNO_int(EINVAL);
		goto out;
	}

	/* flatten pathlist */
	if (! flatten_array(pathlist.array_cookie, & path_array)) {
		warning(_("fts: could not flatten array\n"));
		goto out;
	}

	/* check the flags first, before the array flattening */

	/* get flags */
	flags = (int) flagval.num_value;

	/* enforce physical or logical but not both, and not no_stat */
	if ((flags & (FTS_PHYSICAL|FTS_LOGICAL)) == 0
	    || (flags & (FTS_PHYSICAL|FTS_LOGICAL)) == (FTS_PHYSICAL|FTS_LOGICAL)) {
		update_ERRNO_int(EINVAL);
		goto out;
	}
#if 0
	if ((flags & FTS_NOSTAT) != 0) {
		flags &= ~FTS_NOSTAT;
		if (do_lint)
			lintwarn(_("fts: ignoring sneaky FTS_NOSTAT flag. nyah, nyah, nyah."));
	}
#endif
	flags &= mask;	/* turn off anything else */

	if (flags & FTS_NON_RECURSIVE)
		flags |= FTS_NOCHDIR;

	/* make pathvector */
	count = path_array->count + 1;
	ezalloc(pathvector, char **, count * sizeof(char *), "do_fts");

	/* fill it in */
	count--;	/* ignore final NULL at end of vector */
	for (i = 0; i < count; i++)
		pathvector[i] = path_array->elements[i].value.str_value.str;

	/* clear dest array */
	b = clear_array(dest.array_cookie);
	assert(b);

	/* let's do it! */
	hierarchy = fts_open(pathvector, flags & ~FTS_NON_RECURSIVE, NULL);
	if (hierarchy != NULL) {
		process(hierarchy, dest.array_cookie,
			(flags & FTS_SEEDOT) != 0,
			(flags & FTS_NON_RECURSIVE) != 0);
		fts_close(hierarchy);

		if (fts_errors == 0)
			ret = 0;
	} else
		update_ERRNO_int(errno);

out:
	if (pathvector != NULL)
		free(pathvector);
	if (path_array != NULL)
		(void) release_flattened_array(pathlist.array_cookie, path_array);

	return make_number(ret, result);
}

static awk_ext_func_t func_table[] = {
	{ "chdir",	do_chdir, 1, 1, awk_false, NULL },
	{ "stat",	do_stat, 3, 2, awk_false, NULL },
	{ "fts",	do_fts, 3, 3, awk_false, NULL },
#if defined(HAVE_SYS_STATVFS_H) && defined(HAVE_STATVFS)
	{ "statvfs",	do_statvfs, 2, 2, awk_false, NULL },
#endif
};


/* define the dl_load function using the boilerplate macro */

dl_load_func(init_filefuncs, func_table, filefuncs, "")
