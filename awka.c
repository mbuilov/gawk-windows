/*
 * awka.c --- some speciallized memory allocation routines
 *
 * $Log:	awka.c,v $
 * Revision 1.2  89/03/31  13:26:15  david
 * GNU license
 * 
 * Revision 1.1  89/03/22  21:04:00  david
 * Initial revision
 * 
 */

/* 
 * Copyright (C) 1986, 1988, 1989 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

#define NODECHUNK	50

NODE *nextfree = NULL;
NODE *lastfree = NULL;

NODE *
newnode(ty)
NODETYPE ty;
{
	NODE *it;
	NODE *np;

	if (nextfree == lastfree) {
		emalloc(nextfree, NODE *, NODECHUNK * sizeof(NODE), "newnode");
		for (np = nextfree; np < &nextfree[NODECHUNK - 1]; np++)
			np->nextp = np + 1;
		np->nextp = lastfree;
		lastfree = np;
	}
	it = nextfree;
	nextfree = nextfree->nextp;
	it->type = ty;
	it->flags = MALLOC;
	return it;
}

freenode(it)
NODE *it;
{
	lastfree->nextp = it;
	it->nextp = NULL;
	lastfree = it;
}
