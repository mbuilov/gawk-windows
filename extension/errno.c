/*
 * errno.c - Builtin functions to map errno values.
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
static const char *ext_version = "errno extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

static const char *const errno2name[] = {
#define init_errno(A, B) [A] = B,
#include "errlist.h"
#undef init_errno
};
#define NUMERR	sizeof(errno2name)/sizeof(errno2name[0])

/*  do_strerror --- call strerror */

static awk_value_t *
do_strerror(int nargs, awk_value_t *result)
{
	awk_value_t errnum;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("strerror: called with too many arguments"));

	if (get_argument(0, AWK_NUMBER, & errnum)) {
		const char *str = gettext(strerror(errnum.num_value));
		return make_const_string(str, strlen(str), result);
	}
	if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("strerror: called with no arguments"));
		else
			lintwarn(ext_id, _("strerror: called with inappropriate argument(s)"));
	}
	return make_null_string(result);
}

/*  do_errno2name --- convert an integer errno value to it's symbolic name */

static awk_value_t *
do_errno2name(int nargs, awk_value_t *result)
{
	awk_value_t errnum;
	const char *str;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("errno2name: called with too many arguments"));

	if (get_argument(0, AWK_NUMBER, & errnum)) {
		int i = errnum.num_value;

		if ((i == errnum.num_value) && (i >= 0) && ((size_t)i < NUMERR) && errno2name[i])
			return make_const_string(errno2name[i], strlen(errno2name[i]), result);
		warning(ext_id, _("errno2name: called with invalid argument"));
	} else if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("errno2name: called with no arguments"));
		else
			lintwarn(ext_id, _("errno2name: called with inappropriate argument(s)"));
	}
	return make_null_string(result);
}

/*  do_name2errno --- convert a symbolic errno name to an integer */

static awk_value_t *
do_name2errno(int nargs, awk_value_t *result)
{
	awk_value_t err;
	const char *str;

	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("name2errno: called with too many arguments"));

	if (get_argument(0, AWK_STRING, & err)) {
		size_t i;

		for (i = 0; i < NUMERR; i++) {
			if (errno2name[i] && ! strcasecmp(err.str_value.str, errno2name[i]))
				return make_number(i, result);
		}
		warning(ext_id, _("name2errno: called with invalid argument"));
	} else if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("name2errno: called with no arguments"));
		else
			lintwarn(ext_id, _("name2errno: called with inappropriate argument(s)"));
	}
	return make_number(-1, result);
}

static awk_ext_func_t func_table[] = {
	{ "strerror", do_strerror, 1 },
	{ "errno2name", do_errno2name, 1 },
	{ "name2errno", do_name2errno, 1 },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, errno, "")
