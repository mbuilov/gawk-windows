/*
 * ext.c - Builtin function that links external gawk functions and related
 *	   utilities.
 *
 * Christos Zoulas, Thu Jun 29 17:40:41 EDT 1995
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 */

/*
 * Copyright (C) 1995 - 2001, 2003-2011 the Free Software Foundation, Inc.
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

#ifdef __GNUC__
static unsigned long long dummy;	/* fake out gcc for dynamic loading? */
#endif

/* do_ext --- load an extension at run-time: interface to load_ext */
 
NODE *
do_ext(int nargs)
{
	NODE *obj, *fun, *ret = NULL;
	SRCFILE *s;
	extern SRCFILE *srcfiles;

	fun = POP_STRING();
	obj = POP_STRING();

	s = add_srcfile(SRC_EXTLIB, obj->stptr, srcfiles, NULL, NULL);
	if (s != NULL)
		ret = load_ext(s->fullpath, fun->stptr, obj);
	DEREF(obj);
	DEREF(fun);
	if (ret == NULL)
		ret = dupnode(Nnull_string);
	return ret;
}

/* load_ext --- load an external library */

NODE *
load_ext(const char *lib_name, const char *init_func, NODE *obj)
{
	NODE *tmp = NULL;
	NODE *(*func)(NODE *, void *);
	void *dl;
	int flags = RTLD_LAZY;
	int *gpl_compat;

#ifdef __GNUC__
	AWKNUM junk;

	junk = (AWKNUM) dummy;
#endif

	if (do_sandbox)
		fatal(_("extensions are not allowed in sandbox mode"));

	if (do_traditional || do_posix)
		fatal(_("`extension' is a gawk extension"));

#ifdef RTLD_GLOBAL
	flags |= RTLD_GLOBAL;
#endif

	if ((dl = dlopen(lib_name, flags)) == NULL)
		fatal(_("extension: cannot open library `%s' (%s)\n"), lib_name,
		      dlerror());

	/* Per the GNU Coding standards */
	gpl_compat = (int *) dlsym(dl, "plugin_is_GPL_compatible");
	if (gpl_compat == NULL)
		fatal(_("extension: library `%s': does not define `plugin_is_GPL_compatible' (%s)\n"),
				lib_name, dlerror());
	func = (NODE *(*)(NODE *, void *)) dlsym(dl, init_func);
	if (func == NULL)
		fatal(_("extension: library `%s': cannot call function `%s' (%s)\n"),
				lib_name, init_func, dlerror());

	if (obj == NULL) {
		obj = make_string(lib_name, strlen(lib_name));
		tmp = (*func)(obj, dl);
		unref(tmp);
		unref(obj);
		return NULL;
	}

	tmp = (*func)(obj, dl);
	return tmp; 
}


/* make_builtin --- register name to be called as func with a builtin body */

void
make_builtin(const char *name, NODE *(*func)(int), int count)
{
	NODE *symbol, *f;
	INSTRUCTION *b;
	const char *sp;
	char c;

	sp = name;
	if (sp == NULL || *sp == '\0')
		fatal(_("extension: missing function name"));

	while ((c = *sp++) != '\0') {
		if ((sp == &name[1] && c != '_' && ! isalpha((unsigned char) c))
				|| (sp > &name[1] && ! is_identchar((unsigned char) c)))
			fatal(_("extension: illegal character `%c' in function name `%s'"), c, name);
	}

	f = lookup(name);

	if (f != NULL) {
		if (f->type == Node_func) {
			/* user-defined function */
			fatal(_("extension: can't redefine function `%s'"), name);
		} else if (f->type == Node_ext_func) {
			/* multiple extension() calls etc. */ 
			if (do_lint)
				lintwarn(_("extension: function `%s' already defined"), name);
			return;
		} else
			/* variable name etc. */ 
			fatal(_("extension: function name `%s' previously defined"), name);
	} else if (check_special(name) >= 0)
		fatal(_("extension: can't use gawk built-in `%s' as function name"), name); 

	if (count < 0)
		fatal(_("make_builtin: negative argument count for function `%s'"),
				name);

	b = bcalloc(Op_symbol, 1, 0);
	b->builtin = func;
	b->expr_count = count;

	/* NB: extension sub must return something */

       	symbol = install_symbol(estrdup(name, strlen(name)), Node_ext_func);
	symbol->code_ptr = b;
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
get_actual_argument(int i, int optional, int want_array)
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
			return get_array(t, FALSE);
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

/* do_ext --- dummy version if extensions not available */

NODE *
do_ext(int nargs)
{
	const char *emsg = _("Operation Not Supported");

	unref(ERRNO_node->var_value);
	ERRNO_node->var_value = make_string(emsg, strlen(emsg));
	return make_number((AWKNUM) -1);
}

/* load_ext --- dummy version if extensions not available */

NODE *
load_ext(const char *lib_name, const char *init_func, NODE *obj)
{
	fatal(_("dynamic loading of library not supported"));
	return NULL;
}
#endif
