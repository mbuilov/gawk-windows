/*
 * stack.c -- Implementation for stack functions for use by extensions.
 */

/*
 * Copyright (C) 2012, 2013 the Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <wchar.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "stack.h"
#include "gawkapi.h"

#define INITIAL_STACK	20
#define MAX_DBL_SIZE	2000

static size_t size;
static void **stack;
static size_t filled;

/* stack_empty --- return true if stack is empty */

int
stack_empty(void)
{
	return filled == 0;
}

/* stack_top --- return top object on the stack */

void *
stack_top(void)
{
	if (stack_empty())
		return NULL;

	return stack[filled - 1];
}

/* stack_pop --- pop top object and return it */

void *
stack_pop(void)
{
	if (stack_empty())
		return NULL;

	return stack[--filled];
}

/* stack_push --- push an object onto the stack */

int stack_push(void *object)
{
	size_t new_size;
	void **new_stack;

	if (stack == NULL) {
		stack = (void **) malloc(INITIAL_STACK * sizeof(void *));
		if (stack == NULL)
			return 0;
		size = INITIAL_STACK;
	} else if (filled == size) {
		if (size < MAX_DBL_SIZE)
			new_size = size * 2;
		else if (size > (size_t)-1/sizeof(void *) - MAX_DBL_SIZE)
			return 0;
		else
			new_size = size + MAX_DBL_SIZE;
		new_stack = (void**) realloc(stack, new_size * sizeof(void *));
		if (new_stack == NULL)
			return 0;
		size = new_size;
		stack = new_stack;
	}

	stack[filled++] = object;
	return 1;
}

/* stack_clear --- deallocate stack */

void stack_clear(void)
{
	if (stack != NULL) {
		free(stack);
		stack = NULL;
		filled = 0;
		size = 0;
	}
}
