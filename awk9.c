/*
 * awk9.c -- routines for node management
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

AWKNUM
r_force_number(n)
NODE *n;
{
#ifdef DEBUG
	if (n == NULL)
		cant_happen();
	if (n->type != Node_val)
		cant_happen();
	if(n->flags == 0)
		cant_happen();
	if (n->flags & NUM)
		return n->numbr;
#endif
	n->numbr = (AWKNUM) atof(n->stptr);
	n->flags |= NUM;
	return n->numbr;
}

/*
 * the following lookup table is used as an optimization in force_string
 * variations on this theme didn't seem to pay off, but systematic
 * testing might be in order at some point
 */
static char *values[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
};
#define	NVAL	(sizeof(values)/sizeof(values[0]))

NODE *
r_force_string(s)
NODE *s;
{
	char buf[128];
	char *fmt;
	int num;
	char *sp = buf;

#ifdef DEBUG
	if (s == NULL)
		cant_happen();
	if (s->type != Node_val)
		cant_happen();
	if (s->flags & STR)
		return s;
	if (!(s->flags & NUM))
		cant_happen();
	if (s->stref != 0)
		cant_happen();
#endif
	s->flags |= STR;
	/* should check validity of user supplied OFMT */
	fmt = OFMT_node->var_value->stptr;
	if (STREQ(fmt, "%.6g") && (long) s->numbr == s->numbr) {
	/* integral value */
		if ((num = s->numbr) < NVAL) {
			sp = values[num];
			s->stlen = 1;
		} else {
		fmt = "%.11g";
			(void) sprintf(sp, fmt, s->numbr);
			s->stlen = strlen(sp);
		}
	} else {
		(void) sprintf(sp, fmt, s->numbr);
		s->stlen = strlen(sp);
	}
	s->stref = 1;
	emalloc(s->stptr, char *, s->stlen + 1, "force_string");
	bcopy(sp, s->stptr, s->stlen+1);
	return s;
}

/*
 * This allocates a new node of type ty.  Note that this node will not go
 * away unless freed.
 */
#ifdef notdef
NODE *
newnode(ty)
NODETYPE ty;
{
	register NODE *r;

	emalloc(r, NODE *, sizeof(NODE), "newnode");
	r->type = ty;
	r->flags = MALLOC;
	return r;
}

freenode(n)
NODE *n;
{
	free((char *)n);
}
#endif

/*
 * Duplicate a node.  (For global strings, "duplicate" means crank up the
 * reference count.)  This creates global nodes. . . 
 */
NODE *
dupnode(n)
NODE *n;
{
	register NODE *r;

	if (n->flags & TEMP) {
		n->flags &= ~TEMP;
		n->flags |= MALLOC;
		return n;
	}
	if ((n->flags & (MALLOC|STR)) == (MALLOC|STR)) {
		if (n->stref < 255)
			n->stref++;
		return n;
	}
	r = newnode(Node_illegal);
	*r = *n;
	r->flags &= ~(PERM|TEMP);
	r->flags |= MALLOC;
	if (n->type == Node_val && (n->flags & STR)) {
		r->stref = 1;
		emalloc(r->stptr, char *, r->stlen + 1, "dupnode");
		bcopy(n->stptr, r->stptr, r->stlen);
		r->stptr[r->stlen] = '\0';
	}
	return r;
}

/* this allocates a node with defined numbr */
/* This creates global nodes! */
NODE *
make_number(x)
AWKNUM x;
{
	register NODE *r;

	r = newnode(Node_val);
	r->numbr = x;
	r->flags |= NUM;
	r->stref = 0;
	return r;
}

/*
 * This creates temporary nodes.  They go away quite quicly, so don't use
 * them for anything important 
 */
NODE *
tmp_number(x)
AWKNUM x;
{
	NODE *r;

	r = make_number(x);
	r->flags |= TEMP;
	return r;
}

/*
 * Make a string node.  If len==-1, the string passed in S is supposed to end
 * with a double quote, but have had the beginning double quote already
 * stripped off by yylex. If LEN!=-1, we don't care what s ends with.  This
 * creates a global node 
 */

NODE *
make_string(s, len)
char *s;
{
	register NODE *r;
	register char *pf, *pt;
	register int c;
	int count;

	/*
	 * the aborts are impossible because yylex is supposed to have
	 * already checked for unterminated strings 
	 */
	if (len == -1) {	/* Called from yyparse, find our own len */
		for (pf = pt = s; *pf != '\0' && *pf != '\"';) {
			c = *pf++;
			switch (c) {
			case '\0':
				cant_happen();

			case '\\':
				if (*pf == '\0')
					cant_happen();
				c = *pf++;
				switch (c) {
				case '\\':	/* no massagary needed */
				case '\'':
				case '\"':
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
#ifdef notdef
				case '8':
				case '9':
#endif
					c -= '0';
					count = 1;
					while (*pf && *pf >= '0' && *pf <= '7') {
						c = c * 8 + *pf++ - '0';
						if (++count >= 3)
							break;
					}
					break;
				case 'a':
					if (strict)
						goto def;
					else
						c = BELL;
					break;
				case 'b':
					c = '\b';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case 'v':
					if (strict)
						goto def;
					else
						c = '\v';
					break;
				case 'x':
					if (strict)
						goto def;
					else {
						c = 0;
						while (*pf && isxdigit(*pf)) {
							if (isdigit(*pf))
								c += *pf - '0';
							else if (isupper(*pf))
								c += *pf - 'A' + 10;
							else
								c += *pf - 'a' + 10;
							pf++;
						}
					}
					break;
				default:
				def:
					*pt++ = '\\';
					break;
				}
				/* FALL THROUGH */
			default:
				*pt++ = c;
				break;
			}
		}
		if (*pf == '\0')
			cant_happen();	/* hit the end of the buf */
		len = pt - s;
	}
	r = newnode(Node_val);
	emalloc(r->stptr, char *, len + 1, s);
	r->stlen = len;
	r->stref = 1;
	bcopy(s, r->stptr, len);
	r->stptr[len] = '\0';	/* a hack */
	r->flags = (STR|MALLOC);

	return r;
}

/* This should be a macro for speed, but the C compiler chokes. */
/* Read the warning under tmp_number */
NODE *
tmp_string(s, len)
char *s;
int len;
{
	register NODE *r;

	r = make_string(s, len);
	r->flags |= TEMP;
	return r;
}
