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

/* Function attributes.  */
#if !defined(__attribute__) && !( \
  (defined(__GNUC__) && __GNUC__ > 2 - (__GNUC_MINOR__ >= 7)) || \
  defined(__clang__))
# define __attribute__(arg)
#endif

/* Annotate function parameter that cannot be NULL.  */
#ifndef ATTRIBUTE_NONNULL
# define ATTRIBUTE_NONNULL(a) __attribute__ ((__nonnull__ (a)))
#endif

/* Annotate function that does not return (exits the program).  */
#ifndef ATTRIBUTE_NORETURN
# ifdef _MSC_VER
#  define ATTRIBUTE_NORETURN __declspec(noreturn)
# else
#  define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
# endif
#endif

/* Annotate parameters of printf-like function, e.g.:
  ATTRIBUTE_PRINTF(fmt, 2, 3) int my_printf(int param, const char *fmt, ...);
   or
  ATTRIBUTE_PRINTF(fmt, 2, 0) int my_vprintf(int param, const char *fmt, va_list ap); */
#ifndef ATTRIBUTE_PRINTF
# if defined(_PREFAST_)
/* MSVC */
#  define ATTRIBUTE_PRINTF(fmt, m, n) _At_(fmt, _Printf_format_string_ _Notnull_)
# elif defined(__Wformat_mingw_printf)
/* MinGW.org */
#  define ATTRIBUTE_PRINTF(fmt, m, n) __Wformat_mingw_printf(m, n) ATTRIBUTE_NONNULL(m)
# elif defined(__MINGW_PRINTF_FORMAT)
/* Mingw64 */
#  define ATTRIBUTE_PRINTF(fmt, m, n) \
	__attribute__ ((__format__ (__MINGW_PRINTF_FORMAT, m, n))) ATTRIBUTE_NONNULL(m)
# else
#  define ATTRIBUTE_PRINTF(fmt, m, n) \
/* generic gcc-compatible comiler (e.g., clang) */
	__attribute__ ((__format__ (__printf__, m, n))) ATTRIBUTE_NONNULL(m)
# endif
#endif

/* Annotate parameters of printf-like function pointer, e.g.:
  ATTRIBUTE_PRINTF_PTR(fmt, 2, 3) int (*my_printf)(int param, const char *fmt, ...);
   or
  ATTRIBUTE_PRINTF_PTR(fmt, 2, 0) int (*my_vprintf)(int param, const char *fmt, va_list ap); */
#ifndef ATTRIBUTE_PRINTF_PTR
# if defined(__GNUC__) && __GNUC__ > 3 - (__GNUC_MINOR__ >= 2)
#  define ATTRIBUTE_PRINTF_PTR(fmt, m, n) ATTRIBUTE_PRINTF(fmt, m, n)
# else
#  define ATTRIBUTE_PRINTF_PTR(fmt, m, n)
# endif
#endif

/* Annotate parameters of printf-like function pointer typedef, e.g.:
  ATTRIBUTE_PRINTF_TYPEDEF(fmt, 2, 3) typedef int (*my_printf)(int param, const char *fmt, ...);
   or
  ATTRIBUTE_PRINTF_TYPEDEF(fmt, 2, 0) typedef int (*my_vprintf)(int param, const char *fmt, va_list ap); */
#ifndef ATTRIBUTE_PRINTF_TYPEDEF
# if defined(__GNUC__) && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#  define ATTRIBUTE_PRINTF_TYPEDEF(fmt, m, n) ATTRIBUTE_PRINTF(fmt, m, n)
# else
#  define ATTRIBUTE_PRINTF_TYPEDEF(fmt, m, n)
# endif
#endif

/* Pragmas.  */
#if defined(__GNUC__) && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
# define PRAGMA_WARNING_PUSH_ _Pragma ("GCC diagnostic push")
# define PRAGMA_WARNING_POP_  _Pragma ("GCC diagnostic pop")
# define PRAGMA_WARNING_DISABLE_FORMAT_WARNING_ _Pragma ("GCC diagnostic ignored \"-Wformat-nonliteral\"")
# define PRAGMA_WARNING_DISABLE_COND_IS_CONST_  _Pragma ("GCC diagnostic ignored \"-Wtype-limits\"")
#elif defined(__clang__)
# define PRAGMA_WARNING_PUSH_ _Pragma ("clang diagnostic push")
# define PRAGMA_WARNING_POP_  _Pragma ("clang diagnostic pop")
# define PRAGMA_WARNING_DISABLE_FORMAT_WARNING_ _Pragma ("clang diagnostic ignored \"-Wformat-nonliteral\"")
# define PRAGMA_WARNING_DISABLE_COND_IS_CONST_  _Pragma ("clang diagnostic ignored \"-Wtype-limits\"")
#elif defined(_MSC_VER)
# define PRAGMA_WARNING_PUSH_ __pragma(warning(push))
# define PRAGMA_WARNING_POP_  __pragma(warning(pop))
# define PRAGMA_WARNING_DISABLE_FORMAT_WARNING_ __pragma(warning(disable:4774))
# define PRAGMA_WARNING_DISABLE_COND_IS_CONST_  __pragma(warning(disable:4127))
#else
# define PRAGMA_WARNING_PUSH_
# define PRAGMA_WARNING_POP_
# define PRAGMA_WARNING_DISABLE_FORMAT_WARNING_
# define PRAGMA_WARNING_DISABLE_COND_IS_CONST_
#endif

#ifndef PRAGMA_WARNING_PUSH
# define PRAGMA_WARNING_PUSH PRAGMA_WARNING_PUSH_
#endif

#ifndef PRAGMA_WARNING_POP
# define PRAGMA_WARNING_POP PRAGMA_WARNING_POP_
#endif

#ifndef PRAGMA_WARNING_DISABLE_FORMAT_WARNING
# define PRAGMA_WARNING_DISABLE_FORMAT_WARNING PRAGMA_WARNING_DISABLE_FORMAT_WARNING_
#endif

#ifndef PRAGMA_WARNING_DISABLE_COND_IS_CONST
# define PRAGMA_WARNING_DISABLE_COND_IS_CONST PRAGMA_WARNING_DISABLE_COND_IS_CONST_
#endif

/* Do compile unreachable code only if COMPILE_UNREACHABLE_CODE is defined.  */
#ifndef _MSC_VER
# define COMPILE_UNREACHABLE_CODE
#endif

/* Assume expression X is always TRUE.  */
#ifndef ASSUME
# ifdef __clang__
#  define ASSUME(x) __builtin_assume(x)
# elif defined _MSC_VER
#  define ASSUME(x) __assume(x)
# elif defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#  define ASSUME(x) ((void) ((x) ? 1 : (__builtin_unreachable(), 0)))
# else
#  define ASSUME(x) ((void) (x))
# endif
#endif

/* printf-format specifier for size_t.
   A c99-compatible standard runtime library must support "%zu".  */
#ifndef ZUFMT
# define ZUFMT "zu"
#endif

/* printf-format specifier for unsigned long long.
   A c99-compatible standard runtime library must support "%llu".  */
#ifndef LLUFMT
# define LLUFMT "llu"
#endif

/* Allow use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/* The name of dynamic extension initialization routine.
  It is the "dl_load" in the original GAWK.
  But to avoid conflicts with this modified version, rename it.  */
#define GAWK_EXT_INIT_FUNC_NAME	ext_onload

/* Current version of the API. */
#define gawk_api_major_version 4
#define gawk_api_minor_version 0
enum {
	GAWK_API_MAJOR_VERSION = gawk_api_major_version,
	GAWK_API_MINOR_VERSION = gawk_api_minor_version
};

/* File or socket descriptor type.  */
typedef int fd_t;
#define INVALID_HANDLE (-1)

/* File status information.  */
#if defined(_MSC_VER) || defined(__MINGW32__)
/* Fields st_size, st_atime, st_mtime, st_ctime are 64-bit values.   */
struct __stat64;
typedef struct __stat64 gawk_stat_t;
#else
struct stat;
typedef struct stat gawk_stat_t;
#endif

/*
 * If CRT (C runtime library) is statically linked to the main GAWK executable,
 * extension DLLs should use GAWK CRT API for the CRT calls and should not link
 * (statically or dynamically) other CRTs - to share single copy of internal
 * CRT data (file descriptor table, locale, heap) with the GAWK executable.
 */
#ifdef GAWK_STATIC_CRT
#define GAWK_USE_CRT_API
#endif

#ifdef GAWK_USE_CRT_API
#include "gawkcrtapi.h"
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

	/* Hidden fields (stat buf).  */
} awk_input_buf_t;

/* Extended file status information.  */
#if !defined(_MSC_VER) && !defined(__MINGW32__)
typedef struct stat gawk_xstat_t;
#elif defined(xstat_t) /* Include mscrtx/xstat.h for the definition.  */
/* Extended stat info, 64-bit st_ino is _valid_.  */
typedef xstat_t gawk_xstat_t;
#endif

#if (!defined(_MSC_VER) && !defined(__MINGW32__)) || defined xstat_t

struct awk_input_buf_container {
	awk_input_buf_t input_buf;

	/* put last, for alignment. bleah */
	gawk_xstat_t sbuf;       /* stat buf */
};

/* Note: include mscrtx/xstat.h before gawkapi.h for this function.  */
static inline const gawk_xstat_t *
awk_input_buf_get_stat(const awk_input_buf_t *iobuf)
{
	const void *container = iobuf;

	return &((const struct awk_input_buf_container*)container)->sbuf;
}

#endif /* (!_MSC_VER && !__MINGW32__) || xstat_t */

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
#if defined(_MSC_VER) || defined(__MINGW32__)
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
#if defined(_MSC_VER) || defined(__MINGW32__)
	if (outbuf->socket_fd != INVALID_HANDLE)
		return outbuf->socket_fd;
	return _fileno((FILE*) outbuf->file);
#else
	return fileno((FILE*) outbuf->file);
#endif
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

/* a combination of enum awk_element_flags */
typedef int awk_element_flags_t;

typedef struct awk_element {
	/* convenience linked list pointer, not used by gawk */
	struct awk_element *next;
	awk_element_flags_t flags;
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

/* Extension fills this info on error - if GAWK provides incompatible API.  */
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

/* Optional GAWK CRT API.  */
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
	ATTRIBUTE_PRINTF_PTR(format, 1, 2) void (*api_fatal)(const char *format, ...);
	ATTRIBUTE_PRINTF_PTR(format, 1, 2) void (*api_warning)(const char *format, ...);
	ATTRIBUTE_PRINTF_PTR(format, 1, 2) void (*api_lintwarn)(const char *format, ...);
	ATTRIBUTE_PRINTF_PTR(format, 1, 2) void (*api_nonfatal)(const char *format, ...);

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
	size_t (*api_ob_fwrite)(const void *buf, size_t size, size_t nmemb,
			awk_output_buf_t *outbuf);
	int (*api_ob_ferror)(awk_output_buf_t *outbuf);
	void (*api_ob_clearerr)(awk_output_buf_t *outbuf);
	int (*api_ob_fputc)(int c, awk_output_buf_t *outbuf);
	int (*api_ob_fputs)(const char *s, awk_output_buf_t *outbuf);

	ATTRIBUTE_PRINTF_PTR(format, 2, 3)
	int (*api_ob_fprintf)(awk_output_buf_t *outbuf, const char *format,
				...);

	ATTRIBUTE_PRINTF_PTR(format, 2, 0)
	int (*api_ob_vfprintf)(awk_output_buf_t *outbuf, const char *format,
				va_list ap);

} gawk_api_t;

#ifndef GAWK	/* these are not for the gawk code itself! */

/* gawk_api() hides struct gawk_api */
#if defined __cplusplus && defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

/* Forward declaration - to be able to use gawk API in inline functions in
  headers included between "gawkapi.h" and the definition of
  "gawk_plugin_info".  */
extern const gawk_api_t *gawk_api(void);

#if defined __cplusplus && defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

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

#ifdef GAWK_USE_CRT_API
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
			api_fatal_(api, "%s: malloc of %" ZUFMT " bytes failed", message, size); \
	} while(0)

#define ezalloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) ezalloc1(api, 1, size)) == 0) \
			api_fatal_(api, "%s: calloc of %" ZUFMT " bytes failed", message, size); \
	} while(0)

#define erealloc_(api, pointer, type, size, message) \
	do { \
		if ((pointer = (type) erealloc1(api, pointer, size)) == 0) \
			api_fatal_(api, "%s: realloc of %" ZUFMT " bytes failed", message, size); \
	} while(0)

#define emalloc(pointer, type, size, message) \
	emalloc_(gawk_api(), pointer, type, size, message)
#define ezalloc(pointer, type, size, message) \
	ezalloc_(gawk_api(), pointer, type, size, message)
#define erealloc(pointer, type, size, message) \
	erealloc_(gawk_api(), pointer, type, size, message)

#define outbuf_fflush(outbuf)			(gawk_api()->api_ob_fflush(outbuf))
#define outbuf_fwrite(buf, size, nmemb, outbuf)	(gawk_api()->api_ob_fwrite(buf, size, nmemb, outbuf))
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

static inline char *cast_conststr(const char *s)
{
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
		return (char *) s;
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

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
		result->str_value.str = cast_conststr(string);
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

#if defined(_MSC_VER) || defined(__MINGW32__)
#define GAWK_PLUGIN_EXPORT	__declspec(dllexport)
#else
#define GAWK_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
#define GAWK_PLUGIN_EXTERN_C_BEGIN	extern "C" {
#define GAWK_PLUGIN_EXTERN_C_END	}
#else
#define GAWK_PLUGIN_EXTERN_C_BEGIN
#define GAWK_PLUGIN_EXTERN_C_END
#endif

#if (defined(_MSC_VER) || defined(__MINGW32__)) && defined(GAWK_STATIC_CRT)
/*
 * Do not initialize static version of CRT library linked in extension
 * DLLs - they should use gawk CRT API for all except stateless CRT functions,
 * like memcpy, strlen, fileno, etc.
 */
/* Tip: link extension DLLs with "/Entry:ExtDllMain" option.  */
#define EXTENSION_DLL_MAIN \
int __stdcall ExtDllMain(void *hinstDLL, unsigned long fdwReason, void *lpvReserved); \
int __stdcall ExtDllMain(void *hinstDLL, unsigned long fdwReason, void *lpvReserved) \
{ \
	(void) hinstDLL, (void) fdwReason, (void) lpvReserved; \
	return 1; \
}
#endif /* (_MSC_VER || __MINGW32__) && GAWK_STATIC_CRT */

/* Use default implementation extension's CRT initialization routine.  */
#ifndef EXTENSION_DLL_MAIN
#define EXTENSION_DLL_MAIN
#endif

#define GAWK_PLUGIN_GPL_COMPATIBLE \
  GAWK_PLUGIN_EXTERN_C_BEGIN \
  GAWK_PLUGIN_EXPORT \
  extern int plugin_is_GPL_compatible; \
  int plugin_is_GPL_compatible; \
  GAWK_PLUGIN_EXTERN_C_END

typedef struct gawk_plugin_info {
	const char *const ext_version;
	const gawk_api_t *api;
#ifdef GAWK_USE_CRT_API
	const gawk_crt_api_t *crt;
#endif
} gawk_plugin_info_t;

#ifdef GAWK_USE_CRT_API
#define GAWK_PLUGIN(ver) \
	static gawk_plugin_info_t gawk_plugin_info = {ver, NULL, NULL}
#else
#define GAWK_PLUGIN(ver) \
	static gawk_plugin_info_t gawk_plugin_info = {ver, NULL}
#endif

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
extern int (GAWK_EXT_INIT_FUNC_NAME)(const gawk_api_t *const api_p, gawk_extension_api_ver_t *ver);
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

static inline void dl_load_init_plugin_info(gawk_plugin_info_t *info,
	const gawk_api_t *const api_p)
{
	info->api = api_p;
#ifdef GAWK_USE_CRT_API
	info->crt = api_p->crt_api;
#endif
}

static inline const gawk_api_t *gawk_api_(const gawk_plugin_info_t *info)
{
	return info->api;
}

#ifdef GAWK_USE_CRT_API
static inline const gawk_crt_api_t *gawk_crt_(const gawk_plugin_info_t *info)
{
	return info->crt;
}
#endif

#ifndef GAWK_USE_CRT_API
#define dl_load_define_support_funcs \
const gawk_api_t *gawk_api(void)	{return gawk_api_(&gawk_plugin_info);}
#else
#define dl_load_define_support_funcs \
const gawk_api_t *gawk_api(void)	{return gawk_api_(&gawk_plugin_info);} \
const gawk_crt_api_t *gawk_crt(void)	{return gawk_crt_(&gawk_plugin_info);}
#endif

static inline int gawk_check_api_version(
	const char *api_name,
	int need_major, int need_minor,
	int have_major, int have_minor,
	gawk_extension_api_ver_t *ver/*out*/)
{
	if (have_major != need_major || have_minor < need_minor) {
		ver->api_name = api_name;
		ver->need.major_version = need_major;
		ver->need.minor_version = need_minor;
		ver->have.major_version = have_major;
		ver->have.minor_version = have_minor;
		return -1;
	}
	return 0;
}

static inline awk_bool_t
dl_load_call_init_func(awk_bool_t (*func)(void))
{
	if (func == NULL)
		return awk_true;
	return (*func)();
}

static inline int dl_load_func_impl(
	const char *ext_name,
	const char *gmp_ext_name,
	const char *mpfr_ext_name,
	const char *crt_ext_name,
	gawk_plugin_info_t *info,
	const gawk_api_t *const api_p,
	gawk_extension_api_ver_t *ver,
	awk_ext_func_t func_table[],
	const size_t func_table_size,
	const char *name_space,
	awk_bool_t (*init_func)(void))
{
	size_t i;
	int errors = 0;

	(void) gmp_ext_name, (void) mpfr_ext_name, (void) crt_ext_name;

	if (gawk_check_api_version(ext_name,
		GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION,
		api_p->major_version, api_p->minor_version, ver))
		return -1;

#if defined __GNU_MP_VERSION && defined MPFR_VERSION_MAJOR
	if (gawk_check_api_version(gmp_ext_name,
		__GNU_MP_VERSION, __GNU_MP_VERSION_MINOR,
		api_p->gmp_major_version, api_p->gmp_minor_version, ver))
		return -1;
	if (gawk_check_api_version(mpfr_ext_name,
		MPFR_VERSION_MAJOR, MPFR_VERSION_MINOR,
		api_p->mpfr_major_version, api_p->mpfr_minor_version, ver))
		return -1;
#endif

#ifdef GAWK_USE_CRT_API
	if (gawk_check_api_version(crt_ext_name,
		GAWK_CRT_MAJOR_VERSION, GAWK_CRT_MINOR_VERSION,
		api_p->crt_api ? api_p->crt_api->crt_major_version : -1,
		api_p->crt_api ? api_p->crt_api->crt_minor_version : -1, ver))
		return -1;
#endif

	dl_load_init_plugin_info(info, api_p);

	/* load functions */
	for (i = 0; i < func_table_size; i++) {
		if (func_table[i].name == NULL)
			break;
		if (!api_p->api_add_ext_func(name_space, & func_table[i])) {
			api_p->api_warning("%s: could not add %s", ext_name,
					func_table[i].name);
			errors++;
		}
	}

	if (!dl_load_call_init_func(init_func)) {
		api_p->api_warning("%s: initialization function failed", ext_name);
		errors++;
	}

	if (info->ext_version != NULL)
		api_p->api_register_ext_version(info->ext_version);

	return (errors == 0);
}

#define dl_load_func(init_func, func_table, extension, name_space) \
dl_load_define_support_funcs \
GAWK_PLUGIN_EXTERN_C_BEGIN \
EXTENSION_DLL_MAIN \
GAWK_PLUGIN_EXPORT \
int (GAWK_EXT_INIT_FUNC_NAME)(const gawk_api_t *const api_p, gawk_extension_api_ver_t *ver) \
{ \
	return dl_load_func_impl( \
		#extension, \
		#extension ": GMP", \
		#extension ": MPFR", \
		#extension ": CRT", \
		&gawk_plugin_info, \
		api_p, \
		ver, \
		func_table, \
		sizeof(func_table) / sizeof(func_table[0]), \
		name_space, \
		init_func); \
} \
GAWK_PLUGIN_EXTERN_C_END

#endif /* !GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
