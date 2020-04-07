/*
 * fnmatch.c - Provide an interface to fnmatch(3) routine
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 7/2012
 *
 * Michael M. Builov
 * mbuilov@gmail.com
 * Reworked 4/2020
 *
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

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <sys/stat.h>

#ifdef HAVE_SYS_SYSMACROS_H
#include <sys/sysmacros.h>
#elif defined HAVE_SYS_MKDEV_H
#include <sys/mkdev.h>
#endif /* HAVE_SYS_MKDEV_H */

#include <sys/types.h>

#ifndef HAVE_FNMATCH
/* these headers are included in fnmatch.c,
  include them before "gawkapi.h",
  which overrides some system functions */
#include <errno.h>
#include <locale.h>
#include <ctype.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#endif

#include "gawkapi.h"

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
#define HAVE_WFNMATCH
#endif

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

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

GAWK_PLUGIN_GPL_COMPATIBLE

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "fnmatch extension: version 1.1";

static awk_bool_t init_fnmatch(void);
static awk_bool_t (*init_func)(void) = init_fnmatch;


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
	int int_flags, retval = -1;

	(void) nargs, (void) unused;

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

	int_flags = (int) flags.num_value;
	int_flags &= flags_mask;

#ifdef HAVE_WFNMATCH
	if (MB_CUR_MAX > 1) {
		/* Convert to wide-characters both the pattern and the string */
		wchar_t buf[256];
		wchar_t *dbuf = NULL;
		const unsigned bufavail = sizeof(buf)/sizeof(buf[0]);
		const size_t psz = pattern.str_value.len + 1;
		const size_t ssz = string.str_value.len + 1;
		unsigned wcavail = bufavail;
		wchar_t *pwc = NULL, *swc = NULL;
		size_t n;

		if (psz <= wcavail) {
			wcavail -= (unsigned) psz;
			pwc = buf;
		}
		if (ssz <= wcavail)
			swc = buf + sizeof(buf)/sizeof(buf[0]) - wcavail;

		if (psz > bufavail || ssz > bufavail - psz) {
			size_t need = 0;
			if (psz > bufavail) {
				need += psz;
				if (ssz > bufavail)
					need += ssz;
			}
			else
				need = ssz;

			emalloc(dbuf, wchar_t *, need, "do_fnmatch");

			if (psz > bufavail) {
				pwc = dbuf;
				if (ssz > bufavail)
					swc = dbuf + psz;
			}
			else
				swc = dbuf;
		}

		n = mbstowcs(pwc, pattern.str_value.str, psz);
		if (n > psz) {
			warning(ext_id, _("fnmatch: failed to convert to "
					"wide-characters the pattern: \"%s\". LC_ALL=%s"),
					pattern.str_value.str, setlocale(LC_ALL, NULL));
			goto err;
		}

		n = mbstowcs(swc, string.str_value.str, ssz);
		if (n > ssz) {
			warning(ext_id, _("fnmatch: failed to convert to "
					"wide-characters the string: \"%s\". LC_ALL=%s"),
					string.str_value.str, setlocale(LC_ALL, NULL));
			goto err;
		}

		retval = wfnmatch(pwc, swc, int_flags);
err:
		if (dbuf != NULL)
			free(dbuf);
	}
	else
#endif
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
	return (awk_bool_t) (errors == 0);
}

static awk_ext_func_t func_table[] = {
	{ "fnmatch", do_fnmatch, 3, 3, awk_false, NULL },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, fnmatch, "")
