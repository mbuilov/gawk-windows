/*
 * testext.c - tests for the extension API.
 */

/*
 * Copyright (C) 2012
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

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

int plugin_is_GPL_compatible;

/*
 * The awk code for these tests is embedded in this file and then extracted
 * dynamically to create the script that is run together with this extension.
 * Extraction requires the format for awk code where test code is enclosed
 * in a BEGIN block, with the BEGIN and close brace on lines by themselves
 * and at the front of the lines.
 */

/*
#@load "testext"
#BEGIN {
#	dump_procinfo()
#	print ""
#}
*/
static awk_value_t *
dump_procinfo(int nargs, awk_value_t *result)
{
	assert(result != NULL);
	/* get PROCINFO as flat array and print it */
	return result;
}

/*
@load "testext"
BEGIN {
	testvar = "One Adam Twelve"
	ret = var_test("testvar")
	printf "var_test() returned %d, test_var = %s\n", ret, testvar
	print ""
}
*/

static awk_value_t *
var_test(int nargs, awk_value_t *result)
{
	awk_value_t value, value2;
	awk_value_t *valp;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 1) {
		printf("var_test: nargs not right (%d should be 1)\n", nargs);
		goto out;
	}

	/* look up a reserved variable - should fail */
	if (sym_lookup("ARGC", AWK_NUMBER, & value))
		printf("var_test: sym_lookup of ARGC failed - got a value!\n");
	else
		printf("var_test: sym_lookup of ARGC passed - did not get a value\n");

	/* look up variable whose name is passed in, should pass */
	if (get_argument(0, AWK_STRING, & value)) {
		if (sym_lookup(value.str_value.str, AWK_STRING, & value2)) {
			/* change the value, should be reflected in awk script */
			valp = make_number(42.0, & value2);

			if (sym_update(value.str_value.str, valp)) {
				printf("var_test: sym_update(\"%s\") succeeded\n", value.str_value.str);
			} else {
				printf("var_test: sym_update(\"%s\") failed\n", value.str_value.str);
				goto out;
			}
		} else {
			printf("var_test: sym_lookup(\"%s\") failed\n", value.str_value.str);
			goto out;
		}
	} else {
		printf("var_test: get_argument() failed\n");
		goto out;
	}

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	ERRNO = ""
	ret = test_errno()
	printf "test_errno() returned %d, ERRNO = %s\n", ret, ERRNO
	print ""
}
*/
static awk_value_t *
test_errno(int nargs, awk_value_t *result)
{
	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 0) {
		printf("test_errno: nargs not right (%d should be 0)\n", nargs);
		goto out;
	}

	update_ERRNO_int(ECHILD);

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	for (i = 1; i <= 10; i++)
		test_array[i] = i + 2

	printf ("length of test_array is %d, should be 10\n", length(test_array))
	ret = test_array_size(test_array);
	printf "test_array_size() returned %d, length is now %d\n", ret, length(test_array)
	print ""
}
*/

static awk_value_t *
test_array_size(int nargs, awk_value_t *result)
{
	awk_value_t value;
	size_t count = 0;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 1) {
		printf("test_array_size: nargs not right (%d should be 1)\n", nargs);
		goto out;
	}

	/* get element count and print it; should match length(array) from awk script */
	if (! get_argument(0, AWK_ARRAY, & value)) {
		printf("test_array_size: get_argument failed\n");
		goto out;
	}

	if (! get_element_count(value.array_cookie, & count)) {
		printf("test_array_size: get_element_count failed\n");
		goto out;
	}

	printf("test_array_size: incoming size is %lu\n", (unsigned long) count);

	/* clear array - length(array) should then go to zero in script */
	if (! clear_array(value.array_cookie)) {
		printf("test_array_size: clear_array failed\n");
		goto out;
	}

	make_number(1.0, result);

out:
	return result;
}

/*
BEGIN {
	n = split("one two three four five six", test_array2)
	ret = test_array_elem(test_array2, "3")
	printf "test_array_elem() returned %d, test_array2[3] = %g\n", ret, test_array2[3]
	if ("5" in test_array2)
		printf "error: test_array_elem did not remove element \"5\"\n"
	else
		printf "test_array_elem did remove element \"5\"\n"
	if ("7" in test_array2)
		printf "test_array_elem added element \"7\" --> %s\n", test_array2[7]
	else
		printf "test_array_elem did not add element \"7\"\n"
	print ""
}
*/
static awk_value_t *
test_array_elem(int nargs, awk_value_t *result)
{
	awk_value_t array, index, value;
	awk_element_t element;

	memset(& element, 0, sizeof(element));
	make_number(0.0, result);	/* default return until full success */

	assert(result != NULL);

	if (nargs != 2) {
		printf("test_array_elem: nargs not right (%d should be 2)\n", nargs);
		goto out;
	}

	/* look up an array element and print the value */
	if (! get_argument(0, AWK_ARRAY, & array)) {
		printf("test_array_elem: get_argument 0 (array) failed\n");
		goto out;
	}
	if (! get_argument(1, AWK_STRING, & index)) {
		printf("test_array_elem: get_argument 1 (index) failed\n");
		goto out;
	}
	if (! get_array_element(array.array_cookie, & index, AWK_UNDEFINED, & value)) {
		printf("test_array_elem: get_array_element failed\n");
		goto out;
	}
	printf("test_array_elem: a[\"%.*s\"] = ", (int) index.str_value.len,
			index.str_value.str);
	switch (value.val_type) {
	case AWK_UNDEFINED:
		printf("<undefined>\n");
		break;
	case AWK_ARRAY:
		printf("<array>\n");
		break;
	case AWK_STRING:
		printf("\"%.*s\"\n", (int) value.str_value.len, value.str_value.str);
		break;
	case AWK_NUMBER:
		printf("%g\n", value.num_value);
		break;
	}

	/* change the element - "3" */
	element.index = index.str_value;
	(void) make_number(42.0, & element.value);
	if (! set_array_element(array.array_cookie, & element)) {
		printf("test_array_elem: set_array_element failed\n");
		goto out;
	}

	/* delete another element - "5" */
	(void) make_string("5", 1, & index);
	if (! del_array_element(array.array_cookie, & index)) {
		printf("test_array_elem: del_array_element failed\n");
		goto out;
	}

	/* add a new element - "7" */
	(void) make_string("7", 1, & index);
	element.index = index.str_value;
	(void) make_string("seven", 5, & element.value);
	if (! set_array_element(array.array_cookie, & element)) {
		printf("test_array_elem: set_array_element failed\n");
		goto out;
	}

	/* change and deletion should be reflected in awk script */
	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	printf "Initial value of LINT is %d\n", LINT
	ret = print_do_lint();
	printf "print_do_lint() returned %d\n", ret
	LINT = ! LINT
	printf "Changed value of LINT is %d\n", LINT
	ret = print_do_lint();
	printf "print_do_lint() returned %d\n", ret
	print ""
}
*/
static awk_value_t *
print_do_lint(int nargs, awk_value_t *result)
{
	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 0) {
		printf("print_do_lint: nargs not right (%d should be 0)\n", nargs);
		goto out;
	}

	printf("print_do_lint: lint = %d\n", do_lint);

	make_number(1.0, result);

out:
	return result;
}

/*
#BEGIN {
#	n = split("one two three four five six", test_array3)
#	ret = test_array_flatten(test_array3)
#	printf "test_array_flatten() returned %d\n", ret
#	if ("3" in test_array3)
#		printf "error: test_array_flatten() did not remove element \"3\"\n"
#	else
#		printf "test_array_flatten() did remove element \"3\"\n"
#	print ""
#}
*/

static awk_value_t *
test_array_flatten(int nargs, awk_value_t *result)
{
	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 1) {
		printf("test_array_flatten: nargs not right (%d should be 1)\n", nargs);
		goto out;
	}

	/* FIXME: CODE HERE */

	make_number(1.0, result);

out:
	return result;
}

static void
create_new_array()
{
	awk_element_t element;
	awk_array_t a_cookie;
	awk_value_t index;
	awk_value_t value;

	a_cookie = create_array();

	(void) make_string("hello", 5, & index);
	element.index = index.str_value;
	(void) make_string("world", 5, & element.value);
	if (! set_array_element(a_cookie, & element)) {
		printf("create_new_array:%d: set_array_element failed\n", __LINE__);
		return;
	}

	(void) make_string("answer", 6, & index);
	element.index = index.str_value;
	(void) make_number(42.0, & element.value);
	if (! set_array_element(a_cookie, & element)) {
		printf("create_new_array:%d: set_array_element failed\n", __LINE__);
		return;
	}

	value.val_type = AWK_ARRAY;
	value.array_cookie = a_cookie;

	if (! sym_update("new_array", & value))
		printf("create_new_array: sym_update(\"new_array\") failed!\n");
}

static void at_exit0(void *data, int exit_status)
{
	printf("at_exit0 called (should be third):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = NULL,");
	printf(" exit_status = %d\n", exit_status);
}


static int data_for_1 = 0xDeadBeef;
static void at_exit1(void *data, int exit_status)
{
	int *data_p = (int *) data;

	printf("at_exit1 called (should be second):");
	if (data) {
		if (data == & data_for_1)
			printf(" (data is & data_for_1),");
		else
			printf(" (data is NOT & data_for_1),");
		printf(" data value = %#x,", *data_p);
	} else
		printf(" data = NULL,");
	printf(" exit_status = %d\n", exit_status);
}

static void at_exit2(void *data, int exit_status)
{
	printf("at_exit2 called (should be first):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = NULL,");
	printf(" exit_status = %d\n", exit_status);
}

static awk_ext_func_t func_table[] = {
	{ "dump_procinfo", dump_procinfo, 0 },
	{ "var_test", var_test, 1 },
	{ "test_errno", test_errno, 0 },
	{ "test_array_size", test_array_size, 1 },
	{ "test_array_elem", test_array_elem, 2 },
	{ "test_array_flatten", test_array_flatten, 1 },
	{ "print_do_lint", print_do_lint, 0 },
};

int dl_load(const gawk_api_t *const api_p, awk_ext_id_t id)
{
	size_t i, j;
	int errors = 0;
	awk_value_t value;
	static const char message[] = "hello, world";	/* of course */

	api = api_p;
	ext_id = id;

	if (api->major_version != GAWK_API_MAJOR_VERSION
	    || api->minor_version < GAWK_API_MINOR_VERSION) {
		fprintf(stderr, "testfuncs: version mismatch with gawk!\n");
		fprintf(stderr, "\tmy version (%d, %d), gawk version (%d, %d)\n",
			GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION,
			api->major_version, api->minor_version);
		exit(1);
	}

	/* load functions */
	for (i = 0, j = sizeof(func_table) / sizeof(func_table[0]); i < j; i++) {
		if (! add_ext_func(& func_table[i], "")) {
			warning(ext_id, "testfuncs: could not add %s\n",
					func_table[i].name);
			errors++;
		}
	}

	/* add at_exit functions */
	awk_atexit(at_exit0, NULL);
	awk_atexit(at_exit1, & data_for_1);
	awk_atexit(at_exit2, NULL);

/*
BEGIN {
	printf("answer_num = %g\n", answer_num);
	printf("message_string = %s\n", message_string);
	for (i in new_array)
		printf("new_array[\"%s\"] = \"%s\"\n", i, new_array[i])
	print ""
}
*/

	/* install some variables */
	if (! sym_update("answer_num", make_number(42, & value)))
		printf("testext: sym_update(\"answer_num\") failed!\n");

	if (! sym_update("message_string", dup_string(message, strlen(message), & value)))
		printf("testext: sym_update(\"answer_num\") failed!\n");

	create_new_array();

	return (errors == 0);
}
