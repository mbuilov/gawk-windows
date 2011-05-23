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

/* do_ext --- load an extension */

NODE *
do_ext(int nargs)
{
	NODE *obj;
	NODE *fun;
	NODE *tmp = NULL;
	NODE *(*func)(NODE *, void *);
	void *dl;
	int flags = RTLD_LAZY;
	int fatal_error = FALSE;
	int *gpl_compat;
#if 0
	static short warned = FALSE;
#endif

#ifdef __GNUC__
	AWKNUM junk;

	junk = (AWKNUM) dummy;
#endif

	if (do_sandbox)
		fatal(_("extensions are not allowed in sandbox mode"));

#if 0
	/* already done in parser */
	if (do_lint && ! warned) {
		warned = TRUE;
		lintwarn(_("`extension' is a gawk extension"));
	}
#endif

	if (do_traditional || do_posix)
		error(_("`extension' is a gawk extension"));

	fun = POP_STRING();
	obj = POP_STRING();

#ifdef RTLD_GLOBAL
	flags |= RTLD_GLOBAL;
#endif
	if ((dl = dlopen(obj->stptr, flags)) == NULL) {
		/* fatal needs `obj', and we need to deallocate it! */
		msg(_("fatal: extension: cannot open `%s' (%s)\n"), obj->stptr,
		      dlerror());
		fatal_error = TRUE;
		goto done;
	}

	/* Per the GNU Coding standards */
	gpl_compat = (int *) dlsym(dl, "plugin_is_GPL_compatible");
	if (gpl_compat == NULL) {
		msg(_("fatal: extension: library `%s': does not define `plugin_is_GPL_compatible' (%s)\n"),
				obj->stptr, dlerror());
		fatal_error = TRUE;
		goto done;
	}


	func = (NODE *(*)(NODE *, void *)) dlsym(dl, fun->stptr);
	if (func == NULL) {
		msg(_("fatal: extension: library `%s': cannot call function `%s' (%s)\n"),
				obj->stptr, fun->stptr, dlerror());
		fatal_error = TRUE;
		goto done;
	}

	tmp = (*func)(obj, dl);
	if (tmp == NULL)
		tmp = Nnull_string;
done:
	DEREF(obj);
	DEREF(fun);
	if (fatal_error)
		gawk_exit(EXIT_FATAL);
	return tmp; 
}


/* make_builtin --- register name to be called as func with a builtin body */

void
make_builtin(const char *name, NODE *(*func)(int), int count)
{
	NODE *p, *symbol, *f;
	INSTRUCTION *b, *r;
	const char *sp;
	char *pname;
	char **vnames = NULL;
	char c, buf[200];
	size_t space_needed;
	int i;

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
			INSTRUCTION *pc = f->code_ptr;
			if (pc->opcode != Op_ext_func)	/* user-defined function */
				fatal(_("extension: can't redefine function `%s'"), name);
			else {
				/* multiple extension() calls etc. */ 
				if (do_lint)
					lintwarn(_("extension: function `%s' already defined"), name);
				return;
			}
		} else
			/* variable name etc. */ 
			fatal(_("extension: function name `%s' previously defined"), name);
	} else if (check_special(name) >= 0)
		fatal(_("extension: can't use gawk built-in `%s' as function name"), name); 
	/* count parameters, create artificial list of param names */

	if (count < 0)
		fatal(_("make_builtin: negative argument count for function `%s'"),
					name);

	if (count > 0) {
		sprintf(buf, "p%d", count);
		space_needed = strlen(buf) + 1;
		emalloc(vnames, char **, count * sizeof(char  *), "make_builtin");
		for (i = 0; i < count; i++) {
			emalloc(pname, char *, space_needed, "make_builtin");
			sprintf(pname, "p%d", i);
			vnames[i] = pname;
		}
	}


	getnode(p);
	p->type = Node_param_list;
	p->flags |= FUNC;
	/* get our own copy for name */
	p->param = estrdup(name, strlen(name));
	p->param_cnt = count;

	/* actual source and line numbers set at runtime for these instructions */
	b = bcalloc(Op_builtin, 1, __LINE__);
	b->builtin = func;
	b->expr_count = count;
	b->nexti = bcalloc(Op_K_return, 1, __LINE__);
	r = bcalloc(Op_ext_func, 1, __LINE__);
	r->source_file = __FILE__;
	r->nexti = b;

	/* NB: extension sub must return something */

	symbol = mk_symbol(Node_func, p);
	symbol->parmlist = vnames;
	symbol->code_ptr = r;
	r->func_body = symbol;
	(void) install_symbol(p->param, symbol);
}


/* get_curfunc_arg_count --- return number actual parameters */

size_t
get_curfunc_arg_count()
{
	size_t argc;
	INSTRUCTION *pc;
	
	pc = (INSTRUCTION *) frame_ptr->reti;      /* Op_func_call instruction */
	argc = (pc + 1)->expr_count;        /* # of arguments supplied */
	return argc;
}


/* get_argument --- get the n'th argument of a dynamically linked function */

NODE *
get_argument(int i)
{
	int pcount;
	NODE *t, *f;
	int actual_args;
	INSTRUCTION *pc;
	
	f = frame_ptr->func_node;
	pcount = f->lnode->param_cnt;

	pc = (INSTRUCTION *) frame_ptr->reti;     /* Op_func_call instruction */
	actual_args = (pc + 1)->expr_count;       /* # of arguments supplied */
         
	if (i < 0 || i >= pcount || i >= actual_args)
		return NULL;

	t = GET_PARAM(i);

	if (t->type == Node_array_ref)
		return t->orig_array; 	/* Node_var_new or Node_var_array */
	if (t->type == Node_var_new || t->type == Node_var_array)
		return t;
	return t->var_value;
}


/* get_actual_argument --- get a scalar or array, allowed to be optional */

NODE *
get_actual_argument(int i, int optional, int want_array)
{
	/* optional : if TRUE and i th argument not present return NULL, else fatal. */

	NODE *t, *f;
	int pcount;
	char *fname;

	t = get_argument(i);

	f = frame_ptr->func_node;
	pcount = f->lnode->param_cnt;
	fname = f->lnode->param;

	if (t == NULL) {
		if (i >= pcount)		/* must be fatal */
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
			t->var_value = Nnull_string;
			return Nnull_string;
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
#endif
