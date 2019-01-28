/*
 * str_array.c - routines for associative arrays of string indices.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2013, 2016, 2017, 2018, 2019,
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
 * 11/2002: Modern machines are bigger, cut this down from 10.
 */

static size_t STR_CHAIN_MAX = 2;

extern FILE *output_fp;
extern void indent(int indent_level);

static NODE **str_array_init(NODE *symbol, NODE *subs);
static NODE **str_lookup(NODE *symbol, NODE *subs);
static NODE **str_exists(NODE *symbol, NODE *subs);
static NODE **str_clear(NODE *symbol, NODE *subs);
static NODE **str_remove(NODE *symbol, NODE *subs);
static NODE **str_list(NODE *symbol, NODE *subs);
static NODE **str_copy(NODE *symbol, NODE *newsymb);
static NODE **str_dump(NODE *symbol, NODE *ndump);

const array_funcs_t str_array_func = {
	"str",
	str_array_init,
	(afunc_t) 0,
	str_lookup,
	str_exists,
	str_clear,
	str_remove,
	str_list,
	str_copy,
	str_dump,
	(afunc_t) 0,
};

static NODE **env_remove(NODE *symbol, NODE *subs);
static NODE **env_store(NODE *symbol, NODE *subs);
static NODE **env_clear(NODE *symbol, NODE *subs);

/* special case for ENVIRON */
const array_funcs_t env_array_func = {
	"env",
	str_array_init,
	(afunc_t) 0,
	str_lookup,
	str_exists,
	env_clear,
	env_remove,
	str_list,
	str_copy,
	str_dump,
	env_store,
};

static inline NODE **str_find(NODE *symbol, NODE *s1, size_t code1, unsigned long hash1);
static void grow_table(NODE *symbol);

static unsigned long gst_hash_string(const char *str, size_t len, unsigned long hsize, size_t *code);
static unsigned long scramble(unsigned long x);
static unsigned long awk_hash(const char *s, size_t len, unsigned long hsize, size_t *code);

unsigned long (*hash)(const char *s, size_t len, unsigned long hsize, size_t *code) = awk_hash;


/* str_array_init --- array initialization routine */

static NODE **
str_array_init(NODE *symbol ATTRIBUTE_UNUSED, NODE *subs ATTRIBUTE_UNUSED)
{
	if (symbol == NULL) {		/* first time */
		long newval;
		const char *val;

		/* check relevant environment variables */
		if ((newval = getenv_long("STR_CHAIN_MAX")) > 0)
			STR_CHAIN_MAX = newval;
		if ((val = getenv("AWK_HASH")) != NULL && strcmp(val, "gst") == 0)
			hash = gst_hash_string;
	} else
		null_array(symbol);

	return & success_node;
}


/*
 * str_lookup:
 * Find SYMBOL[SUBS] in the assoc array.  Install it with value "" if it
 * isn't there. Returns a pointer ala get_lhs to where its value is stored.
 *
 * SYMBOL is the address of the node (or other pointer) being dereferenced.
 * SUBS is a number or string used as the subscript.
 */

static NODE **
str_lookup(NODE *symbol, NODE *subs)
{
	unsigned long hash1;
	NODE **lhs;
	BUCKET *b;
	size_t code1;

	subs = force_string(subs);

	if (symbol->buckets == NULL)
		grow_table(symbol);
	hash1 = hash(subs->stptr, subs->stlen,
			(unsigned long) symbol->array_size, & code1);
	if ((lhs = str_find(symbol, subs, code1, hash1)) != NULL)
		return lhs;

	/* It's not there, install it. */
	/* first see if we would need to grow the array, before installing */

	symbol->table_size++;
	if ((symbol->flags & ARRAYMAXED) == 0
			&& (symbol->table_size / symbol->array_size) > STR_CHAIN_MAX) {
		grow_table(symbol);
		/* have to recompute hash value for new size */
		hash1 = code1 % (unsigned long) symbol->array_size;
	}


	/*
	 * Repeat after me: "Array indices are always strings."
	 * "Array indices are always strings."
	 * "Array indices are always strings."
	 * "Array indices are always strings."
	 * ....
	 */
	// Special cases:
	// 1. The string was generated using CONVFMT.
	// 2. The string was from an unassigned variable.
	// 3. The string was from an unassigned field.
	if (   subs->stfmt != STFMT_UNUSED
	    || subs == Nnull_string
	    || (subs->flags & NULL_FIELD) != 0) {
		NODE *tmp;

		/*
		 * Need to freeze this string value --- it must never
		 * change, no matter what happens to the value
		 * that created it or to CONVFMT, etc.; So, get
		 * a private copy.
		 */

		tmp = make_string(subs->stptr, subs->stlen);

		/*
		* Set the numeric value for the index if it's  available. Useful
		* for numeric sorting by index.  Do this only if the numeric
		* value is available, instead of all the time, since doing it
		* all the time is a big performance hit for something that may
		* never be used.
		*/

		if ((subs->flags & (MPFN|MPZN|NUMCUR)) == NUMCUR) {
			tmp->numbr = subs->numbr;
			tmp->flags |= NUMCUR;
		}
		subs = tmp;
	} else {
		/* string value already "frozen" */

		subs = dupnode(subs);
	}

	getbucket(b);
	b->ahnext = symbol->buckets[hash1];
	symbol->buckets[hash1] = b;
	b->ahname = subs;
	b->ahname_str = subs->stptr;
	b->ahname_len = subs->stlen;
	b->ahvalue = dupnode(Nnull_string);
	b->ahcode = code1;
	return & (b->ahvalue);
}

/* str_exists --- test whether the array element symbol[subs] exists or not,
 * 		return pointer to value if it does.
 */

static NODE **
str_exists(NODE *symbol, NODE *subs)
{
	unsigned long hash1;
	size_t code1;

	if (symbol->table_size == 0)
		return NULL;

	subs = force_string(subs);
	hash1 = hash(subs->stptr, subs->stlen, (unsigned long) symbol->array_size, & code1);
	return str_find(symbol, subs, code1, hash1);
}

/* str_clear --- flush all the values in symbol[] */

static NODE **
str_clear(NODE *symbol, NODE *subs ATTRIBUTE_UNUSED)
{
	unsigned long i;
	BUCKET *b, *next;
	NODE *r;

	for (i = 0; i < symbol->array_size; i++) {
		for (b = symbol->buckets[i]; b != NULL; b = next) {
			next = b->ahnext;
			r = b->ahvalue;
			if (r->type == Node_var_array) {
				assoc_clear(r);	/* recursively clear all sub-arrays */
				efree(r->vname);
				freenode(r);
			} else
				unref(r);
			unref(b->ahname);
			freebucket(b);
		}
		symbol->buckets[i] = NULL;
	}

	if (symbol->buckets != NULL)
		efree(symbol->buckets);
	symbol->ainit(symbol, NULL);	/* re-initialize symbol */
	return NULL;
}


/* str_remove --- If SUBS is already in the table, remove it. */

static NODE **
str_remove(NODE *symbol, NODE *subs)
{
	unsigned long hash1;
	BUCKET *b, *prev;
	NODE *s2;
	size_t s1_len;

	if (symbol->table_size == 0)
		return NULL;

	s2 = force_string(subs);
	hash1 = hash(s2->stptr, s2->stlen, (unsigned long) symbol->array_size, NULL);

	for (b = symbol->buckets[hash1], prev = NULL; b != NULL;
				prev = b, b = b->ahnext) {

		/* Array indexes are strings; compare as such, always! */
		s1_len = b->ahname_len;

		if (s1_len != s2->stlen)
			continue;
		if (s1_len == 0		/* "" is a valid index */
			    || memcmp(b->ahname_str, s2->stptr, s1_len) == 0) {
			/* item found */

			unref(b->ahname);
			if (prev != NULL)
				prev->ahnext = b->ahnext;
			else
				symbol->buckets[hash1] = b->ahnext;

			/* delete bucket */
			freebucket(b);

			/* one less element in array */
			if (--symbol->table_size == 0) {
				if (symbol->buckets != NULL)
					efree(symbol->buckets);
				symbol->ainit(symbol, NULL);	/* re-initialize symbol */
			}

			return & success_node;	/* return success */
		}
	}

	return NULL;
}


/* str_copy --- duplicate input array "symbol" */

static NODE **
str_copy(NODE *symbol, NODE *newsymb)
{
	BUCKET **old, **new, **pnew;
	BUCKET *chain, *newchain;
	unsigned long cursize, i;

	assert(symbol->table_size > 0);

	/* find the current hash size */
	cursize = symbol->array_size;

	/* allocate new table */
	ezalloc(new, BUCKET **, cursize * sizeof(BUCKET *), "str_copy");

	old = symbol->buckets;

	for (i = 0; i < cursize; i++) {
		for (chain = old[i], pnew = & new[i]; chain != NULL;
				chain = chain->ahnext
		) {
			NODE *oldval, *newsubs;

			getbucket(newchain);

			/*
			 * copy the corresponding name and
			 * value from the original input list
			 */

			newsubs = newchain->ahname = dupnode(chain->ahname);
			newchain->ahname_str = newsubs->stptr;
			newchain->ahname_len = newsubs->stlen;

			oldval = chain->ahvalue;
			if (oldval->type == Node_val)
				newchain->ahvalue = dupnode(oldval);
			else {
				NODE *r;

				r = make_array();
				r->vname = estrdup(oldval->vname, strlen(oldval->vname));
				r->parent_array = newsymb;
				newchain->ahvalue = assoc_copy(oldval, r);
			}
			newchain->ahcode = chain->ahcode;

			*pnew = newchain;
			newchain->ahnext = NULL;
			pnew = & newchain->ahnext;
		}
	}

	newsymb->table_size = symbol->table_size;
	newsymb->buckets = new;
	newsymb->array_size = cursize;
	newsymb->flags = symbol->flags;
	return NULL;
}


/* str_list --- return a list of array items */

static NODE**
str_list(NODE *symbol, NODE *t)
{
	NODE **list;
	NODE *subs, *val;
	BUCKET *b;
	unsigned long num_elems, list_size, i, k = 0;
	int elem_size = 1;
	assoc_kind_t assoc_kind;

	if (symbol->table_size == 0)
		return NULL;

	assoc_kind = (assoc_kind_t) t->flags;
	if ((assoc_kind & (AINDEX|AVALUE)) == (AINDEX|AVALUE))
		elem_size = 2;

	/* allocate space for array */
	num_elems = symbol->table_size;
	if ((assoc_kind & (AINDEX|AVALUE|ADELETE)) == (AINDEX|ADELETE))
		num_elems = 1;
	list_size =  elem_size * num_elems;

	emalloc(list, NODE **, list_size * sizeof(NODE *), "str_list");

	/* populate it */

	for (i = 0; i < symbol->array_size; i++) {
		for (b = symbol->buckets[i]; b != NULL;	b = b->ahnext) {
			/* index */
			subs = b->ahname;
			if ((assoc_kind & AINUM) != 0)
				(void) force_number(subs);
			list[k++] = dupnode(subs);

			/* value */
			if ((assoc_kind & AVALUE) != 0) {
				val = b->ahvalue;
				if (val->type == Node_val) {
					if ((assoc_kind & AVNUM) != 0)
						(void) force_number(val);
					else if ((assoc_kind & AVSTR) != 0)
						val = force_string(val);
				}
				list[k++] = val;
			}
			if (k >= list_size)
				return list;
		}
	}
	return list;
}


/* str_kilobytes --- calculate memory consumption of the assoc array */

AWKNUM
str_kilobytes(NODE *symbol)
{
	unsigned long bucket_cnt;
	AWKNUM kb;

	bucket_cnt = symbol->table_size;

	/* This does not include extra memory for indices with stfmt != STFMT_UNUSED */
	kb = (((AWKNUM) bucket_cnt) * sizeof (BUCKET) +
		((AWKNUM) symbol->array_size) * sizeof (BUCKET *)) / 1024.0;
	return kb;
}


/* str_dump --- dump array info */

static NODE **
str_dump(NODE *symbol, NODE *ndump)
{
#define HCNT	31

	int indent_level;
	unsigned long i, bucket_cnt;
	BUCKET *b;
	static size_t hash_dist[HCNT + 1];

	indent_level = ndump->alevel;

	if ((symbol->flags & XARRAY) == 0)
		fprintf(output_fp, "%s `%s'\n",
				(symbol->parent_array == NULL) ? "array" : "sub-array",
				array_vname(symbol));
	indent_level++;
	indent(indent_level);
	fprintf(output_fp, "array_func: str_array_func\n");
	if (symbol->flags != 0) {
		indent(indent_level);
		fprintf(output_fp, "flags: %s\n", flags2str(symbol->flags));
	}
	indent(indent_level);
	fprintf(output_fp, "STR_CHAIN_MAX: %lu\n", (unsigned long) STR_CHAIN_MAX);
	indent(indent_level);
	fprintf(output_fp, "array_size: %lu\n", (unsigned long) symbol->array_size);
	indent(indent_level);
	fprintf(output_fp, "table_size: %lu\n", (unsigned long) symbol->table_size);
	indent(indent_level);
	fprintf(output_fp, "Avg # of items per chain: %.2g\n",
				((AWKNUM) symbol->table_size) / symbol->array_size);

	indent(indent_level);
	fprintf(output_fp, "memory: %.2g kB\n", str_kilobytes(symbol));

	/* hash value distribution */

	memset(hash_dist, '\0', (HCNT + 1) * sizeof(size_t));
	for (i = 0; i < symbol->array_size; i++) {
		bucket_cnt = 0;
		for (b = symbol->buckets[i]; b != NULL;	b = b->ahnext)
			bucket_cnt++;
		if (bucket_cnt >= HCNT)
			bucket_cnt = HCNT;
		hash_dist[bucket_cnt]++;
	}

	indent(indent_level);
	fprintf(output_fp, "Hash distribution:\n");
	indent_level++;
	for (i = 0; i <= HCNT; i++) {
		if (hash_dist[i] > 0) {
			indent(indent_level);
			if (i == HCNT)
				fprintf(output_fp, "[>=%lu]:%lu\n",
					(unsigned long) HCNT, (unsigned long) hash_dist[i]);
			else
				fprintf(output_fp, "[%lu]:%lu\n",
					(unsigned long) i, (unsigned long) hash_dist[i]);
		}
	}
	indent_level--;

	/* dump elements */

	if (ndump->adepth >= 0) {
		const char *aname;

		fprintf(output_fp, "\n");
		aname = make_aname(symbol);
		for (i = 0; i < symbol->array_size; i++) {
			for (b = symbol->buckets[i]; b != NULL;	b = b->ahnext)
				assoc_info(b->ahname, b->ahvalue, ndump, aname);
		}
	}

	return NULL;

#undef HCNT
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


/* str_find --- locate symbol[subs] */

static inline NODE **
str_find(NODE *symbol, NODE *s1, size_t code1, unsigned long hash1)
{
	BUCKET *b;
	size_t s2_len;

	for (b = symbol->buckets[hash1]; b != NULL; b = b->ahnext) {
		/*
		 * This used to use cmp_nodes() here.  That's wrong.
		 * Array indexes are strings; compare as such, always!
	 	 */
		s2_len = b->ahname_len;

		if (code1 == b->ahcode
			&& s1->stlen == s2_len
			&& (s2_len == 0		/* "" is a valid index */
				|| memcmp(s1->stptr, b->ahname_str, s2_len) == 0)
		)
			return & (b->ahvalue);
	}
	return NULL;
}


/* grow_table --- grow a hash table */

static void
grow_table(NODE *symbol)
{
	BUCKET **old, **new;
	BUCKET *chain, *next;
	int i, j;
	unsigned long oldsize, newsize, k;
	unsigned long hash1;

	/*
	 * This is an array of primes. We grow the table by an order of
	 * magnitude each time (not just doubling) so that growing is a
	 * rare operation. We expect, on average, that it won't happen
	 * more than twice.  The final size is also chosen to be small
	 * enough so that MS-DOG mallocs can handle it. When things are
	 * very large (> 8K), we just double more or less, instead of
	 * just jumping from 8K to 64K.
	 */

	static const unsigned long sizes[] = {
		13, 127, 1021, 8191, 16381, 32749, 65497,
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
	ezalloc(new, BUCKET **, newsize * sizeof(BUCKET *), "grow_table");

	old = symbol->buckets;
	symbol->buckets = new;
	symbol->array_size = newsize;

	/* brand new hash table, set things up and return */
	if (old == NULL) {
		symbol->table_size = 0;
		return;
	}

	/* old hash table there, move stuff to new, free old */

	/*
	 * note that symbol->table_size does not change if an old array,
	 * and is explicitly set to 0 if a new one.
	 */

	for (k = 0; k < oldsize; k++) {
		for (chain = old[k]; chain != NULL; chain = next) {
			next = chain->ahnext;
			hash1 = chain->ahcode % newsize;

			/* remove from old list, add to new */
			chain->ahnext = new[hash1];
			new[hash1] = chain;
		}
	}
	efree(old);
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

/* env_remove --- for ENVIRON, remove value from real environment */

static NODE **
env_remove(NODE *symbol, NODE *subs)
{
	NODE **val = str_remove(symbol, subs);
	char save;

	if (val != NULL) {
		str_terminate(subs, save);
		(void) unsetenv(subs->stptr);
		str_restore(subs, save);
	}

	return val;
}

/* env_clear --- clear out the environment when ENVIRON is deleted */

static NODE **
env_clear(NODE *symbol, NODE *subs)
{
	extern char **environ;
	NODE **val = str_clear(symbol, subs);

	environ = NULL;	/* ZAP! */

	/* str_clear zaps the vtable, reset it */
	symbol->array_funcs = & env_array_func;

	return val;
}

/* env_store --- post assign function for ENVIRON, put new value into env */

static NODE **
env_store(NODE *symbol, NODE *subs)
{
	NODE **val = str_exists(symbol, subs);
	const char *newval;

	assert(val != NULL);

	newval = (*val)->stptr;
	if (newval == NULL)
		newval = "";

	(void) setenv(subs->stptr, newval, 1);

	return val;
}

/* init_env_array --- set up the pointers for ENVIRON. A bit hacky. */

void
init_env_array(NODE *env_node)
{
	/* If POSIX simply don't reset the vtable and things work as before */
	if (do_posix)
		return;

	env_node->array_funcs = & env_array_func;
}
