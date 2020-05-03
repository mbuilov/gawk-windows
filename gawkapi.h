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
 * 1. Due to gawk internals, after using sym_update() to install an array
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

#ifdef _MSC_VER
#include "pc/xstat.h"
#endif

/* Allow use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

#define gawk_api_major_version 4
#define gawk_api_minor_version 0

/* Current version of the API. */
enum {
	GAWK_API_MAJOR_VERSION = gawk_api_major_version,
	GAWK_API_MINOR_VERSION = gawk_api_minor_version
};

/*
 * If CRT (C runtime library) is statically linked to the main GAWK executable,
 * extension DLLs should use GAWK CRT API for the CRT calls and should not link
 * (statically or dynamically) other CRTs - to share single copy of internal
 * CRT data (file descriptor table, locale, heap) with the GAWK executable.
 */
#ifdef GAWK_STATIC_CRT
#define GAWK_CRT_API
#endif

#ifdef GAWK_CRT_API
/* Current version of the CRT API.  */
enum {
	GAWK_CRT_MAJOR_VERSION = 1,
	GAWK_CRT_MINOR_VERSION = 0
};
#endif

/* This is used to keep extensions from modifying certain fields in some structs. */
#ifdef GAWK
#define awk_const
#else
#define awk_const const
#endif

typedef enum awk_bool {
	awk_false = 0,
	awk_true  = 1
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

/* File or socket descriptor type.  */
typedef int fd_t;
#define INVALID_HANDLE (-1)

#ifdef _MSC_VER
/*
 * Microsoft's CRT library do not supports socket FDs, but because number of
 * file FDs is limited, use most significant bit of positive integer as a flag
 * denoting a socket descriptor.
 */
#define SOCKET_FD_BIT (((unsigned int)-1 >> 2) + 1)
static inline int is_socket(fd_t fd)
{
	return fd >= 0 && (fd & SOCKET_FD_BIT);
}
#endif

/* File status information.  */
#ifdef _MSC_VER
/* Fields st_size, st_atime, st_mtime, st_ctime are 64-bit values.   */
typedef struct _stat64 gawk_stat_t;
/* Extended stat info, 64-bit st_ino is valid.  */
typedef struct xstat gawk_xstat_t;
#else
typedef struct stat gawk_stat_t;
typedef struct stat gawk_xstat_t;
#endif

/* The information about input files that input parsers need to know: */
typedef struct awk_input {
	const char *name;	/* filename */
	fd_t fd;		/* file or socket descriptor */
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
	ssize_t (*read_func)(int fd, void *buffer, size_t count);

	/*
	 * The close_func is called to allow the parser to free private data.
	 * Gawk itself will close the fd unless close_func first sets it to
	 * INVALID_HANDLE.
	 */
	void (*close_func)(struct awk_input *iobuf);

	/* put last, for alignment. bleah */
	gawk_xstat_t sbuf;       /* stat buf */

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
	void *file;		/* stdio file or socket buffer (if socket_fd is valid) */
#ifdef _MSC_VER
	fd_t socket_fd;		/* socket descriptor, INVALID_HANDLE if not valid */
#endif
	awk_bool_t redirected;	/* true if a wrapper is active */
	void *opaque;		/* for use by output wrapper */

	/*
	 * Replacement functions for I/O.
	 * Use GAWK API for low-level output: outbuf_fwrite(), etc.
	 */
	size_t (*gawk_fwrite)(const void *buf, size_t size, size_t count,
				struct awk_output_buf *outbuf);
	int (*gawk_fflush)(struct awk_output_buf *outbuf);
	int (*gawk_ferror)(struct awk_output_buf *outbuf);
	int (*gawk_fclose)(struct awk_output_buf *outbuf);
} awk_output_buf_t;

static inline fd_t
awk_output_buf_get_fd(const awk_output_buf_t *outbuf)
{
#ifdef _MSC_VER
	if (outbuf->socket_fd != INVALID_HANDLE)
		return outbuf->socket_fd;
#endif
	return fileno((FILE*) outbuf->file);
}

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

struct lconv;
struct gawk_crt_api;

/*
 * The API into gawk. Lots of functions here. We hope that they are
 * logically organized.
 *
 * !!! If you make any changes to this structure, please remember to bump !!!
 * !!! gawk_api_major_version and/or gawk_api_minor_version.              !!!
 */
typedef struct gawk_api {
	/* These are what gawk thinks the API version is. */
	awk_const int major_version;
	awk_const int minor_version;

	/* GMP/MPFR versions, if extended-precision is available */
	awk_const int gmp_major_version;
	awk_const int gmp_minor_version;
	awk_const int mpfr_major_version;
	awk_const int mpfr_minor_version;

	/* CRT api, NULL if not supported.  */
	const struct gawk_crt_api *crt_api;

	/* First, data fields. */

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
	awk_bool_t (*api_add_ext_func)(const char *name_space,
			awk_ext_func_t *func);

	/* Register an input parser, for opening files read-only */
	void (*api_register_input_parser)(awk_input_parser_t *input_parser);

	/* Register an output wrapper, for writing files */
	void (*api_register_output_wrapper)(
				awk_output_wrapper_t *output_wrapper);

	/* Register a processor for two way I/O */
	void (*api_register_two_way_processor)(
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
	void (*api_awk_atexit)(void (*funcp)(void *data, int exit_status),
			void *arg0);

	/* Register a version string for this extension with gawk. */
	void (*api_register_ext_version)(const char *version);

	/* Functions to print messages */
	void (*api_fatal)(const char *format, ...);
	void (*api_warning)(const char *format, ...);
	void (*api_lintwarn)(const char *format, ...);
	void (*api_nonfatal)(const char *format, ...);

	/* Functions to update ERRNO */
	void (*api_update_ERRNO_int)(int errno_val);
	void (*api_update_ERRNO_string)(const char *string);
	void (*api_unset_ERRNO)(void);

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
	awk_bool_t (*api_get_argument)(size_t count,
					  awk_valtype_t wanted,
					  awk_value_t *result);

	/*
	 * Convert a parameter that was undefined into an array
	 * (provide call-by-reference for arrays).  Returns false
	 * if count is too big, or if the argument's type is
	 * not undefined.
	 */
	awk_bool_t (*api_set_argument)(size_t count,
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
	 * 	if (! api->sym_lookup(name, wanted, & val))
	 * 		error_code_here();
	 *	else {
	 *		// safe to use val
	 *	}
	 */
	awk_bool_t (*api_sym_lookup)(const char *name_space,
				const char *name,
				awk_valtype_t wanted,
				awk_value_t *result);

	/*
	 * Update a value. Adds it to the symbol table if not there.
	 * Changing types (scalar <--> array) is not allowed.
	 * In fact, using this to update an array is not allowed, either.
	 * Such an attempt returns false.
	 */
	awk_bool_t (*api_sym_update)(const char *name_space,
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
	 * 	api->sym_lookup("variable", AWK_SCALAR, & val);	// get the cookie
	 *	cookie = val.scalar_cookie;
	 *	...
	 *	api->sym_lookup_scalar(cookie, wanted, & val);	// get the value
	 */
	awk_bool_t (*api_sym_lookup_scalar)(awk_scalar_t cookie,
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
	awk_bool_t (*api_sym_update_scalar)(awk_scalar_t cookie,
					awk_value_t *value);

	/* Cached values */

	/*
	 * Create a cached string,regex, or numeric value for efficient later
	 * assignment. This improves performance when you want to assign
	 * the same value to one or more variables repeatedly.  Only
	 * AWK_NUMBER, AWK_STRING, AWK_REGEX and AWK_STRNUM values are allowed.
	 * Any other type is rejected.  We disallow AWK_UNDEFINED since that
	 * case would result in inferior performance.
	 */
	awk_bool_t (*api_create_value)(awk_value_t *value,
				awk_value_cookie_t *result);

	/*
	 * Release the memory associated with a cookie from api_create_value.
	 * Please call this to free memory when the value is no longer needed.
	 */
	awk_bool_t (*api_release_value)(awk_value_cookie_t vc);

	/* Array management */

	/*
	 * Retrieve total number of elements in array.
	 * Returns false if some kind of error.
	 */
	awk_bool_t (*api_get_element_count)(awk_array_t a_cookie,
					size_t *count);

	/*
	 * Return the value of an element - read only!
	 * Use set_array_element() to change it.
	 * Behavior for value and return is same as for api_get_argument
	 * and sym_lookup.
	 */
	awk_bool_t (*api_get_array_element)(awk_array_t a_cookie,
					const awk_value_t *const index,
					awk_valtype_t wanted,
					awk_value_t *result);

	/*
	 * Change (or create) element in existing array with
	 * index and value.
	 *
	 * ARGV and ENVIRON may not be updated.
	 */
	awk_bool_t (*api_set_array_element)(awk_array_t a_cookie,
					const awk_value_t *const index,
					const awk_value_t *const value);

	/*
	 * Remove the element with the given index.
	 * Returns true if removed or false if element did not exist.
	 */
	awk_bool_t (*api_del_array_element)(awk_array_t a_cookie,
					const awk_value_t* const index);

	/* Create a new array cookie to which elements may be added. */
	awk_array_t (*api_create_array)(void);

	/* Clear out an array. */
	awk_bool_t (*api_clear_array)(awk_array_t a_cookie);

	/*
	 * Flatten out an array with type conversions as requested.
	 * This supersedes the earlier api_flatten_array function that
	 * did not allow the caller to specify the requested types.
	 * (That API is still available as a macro, defined below.)
	 */
	awk_bool_t (*api_flatten_array_typed)(awk_array_t a_cookie,
					awk_flat_array_t **data,
					awk_valtype_t index_type,
					awk_valtype_t value_type);

	/* When done, delete any marked elements, release the memory. */
	awk_bool_t (*api_release_flattened_array)(awk_array_t a_cookie,
					awk_flat_array_t *data);

	/*
	 * A function that returns mpfr data should call this function
	 * to allocate and initialize an mpfr_ptr for use in an
	 * awk_value_t structure that will be handed to gawk.
	 */
	void *(*api_get_mpfr)(void);

	/*
	 * A function that returns mpz data should call this function
	 * to allocate and initialize an mpz_ptr for use in an
	 * awk_value_t structure that will be handed to gawk.
	 */
	void *(*api_get_mpz)(void);

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
	awk_bool_t (*api_get_file)(const char *name,
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

	/*
	 * Functions for use in output callbacks of struct awk_output_buf.
	 */
	int (*api_ob_fflush)(awk_output_buf_t *outbuf);
	size_t (*api_ob_fwrite)(const void *ptr, size_t size, size_t nmemb,
			awk_output_buf_t *outbuf);
	int (*api_ob_ferror)(awk_output_buf_t *outbuf);
	void (*api_ob_clearerr)(awk_output_buf_t *outbuf);
	int (*api_ob_fputc)(int c, awk_output_buf_t *outbuf);
	int (*api_ob_fputs)(const char *s, awk_output_buf_t *outbuf);
	int (*api_ob_fprintf)(awk_output_buf_t *outbuf, const char *format,
				...);
	int (*api_ob_vfprintf)(awk_output_buf_t *outbuf, const char *format,
				va_list ap);

} gawk_api_t;

#ifdef GAWK_CRT_API
/*
 * Replacements of CRT library functions.
 *
 * !!! If you make any changes to this structure, please remember to bump !!!
 * !!! GAWK_CRT_MAJOR_VERSION and/or GAWK_CRT_MINOR_VERSION.              !!!
 */
typedef struct gawk_crt_api {

	/* CRT api version */
	awk_const int crt_major_version;
	awk_const int crt_minor_version;

	/*
	 * Hooks to provide access to gawk's memory allocation functions.
	 * This ensures that memory passed between gawk and the extension
	 * is allocated and released by the same library.
	 */
	void *(*crt_malloc)(size_t size);
	void *(*crt_calloc)(size_t nmemb, size_t size);
	void *(*crt_realloc)(void *ptr, size_t size);
	void (*crt_free)(void *ptr);

	int (*crt_printf)(const char *format, ...);

	/* Assert failed: print message and abort the program */
	void (*crt_assert_failed)(const char *sexpr, const char *file,
			unsigned line);

	FILE *crt_stdin;
	FILE *crt_stdout;
	FILE *crt_stderr;

	int (*crt_open)(const char *name, int flags, ...);
	int (*crt_close)(int fd);
	int (*crt_dup)(int oldfd);
	int (*crt_dup2)(int oldfd, int newfd);

	ssize_t (*crt_read)(int fd, void *buf, size_t count);
	ssize_t (*crt_write)(int fd, const void *buf, size_t count);
	long long (*crt_lseek)(int fd, long long offset, int whence);
	long long (*crt_tell)(int fd);
	int (*crt_fsync)(int fd);

	int (*crt_fflush)(FILE *stream);
	int (*crt_fgetpos)(FILE *stream, fpos_t *pos);
	int (*crt_fsetpos)(FILE *stream, const fpos_t *pos);
	void (*crt_rewind)(FILE *stream);
	int (*crt_fseek)(FILE *stream, long offset, int whence);
	long (*crt_ftell)(FILE *stream);

	FILE *(*crt_fopen)(const char *filename, const char *mode);
	int (*crt_fclose)(FILE *stream);
	size_t (*crt_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t (*crt_fwrite)(const void *ptr, size_t size, size_t nmemb,
			FILE *stream);

	void (*crt_clearerr)(FILE *stream);
	int (*crt_feof)(FILE *stream);
	int (*crt_ferror)(FILE *stream);
	int (*crt_fileno)(FILE *stream);

	int (*crt_putchar)(int c);
	int (*crt_fputc)(int c, FILE *stream);
	int (*crt_getchar)(void);
	int (*crt_fgetc)(FILE *stream);
	int (*crt_puts)(const char *s);
	int (*crt_fputs)(const char *s, FILE *stream);

	char *(*crt_fgets)(char *s, int size, FILE *stream);
	int (*crt_ungetc)(int c, FILE *stream);

	int (*crt_mkdir)(const char *dirname);
	int (*crt_rmdir)(const char *dirname);
	int (*crt_remove)(const char *pathname);
	int (*crt_unlink)(const char *pathname);
	int (*crt_rename)(const char *old_name, const char *new_name);
	int (*crt_chdir)(const char *path);

#ifdef _MSC_VER
	wchar_t *(*crt_xpathwc)(const char *path, wchar_t buf[],
		size_t buf_size);
	int (*crt_xfstat)(void *h, const wchar_t *path, struct xstat *buf);
	int (*crt_xwstat)(const wchar_t *path, struct xstat *buf,
			int dont_follow);
	int (*crt_xstat)(const char *path, struct xstat *buf);
	int (*crt_xlstat)(const char *path, struct xstat *buf);
	int (*crt_xstat_root)(const wchar_t *path, struct xstat *buf);
#endif

	int (*crt_stat)(const char *path, gawk_stat_t *buf);
	int (*crt_fstat)(int fd, gawk_stat_t *buf);
	int (*crt_chmod)(const char *path, int mode);

	char *(*crt_setlocale)(int category, const char *locale);
	struct lconv *(*crt_localeconv)(void);

	int (*crt_strcoll)(const char *s1, const char *s2);
	int (*crt_wcscoll)(const wchar_t *s1, const wchar_t *s2);

	int (*crt_mb_cur_max)(void);

	wint_t (*crt_btowc)(int c);

	int (*crt_mblen)(const char *mbchar, size_t count);
	int (*crt_mbtowc)(wchar_t *wchar, const char *mbchar, size_t count);
	int (*crt_wctomb)(char *mbchar, wchar_t wchar);

	size_t (*crt_mbstowcs)(wchar_t *wcstr, const char *mbstr, size_t count);
	size_t (*crt_wcstombs)(char *mbstr, const wchar_t *wcstr, size_t count);

	int (*crt_fprintf)(FILE *stream, const char *format, ...);
	int (*crt_sprintf)(char *str, const char *format, ...);
	int (*crt_snprintf)(char *str, size_t size, const char *format, ...);

	int (*crt_vprintf)(const char *format, va_list ap);
	int (*crt_vfprintf)(FILE *stream, const char *format, va_list ap);
	int (*crt_vsprintf)(char *str, const char *format, va_list ap);
	int (*crt_vsnprintf)(char *str, size_t size, const char *format,
			va_list ap);

	int *(*crt_errno_p)(void);
	char *(*crt_strerror)(int error_number);

	int (*crt_mkstemp)(char *templ);

	char *(*crt_getenv)(const char *name);

	int (*crt_tolower)(int c);
	int (*crt_toupper)(int c);
	int (*crt_isascii)(int c);
	int (*crt_isalnum)(int c);
	int (*crt_isalpha)(int c);
	int (*crt_isblank)(int c);
	int (*crt_iscntrl)(int c);
	int (*crt_isdigit)(int c);
	int (*crt_isgraph)(int c);
	int (*crt_islower)(int c);
	int (*crt_isprint)(int c);
	int (*crt_ispunct)(int c);
	int (*crt_isspace)(int c);
	int (*crt_isupper)(int c);
	int (*crt_isxdigit)(int c);

	wint_t (*crt_towlower)(wint_t c);
	wint_t (*crt_towupper)(wint_t c);
	int (*crt_iswascii)(wint_t c);
	int (*crt_iswalnum)(wint_t c);
	int (*crt_iswalpha)(wint_t c);
	int (*crt_iswblank)(wint_t c);
	int (*crt_iswcntrl)(wint_t c);
	int (*crt_iswdigit)(wint_t c);
	int (*crt_iswgraph)(wint_t c);
	int (*crt_iswlower)(wint_t c);
	int (*crt_iswprint)(wint_t c);
	int (*crt_iswpunct)(wint_t c);
	int (*crt_iswspace)(wint_t c);
	int (*crt_iswupper)(wint_t c);
	int (*crt_iswxdigit)(wint_t c);

	wctype_t (*crt_wctype)(char const *name);
	int (*crt_iswctype)(wint_t c, wctype_t mask);

	/* Add more CRT replacements here.  */
} gawk_crt_api_t;
#endif /* GAWK_CRT_API */

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
 * Use these if you want to make the code a little easier to read.
 * See the sample boilerplate code, below.
 */
#define do_lint		(gawk_api()->do_flags[gawk_do_lint])
#define do_traditional	(gawk_api()->do_flags[gawk_do_traditional])
#define do_profile	(gawk_api()->do_flags[gawk_do_profile])
#define do_sandbox	(gawk_api()->do_flags[gawk_do_sandbox])
#define do_debug	(gawk_api()->do_flags[gawk_do_debug])
#define do_mpfr		(gawk_api()->do_flags[gawk_do_mpfr])

#define get_argument(count, wanted, result) \
	(gawk_api()->api_get_argument(count, wanted, result))
#define set_argument(count, new_array) \
	(gawk_api()->api_set_argument(count, new_array))

#if !defined(_MSC_VER) || !defined(_PREFAST_)
#define fatal		gawk_api()->api_fatal
#define nonfatal	gawk_api()->api_nonfatal
#define warning		gawk_api()->api_warning
#define lintwarn	gawk_api()->api_lintwarn
#define api_fatal_(api, format, message, param) \
	api->api_fatal(format, message, param)
#else
/* Annotate printf-like format string so compiler will check passed args.  */
__declspec(noreturn)
void fatal(_Printf_format_string_ const char *format, ...);
void nonfatal(_Printf_format_string_ const char *format, ...);
void warning(_Printf_format_string_ const char *format, ...);
void lintwarn(_Printf_format_string_ const char *format, ...);
__declspec(noreturn)
void api_fatal_(const gawk_api_t *api,
		_Printf_format_string_ const char *format, ...);
#endif

#ifndef NDEBUG
#define gawk_crt_assert(expr) ((void)((expr) ? 1 : \
	(gawk_crt()->crt_assert_failed(#expr, __FILE__, __LINE__), 0)))
#else
#define gawk_crt_assert(expr) ((void)0)
#endif

#define register_input_parser(parser) \
	(gawk_api()->api_register_input_parser(parser))
#define register_output_wrapper(wrapper) \
	(gawk_api()->api_register_output_wrapper(wrapper))
#define register_two_way_processor(processor) \
	(gawk_api()->api_register_two_way_processor(processor))

#define update_ERRNO_int(e) \
	(gawk_api()->api_update_ERRNO_int(e))
#define update_ERRNO_string(str) \
	(gawk_api()->api_update_ERRNO_string(str))
#define unset_ERRNO() \
	(gawk_api()->api_unset_ERRNO())

#define add_ext_func(ns, func) \
	(gawk_api()->api_add_ext_func(ns, func))
#define awk_atexit(funcp, arg0) \
	(gawk_api()->api_awk_atexit(funcp, arg0))

#define sym_lookup(name, wanted, result) \
	sym_lookup_ns("", name, wanted, result)
#define sym_update(name, value) \
	sym_update_ns("", name, value)

#define sym_lookup_ns(name_space, name, wanted, result) \
	(gawk_api()->api_sym_lookup(name_space, name, wanted, result))
#define sym_update_ns(name_space, name, value) \
	(gawk_api()->api_sym_update(name_space, name, value))

#define sym_lookup_scalar(scalar_cookie, wanted, result) \
	(gawk_api()->api_sym_lookup_scalar(scalar_cookie, wanted, result))
#define sym_update_scalar(scalar_cookie, value) \
	(gawk_api()->api_sym_update_scalar(scalar_cookie, value))

#define get_array_element(array, index, wanted, result) \
	(gawk_api()->api_get_array_element(array, index, wanted, result))

#define set_array_element(array, index, value) \
	(gawk_api()->api_set_array_element(array, index, value))

static inline awk_bool_t
r_set_array_element_by_elem(const gawk_api_t *api,
			    awk_array_t array,
			    const struct awk_element *elem)
{
	return api->api_set_array_element(array,
					  &elem->index,
					  &elem->value);
}

#define set_array_element_by_elem(array, elem) \
	r_set_array_element_by_elem(gawk_api(), array, elem)

#define del_array_element(array, index) \
	(gawk_api()->api_del_array_element(array, index))

#define get_element_count(array, count_p) \
	(gawk_api()->api_get_element_count(array, count_p))

#define create_array()		(gawk_api()->api_create_array())

#define clear_array(array)	(gawk_api()->api_clear_array(array))

#define flatten_array_typed(array, data, index_type, value_type) \
	(gawk_api()->api_flatten_array_typed(array, data, index_type, value_type))

#define flatten_array(array, data) \
	flatten_array_typed(array, data, AWK_STRING, AWK_UNDEFINED)

#define release_flattened_array(array, data) \
	(gawk_api()->api_release_flattened_array(array, data))

#define create_value(value, result) \
	(gawk_api()->api_create_value(value,result))

#define release_value(value) \
	(gawk_api()->api_release_value(value))

#define get_file(name, namelen, filetype, fd, ibuf, obuf) \
	(gawk_api()->api_get_file(name, namelen, filetype, fd, ibuf, obuf))

#define get_mpfr_ptr()	(gawk_api()->api_get_mpfr())
#define get_mpz_ptr()	(gawk_api()->api_get_mpz())

#define register_ext_version(version) \
	(gawk_api()->api_register_ext_version(version))

#ifdef GAWK_CRT_API
#define emalloc1(api, size)		api->crt_api->crt_malloc(size)
#define ezalloc1(api, n, size)		api->crt_api->crt_calloc(n, size)
#define erealloc1(api, ptr, size)	api->crt_api->crt_realloc(ptr, size)
#else
#define emalloc1(api, size)		malloc(size)
#define ezalloc1(api, n, size)		calloc(n, size)
#define erealloc1(api, ptr, size)	realloc(ptr, size)
#endif

#define emalloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) emalloc1(api, size)) == 0) \
			api_fatal_(api, "%s: malloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define ezalloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) ezalloc1(api, 1, size)) == 0) \
			api_fatal_(api, "%s: calloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define erealloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) erealloc1(api, pointer, size)) == 0) \
			api_fatal_(api, "%s: realloc of %llu bytes failed", message, 0ull + size); \
	} while(0)

#define emalloc(pointer, type, size, message) \
	emalloc_(gawk_api(), pointer, type, size, message)
#define ezalloc(pointer, type, size, message) \
	ezalloc_(gawk_api(), pointer, type, size, message)
#define erealloc(pointer, type, size, message) \
	erealloc_(gawk_api(), pointer, type, size, message)

#define outbuf_fflush(outbuf)			(gawk_api()->api_ob_fflush(outbuf))
#define outbuf_fwrite(ptr, size, nmemb, outbuf)	(gawk_api()->api_ob_fwrite(ptr, size, nmemb, outbuf))
#define outbuf_ferror(outbuf)			(gawk_api()->api_ob_ferror(outbuf))
#define outbuf_clearerr(outbuf)			(gawk_api()->api_ob_clearerr(outbuf))
#define outbuf_fputc(c, outbuf)			(gawk_api()->api_ob_fputc(c, outbuf))
#define outbuf_fputs(s, outbuf)			(gawk_api()->api_ob_fputs(s, outbuf))
#define outbuf_vfprintf(outbuf, format, ap)	(gawk_api()->api_ob_vfprintf(outbuf, format, ap))

#if !defined(_MSC_VER) || !defined(_PREFAST_)
#define outbuf_fprintf				gawk_api()->api_ob_fprintf
#else
/* Annotate printf-like format string so compiler will check passed args.  */
int outbuf_fprintf(awk_output_buf_t *outbuf, _Printf_format_string_ const char *format, ...);
#endif

#ifdef GAWK_CRT_API

#define gawk_crt_malloc(size)		(gawk_crt()->crt_malloc(size))
#define gawk_crt_calloc(nmemb, size)	(gawk_crt()->crt_calloc(nmemb, size))
#define gawk_crt_realloc(ptr, size)	(gawk_crt()->crt_realloc(ptr, size))
#define gawk_crt_free(ptr)		(gawk_crt()->crt_free(ptr))

#if !defined(_MSC_VER) || !defined(_PREFAST_)
#define gawk_crt_printf			gawk_crt()->crt_printf
#define gawk_crt_fprintf		gawk_crt()->crt_fprintf
#define gawk_crt_sprintf		gawk_crt()->crt_sprintf
#define gawk_crt_snprintf		gawk_crt()->crt_snprintf
#else
/* Annotate printf-like format string so compiler will check passed args.  */
int gawk_crt_printf(_Printf_format_string_ const char *format, ...);
int gawk_crt_fprintf(FILE *file, _Printf_format_string_ const char *format, ...);
int gawk_crt_sprintf(char *str, _Printf_format_string_ const char *format, ...);
int gawk_crt_snprintf(char *str, size_t size, _Printf_format_string_ const char *format, ...);
#endif

#define gawk_crt_stdin()		(gawk_crt()->crt_stdin)
#define gawk_crt_stdout()		(gawk_crt()->crt_stdout)
#define gawk_crt_stderr()		(gawk_crt()->crt_stderr)

static inline int
r_gawk_close(const gawk_crt_api_t *crt,
	     fd_t fd)
{
	return crt->crt_close(fd);
}

static inline fd_t
r_gawk_dup(const gawk_crt_api_t *crt,
	   fd_t oldfd)
{
	return crt->crt_dup(oldfd);
}

static inline fd_t
r_gawk_dup2(const gawk_crt_api_t *crt,
	    fd_t oldfd, fd_t newfd)
{
	return crt->crt_dup2(oldfd, newfd);
}

static inline ssize_t
r_gawk_read(const gawk_crt_api_t *crt,
	    fd_t fd, void *buf, size_t count)
{
	return crt->crt_read(fd, buf, count);
}

static inline ssize_t
r_gawk_write(const gawk_crt_api_t *crt,
	     fd_t fd, const void *buf, size_t count)
{
	return crt->crt_write(fd, buf, count);
}

static inline long long
r_gawk_lseek(const gawk_crt_api_t *crt,
	      fd_t fd, long long offset, int whence)
{
	return crt->crt_lseek(fd, offset, whence);
}

static inline long long
r_gawk_tell(const gawk_crt_api_t *crt,
	    fd_t fd)
{
	return crt->crt_tell(fd);
}

static inline int
r_gawk_fsync(const gawk_crt_api_t *crt,
	     fd_t fd)
{
	return crt->crt_fsync(fd);
}

static inline int
r_gawk_fstat(const gawk_crt_api_t *crt,
	     fd_t fd, gawk_stat_t *buf)
{
	return crt->crt_fstat(fd, buf);
}

#define gawk_crt_open				(fd_t) gawk_crt()->crt_open
#define gawk_crt_close(fd)			r_gawk_close(gawk_crt(), fd)
#define gawk_crt_dup(oldfd)			r_gawk_dup(gawk_crt(), oldfd)
#define gawk_crt_dup2(oldfd, newfd)		r_gawk_dup2(gawk_crt(), oldfd, newfd)

#define gawk_crt_read(fd, buf, count)		r_gawk_read(gawk_crt(), fd, buf, count)
#define gawk_crt_write(fd, buf, count)		r_gawk_write(gawk_crt(), fd, buf, count)
#define gawk_crt_lseek(fd, offset, whence)	r_gawk_lseek(gawk_crt(), fd, offset, whence)
#define gawk_crt_tell(fd)			r_gawk_tell(gawk_crt(), fd)
#define gawk_crt_fsync(fd)			r_gawk_fsync(gawk_crt(), fd)

#define gawk_crt_fflush(stream)			(gawk_crt()->crt_fflush(stream))
#define gawk_crt_fgetpos(stream, pos)		(gawk_crt()->crt_fgetpos(stream, pos))
#define gawk_crt_fsetpos(stream, pos)		(gawk_crt()->crt_fsetpos(stream, pos))
#define gawk_crt_rewind(stream)			(gawk_crt()->crt_rewind(stream))
#define gawk_crt_fseek(stream, offset, whence)	(gawk_crt()->crt_fseek(stream, offset, whence))
#define gawk_crt_ftell(stream)			(gawk_crt()->crt_ftell(stream))

#define gawk_crt_fopen(filename, mode)		(gawk_crt()->crt_fopen(filename, mode))
#define gawk_crt_fclose(stream)			(gawk_crt()->crt_fclose(stream))

#define gawk_crt_fread(ptr, size, nmemb, stream) \
	(gawk_crt()->crt_fread(ptr, size, nmemb, stream))
#define gawk_crt_fwrite(ptr, size, nmemb, stream) \
	(gawk_crt()->crt_fwrite(ptr, size, nmemb, stream))

#define gawk_crt_clearerr(stream)		(gawk_crt()->crt_clearerr(stream))
#define gawk_crt_feof(stream)			(gawk_crt()->crt_feof(stream))
#define gawk_crt_ferror(stream)			(gawk_crt()->crt_ferror(stream))
#define gawk_crt_fileno(stream)			(gawk_crt()->crt_fileno(stream))

#define gawk_crt_putchar(c)			(gawk_crt()->crt_putchar(c))
#define gawk_crt_fputc(c, stream)		(gawk_crt()->crt_fputc(c, stream))
/* same as fputc */
#define gawk_crt_putc(c, stream)		(gawk_crt()->crt_fputc(c, stream))
#define gawk_crt_getchar()			(gawk_crt()->crt_getchar())
#define gawk_crt_fgetc(stream)			(gawk_crt()->crt_fgetc(stream))
/* same as fgetc */
#define gawk_crt_getc(stream)			(gawk_crt()->crt_fgetc(stream))
#define gawk_crt_puts(s)			(gawk_crt()->crt_puts(s))
#define gawk_crt_fputs(s, stream)		(gawk_crt()->crt_fputs(s, stream))

#define gawk_crt_fgets(s, size, stream)		(gawk_crt()->crt_fgets(s, size, stream))
#define gawk_crt_ungetc(c, stream)		(gawk_crt()->crt_ungetc(s, stream))

#define gawk_crt_mkdir(dirname)			(gawk_crt()->crt_mkdir(dirname))
#define gawk_crt_rmdir(dirname)			(gawk_crt()->crt_rmdir(dirname))
#define gawk_crt_remove(pathname)		(gawk_crt()->crt_remove(pathname))
#define gawk_crt_unlink(pathname)		(gawk_crt()->crt_unlink(pathname))
#define gawk_crt_rename(old_name, new_name)	(gawk_crt()->crt_rename(old_name, new_name))
#define gawk_crt_chdir(path)			(gawk_crt()->crt_chdir(path))

#ifdef _MSC_VER
#define gawk_crt_xpathwc(path, buf, buf_size)	(gawk_crt()->crt_xpathwc(path, buf, buf_size))
#define gawk_crt_xfstat(h, path, buf)		(gawk_crt()->crt_xfstat(h, path, buf))
#define gawk_crt_xwstat(path, buf, dont_follow)	(gawk_crt()->crt_xwstat(path, buf, dont_follow))
#define gawk_crt_xstat(path, buf)		(gawk_crt()->crt_xstat(path, buf))
#define gawk_crt_xlstat(path, buf)		(gawk_crt()->crt_xlstat(path, buf))
#define gawk_crt_xstat_root(path, buf)		(gawk_crt()->crt_xstat_root(path, buf))
#else
#define gawk_crt_xstat(path, buf)		(gawk_crt()->crt_stat(path, buf))
#endif

#define gawk_crt_stat(path, buf)		(gawk_crt()->crt_stat(path, buf))
#define gawk_crt_fstat(fd, buf)			r_gawk_fstat(gawk_crt(), fd, buf)
#define gawk_crt_chmod(path, mode)		(gawk_crt()->crt_chmod(path, mode))

#define gawk_crt_setlocale(category, locale)	(gawk_crt()->crt_setlocale(category, locale))
#define gawk_crt_localeconv()			(gawk_crt()->crt_localeconv())

#define gawk_crt_strcoll(s1, s2)		(gawk_crt()->crt_strcoll(s1, s2))
#define gawk_crt_wcscoll(s1, s2)		(gawk_crt()->crt_wcscoll(s1, s2))

#define gawk_crt_mb_cur_max()			(gawk_crt()->crt_mb_cur_max())

#define gawk_crt_btowc(c)			(gawk_crt()->crt_btowc(c))

#define gawk_crt_mblen(mbchar, count)		(gawk_crt()->crt_mblen(mbchar, count))
#define gawk_crt_mbtowc(wchar, mbchar, count)	(gawk_crt()->crt_mbtowc(wchar, mbchar, count))
#define gawk_crt_wctomb(mbchar, wchar)		(gawk_crt()->crt_wctomb(mbchar, wchar))

#define gawk_crt_mbstowcs(wcstr, mbstr, count)	(gawk_crt()->crt_mbstowcs(wcstr, mbstr, count))
#define gawk_crt_wcstombs(mbstr, wcstr, count)	(gawk_crt()->crt_wcstombs(mbstr, wcstr, count))

#define gawk_crt_vprintf(format, ap)		(gawk_crt()->crt_vprintf(format, ap))
#define gawk_crt_vfprintf(stream, format, ap)	(gawk_crt()->crt_vfprintf(stream, format, ap))
#define gawk_crt_vsprintf(str, format, ap)	(gawk_crt()->crt_vsprintf(str, format, ap))

#define gawk_crt_vsnprintf(str, size, format, ap) \
	(gawk_crt()->crt_vsnprintf(str, size, format, ap))

#define gawk_crt_errno_p()			(gawk_crt()->crt_errno_p())
#define gawk_crt_strerror(error_number)		(gawk_crt()->crt_strerror(error_number))

static inline fd_t
r_gawk_mkstemp(const gawk_crt_api_t *crt,
	       char *templ)
{
	return crt->crt_mkstemp(templ);
}

#define gawk_crt_mkstemp(templ)			r_gawk_mkstemp(gawk_crt(), templ)

#define gawk_crt_getenv(name)        		(gawk_crt()->crt_getenv(name))

#define gawk_crt_tolower(c)			(gawk_crt()->crt_tolower(c))
#define gawk_crt_toupper(c)			(gawk_crt()->crt_toupper(c))
#define gawk_crt_isascii(c)			(gawk_crt()->crt_isascii(c))
#define gawk_crt_isalnum(c)			(gawk_crt()->crt_isalnum(c))
#define gawk_crt_isalpha(c)			(gawk_crt()->crt_isalpha(c))
#define gawk_crt_isblank(c)			(gawk_crt()->crt_isblank(c))
#define gawk_crt_iscntrl(c)			(gawk_crt()->crt_iscntrl(c))
#define gawk_crt_isdigit(c)			(gawk_crt()->crt_isdigit(c))
#define gawk_crt_isgraph(c)			(gawk_crt()->crt_isgraph(c))
#define gawk_crt_islower(c)			(gawk_crt()->crt_islower(c))
#define gawk_crt_isprint(c)			(gawk_crt()->crt_isprint(c))
#define gawk_crt_ispunct(c)			(gawk_crt()->crt_ispunct(c))
#define gawk_crt_isspace(c)			(gawk_crt()->crt_isspace(c))
#define gawk_crt_isupper(c)			(gawk_crt()->crt_isupper(c))
#define gawk_crt_isxdigit(c)			(gawk_crt()->crt_isxdigit(c))

#define gawk_crt_towlower(c)			(gawk_crt()->crt_towlower(c))
#define gawk_crt_towupper(c)			(gawk_crt()->crt_towupper(c))
#define gawk_crt_iswascii(c)			(gawk_crt()->crt_iswascii(c))
#define gawk_crt_iswalnum(c)			(gawk_crt()->crt_iswalnum(c))
#define gawk_crt_iswalpha(c)			(gawk_crt()->crt_iswalpha(c))
#define gawk_crt_iswblank(c)			(gawk_crt()->crt_iswblank(c))
#define gawk_crt_iswcntrl(c)			(gawk_crt()->crt_iswcntrl(c))
#define gawk_crt_iswdigit(c)			(gawk_crt()->crt_iswdigit(c))
#define gawk_crt_iswgraph(c)			(gawk_crt()->crt_iswgraph(c))
#define gawk_crt_iswlower(c)			(gawk_crt()->crt_iswlower(c))
#define gawk_crt_iswprint(c)			(gawk_crt()->crt_iswprint(c))
#define gawk_crt_iswpunct(c)			(gawk_crt()->crt_iswpunct(c))
#define gawk_crt_iswspace(c)			(gawk_crt()->crt_iswspace(c))
#define gawk_crt_iswupper(c)			(gawk_crt()->crt_iswupper(c))
#define gawk_crt_iswxdigit(c)			(gawk_crt()->crt_iswxdigit(c))

#define gawk_crt_wctype(name)			(gawk_crt()->crt_wctype(name))
#define gawk_crt_iswctype(c, mask)		(gawk_crt()->crt_iswctype(c, mask))

/* Add more CRT replacements here.  */

#endif /* GAWK_CRT_API */

/* Constructor functions */

/* r_make_string_type --- make a string or strnum or regexp value in result from the passed-in string */

static inline awk_value_t *
r_make_string_type(const gawk_api_t *api,	/* needed for emalloc */
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
	      const char *string,
	      size_t length,
	      awk_bool_t duplicate,
	      awk_value_t *result)
{
	return r_make_string_type(api, string, length, duplicate, result, AWK_STRING);
}

#define make_const_string(str, len, result) \
	r_make_string(gawk_api(), str, len, awk_true, result)
#define make_malloced_string(str, len, result) \
	r_make_string(gawk_api(), str, len, awk_false, result)

#define make_const_regex(str, len, result) \
	r_make_string_type(gawk_api(), str, len, awk_true, result, AWK_REGEX)
#define make_malloced_regex(str, len, result) \
	r_make_string_type(gawk_api(), str, len, awk_false, result, AWK_REGEX)

/*
 * Note: The caller may not create a STRNUM, but it can create a string that is
 * flagged as user input that MAY be a STRNUM. Gawk will decide whether it's a
 * STRNUM or a string by checking whether the string is numeric.
 */
#define make_const_user_input(str, len, result) \
	r_make_string_type(gawk_api(), str, len, 1, result, AWK_STRNUM)
#define make_malloced_user_input(str, len, result) \
	r_make_string_type(gawk_api(), str, len, 0, result, AWK_STRNUM)

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

#if defined(_MSC_VER) && defined(GAWK_STATIC_CRT)
/*
 * Do not initialize static version of CRT library compiled in extension
 * DLLs - they should use gawk API for all except stateless CRT functions,
 * like memcpy, strlen, fileno, etc.
 */
/* Tip: link extension DLLs with "/Entry:ExtDllMain" option.  */
#define EXTENSION_DLL_MAIN \
	int __stdcall ExtDllMain( \
		void * hinstDLL, \
		unsigned long fdwReason, \
		void *lpvReserved) \
	{ \
		(void) hinstDLL, (void) fdwReason, (void) lpvReserved; \
		return 1; \
	}
#endif /* _MSC_VER && GAWK_STATIC_CRT */

#ifndef EXTENSION_DLL_MAIN
#define EXTENSION_DLL_MAIN
#endif

#define GAWK_PLUGIN_GPL_COMPATIBLE \
  GAWK_PLUGIN_EXTERN_C_BEGIN \
  GAWK_PLUGIN_EXPORT \
  int plugin_is_GPL_compatible; \
  GAWK_PLUGIN_EXTERN_C_END

typedef struct gawk_plugin_info {
	const char *ext_version;
	const gawk_api_t *api;
#ifdef GAWK_CRT_API
	const gawk_crt_api_t *crt;
#endif
} gawk_plugin_info_t;

#define GAWK_PLUGIN(ver) \
	static gawk_plugin_info_t gawk_plugin_info = {ver}

/*
 * Each extension must define a function with this prototype.
 * The return value should be zero on failure and non-zero on success.
 *
 * For the default implementation, use dl_load_func() helper macro.
 * First argument of dl_load_func() is an optional extension initialization
 * routine, which should return non-zero on success and zero upon failure.
 */
GAWK_PLUGIN_EXTERN_C_BEGIN
GAWK_PLUGIN_EXPORT
extern int dl_load(const gawk_api_t *const api_p, gawk_extension_api_ver_t *ver);
GAWK_PLUGIN_EXTERN_C_END

#if 0
/* Boilerplate code: */
#include "gawkapi.h"

GAWK_PLUGIN_GPL_COMPATIBLE
GAWK_PLUGIN("plugin version string");

static awk_ext_func_t func_table[] = {
	{ "name", do_name, 1 },
	/* ... */
};

/* EITHER: */

dl_load_func(NULL, func_table, extension_name, "name_space_in_quotes")

/* OR: */

static awk_bool_t
init_my_extension(void)
{
	...
}

dl_load_func(init_my_extension, func_table, extension_name, "name_space_in_quotes")
#endif

#ifndef GAWK_CRT_API
#define dl_load_init_plugin_info(api_p) do { \
	gawk_plugin_info.api = api_p; \
} while ((void)0,0)
#else
#define dl_load_init_plugin_info(api_p) do { \
	gawk_plugin_info.api = api_p; \
	gawk_plugin_info.crt = api_p->crt_api; \
} while ((void)0,0)
#endif

#ifndef GAWK_CRT_API
#define dl_load_define_support_funcs \
const gawk_api_t *gawk_api(void)	{return gawk_plugin_info.api;}
#else
#define dl_load_define_support_funcs \
const gawk_api_t *gawk_api(void)	{return gawk_plugin_info.api;} \
const gawk_crt_api_t *gawk_crt(void)	{return gawk_plugin_info.crt;}
#endif

#define gawk_check_api_version(name_, MAJOR_, MINOR_, major_, minor_) \
	if ((major_) != MAJOR_ || (minor_) < MINOR_) { \
		ver->api_name = name_; \
		ver->need.major_version = MAJOR_; \
		ver->need.minor_version = MINOR_; \
		ver->have.major_version = major_; \
		ver->have.minor_version = minor_; \
		return -1; \
	} \

static inline awk_bool_t
dl_load_call_init_func(awk_bool_t (*func)(void))
{
	if (func == NULL)
		return awk_true;
	return (*func)();
}

#define dl_load_func(init_func, func_table, extension, name_space) \
dl_load_define_support_funcs \
GAWK_PLUGIN_EXTERN_C_BEGIN \
EXTENSION_DLL_MAIN \
GAWK_PLUGIN_EXPORT \
int dl_load(const gawk_api_t *const api_p, gawk_extension_api_ver_t *ver)  \
{ \
	size_t i, j; \
	int errors = 0; \
\
	gawk_check_api_version(#extension, \
		GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION, \
		api_p->major_version, api_p->minor_version); \
\
	check_mpfr_version(api_p, extension); \
\
	check_crt_version(api_p, extension); \
\
	dl_load_init_plugin_info(api_p); \
\
	/* load functions */ \
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) { \
		if (func_table[i].name == NULL) \
			break; \
		if (! add_ext_func(name_space, & func_table[i])) { \
			warning(#extension ": could not add %s", \
					func_table[i].name); \
			errors++; \
		} \
	} \
\
	if (!dl_load_call_init_func(init_func)) { \
		warning(#extension ": initialization function failed"); \
		errors++; \
	} \
\
	if (gawk_plugin_info.ext_version != NULL) \
		register_ext_version(gawk_plugin_info.ext_version); \
\
	return (errors == 0); \
} \
GAWK_PLUGIN_EXTERN_C_END

#if defined __GNU_MP_VERSION && defined MPFR_VERSION_MAJOR
#define check_mpfr_version(api_p, extension) do { \
	gawk_check_api_version(#extension ": GMP", \
		__GNU_MP_VERSION, __GNU_MP_VERSION_MINOR, \
		api_p->gmp_major_version, api_p->gmp_minor_version); \
	gawk_check_api_version(#extension ": MPFR", \
		MPFR_VERSION_MAJOR, MPFR_VERSION_MINOR, \
		api_p->mpfr_major_version, api_p->mpfr_minor_version); \
} while (0)
#else
#define check_mpfr_version(api_p, extension) /* nothing */
#endif

#ifdef GAWK_CRT_API
#define check_crt_version(api_p, extension) do { \
	gawk_check_api_version(#extension ": CRT", \
		GAWK_CRT_MAJOR_VERSION, GAWK_CRT_MINOR_VERSION, \
		api_p->crt_api ? api_p->crt_api->crt_major_version : -1, \
		api_p->crt_api ? api_p->crt_api->crt_minor_version : -1); \
} while (0)
#else
#define check_crt_version(api_p, extension) /* nothing */
#endif

/* Forward declaration - to be able to use gawk API in inline functions in
  headers included between "gawkapi.h" and the definition of
  "gawk_plugin_info".  */
extern const gawk_api_t *gawk_api(void);
#ifdef GAWK_CRT_API
extern const gawk_crt_api_t *gawk_crt(void);
#endif

#if defined(GAWK_CRT_API) && !defined(GAWK_DONT_REDEFINE_CRT)

#ifdef malloc
#undef malloc
#endif
#define malloc(size) gawk_crt_malloc(size)

#ifdef calloc
#undef calloc
#endif
#define calloc(nmemb, size) gawk_crt_calloc(nmemb, size)

#ifdef realloc
#undef realloc
#endif
#define realloc(ptr, size) gawk_crt_realloc(ptr, size)

#ifdef free
#undef free
#endif
#define free(ptr) gawk_crt_free(ptr)

#ifdef printf
#undef printf
#endif
#define printf gawk_crt_printf

#ifdef assert
#undef assert
#endif
#define assert(expr) gawk_crt_assert(expr)

#ifdef stdin
#undef stdin
#endif
#define stdin gawk_crt_stdin()

#ifdef stdout
#undef stdout
#endif
#define stdout gawk_crt_stdout()

#ifdef stderr
#undef stderr
#endif
#define stderr gawk_crt_stderr()

#ifdef open
#undef open
#endif
#define open gawk_crt_open

#ifdef close
#undef close
#endif
#define close(fd) gawk_crt_close(fd)

#ifdef dup
#undef dup
#endif
#define dup(oldfd) gawk_crt_dup(oldfd)

#ifdef dup2
#undef dup2
#endif
#define dup2(oldfd, newfd) gawk_crt_dup2(oldfd, newfd)

#ifdef read
#undef read
#endif
#define read(fd, buf, count) gawk_crt_read(fd, buf, count)

#ifdef write
#undef write
#endif
#define write(fd, buf, count) gawk_crt_write(fd, buf, count)

#ifdef lseek
#undef lseek
#endif
#define lseek(fd, offset, whence) gawk_crt_lseek(fd, offset, whence)

#ifdef tell
#undef tell
#endif
#define tell(fd) gawk_crt_tell(fd)

#ifdef fsync
#undef fsync
#endif
#define fsync(fd) gawk_crt_fsync(fd)

#ifdef fflush
#undef fflush
#endif
#define fflush(stream) gawk_crt_fflush(stream)

#ifdef fgetpos
#undef fgetpos
#endif
#define fgetpos(stream, pos) gawk_crt_fgetpos(stream, pos)

#ifdef fsetpos
#undef fsetpos
#endif
#define fsetpos(stream, pos) gawk_crt_fsetpos(stream, pos)

#ifdef rewind
#undef rewind
#endif
#define rewind(stream) gawk_crt_rewind(stream)

#ifdef fseek
#undef fseek
#endif
#define fseek(stream, offset, whence) gawk_crt_fseek(stream, offset, whence)

#ifdef ftell
#undef ftell
#endif
#define ftell(stream) gawk_crt_ftell(stream)

#ifdef fopen
#undef fopen
#endif
#define fopen(filename, mode) gawk_crt_fopen(filename, mode)

#ifdef fclose
#undef fclose
#endif
#define fclose(stream) gawk_crt_fclose(stream)

#ifdef fread
#undef fread
#endif
#define fread(ptr, size, nmemb, stream) \
	gawk_crt_fread(ptr, size, nmemb, stream)

#ifdef fwrite
#undef fwrite
#endif
#define fwrite(ptr, size, nmemb, stream) \
	gawk_crt_fwrite(ptr, size, nmemb, stream)

#ifdef clearerr
#undef clearerr
#endif
#define clearerr(stream) gawk_crt_clearerr(stream)

#ifdef feof
#undef feof
#endif
#define feof(stream) gawk_crt_feof(stream)

#ifdef ferror
#undef ferror
#endif
#define ferror(stream) gawk_crt_ferror(stream)

#ifdef fileno
#undef fileno
#endif
#define fileno(stream) gawk_crt_fileno(stream)

#ifdef putchar
#undef putchar
#endif
#define putchar(c) gawk_crt_putchar(c)

#ifdef fputc
#undef fputc
#endif
#define fputc(c, stream) gawk_crt_fputc(c, stream)

#ifdef putc
#undef putc
#endif
#define putc(c, stream) gawk_crt_putc(c, stream)

#ifdef getchar
#undef getchar
#endif
#define getchar() gawk_crt_getchar()

#ifdef fgetc
#undef fgetc
#endif
#define fgetc(stream) gawk_crt_fgetc(stream)

#ifdef getc
#undef getc
#endif
#define getc(stream) gawk_crt_getc(stream)

#ifdef puts
#undef puts
#endif
#define puts(s) gawk_crt_puts(s)

#ifdef fputs
#undef fputs
#endif
#define fputs(s, stream) gawk_crt_fputs(s, stream)

#ifdef fgets
#undef fgets
#endif
#define fgets(s, size, stream) gawk_crt_fgets(s, size, stream)

#ifdef ungetc
#undef ungetc
#endif
#define ungetc(c, stream) gawk_crt_ungetc(c, stream)

#ifdef mkdir
#undef mkdir
#endif
#define mkdir(dirname) gawk_crt_mkdir(dirname)

#ifdef rmdir
#undef rmdir
#endif
#define rmdir(dirname) gawk_crt_rmdir(dirname)

#ifdef remove
#undef remove
#endif
#define remove(pathname) gawk_crt_remove(pathname)

#ifdef unlink
#undef unlink
#endif
#define unlink(pathname) gawk_crt_unlink(pathname)

#ifdef rename
#undef rename
#endif
#define rename(old_name, new_name) gawk_crt_rename(old_name, new_name)

#ifdef chdir
#undef chdir
#endif
#define chdir(path) gawk_crt_chdir(path)

#ifdef _MSC_VER

#ifdef xpathwc
#undef xpathwc
#endif
#define xpathwc(path, buf, buf_size) gawk_crt_xpathwc(path, buf, buf_size)

#ifdef xfstat
#undef xfstat
#endif
#define xfstat(h, path, buf) gawk_crt_xfstat(h, path, buf)

#ifdef xwstat
#undef xwstat
#endif
#define xwstat(path, buf, dont_follow) gawk_crt_xwstat(path, buf, dont_follow)

#ifdef xlstat
#undef xlstat
#endif
#define xlstat(path, buf) gawk_crt_xlstat(path, buf)

#ifdef xstat_root
#undef xstat_root
#endif
#define xstat_root(path, buf) gawk_crt_xstat_root(path, buf)

#endif /* _MSC_VER */

#ifdef xstat
#undef xstat
#endif
#define xstat(path, buf) gawk_crt_xstat(path, buf)

#ifdef stat
#undef stat
#endif
#define stat(path, buf) gawk_crt_stat(path, buf)

#ifdef fstat
#undef fstat
#endif
#define fstat(fd, buf) gawk_crt_fstat(fd, buf)

#ifdef chmod
#undef chmod
#endif
#define chmod(path, mode) gawk_crt_chmod(path, mode)

#ifdef setlocale
#undef setlocale
#endif
#define setlocale(category, locale) gawk_crt_setlocale(category, locale)

#ifdef localeconv
#undef localeconv
#endif
#define localeconv() gawk_crt_localeconv()

#ifdef strcoll
#undef strcoll
#endif
#define strcoll(s1, s2) gawk_crt_strcoll(s1, s2)

#ifdef wcscoll
#undef wcscoll
#endif
#define wcscoll(s1, s2) gawk_crt_wcscoll(s1, s2)

#ifdef MB_CUR_MAX
#undef MB_CUR_MAX
#endif
#define MB_CUR_MAX gawk_crt_mb_cur_max()

#ifdef btowc
#undef btowc
#endif
#define btowc(c) gawk_crt_btowc(c)

#ifdef mblen
#undef mblen
#endif
#define mblen(mbchar, count) gawk_crt_mblen(mbchar, count)

#ifdef mbtowc
#undef mbtowc
#endif
#define mbtowc(wchar, mbchar, count) gawk_crt_mbtowc(wchar, mbchar, count)

#ifdef wctomb
#undef wctomb
#endif
#define wctomb(mbchar, wchar) gawk_crt_wctomb(mbchar, wchar)

#ifdef mbstowcs
#undef mbstowcs
#endif
#define mbstowcs(wcstr, mbstr, count) gawk_crt_mbstowcs(wcstr, mbstr, count)

#ifdef wcstombs
#undef wcstombs
#endif
#define wcstombs(mbstr, wcstr, count) gawk_crt_wcstombs(mbstr, wcstr, count)

#ifdef fprintf
#undef fprintf
#endif
#define fprintf gawk_crt_fprintf

#ifdef sprintf
#undef sprintf
#endif
#define sprintf gawk_crt_sprintf

#ifdef snprintf
#undef snprintf
#endif
#define snprintf gawk_crt_snprintf

#ifdef vprintf
#undef vprintf
#endif
#define vprintf(format, ap) gawk_crt_vprintf(format, ap)

#ifdef vfprintf
#undef vfprintf
#endif
#define vfprintf(stream, format, ap) gawk_crt_vfprintf(stream, format, ap)

#ifdef vsprintf
#undef vsprintf
#endif
#define vsprintf(str, format, ap) gawk_crt_vsprintf(str, format, ap)

#ifdef vsnprintf
#undef vsnprintf
#endif
#define vsnprintf(str, size, format, ap) gawk_crt_vsnprintf(str, size, format, ap)

#ifdef errno
#undef errno
#endif
#define errno (*gawk_crt_errno_p())

#ifdef strerror
#undef strerror
#endif
#define strerror(error_number) gawk_crt_strerror(error_number)

#ifdef mkstemp
#undef mkstemp
#endif
#define mkstemp(templ) gawk_crt_mkstemp(templ)

#ifdef getenv
#undef getenv
#endif
#define getenv(name) gawk_crt_getenv(name)

#ifdef tolower
#undef tolower
#endif
#define tolower(c) gawk_crt_tolower(c)

#ifdef toupper
#undef toupper
#endif
#define toupper(c) gawk_crt_toupper(c)

#ifdef isascii
#undef isascii
#endif
#define isascii(c) gawk_crt_isascii(c)

#ifdef isalnum
#undef isalnum
#endif
#define isalnum(c) gawk_crt_isalnum(c)

#ifdef isalpha
#undef isalpha
#endif
#define isalpha(c) gawk_crt_isalpha(c)

#ifdef isblank
#undef isblank
#endif
#define isblank(c) gawk_crt_isblank(c)

#ifdef iscntrl
#undef iscntrl
#endif
#define iscntrl(c) gawk_crt_iscntrl(c)

#ifdef isdigit
#undef isdigit
#endif
#define isdigit(c) gawk_crt_isdigit(c)

#ifdef isgraph
#undef isgraph
#endif
#define isgraph(c) gawk_crt_isgraph(c)

#ifdef islower
#undef islower
#endif
#define islower(c) gawk_crt_islower(c)

#ifdef isprint
#undef isprint
#endif
#define isprint(c) gawk_crt_isprint(c)

#ifdef ispunct
#undef ispunct
#endif
#define ispunct(c) gawk_crt_ispunct(c)

#ifdef isspace
#undef isspace
#endif
#define isspace(c) gawk_crt_isspace(c)

#ifdef isupper
#undef isupper
#endif
#define isupper(c) gawk_crt_isupper(c)

#ifdef isxdigit
#undef isxdigit
#endif
#define isxdigit(c) gawk_crt_isxdigit(c)

#ifdef towlower
#undef towlower
#endif
#define towlower(c) gawk_crt_towlower(c)

#ifdef towupper
#undef towupper
#endif
#define towupper(c) gawk_crt_towupper(c)

#ifdef iswascii
#undef iswascii
#endif
#define iswascii(c) gawk_crt_iswascii(c)

#ifdef iswalnum
#undef iswalnum
#endif
#define iswalnum(c) gawk_crt_iswalnum(c)

#ifdef iswalpha
#undef iswalpha
#endif
#define iswalpha(c) gawk_crt_iswalpha(c)

#ifdef iswblank
#undef iswblank
#endif
#define iswblank(c) gawk_crt_iswblank(c)

#ifdef iswcntrl
#undef iswcntrl
#endif
#define iswcntrl(c) gawk_crt_iswcntrl(c)

#ifdef iswdigit
#undef iswdigit
#endif
#define iswdigit(c) gawk_crt_iswdigit(c)

#ifdef iswgraph
#undef iswgraph
#endif
#define iswgraph(c) gawk_crt_iswgraph(c)

#ifdef iswlower
#undef iswlower
#endif
#define iswlower(c) gawk_crt_iswlower(c)

#ifdef iswprint
#undef iswprint
#endif
#define iswprint(c) gawk_crt_iswprint(c)

#ifdef iswpunct
#undef iswpunct
#endif
#define iswpunct(c) gawk_crt_iswpunct(c)

#ifdef iswspace
#undef iswspace
#endif
#define iswspace(c) gawk_crt_iswspace(c)

#ifdef iswupper
#undef iswupper
#endif
#define iswupper(c) gawk_crt_iswupper(c)

#ifdef iswxdigit
#undef iswxdigit
#endif
#define iswxdigit(c) gawk_crt_iswxdigit(c)

#ifdef wctype
#undef wctype
#endif
#define wctype(name) gawk_crt_wctype(name)

#ifdef iswctype
#undef iswctype
#endif
#define iswctype(c, mask) gawk_crt_iswctype(c, mask)

/* Add more CRT replacements here.  */

#endif /* GAWK_CRT_API && !GAWK_DONT_REDEFINE_CRT */

#endif /* !GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
