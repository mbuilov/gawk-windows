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

	typedef enum {
		AWK_UNDEFINED,
		AWK_NUMBER,
		AWK_CONST_STRING,
		AWK_STRING,
		AWK_ARRAY
	} awk_valtype_t;

	typedef struct {
		const char *const str;
		const size_t len;
	} awk_const_string_t;

	typedef struct {
		char *str;
		size_t len;
	} awk_string_t;

	typedef struct {
		awk_valtype_t	val_type;
		union {
			awk_const_string	cs;
			awk_string		s;
			double			d;
			void*			a;
		} u;
#define const_str_val	u.cs
#define str_val		u.s
#define num_val		u.d
#define array_cookie	u.a
	} awk_value_t;


	typedef struct {
		const char *name;
		size_t num_args;
		void (*function)(int num_args);
	} awk_ext_func_t;

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

		/* get the number of arguments passed in function call */
		/* FIXME: Needed? Won't we pass the count in the real call? */
		size_t (*get_curfunc_arg_count)(void *ext_id);
		awk_value_t *(*get_curfunc_param)(void *ext_id, size_t count);

		/* functions to print messages */
		void (*fatal)(void *ext_id, const char *format, ...);
		void (*warning)(void *ext_id, const char *format, ...);
		void (*lintwarn)(void *ext_id, const char *format, ...);

		/* register an open hook; for opening files read-only */
		int (*register_open_hook)(void *ext_id,
				void* (*open_func)(IOBUF *));

		/* functions to update ERRNO */
		void (*update_ERRNO_int)(void *ext_id, int);
		void (*update_ERRNO_string)(void *ext_id, const char *string,
				int translate);
		void (*unset_ERRNO)(void *ext_id);

		/* check if a value received from gawk is the null string */
		int (*is_null_string)(void *ext_id, void *value);

		/* add a function to the interpreter */
		int *(add_ext_func)(void *ext_id, const awk_ext_func_t *func);

		/* add an exit call back */
		void (*awk_atexit)(void *ext_id, void (*funcp)(void *data, int exit_status), void *arg0);

		/* Symbol table access */
		awk_value_t *(*sym_lookup)(void *ext_id, const char *name);
		int (*sym_update)(void *ext_id, const char *name, awk_value_t *value);
		int (*sym_remove)(void *ext_id, const char *name);

		/* Array management */
		awk_value_t *(*get_array_element)(void *ext_id, void *a_cookie, const awk_value_t* const index);
		awk_value_t *(*set_array_element)(void *ext_id, void *a_cookie,
				const awk_value_t* const index, const awk_value_t* const value);
		awk_value_t *(*del_array_element)(void *ext_id, void *a_cookie, const awk_value_t* const index);
		size_t (*get_element_count)(void *ext_id, void *a_cookie);

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

#define get_curfunc_arg_count	api->get_curfunc_arg_count
#define get_curfunc_param	api->get_curfunc_param

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
#define sym_remove	api->sym_remove

#define get_array_element	api->get_array_element
#define set_array_element	api->set_array_element
#define del_array_element	api->del_array_element
#define get_element_count	api->get_element_count
#endif /* GAWK */

#ifdef __cplusplus
}
#endif	/* C++ */

#endif /* _GAWK_API_H */
