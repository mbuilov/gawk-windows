/*
 * spec_array.c - Support for specialized associative arrays.
 */

/*
 * Copyright (C) 2012, 2014 the Free Software Foundation, Inc.
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

typedef struct spec_array {
	Fetch_func_t fetch_func;
	Store_func_t store_func;
	Load_func_t load_func;
	void *data;
} array_t;

/*
 * The array_t structure is attached to the array itself without
 * the necessity to maintain a list of symbols; this works only
 * because there is just enough free space in the NODE strcture when
 * the base array is str_array.
 */

#define	SUPER(F)	(*str_array_func[F ## _ind])


/*
 * deferred_array --- Deferred loading of array at run-time.
 *
 * The load routine takes two arguments, the array and
 * a void * data:
 *
 *	void load_func(NODE *array, void *data)
 *
 * Use register_deferred_array(array, load_func, void *data) to
 * bind an array to the load routine.
 */

static NODE **deferred_array_init(NODE *, NODE *);
static NODE **deferred_array_lookup(NODE *, NODE *);
static NODE **deferred_array_exists(NODE *, NODE *);
static NODE **deferred_array_remove(NODE *, NODE *);
static NODE **deferred_array_clear(NODE *, NODE *);
static NODE **deferred_array_list(NODE *, NODE *);
static NODE **deferred_array_copy(NODE *, NODE *);
static NODE **deferred_array_length(NODE *, NODE *);

static afunc_t deferred_array_func[] = {
	deferred_array_init,
	(afunc_t) 0,	/* typeof */
	deferred_array_length,
	deferred_array_lookup,
	deferred_array_exists,
	deferred_array_clear,
	deferred_array_remove,
	deferred_array_list,
	deferred_array_copy,
	null_afunc,	/* dump */
	(afunc_t) 0,	/* store */
};


/* deferred_array_init --- called when array becomes empty, e.g: delete BOUND_ARRAY */

static NODE **
deferred_array_init(NODE *symbol, NODE *subs)
{
	if (symbol != NULL) {
		array_t *av = (array_t *) symbol->xarray;
		symbol->xarray = NULL;	/* this is to avoid an assertion failure in null_array */
		null_array(symbol);	/* typeless empty array */
		if (symbol->parent_array == NULL) {
			/* main array */
			symbol->array_funcs = deferred_array_func;	/* restore type */
			symbol->xarray = (NODE *) av;
		} else if (av)	/* sub-array */
			efree(av);
	}
	return NULL;
}

/* deferred_array_length --- get the length of the array */

static NODE **
deferred_array_length(NODE *symbol, NODE *subs)
{
	static NODE *length_node;
	array_t *av = (array_t *) symbol->xarray;
	if (av) {
		symbol->xarray = NULL;
		(*av->load_func)(symbol, av->data);
		symbol->xarray = (NODE *) av;
	}
	length_node = symbol;
	return & length_node;
}

#define DEF_ARR(F) static NODE **			\
deferred_array_##F(NODE *symbol, NODE *subs)		\
{							\
	array_t *av = (array_t *) symbol->xarray;	\
	if (av) {					\
		symbol->xarray = NULL;			\
		(*av->load_func)(symbol, av->data);	\
		symbol->xarray = (NODE *) av;		\
	}						\
	return SUPER(a##F)(symbol, subs);		\
}

/* the rest of the routines */

DEF_ARR(exists)
DEF_ARR(lookup)
DEF_ARR(list)
DEF_ARR(copy)

#undef DEF_ARR

/* deferred_array_remove --- remove the index from the array */

static NODE **
deferred_array_remove(NODE *symbol, NODE *subs)
{
	array_t *av = (array_t *) symbol->xarray;

	(void) SUPER(aremove)(symbol, subs);
	if (av) {
		symbol->xarray = NULL;
		(*av->load_func)(symbol, av->data);
		symbol->xarray = (NODE *) av;
	}
	return NULL;
}

/* deferred_array_clear --- flush all the values in symbol[] */

static NODE **
deferred_array_clear(NODE *symbol, NODE *subs)
{
	array_t *av = (array_t *) symbol->xarray;

	(void) SUPER(aclear)(symbol, subs);
	if (av) {
		symbol->xarray = NULL;
		(*av->load_func)(symbol, av->data);
		symbol->xarray = (NODE *) av;
	}
	return NULL;
}


/*
 * dyn_array --- array with triggers for reading and writing
 * 	an element.
 *
 * The fetch routine should expect three arguments, the array,
 * the subscript and optional void * data. It should return the value
 * if it exists or NULL otherwise.
 *
 * 	NODE *fetch_func(NODE *array, NODE *subs, void *data)
 *
 * The store routine must take an additional argument for the
 * value. The value can be NULL if the specific element is
 * removed from the array. The subscript (and the value) is NULL
 * when the entire array is deleted.
 *
 * 	void store_func(NODE *array, NODE *subs, NODE *value, void *data)
 *
 * Use register_dyn_array(array, fetch_func, store_func, void *data) to
 * bind an array to the fetch/store routine.
 */


static NODE **dyn_array_init(NODE *, NODE *);
static NODE **dyn_array_lookup(NODE *, NODE *);
static NODE **dyn_array_exists(NODE *, NODE *);
static NODE **dyn_array_remove(NODE *, NODE *);
static NODE **dyn_array_clear(NODE *, NODE *);
static NODE **dyn_array_list(NODE *, NODE *);
static NODE **dyn_array_copy(NODE *, NODE *);
static NODE **dyn_array_store(NODE *, NODE *);

static afunc_t dyn_array_func[] = {
	dyn_array_init,
	(afunc_t) 0,	/* typeof */
	null_length,	/* length */
	dyn_array_lookup,
	dyn_array_exists,
	dyn_array_clear,
	dyn_array_remove,
	dyn_array_list,
	dyn_array_copy,
	null_afunc,	/* dump */
	dyn_array_store,
};

/* dyn_array_init --- called when array becomes empty */

static NODE **
dyn_array_init(NODE *symbol, NODE *subs)
{
	if (symbol != NULL) {
		array_t *av = (array_t *) symbol->xarray;
		symbol->xarray = NULL;
		null_array(symbol);	/* typeless empty array */
		if (symbol->parent_array == NULL) {
			/* main array */
			symbol->array_funcs = dyn_array_func;	/* restore type */
			symbol->xarray = (NODE *) av;
		} else if (av)	/* sub-array */
			efree(av);
	}
	return NULL;
}

/* dyn_array_exists --- check if the SUBS exists */

static NODE **
dyn_array_exists(NODE *symbol, NODE *subs)
{
	NODE *r;
	array_t *av = (array_t *) symbol->xarray;

	if (av && av->fetch_func) {
		symbol->xarray = NULL;
		r = (*av->fetch_func)(symbol, subs, av->data);
		symbol->xarray = (NODE *) av;
		if (r != NULL) {
			NODE **lhs;
			lhs = SUPER(alookup)(symbol, subs);
			unref(*lhs);
			*lhs = r;
			return lhs;
		}
	}

	return SUPER(aexists)(symbol, subs);
}

/* dyn_array_lookup --- lookup SUBS and return a pointer to store its value */

static NODE **
dyn_array_lookup(NODE *symbol, NODE *subs)
{
	NODE **lhs;
	NODE *r;
	array_t *av = (array_t *) symbol->xarray;

	lhs = SUPER(alookup)(symbol, subs);
	if (av && av->fetch_func) {
		symbol->xarray = NULL;
		r = (*av->fetch_func)(symbol, subs, av->data);
		symbol->xarray = (NODE *) av;
		if (r != NULL) {
			unref(*lhs);
			*lhs = r;
		}
	}
	return lhs;
}

/* dyn_array_store --- call the store routine after an assignment */

static NODE **
dyn_array_store(NODE *symbol, NODE *subs)
{
	array_t *av = (array_t *) symbol->xarray;

	if (av && av->store_func) {
		NODE **lhs;
		lhs = SUPER(aexists)(symbol, subs);
		symbol->xarray = NULL;
		(*av->store_func)(symbol, subs, *lhs, av->data);
		symbol->xarray = (NODE *) av;
	}
	return NULL;
}

/* dyn_array_remove --- remove the index from the array */

static NODE **
dyn_array_remove(NODE *symbol, NODE *subs)
{
	array_t *av = (array_t *) symbol->xarray;

	(void) SUPER(aremove)(symbol, subs);
	if (av && av->store_func) {
		symbol->xarray = NULL;
		(*av->store_func)(symbol, subs, NULL, av->data);
		symbol->xarray = (NODE *) av;
	}
	return NULL;
}

/* dyn_array_clear --- flush all the values in symbol[] */

static NODE **
dyn_array_clear(NODE *symbol, NODE *subs)
{
	array_t *av = (array_t *) symbol->xarray;

	(void) SUPER(aclear)(symbol, subs);
	if (av && av->store_func) {
		symbol->xarray = NULL;
		(*av->store_func)(symbol, NULL, NULL, av->data);
		symbol->xarray = (NODE *) av;
	}
	return NULL;
}

/* dyn_array_list --- return a list of items in symbol[] */

static NODE **
dyn_array_list(NODE *symbol, NODE *subs)
{
	return SUPER(alist)(symbol, subs);
}

/* dyn_array_copy --- duplicate the array */

static NODE **
dyn_array_copy(NODE *symbol, NODE *subs)
{
	return SUPER(acopy)(symbol, subs);
}

/* register_array_s --- attach the specified routine(s) to an array */

static void
register_array_s(NODE *symbol, Fetch_func_t fetch_func,
			Store_func_t store_func, Load_func_t load_func, void *data)
{
	array_t *av;

	if (symbol->type != Node_var_array)
		fatal(_("register_array_s: argument is not an array"));

	if (symbol->array_funcs == deferred_array_func
			|| symbol->array_funcs == dyn_array_func)
		fatal(_("register_array_s: `%s' already is a deferred/dyn array"),
			array_vname(symbol));

	assoc_clear(symbol);
	assert(symbol->xarray == NULL);
	emalloc(av, array_t *, sizeof (array_t), "register_spec_array");
	av->fetch_func = fetch_func;
	av->store_func = store_func;
	av->load_func = load_func;
	av->data = data;
	symbol->xarray = (NODE *) av;
}

/* register_deferred_array --- make the array to be loaded at run-time */

void
register_deferred_array(NODE *symbol, Load_func_t load_func, void *dq)
{
	if (! load_func)
		fatal(_("register_deferred_array: null load function"));
	register_array_s(symbol, 0, 0, load_func, dq);
	symbol->array_funcs = deferred_array_func;
}

/* register_dyn_array --- attach read and write triggers to an array */

void
register_dyn_array(NODE *symbol, Fetch_func_t fetch_func,
		Store_func_t store_func, void *dq)
{
	register_array_s(symbol, fetch_func, store_func, 0, dq);
	symbol->array_funcs = dyn_array_func;
}

/* unregister_array_s --- un-special the array */

void *
unregister_array_s(NODE *symbol)
{
	void *data = NULL;
	if (symbol->type != Node_var_array)
		fatal(_("unregister_array_s: argument is not an array"));

	if (symbol->array_funcs == dyn_array_func
		|| symbol->array_funcs == deferred_array_func
	) {
		array_t *av;

		av = (array_t *) symbol->xarray;
		assert(av != NULL);
		data = av->data;
		efree(av);
		symbol->array_funcs = str_array_func;
		symbol->xarray = NULL;
		/* FIXME: do we assoc_clear the array ? */
	}
	return data;
}
