/*
 * gawkapi.h -- Definitions for use by extension functions calling into gawk.
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

/*
 * The following types and/or macros and/or functions are referenced
 * in this file.  For correct use, you must therefore include the
 * corresponding standard header file BEFORE including this file.
 *
 * FILE			- <stdio.h>
 * NULL			- <stddef.h>
 * memset(), memcpy()	- <string.h>
 * size_t		- <sys/types.h>
 * struct stat		- <sys/stat.h>
 *
 * Due to portability concerns, especially to systems that are not
 * fully standards-compliant, it is your responsibility to include
 * the correct files in the correct way. This requirement is necessary
 * in order to keep this file clean, instead of becoming a portability
 * hodge-podge as can be seen in the gawk source code.
 *
 * To pass reasonable integer values for ERRNO, you will also need to
 * include <errno.h>.
 */

#ifndef _GAWK_API_H
#define _GAWK_API_H

/*
 * General introduction:
 *
 * This API purposely restricts itself to ISO C 90 features.  In particular, no
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
 * 1. ALL string values in awk_value_t objects need to come from api_malloc().
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
 * Thus the new array must ultimately be rooted in a global symbol. This is
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

/* This is used to keep extensions from modifying certain fields in some structs. */
#ifdef GAWK
#define awk_const
#else
#define awk_const const
#endif

typedef enum awk_bool {
	awk_false = 0,
	awk_true
} awk_bool_t;	/* we don't use <stdbool.h> on purpose */

/*
 * If an input parser would like to specify the field positions in the input
 * record, it may populate an awk_fieldwidth_info_t structure to indicate
 * the location of each field. The use_chars boolean controls whether the
 * field lengths are specified in terms of bytes or potentially multi-byte
 * characters. Performance will be better if the values are supplied in
 * terms of bytes. The fields[0].skip value indicates how many bytes (or
 * characters) to skip before $1, and fields[0].len is the length of $1, etc.
 */

struct awk_field_info {
	size_t	skip;	/* amount to skip before field starts */
	size_t	len;	/* length of field */
};

typedef struct {
	awk_bool_t	use_chars;	/* false ==> use bytes */
	size_t		nf;
	struct awk_field_info fields[1];	/* actual dimension should be nf */
} awk_fieldwidth_info_t;

/*
 * This macro calculates the total struct size needed. This is useful when
 * calling malloc or realloc.
 */
#define awk_fieldwidth_info_size(NF) (sizeof(awk_fieldwidth_info_t) + \
			(((NF)-1) * sizeof(struct awk_field_info)))

/* File descriptor type.  */
typedef int fd_t;
#define INVALID_HANDLE (-1)

/* The information about input files that input parsers need to know: */
typedef struct awk_input {
	const char *name;	/* filename */
	fd_t fd;		/* file descriptor */
	void *opaque;           /* private data for input parsers */
	/*
	 * The get_record function is called to read the next record of data.
	 *
	 * It should return the length of the input record or EOF, and it
	 * should set *out to point to the contents of $0. The rt_start
	 * and rt_len arguments should be used to return RT to gawk.
	 * If EOF is not returned, the parser must set *rt_len (and
	 * *rt_start if *rt_len is non-zero).
	 *
	 * Note that gawk will make a copy of the record in *out, so the
	 * parser is responsible for managing its own memory buffer.
	 * Similarly, gawk will make its own copy of RT, so the parser
	 * is also responsible for managing this memory.
	 *
	 * It is guaranteed that errcode is a valid pointer, so there is
	 * no need to test for a NULL value.  Gawk sets *errcode to 0,
	 * so there is no need to set it unless an error occurs.
	 *
	 * If an error does occur, the function should return EOF and set
	 * *errcode to a positive value.  In that case, if *errcode is greater
	 * than zero, gawk will automatically update the ERRNO variable based
	 * on the value of *errcode (e.g., setting *errcode = errno should do
	 * the right thing).
	 *
	 * If field_width is non-NULL, then *field_width will be initialized
	 * to NULL, and the function may set it to point to a structure
	 * supplying field width information to override the default
	 * gawk field parsing mechanism. Note that this structure will not
	 * be copied by gawk; it must persist at least until the next call
	 * to get_record or close_func. Note also that field_width will
	 * be NULL when getline is assigning the results to a variable, thus
	 * field parsing is not needed.
	 */
	int (*get_record)(char **out, struct awk_input *iobuf, int *errcode,
			char **rt_start, size_t *rt_len,
			const awk_fieldwidth_info_t **field_width);

	/*
	 * The same arguments/return type as for POSIX read(2).
	 */
	ssize_t (*read_func)(fd_t fd, void *buffer, size_t count);

	/*
	 * The close_func is called to allow the parser to free private data.
	 * Gawk itself will close the fd unless close_func first sets it to
	 * INVALID_HANDLE.
	 */
	void (*close_func)(struct awk_input *iobuf);

	/* put last, for alignment. bleah */
	struct stat sbuf;       /* stat buf */

} awk_input_buf_t;

typedef struct awk_input_parser {
	const char *name;	/* name of parser */

	/*
	 * The can_take_file function should return true if the parser
	 * would like to parse this file.  It should not change any gawk
	 * state!
	 */
	awk_bool_t (*can_take_file)(const awk_input_buf_t *iobuf);

	/*
	 * If this parser is selected, then take_control_of will be called.
	 * It can assume that a previous call to can_take_file was successful,
	 * and no gawk state has changed since that call.  It should populate
	 * the awk_input_buf_t's get_record, close_func, and opaque values as needed.
	 * It should return true if successful.
	 */
	awk_bool_t (*take_control_of)(awk_input_buf_t *iobuf);

	awk_const struct awk_input_parser *awk_const next;	/* for use by gawk */
} awk_input_parser_t;

/*
 * Similar for output wrapper.
 */

/* First the data structure */
typedef struct awk_output_buf {
	const char *name;	/* name of output file */
	const char *mode;	/* mode argument to fopen */
	FILE *fp;		/* stdio file pointer */
	awk_bool_t redirected;	/* true if a wrapper is active */
	void *opaque;		/* for use by output wrapper */

	/*
	 * Replacement functions for I/O.  Just like the regular
	 * versions but also take the opaque pointer argument.
	 */
	size_t (*gawk_fwrite)(const void *buf, size_t size, size_t count,
				FILE *fp, void *opaque);
	int (*gawk_fflush)(FILE *fp, void *opaque);
	int (*gawk_ferror)(FILE *fp, void *opaque);
	int (*gawk_fclose)(FILE *fp, void *opaque);
} awk_output_buf_t;

/* Next the output wrapper registered with gawk */
typedef struct awk_output_wrapper {
	const char *name;	/* name of the wrapper */

	/*
	 * The can_take_file function should return true if the wrapper
	 * would like to process this file.  It should not change any gawk
	 * state!
	 */
	awk_bool_t (*can_take_file)(const awk_output_buf_t *outbuf);

	/*
	 * If this wrapper is selected, then take_control_of will be called.
	 * It can assume that a previous call to can_take_file was successful,
	 * and no gawk state has changed since that call.  It should populate
	 * the awk_output_buf_t function pointers and opaque pointer as needed.
	 * It should return true if successful.
	 */
	awk_bool_t (*take_control_of)(awk_output_buf_t *outbuf);

	awk_const struct awk_output_wrapper *awk_const next;  /* for use by gawk */
} awk_output_wrapper_t;

/* A two-way processor combines an input parser and an output wrapper. */
typedef struct awk_two_way_processor {
	const char *name;	/* name of the two-way processor */

	/*
	 * The can_take_file function should return true if the two-way
	 * processor would like to parse this file.  It should not change
	 * any gawk state!
	 */
	awk_bool_t (*can_take_two_way)(const char *name);

	/*
	 * If this processor is selected, then take_control_of will be called.
	 * It can assume that a previous call to can_take_file was successful,
	 * and no gawk state has changed since that call.  It should populate
	 * the awk_input_buf_t and awk_otuput_buf_t structures as needed.
	 * It should return true if successful.
	 */
	awk_bool_t (*take_control_of)(const char *name, awk_input_buf_t *inbuf,
					awk_output_buf_t *outbuf);

	awk_const struct awk_two_way_processor *awk_const next;  /* for use by gawk */
} awk_two_way_processor_t;

#define gawk_api_major_version 3
#define gawk_api_minor_version 1

/* Current version of the API. */
enum {
	GAWK_API_MAJOR_VERSION = gawk_api_major_version,
	GAWK_API_MINOR_VERSION = gawk_api_minor_version
};

/* A number of typedefs related to different types of values. */

/*
 * A mutable string. Gawk owns the memory pointed to if it supplied
 * the value. Otherwise, it takes ownership of the memory pointed to.
 *
 * The API deals exclusively with regular chars; these strings may
 * be multibyte encoded in the current locale's encoding and character
 * set. Gawk will convert internally to wide characters if necessary.
 *
 * Note that a string provided by gawk will always be terminated
 * with a '\0' character.
 */
typedef struct awk_string {
	char *str;	/* data */
	size_t len;	/* length thereof, in chars */
} awk_string_t;

enum AWK_NUMBER_TYPE {
	AWK_NUMBER_TYPE_DOUBLE,
	AWK_NUMBER_TYPE_MPFR,
	AWK_NUMBER_TYPE_MPZ
};

typedef struct awk_number {
	double d;	/* always populated in data received from gawk */
	enum AWK_NUMBER_TYPE type;
	void *ptr;	/* either NULL or mpfr_ptr or mpz_ptr */
} awk_number_t;

/* Arrays are represented as an opaque type. */
typedef void *awk_array_t;

/* Scalars can be represented as an opaque type. */
typedef void *awk_scalar_t;

/* Any value can be stored as a cookie. */
typedef void *awk_value_cookie_t;

/*
 * This tag defines the type of a value.
 *
 * Values are associated with regular variables and with array elements.
 * Since arrays can be multidimensional (as can regular variables)
 * it's valid to have a "value" that is actually an array.
 */
typedef enum {
	AWK_UNDEFINED,
	AWK_NUMBER,
	AWK_STRING,
	AWK_REGEX,
	AWK_STRNUM,
	AWK_ARRAY,
	AWK_SCALAR,		/* opaque access to a variable */
	AWK_VALUE_COOKIE	/* for updating a previously created value */
} awk_valtype_t;

/*
 * An awk value. The val_type tag indicates what
 * is in the union.
 */
typedef struct awk_value {
	awk_valtype_t	val_type;
	union {
		awk_string_t	s;
		awk_number_t	n;
		awk_array_t	a;
		awk_scalar_t	scl;
		awk_value_cookie_t vc;
	} u;
#define str_value	u.s
#define strnum_value	str_value
#define regex_value	str_value
#define num_value	u.n.d
#define num_type	u.n.type
#define num_ptr		u.n.ptr
#define array_cookie	u.a
#define scalar_cookie	u.scl
#define value_cookie	u.vc
} awk_value_t;

/*
 * A "flattened" array element. Gawk produces an array of these
 * inside the awk_flat_array_t.
 * ALL memory pointed to belongs to gawk. Individual elements may
 * be marked for deletion. New elements must be added individually,
 * one at a time, using the separate API for that purpose.
 */

enum awk_element_flags {
	AWK_ELEMENT_DEFAULT = 0,	/* set by gawk */
	AWK_ELEMENT_DELETE = 1		/* set by extension if
					   should be deleted */
};

typedef struct awk_element {
	/* convenience linked list pointer, not used by gawk */
	struct awk_element *next;
	int flags; /* combination of enum awk_element_flags */
	awk_value_t	index;
	awk_value_t	value;
} awk_element_t;

/*
 * A "flattened" array. See the description above for how
 * to use the elements contained herein.
 */
typedef struct awk_flat_array {
	awk_const void *awk_const opaque1;	/* private data for use by gawk */
	awk_const void *awk_const opaque2;	/* private data for use by gawk */
	awk_const size_t count;			/* how many elements */
	awk_element_t elements[1];		/* will be extended */
} awk_flat_array_t;

/*
 * A record describing an extension function. Upon being
 * loaded, the extension should pass in one of these to gawk for
 * each C function.
 *
 * Each called function must fill in the result with either a scalar
 * (number, string, or regex). Gawk takes ownership of any string memory.
 *
 * The called function must return the value of `result'.
 * This is for the convenience of the calling code inside gawk.
 *
 * Each extension function may decide what to do if the number of
 * arguments isn't what it expected.  Following awk functions, it
 * is likely OK to ignore extra arguments.
 *
 * 'min_required_args' indicates how many arguments MUST be passed.
 * The API will throw a fatal error if not enough are passed.
 *
 * 'max_expected_args' is more benign; if more than that are passed,
 * the API prints a lint message (IFF lint is enabled, of course).
 *
 * In any case, the extension function itself need not compare the
 * actual number of arguments passed to those two values if it does
 * not want to.
 */
typedef struct awk_ext_func {
	const char *name;
	awk_value_t *(*const function)(int num_actual_args,
					awk_value_t *result,
					struct awk_ext_func *finfo);
	const size_t max_expected_args;
	const size_t min_required_args;
	awk_bool_t suppress_lint;
	void *data;		/* opaque pointer to any extra state */
} awk_ext_func_t;

typedef void *awk_ext_id_t;	/* opaque type for extension id */

/*
 * The API into gawk. Lots of functions here. We hope that they are
 * logically organized.
 *
 * !!! If you make any changes to this structure, please remember to bump !!!
 * !!! gawk_api_major_version and/or gawk_api_minor_version.              !!!
 */
typedef struct gawk_api {
	/* First, data fields. */

	/* These are what gawk thinks the API version is. */
	awk_const int major_version;
	awk_const int minor_version;

	/* GMP/MPFR versions, if extended-precision is available */
	awk_const int gmp_major_version;
	awk_const int gmp_minor_version;
	awk_const int mpfr_major_version;
	awk_const int mpfr_minor_version;

	/*
	 * These can change on the fly as things happen within gawk.
	 * Currently only do_lint is prone to change, but we reserve
	 * the right to allow the others to do so also.
	 */
#define DO_FLAGS_SIZE	6
	awk_const int do_flags[DO_FLAGS_SIZE];
/* Use these as indices into do_flags[] array to check the values */
#define gawk_do_lint		0
#define gawk_do_traditional	1
#define gawk_do_profile		2
#define gawk_do_sandbox		3
#define gawk_do_debug		4
#define gawk_do_mpfr		5

	/* Next, registration functions: */

	/*
	 * Add a function to the interpreter, returns true upon success. 
	 * Gawk does not modify what func points to, but the extension
	 * function itself receives this pointer and can modify what it
	 * points to, thus it's not const.
	 */
	awk_bool_t (*api_add_ext_func)(awk_ext_id_t id, const char *name_space,
			awk_ext_func_t *func);

	/* Register an input parser, for opening files read-only */
	void (*api_register_input_parser)(awk_ext_id_t id,
					awk_input_parser_t *input_parser);

	/* Register an output wrapper, for writing files */
	void (*api_register_output_wrapper)(awk_ext_id_t id,
					awk_output_wrapper_t *output_wrapper);

	/* Register a processor for two way I/O */
	void (*api_register_two_way_processor)(awk_ext_id_t id,
				awk_two_way_processor_t *two_way_processor);

	/*
	 * Add an exit call back.
	 *
	 * arg0 is a private data pointer for use by the extension;
	 * gawk saves it and passes it into the function pointed
	 * to by funcp at exit.
	 *
	 * Exit callback functions are called in LIFO order.
	 */
	void (*api_awk_atexit)(awk_ext_id_t id,
			void (*funcp)(void *data, int exit_status),
			void *arg0);

	/* Register a version string for this extension with gawk. */
	void (*api_register_ext_version)(awk_ext_id_t id, const char *version);

	/* Functions to print messages */
	void (*api_fatal)(awk_ext_id_t id, const char *format, ...);
	void (*api_warning)(awk_ext_id_t id, const char *format, ...);
	void (*api_lintwarn)(awk_ext_id_t id, const char *format, ...);
	void (*api_nonfatal)(awk_ext_id_t id, const char *format, ...);

	/* Functions to update ERRNO */
	void (*api_update_ERRNO_int)(awk_ext_id_t id, int errno_val);
	void (*api_update_ERRNO_string)(awk_ext_id_t id, const char *string);
	void (*api_unset_ERRNO)(awk_ext_id_t id);

	/*
	 * All of the functions that return a value from inside gawk
	 * (get a parameter, get a global variable, get an array element)
	 * behave in the same way.
	 *
	 * For a function parameter, the return is false if the argument
	 * count is out of range, or if the actual parameter does not match
	 * what is specified in wanted. In that case,  result->val_type
	 * will hold the actual type of what was passed.
	 *
	 * Similarly for symbol table access to variables and array elements,
	 * the return is false if the actual variable or array element does
	 * not match what was requested, and result->val_type will hold
	 * the actual type.

	Table entry is type returned:


	                        +-------------------------------------------------------+
	                        |                   Type of Actual Value:               |
	                        +--------+--------+--------+--------+-------+-----------+
	                        | String | Strnum | Number | Regex  | Array | Undefined |
	+-----------+-----------+--------+--------+--------+--------+-------+-----------+
	|           | String    | String | String | String | String | false | false     |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Strnum    | false  | Strnum | Strnum | false  | false | false     |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Number    | Number | Number | Number | false  | false | false     |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Regex     | false  | false  | false  | Regex  | false | false     |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|   Type    | Array     | false  | false  | false  | false  | Array | false     |
	| Requested +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Scalar    | Scalar | Scalar | Scalar | Scalar | false | false     |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Undefined | String | Strnum | Number | Regex  | Array | Undefined |
	|           +-----------+--------+--------+--------+--------+-------+-----------+
	|           | Value     | false  | false  | false  | false  | false | false     |
	|           | Cookie    |        |        |        |        |       |           |
	+-----------+-----------+--------+--------+--------+--------+-------+-----------+
	*/

	/* Functions to handle parameters passed to the extension. */

	/*
	 * Get the count'th parameter, zero-based.
	 * Returns false if count is out of range, or if actual parameter
	 * does not match what is specified in wanted. In that case,
	 * result->val_type is as described above.
	 */
	awk_bool_t (*api_get_argument)(awk_ext_id_t id, size_t count,
					  awk_valtype_t wanted,
					  awk_value_t *result);

	/*
	 * Convert a parameter that was undefined into an array
	 * (provide call-by-reference for arrays).  Returns false
	 * if count is too big, or if the argument's type is
	 * not undefined.
	 */
	awk_bool_t (*api_set_argument)(awk_ext_id_t id,
					size_t count,
					awk_array_t array);

	/*
	 * Symbol table access:
	 * 	- Read-only access to special variables (NF, etc.)
	 * 	- One special exception: PROCINFO.
	 *	- Use sym_update() to change a value, including from UNDEFINED
	 *	  to scalar or array.
	 */
	/*
	 * Lookup a variable, fill in value. No messing with the value
	 * returned.
	 * Returns false if the variable doesn't exist or if the wrong type
	 * was requested.  In the latter case, vaule->val_type will have
	 * the real type, as described above.
	 *
	 * 	awk_value_t val;
	 * 	if (! api->sym_lookup(id, name, wanted, & val))
	 * 		error_code_here();
	 *	else {
	 *		// safe to use val
	 *	}
	 */
	awk_bool_t (*api_sym_lookup)(awk_ext_id_t id,
				const char *name_space,
				const char *name,
				awk_valtype_t wanted,
				awk_value_t *result);

	/*
	 * Update a value. Adds it to the symbol table if not there.
	 * Changing types (scalar <--> array) is not allowed.
	 * In fact, using this to update an array is not allowed, either.
	 * Such an attempt returns false.
	 */
	awk_bool_t (*api_sym_update)(awk_ext_id_t id,
				const char *name_space,
				const char *name,
				awk_value_t *value);

	/*
	 * A ``scalar cookie'' is an opaque handle that provide access
	 * to a global variable or array. It is an optimization that
	 * avoids looking up variables in gawk's symbol table every time
	 * access is needed.
	 *
	 * This function retrieves the current value of a scalar cookie.
	 * Once you have obtained a scalar_cookie using sym_lookup, you can
	 * use this function to get its value more efficiently.
	 *
	 * Return will be false if the value cannot be retrieved.
	 *
	 * Flow is thus
	 *	awk_value_t val;
	 * 	awk_scalar_t cookie;
	 * 	api->sym_lookup(id, "variable", AWK_SCALAR, & val);	// get the cookie
	 *	cookie = val.scalar_cookie;
	 *	...
	 *	api->sym_lookup_scalar(id, cookie, wanted, & val);	// get the value
	 */
	awk_bool_t (*api_sym_lookup_scalar)(awk_ext_id_t id,
				awk_scalar_t cookie,
				awk_valtype_t wanted,
				awk_value_t *result);

	/*
	 * Update the value associated with a scalar cookie.
	 * Flow is
	 * 	sym_lookup with wanted == AWK_SCALAR
	 * 	if returns false
	 * 		sym_update with real initial value to install it
	 * 		sym_lookup again with AWK_SCALAR
	 *	else
	 *		use the scalar cookie
	 *
	 * Return will be false if the new value is not one of
	 * AWK_STRING, AWK_NUMBER, AWK_REGEX.
	 *
	 * Here too, the built-in variables may not be updated.
	 */
	awk_bool_t (*api_sym_update_scalar)(awk_ext_id_t id,
				awk_scalar_t cookie, awk_value_t *value);

	/* Cached values */

	/*
	 * Create a cached string,regex, or numeric value for efficient later
	 * assignment. This improves performance when you want to assign
	 * the same value to one or more variables repeatedly.  Only
	 * AWK_NUMBER, AWK_STRING, AWK_REGEX and AWK_STRNUM values are allowed.
	 * Any other type is rejected.  We disallow AWK_UNDEFINED since that
	 * case would result in inferior performance.
	 */
	awk_bool_t (*api_create_value)(awk_ext_id_t id, awk_value_t *value,
		    awk_value_cookie_t *result);

	/*
	 * Release the memory associated with a cookie from api_create_value.
	 * Please call this to free memory when the value is no longer needed.
	 */
	awk_bool_t (*api_release_value)(awk_ext_id_t id, awk_value_cookie_t vc);

	/* Array management */

	/*
	 * Retrieve total number of elements in array.
	 * Returns false if some kind of error.
	 */
	awk_bool_t (*api_get_element_count)(awk_ext_id_t id,
			awk_array_t a_cookie, size_t *count);

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
	 * index and value.
	 *
	 * ARGV and ENVIRON may not be updated.
	 */
	awk_bool_t (*api_set_array_element)(awk_ext_id_t id, awk_array_t a_cookie,
					const awk_value_t *const index,
					const awk_value_t *const value);

	/*
	 * Remove the element with the given index.
	 * Returns true if removed or false if element did not exist.
	 */
	awk_bool_t (*api_del_array_element)(awk_ext_id_t id,
			awk_array_t a_cookie, const awk_value_t* const index);

	/* Create a new array cookie to which elements may be added. */
	awk_array_t (*api_create_array)(awk_ext_id_t id);

	/* Clear out an array. */
	awk_bool_t (*api_clear_array)(awk_ext_id_t id, awk_array_t a_cookie);

	/*
	 * Flatten out an array with type conversions as requested.
	 * This supersedes the earlier api_flatten_array function that
	 * did not allow the caller to specify the requested types.
	 * (That API is still available as a macro, defined below.)
	 */
	awk_bool_t (*api_flatten_array_typed)(awk_ext_id_t id,
			awk_array_t a_cookie,
			awk_flat_array_t **data,
			awk_valtype_t index_type, awk_valtype_t value_type);

	/* When done, delete any marked elements, release the memory. */
	awk_bool_t (*api_release_flattened_array)(awk_ext_id_t id,
			awk_array_t a_cookie,
			awk_flat_array_t *data);

	/*
	 * Hooks to provide access to gawk's memory allocation functions.
	 * This ensures that memory passed between gawk and the extension
	 * is allocated and released by the same library.
	 */
	void *(*api_malloc)(size_t size);
	void *(*api_calloc)(size_t nmemb, size_t size);
	void *(*api_realloc)(void *ptr, size_t size);
	void (*api_free)(void *ptr);

	/*
	 * A function that returns mpfr data should call this function
	 * to allocate and initialize an mpfr_ptr for use in an
	 * awk_value_t structure that will be handed to gawk.
	 */
	void *(*api_get_mpfr)(awk_ext_id_t id);

	/*
	 * A function that returns mpz data should call this function
	 * to allocate and initialize an mpz_ptr for use in an
	 * awk_value_t structure that will be handed to gawk.
	 */
	void *(*api_get_mpz)(awk_ext_id_t id);

	/*
	 * Look up a file.  If the name is NULL or name_len is 0, it returns
	 * data for the currently open input file corresponding to FILENAME
	 * (and it will not access the filetype argument, so that may be
	 * undefined).
	 *
	 * If the file is not already open, try to open it.
	 *
	 * The "filetype" argument should be one of:
	 *
	 *    ">", ">>", "<", "|>", "|<", and "|&"
	 *
	 * If the file is not already open, and the fd argument is non-negative,
	 * gawk will use that file descriptor instead of opening the file
	 * in the usual way. The fd must be opened via gawk_api_open().
	 *
	 * If the fd is non-negative, but the file exists already, gawk
	 * ignores the fd and returns the existing file.  It is the caller's
	 * responsibility to notice that the fd in the returned
	 * awk_input_buf_t does not match the requested value.
	 *
	 * Note that supplying a file descriptor is currently NOT supported
	 * for pipes. It should work for input, output, append, and two-way
	 * (coprocess) sockets.  If the filetype is two-way, we assume that
	 * it is a socket!
	 *
	 * Note that in the two-way case, the input and output file descriptors
	 * may differ.  To check for success, one must check that either of
	 * them matches.
	 *
	 * ibufp and obufp point at gawk's internal copies of the
	 * awk_input_buf_t and awk_output_t associated with the open
	 * file.  Treat these data structures as read-only!
	 */
	awk_bool_t (*api_get_file)(awk_ext_id_t id,
			const char *name,
			size_t name_len,
			const char *filetype,
			/*
			 * If non-negative, must be opened via gawk_api_open()
			 */
			fd_t fd,
			/*
			 * Return values (on success, one or both should
			 * be non-NULL):
			 */
			const awk_input_buf_t **ibufp,
			const awk_output_buf_t **obufp);

	/* Just print message to stdout */
	int (*api_printf)(const char *format, ...);

	/* Assert failed: print message and abort the program */
	void (*api_assert_failed)(const char *sexpr, const char *file, unsigned line);

	/*
	 * Extensions must use standard streams of the gawk executable
	 * (instead the ones provided by the statically linked runtime library).
	 */
	FILE *api_stdin;
	FILE *api_stdout;
	FILE *api_stderr;

	/*
	 * File descriptors passed between gawk and the extension must be
	 * opened and closed in the same CRT library domain (which could
	 * be statically linked to the gawk executable).
	 */
	int (*api_open)(const char *name, int flags, ...);
	int (*api_close)(int fd);
	int (*api_dup)(int oldfd);
	int (*api_dup2)(int oldfd, int newfd);

	/* Locale-dependent functions should also be referenced from gawk
	 * executable, or the extension must set locale (via setlocale() call)
	 * of extension's own statically linked CRT library */
	int (*api_fflush)(FILE *stream);
	int (*api_fgetpos)(FILE *stream, fpos_t *pos);
	int (*api_fsetpos)(FILE *stream, const fpos_t *pos);
	void (*api_rewind)(FILE *stream);
	int (*api_fseek)(FILE *stream, long offset, int whence);
	long (*api_ftell)(FILE *stream);

	FILE *(*api_fopen)(const char *filename, const char *mode);
	int (*api_fclose)(FILE *stream);
	size_t (*api_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t (*api_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);

	int (*api_putchar)(int c);
	int (*api_fputc)(int c, FILE *stream);
	int (*api_getchar)(void);
	int (*api_fgetc)(FILE *stream);
	int (*api_puts)(const char *s);
	int (*api_fputs)(const char *s, FILE *stream);

	char *(*api_fgets)(char *s, int size, FILE *stream);
	int (*api_ungetc)(int c, FILE *stream);

	int (*api_mkdir)(const char *dirname);
	int (*api_rmdir)(const char *dirname);
	int (*api_remove)(const char *pathname);
	int (*api_unlink)(const char *pathname);
	int (*api_rename)(const char *old_name, const char *new_name);
	int (*api_stat)(const char *path, struct stat *buf);
	int (*api_fstat)(int fd, struct stat *buf);
	int (*api_chmod)(const char *path, int mode);

	char *(*api_setlocale)(int category, const char *locale);

	int (*api_strcoll)(const char *s1, const char *s2);
	int (*api_wcscoll)(const wchar_t *s1, const wchar_t *s2);

	int (*api_mb_cur_max)(void);

	wint_t (*api_btowc)(int c);

	int (*api_mblen)(const char *mbchar, size_t count);
	int (*api_mbtowc)(wchar_t *wchar, const char *mbchar, size_t count);
	int (*api_wctomb)(char *mbchar, wchar_t wchar);

	size_t (*api_mbstowcs)(wchar_t *wcstr, const char *mbstr, size_t count);
	size_t (*api_wcstombs)(char *mbstr, const wchar_t *wcstr, size_t count);

	int (*api_fprintf)(FILE *stream, const char *format, ...);
	int (*api_sprintf)(char *str, const char *format, ...);
	int (*api_snprintf)(char *str, size_t size, const char *format, ...);

	int (*api_vprintf)(const char *format, va_list ap);
	int (*api_vfprintf)(FILE *stream, const char *format, va_list ap);
	int (*api_vsprintf)(char *str, const char *format, va_list ap);
	int (*api_vsnprintf)(char *str, size_t size, const char *format, va_list ap);

	int *(*api_errno_p)(void);
	char *(*api_strerror)(int error_number);

	int (*api_mkstemp)(char *templ);

	char *(*api_getenv)(const char *name);

	int (*api_tolower)(int c);
	int (*api_toupper)(int c);
	int (*api_isascii)(int c);
	int (*api_isalnum)(int c);
	int (*api_isalpha)(int c);
	int (*api_isblank)(int c);
	int (*api_iscntrl)(int c);
	int (*api_isdigit)(int c);
	int (*api_isgraph)(int c);
	int (*api_islower)(int c);
	int (*api_isprint)(int c);
	int (*api_ispunct)(int c);
	int (*api_isspace)(int c);
	int (*api_isupper)(int c);
	int (*api_isxdigit)(int c);

	wint_t (*api_towlower)(wint_t c);
	wint_t (*api_towupper)(wint_t c);
	int (*api_iswascii)(wint_t c);
	int (*api_iswalnum)(wint_t c);
	int (*api_iswalpha)(wint_t c);
	int (*api_iswblank)(wint_t c);
	int (*api_iswcntrl)(wint_t c);
	int (*api_iswdigit)(wint_t c);
	int (*api_iswgraph)(wint_t c);
	int (*api_iswlower)(wint_t c);
	int (*api_iswprint)(wint_t c);
	int (*api_iswpunct)(wint_t c);
	int (*api_iswspace)(wint_t c);
	int (*api_iswupper)(wint_t c);
	int (*api_iswxdigit)(wint_t c);

	wctype_t (*api_wctype)(char const *name);
	int (*api_iswctype)(wint_t c, wctype_t mask);

} gawk_api_t;

typedef struct gawk_extension_api_ver {
	const char *api_name;
	struct {
		int major_version;
		int minor_version;
	} need;
	struct {
		int major_version;
		int minor_version;
	} have;
} gawk_extension_api_ver_t;

#ifndef GAWK	/* these are not for the gawk code itself! */
/*
 * Use these if you want to define "global" variables named api
 * and ext_id to make the code a little easier to read.
 * See the sample boilerplate code, below.
 */
#define do_lint		(gawk_api()->do_flags[gawk_do_lint])
#define do_traditional	(gawk_api()->do_flags[gawk_do_traditional])
#define do_profile	(gawk_api()->do_flags[gawk_do_profile])
#define do_sandbox	(gawk_api()->do_flags[gawk_do_sandbox])
#define do_debug	(gawk_api()->do_flags[gawk_do_debug])
#define do_mpfr		(gawk_api()->do_flags[gawk_do_mpfr])

#define get_argument(count, wanted, result) \
	(gawk_api()->api_get_argument(ext_id, count, wanted, result))
#define set_argument(count, new_array) \
	(gawk_api()->api_set_argument(ext_id, count, new_array))

#if defined(_MSC_VER) && defined(_PREFAST_)
/* Annotate printf-like format string so compiler will check passed args.  */
__declspec(noreturn)
void fatal(awk_ext_id_t id, _Printf_format_string_ const char *format, ...);
void nonfatal(awk_ext_id_t id, _Printf_format_string_ const char *format, ...);
void warning(awk_ext_id_t id, _Printf_format_string_ const char *format, ...);
void lintwarn(awk_ext_id_t id, _Printf_format_string_ const char *format, ...);
int gawk_api_printf(_Printf_format_string_ const char *format, ...);
int gawk_api_fprintf(FILE *file, _Printf_format_string_ const char *format, ...);
int gawk_api_sprintf(char *str, _Printf_format_string_ const char *format, ...);
int gawk_api_snprintf(char *str, size_t size, _Printf_format_string_ const char *format, ...);
__declspec(noreturn)
void api_fatal_(const gawk_api_t *api, awk_ext_id_t id, _Printf_format_string_ const char *format, ...);
#else /* !(_MSC_VER && _PREFAST_) */
#define fatal			gawk_api()->api_fatal
#define nonfatal		gawk_api()->api_nonfatal
#define warning			gawk_api()->api_warning
#define lintwarn		gawk_api()->api_lintwarn
#define gawk_api_printf		gawk_api()->api_printf
#define gawk_api_fprintf	gawk_api()->api_fprintf
#define gawk_api_sprintf	gawk_api()->api_sprintf
#define gawk_api_snprintf	gawk_api()->api_snprintf
#define api_fatal_(api, id, format, message, param) api->api_fatal(id, format, message, param)
#endif /* !(_MSC_VER && _PREFAST_) */

#ifndef NDEBUG
#define gawk_api_assert(expr) ((void)((expr) ? 1 : \
	(gawk_api()->api_assert_failed(#expr, __FILE__, __LINE__), 0)))
#else
#define gawk_api_assert(expr) ((void)0)
#endif

#define register_input_parser(parser) \
	(gawk_api()->api_register_input_parser(ext_id, parser))
#define register_output_wrapper(wrapper) \
	(gawk_api()->api_register_output_wrapper(ext_id, wrapper))
#define register_two_way_processor(processor) \
	(gawk_api()->api_register_two_way_processor(ext_id, processor))

#define update_ERRNO_int(e) \
	(gawk_api()->api_update_ERRNO_int(ext_id, e))
#define update_ERRNO_string(str) \
	(gawk_api()->api_update_ERRNO_string(ext_id, str))
#define unset_ERRNO() \
	(gawk_api()->api_unset_ERRNO(ext_id))

#define add_ext_func(ns, func)	(gawk_api()->api_add_ext_func(ext_id, ns, func))
#define awk_atexit(funcp, arg0)	(gawk_api()->api_awk_atexit(ext_id, funcp, arg0))

#define sym_lookup(name, wanted, result) \
	sym_lookup_ns("", name, wanted, result)
#define sym_update(name, value) \
	sym_update_ns("", name, value)

#define sym_lookup_ns(name_space, name, wanted, result) \
	(gawk_api()->api_sym_lookup(ext_id, name_space, name, wanted, result))
#define sym_update_ns(name_space, name, value) \
	(gawk_api()->api_sym_update(ext_id, name_space, name, value))

#define sym_lookup_scalar(scalar_cookie, wanted, result) \
	(gawk_api()->api_sym_lookup_scalar(ext_id, scalar_cookie, wanted, result))
#define sym_update_scalar(scalar_cookie, value) \
	(gawk_api()->api_sym_update_scalar)(ext_id, scalar_cookie, value)

#define get_array_element(array, index, wanted, result) \
	(gawk_api()->api_get_array_element(ext_id, array, index, wanted, result))

#define set_array_element(array, index, value) \
	(gawk_api()->api_set_array_element(ext_id, array, index, value))

static inline awk_bool_t
r_set_array_element_by_elem(const gawk_api_t *api,
			    awk_ext_id_t ext_id,
			    awk_array_t array,
			    const struct awk_element *elem)
{
	return api->api_set_array_element(ext_id,
					  array,
					  &elem->index,
					  &elem->value);
}

#define set_array_element_by_elem(array, elem) \
	r_set_array_element_by_elem(gawk_api(), ext_id, array, elem)

#define del_array_element(array, index) \
	(gawk_api()->api_del_array_element(ext_id, array, index))

#define get_element_count(array, count_p) \
	(gawk_api()->api_get_element_count(ext_id, array, count_p))

#define create_array()		(gawk_api()->api_create_array(ext_id))

#define clear_array(array)	(gawk_api()->api_clear_array(ext_id, array))

#define flatten_array_typed(array, data, index_type, value_type) \
	(gawk_api()->api_flatten_array_typed(ext_id, array, data, index_type, value_type))

#define flatten_array(array, data) \
	flatten_array_typed(array, data, AWK_STRING, AWK_UNDEFINED)

#define release_flattened_array(array, data) \
	(gawk_api()->api_release_flattened_array(ext_id, array, data))

#define gawk_api_malloc(size)		(gawk_api()->api_malloc(size))
#define gawk_api_calloc(nmemb, size)	(gawk_api()->api_calloc(nmemb, size))
#define gawk_api_realloc(ptr, size)	(gawk_api()->api_realloc(ptr, size))
#define gawk_api_free(ptr)		(gawk_api()->api_free(ptr))

#define gawk_api_stdin()		(gawk_api()->api_stdin)
#define gawk_api_stdout()		(gawk_api()->api_stdout)
#define gawk_api_stderr()		(gawk_api()->api_stderr)

static inline int
r_gawk_close(const gawk_api_t *api,
	     fd_t fd)
{
	return api->api_close(fd);
}

static inline fd_t
r_gawk_dup(const gawk_api_t *api,
	     fd_t oldfd)
{
	return api->api_dup(oldfd);
}

static inline fd_t
r_gawk_dup2(const gawk_api_t *api,
	     fd_t oldfd, fd_t newfd)
{
	return api->api_dup2(oldfd, newfd);
}

static inline int
r_gawk_fstat(const gawk_api_t *api,
	     fd_t fd, struct stat *buf)
{
	return api->api_fstat(fd, buf);
}

#define gawk_api_open				(fd_t) gawk_api()->api_open
#define gawk_api_close(fd)			r_gawk_close(gawk_api(), fd)
#define gawk_api_dup(oldfd)			r_gawk_dup(gawk_api(), oldfd)
#define gawk_api_dup2(oldfd, newfd)		r_gawk_dup2(gawk_api(), oldfd, newfd)

#define gawk_api_fflush(stream)			(gawk_api()->api_fflush(stream))
#define gawk_api_fgetpos(stream, pos)		(gawk_api()->api_fgetpos(stream, pos))
#define gawk_api_fsetpos(stream, pos)		(gawk_api()->api_fsetpos(stream, pos))
#define gawk_api_rewind(stream)			(gawk_api()->api_rewind(stream))
#define gawk_api_fseek(stream, offset, whence)	(gawk_api()->api_fseek(stream, offset, whence))
#define gawk_api_ftell(stream)			(gawk_api()->api_ftell(stream))

#define gawk_api_fopen(filename, mode)		(gawk_api()->api_fopen(filename, mode))
#define gawk_api_fclose(stream)			(gawk_api()->api_fclose(stream))

#define gawk_api_fread(ptr, size, nmemb, stream) \
	(gawk_api()->api_fread(ptr, size, nmemb, stream))
#define gawk_api_fwrite(ptr, size, nmemb, stream) \
	(gawk_api()->api_fwrite(ptr, size, nmemb, stream))

#define gawk_api_putchar(c)			(gawk_api()->api_putchar(c))
#define gawk_api_fputc(c, stream)		(gawk_api()->api_fputc(c, stream))
/* same as fputc */
#define gawk_api_putc(c, stream)		(gawk_api()->api_fputc(c, stream))
#define gawk_api_getchar()			(gawk_api()->api_getchar())
#define gawk_api_fgetc(stream)			(gawk_api()->api_fgetc(stream))
/* same as fgetc */
#define gawk_api_getc(stream)			(gawk_api()->api_fgetc(stream))
#define gawk_api_puts(s)			(gawk_api()->api_puts(s))
#define gawk_api_fputs(s, stream)		(gawk_api()->api_fputs(s, stream))

#define gawk_api_fgets(s, size, stream)		(gawk_api()->api_fgets(s, size, stream))
#define gawk_api_ungetc(c, stream)		(gawk_api()->api_ungetc(s, stream))

#define gawk_api_mkdir(dirname)			(gawk_api()->api_mkdir(dirname))
#define gawk_api_rmdir(dirname)			(gawk_api()->api_rmdir(dirname))
#define gawk_api_remove(pathname)		(gawk_api()->api_remove(pathname))
#define gawk_api_unlink(pathname)		(gawk_api()->api_unlink(pathname))
#define gawk_api_rename(old_name, new_name)	(gawk_api()->api_rename(old_name, new_name))
#define gawk_api_stat(path, buf)		(gawk_api()->api_stat(path, buf))
#define gawk_api_fstat(fd, buf)			r_gawk_fstat(gawk_api(), fd, buf)
#define gawk_api_chmod(path, mode)		(gawk_api()->api_chmod(path, mode))

#define gawk_api_setlocale(category, locale)	(gawk_api()->api_setlocale(category, locale))

#define gawk_api_strcoll(s1, s2)		(gawk_api()->api_strcoll(s1, s2))
#define gawk_api_wcscoll(s1, s2)		(gawk_api()->api_wcscoll(s1, s2))

#define gawk_api_mb_cur_max()			(gawk_api()->api_mb_cur_max())

#define gawk_api_btowc(c)			(gawk_api()->api_btowc(c))

#define gawk_api_mblen(mbchar, count)		(gawk_api()->api_mblen(mbchar, count))
#define gawk_api_mbtowc(wchar, mbchar, count)	(gawk_api()->api_mbtowc(wchar, mbchar, count))
#define gawk_api_wctomb(mbchar, wchar)		(gawk_api()->api_wctomb(mbchar, wchar))

#define gawk_api_mbstowcs(wcstr, mbstr, count)	(gawk_api()->api_mbstowcs(wcstr, mbstr, count))
#define gawk_api_wcstombs(mbstr, wcstr, count)	(gawk_api()->api_wcstombs(mbstr, wcstr, count))

#define gawk_api_vprintf(format, ap)		(gawk_api()->api_vprintf(format, ap))
#define gawk_api_vfprintf(stream, format, ap)	(gawk_api()->api_vfprintf(stream, format, ap))
#define gawk_api_vsprintf(str, format, ap)	(gawk_api()->api_vsprintf(str, format, ap))

#define gawk_api_vsnprintf(str, size, format, ap) \
	(gawk_api()->api_vsnprintf(str, size, format, ap))

#define gawk_api_errno_p()			(gawk_api()->api_errno_p())
#define gawk_api_strerror(error_number)		(gawk_api()->api_strerror(error_number))

static inline fd_t
r_gawk_mkstemp(const gawk_api_t *api,
	       char *templ)
{
	return api->api_mkstemp(templ);
}

#define gawk_api_mkstemp(templ)			r_gawk_mkstemp(gawk_api(), templ)

#define gawk_api_getenv(name)        		(gawk_api()->api_getenv(name))

#define gawk_api_tolower(c)			(gawk_api()->api_tolower(c))
#define gawk_api_toupper(c)			(gawk_api()->api_toupper(c))
#define gawk_api_isascii(c)			(gawk_api()->api_isascii(c))
#define gawk_api_isalnum(c)			(gawk_api()->api_isalnum(c))
#define gawk_api_isalpha(c)			(gawk_api()->api_isalpha(c))
#define gawk_api_isblank(c)			(gawk_api()->api_isblank(c))
#define gawk_api_iscntrl(c)			(gawk_api()->api_iscntrl(c))
#define gawk_api_isdigit(c)			(gawk_api()->api_isdigit(c))
#define gawk_api_isgraph(c)			(gawk_api()->api_isgraph(c))
#define gawk_api_islower(c)			(gawk_api()->api_islower(c))
#define gawk_api_isprint(c)			(gawk_api()->api_isprint(c))
#define gawk_api_ispunct(c)			(gawk_api()->api_ispunct(c))
#define gawk_api_isspace(c)			(gawk_api()->api_isspace(c))
#define gawk_api_isupper(c)			(gawk_api()->api_isupper(c))
#define gawk_api_isxdigit(c)			(gawk_api()->api_isxdigit(c))

#define gawk_api_towlower(c)			(gawk_api()->api_towlower(c))
#define gawk_api_towupper(c)			(gawk_api()->api_towupper(c))
#define gawk_api_iswascii(c)			(gawk_api()->api_iswascii(c))
#define gawk_api_iswalnum(c)			(gawk_api()->api_iswalnum(c))
#define gawk_api_iswalpha(c)			(gawk_api()->api_iswalpha(c))
#define gawk_api_iswblank(c)			(gawk_api()->api_iswblank(c))
#define gawk_api_iswcntrl(c)			(gawk_api()->api_iswcntrl(c))
#define gawk_api_iswdigit(c)			(gawk_api()->api_iswdigit(c))
#define gawk_api_iswgraph(c)			(gawk_api()->api_iswgraph(c))
#define gawk_api_iswlower(c)			(gawk_api()->api_iswlower(c))
#define gawk_api_iswprint(c)			(gawk_api()->api_iswprint(c))
#define gawk_api_iswpunct(c)			(gawk_api()->api_iswpunct(c))
#define gawk_api_iswspace(c)			(gawk_api()->api_iswspace(c))
#define gawk_api_iswupper(c)			(gawk_api()->api_iswupper(c))
#define gawk_api_iswxdigit(c)			(gawk_api()->api_iswxdigit(c))

#define gawk_api_wctype(name)			(gawk_api()->api_wctype(name))
#define gawk_api_iswctype(c, mask)		(gawk_api()->api_iswctype(c, mask))

#define create_value(value, result) \
	(gawk_api()->api_create_value(ext_id, value,result))

#define release_value(value) \
	(gawk_api()->api_release_value(ext_id, value))

#define get_file(name, namelen, filetype, fd, ibuf, obuf) \
	(gawk_api()->api_get_file(ext_id, name, namelen, filetype, fd, ibuf, obuf))

#define get_mpfr_ptr()	(gawk_api()->api_get_mpfr(ext_id))
#define get_mpz_ptr()	(gawk_api()->api_get_mpz(ext_id))

#define register_ext_version(version) \
	(gawk_api()->api_register_ext_version(ext_id, version))

#define emalloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) api->api_malloc(size)) == 0) \
			api_fatal_(api, ext_id, "%s: malloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define ezalloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) api->api_calloc(1, size)) == 0) \
			api_fatal_(api, ext_id, "%s: calloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define erealloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) api->api_realloc(pointer, size)) == 0) \
			api_fatal_(api, ext_id, "%s: realloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define emalloc(pointer, type, size, message)	emalloc_(gawk_api(), pointer, type, size, message)
#define ezalloc(pointer, type, size, message)	ezalloc_(gawk_api(), pointer, type, size, message)
#define erealloc(pointer, type, size, message)	erealloc_(gawk_api(), pointer, type, size, message)

/* Constructor functions */

/* r_make_string_type --- make a string or strnum or regexp value in result from the passed-in string */

static inline awk_value_t *
r_make_string_type(const gawk_api_t *api,	/* needed for emalloc */
		   awk_ext_id_t ext_id,		/* ditto */
		   const char *string,
		   size_t length,
		   awk_bool_t duplicate,
		   awk_value_t *result,
		   awk_valtype_t val_type)
{
	char *cp = NULL;

	memset(result, 0, sizeof(*result));

	result->val_type = val_type;
	result->str_value.len = length;

	if (duplicate) {
		emalloc_(api, cp, char *, length + 1, "r_make_string");
		memcpy(cp, string, length);
		cp[length] = '\0';
		result->str_value.str = cp;
	} else {
		result->str_value.str = (char *) string;
	}

	return result;
}

/* r_make_string --- make a string value in result from the passed-in string */

static inline awk_value_t *
r_make_string(const gawk_api_t *api,	/* needed for emalloc */
	      awk_ext_id_t ext_id,	/* ditto */
	      const char *string,
	      size_t length,
	      awk_bool_t duplicate,
	      awk_value_t *result)
{
	return r_make_string_type(api, ext_id, string, length, duplicate, result, AWK_STRING);
}

#define make_const_string(str, len, result) \
	r_make_string(gawk_api(), ext_id, str, len, awk_true, result)
#define make_malloced_string(str, len, result) \
	r_make_string(gawk_api(), ext_id, str, len, awk_false, result)

#define make_const_regex(str, len, result) \
	r_make_string_type(gawk_api(), ext_id, str, len, awk_true, result, AWK_REGEX)
#define make_malloced_regex(str, len, result) \
	r_make_string_type(gawk_api(), ext_id, str, len, awk_false, result, AWK_REGEX)

/*
 * Note: The caller may not create a STRNUM, but it can create a string that is
 * flagged as user input that MAY be a STRNUM. Gawk will decide whether it's a
 * STRNUM or a string by checking whether the string is numeric.
 */
#define make_const_user_input(str, len, result) \
	r_make_string_type(gawk_api(), ext_id, str, len, 1, result, AWK_STRNUM)
#define make_malloced_user_input(str, len, result) \
	r_make_string_type(gawk_api(), ext_id, str, len, 0, result, AWK_STRNUM)

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
	result->val_type = AWK_NUMBER;
	result->num_value = num;
	result->num_type = AWK_NUMBER_TYPE_DOUBLE;
	return result;
}

/*
 * make_number_mpz --- make an mpz number value in result.
 * The mpz_ptr must be from a call to get_mpz_ptr. Gawk will now
 * take ownership of this memory.
 */

static inline awk_value_t *
make_number_mpz(void *mpz_ptr, awk_value_t *result)
{
	result->val_type = AWK_NUMBER;
	result->num_type = AWK_NUMBER_TYPE_MPZ;
	result->num_ptr = mpz_ptr;
	return result;
}

/*
 * make_number_mpfr --- make an mpfr number value in result.
 * The mpfr_ptr must be from a call to get_mpfr_ptr. Gawk will now
 * take ownership of this memory.
 */

static inline awk_value_t *
make_number_mpfr(void *mpfr_ptr, awk_value_t *result)
{
	result->val_type = AWK_NUMBER;
	result->num_type = AWK_NUMBER_TYPE_MPFR;
	result->num_ptr = mpfr_ptr;
	return result;
}

#ifdef _MSC_VER
#define GAWK_PLUGIN_EXPORT __declspec(dllexport)
#else
#define GAWK_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
#define GAWK_PLUGIN_EXTERN_C_BEGIN extern "C" {
#define GAWK_PLUGIN_EXTERN_C_END }
#else
#define GAWK_PLUGIN_EXTERN_C_BEGIN
#define GAWK_PLUGIN_EXTERN_C_END
#endif

#ifdef _MSC_VER
/* Do not initialize static version of CRT library compiled in extension
  DLLs - they should use gawk API for all except stateless CRT functions,
  like memcpy, strlen, fileno, etc. */
#define EXTENSION_DLL_MAIN \
	int __stdcall ExtDllMain( \
		void * hinstDLL, \
		unsigned long fdwReason, \
		void *lpvReserved) \
	{ \
		(void) hinstDLL, (void) fdwReason, (void) lpvReserved; \
		return 1; \
	}
#else
#define EXTENSION_DLL_MAIN
#endif /* _MSC_VER */

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

GAWK_PLUGIN_EXTERN_C_BEGIN
GAWK_PLUGIN_EXPORT
extern int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id);
GAWK_PLUGIN_EXTERN_C_END

#if 0
/* Boilerplate code: */
#include "gawkapi.h"

GAWK_PLUGIN_GPL_COMPATIBLE

static gawk_api_t *const api;
static awk_ext_id_t ext_id;
static const char *ext_version = NULL; /* or ... = "some string" */

static awk_ext_func_t func_table[] = {
	{ "name", do_name, 1 },
	/* ... */
};

/* EITHER: */

static awk_bool_t (*init_func)(void) = NULL;

/* OR: */

static awk_bool_t
init_my_extension(void)
{
	...
}

static awk_bool_t (*init_func)(void) = init_my_extension;

dl_load_func(func_table, some_name, "name_space_in_quotes")
#endif

#define GAWK_PLUGIN_GPL_COMPATIBLE \
  GAWK_PLUGIN_EXTERN_C_BEGIN \
  GAWK_PLUGIN_EXPORT \
  int plugin_is_GPL_compatible; \
  GAWK_PLUGIN_EXTERN_C_END

#define gawk_check_api_version(name_, MAJOR_, MINOR_, major_, minor_) \
	if (major_ != MAJOR_ || minor_ < MINOR_) { \
		gawk_extension_api_ver_t *ver = *(gawk_extension_api_ver_t**) id; \
		ver->api_name = name_; \
		ver->need.major_version = MAJOR_; \
		ver->need.minor_version = MINOR_; \
		ver->have.major_version = major_; \
		ver->have.minor_version = minor_; \
		return -1; \
	} \


#define dl_load_func(func_table, extension, name_space) \
const gawk_api_t *gawk_api(void) {return api;} \
GAWK_PLUGIN_EXTERN_C_BEGIN \
EXTENSION_DLL_MAIN \
GAWK_PLUGIN_EXPORT \
int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id)  \
{ \
	size_t i, j; \
	int errors = 0; \
\
	api = api_p; \
	ext_id = (void **) id; \
\
	gawk_check_api_version(#extension, \
		GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION, \
		api->major_version, api->minor_version); \
\
	check_mpfr_version(extension); \
\
	/* load functions */ \
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) { \
		if (func_table[i].name == NULL) \
			break; \
		if (! add_ext_func(name_space, & func_table[i])) { \
			warning(ext_id, #extension ": could not add %s", \
					func_table[i].name); \
			errors++; \
		} \
	} \
\
	if (init_func != NULL) { \
		if (! init_func()) { \
			warning(ext_id, #extension ": initialization function failed"); \
			errors++; \
		} \
	} \
\
	if (ext_version != NULL) \
		register_ext_version(ext_version); \
\
	return (errors == 0); \
} \
GAWK_PLUGIN_EXTERN_C_END

#if defined __GNU_MP_VERSION && defined MPFR_VERSION_MAJOR
#define check_mpfr_version(extension) do { \
	gawk_check_api_version(#extension ": GMP", \
		__GNU_MP_VERSION, __GNU_MP_VERSION_MINOR, \
		api->gmp_major_version, api->gmp_minor_version); \
	gawk_check_api_version(#extension ": MPFR", \
		MPFR_VERSION_MAJOR, MPFR_VERSION_MINOR, \
		api->mpfr_major_version, api->mpfr_minor_version); \
} while (0)
#else
#define check_mpfr_version(extension) /* nothing */
#endif

/* Forward declaration - to be able to use gawk API in inline functions in
  headers included between "gawkapi.h" and the declaration of "api" pointer */
extern const gawk_api_t *gawk_api(void);

#ifndef GAWK_API_DONT_REDEFINE_CRT

/* Extension DLLs should acquire/release resources (memory, files) via the CRT
  library (possibly statically linked) of the main gawk executable - to be able
  to pass these resources to the gawk executable and to allow it to manage them. */

#ifdef malloc
#undef malloc
#endif
#define malloc(size) gawk_api_malloc(size)

#ifdef calloc
#undef calloc
#endif
#define calloc(nmemb, size) gawk_api_calloc(nmemb, size)

#ifdef realloc
#undef realloc
#endif
#define realloc(ptr, size) gawk_api_realloc(ptr, size)

#ifdef free
#undef free
#endif
#define free(ptr) gawk_api_free(ptr)

#ifdef printf
#undef printf
#endif
#define printf gawk_api_printf

#ifdef assert
#undef assert
#endif
#define assert(expr) gawk_api_assert(expr)

#ifdef stdin
#undef stdin
#endif
#define stdin gawk_api_stdin()

#ifdef stdout
#undef stdout
#endif
#define stdout gawk_api_stdout()

#ifdef stderr
#undef stderr
#endif
#define stderr gawk_api_stderr()

#ifdef open
#undef open
#endif
#define open gawk_api_open

#ifdef close
#undef close
#endif
#define close(fd) gawk_api_close(fd)

#ifdef dup
#undef dup
#endif
#define dup(oldfd) gawk_api_dup(oldfd)

#ifdef dup2
#undef dup2
#endif
#define dup2(oldfd, newfd) gawk_api_dup2(oldfd, newfd)

#ifdef fflush
#undef fflush
#endif
#define fflush(stream) gawk_api_fflush(stream)

#ifdef fgetpos
#undef fgetpos
#endif
#define fgetpos(stream, pos) gawk_api_fgetpos(stream, pos)

#ifdef fsetpos
#undef fsetpos
#endif
#define fsetpos(stream, pos) gawk_api_fsetpos(stream, pos)

#ifdef rewind
#undef rewind
#endif
#define rewind(stream) gawk_api_rewind(stream)

#ifdef fseek
#undef fseek
#endif
#define fseek(stream, offset, whence) gawk_api_fseek(stream, offset, whence)

#ifdef ftell
#undef ftell
#endif
#define ftell(stream) gawk_api_ftell(stream)

#ifdef fopen
#undef fopen
#endif
#define fopen(filename, mode) gawk_api_fopen(filename, mode)

#ifdef fclose
#undef fclose
#endif
#define fclose(stream) gawk_api_fclose(stream)

#ifdef fread
#undef fread
#endif
#define fread(ptr, size, nmemb, stream) \
	gawk_api_fread(ptr, size, nmemb, stream)

#ifdef fwrite
#undef fwrite
#endif
#define fwrite(ptr, size, nmemb, stream) \
	gawk_api_fwrite(ptr, size, nmemb, stream)

#ifdef putchar
#undef putchar
#endif
#define putchar(c) gawk_api_putchar(c)

#ifdef fputc
#undef fputc
#endif
#define fputc(c, stream) gawk_api_fputc(c, stream)

#ifdef putc
#undef putc
#endif
#define putc(c, stream) gawk_api_putc(c, stream)

#ifdef getchar
#undef getchar
#endif
#define getchar() gawk_api_getchar()

#ifdef fgetc
#undef fgetc
#endif
#define fgetc(stream) gawk_api_fgetc(stream)

#ifdef getc
#undef getc
#endif
#define getc(stream) gawk_api_getc(stream)

#ifdef puts
#undef puts
#endif
#define puts(s) gawk_api_puts(s)

#ifdef fputs
#undef fputs
#endif
#define fputs(s, stream) gawk_api_fputs(s, stream)

#ifdef fgets
#undef fgets
#endif
#define fgets(s, size, stream) gawk_api_fgets(s, size, stream)

#ifdef ungetc
#undef ungetc
#endif
#define ungetc(c, stream) gawk_api_ungetc(c, stream)

#ifdef mkdir
#undef mkdir
#endif
#define mkdir(dirname) gawk_api_mkdir(dirname)

#ifdef rmdir
#undef rmdir
#endif
#define rmdir(dirname) gawk_api_rmdir(dirname)

#ifdef remove
#undef remove
#endif
#define remove(pathname) gawk_api_remove(pathname)

#ifdef unlink
#undef unlink
#endif
#define unlink(pathname) gawk_api_unlink(pathname)

#ifdef rename
#undef rename
#endif
#define rename(old_name, new_name) gawk_api_rename(old_name, new_name)

#ifdef stat
#undef stat
#endif
#define stat(path, buf) gawk_api_stat(path, buf)

#ifdef fstat
#undef fstat
#endif
#define fstat(fd, buf) gawk_api_fstat(fd, buf)

#ifdef chmod
#undef chmod
#endif
#define chmod(path, mode) gawk_api_chmod(path, mode)

#ifdef setlocale
#undef setlocale
#endif
#define setlocale(category, locale) gawk_api_setlocale(category, locale)

#ifdef strcoll
#undef strcoll
#endif
#define strcoll(s1, s2) gawk_api_strcoll(s1, s2)

#ifdef wcscoll
#undef wcscoll
#endif
#define wcscoll(s1, s2) gawk_api_wcscoll(s1, s2)

#ifdef MB_CUR_MAX
#undef MB_CUR_MAX
#endif
#define MB_CUR_MAX gawk_api_mb_cur_max()

#ifdef btowc
#undef btowc
#endif
#define btowc(c) gawk_api_btowc(c)

#ifdef mblen
#undef mblen
#endif
#define mblen(mbchar, count) gawk_api_mblen(mbchar, count)

#ifdef mbtowc
#undef mbtowc
#endif
#define mbtowc(wchar, mbchar, count) gawk_api_mbtowc(wchar, mbchar, count)

#ifdef wctomb
#undef wctomb
#endif
#define wctomb(mbchar, wchar) gawk_api_wctomb(mbchar, wchar)

#ifdef mbstowcs
#undef mbstowcs
#endif
#define mbstowcs(wcstr, mbstr, count) gawk_api_mbstowcs(wcstr, mbstr, count)

#ifdef wcstombs
#undef wcstombs
#endif
#define wcstombs(mbstr, wcstr, count) gawk_api_wcstombs(mbstr, wcstr, count)

#ifdef fprintf
#undef fprintf
#endif
#define fprintf gawk_api_fprintf

#ifdef sprintf
#undef sprintf
#endif
#define sprintf gawk_api_sprintf

#ifdef snprintf
#undef snprintf
#endif
#define snprintf gawk_api_snprintf

#ifdef vprintf
#undef vprintf
#endif
#define vprintf(format, ap) gawk_api_vprintf(format, ap)

#ifdef vfprintf
#undef vfprintf
#endif
#define vfprintf(stream, format, ap) gawk_api_vfprintf(stream, format, ap)

#ifdef vsprintf
#undef vsprintf
#endif
#define vsprintf(str, format, ap) gawk_api_vsprintf(str, format, ap)

#ifdef vsnprintf
#undef vsnprintf
#endif
#define vsnprintf(str, size, format, ap) gawk_api_vsnprintf(str, size, format, ap)

#ifdef errno
#undef errno
#endif
#define errno (*gawk_api_errno_p())

#ifdef strerror
#undef strerror
#endif
#define strerror(error_number) gawk_api_strerror(error_number)

#ifdef mkstemp
#undef mkstemp
#endif
#define mkstemp(templ) gawk_api_mkstemp(templ)

#ifdef getenv
#undef getenv
#endif
#define getenv(name) gawk_api_getenv(name)

#ifdef tolower
#undef tolower
#endif
#define tolower(c) gawk_api_tolower(c)

#ifdef toupper
#undef toupper
#endif
#define toupper(c) gawk_api_toupper(c)

#ifdef isascii
#undef isascii
#endif
#define isascii(c) gawk_api_isascii(c)

#ifdef isalnum
#undef isalnum
#endif
#define isalnum(c) gawk_api_isalnum(c)

#ifdef isalpha
#undef isalpha
#endif
#define isalpha(c) gawk_api_isalpha(c)

#ifdef isblank
#undef isblank
#endif
#define isblank(c) gawk_api_isblank(c)

#ifdef iscntrl
#undef iscntrl
#endif
#define iscntrl(c) gawk_api_iscntrl(c)

#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c) gawk_api_isdigit(c)

#ifdef isgraph
#undef isgraph
#endif
#define isgraph(c) gawk_api_isgraph(c)

#ifdef islower
#undef islower
#endif
#define islower(c) gawk_api_islower(c)

#ifdef isprint
#undef isprint
#endif
#define isprint(c) gawk_api_isprint(c)

#ifdef ispunct
#undef ispunct
#endif
#define ispunct(c) gawk_api_ispunct(c)

#ifdef isspace
#undef isspace
#endif
#define isspace(c) gawk_api_isspace(c)

#ifdef isupper
#undef isupper
#endif
#define isupper(c) gawk_api_isupper(c)

#ifdef isxdigit
#undef isxdigit
#endif
#define isxdigit(c) gawk_api_isxdigit(c)

#ifdef towlower
#undef towlower
#endif
#define towlower(c) gawk_api_towlower(c)

#ifdef towupper
#undef towupper
#endif
#define towupper(c) gawk_api_towupper(c)

#ifdef iswascii
#undef iswascii
#endif
#define iswascii(c) gawk_api_iswascii(c)

#ifdef iswalnum
#undef iswalnum
#endif
#define iswalnum(c) gawk_api_iswalnum(c)

#ifdef iswalpha
#undef iswalpha
#endif
#define iswalpha(c) gawk_api_iswalpha(c)

#ifdef iswblank
#undef iswblank
#endif
#define iswblank(c) gawk_api_iswblank(c)

#ifdef iswcntrl
#undef iswcntrl
#endif
#define iswcntrl(c) gawk_api_iswcntrl(c)

#ifdef iswdigit
#undef iswdigit
#endif
#define iswdigit(c) gawk_api_iswdigit(c)

#ifdef iswgraph
#undef iswgraph
#endif
#define iswgraph(c) gawk_api_iswgraph(c)

#ifdef iswlower
#undef iswlower
#endif
#define iswlower(c) gawk_api_iswlower(c)

#ifdef iswprint
#undef iswprint
#endif
#define iswprint(c) gawk_api_iswprint(c)

#ifdef iswpunct
#undef iswpunct
#endif
#define iswpunct(c) gawk_api_iswpunct(c)

#ifdef iswspace
#undef iswspace
#endif
#define iswspace(c) gawk_api_iswspace(c)

#ifdef iswupper
#undef iswupper
#endif
#define iswupper(c) gawk_api_iswupper(c)

#ifdef iswxdigit
#undef iswxdigit
#endif
#define iswxdigit(c) gawk_api_iswxdigit(c)

#ifdef wctype
#undef wctype
#endif
#define wctype(name) gawk_api_wctype(name)

#ifdef iswctype
#undef iswctype
#endif
#define iswctype(c, mask) gawk_api_iswctype(c, mask)

#endif /* !GAWK_API_DONT_REDEFINE_CRT */

#endif /* GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
