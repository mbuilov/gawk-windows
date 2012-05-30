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
 * This API purposely restricts itself to C90 features.  In paticular, no
 * bool, no // comments, no use of the restrict keyword, or anything else,
 * in order to provide maximal portability.
 * 
 * Exception: the "inline" keyword is used below in the "constructor"
 * functions. If your compiler doesn't support it, you should either
 * -Dinline='' on your command line, or use the autotools and include a
 * config.h in your extensions.
 */

/* Allow use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/* struct used for reading records and managing buffers */
typedef struct iobuf {
	const char *name;       /* filename */
	int fd;                 /* file descriptor */
	struct stat sbuf;       /* stat buf */
	char *buf;              /* start data buffer */
	char *off;              /* start of current record in buffer */
	char *dataend;          /* first byte in buffer to hold new data,
				   NULL if not read yet */
	char *end;              /* end of buffer */
	size_t readsize;        /* set from fstat call */
	size_t size;            /* buffer size */
	ssize_t count;          /* amount read last time */
	size_t scanoff;         /* where we were in the buffer when we had
				   to regrow/refill */
	/*
	 * No argument prototype on read_func. See get_src_buf()
	 * in awkgram.y.
	 */
	ssize_t (*read_func)();

	void *opaque;		/* private data for open hooks */
	int (*get_record)(char **out, struct iobuf *, int *errcode);
	void (*close_func)(struct iobuf *);	/* open and close hooks */
	
	int errcode;

	int flag;
#		define	IOP_IS_TTY	1
#		define	IOP_NOFREE_OBJ	2
#		define  IOP_AT_EOF      4
#		define  IOP_CLOSED      8
#		define  IOP_AT_START    16
} IOBUF;

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
	AWK_ARRAY
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

/*
 * An awk value. The val_type tag indicates what
 * is contained.  For scalars, gawk fills in both kinds
 * of values and val_type indicates the assigned type.
 * For arrays, the scalar types will be set to zero.
 */
typedef struct {
	awk_valtype_t	val_type;
	awk_string_t	str_value;
	double		num_value;
	awk_array_t	array_cookie;
} awk_value_t;

/*
 * A "flattened" array element. Gawk produces an array of these.
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
	awk_string_t	index;
	awk_value_t	value;
} awk_element_t;

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
	int major_version;
	int minor_version;

	int do_flags[DO_FLAGS_SIZE];
/* Use these as indices into do_flags[] array to check the values */
#define gawk_do_lint		0
#define gawk_do_traditional	1
#define gawk_do_profile		2
#define gawk_do_sandbox		3
#define gawk_do_debug		4
#define gawk_do_mpfr		5

	/*
	 * Get the count'th paramater, zero-based.
	 * Returns NULL if count is out of range, or if actual paramater
	 * does not match what is specified in wanted.
	 */
	awk_value_t *(*get_curfunc_param)(awk_ext_id_t id, size_t count,
					  awk_valtype_t wanted,
					  awk_value_t *result);

	/* Functions to print messages */
	void (*api_fatal)(awk_ext_id_t id, const char *format, ...);
	void (*api_warning)(awk_ext_id_t id, const char *format, ...);
	void (*api_lintwarn)(awk_ext_id_t id, const char *format, ...);

	/* Register an open hook; for opening files read-only */
	void (*register_open_hook)(awk_ext_id_t id, void* (*open_func)(IOBUF *));

	/* Functions to update ERRNO */
	void (*update_ERRNO_int)(awk_ext_id_t id, int errno_val);
	void (*update_ERRNO_string)(awk_ext_id_t id, const char *string,
			awk_bool_t translate);
	void (*unset_ERRNO)(awk_ext_id_t id);

	/* Add a function to the interpreter, returns true upon success */
	awk_bool_t (*add_ext_func)(awk_ext_id_t id, const awk_ext_func_t *func,
			const char *namespace);

	/* Add an exit call back, returns true upon success */
	awk_bool_t (*awk_atexit)(awk_ext_id_t id,
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
	 * Lookup a variable, return its value. No messing with the value
	 * returned. Return value is NULL if the variable doesn't exist.
	 *
	 * Returns a pointer to a static variable. Correct usage is thus:
	 *
	 * 	awk_value_t val;
	 * 	if (api->sym_lookup(id, name, &val, wanted) == NULL)
	 * 		error_code();
	 *	else {
	 *		// safe to use val
	 *	}
	 */
	awk_value_t *(*sym_lookup)(awk_ext_id_t id, const char *name, awk_value_t *result,
			awk_valtype_t wanted);

	/*
	 * Update a value. Adds it to the symbol table if not there.
	 * Changing types (scalar <--> array) is not allowed.
	 * In fact, using this to update an array is not allowed, either.
	 * Such an attempt returns false.
	 */
	awk_bool_t (*sym_update)(awk_ext_id_t id, const char *name, awk_value_t *value);

	/* Array management */
	/*
	 * Return the value of an element - read only!
	 * Use set_array_element() to change it.
	 */
	awk_value_t *(*get_array_element)(awk_ext_id_t id,
			awk_array_t a_cookie, const awk_value_t *const index,
			awk_value_t *result, awk_valtype_t wanted);

	/*
	 * Change (or create) element in existing array with
	 * element->index and element->value.
	 */
	awk_bool_t (*set_array_element)(awk_ext_id_t id, awk_array_t a_cookie,
					awk_element_t *element);

	/*
	 * Remove the element with the given index.
	 * Returns success if removed or if element did not exist.
	 */
	awk_bool_t (*del_array_element)(awk_ext_id_t id,
			awk_array_t a_cookie, const awk_value_t* const index);

	/*
	 * Retrieve total number of elements in array.
	 * Returns false if some kind of error.
	 */
	awk_bool_t (*get_element_count)(awk_ext_id_t id,
			awk_array_t a_cookie, size_t *count);

	/* Create a new array cookie to which elements may be added */
	awk_array_t (*create_array)(awk_ext_id_t id);

	/* Clear out an array */
	awk_bool_t (*clear_array)(awk_ext_id_t id, awk_array_t a_cookie);

	/* Flatten out an array so that it can be looped over easily. */
	awk_bool_t (*flatten_array)(awk_ext_id_t id,
			awk_array_t a_cookie,
			size_t *count,
			awk_element_t **data);

	/*
	 * When done, delete any marked elements, release the memory.
	 * Count must match what gawk thinks the size is.
	 * Otherwise it's a fatal error.
	 */
	awk_bool_t (*release_flattened_array)(awk_ext_id_t id,
			awk_array_t a_cookie,
			size_t count,
			awk_element_t *data);
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

#define get_curfunc_param(count, wanted, result) \
	(api->get_curfunc_param(ext_id, count, wanted, result))

#define fatal		api->api_fatal
#define warning		api->api_warning
#define lintwarn	api->api_lintwarn

#define register_open_hook(func)	(api->register_open_hook(ext_id, func))

#define update_ERRNO_int(e)	(api->update_ERRNO_int(ext_id, e))
#define update_ERRNO_string(str, translate) \
	(api->update_ERRNO_string(ext_id, str, translate))
#define unset_ERRNO()	(api->unset_ERRNO(ext_id))

#define add_ext_func(func, ns)	(api->add_ext_func(ext_id, func, ns))
#define awk_atexit(funcp, arg0)	(api->awk_atexit(ext_id, funcp, arg0))

#define sym_lookup(name, result, wanted)	(api->sym_lookup(ext_id, name, result, wanted))
#define sym_update(name, value) \
	(api->sym_update(ext_id, name, value))

#define get_array_element(array, element, result, wanted) \
	(api->get_array_element(ext_id, array, element, result, wanted))

#define set_array_element(array, element) \
	(api->set_array_element(ext_id, array, element))

#define del_array_element(array, index) \
	(api->del_array_element(ext_id, array, index))

#define get_element_count(array, count_p) \
	(api->get_element_count(ext_id, array, count_p))

#define create_array()		(api->create_array(ext_id))

#define clear_array(array)	(api->clear_array(ext_id, array))

#define flatten_array(array, count, data) \
	(api->flatten_array(ext_id, array, count, data))

#define release_flattened_array(array, count, data) \
	(api->release_flattened_array(ext_id, array, count, data))

#define emalloc(pointer, type, size, message) \
	do { \
		if ((pointer = (type) malloc(size)) == 0) \
			fatal(ext_id, "malloc of %d bytes failed\n", size); \
	} while(0)

/* Constructor functions */

/* r_make_string --- make a string value in result from the passed-in string */

static inline awk_value_t *
r_make_string(const gawk_api_t *api,
	      awk_ext_id_t *ext_id,
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
		emalloc(cp, char *, length + 2, "make_string");
		memcpy(cp, string, length);
		cp[length] = '\0';
		result->str_value.str = cp;
	} else {
		result->str_value.str = (char *) string;
	}

	return result;
}

#define make_string(str, len, result)	r_make_string(api, ext_id, str, len, 0, result)
#define dup_string(str, len, result)	r_make_string(api, ext_id, str, len, 1, result)

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
 * For the macros to work, the function should save api_p in a global
 * variable named 'api' and save id in a global variable named 'ext_id'.
 * The return value should be zero on failure and non-zero on success.
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
	return (errors == 0); \
}

#endif /* GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
