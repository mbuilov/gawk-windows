/*
 * ext.c - Builtin function that links external gawk functions and related
 *	   utilities.
 *
 * Christos Zoulas, Thu Jun 29 17:40:41 EDT 1995
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 */

/*
 * Copyright (C) 1995 - 2001, 2003-2012 the Free Software Foundation, Inc.
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

#ifdef DYNAMIC

#include <dlfcn.h>

/*
 * is_letter --- function to check letters
 *	isalpha() isn't good enough since it can look at the locale.
 * Underscore counts as a letter in awk identifiers
 */

static bool
is_letter(unsigned char c)
{
	switch (c) {
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	case '_':
		return true;
	default:
		return false;
	}
}

/* is_identifier_char --- return true if a character can be used in an identifier */

static bool
is_identifier_char(unsigned char c)
{
	return (is_letter(c) || isdigit(c));
}


#define INIT_FUNC	"dl_load"

/* load_ext --- load an external library */

void
load_ext(const char *lib_name)
{
	int (*install_func)(const gawk_api_t *const, awk_ext_id_t);
	void *dl;
	int flags = RTLD_LAZY;
	int *gpl_compat;

	if (do_sandbox)
		fatal(_("extensions are not allowed in sandbox mode"));

	if (do_traditional || do_posix)
		fatal(_("-l / @load are gawk extensions"));

	if (lib_name == NULL)
		fatal(_("load_ext: received NULL lib_name"));

	if ((dl = dlopen(lib_name, flags)) == NULL)
		fatal(_("load_ext: cannot open library `%s' (%s)\n"), lib_name,
		      dlerror());

	/* Per the GNU Coding standards */
	gpl_compat = (int *) dlsym(dl, "plugin_is_GPL_compatible");
	if (gpl_compat == NULL)
		fatal(_("load_ext: library `%s': does not define `plugin_is_GPL_compatible' (%s)\n"),
				lib_name, dlerror());

	install_func = (int (*)(const gawk_api_t *const, awk_ext_id_t))
				dlsym(dl, INIT_FUNC);
	if (install_func == NULL)
		fatal(_("load_ext: library `%s': cannot call function `%s' (%s)\n"),
				lib_name, INIT_FUNC, dlerror());

	if (install_func(& api_impl, NULL /* ext_id */) == 0)
		warning(_("load_ext: library `%s' initialization routine `%s' failed\n"),
				lib_name, INIT_FUNC);
}


/* make_builtin --- register name to be called as func with a builtin body */

awk_bool_t
make_builtin(const awk_ext_func_t *funcinfo)
{
	NODE *symbol, *f;
	INSTRUCTION *b;
	const char *sp;
	char c;
	const char *name = funcinfo->name;
	int count = funcinfo->num_expected_args;

	sp = name;
	if (sp == NULL || *sp == '\0')
		fatal(_("make_builtin: missing function name"));

	if (! is_letter(*sp))
		return false;

	sp++;

	while ((c = *sp++) != '\0') {
		if (! is_identifier_char(c))
			return false;
	}

	f = lookup(name);

	if (f != NULL) {
		if (f->type == Node_func) {
			/* user-defined function */
			fatal(_("make_builtin: can't redefine function `%s'"), name);
		} else if (f->type == Node_ext_func) {
			/* multiple extension() calls etc. */ 
			if (do_lint)
				lintwarn(_("make_builtin: function `%s' already defined"), name);
			return false;
		} else
			/* variable name etc. */ 
			fatal(_("make_builtin: function name `%s' previously defined"), name);
	} else if (check_special(name) >= 0)
		fatal(_("make_builtin: can't use gawk built-in `%s' as function name"), name); 

	if (count < 0)
		fatal(_("make_builtin: negative argument count for function `%s'"),
				name);

	b = bcalloc(Op_symbol, 1, 0);
	b->extfunc = funcinfo->function;
	b->expr_count = count;

	/* NB: extension sub must return something */

       	symbol = install_symbol(estrdup(name, strlen(name)), Node_ext_func);
	symbol->code_ptr = b;
	track_ext_func(name);
	return true;
}


/* get_argument --- get the i'th argument of a dynamically linked function */

NODE *
get_argument(int i)
{
	NODE *t;
	int arg_count, pcount;
	INSTRUCTION *pc;
	
	pc = TOP()->code_ptr;		/* Op_ext_builtin instruction */
	pcount = (pc + 1)->expr_count;	/* max # of arguments */
	arg_count = pc->expr_count;	/* # of arguments supplied */

	if (i < 0 || i >= pcount || i >= arg_count)
		return NULL;

	t = PEEK(arg_count - i);
	if (t->type == Node_param_list)
		t = GET_PARAM(t->param_cnt);

	if (t->type == Node_array_ref)
		t = t->orig_array;
	if (t->type == Node_var)	/* See Case Node_var in setup_frame(), eval.c */
		return Nnull_string;
	/* Node_var_new, Node_var_array or Node_val */
	return t;
}


/* get_actual_argument --- get the i'th scalar or array argument of a
	dynamically linked function, allowed to be optional.
*/

NODE *
get_actual_argument(int i, bool optional, bool want_array)
{
	NODE *t;
	char *fname;
	int pcount;
	INSTRUCTION *pc;
	
	pc = TOP()->code_ptr;	/* Op_ext_builtin instruction */
	fname = (pc + 1)->func_name;
	pcount = (pc + 1)->expr_count;
 
	t = get_argument(i);
	if (t == NULL) {
		if (i >= pcount)                /* must be fatal */
			fatal(_("function `%s' defined to take no more than %d argument(s)"),
					fname, pcount);
		if (! optional)
			fatal(_("function `%s': missing argument #%d"),
					fname, i + 1);
		return NULL;
	}

	if (t->type == Node_var_new) {
		if (want_array)
			return get_array(t, false);
		else {
			t->type = Node_var;
			t->var_value = dupnode(Nnull_string);
			return t->var_value;
		}
	}

	if (want_array) {
		if (t->type != Node_var_array)
			fatal(_("function `%s': argument #%d: attempt to use scalar as an array"),
				fname, i + 1);
	} else {
		if (t->type != Node_val)
			fatal(_("function `%s': argument #%d: attempt to use array as a scalar"),
				fname, i + 1);
	}
	assert(t->type == Node_var_array || t->type == Node_val);
	return t;
}

#else

/* load_ext --- dummy version if extensions not available */

void
load_ext(const char *lib_name)
{
	fatal(_("dynamic loading of library not supported"));
}
#endif
