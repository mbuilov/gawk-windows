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

/* qsort comparison function */
static int sort_up_index_string(const void *, const void *);
static int sort_down_index_string(const void *, const void *);
static int sort_up_index_number(const void *, const void *);
static int sort_down_index_number(const void *, const void *);
static int sort_up_value_string(const void *, const void *);
static int sort_down_value_string(const void *, const void *);
static int sort_up_value_number(const void *, const void *);
static int sort_down_value_number(const void *, const void *);


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
	 *
	 * 4/2011: Force the results to 32 bits, to get the same
	 * result on both 32- and 64-bit systems. This may be a
	 * bad idea.
	 */
#define HASHC   htmp = (h << 6);  \
		h = *s++ + htmp + (htmp << 10) - h ; \
		htmp &= 0xFFFFFFFF; \
		h &= 0xFFFFFFFF;

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
	NODE *r, *subs, *sort_str;
	NODE **list, **ptr;
	static char buf[102];
	unsigned long num_elems, i;

	if (nargs == 3)  /* 3rd optional arg */
		sort_str = POP_STRING();
	else
		sort_str = Nnull_string;	/* "" => default sorting */
 
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

	num_elems = array->table_size;
	if (num_elems == 0 || array->var_array == NULL) {	/* source array is empty */
		if (dest != NULL && dest != array)
			assoc_clear(dest);
		return make_number((AWKNUM) 0);
	}

	list = assoc_list(array, sort_str, ctxt);
	DEREF(sort_str);

	/* Must not assoc_clear() the source array before constructing
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
	}

	subs = make_str_node(buf, 100, ALREADY_MALLOCED);   /* fake it */
	subs->flags &= ~MALLOC;		/* safety */
	for (i = 1, ptr = list; i <= num_elems; i++) {
		sprintf(buf, "%lu", i);
		subs->stlen = strlen(buf);
		r = *ptr++;
		if (ctxt == ASORTI) {
			/* We want the indices of the source array as values
			 * of the 'result' array.
			 */

			*assoc_lookup(result, subs, FALSE) =
						make_string(r->ahname_str, r->ahname_len);
		} else {
			NODE *val;

			/* We want the values of the source array. */

			val = r->ahvalue;
			if (val->type == Node_val)
				*assoc_lookup(result, subs, FALSE) = dupnode(val);
			else {
				const char *arr_name = make_aname(result, subs);
				NODE *arr;

				/* there isn't any reference counting for subarrays, so
				 * recursively copy subarrays using dup_table().
				 */
				getnode(arr);
				arr->type = Node_var_array;
				arr->var_array = NULL;
				arr->vname = estrdup(arr_name, strlen(arr_name));
				*assoc_lookup(result, subs, FALSE) = dup_table(val, arr);
			}
		}

		ahash_unref(r);
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

/* cmp_string --- compare two strings; logic similar to cmp_nodes() in eval.c
 *	except the extra case-sensitive comparison when the case-insensitive result
 *	is a match.
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
		/* if case insensitive result is "they're the same",
		 * use case sensitive comparison to force distinct order
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

	/* Array indexes are strings */
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

	if (n1->type == Node_var_array && n2->type == Node_val)
		return 1;	/* n1 is more */
	if (n1->type == Node_val && n2->type == Node_var_array)
		return -1;	/* n1 is less */

 	if (n1->type == Node_var_array && n2->type == Node_var_array) {
		/* sub-array names contain respective indices, effectively resulting
		 * in an index-based (in parent array) ordering.
		 */
		return strcmp(n1->vname, n2->vname);
	}

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

	if (n1->type == Node_var_array && n2->type == Node_val)
		return 1;	/* n1 is more */
	if (n1->type == Node_val && n2->type == Node_var_array)
		return -1;	/* n1 is less */
	if (n1->type == Node_var_array && n2->type == Node_var_array) {
		/* sub-array names contain respective indices, effectively resulting
		 * in an index-based (in parent array) ordering.
		 */

		return strcmp(n1->vname, n2->vname);
	}

	/* n1 and n2 both Node_val, and force_number'ed */
	if (n1->numbr < n2->numbr)
		ret = -1;
	else
		ret = (n1->numbr > n2->numbr);

	if (ret != 0)
		return ret;

	/* break a tie using string comparison. First, make sure both
	 * n1 and n2 have string values.
	 */
	(void) force_string(n1);
	(void) force_string(n2);
	return cmp_string(n1, n2);
}


/* sort_down_value_string --- descending value number */

static int
sort_down_value_number(const void *p1, const void *p2)
{
	return -sort_up_value_number(p1, p2);
}


/*
 * sort_selection --- parse user-specified sort specification;
 * returns an index into sort_funcs table located in assoc_list().
 */

static int
sort_selection(NODE *sort_str, SORT_CTXT sort_ctxt)
{
	/* first 4 bits used to calculate index into sort_funcs table in assoc_list(),
	 * next 4 bits for grouping individual components. Note that "Unsorted"
	 * belongs to all the groups.
	 */

	enum sort_bits {
		Unrecognized = 0xFF,	/* not part of a sort phrase */
		Unsorted     = 0xF8,
		Ascending    = 0x40,
		Descending   = 0x44,
		by_Index     = 0x20,
		by_Value     = 0x22,
		as_String    = 0x10,
		as_Number    = 0x11
	};

#define INDEX_MASK	0x0F
#define GROUP_MASK	0xF0

	/*
	 * The documented values are singular, but it's trivial to accept
	 * "index numbers" and "descending values" since we're using a
	 * prefix match.  Latin plural of index is the only complication.
	 */
	static const struct sort_keys {
		const char *const keyword;
		const size_t keyword_len;
		enum  sort_bits keybit;
	} sorts[] = {
		{ "unsorted",   8, Unsorted },
		{ "ascending",  9, Ascending },	/* ascending vs descending */
		{ "descending", 10, Descending  },
		{ "indexes", 7, by_Index  },	/* by_Index vs by_Value */
		{ "indices", 7, by_Index  },	/* synonym for plural match */
		{ "values", 6,  by_Value  },
		{ "strings", 7, as_String },	/* as_String vs as_Number */
		{ "numbers", 7, as_Number },
		{ "numeric", 7, as_Number },	/* synonym; singular only */
	};
	static int num_keys = sizeof(sorts) / sizeof(sorts[0]);
	int num_words, i;
	char *word, *s;
	size_t word_len;
	enum sort_bits allparts, bval;

	if (sort_str == NULL) {
		assert(sort_ctxt == SORTED_IN);
		return (Unsorted & INDEX_MASK);		/* no sorting */
	}

	(void) force_string(sort_str);
	sort_str->stptr[sort_str->stlen] = '\0';	/* safety */

	/* Initialize with the context-sensitive defaults; Note that group-bits aren't
	 * copied, doing so will prevent the user from explicitely specifying the defaults.
	 */
 
	if (sort_ctxt == ASORT)
		allparts = (Ascending|by_Value|as_String) & INDEX_MASK;
	else
		allparts = (Ascending|by_Index|as_String) & INDEX_MASK;

	num_words = 0;

	for (s = sort_str->stptr; *s != '\0'; ) {
		/* skip leading spaces */
		while (*s == ' ')
			s++;
		if (*s == '\0')
			break;
		word = s;
		/* find the end of the word */
		while (*s && *s != ' ')
			s++;
		word_len = (size_t) (s - word);

		if (++num_words > 3)	/* too many words in phrase */
			goto un_recognized;

		bval = Unrecognized;
		for (i = 0; i < num_keys; i++) {
			if (word_len <= sorts[i].keyword_len
       		 	      && strncasecmp(sorts[i].keyword, word, word_len) == 0) {
				bval = sorts[i].keybit;
				break;
			}
		}
		  		
		if (bval == Unrecognized         /* invalid word in phrase */
			||	(sort_ctxt == ASORT
					&& (bval == by_Index || bval == Unsorted)
				)                        /* "index" used in phrase for asort etc. */
			||	(sort_ctxt == ASORTI
					&& (bval == by_Value || bval == Unsorted)
				)                        /* "value" used in phrase for asorti etc. */
			||	((allparts & bval) & GROUP_MASK
				)                        /* invalid grouping of words e.g. "str num" */
		)
			goto un_recognized;

		allparts |= bval;
	}

	/* num_words <= 3 */
	return (allparts & INDEX_MASK);

un_recognized:
	switch (sort_ctxt) {
	case ASORT:
		fatal(_("asort: invalid sort specification `%s'"), sort_str->stptr);
	case ASORTI:
		fatal(_("asorti: invalid sort specification `%s'"), sort_str->stptr);

	case SORTED_IN:
		/* fall through */
	default:
		if (do_lint) {
			static NODE *warned_str = NULL;

			/* warning for each UNIQUE unrecognized sort_str specification */              
			if (warned_str == NULL || ! STREQ(warned_str->stptr, sort_str->stptr)) {
				lintwarn(_("PROCINFO[\"sorted_in\"]: invalid sort specification `%s'"),
						sort_str->stptr);
				unref(warned_str);	/* unref(NULL) is OK */ 
				warned_str = dupnode(sort_str);
			}
		}
		break;
	}
	return (Unsorted & INDEX_MASK);

#undef INDEX_MASK
#undef GROUP_MASK
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

		/* Kludge: the use of NUMCUR flag for a Node_ahash is only to detect
		 * multiple conversion attempts. Also, the flag value of a Node_ahash
		 * isn't used anywhere else in the gawk source. 
		 * Maybe a new flag other than NUMCUR should be used ?
		 */

		if ((r->flags & NUMCUR) != 0)	/* once in a lifetime is more than enough */
			continue;
		temp_node.type = Node_val;
		temp_node.stptr = r->ahname_str;
		temp_node.stlen = r->ahname_len;
		temp_node.flags = 0;	/* only interested in the return value of r_force_number */
		r->ahname_num = r_force_number(&temp_node);
		r->flags |= NUMCUR;
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
assoc_list(NODE *array, NODE *sort_str, SORT_CTXT sort_ctxt)
{
	typedef void (*qsort_prefunc)(NODE **, size_t);
	typedef int (*qsort_compfunc)(const void *, const void *);

	static const struct qsort_funcs {
		qsort_compfunc comp_func;
		qsort_prefunc pre_func;		/* pre-processing of list items */
	} sort_funcs[] = {
		{ sort_up_index_string,		0                       },	/* ascending index string */
		{ sort_up_index_number,		sort_force_index_number },	/* ascending index number */
		{ sort_up_value_string,		sort_force_value_string },	/* ascending value string */
		{ sort_up_value_number,		sort_force_value_number	},	/* ascending value number */
		{ sort_down_index_string,	0                       },	/* descending index string */ 
		{ sort_down_index_number,	sort_force_index_number },	/* descending index number */	
		{ sort_down_value_string,	sort_force_value_string },	/* descending value string */ 
		{ sort_down_value_number,	sort_force_value_number },	/* descending value number */
		{ 0,	0 }                                            	/* unsorted */
	};
	int qi;
	NODE **list;
	NODE *r;
	size_t num_elems, i, j;
	qsort_compfunc cmp_func;
	qsort_prefunc pre_func;

	num_elems = array->table_size;
	assert(num_elems > 0);

	qi = sort_selection(sort_str, sort_ctxt);

	/* allocate space for array; the extra space is used in for(i in a) opcode (eval.c) */
	emalloc(list, NODE **, (num_elems + 1) * sizeof(NODE *), "assoc_list");

	/* populate it */
	for (i = j = 0; i < array->array_size; i++)
		for (r = array->var_array[i]; r != NULL; r = r->ahnext)
			list[j++] = ahash_dupnode(r);
	list[num_elems] = NULL;

	cmp_func = sort_funcs[qi].comp_func;
	if (! cmp_func)	/* unsorted */	
		return list;

	/* special pre-processing of list items */
	pre_func = sort_funcs[qi].pre_func;
	if (pre_func)
		pre_func(list, num_elems);

	qsort(list, num_elems, sizeof(NODE *), cmp_func); /* shazzam! */
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
