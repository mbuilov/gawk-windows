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
@load testext
BEGIN {
	dump_procinfo()
}
*/
static awk_value_t *
dump_procinfo(int nargs, awk_value_t *result)
{
	/* get PROCINFO as flat array and print it */
}

/*
BEGIN {
	testvar = "One Adam Twelve"
	ret = var_test("testvar")
	printf "var_test() returned %d, test_var = %s\n", ret, testvar
}
*/

static awk_value_t *
var_test(int nargs, awk_value_t *result)
{
	awk_value_t value;

	if (nargs != 1 || result == NULL)
		return NULL;

	/* look up a reserved variable - should fail */
	if (sym_lookup("ARGC", & value, AWK_NUMBER) != NULL)
		printf("var_test: sym_lookup of ARGC failed - got a value!\n");
	else
		printf("var_test: sym_lookup of ARGC passed\n");

	/* look up variable whose name is passed in, should pass */
	/* change the value, should be reflected in awk script */
}

/*
BEGIN {
	ERRNO = ""
	ret = test_errno()
	printf "test_errno() returned %d, ERRNO = %s\n", ret, ERRNO
}
*/
static awk_value_t *
test_errno(int nargs, awk_value_t *result)
{
	update_ERRNO_int(ECHILD);
}

/*
BEGIN {
	for (i = 1; i <= 10; i++)
		test_array[i] = i + 2

	printf ("length of test_array is %d, should be 10\n", length(test_array)
	ret = test_array_size(test_array);
	printf "test_array_size() returned %d, length is now %d\n", ret, length(test_array)
}
*/

static awk_value_t *
test_array_size(int nargs, awk_value_t *result)
{
	/* get element count and print it; should match length(array) from awk script */
	/* clear array - length(array) should then go to zero in script */
}

/*
BEGIN {
	n = split("one two three four five six", test_array2)
	ret = test_array_elem(test_array2, "3")
	printf "test_array_elem() returned %d, test_array2[3] = %g\n", ret, test_array2[3]
}
*/
static awk_value_t *
test_array_elem(int nargs, awk_value_t *result)
{
	/* look up an array element and print the value */
	/* change the element */
	/* delete another element */
	/* change and deletion should be reflected in awk script */
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
}
*/
static awk_value_t *
print_do_lint(int nargs, awk_value_t *result)
{
	printf("print_do_lint: lint = %d\n", do_lint);
}

static void at_exit0(void *data, int exit_status)
{
	printf("at_exit0 called (should be third):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = <null>,");
	printf(" exit_status = %d\n", exit_status);
}


static int data_for_1 = 0xDeadBeef;
static void at_exit1(void *data, int exit_status)
{
	printf("at_exit1 called (should be second):");
	if (data) {
		printf(" data = %p", data);
		if (data == & data_for_1)
			printf(" (data is & data_for_1),");
		else
			printf(" (data is NOT & data_for_1),");
	} else
		printf(" data = <null>,");
	printf(" exit_status = %d\n", exit_status);
}

static void at_exit2(void *data, int exit_status)
{
	printf("at_exit2 called (should be first):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = <null>,");
	printf(" exit_status = %d\n", exit_status);
}

static awk_ext_func_t func_table[] = {
	{ "dump_procinfo", dump_procinfo, 0 },
	{ "var_test", var_test, 1 },
	{ "test_errno", test_errno, 0 },
	{ "test_array_size", test_array_size, 1 },
	{ "test_array_elem", test_array_elem, 2 },
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
}
*/

	/* install some variables */
	if (! sym_update("answer_num", make_number(42, & value)))
			printf("textext: sym_update(\"answer_num\") failed!\n");

	if (! sym_update("message_string", dup_string(message, strlen(message), & value)))
			printf("textext: sym_update(\"answer_num\") failed!\n");

	return (errors == 0);
}
