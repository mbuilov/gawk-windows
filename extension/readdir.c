/*
 * readdir.c --- Provide an input parser to read directories
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2012
 */

/*
 * Copyright (C) 2012 the Free Software Foundation, Inc.
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

#define _BSD_SOURCE
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
#error Cannot compile the dirent extension on this system!
#endif

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

static awk_bool_t init_readdir(void);
static awk_bool_t (*init_func)(void) = init_readdir;

int plugin_is_GPL_compatible;

#ifdef DT_BLK
static const int do_ftype = 1;
#else
static int do_ftype;
#endif

/* ftype --- return type of file as a single character string */

static const char *
ftype(struct dirent *entry)
{
#ifdef DT_BLK
	switch (entry->d_type) {
	case DT_BLK:	return "b";
	case DT_CHR:	return "c";
	case DT_DIR:	return "d";
	case DT_FIFO:	return "p";
	case DT_LNK:	return "l";
	case DT_REG:	return "f";
	case DT_SOCK:	return "s";
	default:
	case DT_UNKNOWN:	return "u";
	}
#else
	struct stat sbuf;

	if (lstat(entry->d_name, & sbuf) < 0)
		return "u";

	switch (sbuf.st_mode & S_IFMT) {
	case S_IFREG: return "f";
	case S_IFBLK: return "b";
	case S_IFCHR: return "c";
	case S_IFDIR: return "d";
#ifdef S_IFSOCK
	case S_IFSOCK: return "s";
#endif
#ifdef S_IFIFO
	case S_IFIFO: return "p";
#endif
#ifdef S_IFLNK
	case S_IFLNK: return "l";
#endif
#ifdef S_IFDOOR	/* Solaris weirdness */
	case S_IFDOOR: return "D";
#endif /* S_IFDOOR */
	}
	return "u";
#endif
}


typedef struct open_directory {
	DIR *dp;
	char *buf;
} open_directory_t;

/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, struct iobuf_public *iobuf, int *errcode)
{
	DIR *dp;
	struct dirent *dirent;
	size_t len;
	open_directory_t *the_dir;
	static const awk_value_t null_val = { AWK_UNDEFINED, { { 0, 0 } } };

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs. Except that the compiler complains that it
	 * is unused, so we set it anyways.
	 */
	*errcode = 0;	/* keep the compiler happy */

	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	set_RT((awk_value_t *) & null_val);
	the_dir = (open_directory_t *) iobuf->opaque;
	dp = the_dir->dp;
	dirent = readdir(dp);
	if (dirent == NULL)
		return EOF;

	if (do_ftype)
		sprintf(the_dir->buf, "%ld/%s/%s",
				dirent->d_ino, dirent->d_name, ftype(dirent));
	else
		sprintf(the_dir->buf, "%ld/%s",
				dirent->d_ino, dirent->d_name);

	*out = the_dir->buf;
	len = strlen(the_dir->buf);

	return len;
}

/* dir_close --- close up when done */

static void
dir_close(struct iobuf_public *iobuf)
{
	open_directory_t *the_dir;

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	the_dir = (open_directory_t *) iobuf->opaque;

	closedir(the_dir->dp);
	free(the_dir->buf);
	free(the_dir);

	iobuf->fd = -1;
}

/* dir_can_take_file --- return true if we want the file */

static int
dir_can_take_file(const IOBUF_PUBLIC *iobuf)
{
	struct stat sbuf;
	int fd;

	if (iobuf == NULL)
		return 0;

	fd = iobuf->fd;
	return (fd >= 0 && fstat(fd, & sbuf) >= 0 && S_ISDIR(sbuf.st_mode));
}

/*
 * dir_take_control_of --- set up input parser.
 * We can assume that dir_can_take_file just returned true,
 * and no state has changed since then.
 */

static int
dir_take_control_of(IOBUF_PUBLIC *iobuf)
{
	DIR *dp;
	open_directory_t *the_dir;
	size_t size;

#ifdef HAVE_FDOPENDIR
	dp = fdopendir(iobuf->fd);
#else
	dp = opendir(iobuf->name);
	if (dp != NULL)
		iobuf->fd = dirfd(dp);
#endif
	if (dp == NULL)
		return 0;

	emalloc(the_dir, open_directory_t *, sizeof(open_directory_t), "dir_take_control_of");
	the_dir->dp = dp;
	size = sizeof(struct dirent) + 20 /* max digits in inode */ + 2 /* slashes */;
	emalloc(the_dir->buf, char *, size, "dir_take_control_of");

	iobuf->opaque = the_dir;
	iobuf->get_record = dir_get_record;
	iobuf->close_func = dir_close;

	return 1;
}

static awk_input_parser_t readdir_parser = {
	"readdir",
	dir_can_take_file,
	dir_take_control_of,
	NULL
};

#ifdef TEST_DUPLICATE
static awk_input_parser_t readdir_parser2 = {
	"readdir2",
	dir_can_take_file,
	dir_take_control_of,
	NULL
};
#endif

/* init_readdir --- set things ups */

static awk_bool_t
init_readdir()
{
	register_input_parser(& readdir_parser);
#ifdef TEST_DUPLICATE
	register_input_parser(& readdir_parser2);
#endif

	return 1;
}

/* do_readdir_do_ftype --- enable / disable ftype where need to do stat */

static awk_value_t *
do_readdir_do_ftype(int nargs, awk_value_t *result)
{
	awk_value_t flag;

	make_number(1.0, result);
	if (nargs < 1) {
		warning(ext_id, _("readdir_do_ftype: called with no arguments"));
		make_number(0.0, result);
		goto out;
	} else if (do_lint && nargs > 3)
		lintwarn(ext_id, _("readdir_do_ftype: called with more than one argument"));

	if (! get_argument(0, AWK_NUMBER, & flag)) {
		warning(ext_id, _("readdir_do_ftype: could not get argument"));
		make_number(0.0, result);
		goto out;
	}

#ifndef DT_BLK
	do_ftype = (flag.num_value != 0.0);
#endif

out:
	return result;
}

static awk_ext_func_t func_table[] = {
	{ "readdir_do_ftype", do_readdir_do_ftype, 1 },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, readdir, "")
