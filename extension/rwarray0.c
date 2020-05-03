/*
 * rwarray.c - Builtin functions to binary read / write arrays to a file.
 *
 * Arnold Robbins
 * May 2009
 * Redone June 2012
 */

 /*
  * Michael M. Builov
  * mbuilov@gmail.com
  * Ported to _MSC_VER April 2020
  */

/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2018, 2020,
 * the Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef __MINGW32__
#include <winsock2.h>
#include <stdint.h>
#elif !defined _MSC_VER
#include <arpa/inet.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

/* Include <locale.h> before "gawkapi.h" redefines setlocale().
  "gettext.h" will include <locale.h> anyway */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#define MAGIC "awkrulz\n"
#define MAJOR 3
#define MINOR 0

GAWK_PLUGIN_GPL_COMPATIBLE
GAWK_PLUGIN("rwarray0 extension: version 1.0");

static awk_bool_t write_array(fd_t fd, awk_array_t array);
static awk_bool_t write_elem(fd_t fd, awk_element_t *element);
static awk_bool_t write_value(fd_t fd, awk_value_t *val);

static awk_bool_t read_array(fd_t fd, awk_array_t array);
static awk_bool_t read_elem(fd_t fd, awk_element_t *element);
static awk_bool_t read_value(fd_t fd, awk_value_t *value);

#ifdef _MSC_VER
typedef int check_sizeof_ulong[1-2*(sizeof(unsigned long) != 4)];
typedef unsigned long uint32_t;
static inline uint32_t ntohl(uint32_t n)
{
	const unsigned i = 1;
	return (*(const char*)&i) ? (uint32_t) _byteswap_ulong(n) : n;
}
static inline uint32_t htonl(uint32_t n)
{
	return ntohl(n);
}
#endif

#ifdef _MSC_VER
# ifndef S_IRUSR
#  define S_IRUSR _S_IREAD
# endif
# ifndef S_IWUSR
#  define S_IWUSR _S_IWRITE
# endif
# ifndef O_CREAT
#  define O_CREAT _O_CREAT
# endif
# ifndef O_WRONLY
#  define O_WRONLY _O_WRONLY
# endif
# ifndef O_TRUNC
#  define O_TRUNC _O_TRUNC
# endif
# ifndef O_RDONLY
#  define O_RDONLY _O_RDONLY
# endif
# ifndef O_BINARY
#  define O_BINARY _O_BINARY
# endif
#endif

/*
 * Format of array info:
 *
 * MAGIC		8 bytes
 * Major version	4 bytes - network order
 * Minor version	4 bytes - network order
 * Element count	4 bytes - network order
 * Elements
 *
 * For each element:
 * Length of index val:	4 bytes - network order
 * Index val as characters (N bytes)
 * Value type		4 bytes (0 = string, 1 = number, 2 = array)
 * IF string:
 * 	Length of value	4 bytes
 * 	Value as characters (N bytes)
 * ELSE IF number:
 * 	8 bytes as native double
 * ELSE
 * 	Element count
 * 	Elements
 * END IF
 */

/* do_writea --- write an array */

static awk_value_t *
do_writea(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename, array;
	fd_t fd = INVALID_HANDLE;
	uint32_t major = MAJOR;
	uint32_t minor = MINOR;

	(void) unused;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs < 2)
		goto out;

	/* directory is first arg, array to dump is second */
	if (! get_argument(0, AWK_STRING, & filename)) {
		warning(_("do_writea: argument 0 is not a string"));
		errno = EINVAL;
		goto done1;
	}

	if (! get_argument(1, AWK_ARRAY, & array)) {
		warning(_("do_writea: argument 1 is not an array"));
		errno = EINVAL;
		goto done1;
	}

	/* open the file, if error, set ERRNO and return */
	/* fd = creat(filename.str_value.str, 0600); */
	fd = open(filename.str_value.str,
		O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
		S_IRUSR | S_IWUSR);
	if (fd < 0)
		goto done1;

	if (write(fd, MAGIC, strlen(MAGIC)) != (ssize_t) strlen(MAGIC))
		goto done1;

	major = htonl(major);
	if (write(fd, & major, sizeof(major)) != sizeof(major))
		goto done1;

	minor = htonl(minor);
	if (write(fd, & minor, sizeof(minor)) != sizeof(minor))
		goto done1;

	if (write_array(fd, array.array_cookie)) {
		make_number(1.0, result);
		goto done0;
	}

done1:
	update_ERRNO_int(errno);
	unlink(filename.str_value.str);

done0:
	close(fd);
out:
	return result;
}


/* write_array --- write out an array or a sub-array */

static awk_bool_t
write_array(fd_t fd, awk_array_t array)
{
	uint32_t i, n, count;
	awk_flat_array_t *flat_array;

	if (! flatten_array(array, & flat_array)) {
		warning(_("write_array: could not flatten array"));
		return awk_false;
	}

	n = (uint32_t) flat_array->count;

	count = htonl(n);
	if (write(fd, & count, sizeof(count)) != sizeof(count))
		return awk_false;

	for (i = 0; i < n; i++) {
		if (! write_elem(fd, & flat_array->elements[i])) {
			(void) release_flattened_array(array, flat_array);
			return awk_false;
		}
	}

	if (! release_flattened_array(array, flat_array)) {
		warning(_("write_array: could not release flattened array"));
		return awk_false;
	}

	return awk_true;
}

/* write_elem --- write out a single element */

static awk_bool_t
write_elem(fd_t fd, awk_element_t *element)
{
	uint32_t indexval_len;
	ssize_t write_count;
	uint32_t len;

	len = (uint32_t) element->index.str_value.len;

	indexval_len = htonl(len);
	if (write(fd, & indexval_len, sizeof(indexval_len)) != sizeof(indexval_len))
		return awk_false;

	if (len > 0) {
		write_count = write(fd, element->index.str_value.str, len);
		if (write_count != (ssize_t) len)
			return awk_false;
	}

	return write_value(fd, & element->value);
}

/* write_value --- write a number or a string or a array */

static awk_bool_t
write_value(fd_t fd, awk_value_t *val)
{
	uint32_t code, len;

	if (val->val_type == AWK_ARRAY) {
		code = htonl(2);
		if (write(fd, & code, sizeof(code)) != sizeof(code))
			return awk_false;
		return write_array(fd, val->array_cookie);
	}

	if (val->val_type == AWK_NUMBER) {
		code = htonl(1);
		if (write(fd, & code, sizeof(code)) != sizeof(code))
			return awk_false;

		if (write(fd, & val->num_value, sizeof(val->num_value)) != sizeof(val->num_value))
			return awk_false;
	} else {
		code = 0;
		if (write(fd, & code, sizeof(code)) != sizeof(code))
			return awk_false;

		len = htonl((uint32_t) val->str_value.len);
		if (write(fd, & len, sizeof(len)) != sizeof(len))
			return awk_false;

		if (write(fd, val->str_value.str, (uint32_t) val->str_value.len)
				!= (ssize_t) val->str_value.len)
			return awk_false;
	}

	return awk_true;
}

/* do_reada --- read an array */

static awk_value_t *
do_reada(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename, array;
	fd_t fd = INVALID_HANDLE;
	uint32_t major;
	uint32_t minor;
	char magic_buf[30];

	(void) unused;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs < 2)
		goto out;

	/* directory is first arg, array to read is second */
	if (! get_argument(0, AWK_STRING, & filename)) {
		warning(_("do_reada: argument 0 is not a string"));
		errno = EINVAL;
		goto done1;
	}

	if (! get_argument(1, AWK_ARRAY, & array)) {
		warning(_("do_reada: argument 1 is not an array"));
		errno = EINVAL;
		goto done1;
	}

	fd = open(filename.str_value.str, O_RDONLY | O_BINARY);
	if (fd < 0)
		goto done1;

	memset(magic_buf, '\0', sizeof(magic_buf));
	if (read(fd, magic_buf, strlen(MAGIC)) != (ssize_t) strlen(MAGIC)) {
		errno = EBADF;
		goto done1;
	}

	if (strcmp(magic_buf, MAGIC) != 0) {
		errno = EBADF;
		goto done1;
	}

	if (read(fd, & major, sizeof(major)) != sizeof(major)) {
		errno = EBADF;
		goto done1;
	}
	major = ntohl(major);

	if (major != MAJOR) {
		errno = EBADF;
		goto done1;
	}

	if (read(fd, & minor, sizeof(minor)) != sizeof(minor)) {
		/* read() sets errno */
		goto done1;
	}

	minor = ntohl(minor);
	if (minor != MINOR) {
		errno = EBADF;
		goto done1;
	}

	if (! clear_array(array.array_cookie)) {
		errno = ENOMEM;
		warning(_("do_reada: clear_array failed"));
		goto done1;
	}

	if (read_array(fd, array.array_cookie)) {
		make_number(1.0, result);
		goto done0;
	}

done1:
	update_ERRNO_int(errno);
done0:
	if (fd >= 0)
		close(fd);
out:
	return result;
}


/* read_array --- read in an array or sub-array */

static awk_bool_t
read_array(fd_t fd, awk_array_t array)
{
	uint32_t i;
	uint32_t count;
	awk_element_t new_elem;

	if (read(fd, & count, sizeof(count)) != sizeof(count))
		return awk_false;

	count = ntohl(count);

	for (i = 0; i < count; i++) {
		if (! read_elem(fd, & new_elem))
			return awk_false;

		/* add to array */
		if (! set_array_element_by_elem(array, & new_elem)) {
			warning(_("read_array: set_array_element failed"));
			return awk_false;
		}
	}

	return awk_true;
}

/* read_elem --- read in a single element */

static awk_bool_t
read_elem(fd_t fd, awk_element_t *element)
{
	uint32_t index_len;
	ssize_t ret;

	if ((ret = read(fd, & index_len, sizeof(index_len))) != sizeof(index_len))
		return awk_false;
	index_len = ntohl(index_len);

	memset(element, 0, sizeof(*element));

	if (index_len > 0) {
		char *str = (char*) malloc(index_len + 1);
		if (str == NULL)
			return awk_false;

		if (read(fd, str, index_len) != (ssize_t) index_len) {
			free(str);
			return awk_false;
		}

		str[index_len] = '\0';

		make_malloced_string(str, index_len, & element->index);
	} else
		make_null_string(& element->index);

	if (! read_value(fd, & element->value)) {
		if (index_len > 0)
			free(element->index.str_value.str);
		return awk_false;
	}

	return awk_true;
}

/* read_value --- read a number or a string */

static awk_bool_t
read_value(fd_t fd, awk_value_t *value)
{
	uint32_t code, len;

	if (read(fd, & code, sizeof(code)) != sizeof(code))
		return awk_false;

	code = ntohl(code);

	if (code == 2) {
		awk_array_t array = create_array();

		if (! read_array(fd, array))
			return awk_false;

		/* hook into value */
		value->val_type = AWK_ARRAY;
		value->array_cookie = array;
	} else if (code == 1) {
		double d;

		if (read(fd, & d, sizeof(d)) != sizeof(d))
			return awk_false;

		/* hook into value */
		value->val_type = AWK_NUMBER;
		value->num_value = d;
	} else {
		if (read(fd, & len, sizeof(len)) != sizeof(len))
			return awk_false;
		len = ntohl(len);
		value->val_type = AWK_STRING;
		value->str_value.len = len;
		value->str_value.str = (char*) malloc(len + 1);
		if (value->str_value.str == NULL)
			return awk_false;

		if (read(fd, value->str_value.str, len) != (ssize_t) len) {
			free(value->str_value.str);
			return awk_false;
		}
		value->str_value.str[len] = '\0';
	}

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ "writea", do_writea, 2, 2, awk_false, NULL },
	{ "reada", do_reada, 2, 2, awk_false, NULL },
};


/* define the dl_load function using the boilerplate macro */

dl_load_func(NULL, func_table, rwarray, "")
