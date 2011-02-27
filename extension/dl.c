/*
 * dl.c - Example of adding a new builtin function to gawk.
 *
 * Christos Zoulas, Thu Jun 29 17:40:41 EDT 1995
 * Arnold Robbins, update for 3.1, Wed Sep 13 09:38:56 2000
 */

/*
 * Copyright (C) 1995 - 2001, 2011 the Free Software Foundation, Inc.
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
#include <dlfcn.h>

int plugin_is_GPL_compatible;

static void *sdl = NULL;

static NODE *
zaxxon(int nargs)
{
	NODE *obj;
	int i;
	int comma = 0;

	/*
	 * Print the arguments
	 */
	printf("External linkage zaxxon(");

	for (i = 0; i < nargs; i++) {

		obj = get_scalar_argument(i, TRUE);

		if (obj == NULL)
			break;

		force_string(obj);

		printf(comma ? ", %s" : "%s", obj->stptr);
		comma = 1;
	}

	printf(");\n");

	/*
	 * Do something useful
	 */
	obj = get_scalar_argument(0, FALSE);

	if (obj != NULL) {
		force_string(obj);
		if (strcmp(obj->stptr, "unload") == 0 && sdl) {
			/*
			 * XXX: How to clean up the function? 
			 * I would like the ability to remove a function...
			 */
			dlclose(sdl);
			sdl = NULL;
		}
	}

	/* Set the return value */
	return make_number((AWKNUM) 3.14);
}

NODE *
dlload(tree, dl)
NODE *tree;
void *dl;
{
	sdl = dl;
	make_builtin("zaxxon", zaxxon, 4);
	return make_number((AWKNUM) 0);
}
