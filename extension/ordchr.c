/*
 * ordchr.c - Builtin functions that provide ord() and chr() functions.
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * 8/2001
 * Revised 6/2004
 * Revised 5/2012
 */

/*
 * Copyright (C) 2001, 2004, 2011, 2012, 2013, 2018, 2020,
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
static awk_ext_id_t ext_id;
static const char *ext_version = "ordchr extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

/*  do_ord --- return numeric value of first char of string */

static awk_value_t *
do_ord(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t str;
	double ret = -1;

	assert(result != NULL);

	if (get_argument(0, AWK_STRING, & str)) {
		ret = str.str_value.str[0];
	} else if (do_lint)
		lintwarn(ext_id, _("ord: first argument is not a string"));

	/* Set the return value */
	return make_number(ret, result);
}

/*  do_chr --- turn numeric value into a string */

static awk_value_t *
do_chr(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t num;
	unsigned int ret = 0;
	double val = 0.0;
	char str[2];

	str[0] = str[1] = '\0';

	assert(result != NULL);

	if (get_argument(0, AWK_NUMBER, & num)) {
		val = num.num_value;
		ret = val;	/* convert to int */
		ret &= 0xff;
		str[0] = ret;
		str[1] = '\0';
	} else if (do_lint)
		lintwarn(ext_id, _("chr: first argument is not a number"));

	/* Set the return value */
	return make_const_string(str, 1, result);
}

static awk_ext_func_t func_table[] = {
	{ "ord", do_ord, 1, 1, awk_false, NULL },
	{ "chr", do_chr, 1, 1, awk_false, NULL },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, ord_chr, "")
