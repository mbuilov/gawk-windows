/*
 * array.c - routines for associative arrays.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2011 the Free Software Foundation, Inc.
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

/*
 * Tree walks (``for (iggy in foo)'') and array deletions use expensive
 * linear searching.  So what we do is start out with small arrays and
 * grow them as needed, so that our arrays are hopefully small enough,
 * most of the time, that they're pretty full and we're not looking at
 * wasted space.
 *
 * The decision is made to grow the array if the average chain length is
 * ``too big''. This is defined as the total number of entries in the table
 * divided by the size of the array being greater than some constant.
 *
 * We make the constant a variable, so that it can be tweaked
 * via environment variable.
 */

static size_t AVG_CHAIN_MAX = 2;	/* Modern machines are bigger, reduce this from 10. */

static size_t SUBSEPlen;
static char *SUBSEP;

static NODE *assoc_find(NODE *symbol, NODE *subs, unsigned long hash1, NODE **last);
static void grow_table(NODE *symbol);

static unsigned long gst_hash_string(const char *str, size_t len, unsigned long hsize, size_t *code);
static unsigned long scramble(unsigned long x);
static unsigned long awk_hash(const char *s, size_t len, unsigned long hsize, size_t *code);

unsigned long (*hash)(const char *s, size_t len, unsigned long hsize, size_t *code) = awk_hash;

/* qsort comparison function */
static int sort_up_index_string(const void *, const void *);
static int sort_down_index_string(const void *, const void *);
static int sort_up_index_number(const void *, const void *);
static int sort_down_index_number(const void *, const void *);
static int sort_up_value_string(const void *, const void *);
static int sort_down_value_string(const void *, const void *);
static int sort_up_value_number(const void *, const void *);
static int sort_down_value_number(const void *, const void *);
static int sort_up_value_type(const void *, const void *);
static int sort_down_value_type(const void *, const void *);

/* array_init --- check relevant environment variables */

void
array_init()
{
	const char *val;
	char *endptr;
	size_t newval;

	if ((val = getenv("AVG_CHAIN_MAX")) != NULL && isdigit((unsigned char) *val)) {
		newval = strtoul(val, & endptr, 10);
		if (endptr != val && newval > 0)
			AVG_CHAIN_MAX = newval;
	}

	if ((val = getenv("AWK_HASH")) != NULL && strcmp(val, "gst") == 0)
		hash = gst_hash_string; 
}

/* make_aname --- construct a 'vname' for a (sub)array */

static char *
make_aname(const NODE *symbol)
{
	static char *aname = NULL;
	static size_t alen;
	static size_t max_alen;
#define SLEN 256

	if (symbol->parent_array != NULL) {
		size_t slen;

		(void) make_aname(symbol->parent_array);
		slen = strlen(symbol->vname);	/* subscript in parent array */
		if (alen + slen + 4 > max_alen) {		/* sizeof("[\"\"]") = 4 */
			max_alen = alen + slen + 4 + SLEN;
			erealloc(aname, char *, (max_alen + 1) * sizeof(char *), "make_aname");
		}
		alen += sprintf(aname + alen, "[\"%s\"]", symbol->vname);
	} else {
		alen = strlen(symbol->vname);
		if (aname == NULL) {
			max_alen = alen + SLEN;
			emalloc(aname, char *, (max_alen + 1) * sizeof(char *), "make_aname");
		} else if (alen > max_alen) {
			max_alen = alen + SLEN; 
			erealloc(aname, char *, (max_alen + 1) * sizeof(char *), "make_aname");
		}
		memcpy(aname, symbol->vname, alen + 1);
	} 
	return aname;
#undef SLEN
}

/*
 * array_vname --- print the name of the array
 *
 * Returns a pointer to a statically maintained dynamically allocated string.
 * It's appropriate for printing the name once; if the caller wants
 * to save it, they have to make a copy.
 */

char *
array_vname(const NODE *symbol)
{
	static char *message = NULL;
	static size_t msglen = 0;
	char *s;
	size_t len;
	int n;
	const NODE *save_symbol = symbol;
	const char *from = _("from %s");
	const char *aname;
	
	if (symbol->type != Node_array_ref
			|| symbol->orig_array->type != Node_var_array
	) {
		if (symbol->type != Node_var_array || symbol->parent_array == NULL)	
			return symbol->vname;
		return make_aname(symbol);
	}

	/* First, we have to compute the length of the string: */

	len = 2; /* " (" */
	n = 0;
	while (symbol->type == Node_array_ref) {
		len += strlen(symbol->vname);
		n++;
		symbol = symbol->prev_array;
	}

	/* Get the (sub)array name */
	if (symbol->parent_array == NULL)
		aname = symbol->vname;
	else
		aname = make_aname(symbol);
	len += strlen(aname);

	/*
	 * Each node contributes by strlen(from) minus the length
	 * of "%s" in the translation (which is at least 2)
	 * plus 2 for ", " or ")\0"; this adds up to strlen(from).
	 */
	len += n * strlen(from);

	/* (Re)allocate memory: */
	if (message == NULL) {
		emalloc(message, char *, len, "array_vname");
		msglen = len;
	} else if (len > msglen) {
		erealloc(message, char *, len, "array_vname");
		msglen = len;
	} /* else
		current buffer can hold new name */

	/* We're ready to print: */
	symbol = save_symbol;
	s = message;
	/*
	 * Ancient systems have sprintf() returning char *, not int.
	 * If you have one of those, use sprintf(..); s += strlen(s) instead.
	 */

	s += sprintf(s, "%s (", symbol->vname);
	for (;;) {
		symbol = symbol->prev_array;
		if (symbol->type != Node_array_ref)
			break;
		s += sprintf(s, from, symbol->vname);
		s += sprintf(s, ", ");
	}
	s += sprintf(s, from, aname);
	strcpy(s, ")");

	return message;
}


/*
 *  get_array --- proceed to the actual Node_var_array,
 *	change Node_var_new to an array.
 *	If canfatal and type isn't good, die fatally,
 *	otherwise return the final actual value.
 */

NODE *
get_array(NODE *symbol, int canfatal)
{
	NODE *save_symbol = symbol;
	int isparam = FALSE;

	if (symbol->type == Node_param_list && (symbol->flags & FUNC) == 0) {
		save_symbol = symbol = GET_PARAM(symbol->param_cnt);
		isparam = TRUE;
		if (symbol->type == Node_array_ref)
			symbol = symbol->orig_array;
	}

	switch (symbol->type) {
	case Node_var_new:
		symbol->type = Node_var_array;
		symbol->var_array = NULL;
		symbol->parent_array = NULL;	/* main array has no parent */
		/* fall through */
	case Node_var_array:
		break;

	case Node_array_ref:
	case Node_param_list:
		if ((symbol->flags & FUNC) == 0)
			cant_happen();
		/* else
			fall through */

	default:
		/* notably Node_var but catches also e.g. FS[1] = "x" */
		if (canfatal) {
			if (symbol->type == Node_val)
				fatal(_("attempt to use a scalar value as array"));

			if ((symbol->flags & FUNC) != 0)
				fatal(_("attempt to use function `%s' as an array"),
								save_symbol->vname);
			else if (isparam)
				fatal(_("attempt to use scalar parameter `%s' as an array"),
								save_symbol->vname);
			else
				fatal(_("attempt to use scalar `%s' as an array"),
								save_symbol->vname);
		} else
			break;
	}

	return symbol;
}


/* set_SUBSEP --- update SUBSEP related variables when SUBSEP assigned to */
                                
void
set_SUBSEP()
{
	SUBSEP = force_string(SUBSEP_node->var_value)->stptr;
	SUBSEPlen = SUBSEP_node->var_value->stlen;
}                     

/* concat_exp --- concatenate expression list into a single string */

NODE *
concat_exp(int nargs, int do_subsep)
{
	/* do_subsep is false for Node-concat */
	NODE *r;
	char *str;
	char *s;
	size_t len;
	size_t subseplen = 0;
	int i;
	extern NODE **args_array;
	
	if (nargs == 1)
		return POP_STRING();

	if (do_subsep)
		subseplen = SUBSEPlen;

	len = 0;
	for (i = 1; i <= nargs; i++) {
		r = POP();
		if (r->type == Node_var_array) {
			while (--i > 0)
				DEREF(args_array[i]);	/* avoid memory leak */
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(r));
		} 
		args_array[i] = force_string(r);
		len += r->stlen;
	}
	len += (nargs - 1) * subseplen;

	emalloc(str, char *, len + 2, "concat_exp");

	r = args_array[nargs];
	memcpy(str, r->stptr, r->stlen);
	s = str + r->stlen;
	DEREF(r);
	for (i = nargs - 1; i > 0; i--) {
		if (subseplen == 1)
			*s++ = *SUBSEP;
		else if (subseplen > 0) {
			memcpy(s, SUBSEP, subseplen);
			s += subseplen;
		}
		r = args_array[i];
		memcpy(s, r->stptr, r->stlen);
		s += r->stlen;
		DEREF(r);
	}

	return make_str_node(str, len, ALREADY_MALLOCED);
}


/* assoc_clear --- flush all the values in symbol[] */

void
assoc_clear(NODE *symbol)
{
	long i;
	NODE *bucket, *next;

	if (symbol->var_array == NULL)
		return;

	for (i = 0; i < symbol->array_size; i++) {
		for (bucket = symbol->var_array[i]; bucket != NULL; bucket = next) {
			next = bucket->ahnext;
			if (bucket->ahvalue->type == Node_var_array) {
				NODE *r = bucket->ahvalue;
				assoc_clear(r);		/* recursively clear all sub-arrays */
				efree(r->vname);			
				freenode(r);
			} else
				unref(bucket->ahvalue);

			unref(bucket);	/* unref() will free the ahname_str */
		}
		symbol->var_array[i] = NULL;
	}
	efree(symbol->var_array);
	symbol->var_array = NULL;
	symbol->array_size = symbol->table_size = 0;
	symbol->flags &= ~ARRAYMAXED;
}

/* awk_hash --- calculate the hash function of the string in subs */

static unsigned long
awk_hash(const char *s, size_t len, unsigned long hsize, size_t *code)
{
	unsigned long h = 0;
	unsigned long htmp;

	/*
	 * Ozan Yigit's original sdbm hash, copied from Margo Seltzers
	 * db package.
	 *
	 * This is INCREDIBLY ugly, but fast.  We break the string up into
	 * 8 byte units.  On the first time through the loop we get the
	 * "leftover bytes" (strlen % 8).  On every other iteration, we
	 * perform 8 HASHC's so we handle all 8 bytes.  Essentially, this
	 * saves us 7 cmp & branch instructions.  If this routine is
	 * heavily used enough, it's worth the ugly coding.
	 */

	/*
	 * Even more speed:
	 * #define HASHC   h = *s++ + 65599 * h
	 * Because 65599 = pow(2, 6) + pow(2, 16) - 1 we multiply by shifts
	 *
	 * 4/2011: Force the results to 32 bits, to get the same
	 * result on both 32- and 64-bit systems. This may be a
	 * bad idea.
	 */
#define HASHC   htmp = (h << 6);  \
		h = *s++ + htmp + (htmp << 10) - h ; \
		htmp &= 0xFFFFFFFF; \
		h &= 0xFFFFFFFF

	h = 0;

	/* "Duff's Device" */
	if (len > 0) {
		size_t loop = (len + 8 - 1) >> 3;

		switch (len & (8 - 1)) {
		case 0:
			do {	/* All fall throughs */
				HASHC;
		case 7:		HASHC;
		case 6:		HASHC;
		case 5:		HASHC;
		case 4:		HASHC;
		case 3:		HASHC;
		case 2:		HASHC;
		case 1:		HASHC;
			} while (--loop);
		}
	}

	if (code != NULL)
		*code = h;

	if (h >= hsize)
		h %= hsize;
	return h;
}

/* assoc_find --- locate symbol[subs] */

static NODE *				/* NULL if not found */
assoc_find(NODE *symbol, NODE *subs, unsigned long hash1, NODE **last)
{
	NODE *bucket, *prev;
	const char *s1_str;
	size_t s1_len;
	NODE *s2;

	for (prev = NULL, bucket = symbol->var_array[hash1]; bucket != NULL;
			prev = bucket, bucket = bucket->ahnext) {
		/*
		 * This used to use cmp_nodes() here.  That's wrong.
		 * Array indices are strings; compare as such, always!
		 */
		s1_str = bucket->ahname_str;
		s1_len = bucket->ahname_len;
		s2 = subs;

		if (s1_len == s2->stlen) {
			if (s1_len == 0		/* "" is a valid index */
			    || memcmp(s1_str, s2->stptr, s1_len) == 0)
				break;
		}
	}
	if (last != NULL)
		*last = prev;
	return bucket;
}

/* in_array --- test whether the array element symbol[subs] exists or not,
 * 		return pointer to value if it does.
 */

NODE *
in_array(NODE *symbol, NODE *subs)
{
	unsigned long hash1;
	NODE *ret;

	assert(symbol->type == Node_var_array);

	if (symbol->var_array == NULL)
		return NULL;

	hash1 = hash(subs->stptr, subs->stlen, (unsigned long) symbol->array_size, NULL);
	ret = assoc_find(symbol, subs, hash1, NULL);
	return (ret ? ret->ahvalue : NULL);
}

/*
 * assoc_lookup:
 * Find SYMBOL[SUBS] in the assoc array.  Install it with value "" if it
 * isn't there. Returns a pointer ala get_lhs to where its value is stored.
 *
 * SYMBOL is the address of the node (or other pointer) being dereferenced.
 * SUBS is a number or string used as the subscript. 
 */

NODE **
assoc_lookup(NODE *symbol, NODE *subs, int reference)
{
	unsigned long hash1;
	NODE *bucket;
	size_t code;

	assert(symbol->type == Node_var_array);

	(void) force_string(subs);

	if (symbol->var_array == NULL) {
		symbol->array_size = symbol->table_size = 0;	/* sanity */
		symbol->flags &= ~ARRAYMAXED;
		grow_table(symbol);
		hash1 = hash(subs->stptr, subs->stlen,
				(unsigned long) symbol->array_size, & code);
	} else {
		hash1 = hash(subs->stptr, subs->stlen,
				(unsigned long) symbol->array_size, & code);
		bucket = assoc_find(symbol, subs, hash1, NULL);
		if (bucket != NULL)
			return &(bucket->ahvalue);
	}

	if (do_lint && reference) {
		lintwarn(_("reference to uninitialized element `%s[\"%.*s\"]'"),
		      array_vname(symbol), (int)subs->stlen, subs->stptr);
	}

	/* It's not there, install it. */
	if (do_lint && subs->stlen == 0)
		lintwarn(_("subscript of array `%s' is null string"),
			array_vname(symbol));

	/* first see if we would need to grow the array, before installing */
	symbol->table_size++;
	if ((symbol->flags & ARRAYMAXED) == 0
	    && (symbol->table_size / symbol->array_size) > AVG_CHAIN_MAX) {
		grow_table(symbol);
		/* have to recompute hash value for new size */
		hash1 = code % (unsigned long) symbol->array_size;
	}

	getnode(bucket);
	bucket->type = Node_ahash;

	/*
	 * Freeze this string value --- it must never
	 * change, no matter what happens to the value
	 * that created it or to CONVFMT, etc.
	 *
	 * One day: Use an atom table to track array indices,
	 * and avoid the extra memory overhead.
	 */
	bucket->flags |= MALLOC;
	bucket->ahname_ref = 1;

	emalloc(bucket->ahname_str, char *, subs->stlen + 2, "assoc_lookup");
	bucket->ahname_len = subs->stlen;
	memcpy(bucket->ahname_str, subs->stptr, subs->stlen);
	bucket->ahname_str[bucket->ahname_len] = '\0';
	bucket->ahvalue = Nnull_string;
 
	bucket->ahnext = symbol->var_array[hash1];
	bucket->ahcode = code;

	/*
	 * Set the numeric value for the index if it's  available. Useful
	 * for numeric sorting by index.  Do this only if the numeric
	 * value is available, instead of all the time, since doing it
	 * all the time is a big performance hit for something that may
	 * never be used.
	 */
	if ((subs->flags & NUMCUR) != 0) {
		bucket->ahname_num = subs->numbr;
		bucket->flags |= NUMIND;
	}

	/* hook it into the symbol table */
	symbol->var_array[hash1] = bucket;
	return &(bucket->ahvalue);
}


/* adjust_fcall_stack: remove subarray(s) of symbol[] from
 *	function call stack.
 */

static void
adjust_fcall_stack(NODE *symbol, int nsubs)
{
	NODE *func, *r, *n;
	NODE **sp;
	int pcount;

	/*
	 * Solve the nasty problem of disappearing subarray arguments:
	 *
	 *  function f(c, d) { delete c; .. use non-existent array d .. }
	 *  BEGIN { a[0][0] = 1; f(a, a[0]); .. }
	 *
	 * The fix is to convert 'd' to a local empty array; This has
	 * to be done before clearing the parent array to avoid referring to
	 * already free-ed memory.
	 *
	 * Similar situations exist for builtins accepting more than
	 * one array argument: split, patsplit, asort and asorti. For example:
	 *
	 *  BEGIN { a[0][0] = 1; split("abc", a, "", a[0]) }
	 *
	 * These cases do not involve the function call stack, and are
	 * handled individually in their respective routines.
	 */

	func = frame_ptr->func_node;
	if (func == NULL)	/* in main */
		return;
	pcount = func->lnode->param_cnt;
	sp = frame_ptr->stack;

	for (; pcount > 0; pcount--) {
		r = *sp++;
		if (r->type != Node_array_ref
				|| r->orig_array->type != Node_var_array)
			continue;
		n = r->orig_array;

		/* Case 1 */
		if (n == symbol
			&& symbol->parent_array != NULL
			&& nsubs > 0
		) {
			/* 'symbol' is a subarray, and 'r' is the same subarray:
			 *
			 *   function f(c, d) { delete c[0]; .. }
			 *   BEGIN { a[0][0] = 1; f(a, a[0]); .. }
			 *
			 * But excludes cases like (nsubs = 0):
			 *
			 *   function f(c, d) { delete c; ..}
			 *   BEGIN { a[0][0] = 1; f(a[0], a[0]); ...}  
			 */
			char *save;
local_array:
			save = r->vname;
			memset(r, '\0', sizeof(NODE));
			r->vname = save;
			r->type = Node_var_array;
			continue;
		}			

		/* Case 2 */
		for (n = n->parent_array; n != NULL; n = n->parent_array) {
			assert(n->type == Node_var_array);
			if (n == symbol) {
				/* 'r' is a subarray of 'symbol':
				 *
				 *    function f(c, d) { delete c; .. use d as array .. }
				 *    BEGIN { a[0][0] = 1; f(a, a[0]); .. }
				 *	OR
				 *    BEGIN { a[0][0][0][0] = 1; f(a[0], a[0][0][0]); .. }
				 *
				 */

				goto local_array;
			}
		}
	}
}


/* do_delete --- perform `delete array[s]' */

/*
 * `symbol' is array
 * `nsubs' is number of subscripts
 */

void
do_delete(NODE *symbol, int nsubs)
{
	unsigned long hash1 = 0;
	NODE *subs, *bucket, *last, *r;
	int i;

	assert(symbol->type == Node_var_array);
	subs = bucket = last = r = NULL;	/* silence the compiler */

	/*
	 * The force_string() call is needed to make sure that
	 * the string subscript is reasonable.  For example, with it:
	 *
	 * $ ./gawk --posix 'BEGIN { CONVFMT="%ld"; delete a[1.233]}'
	 * gawk: cmd. line:1: fatal: `%l' is not permitted in POSIX awk formats
	 *
	 * Without it, the code does not fail.
	 */

#define free_subs(n) \
do {								\
    NODE *s = PEEK(n - 1);                                      \
    if (s->type == Node_val) {                                  \
        (void) force_string(s);	/* may have side effects ? */   \
        DEREF(s);                                               \
    }                                                           \
} while (--n > 0)

	if (nsubs == 0) {	/* delete array */
		adjust_fcall_stack(symbol, 0);	/* fix function call stack; See above. */
		assoc_clear(symbol);
		return;
	}

	/* NB: subscripts are in reverse order on stack */

	for (i = nsubs; i > 0; i--) {
		subs = PEEK(i - 1);
		if (subs->type != Node_val) {
			free_subs(i);
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(subs));
		}
		(void) force_string(subs);

		last = NULL;	/* shut up gcc -Wall */
		hash1 = 0;	/* ditto */
		bucket = NULL;	/* array may be empty */

		if (symbol->var_array != NULL) {
			hash1 = hash(subs->stptr, subs->stlen,
					(unsigned long) symbol->array_size, NULL);
			bucket = assoc_find(symbol, subs, hash1, &last);
		}

		if (bucket == NULL) {
			if (do_lint)
				lintwarn(_("delete: index `%s' not in array `%s'"),
					subs->stptr, array_vname(symbol));
			/* avoid memory leak, free all subs */
			free_subs(i);
			return;
		}

		if (i > 1) {
			if (bucket->ahvalue->type != Node_var_array) {
				/* e.g.: a[1] = 1; delete a[1][1] */
				free_subs(i);
				fatal(_("attempt to use scalar `%s[\"%.*s\"]' as an array"),
					array_vname(symbol),
					(int) bucket->ahname_len,
					bucket->ahname_str);
			}
			symbol = bucket->ahvalue;
		}
		DEREF(subs);
	}

	r = bucket->ahvalue;
	if (r->type == Node_var_array) {
		adjust_fcall_stack(r, nsubs);	/* fix function call stack; See above. */
		assoc_clear(r);
		/* cleared a sub-array, free Node_var_array */
		efree(r->vname);
		freenode(r);
	} else
		unref(r);

	if (last != NULL)
		last->ahnext = bucket->ahnext;
	else
		symbol->var_array[hash1] = bucket->ahnext;

	unref(bucket);	/* unref() will free the ahname_str */
	symbol->table_size--;
	if (symbol->table_size <= 0) {
		symbol->table_size = symbol->array_size = 0;
		symbol->flags &= ~ARRAYMAXED;
		if (symbol->var_array != NULL) {
			efree(symbol->var_array);
			symbol->var_array = NULL;
		}
	}

#undef free_subs
}


/* do_delete_loop --- simulate ``for (iggy in foo) delete foo[iggy]'' */

/*
 * The primary hassle here is that `iggy' needs to have some arbitrary
 * array index put in it before we can clear the array, we can't
 * just replace the loop with `delete foo'.
 */

void
do_delete_loop(NODE *symbol, NODE **lhs)
{
	long i;

	assert(symbol->type == Node_var_array);

	if (symbol->var_array == NULL)
		return;

	/* get first index value */
	for (i = 0; i < symbol->array_size; i++) {
		if (symbol->var_array[i] != NULL) {
			unref(*lhs);
			*lhs = make_string(symbol->var_array[i]->ahname_str,
					symbol->var_array[i]->ahname_len);
			break;
		}
	}

	/* blast the array in one shot */
	adjust_fcall_stack(symbol, 0);
	assoc_clear(symbol);
}

/* grow_table --- grow a hash table */

static void
grow_table(NODE *symbol)
{
	NODE **old, **new, *chain, *next;
	int i, j;
	unsigned long hash1;
	unsigned long oldsize, newsize, k;
	/*
	 * This is an array of primes. We grow the table by an order of
	 * magnitude each time (not just doubling) so that growing is a
	 * rare operation. We expect, on average, that it won't happen
	 * more than twice.  When things are very large (> 8K), we just
	 * double more or less, instead of just jumping from 8K to 64K.
	 */
	static const long sizes[] = {
		13, 127, 1021, 8191, 16381, 32749, 65497, 131101, 262147,
		524309, 1048583, 2097169, 4194319, 8388617, 16777259, 33554467, 
		67108879, 134217757, 268435459, 536870923, 1073741827
	};

	/* find next biggest hash size */
	newsize = oldsize = symbol->array_size;
	for (i = 0, j = sizeof(sizes)/sizeof(sizes[0]); i < j; i++) {
		if (oldsize < sizes[i]) {
			newsize = sizes[i];
			break;
		}
	}

	if (newsize == oldsize) {	/* table already at max (!) */
		symbol->flags |= ARRAYMAXED;
		return;
	}

	/* allocate new table */
	emalloc(new, NODE **, newsize * sizeof(NODE *), "grow_table");
	memset(new, '\0', newsize * sizeof(NODE *));

	/* brand new hash table, set things up and return */
	if (symbol->var_array == NULL) {
		symbol->table_size = 0;
		goto done;
	}

	/* old hash table there, move stuff to new, free old */
	old = symbol->var_array;
	for (k = 0; k < oldsize; k++) {
		if (old[k] == NULL)
			continue;

		for (chain = old[k]; chain != NULL; chain = next) {
			next = chain->ahnext;
			hash1 = chain->ahcode % newsize;

			/* remove from old list, add to new */
			chain->ahnext = new[hash1];
			new[hash1] = chain;
		}
	}
	efree(old);

done:
	/*
	 * note that symbol->table_size does not change if an old array,
	 * and is explicitly set to 0 if a new one.
	 */
	symbol->var_array = new;
	symbol->array_size = newsize;
}

/* pr_node --- print simple node info */

static void
pr_node(NODE *n)
{
	if ((n->flags & NUMBER) != 0)
		printf("%s %g p: %p", flags2str(n->flags), n->numbr, n);
	else
		printf("%s %.*s p: %p", flags2str(n->flags),
				(int) n->stlen, n->stptr, n);
}


static void
indent(int indent_level)
{
	int k;
	for (k = 0; k < indent_level; k++)
		putchar('\t');
}

/* assoc_dump --- dump the contents of an array */

NODE *
assoc_dump(NODE *symbol, int indent_level)
{
	long i;
	NODE *bucket;

	indent(indent_level);
	if (symbol->var_array == NULL) {
		printf(_("%s: empty (null)\n"), symbol->vname);
		return make_number((AWKNUM) 0);
	}

	if (symbol->table_size == 0) {
		printf(_("%s: empty (zero)\n"), symbol->vname);
		return make_number((AWKNUM) 0);
	}

	printf(_("%s: table_size = %d, array_size = %d\n"), symbol->vname,
			(int) symbol->table_size, (int) symbol->array_size);

	for (i = 0; i < symbol->array_size; i++) {
		for (bucket = symbol->var_array[i]; bucket != NULL;
				bucket = bucket->ahnext) {
			indent(indent_level);
			printf("%s: I: [len %d <%.*s> p: %p] V: [",
				symbol->vname,
				(int) bucket->ahname_len,
				(int) bucket->ahname_len,
				bucket->ahname_str,
				bucket->ahname_str);
			if (bucket->ahvalue->type == Node_var_array) {
				printf("\n");
				assoc_dump(bucket->ahvalue, indent_level + 1);
				indent(indent_level);
			} else
				pr_node(bucket->ahvalue);
			printf("]\n");
		}
	}

	return make_number((AWKNUM) 0);
}

/* do_adump --- dump an array: interface to assoc_dump */

NODE *
do_adump(int nargs)
{
	NODE *r, *a;

	a = POP();
	if (a->type == Node_param_list) {
		printf(_("%s: is parameter\n"), a->vname);
		a = GET_PARAM(a->param_cnt);
	}
	if (a->type == Node_array_ref) {
		printf(_("%s: array_ref to %s\n"), a->vname,
					a->orig_array->vname);
		a = a->orig_array;
	}
	if (a->type != Node_var_array)
		fatal(_("adump: argument not an array"));
	r = assoc_dump(a, 0);
	return r;
}

/*
 * The following functions implement the builtin
 * asort function.  Initial work by Alan J. Broder,
 * ajb@woti.com.
 */

/* dup_table --- recursively duplicate input array "symbol" */

static NODE *
dup_table(NODE *symbol, NODE *newsymb)
{
	NODE **old, **new, *chain, *bucket;
	long i;
	unsigned long cursize;

	/* find the current hash size */
	cursize = symbol->array_size;

	new = NULL;

	/* input is a brand new hash table, so there's nothing to copy */
	if (symbol->var_array == NULL)
		newsymb->table_size = 0;
	else {
		/* old hash table there, dupnode stuff into a new table */

		/* allocate new table */
		emalloc(new, NODE **, cursize * sizeof(NODE *), "dup_table");
		memset(new, '\0', cursize * sizeof(NODE *));

		/* do the copying/dupnode'ing */
		old = symbol->var_array;
		for (i = 0; i < cursize; i++) {
			if (old[i] != NULL) {
				for (chain = old[i]; chain != NULL;
						chain = chain->ahnext) {
					/* get a node for the linked list */
					getnode(bucket);
					bucket->type = Node_ahash;
					bucket->flags |= MALLOC;
					bucket->ahname_ref = 1;
					bucket->ahcode = chain->ahcode;
					if ((chain->flags & NUMIND) != 0) {
						bucket->ahname_num = chain->ahname_num;
						bucket->flags |= NUMIND;
					}

					/*
					 * copy the corresponding name and
					 * value from the original input list
					 */
					emalloc(bucket->ahname_str, char *, chain->ahname_len + 2, "dup_table");
					bucket->ahname_len = chain->ahname_len;

					memcpy(bucket->ahname_str, chain->ahname_str, chain->ahname_len);
					bucket->ahname_str[bucket->ahname_len] = '\0';

					if (chain->ahvalue->type == Node_var_array) {
						NODE *r;
						getnode(r);
						r->type = Node_var_array;
						r->vname = estrdup(chain->ahname_str, chain->ahname_len);
						r->parent_array = newsymb;
						bucket->ahvalue = dup_table(chain->ahvalue, r);
					} else
						bucket->ahvalue = dupnode(chain->ahvalue);

					/*
					 * put the node on the corresponding
					 * linked list in the new table
					 */
					bucket->ahnext = new[i];
					new[i] = bucket;
				}
			}
		}
		newsymb->table_size = symbol->table_size;
	}

	newsymb->var_array = new;
	newsymb->array_size = cursize;
	newsymb->flags = symbol->flags;	/* ARRAYMAXED */
	return newsymb;
}


/* asort_actual --- do the actual work to sort the input array */

static NODE *
asort_actual(int nargs, SORT_CTXT ctxt)
{
	NODE *array, *dest = NULL, *result;
	NODE *r, *subs, *s;
	NODE **list, **ptr;
#define TSIZE	100	/* an arbitrary amount */
	static char buf[TSIZE+2];
	unsigned long num_elems, i;
	const char *sort_str;

	if (nargs == 3)  /* 3rd optional arg */
		s = POP_STRING();
	else
		s = Nnull_string;	/* "" => default sorting */

	s = force_string(s);
	sort_str = s->stptr;
	if (s->stlen == 0) {		/* default sorting */
		if (ctxt == ASORT)
			sort_str = "@val_type_asc";
		else
			sort_str = "@ind_str_asc";
	}


	if (nargs >= 2) {  /* 2nd optional arg */
		dest = POP_PARAM();
		if (dest->type != Node_var_array) {
			fatal(ctxt == ASORT ?
				_("asort: second argument not an array") :
				_("asorti: second argument not an array"));
		}
	}

	array = POP_PARAM();
	if (array->type != Node_var_array) {
		fatal(ctxt == ASORT ?
			_("asort: first argument not an array") :
			_("asorti: first argument not an array"));
	}

	if (dest != NULL) {
		for (r = dest->parent_array; r != NULL; r = r->parent_array) {
			if (r == array)
				fatal(ctxt == ASORT ?
					_("asort: cannot use a subarray of first arg for second arg") :
					_("asorti: cannot use a subarray of first arg for second arg"));
		}
		for (r = array->parent_array; r != NULL; r = r->parent_array) {
			if (r == dest)
				fatal(ctxt == ASORT ?
					_("asort: cannot use a subarray of second arg for first arg") :
					_("asorti: cannot use a subarray of second arg for first arg"));
                }
	}

	num_elems = array->table_size;
	if (num_elems == 0 || array->var_array == NULL) {	/* source array is empty */
		if (dest != NULL && dest != array)
			assoc_clear(dest);
		return make_number((AWKNUM) 0);
	}

	/* sorting happens inside assoc_list */
	list = assoc_list(array, sort_str, ctxt);
	DEREF(s);

	/*
	 * Must not assoc_clear() the source array before constructing
	 * the output array. assoc_list() does not duplicate array values
	 * which are needed for asort().
	 */

	if (dest != NULL && dest != array) {
		assoc_clear(dest);
		result = dest;
	} else {
		/* use 'result' as a temporary destination array */
		getnode(result);
		memset(result, '\0', sizeof(NODE));
		result->type = Node_var_array;
		result->vname = array->vname;
		result->parent_array = array->parent_array;
	}

	subs = make_str_node(buf, TSIZE, ALREADY_MALLOCED);   /* fake it */
	subs->flags &= ~MALLOC;		/* safety */
	for (i = 1, ptr = list; i <= num_elems; i++) {
		sprintf(buf, "%lu", i);
		subs->stlen = strlen(buf);
		/* make number valid in case this array gets sorted later */
		subs->numbr = i;
		subs->flags |= NUMCUR;
		r = *ptr++;
		if (ctxt == ASORTI) {
			/*
			 * We want the indices of the source array as values
			 * of the 'result' array.
			 */
			*assoc_lookup(result, subs, FALSE) =
					make_string(r->ahname_str, r->ahname_len);
		} else {
			NODE *val;

			/* We want the values of the source array. */

			val = r->ahvalue;
			if (result != dest) {
				/* optimization for dest = NULL or dest = array */

				if (val->type == Node_var_array) {
					/* update subarray index in parent array */
					efree(val->vname);
					val->vname = estrdup(subs->stptr, subs->stlen);
				} 
				*assoc_lookup(result, subs, FALSE) = val;
				r->ahvalue = Nnull_string;
			} else {
				if (val->type == Node_val)
					*assoc_lookup(result, subs, FALSE) = dupnode(val);
				else {
					NODE *arr;

					/*
					 * There isn't any reference counting for
					 * subarrays, so recursively copy subarrays
					 * using dup_table().
					 */
					getnode(arr);
					arr->type = Node_var_array;
					arr->var_array = NULL;
					arr->vname = estrdup(subs->stptr, subs->stlen);
					arr->parent_array = array; /* actual parent, not the temporary one. */
					*assoc_lookup(result, subs, FALSE) = dup_table(val, arr);
				}
			}
		}

		unref(r);
 	}

	freenode(subs);	/* stptr(buf) not malloc-ed */
	efree(list);

	if (result != dest) {
		/* dest == NULL or dest == array */
		assoc_clear(array);
		*array = *result;	/* copy result into array */
		freenode(result);
	} /* else
		result == dest
		dest != NULL and dest != array */

	return make_number((AWKNUM) num_elems);
}
#undef TSIZE

/* do_asort --- sort array by value */

NODE *
do_asort(int nargs)
{
	return asort_actual(nargs, ASORT);
}

/* do_asorti --- sort array by index */

NODE *
do_asorti(int nargs)
{
	return asort_actual(nargs, ASORTI);
}

/*
 * cmp_string --- compare two strings; logic similar to cmp_nodes() in eval.c
 *	except the extra case-sensitive comparison when the case-insensitive
 *	result is a match.
 */

static int
cmp_string(const NODE *n1, const NODE *n2)
{
	char *s1, *s2;
	size_t len1, len2;
	int ret;
	size_t lmin;

	assert(n1->type == n2->type);
	if (n1->type == Node_ahash) {
		s1 = n1->ahname_str;
		len1 = n1->ahname_len;
		s2 =  n2->ahname_str;
		len2 = n2->ahname_len;
	} else {
		s1 = n1->stptr;
		len1 = n1->stlen;
		s2 =  n2->stptr;
		len2 = n2->stlen;
	}

	if (len1 == 0)
		return len2 == 0 ? 0 : -1;
	if (len2 == 0)
		return 1;

	/* len1 > 0 && len2 > 0 */
	lmin = len1 < len2 ? len1 : len2;

	if (IGNORECASE) {
		const unsigned char *cp1 = (const unsigned char *) s1;
		const unsigned char *cp2 = (const unsigned char *) s2;

#ifdef MBS_SUPPORT
		if (gawk_mb_cur_max > 1) {
			ret = strncasecmpmbs((const unsigned char *) cp1,
					     (const unsigned char *) cp2, lmin);
		} else
#endif
		for (ret = 0; lmin-- > 0 && ret == 0; cp1++, cp2++)
			ret = casetable[*cp1] - casetable[*cp2];
		if (ret != 0)
			return ret;
		/*
		 * If case insensitive result is "they're the same",
		 * use case sensitive comparison to force distinct order.
		 */
	}

	ret = memcmp(s1, s2, lmin);
	if (ret != 0 || len1 == len2)
		return ret;
	return (len1 < len2) ? -1 : 1;
}


/* sort_up_index_string --- qsort comparison function; ascending index strings. */

static int
sort_up_index_string(const void *p1, const void *p2)
{
	const NODE *t1, *t2;

	/* Array indices are strings */
	t1 = *((const NODE *const *) p1);
	t2 = *((const NODE *const *) p2);
	return cmp_string(t1, t2);
}


/* sort_down_index_string --- descending index strings */

static int
sort_down_index_string(const void *p1, const void *p2)
{
	/*
	 * Negation versus transposed arguments:  when all keys are
	 * distinct, as with array indices here, either method will
	 * transform an ascending sort into a descending one.  But if
	 * there are equal keys--such as when IGNORECASE is honored--
	 * that get disambiguated into a determisitc order, negation
	 * will reverse those but transposed arguments would retain
	 * their relative order within the rest of the reversed sort.
	 */
	return -sort_up_index_string(p1, p2);
}


/* sort_up_index_number --- qsort comparison function; ascending index numbers. */

static int
sort_up_index_number(const void *p1, const void *p2)
{
	const NODE *n1, *n2;
	int ret;

	n1 = *((const NODE *const *) p1);
	n2 = *((const NODE *const *) p2);

	if (n1->ahname_num < n2->ahname_num)
		ret = -1;
	else
		ret = (n1->ahname_num > n2->ahname_num);

	/* break a tie with the index string itself */
	if (ret == 0)
		return cmp_string(n1, n2);
	return ret;
}


/* sort_down_index_number --- qsort comparison function; descending index numbers */

static int
sort_down_index_number(const void *p1, const void *p2)
{
	return -sort_up_index_number(p1, p2);
}


/* sort_up_value_string --- qsort comparison function; ascending value string */

static int
sort_up_value_string(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	NODE *n1, *n2;

	/* we're passed a pair of index (array subscript) nodes */
	t1 = *(const NODE *const *) p1;
	t2 = *(const NODE *const *) p2;

	/* and we want to compare the element values they refer to */
	n1 = t1->ahvalue;
	n2 = t2->ahvalue;

	if (n1->type == Node_var_array) {
		/* return 0 if n2 is a sub-array too, else return 1 */
		return (n2->type != Node_var_array);
	}
	if (n2->type == Node_var_array)
		return -1;		/* n1 (scalar) < n2 (sub-array) */

	/* n1 and n2 both have string values; See sort_force_value_string(). */
	return cmp_string(n1, n2);
}


/* sort_down_value_string --- descending value string */

static int
sort_down_value_string(const void *p1, const void *p2)
{
	return -sort_up_value_string(p1, p2);
}

/* sort_up_value_number --- qsort comparison function; ascending value number */

static int
sort_up_value_number(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	NODE *n1, *n2;
	int ret;

	/* we're passed a pair of index (array subscript) nodes */
	t1 = *(const NODE *const *) p1;
	t2 = *(const NODE *const *) p2;

	/* and we want to compare the element values they refer to */
	n1 = t1->ahvalue;
	n2 = t2->ahvalue;

	if (n1->type == Node_var_array) {
		/* return 0 if n2 is a sub-array too, else return 1 */
		return (n2->type != Node_var_array);
	}
	if (n2->type == Node_var_array)
		return -1;		/* n1 (scalar) < n2 (sub-array) */

	/* n1 and n2 both Node_val, and force_number'ed */
	if (n1->numbr < n2->numbr)
		ret = -1;
	else
		ret = (n1->numbr > n2->numbr);

	if (ret == 0) {
		/*
		 * Use string value to guarantee same sort order on all
		 * versions of qsort().
		 */
		n1 = force_string(n1);
		n2 = force_string(n2);
		ret = cmp_string(n1, n2);
	}

	return ret;
}

/* sort_down_value_number --- descending value number */

static int
sort_down_value_number(const void *p1, const void *p2)
{
	return -sort_up_value_number(p1, p2);
}

/* sort_up_value_type --- qsort comparison function; ascending value type */

static int
sort_up_value_type(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	NODE *n1, *n2;

	/* we're passed a pair of index (array subscript) nodes */
	t1 = *(const NODE *const *) p1;
	t2 = *(const NODE *const *) p2;

	/* and we want to compare the element values they refer to */
	n1 = t1->ahvalue;
	n2 = t2->ahvalue;

	/* 1. Arrays vs. scalar, scalar is less than array */
	if (n1->type == Node_var_array) {
		/* return 0 if n2 is a sub-array too, else return 1 */
		return (n2->type != Node_var_array);
	}
	if (n2->type == Node_var_array) {
		return -1;		/* n1 (scalar) < n2 (sub-array) */
	}

	/* two scalars */
	/* 2. Resolve MAYBE_NUM, so that have only NUMBER or STRING */
	if ((n1->flags & MAYBE_NUM) != 0)
		(void) force_number(n1);
	if ((n2->flags & MAYBE_NUM) != 0)
		(void) force_number(n2);

	if ((n1->flags & NUMBER) != 0 && (n2->flags & NUMBER) != 0) {
		if (n1->numbr < n2->numbr)
			return -1;
		else if (n1->numbr > n2->numbr)
			return 1;
		else
			return 0;
	}

	/* 3. All numbers are less than all strings. This is aribitrary. */
	if ((n1->flags & NUMBER) != 0 && (n2->flags & STRING) != 0) {
		return -1;
	} else if ((n1->flags & STRING) != 0 && (n2->flags & NUMBER) != 0) {
		return 1;
	}

	/* 4. Two strings */
	return cmp_string(n1, n2);
}

/* sort_down_value_type --- descending value type */

static int
sort_down_value_type(const void *p1, const void *p2)
{
	return -sort_up_value_type(p1, p2);
}

/* sort_user_func --- user defined qsort comparison function */

static int
sort_user_func(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	NODE *idx1, *idx2, *val1, *val2;
	AWKNUM ret;
	INSTRUCTION *code;
	extern int exiting;

	t1 = *((const NODE *const *) p1);
	t2 = *((const NODE *const *) p2);

	idx1 = make_string(t1->ahname_str, t1->ahname_len);
	idx2 = make_string(t2->ahname_str, t2->ahname_len);
	val1 = t1->ahvalue;
	val2 = t2->ahvalue;

	code = TOP()->code_ptr;	/* comparison function call instructions */

	/* setup 4 arguments to comp_func() */
	PUSH(idx1);
	if (val1->type == Node_val)
		UPREF(val1);
	PUSH(val1);
	PUSH(idx2);
	if (val2->type == Node_val)
		UPREF(val2);
	PUSH(val2);

	/* execute the comparison function */
	(void) interpret(code);

	if (exiting)	/* do not assume anything about the user-defined function! */
		gawk_exit(exit_val);

	/* return value of the comparison function */
	POP_NUMBER(ret);

	return (ret < 0.0) ? -1 : (ret > 0.0);
}

/* sort_force_index_number -- pre-process list items for sorting indices as numbers */

static void
sort_force_index_number(NODE **list, size_t num_elems)
{
	size_t i;
	NODE *r;
	static NODE temp_node;

	for (i = 0; i < num_elems; i++) {
		r = list[i];

		if ((r->flags & NUMIND) != 0)	/* once in a lifetime is plenty */
			continue;
		temp_node.type = Node_val;
		temp_node.stptr = r->ahname_str;
		temp_node.stlen = r->ahname_len;
		temp_node.flags = 0;	/* only interested in the return value of r_force_number */
		r->ahname_num = r_force_number(& temp_node);
		r->flags |= NUMIND;
	}
}

/* sort_force_value_number -- pre-process list items for sorting values as numbers */

static void
sort_force_value_number(NODE **list, size_t num_elems)
{
	size_t i;
	NODE *r, *val;

	for (i = 0; i < num_elems; i++) {
		r = list[i];
		val = r->ahvalue;
		if (val->type == Node_val)
			(void) force_number(val);
	}
}

/* sort_force_value_string -- pre-process list items for sorting values as strings */

static void
sort_force_value_string(NODE **list, size_t num_elems)
{
	size_t i;
	NODE *r, *val;

	for (i = 0; i < num_elems; i++) {
		r = list[i];
		val = r->ahvalue;
		if (val->type == Node_val)
			r->ahvalue = force_string(val);
	}
}

/* assoc_list -- construct, and optionally sort, a list of array elements */  

NODE **
assoc_list(NODE *array, const char *sort_str, SORT_CTXT sort_ctxt)
{
	typedef void (*qsort_prefunc)(NODE **, size_t);
	typedef int (*qsort_compfunc)(const void *, const void *);

	static const struct qsort_funcs {
		const char *name;
		qsort_compfunc comp_func;
		qsort_prefunc pre_func;		/* pre-processing of list items */
	} sort_funcs[] = {
		{ "@ind_str_asc",	sort_up_index_string,	0 },
		{ "@ind_num_asc",	sort_up_index_number,	sort_force_index_number },
		{ "@val_str_asc",	sort_up_value_string,	sort_force_value_string },
		{ "@val_num_asc",	sort_up_value_number,	sort_force_value_number	},
		{ "@ind_str_desc",	sort_down_index_string,	0 },
		{ "@ind_num_desc",	sort_down_index_number,	sort_force_index_number },
		{ "@val_str_desc",	sort_down_value_string,	sort_force_value_string },
		{ "@val_num_desc",	sort_down_value_number,	sort_force_value_number },
		{ "@val_type_asc",	sort_up_value_type,	0 },
		{ "@val_type_desc",	sort_down_value_type,	0 },
		{ "@unsorted",		0,	0 },
	};
	NODE **list;
	NODE *r;
	size_t num_elems, i, j;
	qsort_compfunc cmp_func = 0;
	qsort_prefunc pre_func = 0;
	INSTRUCTION *code = NULL;
	int qi;
	extern int currule;
	
	num_elems = array->table_size;
	assert(num_elems > 0);

	for (qi = 0, j = sizeof(sort_funcs)/sizeof(sort_funcs[0]); qi < j; qi++) {
		if (strcmp(sort_funcs[qi].name, sort_str) == 0)
			break;
	}

	if (qi >= 0 && qi < j) {
		cmp_func = sort_funcs[qi].comp_func;
		pre_func = sort_funcs[qi].pre_func;

	} else {		/* unrecognized */
		NODE *f;
		const char *sp;	

		assert(sort_str != NULL);

		for (sp = sort_str; *sp != '\0'
		     && ! isspace((unsigned char) *sp); sp++)
			continue;

		/* empty string or string with space(s) not valid as function name */
		if (sp == sort_str || *sp != '\0')
			fatal(_("`%s' is invalid as a function name"), sort_str);

		f = lookup(sort_str);
		if (f == NULL || f->type != Node_func)
			fatal(_("sort comparison function `%s' is not defined"), sort_str);

		cmp_func = sort_user_func;
		/* pre_func is still NULL */

		/* make function call instructions */
		code = bcalloc(Op_func_call, 2, 0);
		code->func_body = f;
		code->func_name = NULL;		/* not needed, func_body already assigned */
		(code + 1)->expr_count = 4;	/* function takes 4 arguments */
		code->nexti = bcalloc(Op_stop, 1, 0);	

		/* make non-local jumps `next' and `nextfile' fatal in
		 * callback function by setting currule in interpret()
		 * to undefined (0). `exit' is handled in sort_user_func.
		 */

		(code + 1)->inrule = currule;	/* save current rule */
		currule = 0;

		PUSH_CODE(code);
	}

	/* allocate space for array; the extra space is used in for(i in a) opcode (eval.c) */
	emalloc(list, NODE **, (num_elems + 1) * sizeof(NODE *), "assoc_list");

	/* populate it */
	for (i = j = 0; i < array->array_size; i++)
		for (r = array->var_array[i]; r != NULL; r = r->ahnext)
			list[j++] = dupnode(r);
	list[num_elems] = NULL;

	if (! cmp_func) /* unsorted */
		return list;

	/* special pre-processing of list items */
	if (pre_func)
		pre_func(list, num_elems);

	qsort(list, num_elems, sizeof(NODE *), cmp_func); /* shazzam! */

	if (cmp_func == sort_user_func) {
		code = POP_CODE();
		currule = (code + 1)->inrule;   /* restore current rule */ 
		bcfree(code->nexti);            /* Op_stop */
		bcfree(code);                   /* Op_func_call */
	}

	return list;
}


/*
From bonzini@gnu.org  Mon Oct 28 16:05:26 2002
Date: Mon, 28 Oct 2002 13:33:03 +0100
From: Paolo Bonzini <bonzini@gnu.org>
To: arnold@skeeve.com
Subject: Hash function
Message-ID: <20021028123303.GA6832@biancaneve>

Here is the hash function I'm using in GNU Smalltalk.  The scrambling is
needed if you use powers of two as the table sizes.  If you use primes it
is not needed.

To use double-hashing with power-of-two size, you should use the
_gst_hash_string(str, len) as the primary hash and
scramble(_gst_hash_string (str, len)) | 1 as the secondary hash.

Paolo

*/
/*
 * ADR: Slightly modified to work w/in the context of gawk.
 */

static unsigned long
gst_hash_string(const char *str, size_t len, unsigned long hsize, size_t *code)
{
	unsigned long hashVal = 1497032417;    /* arbitrary value */
	unsigned long ret;

	while (len--) {
		hashVal += *str++;
		hashVal += (hashVal << 10);
		hashVal ^= (hashVal >> 6);
	}

	ret = scramble(hashVal);

	if (code != NULL)
		*code = ret;

	if (ret >= hsize)
		ret %= hsize;

	return ret;
}

static unsigned long
scramble(unsigned long x)
{
	if (sizeof(long) == 4) {
		int y = ~x;

		x += (y << 10) | (y >> 22);
		x += (x << 6)  | (x >> 26);
		x -= (x << 16) | (x >> 16);
	} else {
		x ^= (~x) >> 31;
		x += (x << 21) | (x >> 11);
		x += (x << 5) | (x >> 27);
		x += (x << 27) | (x >> 5);
		x += (x << 31);
	}

	return x;
}
