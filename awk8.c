/*
 * routines for associative arrays.  SYMBOL is the address of the node (or
 * other pointer) being dereferenced.  SUBS is a number or string used as the
 * subscript. 
 *
 * Copyright (C) 1988 Free Software Foundation
 *
 */

/*
 * GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY.  No author or distributor accepts responsibility to anyone for
 * the consequences of using it or for whether it serves any particular
 * purpose or works at all, unless he says so in writing. Refer to the GAWK
 * General Public License for full details. 
 *
 * Everyone is granted permission to copy, modify and redistribute GAWK, but
 * only under the conditions described in the GAWK General Public License.  A
 * copy of this license is supposed to have been given to you along with GAWK
 * so you can know your rights and responsibilities.  It should be in a file
 * named COPYING.  Among other things, the copyright notice and this notice
 * must be preserved on all copies. 
 *
 * In other words, go ahead and share GAWK, but don't try to stop anyone else
 * from sharing it farther.  Help stamp out software hoarding! 
 */

#include "awk.h"

#ifdef DONTDEF
int primes[] = {31, 61, 127, 257, 509, 1021, 2053, 4099, 8191, 16381};
#endif

#define ASSOC_HASHSIZE 127
#define STIR_BITS(n) ((n) << 5 | (((n) >> 27) & 0x1f))
#define HASHSTEP(old, c) ((old << 1) + c)
#define MAKE_POS(v) (v & ~0x80000000)	/* make number positive */

NODE *
concat_exp(tree)
NODE *tree;
{
	NODE *r;
	NODE *n;
	char *s;
	unsigned char save;
	unsigned len;
	int subseplen;
	char *subsep;
	extern NODE *SUBSEP_node;

	if (tree->type != Node_expression_list)
		return force_string(tree_eval(tree));
	r = force_string(tree_eval(tree->lnode));
	if (tree->rnode == NULL)
		return r;
	subseplen = SUBSEP_node->lnode->stlen;
	subsep = SUBSEP_node->lnode->stptr;
	len = r->stlen + subseplen;
	emalloc(s, char *, len + 1, "concat_exp");
	(void) strcpy(s, r->stptr);
	free_temp(r);
	tree = tree->rnode;
	while (tree) {
		(void) strcat(s, subsep);
		r = force_string(tree_eval(tree->lnode));
		len += r->stlen + subseplen;
		erealloc(s, char *, len + 1, "concat_exp");
		(void) strcat(s, r->stptr);
		free_temp(r);
		tree = tree->rnode;
	}
	len -= subseplen;
	r = tmp_string(s, (int) len);
	free(s);
	return r;
}

/* Flush all the values in symbol[] before doing a split() */
assoc_clear(symbol)
NODE *symbol;
{
	int i;
	AHASH *bucket, *next;

	if (symbol->var_array == 0)
		return;
	for (i = 0; i < ASSOC_HASHSIZE; i++) {
		for (bucket = symbol->var_array[i]; bucket; bucket = next) {
			next = bucket->next;
			deref = bucket->name;
			do_deref();
			deref = bucket->value;
			do_deref();
			free((char *) bucket);
		}
		symbol->var_array[i] = 0;
	}
}

/*
 * calculate the hash function of the string subs, also returning in *typtr
 * the type (string or number) 
 */
static int
hash_calc(subs)
NODE *subs;
{
	register int hash1 = 0, i;

	subs = force_string(subs);
	for (i = 0; i < subs->stlen; i++)
		hash1 = HASHSTEP(hash1, subs->stptr[i]);

	hash1 = MAKE_POS(STIR_BITS((int) hash1)) % ASSOC_HASHSIZE;
	return (hash1);
}

/*
 * locate symbol[subs], given hash of subs and type 
 */
static AHASH *				/* NULL if not found */
assoc_find(symbol, subs, hash1)
NODE *symbol, *subs;
int hash1;
{
	register AHASH *bucket;

	for (bucket = symbol->var_array[hash1]; bucket; bucket = bucket->next) {
		if (cmp_nodes(bucket->name, subs))
			continue;
		return bucket;
	}
	return NULL;
}

/*
 * test whether the array element symbol[subs] exists or not 
 */
int
in_array(symbol, subs)
NODE *symbol, *subs;
{
	register int hash1;

	if (symbol->type == Node_param_list)
		symbol = stack_ptr[symbol->param_cnt];
	if (symbol->var_array == 0)
		return 0;
	subs = concat_exp(subs);
	hash1 = hash_calc(subs);
	if (assoc_find(symbol, subs, hash1) == NULL) {
		free_temp(subs);
		return 0;
	} else {
		free_temp(subs);
		return 1;
	}
}

/*
 * Find SYMBOL[SUBS] in the assoc array.  Install it with value "" if it
 * isn't there. Returns a pointer ala get_lhs to where its value is stored 
 */
NODE **
assoc_lookup(symbol, subs)
NODE *symbol, *subs;
{
	register int hash1 = 0, i;
	register AHASH *bucket;

	hash1 = hash_calc(subs);

	if (symbol->var_array == 0) {	/* this table really should grow
					 * dynamically */
		emalloc(symbol->var_array, AHASH **, (sizeof(AHASH *) *
			ASSOC_HASHSIZE), "assoc_lookup");
		for (i = 0; i < ASSOC_HASHSIZE; i++)
			symbol->var_array[i] = 0;
		symbol->type = Node_var_array;
	} else {
		bucket = assoc_find(symbol, subs, hash1);
		if (bucket != NULL) {
			free_temp(subs);
			return &(bucket->value);
		}
	}
	emalloc(bucket, AHASH *, sizeof(AHASH), "assoc_lookup");
	bucket->symbol = symbol;
	bucket->name = dupnode(subs);
	bucket->value = Nnull_string;
	bucket->next = symbol->var_array[hash1];
	symbol->var_array[hash1] = bucket;
	return &(bucket->value);
}

do_delete(symbol, tree)
NODE *symbol, *tree;
{
	register int hash1 = 0;
	register AHASH *bucket, *last;
	NODE *subs;

	if (symbol->var_array == 0)
		return;
	subs = concat_exp(tree);
	hash1 = hash_calc(subs);

	last = NULL;
	for (bucket = symbol->var_array[hash1]; bucket; last = bucket, bucket = bucket->next)
		if (cmp_nodes(bucket->name, subs) == 0)
			break;
	free_temp(subs);
	if (bucket == NULL)
		return;
	if (last)
		last->next = bucket->next;
	else
		symbol->var_array[hash1] = NULL;
	deref = bucket->name;
	do_deref();
	deref = bucket->value;
	do_deref();
	free((char *) bucket);
}

struct search *
assoc_scan(symbol)
NODE *symbol;
{
	struct search *lookat;

	if (!symbol->var_array)
		return 0;
	emalloc(lookat, struct search *, sizeof(struct search), "assoc_scan");
	lookat->numleft = ASSOC_HASHSIZE;
	lookat->arr_ptr = symbol->var_array;
	lookat->bucket = symbol->var_array[0];
	return assoc_next(lookat);
}

struct search *
assoc_next(lookat)
struct search *lookat;
{
	for (; lookat->numleft; lookat->numleft--) {
		while (lookat->bucket != 0) {
			lookat->retval = lookat->bucket->name;
			lookat->bucket = lookat->bucket->next;
			return lookat;
		}
		lookat->bucket = *++(lookat->arr_ptr);
	}
	free((char *) lookat);
	return 0;
}
