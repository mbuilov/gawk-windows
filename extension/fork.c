/*
 * fork.c - Provide fork and waitpid functions for gawk.
 *
 * Revised 6/2004
 */

/*
 * Copyright (C) 2001, 2004, 2011 the Free Software Foundation, Inc.
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

int plugin_is_GPL_compatible;

/*  do_fork --- provide dynamically loaded fork() builtin for gawk */

static NODE *
do_fork(int nargs)
{
	int ret = -1;
	NODE **aptr;
	NODE *tmp;

	if  (do_lint && get_curfunc_arg_count() > 0)
		lintwarn("fork: called with too many arguments");

	ret = fork();

	if (ret < 0)
		update_ERRNO();
	else if (ret == 0) {
		/* update PROCINFO in the child */

		aptr = assoc_lookup(PROCINFO_node, tmp = make_string("pid", 3), FALSE);
		(*aptr)->numbr = (AWKNUM) getpid();
		unref(tmp);

		aptr = assoc_lookup(PROCINFO_node, tmp = make_string("ppid", 4), FALSE);
		(*aptr)->numbr = (AWKNUM) getppid();
		unref(tmp);
	}

	/* Set the return value */
	return make_number((AWKNUM) ret);
}


/*  do_waitpid --- provide dynamically loaded waitpid() builtin for gawk */

static NODE *
do_waitpid(int nargs)
{
	NODE *pidnode;
	int ret = -1;
	double pidval;
	pid_t pid;
	int options = 0;

	if  (do_lint && get_curfunc_arg_count() > 1)
		lintwarn("waitpid: called with too many arguments");

	pidnode = get_scalar_argument(0, FALSE);
	if (pidnode != NULL) {
		pidval = force_number(pidnode);
		pid = (int) pidval;
		options = WNOHANG|WUNTRACED;
		ret = waitpid(pid, NULL, options);
		if (ret < 0)
			update_ERRNO();
	} else if (do_lint)
		lintwarn("wait: called with no arguments");

	/* Set the return value */
	return make_number((AWKNUM) ret);
}

/* dlload --- load new builtins in this library */

NODE *
dlload(tree, dl)
NODE *tree;
void *dl;
{
	make_builtin("fork", do_fork, 0);
	make_builtin("waitpid", do_waitpid, 1);
	return make_number((AWKNUM) 0);
}
