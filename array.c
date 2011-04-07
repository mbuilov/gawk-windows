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
 * 11/2002: We make the constant a variable, so that it can be tweaked
 * via environment variable.
 */

static int AVG_CHAIN_MAX = 2;	/* 11/2002: Modern machines are bigger, cut this down from 10. */

#include "awk.h"

static size_t SUBSEPlen;
static char *SUBSEP;

static NODE *assoc_find(NODE *symbol, NODE *subs, unsigned long hash1);
static void grow_table(NODE *symbol);

static unsigned long gst_hash_string(const char *str, size_t len, unsigned long hsize, size_t *code);
static unsigned long scramble(unsigned long x);
static unsigned long awk_hash(const char *s, size_t len, unsigned long hsize, size_t *code);

unsigned long (*hash)(const char *s, size_t len, unsigned long hsize, size_t *code) = awk_hash;

/* used by sort_maybe_numeric_index() and sort_up_index_number() */
struct sort_num { AWKNUM sort_numbr; NODE *sort_index; };

/* sorting support code */
static qsort_compfunc sort_selection(NODE *, const char *, int);
static size_t sort_match(const char *, size_t, const char *);
static int sort_ignorecase(const char *, size_t, const char *, size_t);
static int sort_cmp_nodes(NODE *, NODE *);

/* qsort callback routines */
static int sort_up_index_string(const void *, const void *);
static int sort_down_index_string(const void *, const void *);
static int sort_up_index_ignrcase(const void *, const void *);
static int sort_down_index_ignrcase(const void *, const void *);
static int sort_up_index_number(const void *, const void *);
static int sort_down_index_number(const void *, const void *);
static int sort_up_value(const void *, const void *);
static int sort_down_value(const void *, const void *);

/* array_init --- possibly temporary function for experimentation purposes */

void
array_init()
{
	const char *val;
	int newval;

	if ((val = getenv("AVG_CHAIN_MAX")) != NULL && isdigit((unsigned char) *val)) {
		for (newval = 0; *val && isdigit((unsigned char) *val); val++)
			newval = (newval * 10) + *val - '0';

		AVG_CHAIN_MAX = newval;
	}

	if ((val = getenv("AWK_HASH")) != NULL && strcmp(val, "gst") == 0)
		hash = gst_hash_string; 
}

/*
 * array_vname --- print the name of the array
 *
 * Returns a pointer to a statically maintained dynamically allocated string.
 * It's appropriate for printing the name once; if the caller wants
 * to save it, they have to make a copy.
 *
 * Setting MAX_LEN to a positive value (eg. 140) would limit the length
 * of the output to _roughly_ that length.
 *
 * If MAX_LEN == 0, which is the default, the whole stack is printed.
 */
#define	MAX_LEN 0

char *
array_vname(const NODE *symbol)
{
	static char *message = NULL;
	
	if (symbol->type != Node_array_ref || symbol->orig_array->type != Node_var_array)
		return symbol->vname;
	else {
		static size_t msglen = 0;
		char *s;
		size_t len;
		int n;
		const NODE *save_symbol = symbol;
		const char *from = _("from %s");

#if (MAX_LEN <= 0) || !defined(HAVE_SNPRINTF)
		/* This is the default branch. */

		/* First, we have to compute the length of the string: */
		len = strlen(symbol->vname) + 2;	/* "%s (" */
		n = 0;
		do {
			symbol = symbol->prev_array;
			len += strlen(symbol->vname);
			n++;
		} while	(symbol->type == Node_array_ref);
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
		 * Thus, `s += sprintf(s, from, name);' is a no-no.
		 */
		sprintf(s, "%s (", symbol->vname);
		s += strlen(s);
		for (;;) {
			symbol = symbol->prev_array;
			sprintf(s, from, symbol->vname);
			s += strlen(s);
			if (symbol->type != Node_array_ref)
				break;
			sprintf(s, ", ");
			s += strlen(s);
		}
		sprintf(s, ")");

#else /* MAX_LEN > 0 */

		/*
		 * The following check fails only on
		 * abnormally_long_variable_name.
		 */
#define PRINT_CHECK \
		if (n <= 0 || n >= len) \
			return save_symbol->vname; \
		s += n; len -= n
#define PRINT(str) \
		n = snprintf(s, len, str); \
		PRINT_CHECK
#define PRINT_vname(str) \
		n = snprintf(s, len, str, symbol->vname); \
		PRINT_CHECK

		if (message == NULL)
			emalloc(message, char *, MAX_LEN, "array_vname");

		s = message;
		len = MAX_LEN;

		/* First, print the vname of the node. */
		PRINT_vname("%s (");

		for (;;) {
			symbol = symbol->prev_array;
			/*
			 * When we don't have enough space and this is not
			 * the last node, shorten the list.
			 */
			if (len < 40 && symbol->type == Node_array_ref) {
				PRINT("..., ");
				symbol = symbol->orig_array;
			}
			PRINT_vname(from);
			if (symbol->type != Node_array_ref)
				break;
			PRINT(", ");
		}
		PRINT(")");

#undef PRINT_CHECK
#undef PRINT
#undef PRINT_vname
#endif /* MAX_LEN <= 0 */

		return message;
	}
}
#undef MAX_LEN

/* make_aname --- construct a sub-array name for multi-dimensional array */

char *
make_aname(NODE *array, NODE *subs)
{
	static char *aname = NULL;
	static size_t aname_len;
	size_t slen;

	slen = strlen(array->vname) + subs->stlen + 6;
	if (aname == NULL) {
		emalloc(aname, char *, slen, "make_aname");
		aname_len = slen;
	} else if (slen > aname_len) {
		erealloc(aname, char *, slen, "make_aname");
		aname_len = slen;
	}
	sprintf(aname, "%s[\"%.*s\"]", array->vname, (int) subs->stlen, subs->stptr);
	return aname;
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
	long len;	/* NOT size_t, which is unsigned! */
	size_t subseplen = 0;
	int i;
	extern NODE **args_array;
	
	if (nargs == 1)
		return POP_STRING();

	if (do_subsep)
		subseplen = SUBSEPlen;

	len = -subseplen;
	for (i = 1; i <= nargs; i++) {
		r = POP();
		if (r->type == Node_var_array) {
			while (--i > 0)
				DEREF(args_array[i]);	/* avoid memory leak */
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(r));
		} 
		args_array[i] = force_string(r);
		len += r->stlen + subseplen;
	}

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

/* ahash_unref --- remove reference to a Node_ahash */

void
ahash_unref(NODE *tmp)
{
	if (tmp == NULL)
		return;

	assert(tmp->type == Node_ahash);
		
	if (tmp->ahname_ref > 1)
		tmp->ahname_ref--;
	else {
		efree(tmp->ahname_str);
		freenode(tmp);
	}
}

/* assoc_clear --- flush all the values in symbol[] before doing a split() */

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
			ahash_unref(bucket);	/* unref() will free the ahname_str */
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

	/*
	 * This is INCREDIBLY ugly, but fast.  We break the string up into
	 * 8 byte units.  On the first time through the loop we get the
	 * "leftover bytes" (strlen % 8).  On every other iteration, we
	 * perform 8 HASHC's so we handle all 8 bytes.  Essentially, this
	 * saves us 7 cmp & branch instructions.  If this routine is
	 * heavily used enough, it's worth the ugly coding.
	 *
	 * OZ's original sdbm hash, copied from Margo Seltzers db package.
	 */

	/*
	 * Even more speed:
	 * #define HASHC   h = *s++ + 65599 * h
	 * Because 65599 = pow(2, 6) + pow(2, 16) - 1 we multiply by shifts
	 */
#define HASHC   htmp = (h << 6);  \
		h = *s++ + htmp + (htmp << 10) - h

	unsigned long htmp;

	h = 0;

#if defined(VAXC)
	/*	
	 * This was an implementation of "Duff's Device", but it has been
	 * redone, separating the switch for extra iterations from the
	 * loop. This is necessary because the DEC VAX-C compiler is
	 * STOOPID.
	 */
	switch (len & (8 - 1)) {
	case 7:		HASHC;
	case 6:		HASHC;
	case 5:		HASHC;
	case 4:		HASHC;
	case 3:		HASHC;
	case 2:		HASHC;
	case 1:		HASHC;
	default:	break;
	}

	if (len > (8 - 1)) {
		size_t loop = len >> 3;
		do {
			HASHC;
			HASHC;
			HASHC;
			HASHC;
			HASHC;
			HASHC;
			HASHC;
			HASHC;
		} while (--loop);
	}
#else /* ! VAXC */
	/* "Duff's Device" for those who can handle it */
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
#endif /* ! VAXC */
	if (code != NULL)
		*code = h;

	if (h >= hsize)
		h %= hsize;
	return h;
}

/* assoc_find --- locate symbol[subs] */

static NODE *				/* NULL if not found */
assoc_find(NODE *symbol, NODE *subs, unsigned long hash1)
{
	NODE *bucket;
	const char *s1_str;
	size_t s1_len;
	NODE *s2;

	for (bucket = symbol->var_array[hash1]; bucket != NULL;
			bucket = bucket->ahnext) {
		/*
		 * This used to use cmp_nodes() here.  That's wrong.
		 * Array indexes are strings; compare as such, always!
		 */
		s1_str = bucket->ahname_str;
		s1_len = bucket->ahname_len;
		s2 = subs;

		if (s1_len == s2->stlen) {
			if (s1_len == 0		/* "" is a valid index */
			    || memcmp(s1_str, s2->stptr, s1_len) == 0)
				return bucket;
		}
	}
	return NULL;
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
	ret = assoc_find(symbol, subs, hash1);
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
		bucket = assoc_find(symbol, subs, hash1);
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
	symbol->var_array[hash1] = bucket;
	return &(bucket->ahvalue);
}

/* do_delete --- perform `delete array[s]' */

/*
 * `symbol' is array
 * `nsubs' is no of subscripts
 */

void
do_delete(NODE *symbol, int nsubs)
{
	unsigned long hash1;
	NODE *bucket, *last;
	NODE *subs;

	assert(symbol->type == Node_var_array);

#define free_subs(n)    do {                                    \
    NODE *s = PEEK(n - 1);                                      \
    if (s->type == Node_val) {                                  \
        (void) force_string(s);	/* may have side effects ? */   \
        DEREF(s);                                               \
    }                                                           \
} while (--n > 0)

	if (nsubs == 0) {	/* delete array */
		assoc_clear(symbol);
		return;
	}

	/* NB: subscripts are in reverse order on stack */
	subs = PEEK(nsubs - 1);
	if (subs->type != Node_val) {
		if (--nsubs > 0)
			free_subs(nsubs);
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(subs));
	}
	(void) force_string(subs);

	last = NULL;	/* shut up gcc -Wall */
	hash1 = 0;	/* ditto */

	if (symbol->var_array != NULL) {
		hash1 = hash(subs->stptr, subs->stlen,
				(unsigned long) symbol->array_size, NULL);
		last = NULL;
		for (bucket = symbol->var_array[hash1]; bucket != NULL;
				last = bucket, bucket = bucket->ahnext) {
			/*
			 * This used to use cmp_nodes() here.  That's wrong.
			 * Array indexes are strings; compare as such, always!
			 */
			const char *s1_str;
			size_t s1_len;
			NODE *s2;

			s1_str = bucket->ahname_str;
			s1_len = bucket->ahname_len;
			s2 = subs;
	
			if (s1_len == s2->stlen) {
				if (s1_len == 0		/* "" is a valid index */
				    || memcmp(s1_str, s2->stptr, s1_len) == 0)
					break;
			}
		}
	} else
		bucket = NULL;	/* The array is empty.  */

	if (bucket == NULL) {
		if (do_lint)
			lintwarn(_("delete: index `%s' not in array `%s'"),
				subs->stptr, array_vname(symbol));
		DEREF(subs);

		/* avoid memory leak, free rest of the subs */
		if (--nsubs > 0)
			free_subs(nsubs);
		return;
	}

	DEREF(subs);
	if (bucket->ahvalue->type == Node_var_array) {
		NODE *r = bucket->ahvalue;
		do_delete(r, nsubs - 1);
		if (r->var_array != NULL || nsubs > 1)
			return;
		/* else
			cleared a sub-array, free the array node
			and the bucket in parent array */
		efree(r->vname);
		freenode(r);
	} else if (--nsubs > 0) {
		/* e.g.: a[1] = 1; delete a[1][1] */
		free_subs(nsubs);
		fatal(_("attempt to use scalar `%s[\"%.*s\"]' as an array"),
					symbol->vname,
					(int) bucket->ahname_len,
					bucket->ahname_str);
	} else
		unref(bucket->ahvalue);

	if (last != NULL)
		last->ahnext = bucket->ahnext;
	else
		symbol->var_array[hash1] = bucket->ahnext;

	ahash_unref(bucket);	/* unref() will free the ahname_str */
	symbol->table_size--;
	if (symbol->table_size <= 0) {
		symbol->table_size = symbol->array_size = 0;
		symbol->flags &= ~ARRAYMAXED;
		if (symbol->var_array != NULL) {
			efree((char *) symbol->var_array);
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
	 * more than twice.  The final size is also chosen to be small
	 * enough so that MS-DOG mallocs can handle it. When things are
	 * very large (> 8K), we just double more or less, instead of
	 * just jumping from 8K to 64K.
	 */
	static const long sizes[] = { 13, 127, 1021, 8191, 16381, 32749, 65497,
				131101, 262147, 524309, 1048583, 2097169,
				4194319, 8388617, 16777259, 33554467, 
				67108879, 134217757, 268435459, 536870923,
				1073741827
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
	if ((n->flags & (NUMCUR|NUMBER)) != 0)
		printf("%s %g p: %p", flags2str(n->flags), n->numbr, n);
	else
		printf("%s %.*s p: %p", flags2str(n->flags), (int) n->stlen, n->stptr, n);
}


static void
indent(int indent_level)
{
	int k;
	for (k = 0; k < indent_level; k++)
		printf("\t");
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

/* dup_table --- duplicate input symbol table "symbol" */

static void
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
						char *aname;
						size_t aname_len;
						getnode(r);
						r->type = Node_var_array;
						aname_len = strlen(newsymb->vname) + chain->ahname_len + 4;
						emalloc(aname, char *, aname_len + 2, "dup_table");
						sprintf(aname, "%s[\"%.*s\"]", newsymb->vname, (int) chain->ahname_len, chain->ahname_str);
						r->vname = aname;
						dup_table(chain->ahvalue, r);
						bucket->ahvalue = r;
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
}

/* merge --- do a merge of two sorted lists */

static NODE *
merge(NODE *left, NODE *right)
{
	NODE *ans, *cur;

	/*
	 * Use sort_cmp_nodes(): see the comment there as to why. That
	 * function will call cmp_nodes() on strings, which means that
	 * IGNORECASE influences the comparison.  This is OK, but it may
	 * be surprising.  This comment serves to remind us that we
	 * know about this and that it's OK.
	 */
	if (sort_cmp_nodes(left->ahvalue, right->ahvalue) <= 0) {
		ans = cur = left;
		left = left->ahnext;
	} else {
		ans = cur = right;
		right = right->ahnext;
	}

	while (left != NULL && right != NULL) {
		if (cmp_nodes(left->ahvalue, right->ahvalue) <= 0) {
			cur->ahnext = left;
			cur = left;
			left  = left->ahnext;
		} else {
			cur->ahnext = right;
			cur = right;
			right = right->ahnext;
		}
	}

	cur->ahnext = (left != NULL ? left : right);

	return ans;
}

/* merge_sort --- recursively sort the left and right sides of a list */

static NODE *
merge_sort(NODE *left, unsigned long size)
{
	NODE *right, *tmp;
	int i, half;

	if (size <= 1)
		return left;

	/* walk down the list, till just one before the midpoint */
	tmp = left;
	half = size / 2;
	for (i = 0; i < half-1; i++)
		tmp = tmp->ahnext;

	/* split the list into two parts */
	right = tmp->ahnext;
	tmp->ahnext = NULL;

	/* sort the left and right parts of the list */
	left  = merge_sort(left,       half);
	right = merge_sort(right, size-half);

	/* merge the two sorted parts of the list */
	return merge(left, right);
}


/*
 * assoc_from_list -- Populate an array with the contents of a list of NODEs, 
 * using increasing integers as the key.
 */

static void
assoc_from_list(NODE *symbol, NODE *list)
{
	NODE *next;
	unsigned long i = 0;
	unsigned long hash1;
	char buf[100];

	for (; list != NULL; list = next) {
		size_t code;

		next = list->ahnext;

		/* make an int out of i++ */
		i++;
		sprintf(buf, "%lu", i);
		assert(list->ahname_str == NULL);
		assert(list->ahname_ref == 1);
		emalloc(list->ahname_str, char *, strlen(buf) + 2, "assoc_from_list");
		list->ahname_len = strlen(buf);
		strcpy(list->ahname_str, buf);

		/* find the bucket where it belongs */
		hash1 = hash(list->ahname_str, list->ahname_len,
				symbol->array_size, & code);
		list->ahcode = code;

		/* link the node into the chain at that bucket */
		list->ahnext = symbol->var_array[hash1];
		symbol->var_array[hash1] = list;
	}
}

/*
 * assoc_sort_inplace --- sort all the values in symbol[], replacing
 * the sorted values back into symbol[], indexed by integers starting with 1.
 */

typedef enum asort_how { VALUE, INDEX } ASORT_TYPE;

static NODE *
assoc_sort_inplace(NODE *symbol, ASORT_TYPE how)
{
	unsigned long i, num;
	NODE *bucket, *next, *list;

	if (symbol->var_array == NULL
	    || symbol->array_size <= 0
	    || symbol->table_size <= 0)
		return make_number((AWKNUM) 0);

	/* build a linked list out of all the entries in the table */
	if (how == VALUE) {
		list = NULL;
		num = 0;
		for (i = 0; i < symbol->array_size; i++) {
			for (bucket = symbol->var_array[i]; bucket != NULL; bucket = next) {
				if (bucket->ahvalue->type == Node_var_array)
					fatal(_("attempt to use array in a scalar context"));
				next = bucket->ahnext;
				if (bucket->ahname_ref == 1) {
					efree(bucket->ahname_str);
					bucket->ahname_str = NULL;
					bucket->ahname_len = 0;
				} else {
					NODE *r;

					getnode(r);
					*r = *bucket;
					ahash_unref(bucket);
					bucket = r;
					bucket->flags |= MALLOC;
					bucket->ahname_ref = 1;
					bucket->ahname_str = NULL;
					bucket->ahname_len = 0;
				}
				bucket->ahnext = list;
				list = bucket;
				num++;
			}
			symbol->var_array[i] = NULL;
		}
	} else {	/* how == INDEX */
		list = NULL;
		num = 0;
		for (i = 0; i < symbol->array_size; i++) {
			for (bucket = symbol->var_array[i]; bucket != NULL; bucket = next) {
				next = bucket->ahnext;

				/* toss old value */
				if (bucket->ahvalue->type == Node_var_array) {
					NODE *r = bucket->ahvalue;
					assoc_clear(r);
					efree(r->vname);
					freenode(r);
				} else
					unref(bucket->ahvalue);

				/* move index into value */
				if (bucket->ahname_ref == 1) {
					bucket->ahvalue = make_str_node(bucket->ahname_str,
								bucket->ahname_len, ALREADY_MALLOCED);
					bucket->ahname_str = NULL;
					bucket->ahname_len = 0;
				} else {
					NODE *r;

					bucket->ahvalue = make_string(bucket->ahname_str, bucket->ahname_len);
					getnode(r);
					*r = *bucket;
					ahash_unref(bucket);
					bucket = r;
					bucket->flags |= MALLOC;
					bucket->ahname_ref = 1;
					bucket->ahname_str = NULL;
					bucket->ahname_len = 0;
				}

				bucket->ahnext = list;
				list = bucket;
				num++;
			}
			symbol->var_array[i] = NULL;
		}
	}

	/*
	 * Sort the linked list of NODEs.
	 * (The especially nice thing about using a merge sort here is that
	 * we require absolutely no additional storage. This is handy if the
	 * array has grown to be very large.)
	 */
	list = merge_sort(list, num);

	/*
	 * now repopulate the original array, using increasing
	 * integers as the key
	 */
	assoc_from_list(symbol, list);

	return make_number((AWKNUM) num);
}

/* asort_actual --- do the actual work to sort the input array */

static NODE *
asort_actual(int nargs, ASORT_TYPE how)
{
	NODE *dest = NULL;
	NODE *array;

	if (nargs == 2) {  /* 2nd optional arg */
		dest = POP_PARAM();
		if (dest->type != Node_var_array) {
			fatal(how == VALUE ?
				_("asort: second argument not an array") :
				_("asorti: second argument not an array"));
		}
	}

	array = POP_PARAM();
	if (array->type != Node_var_array) {
		fatal(how == VALUE ?
			_("asort: first argument not an array") :
			_("asorti: first argument not an array"));
	}

	if (dest != NULL && dest != array) {
		assoc_clear(dest);
		dup_table(array, dest);
		array = dest;
	}

	return assoc_sort_inplace(array, how);
}

/* do_asort --- sort array by value */

NODE *
do_asort(int nargs)
{
	return asort_actual(nargs, VALUE);
}

/* do_asorti --- sort array by index */

NODE *
do_asorti(int nargs)
{
	return asort_actual(nargs, INDEX);
}


/* comp_func --- array index comparison function for qsort, used in debug.c */

int
comp_func(const void *p1, const void *p2)
{
	return sort_up_index_string(p1, p2);
}

/*
 * sort_cmp_nodes --- compare two nodes.  Unlike cmp_nodes(), we don't
 * mix numeric and string comparisons.  Numbers compare as less than
 * strings, which in turn compare as less than sub-arrays.  All
 * sub-arrays compare equal to each other, regardless of their contents.
 */

static int
sort_cmp_nodes(NODE *n1, NODE *n2)
{
	int ret;

	if (n1->type == Node_var_array) {
		/* return 0 if n2 is a sub-array too, else return 1 */
		ret = (n2->type != Node_var_array);
	} else if (n2->type == Node_var_array) {
		ret = -1;	/* n1 (scalar) < n2 (sub-array) */
	} else {
		/*
		 * Both scalars; we can't settle for a regular
		 * MAYBE_NUM comparison because that can cause
		 * problems when strings fall between numbers
		 * whose value makes them be ordered differently
		 * when handled as strings than as numbers.
		 * For example, { 10, 5, "3D" } yields a cycle:
		 * 5 < 10, "10" < "3D", "3D" < "5", so would
		 * produce different sort results depending upon
		 * the order in which comparisons between pairs
		 * of values happen to be performed.  We force
		 * numbers to be less than strings in order to
		 * avoid that: 5 < 10, 10 < "3D", 5 < "3D".
		 */
		/* first resolve any undecided types */
		if (n1->flags & MAYBE_NUM)
			(void) force_number(n1);
		if (n2->flags & MAYBE_NUM)
			(void) force_number(n2);
		/*
		 * If both types are the same, use cmp_nodes();
		 * otherwise, force the numeric value to be less
		 * than the string one.
		 */
		if ((n1->flags & NUMBER) == (n2->flags & NUMBER))
			ret = cmp_nodes(n1, n2);
		else
			ret = (n1->flags & NUMBER) ? -1 : 1;
	}
	return ret;
}

/* sort_ignorecase --- case insensitive string comparison from cmp_nodes() */

static int
sort_ignorecase(const char *s1, size_t l1, const char *s2, size_t l2)
{
	size_t l;
	int ret;

	l = (l1 < l2) ? l1 : l2;
#ifdef MBS_SUPPORT
	if (gawk_mb_cur_max > 1) {
		ret = strncasecmpmbs((const unsigned char *) s1,
				     (const unsigned char *) s2, l);
	} else
#endif
	for (ret = 0; l-- > 0 && ret == 0; s1++, s2++)
		ret = casetable[*(unsigned char *) s1]
		      - casetable[*(unsigned char *) s2];
	if (ret == 0 && l1 != l2)
		ret = (l1 < l2) ? -1 : 1;
	return ret;
}

/*
 * sort_up_index_string --- qsort comparison function; ascending index strings;
 * index strings are distinct within an array, so no comparisons will yield
 * equal and warrant disambiguation
 */

static int
sort_up_index_string(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	const char *str1, *str2;
	size_t len1, len2;
	int ret;

	/* Array indexes are strings and distinct, never equal */
	t1 = *((const NODE *const *) p1);
	t2 = *((const NODE *const *) p2);

	str1 = t1->ahname_str;
	len1 = t1->ahname_len;
	str2 = t2->ahname_str;
	len2 = t2->ahname_len;

	ret = memcmp(str1, str2, (len1 < len2) ? len1 : len2);
	/*
	 * if they compared equal but their lengths differ, the
	 * shorter one is a prefix of the longer and compares as less
	 */
	if (ret == 0 && len1 != len2)
		ret = (len1 < len2) ? -1 : 1;

	/* indices are unique within an array, so result should never be 0 */
	assert(ret != 0);
	return ret;
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

/*
 * sort_up_index_ignrcase --- ascending index string, case insensitive;
 * case insensitive comparison can cause distinct index strings to compare
 * equal, so disambiguation in necessary
 */

static int
sort_up_index_ignrcase(const void *p1, const void *p2)
{
	const NODE *t1, *t2;
	int ret;

	t1 = *((const NODE *const *) p1);
	t2 = *((const NODE *const *) p2);

	ret = sort_ignorecase(t1->ahname_str, t1->ahname_len,
			      t2->ahname_str, t2->ahname_len);

	/*
	 * if case insensitive result is "they're the same",
	 * use case sensitive comparison to force distinct order
	 */
	if (ret == 0)
		ret = sort_up_index_string(p1, p2);
	return ret;
}

/* sort_down_index_ignrcase --- descending index strings, case insensitive */

static int
sort_down_index_ignrcase(const void *p1, const void *p2)
{
	return -sort_up_index_ignrcase(p1, p2);
}

/*
 * sort_up_index_number --- qsort comparison function; ascending index numbers;
 * the args have been built for us by sort_maybe_numeric_index() and point
 * to custom structs rather than to gawk nodes
 */

static int
sort_up_index_number(const void *p1, const void *p2)
{
	const struct sort_num *n1, *n2;
	int ret;

	n1 = *((const struct sort_num *const *) p1);
	n2 = *((const struct sort_num *const *) p2);

	if (n1->sort_numbr < n2->sort_numbr)
		ret = -1;
	else
		ret = (n1->sort_numbr > n2->sort_numbr);

	/* break a tie with the index string itself */
	if (ret == 0)
		return sort_up_index_string((const void *) &n1->sort_index,
					    (const void *) &n2->sort_index);
	return ret;
}

/* sort_down_index_number --- descending index numbers */

static int
sort_down_index_number(const void *p1, const void *p2)
{
	return -sort_up_index_number(p1, p2);
}

/* sort_up_value --- qsort comparison function; ascending values */

static int
sort_up_value(const void *p1, const void *p2)
{
	const NODE *idx1, *idx2;
	NODE *val1, *val2;
	int ret;

	/* we're passed a pair of index (array subscript) nodes */
	idx1 = *(const NODE *const *) p1;
	idx2 = *(const NODE *const *) p2;
	if (idx1->type != Node_ahash || idx2->type != Node_ahash)
		cant_happen();

	/* and we want to compare the element values they refer to */
	val1 = idx1->hvalue;
	val2 = idx2->hvalue;

	ret = sort_cmp_nodes(val1, val2);

	/* disambiguate ties to force a deterministic order */
	if (ret == 0)
		ret = sort_up_index_string(p1, p2);
	return ret;
}

/* sort_down_value --- descending value maybe-nums */

static int
sort_down_value(const void *p1, const void *p2)
{
	return -sort_up_value(p1, p2);
}

/* sort_maybe_numeric_index --- special handling for sorting indices as numbers */

void
sort_maybe_numeric_index(qsort_compfunc func, NODE **list,
			 size_t num_elems, int setup)
{
	static const NODE empty_node;

	size_t i;
	NODE temp_node;
	struct sort_num *fake_node;

	/* if we're not sorting indices by numeric value, do nothing */
	if (func != sort_up_index_number && func != sort_down_index_number)
		return;

	/*
	 * Insert a new struct in front of each index, so there's somewhere
	 * to store its numeric value after that's calculated once, avoiding
	 * having the qsort callback recalculate it for every comparison.
	 * Our caller will call us back to destroy this intermediate array
	 * as soon as qsort() finishes; the rest of gawk won't see it.
	 * We could use--or perhaps mis-use--gawk nodes here, but they'd tie
	 * up quite a bit more memory during the sort.
	 */
	if (setup) {
		temp_node = empty_node;	/* init union fields to 0,NULL */
		for (i = 0; i < num_elems; ++i) {
			emalloc(fake_node, struct sort_num *,
				sizeof (struct sort_num), "sort numeric index");
			/* populate enough fields to satisfy force_number();
			   we don't need to maintain reference counts */
			temp_node.type = Node_val;
			temp_node.stptr = list[i]->ahname_str;
			temp_node.stlen = list[i]->ahname_len;
			temp_node.flags = PERM | STRING | MAYBE_NUM;
			/* insert the extra data in front of our input node */
			fake_node->sort_numbr = force_number(&temp_node);
			fake_node->sort_index = list[i];
			list[i] = (NODE *) fake_node;
		}
	} else {	/* post sort cleanup */
		for (i = 0; i < num_elems; ++i) {
			/* undo the setup manipulations */
			fake_node = (struct sort_num *) list[i];
			list[i] = fake_node->sort_index;
			efree((void *) fake_node);
		}
	}
}

/* sort_match --- compare leading part of a string against a sort option */

static size_t
sort_match(const char *str, size_t slen, const char *target)
{
	const char *endofword;

	/*
	 * STR comes from the user and is SLEN characters long;
	 * if it has spaces then we care about the subset up until
	 * its first space rather than the full length.
	 * We check whether STR is the same as--or proper prefix
	 * of--TARGET.
	 * Accept "by-foo" or "as-foo" as a synonym match for "foo".
	 */
	endofword = strchr(str, ' ');
	if (endofword != NULL)
		slen = (size_t) (endofword - str);
	/* try for exact match */
	if (slen > 0 && strncasecmp(str, target, slen) == 0)
		return slen;
	/* ingore "by-" or "as-" prefix and try again */
	if (slen > 3
	    && (strncasecmp(str, "by-", 3) == 0
		|| strncasecmp(str, "as-", 3) == 0)
	    && strncasecmp(str + 3, target, slen - 3) == 0)
		return slen;
	/* no match */
	return 0;
}

/*
 * sort_selection --- parse user-specified sort ordering
 * ("ascending index" or "value number" and so forth);
 * returns a qsort comparison function
 */

static qsort_compfunc
sort_selection(NODE *r, const char *warn_arg, int default_by_value)
{
	enum sort_bits {
		unrecognized =    0,
		Unsorted     = 0x01,
		Ascending    = 0x02,
		Descending   = 0x04,
		by_Index     = 0x08,
		by_Value     = 0x10,
		as_String    = 0x20,
		as_Number    = 0x40,
		as_Inserted  = 0x80,
		allbitsused  = 0xff
	};
	/*
	 * The documented values are singular, but it's trivial to accept
	 * "index numbers" and "descending values" since we're using a
	 * prefix match.  Latin plural of index is the only complication.
	 */
	static const struct sort_keys {
		const char *const keyword;
		enum  sort_bits keybit;
	} sorts[] = {
		{ "unsorted",   Unsorted    },	/* not part of a three-part key */
		{ "ascending",  Ascending   },	/* ascending vs descending */
		{ "descending", Descending  },
		{ "indexes",    by_Index    },	/* by_Index vs by_Number */
		{ "indices",    by_Index    },	/* synonym for plural match */
		{ "values",     by_Value    },
		{ "strings",    as_String   },	/* as_String vs as_Number */
		{ "numbers",    as_Number   },
		{ "numeric",    as_Number   },	/* synonym; singular only */
		{ "inserted",   as_Inserted },	/* not part of a three-part key */
		{ "insertion",  as_Inserted },	/* another synonym */
	};

	static short warned_unrecognized = FALSE;
	static enum sort_bits prev_invalid = unrecognized;

	char *s;
	size_t l, matchlen;
	unsigned i;
	const char *errfmt;
	enum sort_bits allparts;
	qsort_compfunc sort_func, default_sort_func;

	/* deduce our caller from the args provided */
	if (warn_arg != NULL)		/* for-in statement */
		default_sort_func = (qsort_compfunc) 0;	/* unsorted */
	else if (default_by_value)	/* asort() */
		default_sort_func = sort_up_value;
	else				/* asorti() */
		default_sort_func = ! IGNORECASE ? sort_up_index_string
						: sort_up_index_ignrcase;

	r = force_string(r);
	s = r->stptr;
	l = r->stlen;

	/* treat empty sort-order string as request for default */
	if (l == 0)
		return default_sort_func;

	/* no ordering specification yet */
	allparts = unrecognized;
	/*
	 * Scan through S until length L is exhausted, checking its
	 * starting word against each possible ordering choice and
	 * then skipping past that to get to the next word.  If no
	 * choice matches, an error has been detected.  Duplicate
	 * words are accepted; contradictory ones get noticed and
	 * rejected after the string parsing has completed.
	 */
	while (l > 0) {
		matchlen = 0;	/* lint suppression */
		for (i = 0; i < (sizeof sorts / sizeof *sorts); ++i) {
			matchlen = sort_match(s, l, sorts[i].keyword);
			if (matchlen > 0) {
				allparts |= sorts[i].keybit;
				break;
			}
		}
		if (matchlen == 0) {
			/* failed to match any possible component */
			allparts = unrecognized;
			break;
		}
		/* move past the part just handled */
		s += matchlen, l -= matchlen;
		/* (l > 0) here would accept a trailing space; (l > 1) won't */
		if (l > 1 && *s == ' ')
			++s, --l;
	} /* while (l > 0) */

	/*
	 * If we have one or two parts of a three part key, fill in
	 * whichever parts are missing with default values.  For key
	 * type the default is specified by our caller, for direction
	 * it is Ascending, and for comparison mode it is as_String
	 * but only when sorting by_Index.  When sorting by_Value,
	 * comparison mode is out of the user's control and we don't
	 * need or want to fill in either as_String or as_Number.
	 */
	if ((allparts & (Ascending|Descending|as_String|as_Number)) != 0
	    && (allparts & (Unsorted|by_Index|by_Value|as_Inserted)) == 0)
		allparts |= default_by_value ? by_Value : by_Index;
	if ((allparts & (by_Index|by_Value|as_String|as_Number)) != 0
	    && (allparts & (Unsorted|Ascending|Descending|as_Inserted)) == 0)
		allparts |= Ascending;
	/* by_Value is handled differently from by_Index */
	if ((allparts & (Ascending|Descending|by_Index)) != 0
	    && (allparts & (Unsorted|by_Value|as_String|as_Number|as_Inserted)) == 0)
		allparts |= as_String;

	/*
	 * ALLPARTS is a bit mask of all the user's specified ordering
	 * choices; partial specifications have been extended into full
	 * three part key.  Valid combinations yield a qsort(3) comparison
	 * routine result.  Invalid ones are rejected and yield default
	 * sort, with an error message for asort()/asorti(), a warning
	 * for for-in statement if this is a different invalid combination
	 * than the most recently warned about one, or a one-time warning
	 * if this is an unrecognized ordering specification and we're
	 * running in lint checking mode.
	 */
	sort_func = default_sort_func;
	errfmt = NULL;
	switch (allparts) {
	case Unsorted:
		sort_func = (qsort_compfunc) 0;
		break;
	case Ascending|by_Index|as_String:
		sort_func = ! IGNORECASE ? sort_up_index_string
					: sort_up_index_ignrcase;
		break;
	case Descending|by_Index|as_String:
		sort_func = ! IGNORECASE ? sort_down_index_string
					: sort_down_index_ignrcase;
		break;
	case Ascending|by_Index|as_Number:
		sort_func = sort_up_index_number;
		break;
	case Descending|by_Index|as_Number:
		sort_func = sort_down_index_number;
		break;
	case Ascending|by_Value:
		sort_func = sort_up_value;
		break;
	case Descending|by_Value:
		sort_func = sort_down_value;
		break;
	/* for value sorts, distinction between string and number is not
	   allowed; they're always ordered by basic awk MAYBE_NUM comparison */
	case Ascending|by_Value|as_String:
	case Descending|by_Value|as_String:
		if (errfmt == NULL)
			errfmt = _("%s: sorting by value can't be forced to use string comparison");
		/* FALL THROUGH */
	case Ascending|by_Value|as_Number:
	case Descending|by_Value|as_Number:
		if (errfmt == NULL)
			errfmt = _("%s: sorting by value can't be forced to use numeric comparison");
		/* FALL THROUGH */
	/* gawk doesn't maintain the data necessary to sort by
	   order of insertion */
	case as_Inserted:
		if (errfmt == NULL)
			errfmt = _("%s: sort ordering \"as-inserted\" is not implemented");
		/* FALL THROUGH */
	default:
		/* sort_func still has default value */
		if (errfmt == NULL)
			errfmt = (allparts == unrecognized)
				? _("%s: sort specification is not recognized")
				: _("%s: sort specification is invalid");
		if (warn_arg == NULL) {
			/* asort()/asorti() bad sort specification argument */
			error(errfmt, default_by_value ? "asort" : "asorti");
		} else if (allparts != unrecognized) {
			/* invalid combination of valid ordering components */
			if (allparts != prev_invalid)
				warning(errfmt, warn_arg);
			prev_invalid = allparts;
		} else if (do_lint) {
			/* unrecognized ordering specification string */              
			if (! warned_unrecognized)
				lintwarn(errfmt, warn_arg);
			warned_unrecognized = TRUE;
		}
		break;
	}
	return sort_func;
}

/*
 * sorted_in --- fetch value of PROCINFO["sorted_in"] and use sort_selection()
 * to parse it; controls whether and how traversal of ``for (index in array)''
 * should be sorted; returns a qsort comparison function
 */

qsort_compfunc
sorted_in(void)		/* called from r_interpret(eval.c) */
{
	static NODE *sorted_str = NULL;
	static short warned_extension = FALSE;

	NODE *r;

	/* if there's no PROCINFO[], there's no ["sorted_in"], so no sorting */
	if (PROCINFO_node == NULL)
		return (qsort_compfunc) 0;

	if (sorted_str == NULL)		/* do this once */
		sorted_str = make_string("sorted_in", 9);

	r = (NODE *) NULL;
	if (in_array(PROCINFO_node, sorted_str))
		r = *assoc_lookup(PROCINFO_node, sorted_str, TRUE);
	/* if there's no PROCINFO["sorted_in"], there's no sorting */
	if (r == NULL)
		return (qsort_compfunc) 0;

	/* we're going to make use of PROCINFO["sorted_in"] */
	if (do_lint && ! warned_extension) {
		warned_extension = TRUE;
		lintwarn(_("sorted array traversal is a gawk extension"));
	}

	return sort_selection(r, "`PROCINFO[\"sorted_in\"]'", FALSE);
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
