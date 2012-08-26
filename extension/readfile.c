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
 */

/*
 * Copyright (C) 2002, 2003, 2004, 2011, 2012 the Free Software Foundation, Inc.
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
static awk_ext_id_t *ext_id;
static const char *ext_version = "readfile extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

/* do_readfile --- read a file into memory */

static awk_value_t *
do_readfile(int nargs, awk_value_t *result)
{
	awk_value_t filename;
	int ret;
	struct stat sbuf;
	char *text;
	int fd;

	assert(result != NULL);
	make_null_string(result);	/* default return value */

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("readfile: called with too many arguments"));

	unset_ERRNO();

	if (get_argument(0, AWK_STRING, &filename)) {
		ret = stat(filename.str_value.str, & sbuf);
		if (ret < 0) {
			update_ERRNO_int(errno);
			goto done;
		} else if ((sbuf.st_mode & S_IFMT) != S_IFREG) {
			errno = EINVAL;
			update_ERRNO_int(errno);
			goto done;
		}

		if ((fd = open(filename.str_value.str, O_RDONLY|O_BINARY)) < 0) {
			update_ERRNO_int(errno);
			goto done;
		}

		emalloc(text, char *, sbuf.st_size + 2, "do_readfile");
		memset(text, '\0', sbuf.st_size + 2);

		if ((ret = read(fd, text, sbuf.st_size)) != sbuf.st_size) {
			(void) close(fd);
			update_ERRNO_int(errno);
			goto done;
		}

		close(fd);
		make_malloced_string(text, sbuf.st_size, result);
		goto done;
	} else if (do_lint)
		lintwarn(ext_id, _("readfile: called with no arguments"));


done:
	/* Set the return value */
	return result;
}

static awk_ext_func_t func_table[] = {
	{ "readfile", do_readfile, 1 },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, readfile, "")
