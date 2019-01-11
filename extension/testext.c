/*
 * testext.c - tests for the extension API.
 */

/*
 * Copyright (C) 2012, 2013, 2014, 2015, 2017, 2018
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gawkapi.h"

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "testext extension: version 1.0";

int plugin_is_GPL_compatible;

static void fill_in_array(awk_value_t *value);

#ifdef __MINGW32__
unsigned int
getuid (void)
{
  /* See pc/getid.c.  */
  return 0;
}
#endif

/* valrep2str --- turn a value into a string */

static const char *
valrep2str(const awk_value_t *value)
{
	static char buf[BUFSIZ];
	int size = BUFSIZ - 3;

	switch (value->val_type) {
	case AWK_UNDEFINED:
		strcpy(buf, "<undefined>");
		break;
	case AWK_ARRAY:
		strcpy(buf, "<array>");
		break;
	case AWK_SCALAR:
		strcpy(buf, "<scalar>");
		break;
	case AWK_VALUE_COOKIE:
		strcpy(buf, "<value-cookie>");
		break;
	case AWK_REGEX:
	case AWK_STRNUM:
	case AWK_STRING:
		if (value->str_value.len < size)
			size = value->str_value.len;
		sprintf(buf, "\"%.*s\"",
				size,
				value->str_value.str);
		break;
	case AWK_NUMBER:
		sprintf(buf, "%g", value->num_value);
		break;
	}
	return buf;
}

/*
 * The awk code for these tests is embedded in this file and then extracted
 * dynamically to create the script that is run together with this extension.
 * Extraction requires the format for awk code where test code is enclosed
 * in a BEGIN block, with the BEGIN and close brace on lines by themselves
 * and at the front of the lines.
 */

/*
@load "testext"
BEGIN {
	n = split("blacky rusty sophie raincloud lucky", pets)
	printf("pets has %d elements\n", length(pets))
	ret = dump_array_and_delete("pets", "3")
	printf("dump_array_and_delete(pets) returned %d\n", ret)
	if ("3" in pets)
		printf("dump_array_and_delete() did NOT remove index \"3\"!\n")
	else
		printf("dump_array_and_delete() did remove index \"3\"!\n")
	print ""
}
*/
static awk_value_t *
dump_array_and_delete(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t value, value2, value3;
	awk_flat_array_t *flat_array;
	size_t count;
	char *name;
	int i;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 2) {
		printf("dump_array_and_delete: nargs not right (%d should be 2)\n", nargs);
		goto out;
	}

	/* get argument named array as flat array and print it */
	if (get_argument(0, AWK_STRING, & value)) {
		name = value.str_value.str;
		if (sym_lookup(name, AWK_ARRAY, & value2))
			printf("dump_array_and_delete: sym_lookup of %s passed\n", name);
		else {
			printf("dump_array_and_delete: sym_lookup of %s failed\n", name);
			goto out;
		}
	} else {
		printf("dump_array_and_delete: get_argument(0) failed\n");
		goto out;
	}

	if (! get_element_count(value2.array_cookie, & count)) {
		printf("dump_array_and_delete: get_element_count failed\n");
		goto out;
	}

	printf("dump_array_and_delete: incoming size is %lu\n", (unsigned long) count);

	if (! flatten_array(value2.array_cookie, & flat_array)) {
		printf("dump_array_and_delete: could not flatten array\n");
		goto out;
	}

	if (flat_array->count != count) {
		printf("dump_array_and_delete: flat_array->count (%lu) != count (%lu)\n",
				(unsigned long) flat_array->count,
				(unsigned long) count);
		goto out;
	}

	if (! get_argument(1, AWK_STRING, & value3)) {
		printf("dump_array_and_delete: get_argument(1) failed\n");
		goto out;
	}

	for (i = 0; i < flat_array->count; i++) {
		printf("\t%s[\"%.*s\"] = %s\n",
			name,
			(int) flat_array->elements[i].index.str_value.len,
			flat_array->elements[i].index.str_value.str,
			valrep2str(& flat_array->elements[i].value));

		if (strcmp(value3.str_value.str, flat_array->elements[i].index.str_value.str) == 0) {
			flat_array->elements[i].flags |= AWK_ELEMENT_DELETE;
			printf("dump_array_and_delete: marking element \"%s\" for deletion\n",
				flat_array->elements[i].index.str_value.str);
		}
	}

	if (! release_flattened_array(value2.array_cookie, flat_array)) {
		printf("dump_array_and_delete: could not release flattened array\n");
		goto out;
	}

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	ENVIRON["testext"]++
	try_modify_environ()
	if ("testext" in ENVIRON)
		print "try_del_environ() could not delete element - pass"
	else
		print "try_del_environ() deleted element! - fail"
	if ("testext2" in ENVIRON)
		print "try_del_environ() added an element - fail"
	else
		print "try_del_environ() could not add an element - pass"
}
*/

static awk_value_t *
try_modify_environ(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t value, index, newvalue;
	awk_flat_array_t *flat_array;
	awk_array_t environ_array;
	size_t count;
	int i;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 0) {
		printf("try_modify_environ: nargs not right (%d should be 0)\n", nargs);
		goto out;
	}

	/* get ENVIRON array */
	if (sym_lookup("ENVIRON", AWK_ARRAY, & value))
		printf("try_modify_environ: sym_lookup of ENVIRON passed\n");
	else {
		printf("try_modify_environ: sym_lookup of ENVIRON failed\n");
		goto out;
	}

	environ_array = value.array_cookie;
	if (! get_element_count(environ_array, & count)) {
		printf("try_modify_environ: get_element_count failed\n");
		goto out;
	}

	/* setting an array element should fail */
	(void) make_const_string("testext2", 8, & index);
	(void) make_const_string("a value", 7, & value);
	if (set_array_element(environ_array, & index, & newvalue)) {
		printf("try_modify_environ: set_array_element of ENVIRON passed\n");
	} else {
		printf("try_modify_environ: set_array_element of ENVIRON failed\n");
		gawk_free(index.str_value.str);
		gawk_free(value.str_value.str);
	}

	if (! flatten_array(environ_array, & flat_array)) {
		printf("try_modify_environ: could not flatten array\n");
		goto out;
	}

	if (flat_array->count != count) {
		printf("try_modify_environ: flat_array->count (%lu) != count (%lu)\n",
				(unsigned long) flat_array->count,
				(unsigned long) count);
		goto out;
	}

	for (i = 0; i < flat_array->count; i++) {
		/* don't print */
	/*
		printf("\t%s[\"%.*s\"] = %s\n",
			name,
			(int) flat_array->elements[i].index.str_value.len,
			flat_array->elements[i].index.str_value.str,
			valrep2str(& flat_array->elements[i].value));
	*/
		if (strcmp("testext", flat_array->elements[i].index.str_value.str) == 0) {
			flat_array->elements[i].flags |= AWK_ELEMENT_DELETE;
			printf("try_modify_environ: marking element \"%s\" for deletion\n",
				flat_array->elements[i].index.str_value.str);
		}
	}

	if (! release_flattened_array(environ_array, flat_array)) {
		printf("try_modify_environ: could not release flattened array\n");
		goto out;
	}

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	testvar = "One Adam Twelve"
	ret = var_test("testvar")
	printf("var_test() returned %d, test_var = %s\n", ret, testvar)
	print ""
}
*/

static awk_value_t *
var_test(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t value, value2;
	awk_value_t *valp;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 1) {
		printf("var_test: nargs not right (%d should be 1)\n", nargs);
		goto out;
	}

	/* look up PROCINFO - should succeed fail */
	if (sym_lookup("PROCINFO", AWK_ARRAY, & value))
		printf("var_test: sym_lookup of PROCINFO passed - got a value!\n");
	else
		printf("var_test: sym_lookup of PROCINFO failed - did not get a value\n");

	/* look up a reserved variable - should pass */
	if (sym_lookup("ARGC", AWK_NUMBER, & value))
		printf("var_test: sym_lookup of ARGC passed - got a value!\n");
	else
		printf("var_test: sym_lookup of ARGC failed - did not get a value\n");

	/* now try to set it - should fail */
	value.num_value++;
	if (sym_update("ARGC", & value))
		printf("var_test: sym_update of ARGC passed and should not have!\n");
	else
		printf("var_test: sym_update of ARGC failed - correctly\n");

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
	printf("test_errno() returned %d, ERRNO = %s\n", ret, ERRNO)
	print ""
}
*/
static awk_value_t *
test_errno(int nargs, awk_value_t *result, struct awk_ext_func *unused)
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
 * 3/2015: This test is no longer strictly necessary,
 * since PROCINFO is no longer a deferred variable.
 * But we leave it in for safety, anyway.
 */
/*
BEGIN {
	print "test_deferred returns", test_deferred()
	print ""
}
*/
static awk_value_t *
test_deferred(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t arr;
	awk_value_t index, value;
	const struct nval {
		const char *name;
		double val;
	} seed[] = {
		{ "fubar", 9.0, },
		{ "rumpus", -5.0, },
	};
	struct nval sysval[] = {
		{ "uid", getuid(), },
		{ "api_major", GAWK_API_MAJOR_VERSION, },
	};
	size_t i;

	assert(result != NULL);
	make_number(0.0, result);

	if (nargs != 0) {
		printf("test_deferred: nargs not right (%d should be 0)\n", nargs);
		goto out;
	}

	if (! sym_lookup("PROCINFO", AWK_ARRAY, & arr)) {
		printf("test_deferred: %d: sym_lookup failed\n", __LINE__);
		goto out;
	}

	for (i = 0; i < sizeof(seed)/sizeof(seed[0]); i++) {
		make_const_string(seed[i].name, strlen(seed[i].name), & index);
		make_number(seed[i].val, & value);
		if (! set_array_element(arr.array_cookie, & index, & value)) {
			printf("test_deferred: %d: set_array_element(%s) failed\n", __LINE__, seed[i].name);
			goto out;
		}
	}

	/* test that it still contains the values we loaded */
	for (i = 0; i < sizeof(seed)/sizeof(seed[0]); i++) {
		make_const_string(seed[i].name, strlen(seed[i].name), & index);
		make_null_string(& value);
		if (! get_array_element(arr.array_cookie, &index, AWK_NUMBER, & value)) {
			printf("test_deferred: %d: get_array_element(%s) failed\n", __LINE__, seed[i].name);
			goto out;
		}
		printf("%s = %g\n", seed[i].name, value.num_value);
	}

	/* check a few automatically-supplied values */
	for (i = 0; i < sizeof(sysval)/sizeof(sysval[0]); i++) {
		make_const_string(sysval[i].name, strlen(sysval[i].name), & index);
		make_null_string(& value);
		if (! get_array_element(arr.array_cookie, &index, AWK_NUMBER, & value)) {
			printf("test_deferred: %d: get_array_element(%s) failed\n", __LINE__, sysval[i].name);
			goto out;
		}
		printf("%s matches %d\n", sysval[i].name, (value.num_value == sysval[i].val));
	}

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	for (i = 1; i <= 10; i++)
		test_array[i] = i + 2

	printf("length of test_array is %d, should be 10\n", length(test_array))
	ret = test_array_size(test_array);
	printf("test_array_size() returned %d, length is now %d\n", ret, length(test_array))
	print ""
}
*/

static awk_value_t *
test_array_size(int nargs, awk_value_t *result, struct awk_ext_func *unused)
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
	printf("test_array_elem() returned %d, test_array2[3] = %g\n", ret, test_array2[3])
	if ("5" in test_array2)
		printf("error: test_array_elem() did not remove element \"5\"\n")
	else
		printf("test_array_elem() did remove element \"5\"\n")
	if ("7" in test_array2)
		printf("test_array_elem() added element \"7\" --> %s\n", test_array2[7])
	else
		printf("test_array_elem() did not add element \"7\"\n")
	if ("subarray" in test_array2) {
		if (isarray(test_array2["subarray"])) {
			for (i in test_array2["subarray"])
				printf("test_array2[\"subarray\"][\"%s\"] = %s\n",
					i, test_array2["subarray"][i])
		} else
			printf("test_array_elem() added element \"subarray\" as scalar\n")
	} else
		printf("test_array_elem() did not add element \"subarray\"\n")
	print ""
}
*/
static awk_value_t *
test_array_elem(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t array, index, index2, value;

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
	(void) make_const_string(index.str_value.str, index.str_value.len, & index2);
	if (! get_array_element(array.array_cookie, & index2, AWK_UNDEFINED, & value)) {
		printf("test_array_elem: get_array_element failed\n");
		goto out;
	}
	printf("test_array_elem: a[\"%.*s\"] = %s\n",
			(int) index.str_value.len,
			index.str_value.str,
			valrep2str(& value));

	/* change the element - "3" */
	(void) make_number(42.0, & value);
	(void) make_const_string(index.str_value.str, index.str_value.len, & index2);
	if (! set_array_element(array.array_cookie, & index2, & value)) {
		printf("test_array_elem: set_array_element failed\n");
		goto out;
	}

	/* delete another element - "5" */
	(void) make_const_string("5", 1, & index);
	if (! del_array_element(array.array_cookie, & index)) {
		printf("test_array_elem: del_array_element failed\n");
		goto out;
	}

	/* add a new element - "7" */
	(void) make_const_string("7", 1, & index);
	(void) make_const_string("seven", 5, & value);
	if (! set_array_element(array.array_cookie, & index, & value)) {
		printf("test_array_elem: set_array_element failed\n");
		goto out;
	}

	/* add a subarray */
	(void) make_const_string("subarray", 8, & index);
	fill_in_array(& value);
	if (! set_array_element(array.array_cookie, & index, & value)) {
		printf("test_array_elem: set_array_element (subarray) failed\n");
		goto out;
	}

	/* change and deletion should be reflected in awk script */
	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	ret = test_array_param(a_new_array)
	printf("test_array_param() returned %d\n", ret)
	printf("isarray(a_new_array) = %d\n", isarray(a_new_array))
	if (isarray(a_new_array))
		for (i in a_new_array)
			printf("a_new_array[\"%s\"] = %s\n",
				i, a_new_array[i])

	a_scalar = 42
	ret = test_array_param(a_scalar)
	printf("test_array_param() returned %d\n", ret)
	printf("isarray(a_scalar) = %d\n", isarray(a_scalar))
	print ""
}
*/

static awk_value_t *
test_array_param(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t new_array;
	awk_value_t arg0;

	(void) nargs;		/* silence warnings */
	make_number(0.0, result);

	if (! get_argument(0, AWK_UNDEFINED, & arg0)) {
		printf("test_array_param: could not get argument\n");
		goto out;
	}

	if (arg0.val_type != AWK_UNDEFINED) {
		printf("test_array_param: argument is not undefined (%d)\n",
				arg0.val_type);
		goto out;
	}

	fill_in_array(& new_array);
	if (! set_argument(0, new_array.array_cookie)) {
		printf("test_array_param: could not change type of argument\n");
		goto out;
	}

	make_number(1.0, result);
out:
	return result;	/* for now */
}

/*
BEGIN {
	printf("Initial value of LINT is %d\n", LINT)
	ret = print_do_lint();
	printf("print_do_lint() returned %d\n", ret)
	LINT = ! LINT
	printf("Changed value of LINT is %d\n", LINT)
	ret = print_do_lint();
	printf("print_do_lint() returned %d\n", ret)
	print ""
}
*/
static awk_value_t *
print_do_lint(int nargs, awk_value_t *result, struct awk_ext_func *unused)
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
BEGIN {
	n = split("1 3 5 7 9 11", nums)
	m = split("the quick brown fox jumps over the lazy dog", strings)
	for (i in nums) {
		ret = test_scalar(nums[i] + 0)
		printf("test_scalar(%d) returned %d, the_scalar is %d\n", nums[i], ret, the_scalar)
	}
	for (i in strings) {
		ret = test_scalar(strings[i])
		printf("test_scalar(%s) returned %d, the_scalar is %s\n", strings[i], ret, the_scalar)
	}
}
*/

/* test_scalar --- test scalar cookie */

static awk_value_t *
test_scalar(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t new_value, new_value2;
	awk_value_t the_scalar;

	(void) nargs;		/* silence warnings */
	make_number(0.0, result);

	if (! sym_lookup("the_scalar", AWK_SCALAR, & the_scalar)) {
		printf("test_scalar: could not get scalar cookie\n");
		goto out;
	}

	if (! get_argument(0, AWK_UNDEFINED, & new_value)) {
		printf("test_scalar: could not get argument\n");
		goto out;
	} else if (new_value.val_type != AWK_STRING && new_value.val_type != AWK_NUMBER) {
		printf("test_scalar: argument is not a scalar\n");
		goto out;
	}

	if (new_value.val_type == AWK_STRING) {
		make_const_string(new_value.str_value.str, new_value.str_value.len, & new_value2);
	} else {
		new_value2 = new_value;
	}

	if (! sym_update_scalar(the_scalar.scalar_cookie, & new_value2)) {
		printf("test_scalar: could not update new_value2!\n");
		goto out;
	}

	make_number(1.0, result);

out:
	return result;
}

/*
BEGIN {
	test_scalar_reserved()
}
*/

/* test_scalar_reserved --- test scalar cookie on special variable */

static awk_value_t *
test_scalar_reserved(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t new_value;
	awk_value_t the_scalar;

	(void) nargs;		/* silence warnings */
	make_number(0.0, result);

	/* look up a reserved variable - should pass */
	if (sym_lookup("ARGC", AWK_SCALAR, & the_scalar)) {
		printf("test_scalar_reserved: sym_lookup of ARGC passed - got a value!\n");
	} else {
		printf("test_scalar_reserved: sym_lookup of ARGC failed - did not get a value\n");
		goto out;
	}

	/* updating it should fail */
	make_number(42.0, & new_value);
	if (! sym_update_scalar(the_scalar.scalar_cookie, & new_value)) {
		printf("test_scalar_reserved: could not update new_value2 for ARGC - pass\n");
	} else {
		printf("test_scalar_reserved: was able to update new_value2 for ARGC - fail\n");
		goto out;
	}

	make_number(1.0, result);

out:
	return result;
}

/*
BEGIN {
	print "line 1" > "testexttmp.txt"
	print "line 2" > "testexttmp.txt"
	print "line 3" > "testexttmp.txt"
	close("testexttmp.txt")
	ARGV[1] = "testexttmp.txt"
	ARGC = 2
	getline
	getline
	getline		# now NR should be 3
#	system("rm testexttmp.txt")
	ret = test_indirect_vars()	# should get correct value of NR
	printf("test_indirect_var() return %d\n", ret)
	delete ARGV[1]
	print ""
}
*/

/* test_indirect_vars --- test that access to NR, NF, get correct vales */

static awk_value_t *
test_indirect_vars(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t value;
	char *name = "NR";

	(void) nargs;		/* silence warnings */
	assert(result != NULL);
	make_number(0.0, result);

	/* system("rm testexttmp.txt") */
	(void) unlink("testexttmp.txt");

	if (sym_lookup(name, AWK_NUMBER, & value))
		printf("test_indirect_var: sym_lookup of %s passed\n", name);
	else {
		printf("test_indirect_var: sym_lookup of %s failed\n", name);
		goto out;
	}

	printf("test_indirect_var: value of NR is %g\n", value.num_value);

	make_number(1.0, result);
out:
	return result;
}

/*
BEGIN {
	outfile = "testexttmp.txt"
	alias = ".test.alias"
	print "line 1" > outfile
	print "line 2" > outfile
	print "line 3" > outfile
	close(outfile)
	ret = test_get_file(outfile, alias)
	printf "test_get_file returned %d\n", ret
	nr = 0
	while ((getline < alias) > 0)
		printf "File [%s] nr [%s]: %s\n", alias, ++nr, $0
	close(alias)
#	system("rm " outfile)
	print ""
}
*/

/* test_get_file --- test that we can create a file */

static awk_value_t *
test_get_file(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename, alias;
	int fd;
	const awk_input_buf_t *ibuf;
	const awk_output_buf_t *obuf;

	if (nargs != 2) {
		printf("%s: nargs not right (%d should be 2)\n", "test_get_file", nargs);
		return make_number(-1.0, result);
	}

	if (! get_argument(0, AWK_STRING, & filename)) {
		printf("%s: cannot get first arg\n", "test_get_file");
		return make_number(-1.0, result);
	}
	if (! get_argument(1, AWK_STRING, & alias)) {
		printf("%s: cannot get second arg\n", "test_get_file");
		return make_number(-1.0, result);
	}
	if ((fd = open(filename.str_value.str, O_RDONLY)) < 0) {
		printf("%s: open(%s) failed\n", "test_get_file", filename.str_value.str);
		return make_number(-1.0, result);
	}
	if (! get_file(alias.str_value.str, strlen(alias.str_value.str), "<", fd, &ibuf, &obuf)) {
		printf("%s: get_file(%s) failed\n", "test_get_file", alias.str_value.str);
		return make_number(-1.0, result);
	}
	if (! ibuf || ibuf->fd != fd) {
		printf("%s: get_file(%s) returned fd %d instead of %d\n", "test_get_file", alias.str_value.str, ibuf ? ibuf->fd : -1, fd);
		return make_number(-1.0, result);
	}
	return make_number(0.0, result);
}

/* do_get_file --- provide access to get_file API */

static awk_value_t *
do_get_file(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename, filetype, fd, res;
	const awk_input_buf_t *ibuf;
	const awk_output_buf_t *obuf;

	if (nargs != 4) {
		printf("%s: nargs not right (%d should be 4)\n", "get_file", nargs);
		return make_number(-1.0, result);
	}

	if (! get_argument(0, AWK_STRING, & filename)) {
		printf("%s: cannot get first arg\n", "get_file");
		return make_number(-1.0, result);
	}
	if (! get_argument(1, AWK_STRING, & filetype)) {
		printf("%s: cannot get second arg\n", "get_file");
		return make_number(-1.0, result);
	}
	if (! get_argument(2, AWK_NUMBER, & fd)) {
		printf("%s: cannot get third arg\n", "get_file");
		return make_number(-1.0, result);
	}
	if (! get_argument(3, AWK_ARRAY, & res)) {
		printf("%s: cannot get fourth arg\n", "get_file");
		return make_number(-1.0, result);
	}
	clear_array(res.array_cookie);

	if (! get_file(filename.str_value.str, strlen(filename.str_value.str), filetype.str_value.str, fd.num_value, &ibuf, &obuf)) {
		printf("%s: get_file(%s, %s, %d) failed\n", "get_file", filename.str_value.str, filetype.str_value.str, (int)(fd.num_value));
		return make_number(0.0, result);
	}

	if (ibuf) {
		awk_value_t idx, val;
		set_array_element(res.array_cookie,
				  make_const_string("input", 5, & idx),
				  make_number(ibuf->fd, & val));
		if (ibuf->name)
			set_array_element(res.array_cookie,
					  make_const_string("input_name", 10, & idx),
					  make_const_string(ibuf->name, strlen(ibuf->name), & val));
	}
	if (obuf) {
		awk_value_t idx, val;
		set_array_element(res.array_cookie,
				  make_const_string("output", 6, & idx),
				  make_number(obuf->fp ? fileno(obuf->fp) : -1,
				  	      & val));
		if (obuf->name)
			set_array_element(res.array_cookie,
					  make_const_string("output_name", 11, & idx),
					  make_const_string(obuf->name, strlen(obuf->name), & val));
	}
	return make_number(1.0, result);
}

/* fill_in_array --- fill in a new array */

static void
fill_in_array(awk_value_t *new_array)
{
	awk_array_t a_cookie;
	awk_value_t index, value;

	a_cookie = create_array();

	(void) make_const_string("hello", 5, & index);
	(void) make_const_string("world", 5, & value);
	if (! set_array_element(a_cookie, & index, & value)) {
		printf("fill_in_array:%d: set_array_element failed\n", __LINE__);
		return;
	}

	(void) make_const_string("answer", 6, & index);
	(void) make_number(42.0, & value);
	if (! set_array_element(a_cookie, & index, & value)) {
		printf("fill_in_array:%d: set_array_element failed\n", __LINE__);
		return;
	}

	new_array->val_type = AWK_ARRAY;
	new_array->array_cookie = a_cookie;
}

/* create_new_array --- create a named array */

static void
create_new_array()
{
	awk_value_t value;

	fill_in_array(& value);
	if (! sym_update("new_array", & value))
		printf("create_new_array: sym_update(\"new_array\") failed!\n");
}

/* at_exit0 --- first at_exit program, runs last */

static void at_exit0(void *data, int exit_status)
{
	printf("at_exit0 called (should be third):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = NULL,");
	printf(" exit_status = %d\n", exit_status);
}

/* at_exit1 --- second at_exit program, runs second */

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

/* at_exit2 --- third at_exit program, runs first */

static void at_exit2(void *data, int exit_status)
{
	printf("at_exit2 called (should be first):");
	if (data)
		printf(" data = %p,", data);
	else
		printf(" data = NULL,");
	printf(" exit_status = %d\n", exit_status);
}

/* do_test_function --- test function for test namespace */

static awk_value_t *
do_test_function(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	printf("test::test_function() called.\n");
	fflush(stdout);

	return make_number(0.0, result);
}

static awk_ext_func_t func_table[] = {
	{ "dump_array_and_delete", dump_array_and_delete, 2, 2, awk_false, NULL },
	{ "try_modify_environ", try_modify_environ, 0, 0, awk_false, NULL },
	{ "var_test", var_test, 1, 1, awk_false, NULL },
	{ "test_deferred", test_deferred, 0, 0, awk_false, NULL },
	{ "test_errno", test_errno, 0, 0, awk_false, NULL },
	{ "test_array_size", test_array_size, 1, 1, awk_false, NULL },
	{ "test_array_elem", test_array_elem, 2, 2, awk_false, NULL },
	{ "test_array_param", test_array_param, 1, 1, awk_false, NULL },
	{ "print_do_lint", print_do_lint, 0, 0, awk_false, NULL },
	{ "test_scalar", test_scalar, 1, 1, awk_false, NULL },
	{ "test_scalar_reserved", test_scalar_reserved, 0, 0, awk_false, NULL },
	{ "test_indirect_vars", test_indirect_vars, 0, 0, awk_false, NULL },
	{ "test_get_file", test_get_file, 2, 2, awk_false, NULL },
	{ "get_file", do_get_file, 4, 4, awk_false, NULL },
};

static awk_ext_func_t ns_test_func = {
	"test_function", do_test_function, 0, 0, awk_false, NULL
};

/* init_testext --- additional initialization function */

static awk_bool_t init_testext(void)
{
	awk_value_t value;
	static const char message[] = "hello, world";	/* of course */
	static const char message2[] = "i am a scalar";
	static const char message3[] = "in namespace test";

	/* This is used by the getfile test */
	if (sym_lookup("TESTEXT_QUIET", AWK_NUMBER, & value))
		return awk_true;

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
	printf("test::testval = %s\n", test::testval)
	test::test_function()
	print ""
}
*/

	/* install some variables */
	if (! sym_update("answer_num", make_number(42, & value)))
		printf("testext: sym_update(\"answer_num\") failed!\n");

	if (! sym_update("message_string",
			make_const_string(message, strlen(message), & value)))
		printf("testext: sym_update(\"answer_num\") failed!\n");

	if (! sym_update("the_scalar",
			make_const_string(message2, strlen(message2), & value)))
		printf("testext: sym_update(\"the_scalar\") failed!\n");

	create_new_array();

	if (! sym_update_ns("test", "testval",
			make_const_string(message3, strlen(message3), & value)))
		printf("testext: sym_update_ns(\"test\", \"testval\") failed!\n");

	if (! add_ext_func("test", & ns_test_func))
		printf("testext: add_ext_func(\"test\", ns_test_func) failed!\n");

	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_testext;

dl_load_func(func_table, testext, "")
