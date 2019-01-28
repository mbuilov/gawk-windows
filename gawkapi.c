/*
 * gawkapi.c -- Implement the functions defined for gawkapi.h
 */

/*
 * Copyright (C) 2012-2019 the Free Software Foundation, Inc.
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

#ifdef HAVE_MPFR
#define getmpfr(n)	getblock(n, BLOCK_MPFR, mpfr_ptr)
#define freempfr(n)	freeblock(n, BLOCK_MPFR)

#define getmpz(n)	getblock(n, BLOCK_MPZ, mpz_ptr)
#define freempz(n)	freeblock(n, BLOCK_MPZ)
#endif

/* Declare some globals used by api_get_file: */
extern IOBUF *curfile;
extern INSTRUCTION *main_beginfile;
extern int currule;

static awk_bool_t node_to_awk_value(NODE *node, awk_value_t *result, awk_valtype_t wanted);
static char *valtype2str(awk_valtype_t type);
static NODE *ns_lookup(const char *name_space, const char *name, char **full_name);

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
	arg = get_array_argument(arg, count);
	if (arg == NULL)
		return awk_false;

	return node_to_awk_value(arg, result, wanted);

scalar:
	/* at this point we have a real type that is not an array */
	arg = get_scalar_argument(arg, count);
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

	arg = get_array_argument(arg, count);
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
	NODE *ext_ret_val = NULL;
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
		switch (retval->num_type) {
		case AWK_NUMBER_TYPE_DOUBLE:
			ext_ret_val = make_number(retval->num_value);
			break;
		case AWK_NUMBER_TYPE_MPFR:
#ifdef HAVE_MPFR
			if (! do_mpfr)
				fatal(_("awk_value_to_node: not in MPFR mode"));
			ext_ret_val = make_number_node(MPFN);
			memcpy(&ext_ret_val->mpg_numbr, retval->num_ptr, sizeof(ext_ret_val->mpg_numbr));
			freempfr(retval->num_ptr);
#else
			fatal(_("awk_value_to_node: MPFR not supported"));
#endif
			break;
		case AWK_NUMBER_TYPE_MPZ:
#ifdef HAVE_MPFR
			if (! do_mpfr)
				fatal(_("awk_value_to_node: not in MPFR mode"));
			ext_ret_val = make_number_node(MPZN);
			memcpy(&ext_ret_val->mpg_i, retval->num_ptr, sizeof(ext_ret_val->mpg_i));
			freempz(retval->num_ptr);
#else
			fatal(_("awk_value_to_node: MPFR not supported"));
#endif
			break;
		default:
			fatal(_("awk_value_to_node: invalid number type `%d'"), retval->num_type);
			break;
		}
		break;
	case AWK_STRING:
		ext_ret_val = make_str_node(retval->str_value.str,
				retval->str_value.len, ALREADY_MALLOCED);
		break;
	case AWK_STRNUM:
		ext_ret_val = make_str_node(retval->str_value.str,
				retval->str_value.len, ALREADY_MALLOCED);
		ext_ret_val->flags |= USER_INPUT;
		break;
	case AWK_REGEX:
		ext_ret_val = make_typed_regex(retval->str_value.str,
				retval->str_value.len);
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

/* api_nonfatal --- print a non fatal error message */

static void
api_nonfatal(awk_ext_id_t id, const char *format, ...)
{
	va_list args;

	(void) id;

	va_start(args, format);
	err(false, _("error: "), format, args);
	va_end(args);
}

/* api_warning --- print a warning message */

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
	} else {
		err(false, _("warning: "), format, args);
	}
	va_end(args);
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

/* api_register_output_wrapper --- register an output wrapper, for writing files / two-way pipes */

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
		const char *name_space,
		awk_ext_func_t *func)
{
	(void) id;

	if (func == NULL)
		return awk_false;

	if (name_space == NULL)
		fatal(_("add_ext_func: received NULL name_space parameter"));

#ifdef DYNAMIC
	return make_builtin(name_space, func);
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

static struct {
	char **strings;
	size_t i, size;
} scopy;

/* free_api_string_copies --- release memory used by string copies */

void
free_api_string_copies()
{
	size_t i;

	for (i = 0; i < scopy.i; i++)
		free(scopy.strings[i]);
	scopy.i = 0;
}

/* assign_string --- return a string node with NUL termination */

static inline void
assign_string(NODE *node, awk_value_t *val, awk_valtype_t val_type)
{
	val->val_type = val_type;
	if (node->stptr[node->stlen] != '\0') {
		/*
		 * This is an unterminated field string, so make a copy.
		 * This should happen only for $n where n > 0 and n < NF.
		 */
		char *s;

		assert((node->flags & MALLOC) == 0);
		if (scopy.i == scopy.size) {
			/* expand list */
			if (scopy.size == 0)
				scopy.size = 8;	/* initial size */
			else
				scopy.size *= 2;
			erealloc(scopy.strings, char **, scopy.size * sizeof(char *), "assign_string");
		}
		emalloc(s, char *, node->stlen + 1, "assign_string");
		memcpy(s, node->stptr, node->stlen);
		s[node->stlen] = '\0';
		val->str_value.str = scopy.strings[scopy.i++] = s;
	}
	else
		val->str_value.str = node->stptr;
	val->str_value.len = node->stlen;
}

/* assign_number -- return a number node */

#define assign_double(val) \
	val->num_value = node->numbr; \
	val->num_type = AWK_NUMBER_TYPE_DOUBLE; \
	val->num_ptr = NULL

static inline void
assign_number(NODE *node, awk_value_t *val)
{
	val->val_type = AWK_NUMBER;

#ifndef HAVE_MPFR
	assign_double(val);
#else
	switch (node->flags & (MPFN|MPZN)) {
	case 0:
		assign_double(val);
		break;
	case MPFN:
		val->num_value = mpfr_get_d(node->mpg_numbr, ROUND_MODE);
		val->num_type = AWK_NUMBER_TYPE_MPFR;
		val->num_ptr = &node->mpg_numbr;
		break;
	case MPZN:
		val->num_value = mpz_get_d(node->mpg_i);
		val->num_type = AWK_NUMBER_TYPE_MPZ;
		val->num_ptr = &node->mpg_i;
		break;
	default:
		fatal(_("node_to_awk_value: detected invalid numeric flags combination `%s'; please file a bug report."), flags2str(node->flags));
		break;
	}
#endif
}
#undef assign_double

/* assign_regex --- return a regex node */

static inline void
assign_regex(NODE *node, awk_value_t *val)
{
	/* a REGEX node cannot be an unterminated field string */
	assert((node->flags & MALLOC) != 0);
	assert(node->stptr[node->stlen] == '\0');
	val->str_value.str = node->stptr;
	val->str_value.len = node->stlen;
	val->val_type = AWK_REGEX;
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
			if (node->flags & REGEX)
				val->val_type = AWK_REGEX;
			else {
				(void) force_number(node);
				assign_number(node, val);
				ret = awk_true;
			}
			break;

		case AWK_STRNUM:
			switch (fixtype(node)->flags & (STRING|NUMBER|USER_INPUT|REGEX)) {
			case STRING:
				val->val_type = AWK_STRING;
				break;
			case NUMBER:
				(void) force_string(node);
				/* fall through */
			case NUMBER|USER_INPUT:
				assign_string(node, val, AWK_STRNUM);
				ret = awk_true;
				break;
			case REGEX:
				val->val_type = AWK_REGEX;
				break;
			case NUMBER|STRING:
				if (node == Nnull_string) {
					val->val_type = AWK_UNDEFINED;
					break;
				}
				/* fall through */
			default:
				warning(_("node_to_awk_value detected invalid flags combination `%s'; please file a bug report."), flags2str(node->flags));
				val->val_type = AWK_UNDEFINED;
				break;
			}
			break;

		case AWK_STRING:
			(void) force_string(node);
			assign_string(node, val, AWK_STRING);
			ret = awk_true;
			break;

		case AWK_REGEX:
			switch (fixtype(node)->flags & (STRING|NUMBER|USER_INPUT|REGEX)) {
			case STRING:
				val->val_type = AWK_STRING;
				break;
			case NUMBER:
				val->val_type = AWK_NUMBER;
				break;
			case NUMBER|USER_INPUT:
				val->val_type = AWK_STRNUM;
				break;
			case REGEX:
				assign_regex(node, val);
				ret = awk_true;
				break;
			case NUMBER|STRING:
				if (node == Nnull_string) {
					val->val_type = AWK_UNDEFINED;
					break;
				}
				/* fall through */
			default:
				warning(_("node_to_awk_value detected invalid flags combination `%s'; please file a bug report."), flags2str(node->flags));
				val->val_type = AWK_UNDEFINED;
				break;
			}
			break;

		case AWK_SCALAR:
			switch (fixtype(node)->flags & (STRING|NUMBER|USER_INPUT|REGEX)) {
			case STRING:
				val->val_type = AWK_STRING;
				break;
			case NUMBER:
				val->val_type = AWK_NUMBER;
				break;
			case NUMBER|USER_INPUT:
				val->val_type = AWK_STRNUM;
				break;
			case REGEX:
				val->val_type = AWK_REGEX;
				break;
			case NUMBER|STRING:
				if (node == Nnull_string) {
					val->val_type = AWK_UNDEFINED;
					break;
				}
				/* fall through */
			default:
				warning(_("node_to_awk_value detected invalid flags combination `%s'; please file a bug report."), flags2str(node->flags));
				val->val_type = AWK_UNDEFINED;
				break;
			}
			break;

		case AWK_UNDEFINED:
			/* return true and actual type for request of undefined */
			switch (fixtype(node)->flags & (STRING|NUMBER|USER_INPUT|REGEX)) {
			case STRING:
				assign_string(node, val, AWK_STRING);
				ret = awk_true;
				break;
			case NUMBER:
				assign_number(node, val);
				ret = awk_true;
				break;
			case NUMBER|USER_INPUT:
				assign_string(node, val, AWK_STRNUM);
				ret = awk_true;
				break;
			case REGEX:
				assign_regex(node, val);
				ret = awk_true;
				break;
			case NUMBER|STRING:
				if (node == Nnull_string) {
					val->val_type = AWK_UNDEFINED;
					ret = awk_true;
					break;
				}
				/* fall through */
			default:
				warning(_("node_to_awk_value detected invalid flags combination `%s'; please file a bug report."), flags2str(node->flags));
				val->val_type = AWK_UNDEFINED;
				break;
			}
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
		const char *name_space,
		const char *name,
		awk_valtype_t wanted,
		awk_value_t *result)
{
	NODE *node;

	update_global_values();		/* make sure stuff like NF, NR, are up to date */

	if (   name == NULL
	    || *name == '\0'
	    || result == NULL
	    || ! is_valid_identifier(name)
	    || name_space == NULL
	    || (name_space[0] != '\0' && ! is_valid_identifier(name_space)))
		return awk_false;

	if ((node = ns_lookup(name_space, name, NULL)) == NULL)
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
		const char *name_space,
		const char *name,
		awk_value_t *value)
{
	NODE *node;
	NODE *array_node;

	if (   name == NULL
	    || *name == '\0'
	    || value == NULL
	    || ! is_valid_identifier(name)
	    || name_space == NULL
	    || (name_space[0] != '\0' && ! is_valid_identifier(name_space)))
		return awk_false;

	switch (value->val_type) {
	case AWK_NUMBER:
	case AWK_STRNUM:
	case AWK_STRING:
	case AWK_REGEX:
	case AWK_UNDEFINED:
	case AWK_ARRAY:
	case AWK_SCALAR:
	case AWK_VALUE_COOKIE:
		break;

	default:
		/* fatal(_("api_sym_update: invalid value for type of new value (%d)"), value->val_type); */
		return awk_false;
	}

	char *full_name = NULL;
	node = ns_lookup(name_space, name, & full_name);

	if (node == NULL) {
		/* new value to be installed */
		if (value->val_type == AWK_ARRAY) {
			array_node = awk_value_to_node(value);
			node = install_symbol(full_name, Node_var_array);
			array_node->vname = node->vname;
			*node = *array_node;
			freenode(array_node);
			value->array_cookie = node;	/* pass new cookie back to extension */
		} else {
			/* regular variable */
			node = install_symbol(full_name, Node_var);
			node->var_value = awk_value_to_node(value);
		}

		return awk_true;
	}

	/*
	 * If we get here, then it exists already.  Any valid type is
	 * OK except for AWK_ARRAY.
	 */
	if (   (node->flags & NO_EXT_SET) != 0
	    || is_off_limits_var(full_name)) {	/* most built-in vars not allowed */
		node->flags |= NO_EXT_SET;
		efree((void *) full_name);
		return awk_false;
	}

	efree((void *) full_name);

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
	case AWK_STRNUM:
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
			if (value->val_type == AWK_STRNUM)
				r->flags |= USER_INPUT;
			r->stfmt = STFMT_UNUSED;
			r->stptr = value->str_value.str;
			r->stlen = value->str_value.len;
#ifdef HAVE_MPFR
			r->strndmode = MPFR_round_mode;
#endif
			return awk_true;
		}
		break;

	case AWK_REGEX:
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
	case AWK_STRNUM:
	case AWK_STRING:
	case AWK_REGEX:
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

	/* don't check for index len zero, null str is ok as index */
	if (   array == NULL
	    || array->type != Node_var_array
	    || (array->flags & NO_EXT_SET) != 0
	    || index == NULL
	    || value == NULL
	    || ! valid_subscript_type(index->val_type))
		return awk_false;

	tmp = awk_value_to_node(index);
	elem = awk_value_to_node(value);
	if (elem->type == Node_var_array) {
		elem->parent_array = array;
		elem->vname = estrdup(index->str_value.str,
					index->str_value.len);
	}
	assoc_set(array, tmp, elem);

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

/* api_flatten_array_typed --- flatten out an array so that it can be looped over easily. */

static awk_bool_t
api_flatten_array_typed(awk_ext_id_t id,
		awk_array_t a_cookie,
		awk_flat_array_t **data,
		awk_valtype_t index_type, awk_valtype_t value_type)
{
	NODE **list;
	size_t i, j;
	NODE *array = (NODE *) a_cookie;
	size_t alloc_size;

	if (   array == NULL
	    || array->type != Node_var_array
	    || assoc_empty(array)
	    || data == NULL)
		return awk_false;

	alloc_size = sizeof(awk_flat_array_t) +
			(array->table_size - 1) * sizeof(awk_element_t);

	ezalloc(*data, awk_flat_array_t *, alloc_size,
			"api_flatten_array_typed");

	list = assoc_list(array, "@unsorted", ASORTI);

	(*data)->opaque1 = array;
	(*data)->opaque2 = list;
	(*data)->count = array->table_size;

	for (i = j = 0; i < 2 * array->table_size; i += 2, j++) {
		NODE *index, *value;

		index = list[i];
		value = list[i + 1]; /* number or string or subarray */

		/* Convert index and value to API types. */
		if (! node_to_awk_value(index,
				& (*data)->elements[j].index, index_type)) {
			fatal(_("api_flatten_array_typed: could not convert index %d to %s"),
						(int) i, valtype2str(index_type));
		}
		if (! node_to_awk_value(value,
				& (*data)->elements[j].value, value_type)) {
			fatal(_("api_flatten_array_typed: could not convert value %d to %s"),
						(int) i, valtype2str(value_type));
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
	case AWK_STRNUM:
	case AWK_STRING:
	case AWK_REGEX:
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

/* api_get_mpfr --- allocate an mpfr_ptr */

static void *
api_get_mpfr(awk_ext_id_t id)
{
#ifdef HAVE_MPFR
	mpfr_ptr p;
	getmpfr(p);
	mpfr_init(p);
	return p;
#else
	fatal(_("api_get_mpfr: MPFR not supported"));
	return NULL;	// silence compiler warning
#endif
}

/* api_get_mpz --- allocate an mpz_ptr */

static void *
api_get_mpz(awk_ext_id_t id)
{
#ifdef HAVE_MPFR
	mpz_ptr p;
	getmpz(p);
	mpz_init(p);
	return p;
#else
	fatal(_("api_get_mpfr: MPFR not supported"));
	return NULL;	// silence compiler warning
#endif
}

/* api_get_file --- return a handle to an existing or newly opened file */

static awk_bool_t
api_get_file(awk_ext_id_t id, const char *name, size_t namelen, const char *filetype,
		int fd, const awk_input_buf_t **ibufp, const awk_output_buf_t **obufp)
{
	const struct redirect *f;
	int flag;	/* not used, sigh */
	enum redirval redirtype;

	if (name == NULL || namelen == 0) {
		if (curfile == NULL) {
			INSTRUCTION *pc;
			int save_rule;
			char *save_source;

			if (nextfile(& curfile, false) <= 0)
				return awk_false;

			pc = main_beginfile;
			/* save execution state */
			save_rule = currule;
			save_source = source;

			for (;;) {
				if (pc == NULL)
					fatal(_("cannot find end of BEGINFILE rule"));
				if (pc->opcode == Op_after_beginfile)
					break;
				pc = pc->nexti;
			}
			pc->opcode = Op_stop;
		        (void) (*interpret)(main_beginfile);
			pc->opcode = Op_after_beginfile;
			after_beginfile(& curfile);
			/* restore execution state */
			currule = save_rule;
			source = save_source;
		}
		*ibufp = &curfile->public;
		*obufp = NULL;

		return awk_true;
	}

	redirtype = redirect_none;
	switch (filetype[0]) {
	case '<':
		if (filetype[1] == '\0')
			redirtype = redirect_input;
		break;
	case '>':
		switch (filetype[1]) {
		case '\0':
			redirtype = redirect_output;
			break;
		case '>':
			if (filetype[2] == '\0')
				redirtype = redirect_append;
			break;
		}
		break;
	case '|':
		if (filetype[2] == '\0') {
			switch (filetype[1]) {
			case '>':
				redirtype = redirect_pipe;
				break;
			case '<':
				redirtype = redirect_pipein;
				break;
			case '&':
				redirtype = redirect_twoway;
				break;
			}
		}
		break;
	}

	if (redirtype == redirect_none) {
		warning(_("cannot open unrecognized file type `%s' for `%s'"),
			filetype, name);
		return awk_false;
	}

	if ((f = redirect_string(name, namelen, 0, redirtype, &flag, fd, false)) == NULL)
		return awk_false;

	*ibufp = f->iop ? & f->iop->public : NULL;
	*obufp = f->output.fp ? & f->output : NULL;
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

#ifdef HAVE_MPFR
	__GNU_MP_VERSION,
	__GNU_MP_VERSION_MINOR,
	MPFR_VERSION_MAJOR,
	MPFR_VERSION_MINOR,
#else
	0, 0, 0, 0,
#endif

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
	api_nonfatal,

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
	api_flatten_array_typed,
	api_release_flattened_array,

	/* Memory allocation */
	malloc,
	calloc,
	realloc,
	free,
	api_get_mpfr,
	api_get_mpz,

	/* Find/open a file */
	api_get_file,
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

/* valtype2str --- return a printable representation of a value type */

static char *
valtype2str(awk_valtype_t type)
{
	static char buf[100];

	// Important: keep in same order as in gawkapi.h!
	static char *values[] = {
		"AWK_UNDEFINED",
		"AWK_NUMBER",
		"AWK_STRING",
		"AWK_REGEX",
		"AWK_STRNUM",
		"AWK_ARRAY",
		"AWK_SCALAR",
		"AWK_VALUE_COOKIE",
	};

	if (AWK_UNDEFINED <= type && type <= AWK_VALUE_COOKIE)
		return values[(int) type];

	sprintf(buf, "unknown type! (%d)", (int) type);

	return buf;
}

/* ns_lookup --- correctly build name before looking it up */

static NODE *
ns_lookup(const char *name_space, const char *name, char **fullname)
{
	assert(name_space != NULL);
	assert(name != NULL);

	if (name_space[0] == '\0' || strcmp(name_space, awk_namespace) == 0) {
		if (fullname != NULL)
			*fullname = estrdup(name, strlen(name));
		return lookup(name);
	}

	size_t len = strlen(name_space) + 2 + strlen(name) + 1;
	char *buf;
	emalloc(buf, char *, len, "ns_lookup");
	sprintf(buf, "%s::%s", name_space, name);

	NODE *f = lookup(buf);
	if (fullname != NULL)
		*fullname = buf;
	else
		efree((void *) buf);

	return f;
}
