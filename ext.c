/*
 * ext.c - Builtin function that links external gawk functions and related
 *	   utilities.
 *
 * Christos Zoulas, Thu Jun 29 17:40:41 EDT 1995
 * Arnold Robbins, update for 3.1, Mon Nov 23 12:53:39 EST 1998
 */

/*
 * Copyright (C) 1995 - 2001, 2003-2014, 2016-2020,
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

#include "awk.h"
extern SRCFILE *srcfiles;

#ifdef DYNAMIC

#include <dlfcn.h>

/* load_ext --- load an external library */

#define TO_STRING_(s)	#s
#define TO_STRING(s)	TO_STRING_(s)

void
load_ext(const char *lib_name)
{
	int (*install_func)(const gawk_api_t *const, gawk_extension_api_ver_t *);
	void *dl;
	int flags = RTLD_LAZY;
	int *gpl_compat;
	int ret;
	gawk_extension_api_ver_t ver;

#ifdef HAVE_DLERROR_BUF
	char dl_buf[1024];
#define dlerror_() dlerror_buf(dl_buf, sizeof(dl_buf))
#else
#define dlerror_() dlerror()
#endif

	if (do_sandbox)
		fatal(_("extensions are not allowed in sandbox mode"));

	if (do_traditional || do_posix)
		fatal(_("-l / @load are gawk extensions"));

	if (lib_name == NULL)
		fatal(_("load_ext: received NULL lib_name"));

	if ((dl = dlopen(lib_name, flags)) == NULL)
		fatal(_("load_ext: cannot open library `%s': %s"), lib_name,
		      dlerror_());

	/* Per the GNU Coding standards */
	gpl_compat = (int *) dlsym(dl, "plugin_is_GPL_compatible");
	if (gpl_compat == NULL)
		fatal(_("load_ext: library `%s': does not define `plugin_is_GPL_compatible': %s"),
		      lib_name, dlerror_());

#if defined(__GNUC__) && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#ifdef __cplusplus
#pragma GCC diagnostic ignored "-Wconditionally-supported"
#endif
#endif
	/* cast pointer-to-object to pointer-to-function */
	install_func = (int (*)(const gawk_api_t *const, gawk_extension_api_ver_t *))
				dlsym(dl, TO_STRING(GAWK_EXT_INIT_FUNC_NAME));
#if defined(__GNUC__) && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

	if (install_func == NULL)
		fatal(_("load_ext: library `%s': cannot call function `%s': %s"),
		      lib_name, TO_STRING(GAWK_EXT_INIT_FUNC_NAME), dlerror_());

	ret = install_func(& api_impl, & ver);
	if (ret < 0) {
		fatal("%s version mismatch with gawk!\n"
		      "\tmy version (API %d.%d), gawk version (API %d.%d)",
		      ver.api_name,
		      ver.need.major_version, ver.need.minor_version,
		      ver.have.major_version, ver.have.minor_version);
	}
	if (ret == 0)
		awkwarn(_("load_ext: library `%s' initialization routine `%s' failed"),
			lib_name, TO_STRING(GAWK_EXT_INIT_FUNC_NAME));
}

/* make_builtin --- register name to be called as func with a builtin body */

awk_bool_t
make_builtin(const char *name_space, awk_ext_func_t *funcinfo)
{
	NODE *symbol, *f;
	INSTRUCTION *b;
	const char *name = funcinfo->name;
	char *install_name;

	if (name == NULL || *name == '\0')
		fatal(_("make_builtin: missing function name"));

	if (! is_valid_identifier(name))
		return awk_false;

	assert(name_space != NULL);
	if (name_space[0] == '\0' || strcmp(name_space, awk_namespace) == 0) {
		if (check_special(name) >= 0)
			fatal(_("make_builtin: cannot use gawk built-in `%s' as function name"), name);

		f = lookup(name);
		install_name = estrdup(name, strlen(name));
	} else {
		if (! is_valid_identifier(name_space))
			return awk_false;

		if (check_special(name_space) >= 0)
			fatal(_("make_builtin: cannot use gawk built-in `%s' as namespace name"), name_space);
		if (check_special(name) >= 0)
			fatal(_("make_builtin: cannot use gawk built-in `%s' as function name"), name);

		size_t len = strlen(name_space) + 2 + strlen(name) + 1;
		char *buf;
		emalloc(buf, char *, len, "make_builtin");
		sprintf(buf, "%s::%s", name_space, name);
		install_name = buf;

		f = lookup(install_name);
	}

	if (f != NULL) {
		// found it, but it shouldn't be there if we want to install this function
		if (f->type == Node_func) {
			/* user-defined function */
			fatal(_("make_builtin: cannot redefine function `%s'"), name);
		} else if (f->type == Node_ext_func) {
			/* multiple extension() calls etc. */
			if (do_lint)
				lintwarn(_("make_builtin: function `%s' already defined"), name);
			return awk_false;
		} else
			/* variable name etc. */
			fatal(_("make_builtin: function name `%s' previously defined"), name);
	}

	b = bcalloc(Op_symbol, 1, 0);
	b->extfunc = funcinfo->function;
	b->c_function = funcinfo;

	/* NB: extension sub must return something */

	symbol = install_symbol(install_name, Node_ext_func);
	symbol->code_ptr = b;
	track_ext_func(name);
	return awk_true;
}

/* get_argument --- get the i'th argument of a dynamically linked function */

NODE *
get_argument(size_t i)
{
	NODE *t;
	size_t arg_count;
	INSTRUCTION *pc;

	pc = TOP()->code_ptr;		/* Op_ext_builtin instruction */
	arg_count = pc->expr_count;	/* # of arguments supplied */

	if (i >= arg_count)
		return NULL;

	t = PEEK(arg_count - i);
	if (t->type == Node_param_list)
		t = GET_PARAM(t->param_cnt);

	if (t->type == Node_array_ref) {
		if (t->orig_array->type == Node_var) {
			/* already a scalar, can no longer use it as array */
			t->type = Node_var;
			t->var_value = Nnull_string;
			return t;
		}
		return t->orig_array; 	/* Node_var_new or Node_var_array */
	}
	if (t->type == Node_var)	/* See Case Node_var in setup_frame(), eval.c */
		return Nnull_string;
	/* Node_var_new, Node_var_array or Node_val */
	return t;
}


/*
 * get_actual_argument --- get the i'th scalar or array argument of a
 *	dynamically linked function.
 */

NODE *
get_actual_argument(NODE *t, size_t i, bool want_array)
{
	const char *fname;
	INSTRUCTION *pc;

	pc = TOP()->code_ptr;	/* Op_ext_builtin instruction */
	fname = (pc + 1)->func_name;

	if (t->type == Node_var_new) {
		if (want_array)
			return force_array(t, false);
		else {
			t->type = Node_var;
			t->var_value = dupnode(Nnull_string);
			return t->var_value;
		}
	}

	if (want_array) {
		if (t->type != Node_var_array)
			fatal(_("function `%s': argument #%" ZUFMT ": attempt to use scalar as an array"),
				fname, i + 1);
	} else {
		if (t->type != Node_val)
			fatal(_("function `%s': argument #%" ZUFMT ": attempt to use array as a scalar"),
				fname, i + 1);
	}
	assert(t->type == Node_var_array || t->type == Node_val);
	return t;
}

#else /* !DYNAMIC */

/* load_ext --- dummy version if extensions not available */

void
load_ext(const char *lib_name)
{
	(void) lib_name;
	fatal(_("dynamic loading of libraries is not supported"));
}
#endif /* !DYNAMIC */

/* close_extensions --- execute extension cleanup routines */

void
close_extensions(void)
{
	SRCFILE *s;

	if (srcfiles == NULL)
		return;

	for (s = srcfiles->next; s != srcfiles; s = s->next)
		if (s->stype == SRC_EXTLIB && s->fini_func)
			(*s->fini_func)();
}

/* is_valid_identifier --- return true if name is a valid simple identifier */

bool
is_valid_identifier(const char *name)
{
	const char *sp = name;
	int c;

	if (! is_letter(*sp))
		return false;

	for (sp++; (c = *sp++) != '\0';) {
		if (! is_identchar(c))
			return false;
	}

	return true;
}
