/*
 * fnmatch.c - Provide an interface to fnmatch(3) routine
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2012
 */

/*
 * Copyright (C) 2012, 2013, 2018 the Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#if HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#elif HAVE_SYS_MKDEV_H
#include <sys/mkdev.h>
#endif /* HAVE_SYS_MKDEV_H */

#include <sys/types.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#ifdef __VMS
#define __iswctype iswctype
#define __btowc btowc
#endif

#define _GNU_SOURCE	1	/* use GNU extensions if they're there */
#ifdef HAVE_FNMATCH_H
#include <fnmatch.h>
#else
#ifdef __VMS
#include "fnmatch.h"	/* version that comes with gawk */
#else
#include "../missing_d/fnmatch.h"	/* version that comes with gawk */
#endif
#define HAVE_FNMATCH_H
#endif

#ifndef HAVE_FNMATCH
#ifdef __VMS
#include "fnmatch.c"	/* ditto */
#else
#include "../missing_d/fnmatch.c"	/* ditto */
#endif
#define HAVE_FNMATCH
#endif

/* Provide GNU extensions as no-ops if not defined */
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD	0
#endif
#ifndef FNM_LEADING_DIR
#define FNM_LEADING_DIR	0
#endif
#ifndef FNM_FILE_NAME
#define FNM_FILE_NAME	0
#endif

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "fnmatch extension: version 1.0";

static awk_bool_t init_fnmatch(void);
static awk_bool_t (*init_func)(void) = init_fnmatch;

int plugin_is_GPL_compatible;


/* do_fnmatch --- implement the fnmatch interface */

static awk_value_t *
do_fnmatch(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
#ifdef HAVE_FNMATCH_H
	static int flags_mask =
		FNM_CASEFOLD    | FNM_FILE_NAME |
		FNM_LEADING_DIR | FNM_NOESCAPE |
		FNM_PATHNAME    | FNM_PERIOD ;
#endif
	awk_value_t pattern, string, flags;
	int int_flags, retval;

	make_number(-1.0, result);	/* default return */

#ifdef HAVE_FNMATCH
	if (! get_argument(0, AWK_STRING, & pattern)) {
		warning(ext_id, _("fnmatch: could not get first argument"));
		goto out;
	}

	if (! get_argument(1, AWK_STRING, & string)) {
		warning(ext_id, _("fnmatch: could not get second argument"));
		goto out;
	}

	if (! get_argument(2, AWK_NUMBER, & flags)) {
		warning(ext_id, _("fnmatch: could not get third argument"));
		goto out;
	}

	int_flags = flags.num_value;
	int_flags &= flags_mask;

	retval = fnmatch(pattern.str_value.str,
			string.str_value.str, int_flags);
	make_number((double) retval, result);

out:
#else
	fatal(ext_id, _("fnmatch is not implemented on this system\n"));
#endif
	return result;
}

#define ENTRY(x)	{ #x, FNM_##x }

static struct fnmflags {
	const char *name;
	int value;
} flagtable[] = {
	ENTRY(CASEFOLD),
	ENTRY(FILE_NAME),
	ENTRY(LEADING_DIR),
	ENTRY(NOESCAPE),
	ENTRY(PATHNAME),
	ENTRY(PERIOD),
	{ NULL, 0 }
};

/* init_fnmatch --- load array with flags */

static awk_bool_t
init_fnmatch(void)
{
	int errors = 0;
#ifdef HAVE_FNMATCH
	awk_value_t index, value, the_array;
	awk_array_t new_array;
	int i;

	if (! sym_update("FNM_NOMATCH", make_number(FNM_NOMATCH, & value))) {
		warning(ext_id, _("fnmatch init: could not add FNM_NOMATCH variable"));
		errors++;
	}

	new_array = create_array();
	for (i = 0; flagtable[i].name != NULL; i++) {
		(void) make_const_string(flagtable[i].name,
				strlen(flagtable[i].name), & index);
		(void) make_number(flagtable[i].value, & value);
		if (! set_array_element(new_array, & index, & value)) {
			warning(ext_id, _("fnmatch init: could not set array element %s"),
					flagtable[i].name);
			errors++;
		}
	}

	the_array.val_type = AWK_ARRAY;
	the_array.array_cookie = new_array;

	if (! sym_update("FNM", & the_array)) {
		warning(ext_id, _("fnmatch init: could not install FNM array"));
		errors++;
	}

#endif
	return errors == 0;
}

static awk_ext_func_t func_table[] = {
	{ "fnmatch", do_fnmatch, 3, 3, awk_false, NULL },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, fnmatch, "")
