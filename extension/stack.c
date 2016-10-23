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

#include <stdlib.h>

#include "stack.h"

#define INITIAL_STACK	20

static size_t size;
static void **stack;
static int index = -1;

/* stack_empty --- return true if stack is empty */

int
stack_empty()
{
	return index < 0;
}

/* stack_top --- return top object on the stack */

void *
stack_top()
{
	if (stack_empty() || stack == NULL)
		return NULL;

	return stack[index];
}

/* stack_pop --- pop top object and return it */

void *
stack_pop()
{
	if (stack_empty() || stack == NULL)
		return NULL;

	return stack[index--];
}

/* stack_push --- push an object onto the stack */

int stack_push(void *object)
{
	void **new_stack;
	size_t new_size = 2 * size;

	if (stack == NULL) {
		stack = (void **) malloc(INITIAL_STACK * sizeof(void *));
		if (stack == NULL)
			return 0;
		size = INITIAL_STACK;
	} else if (index + 1 >= size) {
		if (new_size < size)
			return 0;
		new_stack = realloc(stack, new_size * sizeof(void *));
		if (new_stack == NULL)
			return 0;
		size = new_size;
		stack = new_stack;
	}

	stack[++index] = object;
	return 1;
}
