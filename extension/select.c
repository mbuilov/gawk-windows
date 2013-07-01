/*
 * select.c - Builtin functions to provide select I/O multiplexing.
 */

/*
 * Copyright (C) 2013 the Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;
static const char *ext_version = "ordchr extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

#if defined(HAVE_SELECT) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

/*  do_ord --- return numeric value of first char of string */

static awk_value_t *
do_select(int nargs, awk_value_t *result)
{
	static const char *argname[] = { "read", "write", "except" };
	struct {
		awk_value_t array;
		awk_flat_array_t *flat;
		fd_set bits;
		int *array2fd;
	} fds[3];
	awk_value_t timeout_arg;
	int i;
	struct timeval maxwait;
	struct timeval *timeout;
	int nfds = 0;
	int rc;

	if (do_lint && nargs > 5)
		lintwarn(ext_id, _("select: called with too many arguments"));

#define EL	fds[i].flat->elements[j]
	for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
		size_t j;

		if (! get_argument(i, AWK_ARRAY, & fds[i].array)) {
			warning(ext_id, _("select: bad array parameter `%s'"), argname[i]);
			update_ERRNO_string(_("select: bad array parameter"));
			return make_number(-1, result);
		}
		/* N.B. flatten_array fails for empty arrays, so that's OK */
		FD_ZERO(&fds[i].bits);
		if (flatten_array(fds[i].array.array_cookie, &fds[i].flat)) {
			emalloc(fds[i].array2fd, int *, fds[i].flat->count*sizeof(int), "select");
			for (j = 0; j < fds[i].flat->count; j++) {
				fds[i].array2fd[j] = -1;
				switch (EL.index.val_type) {
				case AWK_NUMBER:
					if (EL.index.num_value >= 0)
						fds[i].array2fd[j] = EL.index.num_value;
					if (fds[i].array2fd[j] != EL.index.num_value)
						fds[i].array2fd[j] = -1;
					break;
				case AWK_STRING:
					if (EL.value.val_type == AWK_STRING) {
						const awk_input_buf_t *buf;
						if ((buf = get_file(EL.index.str_value.str, EL.index.str_value.len, EL.value.str_value.str, EL.value.str_value.len)) != NULL)
							fds[i].array2fd[j] = buf->fd;
					}
					break;
				}
				if (fds[i].array2fd[j] < 0) {
					warning(ext_id, _("select: get_file failed"));
					update_ERRNO_string(_("select: get_file failed"));
					if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
						warning(ext_id, _("select: release_flattened_array failed"));
					free(fds[i].array2fd);
					return make_number(-1, result);
				}
				FD_SET(fds[i].array2fd[j], &fds[i].bits);
				if (nfds <= fds[i].array2fd[j])
					nfds = fds[i].array2fd[j]+1;
			}
		}
		else
			fds[i].flat = NULL;
	}
        if (get_argument(3, AWK_NUMBER, &timeout_arg)) {
		double secs = timeout_arg.num_value;
		if (secs < 0) {
			warning(ext_id, _("select: treating negative timeout as zero"));
			secs = 0;
		}
		maxwait.tv_sec = secs;
		maxwait.tv_usec = (secs-(double)maxwait.tv_sec)*1000000.0;
		timeout = &maxwait;
	} else
		timeout = NULL;
	rc = select(nfds, &fds[0].bits, &fds[1].bits, &fds[2].bits, timeout);

	if (rc < 0) {
		update_ERRNO_int(errno);
		/* bit masks are undefined, so delete all array entries */
		for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
			if (fds[i].flat) {
				size_t j;
				for (j = 0; j < fds[i].flat->count; j++)
					EL.flags |= AWK_ELEMENT_DELETE;
				if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
					warning(ext_id, _("select: release_flattened_array failed"));
				free(fds[i].array2fd);
			}
		}
		return make_number(rc, result);
	}

	for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
		if (fds[i].flat) {
			size_t j;
			/* remove array elements not set in the bit mask */
			for (j = 0; j < fds[i].flat->count; j++) {
				if (! FD_ISSET(fds[i].array2fd[j], &fds[i].bits))
					EL.flags |= AWK_ELEMENT_DELETE;
			}
			if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
				warning(ext_id, _("select: release_flattened_array failed"));
			free(fds[i].array2fd);
		}
	}
#undef EL

	/* Set the return value */
	return make_number(rc, result);
}

static awk_ext_func_t func_table[] = {
	{ "select", do_select, 5 },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, select, "")
