/*
 * sparr.c - Example of changing behavior of arrays in gawk.
 *	See testsparr.awk for usage.
 */

/*
 * Copyright (C) 2012 the Free Software Foundation, Inc.
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


#include "awk.h"
#include "spec_array.h"

int plugin_is_GPL_compatible;

typedef struct {
	int load_file;
	NODE *filename;
} sdata_t;

/* install_array --- install an array in the symbol table */

static NODE *
install_array(const char *name)
{
	NODE *r;

	r = lookup(name);
	if (r == NULL)
		r = install_symbol(estrdup(name, strlen(name)), Node_var_array);
	switch (r->type) {
	case Node_var_new:
		r = force_array(r, false);
		/* fall through */
	case Node_var_array:
		assoc_clear(r);
		break;
	default:
		fatal(_("`%s' is not an array"), name);
	}
	return r;
}

/* fetch_SYS --- fetch routine for the array `SYS' */

static NODE *
fetch_SYS(NODE *symbol, NODE *subs, void *data)
{
	force_string(subs);
	if (strcmp(subs->stptr, "time") == 0)
		return do_strftime(0);
	return NULL;
}

/* store_SYS --- store routine for the array `SYS' */

static void
store_SYS(NODE *symbol, NODE *subs, NODE *val, void *data)
{
	sdata_t *sd = (sdata_t *) data;

	if (subs != NULL && val != NULL && val->type == Node_val) {
		force_string(subs);
		if (strcmp(subs->stptr, "readline") == 0) {
			sd->load_file = true;
			unref(sd->filename);
			sd->filename = dupnode(val);
		}
	}
}

/* load_READLINE --- load routine for the array `READLINE' */

static void
load_READLINE(NODE *symbol, void *data)
{
	sdata_t *sd = (sdata_t *) data;
	NODE *file, *tmp;
	FILE *fp;
	static char linebuf[BUFSIZ];
	int i;
	bool long_line = false;

	if (! sd->load_file)	/* non-existent SYS["readline"] or already loaded */
		return;

	file = sd->filename;
	force_string(file);

	if (file->stlen == 0)
		return;

	assoc_clear(symbol);

	if ((fp = fopen(file->stptr, "r" )) == NULL) {
		warning(_("READLINE (%s): %s"), file->stptr, strerror(errno));
		return;
	}

	for (i = 1; fgets(linebuf, sizeof(linebuf), fp ) != NULL; i++) {
		NODE **lhs;
		size_t sz;

		sz = strlen(linebuf);
		if (sz > 0 && linebuf[sz - 1] == '\n') {
			linebuf[sz - 1] = '\0';
			sz--;
			if (long_line) {
				long_line = false;
				i--;
				continue;
			}
		} else if (long_line) {
			i--;
			continue;
		} else {
			if (do_lint)
				lintwarn(_("file `%s' does not end in newline or line # `%d' is too long"),
					file->stptr, i);
			long_line = true;
		}

		tmp = make_number(i);
		lhs = assoc_lookup(symbol, tmp);
		unref(tmp);
		unref(*lhs);
		*lhs = make_string(linebuf, sz);
	}
	fclose(fp);
	sd->load_file = false;	/* don't load this file again */
}

/* dlload --- load this library */

NODE *
dlload(NODE *obj, void *dl)
{
	NODE *a1, *a2;
	static sdata_t data;

	a1 = install_array("SYS");
	register_dyn_array(a1, fetch_SYS, store_SYS, & data);
	a2 = install_array("READLINE");
	register_deferred_array(a2, load_READLINE, & data);
	return make_number((AWKNUM) 0);
}
