/*
 * readdir.c --- Provide an input parser to read directories
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2012
 *
 * Andrew Schorr and Arnold Robbins: further fixes 8/2012.
 * Simplified 11/2012.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _BSD_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

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
static const char *ext_version = "readdir extension: version 1.0";

static awk_bool_t init_readdir(void);
static awk_bool_t (*init_func)(void) = init_readdir;

int plugin_is_GPL_compatible;

/* data type for the opaque pointer: */

typedef struct open_directory {
	DIR *dp;
	char *buf;
} open_directory_t;

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
	case DT_UNKNOWN: return "u";
	}
#else
	return "u";
#endif
}

/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len)
{
	DIR *dp;
	struct dirent *dirent;
	int len;
	open_directory_t *the_dir;
	const char *ftstr;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	the_dir = (open_directory_t *) iobuf->opaque;
	dp = the_dir->dp;

	/*
	 * Initialize errno, since readdir does not set it to zero on EOF.
	 */
	errno = 0;
	dirent = readdir(dp);
	if (dirent == NULL) {
		*errcode = errno;	/* in case there was an error */
		return EOF;
	}

#ifdef ZOS_USS
	len = sprintf(the_dir->buf, "%lu/%s",
			(unsigned long) dirent->d_ino,
			dirent->d_name);
#else
	len = sprintf(the_dir->buf, "%llu/%s",
			(unsigned long long) dirent->d_ino,
			dirent->d_name);
#endif

	ftstr = ftype(dirent);
	len += sprintf(the_dir->buf + len, "/%s", ftstr);

	*out = the_dir->buf;

	*rt_len = 0;	/* set RT to "" */
	return len;
}

/* dir_close --- close up when done */

static void
dir_close(awk_input_buf_t *iobuf)
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

static awk_bool_t
dir_can_take_file(const awk_input_buf_t *iobuf)
{
	if (iobuf == NULL)
		return awk_false;

	return (iobuf->fd != INVALID_HANDLE && S_ISDIR(iobuf->sbuf.st_mode));
}

/*
 * dir_take_control_of --- set up input parser.
 * We can assume that dir_can_take_file just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
dir_take_control_of(awk_input_buf_t *iobuf)
{
	DIR *dp;
	open_directory_t *the_dir;
	size_t size;

	errno = 0;
#ifdef HAVE_FDOPENDIR
	dp = fdopendir(iobuf->fd);
#else
	dp = opendir(iobuf->name);
	if (dp != NULL)
		iobuf->fd = dirfd(dp);
#endif
	if (dp == NULL) {
		warning(ext_id, _("dir_take_control_of: opendir/fdopendir failed: %s"),
				strerror(errno));
		update_ERRNO_int(errno);
		return awk_false;
	}

	emalloc(the_dir, open_directory_t *, sizeof(open_directory_t), "dir_take_control_of");
	the_dir->dp = dp;
	size = sizeof(struct dirent) + 21 /* max digits in inode */ + 2 /* slashes */;
	emalloc(the_dir->buf, char *, size, "dir_take_control_of");

	iobuf->opaque = the_dir;
	iobuf->get_record = dir_get_record;
	iobuf->close_func = dir_close;

	return awk_true;
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

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0 }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, readdir, "")
