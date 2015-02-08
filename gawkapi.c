/*
 * gawkapi.c -- Implement the functions defined for gawkapi.h
 */

/* 
 * Copyright (C) 2012-2015 the Free Software Foundation, Inc.
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

static awk_bool_t node_to_awk_value(NODE *node, awk_value_t *result, awk_valtype_t wanted);

/*
 * api_get_argument --- get the count'th paramater, zero-based.
 *
 * Returns false if count is out of range, or if actual paramater
 * does not match what is specified in wanted. In the latter
 * case, fills in result->val_type with the actual type.
 */

static awk_bool_t
api_get_argument(awk_ext_id_t id, size_t count,
			awk_valtype_t wanted, awk_value_t *result)
{
#ifdef DYNAMIC
	NODE *arg;

	if (result == NULL)
		return awk_false;

	(void) id;

	/* set up default result */
	memset(result, 0, sizeof(*result));
	result->val_type = AWK_UNDEFINED;

	/*
	 * Song and dance here.  get_array_argument() and get_scalar_argument()
	 * will force a change in type of a parameter that is Node_var_new.
	 *
	 * Start by looking at the unadulterated argument as it was passed.
	 */
	arg = get_argument(count);
	if (arg == NULL)
		return awk_false;

	/* if type is undefined */
	if (arg->type == Node_var_new) {
		if (wanted == AWK_UNDEFINED)
			return true;
		else if (wanted == AWK_ARRAY) {
			goto array;
		} else {
			goto scalar;
		}
	}
	
	/* at this point, we have real type */
	if (arg->type == Node_var_array || arg->type == Node_array_ref) {
		if (wanted != AWK_ARRAY && wanted != AWK_UNDEFINED)
			return false;
		goto array;
	} else
		goto scalar;

array:
	/* get the array here */
	arg = get_array_argument(count, false);
	if (arg == NULL)
		return awk_false;

	return node_to_awk_value(arg, result, wanted);

scalar:
	/* at this point we have a real type that is not an array */
	arg = get_scalar_argument(count, false);
	if (arg == NULL)
		return awk_false;

	return node_to_awk_value(arg, result, wanted);
#else
	return awk_false;
#endif
}

/* api_set_argument --- convert an argument to an array */

static awk_bool_t
api_set_argument(awk_ext_id_t id,
		size_t count,
		awk_array_t new_array)
{
#ifdef DYNAMIC
	NODE *arg;
	NODE *array = (NODE *) new_array;

	(void) id;

	if (array == NULL || array->type != Node_var_array)
		return awk_false;

	if (   (arg = get_argument(count)) == NULL
	    || arg->type != Node_var_new)
		return awk_false;

	arg = get_array_argument(count, false);
	if (arg == NULL)
		return awk_false;

	array->vname = arg->vname;
	*arg = *array;
	freenode(array);

	return awk_true;
#else
	return awk_false;
#endif
}

/* awk_value_to_node --- convert a value into a NODE */

NODE *
awk_value_to_node(const awk_value_t *retval)
{
	NODE *ext_ret_val;
	NODE *v;

	if (retval == NULL)
		fatal(_("awk_value_to_node: received null retval"));

	switch (retval->val_type) {
	case AWK_ARRAY:
		ext_ret_val = (NODE *) retval->array_cookie;
		break;
	case AWK_UNDEFINED:
		ext_ret_val = dupnode(Nnull_string);
		break;
	case AWK_NUMBER:
		ext_ret_val = make_number(retval->num_value);
		break;
	case AWK_STRING:
		ext_ret_val = make_str_node(retval->str_value.str,
				retval->str_value.len, ALREADY_MALLOCED);
		break;
	case AWK_SCALAR:
		v = (NODE *) retval->scalar_cookie;
		if (v->type != Node_var)
			ext_ret_val = NULL;
		else
			ext_ret_val = dupnode(v->var_value);
		break;
	case AWK_VALUE_COOKIE:
		ext_ret_val = dupnode((NODE *)(retval->value_cookie));
		break;
	default:	/* any invalid type */
		ext_ret_val = NULL;
		break;
	}

	return ext_ret_val;
}

/* Functions to print messages */

/* api_fatal --- print a fatal message and exit */

static void
api_fatal(awk_ext_id_t id, const char *format, ...)
{
	va_list args;

	(void) id;

	va_start(args, format);
	err(true, _("fatal: "), format, args);
	va_end(args);
}

/* api_warning --- print a warning message and exit */

static void
api_warning(awk_ext_id_t id, const char *format, ...)
{
	va_list args;

	(void) id;

	va_start(args, format);
	err(false, _("warning: "), format, args);
	va_end(args);
}

/* api_lintwarn --- print a lint warning message and exit if appropriate */

static void
api_lintwarn(awk_ext_id_t id, const char *format, ...)
{
	va_list args;

	(void) id;

	va_start(args, format);
	if (lintwarn == r_fatal) {
		err(true, _("fatal: "), format, args);
		va_end(args);
	} else {
		err(false, _("warning: "), format, args);
		va_end(args);
	}
}

/* api_register_input_parser --- register an input_parser; for opening files read-only */

static void
api_register_input_parser(awk_ext_id_t id, awk_input_parser_t *input_parser)
{
	(void) id;

	if (input_parser == NULL)
		return;

	register_input_parser(input_parser);
}

/* api_register_output_wrapper --- egister an output wrapper, for writing files / two-way pipes */

static void api_register_output_wrapper(awk_ext_id_t id,
		awk_output_wrapper_t *output_wrapper)
{
	(void) id;

	if (output_wrapper == NULL)
		return;

	register_output_wrapper(output_wrapper);
}

/* api_register_two_way_processor --- register a processor for two way I/O */

static void
api_register_two_way_processor(awk_ext_id_t id,
		awk_two_way_processor_t *two_way_processor)
{
	(void) id;

	if (two_way_processor == NULL)
		return;

	register_two_way_processor(two_way_processor);
}

/* Functions to update ERRNO */

/* api_update_ERRNO_int --- update ERRNO with an integer value */

static void
api_update_ERRNO_int(awk_ext_id_t id, int errno_val)
{
	(void) id;

	update_ERRNO_int(errno_val);
}

/* api_update_ERRNO_string --- update ERRNO with a string value */

static void
api_update_ERRNO_string(awk_ext_id_t id,
			const char *string)
{
	(void) id;

	if (string == NULL)
		return;

	update_ERRNO_string(string);
}

/* api_unset_ERRNO --- unset ERRNO */

static void
api_unset_ERRNO(awk_ext_id_t id)
{
	(void) id;

	unset_ERRNO();
}


/* api_add_ext_func --- add a function to the interpreter, returns true upon success */

static awk_bool_t
api_add_ext_func(awk_ext_id_t id,
		const char *namespace,
		const awk_ext_func_t *func)
{
	(void) id;
	(void) namespace;

	if (func == NULL)
		return awk_false;

#ifdef DYNAMIC
	return make_builtin(func);
#else
	return awk_false;
#endif
}

/* Stuff for exit handler - do it as linked list */

struct ext_exit_handler {
	struct ext_exit_handler *next;
	void (*funcp)(void *data, int exit_status);
	void *arg0;
};
static struct ext_exit_handler *list_head = NULL;

/* run_ext_exit_handlers --- run the extension exit handlers, LIFO order */

void
run_ext_exit_handlers(int exitval)
{
	struct ext_exit_handler *p, *next;

	for (p = list_head; p != NULL; p = next) {
		next = p->next;
		p->funcp(p->arg0, exitval);
		free(p);
	}
	list_head = NULL;
}

/* api_awk_atexit --- add an exit call back */

static void
api_awk_atexit(awk_ext_id_t id,
		void (*funcp)(void *data, int exit_status),
		void *arg0)
{
	struct ext_exit_handler *p;

	(void) id;

	if (funcp == NULL)
		return;

	/* allocate memory */
	emalloc(p, struct ext_exit_handler *, sizeof(struct ext_exit_handler), "api_awk_atexit");

	/* fill it in */
	p->funcp = funcp;
	p->arg0 = arg0;

	/* add to linked list, LIFO order */
	p->next = list_head;
	list_head = p;
}

/* node_to_awk_value --- convert a node into a value for an extension */

static awk_bool_t
node_to_awk_value(NODE *node, awk_value_t *val, awk_valtype_t wanted)
{
	awk_bool_t ret = awk_false;

	if (node == NULL)
		fatal(_("node_to_awk_value: received null node"));

	if (val == NULL)
		fatal(_("node_to_awk_value: received null val"));

	switch (node->type) {
	case Node_var_new:	/* undefined variable */
		val->val_type = AWK_UNDEFINED;
		if (wanted == AWK_UNDEFINED) {
			ret = awk_true;
		}
		break;

	case Node_var:
		/* a scalar value */
		if (wanted == AWK_SCALAR) {
			val->val_type = AWK_SCALAR;
			val->scalar_cookie = (void *) node;
			ret = awk_true;
			break;
		}

		node = node->var_value;
		/* FALL THROUGH */
	case Node_val:
		/* a scalar value */
		switch (wanted) {
		case AWK_NUMBER:
			val->val_type = AWK_NUMBER;

			(void) force_number(node);
			if ((node->flags & NUMCUR) != 0) {
				val->num_value = get_number_d(node);
				ret = awk_true;
			}
			break;

		case AWK_STRING:
			val->val_type = AWK_STRING;

			(void) force_string(node);
			if ((node->flags & STRCUR) != 0) {
				val->str_value.str = node->stptr;
				val->str_value.len = node->stlen;
				ret = awk_true;
			}
			break;

		case AWK_SCALAR:
			if ((node->flags & NUMBER) != 0) {
				val->val_type = AWK_NUMBER;
			} else if ((node->flags & STRING) != 0) {
				val->val_type = AWK_STRING;
			} else
				val->val_type = AWK_UNDEFINED;
			ret = awk_false;
			break;

		case AWK_UNDEFINED:
			/* return true and actual type for request of undefined */
			if (node == Nnull_string) {
				val->val_type = AWK_UNDEFINED;
				ret = awk_true;
			} else if ((node->flags & NUMBER) != 0) {
				val->val_type = AWK_NUMBER;
				val->num_value = get_number_d(node);
				ret = awk_true;
			} else if ((node->flags & STRING) != 0) {
				val->val_type = AWK_STRING;
				val->str_value.str = node->stptr;
				val->str_value.len = node->stlen;
				ret = awk_true;
			} else
				val->val_type = AWK_UNDEFINED;
			break;

		case AWK_ARRAY:
		case AWK_VALUE_COOKIE:
			break;
		}
		break;

	case Node_var_array:
		val->val_type = AWK_ARRAY;
		if (wanted == AWK_ARRAY || wanted == AWK_UNDEFINED) {
			val->array_cookie = node;
			ret = awk_true;
		} else
			ret = awk_false;
		break;

	default:
		val->val_type = AWK_UNDEFINED;
		ret = awk_false;
		break;
	}

	return ret;
}

/*
 * Symbol table access:
 * 	- No access to special variables (NF, etc.)
 * 	- One special exception: PROCINFO.
 *	- Use sym_update() to change a value, including from UNDEFINED
 *	  to scalar or array. 
 */
/*
 * Lookup a variable, fills in value. No messing with the value
 * returned. Returns false if the variable doesn't exist
 * or the wrong type was requested.
 * In the latter case, fills in vaule->val_type with the real type.
 * Built-in variables (except PROCINFO) may not be accessed by an extension.
 */

/* api_sym_lookup --- look up a symbol */

static awk_bool_t
api_sym_lookup(awk_ext_id_t id,
		const char *name,
		awk_valtype_t wanted,
		awk_value_t *result)
{
	NODE *node;

	update_global_values();		/* make sure stuff like NF, NR, are up to date */

	if (   name == NULL
	    || *name == '\0'
	    || result == NULL
	    || (node = lookup(name)) == NULL)
		return awk_false;

	if (is_off_limits_var(name))	/* a built-in variable */
		node->flags |= NO_EXT_SET;

	return node_to_awk_value(node, result, wanted);
}

/* api_sym_lookup_scalar --- retrieve the current value of a scalar */

static awk_bool_t
api_sym_lookup_scalar(awk_ext_id_t id,
			awk_scalar_t cookie,
			awk_valtype_t wanted,
			awk_value_t *result)
{
	NODE *node = (NODE *) cookie;

	if (node == NULL
	    || result == NULL
	    || node->type != Node_var)
		return awk_false;

	update_global_values();	/* make sure stuff like NF, NR, are up to date */

	return node_to_awk_value(node, result, wanted);
}

/* api_sym_update --- update a symbol's value, see gawkapi.h for semantics */

static awk_bool_t
api_sym_update(awk_ext_id_t id,
		const char *name,
		awk_value_t *value)
{
	NODE *node;
	NODE *array_node;

	if (   name == NULL
	    || *name == '\0'
	    || value == NULL)
		return awk_false;

	switch (value->val_type) {
	case AWK_NUMBER:
	case AWK_STRING:
	case AWK_UNDEFINED:
	case AWK_ARRAY:
	case AWK_SCALAR:
	case AWK_VALUE_COOKIE:
		break;

	default:
		/* fatal(_("api_sym_update: invalid value for type of new value (%d)"), value->val_type); */
		return awk_false;
	}

	node = lookup(name);

	if (node == NULL) {
		/* new value to be installed */
		if (value->val_type == AWK_ARRAY) {
			array_node = awk_value_to_node(value);
			node = install_symbol(estrdup((char *) name, strlen(name)),
					Node_var_array);
			array_node->vname = node->vname;
			*node = *array_node;
			freenode(array_node);
			value->array_cookie = node;	/* pass new cookie back to extension */
		} else {
			/* regular variable */
			node = install_symbol(estrdup((char *) name, strlen(name)),
					Node_var);
			node->var_value = awk_value_to_node(value);
		}

		return awk_true;
	}

	/*
	 * If we get here, then it exists already.  Any valid type is
	 * OK except for AWK_ARRAY.
	 */
	if (   (node->flags & NO_EXT_SET) != 0
	    || is_off_limits_var(name)) {	/* most built-in vars not allowed */
		node->flags |= NO_EXT_SET;
		return awk_false;
	}

	if (    value->val_type != AWK_ARRAY
	    && (node->type == Node_var || node->type == Node_var_new)) {
		unref(node->var_value);
		node->var_value = awk_value_to_node(value);
		if (node->type == Node_var_new && value->val_type != AWK_UNDEFINED)
			node->type = Node_var;

		return awk_true;
	}

	return awk_false;
}

/* api_sym_update_scalar --- update a scalar cookie */

static awk_bool_t
api_sym_update_scalar(awk_ext_id_t id,
			awk_scalar_t cookie,
			awk_value_t *value)
{
	NODE *node = (NODE *) cookie;

	if (value == NULL
	    || node == NULL
	    || node->type != Node_var
	    || (node->flags & NO_EXT_SET) != 0)
		return awk_false;

	/*
	 * Optimization: if valref is 1, and the new value is a string or
	 * a number, we can avoid calling unref and then making a new node
	 * by simply installing the new value.  First, we follow the same
	 * recipe used by node.c:r_unref to wipe the current values, and then
	 * we copy the logic from r_make_number or make_str_node to install
	 * the new value.
	 */
	switch (value->val_type) {
	case AWK_NUMBER:
		if (node->var_value->valref == 1 && ! do_mpfr) {
			NODE *r = node->var_value;

			/* r_unref: */
			if ((r->flags & (MALLOC|STRCUR)) == (MALLOC|STRCUR))
				efree(r->stptr);
			free_wstr(r);

			/* r_make_number: */
			r->numbr = value->num_value;
			r->flags = MALLOC|NUMBER|NUMCUR;
			r->stptr = NULL;
			r->stlen = 0;
			return awk_true;
		}
		break;
	case AWK_STRING:
		if (node->var_value->valref == 1) {
			NODE *r = node->var_value;

			/* r_unref: */
			if ((r->flags & (MALLOC|STRCUR)) == (MALLOC|STRCUR))
				efree(r->stptr);

			mpfr_unset(r);
			free_wstr(r);

			/* make_str_node(s, l, ALREADY_MALLOCED): */
			r->numbr = 0;
			r->flags = (MALLOC|STRING|STRCUR);
			r->stfmt = -1;
			r->stptr = value->str_value.str;
			r->stlen = value->str_value.len;
			return awk_true;
		}
		break;
	case AWK_UNDEFINED:
	case AWK_SCALAR:
	case AWK_VALUE_COOKIE:
		break;

	default:	/* AWK_ARRAY or invalid type */
		return awk_false;
	}

	/* do it the hard (slow) way */
	unref(node->var_value);
	node->var_value = awk_value_to_node(value);
	return awk_true;
}

/*
 * valid_subscript_type --- test if a type is allowed for an array subscript.
 *
 * Any scalar value is fine, so only AWK_ARRAY (or an invalid type) is illegal.
 */

static inline bool
valid_subscript_type(awk_valtype_t valtype)
{
	switch (valtype) {
	case AWK_UNDEFINED:
	case AWK_NUMBER:
	case AWK_STRING:
	case AWK_SCALAR:
	case AWK_VALUE_COOKIE:
		return true;
	default:	/* AWK_ARRAY or an invalid type */
		return false;
	}
}

/* Array management */
/*
 * api_get_array_element --- teturn the value of an element - read only!
 *
 * Use set_array_element to change it.
 */

static awk_bool_t
api_get_array_element(awk_ext_id_t id,
		awk_array_t a_cookie,
		const awk_value_t *const index,
		awk_valtype_t wanted,
		awk_value_t *result)
{
	NODE *array = (NODE *) a_cookie;
	NODE *subscript;
	NODE **aptr;

	/* don't check for index len zero, null str is ok as index */
	if (   array == NULL
	    || array->type != Node_var_array
	    || result == NULL
	    || index == NULL
	    || ! valid_subscript_type(index->val_type))
		return awk_false;

	subscript = awk_value_to_node(index);

	/* if it doesn't exist, return false */
	if (in_array(array, subscript) == NULL) {
		unref(subscript);
		return awk_false;
	}

	aptr = assoc_lookup(array, subscript);

	if (aptr == NULL) {	/* can't happen */
		unref(subscript);
		return awk_false;
	}

	unref(subscript);

	return node_to_awk_value(*aptr, result, wanted);
}

/*
 * api_set_array_element --- change (or create) element in existing array
 *	with element->index and element->value.
 */

static awk_bool_t
api_set_array_element(awk_ext_id_t id, awk_array_t a_cookie,
					const awk_value_t *const index,
					const awk_value_t *const value)
{
	NODE *array = (NODE *)a_cookie;
	NODE *tmp;
	NODE *elem;
	NODE **aptr;

	/* don't check for index len zero, null str is ok as index */
	if (   array == NULL
	    || array->type != Node_var_array
	    || (array->flags & NO_EXT_SET) != 0
	    || index == NULL
	    || value == NULL
	    || ! valid_subscript_type(index->val_type))
		return awk_false;

	tmp = awk_value_to_node(index);
	aptr = assoc_lookup(array, tmp);
	unref(tmp);
	unref(*aptr);
	elem = *aptr = awk_value_to_node(value);
	if (elem->type == Node_var_array) {
		elem->parent_array = array;
		elem->vname = estrdup(index->str_value.str,
					index->str_value.len);
	}

	return awk_true;
}

/*
 * remove_element --- remove an array element 
 *		common code used by multiple functions
 */

static void
remove_element(NODE *array, NODE *subscript)
{
	NODE *val;

	if (array == NULL)
		fatal(_("remove_element: received null array"));

	if (subscript == NULL)
		fatal(_("remove_element: received null subscript"));

	val = in_array(array, subscript);

	if (val == NULL)
		return;

	if (val->type == Node_var_array) {
		assoc_clear(val);
		/* cleared a sub-array, free Node_var_array */
		efree(val->vname);
		freenode(val);
	} else
		unref(val);

	(void) assoc_remove(array, subscript);
}

/*
 * api_del_array_element --- remove the element with the given index.
 *	Return success if removed or if element did not exist.
 */

static awk_bool_t
api_del_array_element(awk_ext_id_t id,
		awk_array_t a_cookie, const awk_value_t* const index)
{
	NODE *array, *sub;

	array = (NODE *) a_cookie;
	if (   array == NULL
	    || array->type != Node_var_array
	    || (array->flags & NO_EXT_SET) != 0
	    || index == NULL
	    || ! valid_subscript_type(index->val_type))
		return awk_false;

	sub = awk_value_to_node(index);
	remove_element(array, sub);
	unref(sub);

	return awk_true;
}

/*
 * api_get_element_count --- retrieve total number of elements in array.
 *	Return false if some kind of error.
 */

static awk_bool_t
api_get_element_count(awk_ext_id_t id,
		awk_array_t a_cookie, size_t *count)
{
	NODE *node = (NODE *) a_cookie;

	if (count == NULL || node == NULL || node->type != Node_var_array)
		return awk_false;

	*count = node->table_size;
	return awk_true;
}

/* api_create_array --- create a new array cookie to which elements may be added */

static awk_array_t
api_create_array(awk_ext_id_t id)
{
	NODE *n;

	getnode(n);
	memset(n, 0, sizeof(NODE));
	null_array(n);

	return (awk_array_t) n;
}

/* api_clear_array --- clear out an array */

static awk_bool_t
api_clear_array(awk_ext_id_t id, awk_array_t a_cookie)
{
	NODE *node = (NODE *) a_cookie;

	if (   node == NULL
	    || node->type != Node_var_array
	    || (node->flags & NO_EXT_SET) != 0)
		return awk_false;

	assoc_clear(node);
	return awk_true;
}

/* api_flatten_array --- flatten out an array so that it can be looped over easily. */

static awk_bool_t
api_flatten_array(awk_ext_id_t id,
		awk_array_t a_cookie,
		awk_flat_array_t **data)
{
	NODE **list;
	size_t i, j;
	NODE *array = (NODE *) a_cookie;
	size_t alloc_size;

	if (   array == NULL
	    || array->type != Node_var_array
	    || array->table_size == 0
	    || data == NULL)
		return awk_false;

	alloc_size = sizeof(awk_flat_array_t) +
			(array->table_size - 1) * sizeof(awk_element_t);

	emalloc(*data, awk_flat_array_t *, alloc_size,
			"api_flatten_array");
	memset(*data, 0, alloc_size);

	list = assoc_list(array, "@unsorted", ASORTI);

	(*data)->opaque1 = array;
	(*data)->opaque2 = list;
	(*data)->count = array->table_size;

	for (i = j = 0; i < 2 * array->table_size; i += 2, j++) {
		NODE *index, *value;

		index = list[i];
		value = list[i + 1]; /* number or string or subarray */

		/*
		 * Convert index and value to ext types.  Force the
		 * index to be a string, since indices are always
		 * conceptually strings, regardless of internal optimizations
		 * to treat them as integers in some cases.
		 */
		if (! node_to_awk_value(index,
				& (*data)->elements[j].index, AWK_STRING)) {
			fatal(_("api_flatten_array: could not convert index %d\n"),
						(int) i);
		}
		if (! node_to_awk_value(value,
				& (*data)->elements[j].value, AWK_UNDEFINED)) {
			fatal(_("api_flatten_array: could not convert value %d\n"),
						(int) i);
		}
	}
	return awk_true;
}

/*
 * api_release_flattened_array --- release array memory,
 *	delete any marked elements. Count must match what
 *	gawk thinks the size is.
 */

static awk_bool_t
api_release_flattened_array(awk_ext_id_t id,
		awk_array_t a_cookie,
		awk_flat_array_t *data)
{
	NODE *array = a_cookie;
	NODE **list;
	size_t i, j, k;

	if (   array == NULL
	    || array->type != Node_var_array
	    || data == NULL
	    || array != (NODE *) data->opaque1
	    || data->count != array->table_size
	    || data->opaque2 == NULL)
		return awk_false;

	list = (NODE **) data->opaque2;

	/* free index nodes */
	for (i = j = 0, k = 2 * array->table_size; i < k; i += 2, j++) {
		/* Delete items flagged for delete. */
		if (   (data->elements[j].flags & AWK_ELEMENT_DELETE) != 0
		    && (array->flags & NO_EXT_SET) == 0) {
			remove_element(array, list[i]);
		}
		unref(list[i]);
	}

	efree(list);
	efree(data);

	return awk_true;
}

/* api_create_value --- create a cached value */

static awk_bool_t
api_create_value(awk_ext_id_t id, awk_value_t *value,
		awk_value_cookie_t *result)
{
	if (value == NULL || result == NULL)
		return awk_false;

	switch (value->val_type) {
	case AWK_NUMBER:
	case AWK_STRING:
		break;
	default:
		/* reject anything other than a simple scalar */
		return awk_false;
	}

	return (awk_bool_t) ((*result = awk_value_to_node(value)) != NULL);
}

/* api_release_value --- release a cached value */

static awk_bool_t
api_release_value(awk_ext_id_t id, awk_value_cookie_t value)
{
	NODE *val = (NODE *) value;

	if (val == NULL)
		return awk_false;

	unref(val);
	return awk_true;
}

/*
 * Register a version string for this extension with gawk.
 */

struct version_info {
	const char *version;
	struct version_info *next;
};

static struct version_info *vi_head;

/* api_register_ext_version --- add an extension version string to the list */

static void
api_register_ext_version(awk_ext_id_t id, const char *version)
{
	struct version_info *info;

	if (version == NULL)
		return;

	(void) id;

	emalloc(info, struct version_info *, sizeof(struct version_info), "register_ext_version");
	info->version = version;
	info->next = vi_head;
	vi_head = info;
}

/* the struct api */
gawk_api_t api_impl = {
	/* data */
	GAWK_API_MAJOR_VERSION,	/* major and minor versions */
	GAWK_API_MINOR_VERSION,
	{ 0 },			/* do_flags */

	/* registration functions */
	api_add_ext_func,
	api_register_input_parser,
	api_register_output_wrapper,
	api_register_two_way_processor,
	api_awk_atexit,
	api_register_ext_version,

	/* message printing functions */
	api_fatal,
	api_warning,
	api_lintwarn,

	/* updating ERRNO */
	api_update_ERRNO_int,
	api_update_ERRNO_string,
	api_unset_ERRNO,

	/* Function arguments */
	api_get_argument,
	api_set_argument,

	/* Accessing and installing variables and constants */
	api_sym_lookup,
	api_sym_update,

	/* Accessing and modifying variables via scalar cookies */
	api_sym_lookup_scalar,
	api_sym_update_scalar,

	/* Cached values */
	api_create_value,
	api_release_value,

	/* Array management */
	api_get_element_count,
	api_get_array_element,
	api_set_array_element,
	api_del_array_element,
	api_create_array,
	api_clear_array,
	api_flatten_array,
	api_release_flattened_array,

	/* Memory allocation */
	malloc,
	calloc,
	realloc,
	free,
};

/* init_ext_api --- init the extension API */

void
init_ext_api()
{
	/* force values to 1 / 0 */
	api_impl.do_flags[0] = (do_lint ? 1 : 0);
	api_impl.do_flags[1] = (do_traditional ? 1 : 0);
	api_impl.do_flags[2] = (do_profile ? 1 : 0);
	api_impl.do_flags[3] = (do_sandbox ? 1 : 0);
	api_impl.do_flags[4] = (do_debug ? 1 : 0);
	api_impl.do_flags[5] = (do_mpfr ? 1 : 0);
}

/* update_ext_api --- update the variables in the API that can change */

void
update_ext_api()
{
	api_impl.do_flags[0] = (do_lint ? 1 : 0);
}

/* print_ext_versions --- print the list */

extern void
print_ext_versions(void)
{
	struct version_info *p;

	for (p = vi_head; p != NULL; p = p->next)
		printf("%s\n", p->version);
}
