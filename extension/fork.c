/*
 * fork.c - Provide fork and waitpid functions for gawk.
 *
 * Revised 6/2004
 * Revised 5/2012 for new extension API.
 */

/*
 * Copyright (C) 2001, 2004, 2011, 2012, 2013, 2018
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

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "fork extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;


/* array_set --- set an array element */

static void
array_set_numeric(awk_array_t array, const char *sub, double num)
{
	awk_value_t index, value;

	set_array_element(array,
		make_const_string(sub, strlen(sub), & index),
		make_number(num, & value));

}

/*  do_fork --- provide dynamically loaded fork() builtin for gawk */

static awk_value_t *
do_fork(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	int ret = -1;

	assert(result != NULL);

	ret = fork();

	if (ret < 0)
		update_ERRNO_int(errno);
	else if (ret == 0) {
		/* update PROCINFO in the child, if the array exists */
		awk_value_t procinfo;

		if (sym_lookup("PROCINFO", AWK_ARRAY, & procinfo)) {
			if (procinfo.val_type != AWK_ARRAY) {
				if (do_lint)
					lintwarn(ext_id, _("fork: PROCINFO is not an array!"));
			} else {
				array_set_numeric(procinfo.array_cookie, "pid", getpid());
				array_set_numeric(procinfo.array_cookie, "ppid", getppid());
			}
		}
	}

	/* Set the return value */
	return make_number(ret, result);
}

/*  do_waitpid --- provide dynamically loaded waitpid() builtin for gawk */

static awk_value_t *
do_waitpid(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t pid;
	int ret = -1;
	int options = 0;

	assert(result != NULL);

	if (get_argument(0, AWK_NUMBER, &pid)) {
		options = WNOHANG|WUNTRACED;
		ret = waitpid(pid.num_value, NULL, options);
		if (ret < 0)
			update_ERRNO_int(errno);
	}

	/* Set the return value */
	return make_number(ret, result);
}


/*  do_wait --- provide dynamically loaded wait() builtin for gawk */

static awk_value_t *
do_wait(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	int ret;

	assert(result != NULL);

	ret = wait(NULL);
	if (ret < 0)
		update_ERRNO_int(errno);

	/* Set the return value */
	return make_number(ret, result);
}

static awk_ext_func_t func_table[] = {
	{ "fork", do_fork, 0, 0, awk_false, NULL },
	{ "waitpid", do_waitpid, 1, 1, awk_false, NULL },
	{ "wait", do_wait, 0, 0, awk_false, NULL },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, fork, "")
