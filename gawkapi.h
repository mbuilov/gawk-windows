/*
 * gawkapi.h -- Definitions for use by extension functions calling into gawk.
 */

/* 
 * Copyright (C) 2012, the Free Software Foundation, Inc.
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

/*
 * N.B. You must include <sys/types.h> and <sys/stat.h>
 * before including this file!
 */

#ifndef _GAWK_API_H
#define _GAWK_API_H

/*
 * General introduction:
 *
 * This API purposely restricts itself to C90 features.  In particular, no
 * bool, no // comments, no use of the restrict keyword, or anything else,
 * in order to provide maximal portability.
 * 
 * Exception: the "inline" keyword is used below in the "constructor"
 * functions. If your compiler doesn't support it, you should either
 * -Dinline='' on your command line, or use the autotools and include a
 * config.h in your extensions.
 *
 * Additional important information:
 *
 * 1. ALL string values in awk_value_t objects need to come from malloc().
 * Gawk will handle releasing the storage if necessary.  This is slightly
 * awkward, in that you can't take an awk_value_t that you got from gawk
 * and reuse it directly, even for something that is conceptually pass
 * by value.
 *
 * 2. Due to gawk internals, after using sym_update() to install an array
 * into gawk, you have to retrieve the array cookie from the value
 * passed in to sym_update().  Like so:
 *
 *	new_array = create_array();
 *	val.val_type = AWK_ARRAY;
 *	val.array_cookie = new_array;
 *	sym_update("array", & val);	// install array in the symbol table
 *
 *	new_array = val.array_cookie;	// MUST DO THIS
 *
 *	// fill in new array with lots of subscripts and values
 *
 * Similarly, if installing a new array as a subarray of an existing
 * array, you must add the new array to its parent before adding any
 * elements to it.
 *
 * You must also retrieve the value of the array_cookie after the call
 * to set_element().
 *
 * Thus, the correct way to build an array is to work "top down".
 * Create the array, and immediately install it in gawk's symbol table
 * using sym_update(), or install it as an element in a previously
 * existing array using set_element().
 *
 * The new array must ultimately be rooted in a global symbol. This is
 * necessary before installing any subarrays in it, due to gawk's
 * internal implementation.  Strictly speaking, this is required only
 * for arrays that will have subarrays as elements; however it is
 * a good idea to always do this.  This restriction may be relaxed
 * in a subsequent revision of the API.
 */

/* Allow use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/* This is used to keep the extension from modifying certain fields in some structs. */
#ifdef GAWK
#define awk_const
#else
#define awk_const const
#endif

/* portions of IOBUF that should be accessible to extension functions: */
typedef struct iobuf_public {
	const char *name;	/* filename */
	int fd;			/* file descriptor */
	void *opaque;           /* private data for input parsers */
	/*
	 * The get_record function is called to read the next record of data.
	 * It should return the length of the input record (or EOF), and
	 * it should set *out to point to the contents of $0.  Note that
	 * gawk will make a copy of the record in *out, so the parser is
	 * responsible for managing its own memory buffer.  If an error
	 * occurs, the function should return EOF and set *errcode
	 * to a non-zero value.  In that case, if *errcode does not equal
	 * -1, gawk will automatically update the ERRNO variable based on
	 * the value of *errcode (e.g. setting *errcode = errno should do
	 * the right thing).  It is guaranteed that errcode is a valid
	 * pointer, so there is no need to test for a NULL value.  The
	 * caller sets *errcode to 0, so there is no need to set it unless
	 * an error occurs.  The rt_start and rt_len arguments should be
	 * used to return RT to gawk.  Gawk will make its own copy of RT,
	 * so the parser is responsible for managing this memory.  If EOF is
	 * not returned, the parser must set *rt_len (and *rt_start if *rt_len
	 * is non-zero).
	 */
	int (*get_record)(char **out, struct iobuf_public *, int *errcode,
			char **rt_start, size_t *rt_len);
	/*
	 * The close_func is called to allow the parser to free private data.
	 * Gawk itself will close the fd unless close_func sets it to -1.
	 */
	void (*close_func)(struct iobuf_public *);
							
} IOBUF_PUBLIC;

typedef struct input_parser {
	const char *name;	/* name of parser */
	/*
	 * The can_take_file function should return non-zero if the parser
	 * would like to parse this file.  It should not change any gawk
	 * state!
	 */
	int (*can_take_file)(const IOBUF_PUBLIC *iobuf);
	/*
	 * If this parser is selected, then take_control_of will be called.
	 * It can assume that a previous call to can_take_file was successful,
	 * and no gawk state has changed since that call.  It should populate
	 * the IOBUF_PUBLIC get_record, close_func, and opaque values as needed.
	 * It should return non-zero if successful.
	 */
	int (*take_control_of)(IOBUF_PUBLIC *iobuf);
	struct input_parser *awk_const next;	/* for use by gawk */
} awk_input_parser_t;


#define GAWK_API_MAJOR_VERSION	0
#define GAWK_API_MINOR_VERSION	0

#define DO_FLAGS_SIZE	6

/*
 * This tag defines the type of a value.
 * Values are associated with regular variables and with array elements.
 * Since arrays can be multidimensional (as can regular variables)
 * it's valid to have a "value" that is actually an array.
 */
typedef enum {
	AWK_UNDEFINED,
	AWK_NUMBER,
	AWK_STRING,
	AWK_ARRAY,
	AWK_SCALAR,	/* opaque access to a variable */
	AWK_VALUE_COOKIE,	/* for updating to a previously created value */
} awk_valtype_t;

/*
 * A mutable string. Gawk owns the memory pointed to if it supplied
 * the value. Otherwise, it takes ownership of the memory pointed to.
 */
typedef struct {
	char *str;
	size_t len;
} awk_string_t;

/* Arrays are represented as an opaque type. */
typedef void *awk_array_t;

/* Scalars can be represented as an opaque type. */
typedef void *awk_scalar_t;

/* Any value can be stored as a cookie. */
typedef void *awk_value_cookie_t;

/*
 * An awk value. The val_type tag indicates what
 * is in the union.
 */
typedef struct {
	awk_valtype_t	val_type;
	union {
		awk_string_t	s;
		double		d;
		awk_array_t	a;
		awk_scalar_t	scl;
		awk_value_cookie_t vc;
	} u;
#define str_value	u.s
#define num_value	u.d
#define array_cookie	u.a
#define scalar_cookie	u.scl
#define value_cookie	u.vc
} awk_value_t;

/*
 * A "flattened" array element. Gawk produces an array of these
 * inside the awk_flattened_array_t.
 * ALL memory pointed to belongs to gawk. Individual elements may
 * be marked for deletion. New elements must be added individually,
 * one at a time, using the separate API for that purpose.
 */

typedef struct awk_element {
	/* convenience linked list pointer, not used by gawk */
	struct awk_element *next;
	enum {
		AWK_ELEMENT_DEFAULT = 0,	/* set by gawk */
		AWK_ELEMENT_DELETE = 1		/* set by extension if
						   should be deleted */
	} flags;
	awk_value_t	index;
	awk_value_t	value;
} awk_element_t;

/*
 * A "flattened" array. See the description above for how
 * to use the elements contained herein.
 */
typedef struct awk_flat_array {
	awk_const void *opaque1;	/* private data for use by gawk */
	awk_const void *opaque2;	/* private data for use by gawk */
	awk_const size_t count;		/* how many elements */
	awk_element_t elements[1];	/* will be extended */
} awk_flat_array_t;

/*
 * A record describing an extension function. Upon being
 * loaded, the extension should pass in one of these for
 * each C function.
 *
 * Each called function must fill in the result with eiher a number
 * or string. Gawk takes ownership of any string memory.
 *
 * The called function should return the value of `result'.
 * This is for the convenience of the calling code inside gawk.
 *
 * Each extension function may decide what to do if the number of
 * arguments isn't what it expected.  Following awk functions, it
 * is likely OK to ignore extra arguments.
 */
typedef struct {
	const char *name;
	awk_value_t *(*function)(int num_args_actual, awk_value_t *result);
	size_t num_args_expected;
} awk_ext_func_t;

typedef int awk_bool_t;	/* we don't use <stdbool.h> on purpose */

typedef void *awk_ext_id_t;	/* opaque type for extension id */

/*
 * The API into gawk. Lots of functions here. We hope that they are
 * logically organized.
 */
typedef struct gawk_api {
	awk_const int major_version;
	awk_const int minor_version;

	/*
	 * These can change on the fly as things happen within gawk.
	 * Currently only do_lint is prone to change, but we reserve
	 * the right to allow the others also.
	 */
	awk_const int do_flags[DO_FLAGS_SIZE];
/* Use these as indices into do_flags[] array to check the values */
#define gawk_do_lint		0
#define gawk_do_traditional	1
#define gawk_do_profile		2
#define gawk_do_sandbox		3
#define gawk_do_debug		4
#define gawk_do_mpfr		5

	/*
	 * All of the functions that return a value from inside gawk
	 * (get a parameter, get a global variable, get an array element)
	 * behave in the same way.
	 *
	 * Returns false if count is out of range, or if actual paramater
	 * does not match what is specified in wanted. In that case,
	 * result->val_type will hold the actual type of what was passed.

	Table entry is type returned:


	                        +-------------------------------------------------+
	                        |                Type of Actual Value:            |
	                        +------------+------------+-----------+-----------+
	                        |   String   |   Number   | Array     | Undefined |
	+-----------+-----------+------------+------------+-----------+-----------+
	|           | String    |   String   |   String   | false     | false     |
	|           |-----------+------------+------------+-----------+-----------+
	|           | Number    | Number if  |   Number   | false     | false     |
	|           |           | can be     |            |           |           |
	|           |           | converted, |            |           |           |
	|           |           | else false |            |           |           |
	|           |-----------+------------+------------+-----------+-----------+
	|   Type    | Array     |   false    |   false    | Array     | false     |
	| Requested |-----------+------------+------------+-----------+-----------+
	|           | Scalar    |   Scalar   |   Scalar   | false     | false     |
	|           |-----------+------------+------------+-----------+-----------+
	|           | Undefined |  String    |   Number   | Array     | Undefined |
	|           |-----------+------------+------------+-----------+-----------+
	|           | Value     |   false    |   false    | false     | false     |
	|           | Cookie    |            |            |           |           |
	+-----------+-----------+------------+------------+-----------+-----------+
	*/

	/*
	 * Get the count'th paramater, zero-based.
	 * Returns false if count is out of range, or if actual paramater
	 * does not match what is specified in wanted. In that case,
	 * result->val_type is as described above.
	 */
	awk_bool_t (*api_get_argument)(awk_ext_id_t id, size_t count,
					  awk_valtype_t wanted,
					  awk_value_t *result);

	/*
	 * Convert a paramter that was undefined into an array
	 * (provide call-by-reference for arrays).  Returns false
	 * if count is too big, or if the argument's type is
	 * not undefined.
	 */
	awk_bool_t (*api_set_argument)(awk_ext_id_t id,
					size_t count,
					awk_array_t array);

	/* Functions to print messages */
	void (*api_fatal)(awk_ext_id_t id, const char *format, ...);
	void (*api_warning)(awk_ext_id_t id, const char *format, ...);
	void (*api_lintwarn)(awk_ext_id_t id, const char *format, ...);

	/* Register an input parser; for opening files read-only */
	void (*api_register_input_parser)(awk_ext_id_t id, awk_input_parser_t *input_parser);

	/* Functions to update ERRNO */
	void (*api_update_ERRNO_int)(awk_ext_id_t id, int errno_val);
	void (*api_update_ERRNO_string)(awk_ext_id_t id, const char *string);
	void (*api_unset_ERRNO)(awk_ext_id_t id);

	/* Add a function to the interpreter, returns true upon success */
	awk_bool_t (*api_add_ext_func)(awk_ext_id_t id, const awk_ext_func_t *func,
			const char *namespace);

	/* Add an exit call back, returns true upon success */
	void (*api_awk_atexit)(awk_ext_id_t id,
			void (*funcp)(void *data, int exit_status),
			void *arg0);

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
	 * or if the wrong type was requested.
	 * In the latter case, fills in vaule->val_type with the real type,
	 * as described above.
	 * Built-in variables (except PROCINFO) may not be accessed by an
	 * extension.
	 *
	 * 	awk_value_t val;
	 * 	if (! api->sym_lookup(id, name, wanted, & val))
	 * 		error_code();
	 *	else {
	 *		// safe to use val
	 *	}
	 */
	awk_bool_t (*api_sym_lookup)(awk_ext_id_t id,
				const char *name,
				awk_valtype_t wanted,
				awk_value_t *result);

	/*
	 * Retrieve the current value of a scalar cookie.  Once
	 * you have obtained a saclar_cookie using sym_lookup, you can
	 * use this function to get its value more efficiently.
	 *
	 * Return will be false if the value cannot be retrieved.
	 */
	awk_bool_t (*api_sym_lookup_scalar)(awk_ext_id_t id,
				awk_scalar_t cookie,
				awk_valtype_t wanted,
				awk_value_t *result);

	/*
	 * Update a value. Adds it to the symbol table if not there.
	 * Changing types (scalar <--> array) is not allowed.
	 * In fact, using this to update an array is not allowed, either.
	 * Such an attempt returns false.
	 */
	awk_bool_t (*api_sym_update)(awk_ext_id_t id,
				const char *name,
				awk_value_t *value);
	/*
	 * Install a constant value.
	 */
	awk_bool_t (*api_sym_constant)(awk_ext_id_t id,
				const char *name,
				awk_value_t *value);

	/*
	 * Work with a scalar cookie.
	 * Flow is
	 * 	sym_lookup with wanted == AWK_SCALAR
	 * 	if returns false
	 * 		sym_update with real initial value
	 * 		sym_lookup again with AWK_SCALAR
	 *	else
	 *		use the scalar cookie
	 *
	 * Return will be false if the new value is not one of
	 * AWK_STRING or AWK_NUMBER.
	 */
	awk_bool_t (*api_sym_update_scalar)(awk_ext_id_t id,
				awk_scalar_t cookie, awk_value_t *value);

	/* Array management */
	/*
	 * Return the value of an element - read only!
	 * Use set_array_element() to change it.
	 * Behavior for value and return is same as for api_get_argument
	 * and sym_lookup.
	 */
	awk_bool_t (*api_get_array_element)(awk_ext_id_t id,
			awk_array_t a_cookie,
			const awk_value_t *const index,
			awk_valtype_t wanted,
			awk_value_t *result);

	/*
	 * Change (or create) element in existing array with
	 * element->index and element->value.
	 */
	awk_bool_t (*api_set_array_element)(awk_ext_id_t id, awk_array_t a_cookie,
					const awk_value_t *const index,
					const awk_value_t *const value);

	/*
	 * Remove the element with the given index.
	 * Returns success if removed or if element did not exist.
	 */
	awk_bool_t (*api_del_array_element)(awk_ext_id_t id,
			awk_array_t a_cookie, const awk_value_t* const index);

	/*
	 * Retrieve total number of elements in array.
	 * Returns false if some kind of error.
	 */
	awk_bool_t (*api_get_element_count)(awk_ext_id_t id,
			awk_array_t a_cookie, size_t *count);

	/* Create a new array cookie to which elements may be added */
	awk_array_t (*api_create_array)(awk_ext_id_t id);

	/* Clear out an array */
	awk_bool_t (*api_clear_array)(awk_ext_id_t id, awk_array_t a_cookie);

	/* Flatten out an array so that it can be looped over easily. */
	awk_bool_t (*api_flatten_array)(awk_ext_id_t id,
			awk_array_t a_cookie,
			awk_flat_array_t **data);

	/*
	 * When done, delete any marked elements, release the memory.
	 * Count must match what gawk thinks the size is.
	 * Otherwise it's a fatal error.
	 */
	awk_bool_t (*api_release_flattened_array)(awk_ext_id_t id,
			awk_array_t a_cookie,
			awk_flat_array_t *data);

	/*
	 * Cache a string or numeric value for efficient later assignment.
	 * This improves performance when you want to assign the same value
	 * to one or more variables repeatedly.  Only AWK_NUMBER and AWK_STRING
	 * values are allowed.  Any other type is rejected.  We disallow
	 * AWK_UNDEFINED since that case would result in inferior performance.
	 */
	awk_bool_t (*api_create_value)(awk_ext_id_t id, awk_value_t *value,
		    awk_value_cookie_t *result);

	/*
	 * Release the memory associated with a cookie from api_create_value.
	 * Please call this to free memory when the value is no longer needed.
	 */
	awk_bool_t (*api_release_value)(awk_ext_id_t id, awk_value_cookie_t vc);
} gawk_api_t;

#ifndef GAWK	/* these are not for the gawk code itself! */
/*
 * Use these if you want to define "global" variables named api
 * and ext_id to make the code a little easier to read.
 */
#define do_lint		(api->do_flags[gawk_do_lint])
#define do_traditional	(api->do_flags[gawk_do_traditional])
#define do_profile	(api->do_flags[gawk_do_profile])
#define do_sandbox	(api->do_flags[gawk_do_sandbox])
#define do_debug	(api->do_flags[gawk_do_debug])
#define do_mpfr		(api->do_flags[gawk_do_mpfr])

#define get_argument(count, wanted, result) \
	(api->api_get_argument(ext_id, count, wanted, result))
#define set_argument(count, new_array) \
	(api->api_set_argument(ext_id, count, new_array))

#define fatal		api->api_fatal
#define warning		api->api_warning
#define lintwarn	api->api_lintwarn

#define register_input_parser(parser)	(api->api_register_input_parser(ext_id, parser))

#define update_ERRNO_int(e)	(api->api_update_ERRNO_int(ext_id, e))
#define update_ERRNO_string(str) \
	(api->api_update_ERRNO_string(ext_id, str))
#define unset_ERRNO()	(api->api_unset_ERRNO(ext_id))

#define add_ext_func(func, ns)	(api->api_add_ext_func(ext_id, func, ns))
#define awk_atexit(funcp, arg0)	(api->api_awk_atexit(ext_id, funcp, arg0))

#define sym_lookup(name, wanted, result) \
	(api->api_sym_lookup(ext_id, name, wanted, result))
#define sym_lookup_scalar(scalar_cookie, wanted, result) \
	(api->api_sym_lookup_scalar(ext_id, scalar_cookie, wanted, result))
#define sym_update(name, value) \
	(api->api_sym_update(ext_id, name, value))
#define sym_constant(name, value) \
	(api->api_sym_constant(ext_id, name, value))
#define sym_update_scalar(scalar_cookie, value) \
	(api->api_sym_update_scalar)(ext_id, scalar_cookie, value)

#define get_array_element(array, index, wanted, result) \
	(api->api_get_array_element(ext_id, array, index, wanted, result))

#define set_array_element(array, index, value) \
	(api->api_set_array_element(ext_id, array, index, value))

#define set_array_element_by_elem(array, elem) \
	(api->api_set_array_element(ext_id, array, & (elem)->index, & (elem)->value))

#define del_array_element(array, index) \
	(api->api_del_array_element(ext_id, array, index))

#define get_element_count(array, count_p) \
	(api->api_get_element_count(ext_id, array, count_p))

#define create_array()		(api->api_create_array(ext_id))

#define clear_array(array)	(api->api_clear_array(ext_id, array))

#define flatten_array(array, data) \
	(api->api_flatten_array(ext_id, array, data))

#define release_flattened_array(array, data) \
	(api->api_release_flattened_array(ext_id, array, data))

#define create_value(value, result) \
	(api->api_create_value(ext_id, value,result))

#define release_value(value) \
	(api->api_release_value(ext_id, value))

#define emalloc(pointer, type, size, message) \
	do { \
		if ((pointer = (type) malloc(size)) == 0) \
			fatal(ext_id, "malloc of %d bytes failed\n", size); \
	} while(0)

#define erealloc(pointer, type, size, message) \
	do { \
		if ((pointer = (type) realloc(pointer, size)) == 0) \
			fatal(ext_id, "realloc of %d bytes failed\n", size); \
	} while(0)

/* Constructor functions */

/* r_make_string --- make a string value in result from the passed-in string */

static inline awk_value_t *
r_make_string(const gawk_api_t *api,	/* needed for emalloc */
	      awk_ext_id_t *ext_id,	/* ditto */
	      const char *string,
	      size_t length,
	      awk_bool_t duplicate,
	      awk_value_t *result)
{
	char *cp = NULL;

	memset(result, 0, sizeof(*result));

	result->val_type = AWK_STRING;
	result->str_value.len = length;

	if (duplicate) {
		emalloc(cp, char *, length + 2, "r_make_string");
		memcpy(cp, string, length);
		cp[length] = '\0';
		result->str_value.str = cp;
	} else {
		result->str_value.str = (char *) string;
	}

	return result;
}

#define make_const_string(str, len, result)	r_make_string(api, ext_id, str, len, 1, result)
#define make_malloced_string(str, len, result)	r_make_string(api, ext_id, str, len, 0, result)

/* make_null_string --- make a null string value */

static inline awk_value_t *
make_null_string(awk_value_t *result)
{
	memset(result, 0, sizeof(*result));
	result->val_type = AWK_UNDEFINED;

	return result;
}

/* make_number --- make a number value in result */

static inline awk_value_t *
make_number(double num, awk_value_t *result)
{
	memset(result, 0, sizeof(*result));

	result->val_type = AWK_NUMBER;
	result->num_value = num;

	return result;
}

/*
 * Each extension must define a function with this prototype:
 *
 *	int dl_load(gawk_api_t *api_p, awk_ext_id_t id)
 *
 * The return value should be zero on failure and non-zero on success.
 *
 * For the macros to work, the function should save api_p in a global
 * variable named 'api' and save id in a global variable named 'ext_id'.
 * In addition, a global function pointer named 'init_func' should be
 * defined and set to either NULL or an initialization function that
 * returns non-zero on success and zero upon failure.
 */

extern int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id);

#if 0
/* Boiler plate code: */
static gawk_api_t *const api;
static awk_ext_id_t ext_id;
static awk_ext_func_t func_table[] = {
	{ "name", do_name, 1 },
	/* ... */
};

/* EITHER: */

static awk_bool_t (*init_func)(void) = NULL;

/* OR: */

static awk_bool_t init_my_module(void)
{
	...
}

static awk_bool_t (*init_func)(void) = init_my_module;

dl_load_func(func_table, some_name, "name_space_in_quotes")
#endif

#define dl_load_func(func_table, module, name_space) \
int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id)  \
{ \
	size_t i, j; \
	int errors = 0; \
\
	api = api_p; \
	ext_id = id; \
\
	if (api->major_version != GAWK_API_MAJOR_VERSION \
	    || api->minor_version < GAWK_API_MINOR_VERSION) { \
		fprintf(stderr, #module ": version mismatch with gawk!\n"); \
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n", \
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION, \
			api->major_version, api->minor_version); \
		exit(1); \
	} \
\
	/* load functions */ \
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) { \
		if (! add_ext_func(& func_table[i], name_space)) { \
			warning(ext_id, #module ": could not add %s\n", \
					func_table[i].name); \
			errors++; \
		} \
	} \
\
	if (init_func != NULL) { \
		if (! init_func()) { \
			warning(ext_id, #module ": initialization function failed\n"); \
			errors++; \
		} \
	} \
\
	return (errors == 0); \
}

#endif /* GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
