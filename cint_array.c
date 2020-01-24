/*
 * cint_array.c - routines for arrays of (mostly) consecutive positive integer indices.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2013, 2016, 2017, 2019, 2020,
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

#include "awk.h"

#define INT32_BIT 32

extern FILE *output_fp;
extern void indent(int indent_level);
extern NODE **is_integer(NODE *symbol, NODE *subs);

/*
 * NHAT         ---  maximum size of a leaf array (2^NHAT).
 * THRESHOLD    ---  Maximum capacity waste; THRESHOLD >= 2^(NHAT + 1).
 */

static int NHAT = 10;
static long THRESHOLD;

/*
 * What is the optimium NHAT ? timing results suggest that 10 is a good choice,
 * although differences aren't that significant for > 10.
 */


static NODE **cint_array_init(NODE *symbol, NODE *subs);
static NODE **is_uinteger(NODE *symbol, NODE *subs);
static NODE **cint_lookup(NODE *symbol, NODE *subs);
static NODE **cint_exists(NODE *symbol, NODE *subs);
static NODE **cint_clear(NODE *symbol, NODE *subs);
static NODE **cint_remove(NODE *symbol, NODE *subs);
static NODE **cint_list(NODE *symbol, NODE *t);
static NODE **cint_copy(NODE *symbol, NODE *newsymb);
static NODE **cint_dump(NODE *symbol, NODE *ndump);
#ifdef ARRAYDEBUG
static void cint_print(NODE *symbol);
#endif

const array_funcs_t cint_array_func = {
	"cint",
	cint_array_init,
	is_uinteger,
	cint_lookup,
	cint_exists,
	cint_clear,
	cint_remove,
	cint_list,
	cint_copy,
	cint_dump,
	(afunc_t) 0,
};


static NODE **argv_store(NODE *symbol, NODE *subs);

/* special case for ARGV in sandbox mode */
const array_funcs_t argv_array_func = {
	"argv",
	cint_array_init,
	is_uinteger,
	cint_lookup,
	cint_exists,
	cint_clear,
	cint_remove,
	cint_list,
	cint_copy,
	cint_dump,
	argv_store,
};

static inline int cint_hash(long k);
static inline NODE **cint_find(NODE *symbol, long k, int h1);

static inline NODE *make_node(NODETYPE type);

static NODE **tree_lookup(NODE *symbol, NODE *tree, long k, int m, long base);
static NODE **tree_exists(NODE *tree, long k);
static void tree_clear(NODE *tree);
static int tree_remove(NODE *symbol, NODE *tree, long k);
static void tree_copy(NODE *newsymb, NODE *tree, NODE *newtree);
static long tree_list(NODE *tree, NODE **list, assoc_kind_t assoc_kind);
static inline NODE **tree_find(NODE *tree, long k, int i);
static void tree_info(NODE *tree, NODE *ndump, const char *aname);
static size_t tree_kilobytes(NODE *tree);
#ifdef ARRAYDEBUG
static void tree_print(NODE *tree, size_t bi, int indent_level);
#endif

static inline NODE **leaf_lookup(NODE *symbol, NODE *array, long k, long size, long base);
static inline NODE **leaf_exists(NODE *array, long k);
static void leaf_clear(NODE *array);
static int leaf_remove(NODE *symbol, NODE *array, long k);
static void leaf_copy(NODE *newsymb, NODE *array, NODE *newarray);
static long leaf_list(NODE *array, NODE **list, assoc_kind_t assoc_kind);
static void leaf_info(NODE *array, NODE *ndump, const char *aname);
#ifdef ARRAYDEBUG
static void leaf_print(NODE *array, size_t bi, int indent_level);
#endif

/* powers of 2 table upto 2^30 */
static const long power_two_table[] = {
	1, 2, 4, 8, 16, 32, 64,
	128, 256, 512, 1024, 2048, 4096,
	8192, 16384, 32768, 65536, 131072, 262144,
	524288, 1048576, 2097152, 4194304, 8388608, 16777216,
	33554432, 67108864, 134217728, 268435456, 536870912, 1073741824
};


#define ISUINT(a, s)	((((s)->flags & NUMINT) != 0 || is_integer(a, s) != NULL) \
                                    && (s)->numbr >= 0)

/*
 * To store 2^n integers, allocate top-level array of size n, elements
 * of which are 1-Dimensional (leaf-array) of geometrically increasing
 * size (power of 2).
 *
 *  [0]   -->  [ 0 ]
 *  [1]   -->  [ 1 ]
 *  |2|   -->  [ 2 | 3 ]
 *  |3|   -->  [ 4 | 5 | 6 | 7 ]
 *  |.|
 *  |k|   -->  [ 2^(k - 1)| ...  | 2^k - 1 ]
 *  ...
 *
 * For a given integer n (> 0), the leaf-array is at 1 + floor(log2(n)).
 *
 * The idea for the geometrically increasing array sizes is from:
 * 	Fast Functional Lists, Hash-Lists, Deques and Variable Length Arrays.
 * 	Bagwell, Phil (2002).
 * 	http://infoscience.epfl.ch/record/64410/files/techlists.pdf
 *
 * Disadvantage:
 * Worst case memory waste > 99% and will happen when each of the
 * leaf arrays contains only a single element. Even with consecutive
 * integers, memory waste can be as high as 50%.
 *
 * Solution: Hashed Array Trees (HATs).
 *
 */

/* cint_array_init ---  array initialization routine */

static NODE **
cint_array_init(NODE *symbol ATTRIBUTE_UNUSED, NODE *subs ATTRIBUTE_UNUSED)
{
	if (symbol == NULL) {
		long newval;
		size_t nelems = (sizeof(power_two_table) / sizeof(power_two_table[0]));

		/* check relevant environment variables */
		if ((newval = getenv_long("NHAT")) > 1 && newval < INT32_BIT)
			NHAT = newval;
		/* don't allow overflow off the end of the table */
		if (NHAT > nelems - 2)
			NHAT = nelems - 2;
		THRESHOLD = power_two_table[NHAT + 1];
	} else
		null_array(symbol);

	return & success_node;
}


/* is_uinteger --- test if the subscript is an integer >= 0 */

NODE **
is_uinteger(NODE *symbol, NODE *subs)
{
	if (is_integer(symbol, subs) != NULL && subs->numbr >= 0)
		return & success_node;
	return NULL;
}


/* cint_lookup --- Find the subscript in the array; Install it if it isn't there. */

static NODE **
cint_lookup(NODE *symbol, NODE *subs)
{
	NODE **lhs;
	long k;
	int h1 = -1, m, li;
	NODE *tn, *xn;
	long cint_size, capacity;

	k = -1;
	if (ISUINT(symbol, subs)) {
		k = subs->numbr;	/* k >= 0 */
		h1 = cint_hash(k);	/* h1 >= NHAT */
		if ((lhs = cint_find(symbol, k, h1)) != NULL)
			return lhs;
	}
	xn = symbol->xarray;
	if (xn != NULL && (lhs = xn->aexists(xn, subs)) != NULL)
		return lhs;

	/* It's not there, install it */

	if (k < 0)
		goto xinstall;

	m = h1 - 1;	/* m >= (NHAT- 1) */

	/* Estimate capacity upper bound.
	 * capacity upper bound = current capacity + leaf array size.
	 */
	li = m > NHAT ? m : NHAT;
	while (li >= NHAT) {
		/* leaf-array of a HAT */
		li = (li + 1) / 2;
	}
	capacity = symbol->array_capacity + power_two_table[li];

	cint_size = (xn == NULL) ? symbol->table_size
				: (symbol->table_size - xn->table_size);
	assert(cint_size >= 0);
	if ((capacity - cint_size) > THRESHOLD)
		goto xinstall;

	if (symbol->nodes == NULL) {
		symbol->array_capacity = 0;
		assert(symbol->table_size == 0);

		/* nodes[0] .. nodes[NHAT- 1] not used */
		ezalloc(symbol->nodes, NODE **, INT32_BIT * sizeof(NODE *), "cint_lookup");
	}

	symbol->table_size++;	/* one more element in array */

	tn = symbol->nodes[h1];
	if (tn == NULL) {
		tn = make_node(Node_array_tree);
		symbol->nodes[h1] = tn;
	}

	if (m < NHAT)
		return tree_lookup(symbol, tn, k, NHAT, 0);
	return tree_lookup(symbol, tn, k, m, power_two_table[m]);

xinstall:

	symbol->table_size++;
	if (xn == NULL) {
		xn = symbol->xarray = make_array();
		xn->vname = symbol->vname;	/* shallow copy */

		/*
		 * Avoid using assoc_lookup(xn, subs) which may lead
		 * to infinite recursion.
		 */

		if (is_integer(xn, subs))
			xn->array_funcs = & int_array_func;
		else
			xn->array_funcs = & str_array_func;
		xn->flags |= XARRAY;
	}
	return xn->alookup(xn, subs);
}


/* cint_exists --- test whether an index is in the array or not. */

static NODE **
cint_exists(NODE *symbol, NODE *subs)
{
	NODE *xn;

	if (ISUINT(symbol, subs)) {
		long k = subs->numbr;
		NODE **lhs;
		if ((lhs = cint_find(symbol, k, cint_hash(k))) != NULL)
			return lhs;
	}
	if ((xn = symbol->xarray) == NULL)
		return NULL;
	return xn->aexists(xn, subs);
}


/* cint_clear --- flush all the values in symbol[] */

static NODE **
cint_clear(NODE *symbol, NODE *subs ATTRIBUTE_UNUSED)
{
	size_t i;
	NODE *tn;

	assert(symbol->nodes != NULL);

	if (symbol->xarray != NULL) {
		NODE *xn = symbol->xarray;
		assoc_clear(xn);
		freenode(xn);
		symbol->xarray = NULL;
	}

	for (i = NHAT; i < INT32_BIT; i++) {
		tn = symbol->nodes[i];
		if (tn != NULL) {
			tree_clear(tn);
			freenode(tn);
		}
	}

	efree(symbol->nodes);
	symbol->ainit(symbol, NULL);	/* re-initialize symbol */
	return NULL;
}


/* cint_remove --- remove an index from the array */

static NODE **
cint_remove(NODE *symbol, NODE *subs)
{
	long k;
	int h1;
	NODE *tn, *xn = symbol->xarray;

	if (symbol->table_size == 0)
		return NULL;

	if (! ISUINT(symbol, subs))
		goto xremove;

	assert(symbol->nodes != NULL);

	k = subs->numbr;
	h1 = cint_hash(k);
	tn = symbol->nodes[h1];
	if (tn == NULL || ! tree_remove(symbol, tn, k))
		goto xremove;

	if (tn->table_size == 0) {
		freenode(tn);
		symbol->nodes[h1] = NULL;
	}

	symbol->table_size--;

	if (xn == NULL && symbol->table_size == 0) {
		efree(symbol->nodes);
		symbol->ainit(symbol, NULL);	/* re-initialize array 'symbol' */
	} else if(xn != NULL && symbol->table_size == xn->table_size) {
		/* promote xn to symbol */

		xn->flags &= ~XARRAY;
		xn->parent_array = symbol->parent_array;
		efree(symbol->nodes);
		*symbol = *xn;
		freenode(xn);
	}

	return & success_node;

xremove:
	xn = symbol->xarray;
	if (xn == NULL || xn->aremove(xn, subs) == NULL)
		return NULL;
	if (xn->table_size == 0) {
		freenode(xn);
		symbol->xarray = NULL;
	}
	symbol->table_size--;
	assert(symbol->table_size > 0);

	return & success_node;
}


/* cint_copy --- duplicate input array "symbol" */

static NODE **
cint_copy(NODE *symbol, NODE *newsymb)
{
	NODE **old, **new;
	size_t i;

	assert(symbol->nodes != NULL);

	/* allocate new table */
	ezalloc(new, NODE **, INT32_BIT * sizeof(NODE *), "cint_copy");

	old = symbol->nodes;
	for (i = NHAT; i < INT32_BIT; i++) {
		if (old[i] == NULL)
			continue;
		new[i] = make_node(Node_array_tree);
		tree_copy(newsymb, old[i], new[i]);
	}

	if (symbol->xarray != NULL) {
		NODE *xn, *n;
		xn = symbol->xarray;
		n = make_array();
		n->vname = newsymb->vname;
		(void) xn->acopy(xn, n);
		newsymb->xarray = n;
	} else
		newsymb->xarray = NULL;

	newsymb->nodes = new;
	newsymb->table_size = symbol->table_size;
	newsymb->array_capacity = symbol->array_capacity;
	newsymb->flags = symbol->flags;

	return NULL;
}


/* cint_list --- return a list of items */

static NODE**
cint_list(NODE *symbol, NODE *t)
{
	NODE **list = NULL;
	NODE *tn, *xn;
	unsigned long k = 0, num_elems, list_size;
	size_t j, ja, jd;
	int elem_size = 1;
	assoc_kind_t assoc_kind;

	num_elems = symbol->table_size;
	if (num_elems == 0)
		return NULL;
	assoc_kind = (assoc_kind_t) t->flags;
	if ((assoc_kind & (AINDEX|AVALUE|ADELETE)) == (AINDEX|ADELETE))
		num_elems = 1;

	if ((assoc_kind & (AINDEX|AVALUE)) == (AINDEX|AVALUE))
		elem_size = 2;
	list_size = num_elems * elem_size;

	if (symbol->xarray != NULL) {
		xn = symbol->xarray;
		list = xn->alist(xn, t);
		assert(list != NULL);
		assoc_kind &= ~(AASC|ADESC);
		t->flags = (unsigned int) assoc_kind;
		if (num_elems == 1 || num_elems == xn->table_size)
			return list;
		erealloc(list, NODE **, list_size * sizeof(NODE *), "cint_list");
		k = elem_size * xn->table_size;
	} else
		emalloc(list, NODE **, list_size * sizeof(NODE *), "cint_list");

	if ((assoc_kind & AINUM) == 0) {
		/* not sorting by "index num" */
		assoc_kind &= ~(AASC|ADESC);
		t->flags = (unsigned int) assoc_kind;
	}

	/* populate it with index in ascending or descending order */

	for (ja = NHAT, jd = INT32_BIT - 1; ja < INT32_BIT && jd >= NHAT; ) {
		j = (assoc_kind & ADESC) != 0 ? jd-- : ja++;
		tn = symbol->nodes[j];
		if (tn == NULL)
			continue;
		k += tree_list(tn, list + k, assoc_kind);
		if (k >= list_size)
			return list;
	}
	return list;
}


/* cint_dump --- dump array info */

static NODE **
cint_dump(NODE *symbol, NODE *ndump)
{
	NODE *tn, *xn = NULL;
	int indent_level;
	size_t i;
	long cint_size = 0, xsize = 0;
	AWKNUM kb = 0;
	extern AWKNUM int_kilobytes(NODE *symbol);
	extern AWKNUM str_kilobytes(NODE *symbol);

	indent_level = ndump->alevel;

	if (symbol->xarray != NULL) {
		xn = symbol->xarray;
		xsize = xn->table_size;
	}
	cint_size = symbol->table_size - xsize;

	if ((symbol->flags & XARRAY) == 0)
		fprintf(output_fp, "%s `%s'\n",
			(symbol->parent_array == NULL) ? "array" : "sub-array",
			array_vname(symbol));
	indent_level++;
	indent(indent_level);
	fprintf(output_fp, "array_func: cint_array_func\n");
	if (symbol->flags != 0) {
		indent(indent_level);
		fprintf(output_fp, "flags: %s\n", flags2str(symbol->flags));
	}
	indent(indent_level);
	fprintf(output_fp, "NHAT: %d\n", NHAT);
	indent(indent_level);
	fprintf(output_fp, "THRESHOLD: %ld\n", THRESHOLD);
	indent(indent_level);
	fprintf(output_fp, "table_size: %ld (total), %ld (cint), %ld (int + str)\n",
				symbol->table_size, cint_size, xsize);
	indent(indent_level);
	fprintf(output_fp, "array_capacity: %lu\n", (unsigned long) symbol->array_capacity);
	indent(indent_level);
	fprintf(output_fp, "Load Factor: %.2g\n", (AWKNUM) cint_size / symbol->array_capacity);

	for (i = NHAT; i < INT32_BIT; i++) {
		tn = symbol->nodes[i];
		if (tn == NULL)
			continue;
		/* Node_array_tree  + HAT */
		kb += (sizeof(NODE) + tree_kilobytes(tn)) / 1024.0;
	}
	kb += (INT32_BIT * sizeof(NODE *)) / 1024.0;	/* symbol->nodes */
	kb += (symbol->array_capacity * sizeof(NODE *)) / 1024.0;	/* value nodes in Node_array_leaf(s) */
	if (xn != NULL) {
		if (xn->array_funcs == & int_array_func)
			kb += int_kilobytes(xn);
		else
			kb += str_kilobytes(xn);
	}

	indent(indent_level);
	fprintf(output_fp, "memory: %.2g kB (total)\n", kb);

	/* dump elements */

	if (ndump->adepth >= 0) {
		const char *aname;

		fprintf(output_fp, "\n");
		aname = make_aname(symbol);
		for (i = NHAT; i < INT32_BIT; i++) {
			tn = symbol->nodes[i];
			if (tn != NULL)
				tree_info(tn, ndump, aname);
		}
	}

	if (xn != NULL) {
		fprintf(output_fp, "\n");
		xn->adump(xn, ndump);
	}

#ifdef ARRAYDEBUG
	if (ndump->adepth < -999)
		cint_print(symbol);
#endif

	return NULL;
}


/* cint_hash --- locate the HAT for a given number 'k' */

static inline int
cint_hash(long k)
{
	uint32_t num, r, shift;

	assert(k >= 0);
	if (k == 0)
		return NHAT;
	num = k;

	/* Find the Floor(log base 2 of 32-bit integer) */

	/*
	 * Warren Jr., Henry S. (2002). Hacker's Delight.
	 * Addison Wesley. pp. pp. 215. ISBN 978-0201914658.
	 *
	 *	r = 0;
	 *	if (num >= 1<<16) { num >>= 16;	r += 16; }
	 *	if (num >= 1<< 8) { num >>=  8;	r +=  8; }
	 *	if (num >= 1<< 4) { num >>=  4;	r +=  4; }
	 *	if (num >= 1<< 2) { num >>=  2;	r +=  2; }
	 *	if (num >= 1<< 1) {		r +=  1; }
	 */


	/*
	 * Slightly different code copied from:
	 *
	 * http://www-graphics.stanford.edu/~seander/bithacks.html
	 * Bit Twiddling Hacks
	 * By Sean Eron Anderson
	 * seander@cs.stanford.edu
	 * Individually, the code snippets here are in the public domain
	 * (unless otherwise noted) --- feel free to use them however you please.
	 * The aggregate collection and descriptions are (C) 1997-2005
	 * Sean Eron Anderson. The code and descriptions are distributed in the
	 * hope that they will be useful, but WITHOUT ANY WARRANTY and without
	 * even the implied warranty of merchantability or fitness for a particular
	 * purpose.
	 *
	 */

	r = (num > 0xFFFF) << 4; num >>= r;
	shift = (num > 0xFF) << 3; num >>= shift; r |= shift;
	shift = (num > 0x0F) << 2; num >>= shift; r |= shift;
	shift = (num > 0x03) << 1; num >>= shift; r |= shift;
	r |= (num >> 1);

	/* We use a single HAT for 0 <= num < 2^NHAT */
	if (r < NHAT)
		return NHAT;

	return (1 + r);
}


/* cint_find --- locate the integer subscript */

static inline NODE **
cint_find(NODE *symbol, long k, int h1)
{
	NODE *tn;

	if (symbol->nodes == NULL || (tn = symbol->nodes[h1]) == NULL)
		return NULL;
	return tree_exists(tn, k);
}


#ifdef ARRAYDEBUG

/* cint_print --- print structural info */

static void
cint_print(NODE *symbol)
{
	NODE *tn;
	size_t i;

	fprintf(output_fp, "I[%4lu:%-4lu]\n", (unsigned long) INT32_BIT,
				(unsigned long) symbol->table_size);
	for (i = NHAT; i < INT32_BIT; i++) {
		tn = symbol->nodes[i];
		if (tn == NULL)
			continue;
		tree_print(tn, i, 1);
	}
}

#endif


/*------------------------ Hashed Array Trees -----------------------------*/

/*
 * HATs: Hashed Array Trees
 * Fast variable-length arrays
 * Edward Sitarski
 * http://www.drdobbs.com/architecture-and-design/184409965
 *
 *  HAT has a top-level array containing a power of two
 *  number of leaf arrays. All leaf arrays are the same size as the
 *  top-level array. A full HAT can hold n^2 elements,
 *  where n (some power of 2) is the size of each leaf array.
 *  [i/n][i & (n - 1)] locates the `i th' element in a HAT.
 *
 */

/*
 *  A half HAT is defined here as a HAT with a top-level array of size n^2/2
 *  and holds the first n^2/2 elements.
 *
 *   1. 2^8 elements can be stored in a full HAT of size 2^4.
 *   2. 2^9 elements can be stored in a half HAT of size 2^5.
 *   3. When the number of elements is some power of 2, it
 *      can be stored in a full or a half HAT.
 *   4. When the number of elements is some power of 2, it
 *      can be stored in a HAT (full or half) with HATs as leaf elements
 *      (full or half),  and so on (e.g. 2^8 elements in a HAT of size 2^4 (top-level
 *      array dimension) with each leaf array being a HAT of size 2^2).
 *
 *  IMPLEMENTATION DETAILS:
 *    1. A HAT of 2^12 elements needs 2^6 house-keeping NODEs
 *       of Node_array_leaf.
 *
 *    2. A HAT of HATS of 2^12 elements needs
 *       2^6 * (1 Node_array_tree + 2^3 Node_array_leaf)
 *       ~ 2^9 house-keeping NODEs.
 *
 *    3. When a leaf array (or leaf HAT) becomes empty, the memory
 *       is deallocated, and when there is no leaf array (or leaf HAT) left,
 *       the HAT is deleted.
 *
 *    4. A HAT stores the base (first) element, and locates the leaf array/HAT
 *       for the `i th' element using integer division
 *       (i - base)/n where n is the size of the top-level array.
 *
 */

/* make_node --- initialize a NODE */

static inline NODE *
make_node(NODETYPE type)
{
	NODE *n;
	getnode(n);
	memset(n, '\0', sizeof(NODE));
	n->type = type;
	return n;
}


/* tree_lookup --- Find an integer subscript in a HAT; Install it if it isn't there */

static NODE **
tree_lookup(NODE *symbol, NODE *tree, long k, int m, long base)
{
	NODE **lhs;
	NODE *tn;
	int i, n;
	size_t size;
	long num = k;

	/*
	 * HAT size (size of Top & Leaf array) = 2^n
	 * where n = Floor ((m + 1)/2). For an odd value of m,
	 * only the first half of the HAT is needed.
	 */

	n = (m + 1) / 2;

	if (tree->table_size == 0) {
		size_t actual_size;
		NODE **table;

		assert(tree->nodes == NULL);

		/* initialize top-level array */
		size = actual_size = power_two_table[n];
		tree->array_base = base;
		tree->array_size = size;
		tree->table_size = 0;	/* # of elements in the array */
		if (n > m/2) {
			/* only first half of the array used */
			actual_size /= 2;
			tree->flags |= HALFHAT;
		}
		ezalloc(table, NODE **, actual_size * sizeof(NODE *), "tree_lookup");
		tree->nodes = table;
	} else
		size = tree->array_size;

	num -= tree->array_base;
	i = num / size;	/* top-level array index */
	assert(i >= 0);

	if ((lhs = tree_find(tree, k, i)) != NULL)
		return lhs;

	/* It's not there, install it */

	tree->table_size++;
	base += (size * i);
	tn = tree->nodes[i];
	if (n > NHAT) {
		if (tn == NULL)
			tn = tree->nodes[i] = make_node(Node_array_tree);
		return tree_lookup(symbol, tn, k, n, base);
	} else {
		if (tn == NULL)
			tn = tree->nodes[i] = make_node(Node_array_leaf);
		return leaf_lookup(symbol, tn, k, size, base);
	}
}


/* tree_exists --- test whether integer subscript `k' exists or not */

static NODE **
tree_exists(NODE *tree, long k)
{
	int i;
	NODE *tn;

	i = (k - tree->array_base) / tree->array_size;
	assert(i >= 0);
	tn = tree->nodes[i];
	if (tn == NULL)
		return NULL;
	if (tn->type == Node_array_tree)
		return tree_exists(tn, k);
	return leaf_exists(tn, k);
}

/* tree_clear --- flush all the values */

static void
tree_clear(NODE *tree)
{
	NODE *tn;
	size_t	j, hsize;

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;

	for (j = 0; j < hsize; j++) {
		tn = tree->nodes[j];
		if (tn == NULL)
			continue;
		if (tn->type == Node_array_tree)
			tree_clear(tn);
		else
			leaf_clear(tn);
		freenode(tn);
	}

	efree(tree->nodes);
	memset(tree, '\0', sizeof(NODE));
	tree->type = Node_array_tree;
}


/* tree_remove --- If the integer subscript is in the HAT, remove it */

static int
tree_remove(NODE *symbol, NODE *tree, long k)
{
	int i;
	NODE *tn;

	i = (k - tree->array_base) / tree->array_size;
	assert(i >= 0);
	tn = tree->nodes[i];
	if (tn == NULL)
		return false;

	if (tn->type == Node_array_tree
			&& ! tree_remove(symbol, tn, k))
		return false;
	else if (tn->type == Node_array_leaf
			&& ! leaf_remove(symbol, tn, k))
		return false;

	if (tn->table_size == 0) {
		freenode(tn);
		tree->nodes[i] = NULL;
	}

	/* one less item in array */
	if (--tree->table_size == 0) {
		efree(tree->nodes);
		memset(tree, '\0', sizeof(NODE));
		tree->type = Node_array_tree;
	}
	return true;
}


/* tree_find --- locate an interger subscript in the HAT */

static inline NODE **
tree_find(NODE *tree, long k, int i)
{
	NODE *tn;

	assert(tree->nodes != NULL);
	tn = tree->nodes[i];
	if (tn != NULL) {
		if (tn->type == Node_array_tree)
			return tree_exists(tn, k);
		return leaf_exists(tn, k);
	}
	return NULL;
}


/* tree_list --- return a list of items in the HAT */

static long
tree_list(NODE *tree, NODE **list, assoc_kind_t assoc_kind)
{
	NODE *tn;
	size_t j, cj, hsize;
	long k = 0;

	assert(list != NULL);

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;

	for (j = 0; j < hsize; j++) {
		cj = (assoc_kind & ADESC) != 0 ? (hsize - 1 - j) : j;
		tn = tree->nodes[cj];
		if (tn == NULL)
			continue;
		if (tn->type == Node_array_tree)
			k += tree_list(tn, list + k, assoc_kind);
		else
			k += leaf_list(tn, list + k, assoc_kind);
		if ((assoc_kind & ADELETE) != 0 && k >= 1)
			return k;
	}
	return k;
}


/* tree_copy --- duplicate a HAT */

static void
tree_copy(NODE *newsymb, NODE *tree, NODE *newtree)
{
	NODE **old, **new;
	size_t j, hsize;

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;

	ezalloc(new, NODE **, hsize * sizeof(NODE *), "tree_copy");
	newtree->nodes = new;
	newtree->array_base = tree->array_base;
	newtree->array_size = tree->array_size;
	newtree->table_size = tree->table_size;
	newtree->flags = tree->flags;

	old = tree->nodes;
	for (j = 0; j < hsize; j++) {
		if (old[j] == NULL)
			continue;
		if (old[j]->type == Node_array_tree) {
			new[j] = make_node(Node_array_tree);
			tree_copy(newsymb, old[j], new[j]);
		} else {
			new[j] = make_node(Node_array_leaf);
			leaf_copy(newsymb, old[j], new[j]);
		}
	}
}


/* tree_info --- print index, value info */

static void
tree_info(NODE *tree, NODE *ndump, const char *aname)
{
	NODE *tn;
	size_t j, hsize;

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;

	for (j = 0; j < hsize; j++) {
		tn = tree->nodes[j];
		if (tn == NULL)
			continue;
		if (tn->type == Node_array_tree)
			tree_info(tn, ndump, aname);
		else
			leaf_info(tn, ndump, aname);
	}
}


/* tree_kilobytes --- calculate memory consumption of a HAT */

static size_t
tree_kilobytes(NODE *tree)
{
	NODE *tn;
	size_t j, hsize;
	size_t sz = 0;

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;
	for (j = 0; j < hsize; j++) {
		tn = tree->nodes[j];
		if (tn == NULL)
			continue;
		sz += sizeof(NODE);	/* Node_array_tree or Node_array_leaf */
		if (tn->type == Node_array_tree)
			sz += tree_kilobytes(tn);
	}
	sz += hsize * sizeof(NODE *);	/* tree->nodes */
	return sz;
}

#ifdef ARRAYDEBUG

/* tree_print --- print the HAT structures */

static void
tree_print(NODE *tree, size_t bi, int indent_level)
{
	NODE *tn;
	size_t j, hsize;

	indent(indent_level);

	hsize = tree->array_size;
	if ((tree->flags & HALFHAT) != 0)
		hsize /= 2;
	fprintf(output_fp, "%4lu:%s[%4lu:%-4lu]\n",
			(unsigned long) bi,
			(tree->flags & HALFHAT) != 0 ? "HH" : "H",
			(unsigned long) hsize, (unsigned long) tree->table_size);

	for (j = 0; j < hsize; j++) {
		tn = tree->nodes[j];
		if (tn == NULL)
			continue;
		if (tn->type == Node_array_tree)
			tree_print(tn, j, indent_level + 1);
		else
			leaf_print(tn, j, indent_level + 1);
	}
}
#endif

/*--------------------- leaf (linear 1-D) array --------------------*/

/*
 * leaf_lookup --- find an integer subscript in the array; Install it if
 *	it isn't there.
 */

static inline NODE **
leaf_lookup(NODE *symbol, NODE *array, long k, long size, long base)
{
	NODE **lhs;

	if (array->nodes == NULL) {
		array->table_size = 0;	/* sanity */
		array->array_size = size;
		array->array_base = base;
		ezalloc(array->nodes, NODE **, size * sizeof(NODE *), "leaf_lookup");
		symbol->array_capacity += size;
	}

	lhs = array->nodes + (k - base); /* leaf element */
	if (*lhs == NULL) {
		array->table_size++;	/* one more element in leaf array */
		*lhs = dupnode(Nnull_string);
	}
	return lhs;
}


/* leaf_exists --- check if the array contains an integer subscript */

static inline NODE **
leaf_exists(NODE *array, long k)
{
	NODE **lhs;
	lhs = array->nodes + (k - array->array_base);
	return (*lhs != NULL) ? lhs : NULL;
}


/* leaf_clear --- flush all values in the array */

static void
leaf_clear(NODE *array)
{
	long i, size = array->array_size;
	NODE *r;

	for (i = 0; i < size; i++) {
		r = array->nodes[i];
		if (r == NULL)
			continue;
		if (r->type == Node_var_array) {
			assoc_clear(r);		/* recursively clear all sub-arrays */
			efree(r->vname);
			freenode(r);
		} else
			unref(r);
	}
	efree(array->nodes);
	array->nodes = NULL;
	array->array_size = array->table_size = 0;
}


/* leaf_remove --- remove an integer subscript from the array */

static int
leaf_remove(NODE *symbol, NODE *array, long k)
{
	NODE **lhs;

	lhs = array->nodes + (k - array->array_base);
	if (*lhs == NULL)
		return false;
	*lhs = NULL;
	if (--array->table_size == 0) {
		efree(array->nodes);
		array->nodes = NULL;
		symbol->array_capacity -= array->array_size;
		array->array_size = 0;	/* sanity */
	}
	return true;
}


/* leaf_copy --- duplicate a leaf array */

static void
leaf_copy(NODE *newsymb, NODE *array, NODE *newarray)
{
	NODE **old, **new;
	long size, i;

	size = array->array_size;
	ezalloc(new, NODE **, size * sizeof(NODE *), "leaf_copy");
	newarray->nodes = new;
	newarray->array_size = size;
	newarray->array_base = array->array_base;
	newarray->flags = array->flags;
	newarray->table_size = array->table_size;

	old = array->nodes;
	for (i = 0; i < size; i++) {
		if (old[i] == NULL)
			continue;
		if (old[i]->type == Node_val)
			new[i] = dupnode(old[i]);
		else {
			NODE *r;
			r = make_array();
			r->vname = estrdup(old[i]->vname, strlen(old[i]->vname));
			r->parent_array = newsymb;
			new[i] = assoc_copy(old[i], r);
		}
	}
}


/* leaf_list --- return a list of items */

static long
leaf_list(NODE *array, NODE **list, assoc_kind_t assoc_kind)
{
	NODE *r, *subs;
	long num, i, ci, k = 0;
	long size = array->array_size;
	static char buf[100];

	for (i = 0; i < size; i++) {
		ci = (assoc_kind & ADESC) != 0 ? (size - 1 - i) : i;
		r = array->nodes[ci];
		if (r == NULL)
			continue;

		/* index */
		num = array->array_base + ci;
		if ((assoc_kind & AISTR) != 0) {
			sprintf(buf, "%ld", num);
			subs = make_string(buf, strlen(buf));
			subs->numbr = num;
			subs->flags |= (NUMCUR|NUMINT);
		} else {
			subs = make_number((AWKNUM) num);
			subs->flags |= (INTIND|NUMINT);
		}
		list[k++] = subs;

		/* value */
		if ((assoc_kind & AVALUE) != 0) {
			if (r->type == Node_val) {
				if ((assoc_kind & AVNUM) != 0)
					(void) force_number(r);
				else if ((assoc_kind & AVSTR) != 0)
					r = force_string(r);
			}
			list[k++] = r;
		}
		if ((assoc_kind & ADELETE) != 0 && k >= 1)
			return k;
	}

	return k;
}


/* leaf_info --- print index, value info */

static void
leaf_info(NODE *array, NODE *ndump, const char *aname)
{
	NODE *subs, *val;
	size_t i, size;

	size = array->array_size;

	subs = make_number((AWKNUM) 0.0);
	subs->flags |= (INTIND|NUMINT);
	for (i = 0; i < size; i++) {
		val = array->nodes[i];
		if (val == NULL)
			continue;
		subs->numbr = array->array_base + i;
		assoc_info(subs, val, ndump, aname);
	}
	unref(subs);
}

#ifdef ARRAYDEBUG

/* leaf_print --- print the leaf-array structure */


static void
leaf_print(NODE *array, size_t bi, int indent_level)
{
	indent(indent_level);
	fprintf(output_fp, "%4lu:L[%4lu:%-4lu]\n",
			(unsigned long) bi,
			(unsigned long) array->array_size,
			(unsigned long) array->table_size);
}
#endif

static NODE *argv_shadow_array = NULL;

/* argv_store --- post assign function for ARGV in sandbox mode */

static NODE **
argv_store(NODE *symbol, NODE *subs)
{
	NODE **val = cint_exists(symbol, subs);
	NODE *newval = *val;
	char *cp;

	if (newval->stlen == 0)	// empty strings in ARGV are OK
		return val;

	if ((cp = strchr(newval->stptr, '=')) == NULL) {
		if (! in_array(argv_shadow_array, newval))
			fatal(_("cannot add a new file (%.*s) to ARGV in sandbox mode"),
				(int) newval->stlen, newval->stptr);
	} else {
		// check if it's a valid variable assignment
		bool badvar = false;
		char *arg = newval->stptr;
		char *cp2;

		*cp = '\0';	// temporarily

		if (! is_letter((unsigned char) arg[0]))
			badvar = true;
		else
			for (cp2 = arg+1; *cp2; cp2++)
				if (! is_identchar((unsigned char) *cp2) && *cp2 != ':') {
					badvar = true;
					break;
				}

		// further checks
		if (! badvar) {
			char *cp = strchr(arg, ':');
			if (cp && (cp[1] != ':' || strchr(cp + 2, ':') != NULL))
				badvar = true;
		}
		*cp = '=';	// restore the '='

		if (badvar && ! in_array(argv_shadow_array, newval))
			fatal(_("cannot add a new file (%.*s) to ARGV in sandbox mode"),
				(int) newval->stlen, newval->stptr);

		// otherwise, badvar is false, let it through as variable assignment
	}
	return val;
}

/* init_argv_array --- set up the pointers for ARGV in sandbox mode. A bit hacky. */

void
init_argv_array(NODE *argv_node, NODE *shadow_node)
{
	/* If POSIX simply don't reset the vtable and things work as before */
	if (! do_sandbox)
		return;

	argv_node->array_funcs = & argv_array_func;
	argv_shadow_array = shadow_node;
}
