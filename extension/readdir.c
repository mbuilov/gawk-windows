/*
 * readdir.c --- Provide an open hook to read directories
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
#include <dirent.h>

#include "config.h"
#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

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


/* dir_get_record --- get one record at a time out of a directory */

static int
dir_get_record(char **out, struct iobuf_public *iobuf, int *errcode)
{
	DIR *dp;
	struct dirent *dirent;
	char buf[1000];
	size_t len;

	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	/* FIXME: Need stuff for setting RT */
	dp = (DIR *) iobuf->opaque;
	dirent = readdir(dp);
	if (dirent == NULL)
		return EOF;

	sprintf(buf, "%ld/%s/%s", dirent->d_ino, dirent->d_name, ftype(dirent));
	len = strlen(buf);
	emalloc(*out, char *, len + 1, "dir_get_record");
	strcpy(*out, buf);
	return len;
}

/* dir_close --- close up when done */

static void
dir_close(struct iobuf_public *iobuf)
{
	DIR *dp;

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	dp = (DIR *) iobuf->opaque;
	closedir(dp);
	iobuf->fd = -1;
}

/* dir_can_take_file --- return true if we want the file */

static int
dir_can_take_file(IOBUF_PUBLIC *iobuf)
{
	struct stat sbuf;
	int fd;

	if (iobuf == NULL)
		return 0;

	fd = iobuf->fd;
	return (fd >= 0 && fstat(fd, & sbuf) >= 0 && S_ISDIR(sbuf.st_mode));
}

/* dir_take_control_of --- set up input parser.  We can assume that dir_can_take_file just returned true, and no state has changed since then. */

static int
dir_take_control_of(IOBUF_PUBLIC *iobuf)
{
	struct stat sbuf;
	DIR *dp;

	dp = fdopendir(iobuf->fd);
	if (dp == NULL)
		return 0;

	iobuf->opaque = dp;
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

int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id) 
{
	api = api_p;
	ext_id = id;

	if (api->major_version != GAWK_API_MAJOR_VERSION
	    || api->minor_version < GAWK_API_MINOR_VERSION) {
		fprintf(stderr, "readdir: version mismatch with gawk!\n");
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n",
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION,
			api->major_version, api->minor_version);
		exit(1);
	}


	register_input_parser(& readdir_parser);
#ifdef TEST_DUPLICATE
	register_input_parser(& readdir_parser2);
#endif

	return 1;
}
