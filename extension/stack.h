/*
 * stack.h -- Definitions for stack functions for use by extensions.
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

extern int stack_empty();	/* return true if stack is empty */
extern void *stack_top();	/* return top object on the stack */
extern void *stack_pop();	/* pop top object and return it */
extern int stack_push(void *);	/* push an object onto the stack,
				 * return 0 if failed, 1 if success 
				 */
