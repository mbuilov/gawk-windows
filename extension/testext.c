/*
 * testext.c - tests for the extension API.
 */

/*
 * Copyright (C) 2012
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

static awk_value_t *
do_func1(int nargs, awk_value_t *result)
{
	/* get PROCINFO as flat array and print it */
}

static awk_value_t *
do_func2(int nargs, awk_value_t *result)
{
	/* look up a reserved variable - should fail */
	/* look up variable whose name is passed in, should pass */
	/* change the value, should be reflected in awk script */
}

static awk_value_t *
do_func3(int nargs, awk_value_t *result)
{
	/* set ERRNO, should be reflected in awk script */
}

static awk_value_t *
do_func4(int nargs, awk_value_t *result)
{
	/* get element count and print it; should match length(array) from awk script */
	/* clear array - length(array) should then go to zero in script */
}

static awk_value_t *
do_func5(int nargs, awk_value_t *result)
{
	/* look up an array element and print the value */
	/* change the element */
	/* delete another element */
	/* change and deletion should be reflected in awk script */
}


static awk_ext_func_t func_table[] = {
	{ "test_func1", do_func1, 1 },
};



int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id)
{
	size_t i, j;
	int errors = 0;

	api = api_p;
	ext_id = id;

	if (api->major_version != GAWK_API_MAJOR_VERSION
	    || api->minor_version < GAWK_API_MINOR_VERSION) {
		fprintf(stderr, "testfuncs: version mismatch with gawk!\n");
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n",
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION,
			api->major_version, api->minor_version);
		exit(1);
	}

	/* load functions */
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) {
		if (! add_ext_func(& func_table[i], name_space)) {
			warning(ext_id, "testfuncs: could not add %s\n",
					func_table[i].name);
			errors++;
		}
	}

	/* add at_exit functions */

	/* install some variables */

	return (errors == 0);
}
