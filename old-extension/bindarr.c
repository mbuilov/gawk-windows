/*
 * bindarr.c - routines for binding (attaching) user-defined functions
 *		to array and array elements.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2011 the Free Software Foundation, Inc.
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

/*
 * Binding an array is basically the binding of functions to the internal
 * triggers for reading and writing that array or an element of that array.
 * This allows the user to define the set of behaviors for gawk arrays
 * using gawk functions. With arrays you can assign and read values of
 * specific elements, provide list of indices and values, and tell if a
 * certain index exists or not. A variable can be "tied" by including
 * code which overrides any or all of the standard behaviors of awk arrays.
 *
 * See dbarray.awk and testdbarray.awk to learn how to bind an array
 * to an external database for persistent storage.
 */

int plugin_is_GPL_compatible;

static NODE **bind_array_lookup(NODE *, NODE *);
static NODE **bind_array_exists(NODE *, NODE *);
static NODE **bind_array_clear(NODE *, NODE *);
static NODE **bind_array_remove(NODE *, NODE *);
static NODE **bind_array_list(NODE *, NODE *);
static NODE **bind_array_store(NODE *, NODE *);
static NODE **bind_array_length(NODE *, NODE *);

static afunc_t bind_array_func[] = {
	(afunc_t) 0,
	(afunc_t) 0,
	bind_array_length,
	bind_array_lookup,
	bind_array_exists,
	bind_array_clear,
	bind_array_remove,
	bind_array_list,
	null_afunc,	/* copy */
	null_afunc,	/* dump */
	bind_array_store,
};

enum { INIT, FINI, COUNT, EXISTS, LOOKUP,
	STORE, DELETE, CLEAR, FETCHALL
};

static const char *const bfn[] = {
	"init", "fini", "count", "exists", "lookup",
	"store", "delete", "clear", "fetchall",
};

typedef struct {
	NODE *func[sizeof(bfn)/sizeof(char *)];
	NODE *arg0;
} array_t;

static NODE *call_func(NODE *func, NODE **arg, int arg_count);
static long array_func_call(NODE *, NODE *, int);


/* bind_array_length -- find the number of elements in the array */

static NODE **
bind_array_length(NODE *symbol, NODE *subs ATTRIBUTE_UNUSED)
{
	static NODE *length_node;

	symbol->table_size = array_func_call(symbol, NULL, COUNT);
	length_node = symbol;
	return & length_node;
}

/* bind_array_lookup --- find element in the array; return a pointer to value. */

static NODE **
bind_array_lookup(NODE *symbol, NODE *subs)
{
	NODE *xn = symbol->xarray;
	(void) array_func_call(symbol, subs, LOOKUP);
	return xn->alookup(xn, subs);
}

/*
 * bind_array_exists --- test whether the array element symbol[subs] exists or not,
 *	return pointer to value if it does.
 */

static NODE **
bind_array_exists(NODE *symbol, NODE *subs)
{
	NODE *xn = symbol->xarray;
	(void) array_func_call(symbol, subs, EXISTS);
	return xn->aexists(xn, subs);
}

/* bind_array_clear --- flush all the values in symbol[] */

static NODE **
bind_array_clear(NODE *symbol, NODE *subs ATTRIBUTE_UNUSED)
{
	NODE *xn = symbol->xarray;
	(void) xn->aclear(xn, NULL);
	(void) array_func_call(symbol, NULL, CLEAR);
	return NULL;
}

/* bind_array_remove --- if subs is already in the table, remove it. */

static NODE **
bind_array_remove(NODE *symbol, NODE *subs)
{
	NODE *xn = symbol->xarray;
	(void) xn->aremove(xn, subs);
	(void) array_func_call(symbol, subs, DELETE);
	return NULL;
}

/* bind_array_store --- update the value for the SUBS */

static NODE **
bind_array_store(NODE *symbol, NODE *subs)
{
	(void) array_func_call(symbol, subs, STORE);
	return NULL;
}

/* bind_array_list --- return a list of array items */

static NODE**
bind_array_list(NODE *symbol, NODE *akind)
{
	NODE *xn = symbol->xarray;
	(void) array_func_call(symbol, NULL, FETCHALL);
	return xn->alist(xn, akind);
}


/* array_func_call --- call user-defined array routine */

static long
array_func_call(NODE *symbol, NODE *arg1, int fi)
{
	NODE *argp[3];
	NODE *retval;
	long ret;
	int i = 0;
	array_t *aq;

	aq = symbol->a_opaque;
	if (! aq)	/* an array routine invoked from the same or another routine */
		fatal(_("bind_array: cannot access bound array, operation not allowed"));
	symbol->a_opaque = NULL; 	/* avoid infinite recursion */

	argp[i++] = symbol->xarray;
	argp[i++] = aq->arg0;
	if (arg1 != NULL)
		argp[i++] = arg1;

	retval = call_func(aq->func[fi], argp, i);
	symbol->a_opaque = aq;
	force_number(retval);
	ret = get_number_si(retval);
	unref(retval);
	if (ret < 0) {
		if (ERRNO_node->var_value->stlen > 0)
			fatal(_("%s"), ERRNO_node->var_value->stptr);
		else
			fatal(_("unknown reason"));
	}
	return ret;
}

/* do_bind_array --- bind an array to user-defined functions */

static NODE *
do_bind_array(int nargs)
{
	NODE *symbol, *xn, *t, *td;
	int i;
	array_t *aq;
	char *aname;

	symbol = get_array_argument(0, false);
	if (symbol->array_funcs == bind_array_func)
		fatal(_("bind_array: array `%s' already bound"), array_vname(symbol));

	assoc_clear(symbol);

	ezalloc(aq, array_t *, sizeof(array_t), "do_bind_array");

	t = get_array_argument(1, false);

	for (i = 0; i < sizeof(bfn)/sizeof(char *); i++) {
		NODE *subs, *val, *f;

		subs = make_string(bfn[i], strlen(bfn[i]));
		val = in_array(t, subs);
		unref(subs);
		if (val == NULL) {
			if (i != INIT && i != FINI)
				fatal(_("bind_array: array element `%s[\"%s\"]' not defined"),
						t->vname, bfn[i]);
			continue;
		}

		force_string(val);
		f = lookup(val->stptr);
		if (f == NULL || f->type != Node_func)
			fatal(_("bind_array: function `%s' is not defined"), val->stptr);
		aq->func[i] = f;
	}

	/* copy the array -- this is passed as the second argument to the functions */
	emalloc(aname, char *, 1 + strlen(symbol->vname) + 1, "do_bind_array");
	aname[0] = '~';		/* any illegal character */
	strcpy(& aname[1], symbol->vname);
	td = make_array();
	td->vname = aname;
	assoc_copy(t, td);
	aq->arg0 = td;

	/* internal array for the actual storage */
	xn = make_array();
	xn->vname = symbol->vname;	/* shallow copy */
	xn->flags |= XARRAY;
	symbol->a_opaque = aq;
	symbol->array_funcs = bind_array_func;
	symbol->xarray = xn;

	if (aq->func[INIT] != NULL)
		(void) array_func_call(symbol, NULL, INIT);

	return make_number(0);
}

/* do_unbind_array --- unbind an array */

static NODE *
do_unbind_array(int nargs)
{
	NODE *symbol, *xn, *td;
	array_t *aq;

	symbol = get_array_argument(0, false);
	if (symbol->array_funcs != bind_array_func)
		fatal(_("unbind_array: `%s' is not a bound array"), array_vname(symbol));

	aq = symbol->a_opaque;
	if (aq->func[FINI] != NULL)
		(void) array_func_call(symbol, NULL, FINI);

	td = aq->arg0;
	assoc_clear(td);
	efree(td->vname);
	freenode(td);
	efree(aq);

	/* promote xarray to symbol */
	xn = symbol->xarray;
	xn->flags &= ~XARRAY;
	xn->parent_array = symbol->parent_array;
	*symbol = *xn;
	freenode(xn);

	return make_number(0);
}


/* call_func --- call a user-defined gawk function */

static NODE *
call_func(NODE *func, NODE **arg, int arg_count)
{
	NODE *ret;
	INSTRUCTION *code;
	extern int currule;
	int i, save_rule = 0;

	if (arg_count > func->param_cnt)
		fatal(_("function `%s' called with too many parameters"), func->vname);

	/* make function call instructions */
	code = bcalloc(Op_func_call, 2, 0);
	code->func_body = func;
	code->func_name = NULL;		/* not needed, func_body already assigned */
	(code + 1)->expr_count = arg_count;
	code->nexti = bcalloc(Op_stop, 1, 0);

	save_rule = currule;    /* save current rule */
	currule = 0;

	/* push arguments onto stack */
	for (i = 0; i < arg_count; i++) {
		if (arg[i]->type == Node_val)
			UPREF(arg[i]);
		PUSH(arg[i]);
	}

	/* execute the function */
	(void) interpret(code);

	ret = POP_SCALAR();	/* the return value of the function */

	/* restore current rule */
	currule = save_rule;

	/* free code */
	bcfree(code->nexti);
	bcfree(code);

	return ret;
}


/* dlload --- load this library */

NODE *
dlload(NODE *obj, void *dl)
{
	make_old_builtin("bind_array", do_bind_array, 2);
	make_old_builtin("unbind_array", do_unbind_array, 1);
	return make_number((AWKNUM) 0);
}
