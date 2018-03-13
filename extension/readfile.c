/*
 * readfile.c - Read an entire file into a string.
 *
 * Arnold Robbins
 * Tue Apr 23 17:43:30 IDT 2002
 * Revised per Peter Tillier
 * Mon Jun  9 17:05:11 IDT 2003
 * Revised for new dynamic function facilities
 * Mon Jun 14 14:53:07 IDT 2004
 * Revised for formal API May 2012
 * Added input parser March 2014
 */

/*
 * Copyright (C) 2002, 2003, 2004, 2011, 2012, 2013, 2014, 2018
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

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#ifndef O_BINARY
#define O_BINARY 0
#endif

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "readfile extension: version 2.0";
static awk_bool_t init_readfile();
static awk_bool_t (*init_func)(void) = init_readfile;

int plugin_is_GPL_compatible;

/* read_file_to_buffer --- handle the mechanics of reading the file */

static char *
read_file_to_buffer(int fd, const struct stat *sbuf)
{
	char *text;

	if ((sbuf->st_mode & S_IFMT) != S_IFREG) {
		errno = EINVAL;
		update_ERRNO_int(errno);
		return NULL;
	}

	emalloc(text, char *, sbuf->st_size + 1, "do_readfile");

	if (read(fd, text, sbuf->st_size) != sbuf->st_size) {
		update_ERRNO_int(errno);
		gawk_free(text);
		return NULL;
	}
	text[sbuf->st_size] = '\0';
	return text;
}

/* do_readfile --- read a file into memory */

static awk_value_t *
do_readfile(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename;
	int ret;
	struct stat sbuf;
	char *text;
	int fd;

	assert(result != NULL);
	make_null_string(result);	/* default return value */

	unset_ERRNO();

	if (get_argument(0, AWK_STRING, &filename)) {
		ret = stat(filename.str_value.str, & sbuf);
		if (ret < 0) {
			update_ERRNO_int(errno);
			goto done;
		}

		if ((fd = open(filename.str_value.str, O_RDONLY|O_BINARY)) < 0) {
			update_ERRNO_int(errno);
			goto done;
		}

		text = read_file_to_buffer(fd, & sbuf);
		if (text == NULL)
			goto done;	/* ERRNO already updated */

		close(fd);
		make_malloced_string(text, sbuf.st_size, result);
		goto done;
	} else if (do_lint)
		lintwarn(ext_id, _("readfile: called with wrong kind of argument"));

done:
	/* Set the return value */
	return result;
}

/* readfile_get_record --- read the whole file as one record */

static int
readfile_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
			char **rt_start, size_t *rt_len,
			const awk_fieldwidth_info_t **unused)
{
	char *text;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	if (out == NULL || iobuf == NULL)
		return EOF;

	if (iobuf->opaque != NULL) {
		/*
		 * Already read the whole file,
		 * free up stuff and return EOF
		 */
		gawk_free(iobuf->opaque);
		iobuf->opaque = NULL;
		return EOF;
	}

	/* read file */
	text = read_file_to_buffer(iobuf->fd, & iobuf->sbuf);
	if (text == NULL)
		return EOF;

	/* set up the iobuf for next time */
	iobuf->opaque = text;

	/* set return values */
	*rt_start = NULL;
	*rt_len = 0;
	*out = text;

	/* return count */
	return iobuf->sbuf.st_size;
}

/* readfile_can_take_file --- return true if we want the file */

static awk_bool_t
readfile_can_take_file(const awk_input_buf_t *iobuf)
{
	awk_value_t array, index, value;

	if (iobuf == NULL)
		return awk_false;

	/*
	 * This could fail if PROCINFO isn't referenced from
	 * the awk program. It's not a "can't happen" error.
	 */
	if (! sym_lookup("PROCINFO", AWK_ARRAY, & array)) {
		return awk_false;
	}

	(void) make_const_string("readfile", 8, & index);

	if (! get_array_element(array.array_cookie, & index, AWK_UNDEFINED, & value)) {
		return awk_false;
	}

	return awk_true;
}

/* readfile_take_control_of --- take over the file */

static awk_bool_t
readfile_take_control_of(awk_input_buf_t *iobuf)
{
	if (iobuf == NULL)
		return awk_false;

	iobuf->get_record = readfile_get_record;
	return awk_true;
}

static awk_input_parser_t readfile_parser = {
	"readfile",
	readfile_can_take_file,
	readfile_take_control_of,
	NULL
};

/* init_readfile --- set things up */

static awk_bool_t
init_readfile()
{
	register_input_parser(& readfile_parser);

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ "readfile", do_readfile, 1, 1, awk_false, NULL },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, readfile, "")
