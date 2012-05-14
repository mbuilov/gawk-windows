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

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

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
	void (*close_func)(struct iobuf *);		/* open and close hooks */
	
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
 * Values are associated with regular variables,
 * and with array elements. Since arrays can
 * be multidimensional (as can regular variables)
 * it's valid to have a "value" that is actually
 * an array.
 */
typedef enum {
	AWK_UNDEFINED,
	AWK_NUMBER,
	AWK_CONST_STRING,
	AWK_STRING,
	AWK_ARRAY
} awk_valtype_t;

/*
 * A constant string - one that the extension is
 * not allowed to change. The memory pointed belongs to gawk
 * when it's in a value that gawk gives to the extension.
 */
typedef struct {
	const char *const str;
	const size_t len;
} awk_const_string_t;

/*
 * A mutable string. Depending upon the use, gawk may
 * take ownership of the memory pointed to.
 */
typedef struct {
	char *str;
	size_t len;
} awk_string_t;

/* Arrays are represented as an opaque type */
typedef void *awk_array_t;
/*
 * An awk value. The val_type tag indicates what
 * is in the union.
 */
typedef struct {
	awk_valtype_t	val_type;
	union {
		awk_const_string	cs;
		awk_string		s;
		double			d;
		awk_array_t		a;
	} u;
#define const_str_val	u.cs
#define str_val		u.s
#define num_val		u.d
#define array_cookie	u.a
} awk_value_t;

/*
 * A "flattened" array element. Gawk produces an array of these.
 * ALL memory pointed to belongs to gawk. Individual elements may
 * be marked for deletion. New elements must be added individually,
 * one at a time, using the API for that purpose.
 */

typedef struct awk_element {
	struct awk_element *next;	/* convenience linked list pointer, not used by gawk */
	enum {
		AWK_ELEMENT_DEFAULT = 0,	/* set by gawk */
		AWK_ELEMENT_DELETE = 1		/* extension sets if should be deleted */
	} flags;
	awk_const_str	index;
	awk_value_t	value;
} awk_element_t;

/*
 * A record describing an extension function. Upon being
 * loaded, the extension should pass in one of these for
 * each C function.
 */
typedef struct {
	const char *name;
	size_t num_args_expected;
	void (*function)(int num_args_actual);
} awk_ext_func_t;

typedef int awkbool_t;	/* we don't use <stdbool.h> on purpose */

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

	/* Get the count'th paramater, zero-based */
	awk_value_t *(*get_curfunc_param)(void *ext_id, size_t count);

	/* Set the return value. Gawk takes ownership of string memory */
	void (*set_return_value)(void *ext_id, const awk_value_t *retval);

	/* functions to print messages */
	void (*fatal)(void *ext_id, const char *format, ...);
	void (*warning)(void *ext_id, const char *format, ...);
	void (*lintwarn)(void *ext_id, const char *format, ...);

	/* register an open hook; for opening files read-only */
	int (*register_open_hook)(void *ext_id, void* (*open_func)(IOBUF *));

	/* functions to update ERRNO */
	void (*update_ERRNO_int)(void *ext_id, int);
	void (*update_ERRNO_string)(void *ext_id, const char *string,
			awkbool_t translate);
	void (*unset_ERRNO)(void *ext_id);

	/* check if a value received from gawk is the null string */
	awkbool_t (*is_null_string)(void *ext_id, void *value);

	/* add a function to the interpreter, returns true upon success */
	awkbool_t *(add_ext_func)(void *ext_id, const awk_ext_func_t *func);

	/* add an exit call back, returns true upon success */
	awkbook_t (*awk_atexit)(void *ext_id, void (*funcp)(void *data, int exit_status), void *arg0);

	/*
	 * Symbol table access:
	 * 	- No access to special variables (NF, etc.)
	 * 	- This may be relaxed to allow access to PROCINFO
	 *	- Use sym_update() to change a value, including from UNDEFINED
	 *	  to scalar or array. 
	 */
	/*
	 * Lookup a variable, return its value. No messing with the value returned
	 * Return value is NULL if the variable doesn't exist.
	 */
	const awk_value_t *const (*sym_lookup)(void *ext_id, const char *name);

	/* Update a value. Adds it to the symbol table if not there. Changing types is not allowed */
	awkbool_t (*sym_update)(void *ext_id, const char *name, awk_value_t *value);

	/* Array management */
	/* Return the value of an element - read only! Use set_array_element to change it */
	const awk_value_t *const (*get_array_element)(void *ext_id, awk_array_t a_cookie, const awk_value_t* const index);

	/* Change (or create) element in existing array with element->index and element->value */
	awk_bool_t (*set_array_element)(void *ext_id, awk_array_t a_cookie,
					awk_element_t *element);

	/* Remove the element with the given index. Returns success if removed or if element did not exist */
	awk_bool_t (*del_array_element)(void *ext_id, awk_array_t a_cookie, const awk_value_t* const index);

	/* Retrieve total number of elements in array. Returns false if some kind of error */
	awk_bool_t (*get_element_count)(void *ext_id, awk_array_t a_cookie, size_t *count);

	/* Create a new array cookie to which elements may be added */
	awk_array_t (*create_array)(void *ext_id);

	/* Flatten out an array so that it can be looped over easily. */
	awk_bool_t (*flatten_array)(void *ext_id, awk_array_t a_cookie, size_t *count, awk_element_t **data);

	/* When done, release the memory, delete any marked elements */
	awk_bool_t (*release_flattened_array)(void *ext_id, awk_array_t a_cookie, size_t count, awk_element_t *data);

} gawk_api_t;

#ifndef GAWK	/* these are not for the gawk code itself */
/*
 * Use these if you want to define a "global" variable named api
 * to make the code a little easier to read.
 */
#define do_lint		api->do_flags[gawk_do_lint]
#define do_traditional	api->do_flags[gawk_do_traditional]
#define do_profile	api->do_flags[gawk_do_profile]
#define do_sandbox	api->do_flags[gawk_do_sandbox]
#define do_debug	api->do_flags[gawk_do_debug]
#define do_mpfr		api->do_flags[gawk_do_mpfr]

#define get_curfunc_param	api->get_curfunc_param
#define set_return_value	api->set_return_value

#define fatal		api->fatal
#define warning		api->warning
#define lintwarn	api->lintwarn

#define register_open_hook	api->register_open_hook

#define update_ERRNO_int	api->update_ERRNO_int
#define update_ERRNO_string	api->update_ERRNO_string
#define unset_ERRNO	api->unset_ERRNO

#define is_null_string	api->is_null_string

#define add_ext_func	api->add_ext_func
#define awk_atexit	api->awk_atexit

#define sym_lookup	api->sym_lookup
#define sym_update	api->sym_update

#define get_array_element	api->get_array_element
#define set_array_element	api->set_array_element
#define del_array_element	api->del_array_element
#define get_element_count	api->get_element_count
#define create_array		api->create_array
#define flatten_array		api->flatten_array
#define release_flattened_array	api->release_flattened_array
#endif /* GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
