/*
 * revoutput.c --- Provide an output wrapper that reverses lines.
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 8/2012
 */

/*
 * Copyright (C) 2012, 2013, 2015, 2018 the Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "revoutput extension: version 1.1";

static awk_bool_t init_revoutput(void);
static awk_bool_t (*init_func)(void) = init_revoutput;

int plugin_is_GPL_compatible;

/* rev_fwrite --- write out characters in reverse order */

static size_t
rev_fwrite(const void *buf, size_t size, size_t count, FILE *fp, void *opaque)
{
	const char *cp = buf;
	int nbytes = size * count;

	(void) opaque;

	for (; nbytes >= 1; nbytes--)
		putc(cp[nbytes-1], fp);

	return (size * count);
}


/* revoutput_can_take_file --- return true if we want the file */

static awk_bool_t
revoutput_can_take_file(const awk_output_buf_t *outbuf)
{
	awk_value_t value;

	if (outbuf == NULL)
		return awk_false;

	if (! sym_lookup("REVOUT", AWK_NUMBER, & value))
		return awk_false;

	return (value.num_value != 0);
}

/*
 * revoutput_take_control_of --- set up output wrapper.
 * We can assume that revoutput_can_take_file just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
revoutput_take_control_of(awk_output_buf_t *outbuf)
{
	if (outbuf == NULL)
		return awk_false;

	outbuf->gawk_fwrite = rev_fwrite;
	outbuf->redirected = awk_true;
	return awk_true;
}

static awk_output_wrapper_t output_wrapper = {
	"revoutput",
	revoutput_can_take_file,
	revoutput_take_control_of,
	NULL
};

/* init_revoutput --- set things ups */

static awk_bool_t
init_revoutput()
{
	awk_value_t value;

	register_output_wrapper(& output_wrapper);

	if (! sym_lookup("REVOUT", AWK_SCALAR, & value)) {
		/* only install it if not there, e.g. -v REVOUT=1 */
		make_number(0.0, & value);	/* init to false */
		if (! sym_update("REVOUT", & value)) {
			warning(ext_id, _("revoutput: could not initialize REVOUT variable"));

			return awk_false;
		}
	}

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0, 0, awk_false, NULL }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, revoutput, "")
