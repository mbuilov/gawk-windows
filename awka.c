/*
 * some speciallized memory allocation routines
 *
 * $Log:	awka.c,v $
 * Revision 1.1  89/03/22  21:04:00  david
 * Initial revision
 * 
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
