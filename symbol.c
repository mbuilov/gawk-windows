/*
 * symbol.c - routines for symbol table management and code allocation
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

extern SRCFILE *srcfiles;
extern INSTRUCTION *rule_list;

#define HASHSIZE	1021

static NODE *variables[HASHSIZE];
static int func_count;	/* total number of functions */
static int var_count;	/* total number of global variables and functions */

static NODE *symbol_list;
static void (*install_func)(NODE *) = NULL;
static NODE *make_symbol(char *name, NODETYPE type);
static NODE *install(char *name, NODE *hp, NODETYPE type);
static void free_bcpool(INSTRUCTION *pl);

static AWK_CONTEXT *curr_ctxt = NULL;
static int ctxt_level;

/*
 * install_symbol:
 * Install a global name in the symbol table, even if it is already there.
 * Caller must check against redefinition if that is desired. 
 */

NODE *
install_symbol(char *name, NODETYPE type)
{
	return install(name, NULL, type);
}


/* lookup --- find the most recent global or param node for name
 *	installed by install_symbol
 */

NODE *
lookup(const char *name)
{
	NODE *hp;
	size_t len;
	int hash1;

	len = strlen(name);
	hash1 = hash(name, len, (unsigned long) HASHSIZE, NULL);
	for (hp = variables[hash1]; hp != NULL; hp = hp->hnext) {
		if (hp->hlength == len && strncmp(hp->hname, name, len) == 0)
			return hp->hvalue;
	}
	return NULL;
}

/* make_params --- allocate function parameters for the symbol table */

NODE *
make_params(char **pnames, int pcount)
{
	NODE *hp, *parms;
	int i;
	
	if (pcount <= 0 || pnames == NULL)
		return NULL;

	emalloc(parms, NODE *, pcount * sizeof(NODE), "make_params");
	memset(parms, '\0', pcount * sizeof(NODE));

	for (i = 0, hp = parms; i < pcount; i++, hp++) {
		hp->type = Node_param_list;
		hp->hname = pnames[i];	/* shadows pname and vname */
		hp->hlength = strlen(pnames[i]);
		hp->param_cnt = i;
		hp->hvalue = hp;	/* points to itself */
	}

	return parms;
}

/* install_params --- install function parameters into the symbol table */

void
install_params(NODE *func)
{
	int i, pcount;
	NODE *parms;

	if (func == NULL)
		return;
	assert(func->type == Node_func);
	if ((pcount = func->param_cnt) <= 0
			|| (parms = func->fparms) == NULL
	)
		return;
	for (i = 0; i < pcount; i++)
		(void) install(NULL, parms + i, Node_param_list);
}


/*
 * remove_params --- remove function parameters out of the symbol table.
 */

void
remove_params(NODE *func)
{
	NODE *parms, *p, *prev, *n;
	int i, pcount, hash1;

	if (func == NULL)
		return;
	assert(func->type == Node_func);
	if ((pcount = func->param_cnt) <= 0
			|| (parms = func->fparms) == NULL
	)
		return;

	for (i = pcount - 1; i >= 0; i--) {
		p = parms + i;
		hash1 = p->hcode;
		if (hash1 < 0 || hash1 >= HASHSIZE)
			continue;
		for (prev = NULL, n = variables[hash1]; n != NULL;
					prev = n, n = n->hnext) {
			if (n == p)
				break;
		}
		if (n == NULL)
			continue;
		if (prev == NULL)
			variables[hash1] = n->hnext;	/* param at the head of the chain */
		else
			prev->hnext = n->hnext;		/* param not at the head */ 
	}
}


/* remove_symbol --- remove a symbol from the symbol table */

NODE *
remove_symbol(NODE *r)
{
	NODE *prev, *hp;
	int hash1;
	
	hash1 = hash(r->vname, strlen(r->vname), (unsigned long) HASHSIZE, NULL);
	for (prev = NULL, hp = variables[hash1]; hp != NULL;
				prev = hp, hp = hp->hnext) {
		if (hp->hvalue == r)
			break;
	}

	if (hp == NULL)
		return NULL;
	assert(hp->hcode == hash1);

	if (prev == NULL)
		variables[hash1] = hp->hnext;	/* symbol at the head of chain */
	else
		prev->hnext = hp->hnext;	/* symbol not at the head */

	if (r->type == Node_param_list)
		return r;	/* r == hp */
	if (r->type == Node_func)
		func_count--;
	if (r->type != Node_ext_func)
		var_count--;
	freenode(hp);
	return r;
}


/* destroy_symbol --- remove a symbol from symbol table
*	and free all associated memory.
*/

void
destroy_symbol(NODE *r)
{
	r = remove_symbol(r);
	if (r == NULL)
		return;

	switch (r->type) {
	case Node_func:
		if (r->param_cnt > 0) {
			NODE *n;
			int i;
			int pcount = r->param_cnt;
				
			/* function parameters of type Node_param_list */				
			for (i = 0; i < pcount; i++) {
				n = r->fparms + i;
				efree(n->param);
			}		
			efree(r->fparms);
		}
		break;

	case Node_ext_func:
		bcfree(r->code_ptr);
		break;

	case Node_var_array:
		assoc_clear(r);
		break;

	case Node_var: 
		unref(r->var_value);
		break;

	default:
		/* Node_param_list -- YYABORT */
		return;
	}

	efree(r->vname);
	freenode(r);
}


/* make_symbol --- allocates a global symbol for the symbol table. */

static NODE *
make_symbol(char *name, NODETYPE type)
{
	NODE *hp, *r;

	getnode(hp);
	hp->type = Node_hashnode;
	hp->hlength = strlen(name);
	hp->hname = name;
	getnode(r);
	memset(r, '\0', sizeof(NODE));
	hp->hvalue = r;
	if (type == Node_var_array)
		init_array(r);
	else if (type == Node_var)
		r->var_value = dupnode(Nnull_string);
	r->vname = name;
	r->type = type;
	return hp;
}

/* install --- install a global name or function parameter in the symbol table */

static NODE *
install(char *name, NODE *hp, NODETYPE type)
{
	int hash1;
	NODE *r;

	if (hp == NULL) {
		/* global symbol */
		hp = make_symbol(name, type);
		if (type == Node_func)
			func_count++;
		if (type != Node_ext_func)
			var_count++;	/* total, includes Node_func */
	}

	r = hp->hvalue;
	hash1 = hash(hp->hname, hp->hlength, (unsigned long) HASHSIZE, NULL);
	hp->hcode = hash1;
	hp->hnext = variables[hash1];
	variables[hash1] = hp;

	if (install_func)
		(*install_func)(r);

	return r;
}


/* comp_symbol --- compare two (variable or function) names */

static int
comp_symbol(const void *v1, const void *v2)
{
	const NODE *const *npp1, *const *npp2;
	const NODE *n1, *n2;

	npp1 = (const NODE *const *) v1;
	npp2 = (const NODE *const *) v2;
	n1 = *npp1;
	n2 = *npp2;

	return strcmp(n1->vname, n2->vname);
}


typedef enum { FUNCTION = 1, VARIABLE } SYMBOL_TYPE;

/* get_symbols --- return a list of optionally sorted symbols */
 
static NODE **
get_symbols(SYMBOL_TYPE what, int sort)
{
	int i;
	NODE **table;
	NODE *hp, *r;
	long j, count = 0;

	if (what == FUNCTION)
		count = func_count;
	else	/* if (what == VARIABLE) */
		count = var_count;

	emalloc(table, NODE **, (count + 1) * sizeof(NODE *), "symbol_list");
	if (what == VARIABLE)
		update_global_values();

	for (i = j = 0; i < HASHSIZE; i++)
		for (hp = variables[i]; hp != NULL; hp = hp->hnext) {
			if (hp->type != Node_hashnode)
				continue;
			r = hp->hvalue;
			if (r->type == Node_ext_func)
				continue;
			if (what == FUNCTION && r->type == Node_func)
				table[j++] = r;
			else if (what == VARIABLE)
				table[j++] = r;
		}

	if (sort && count > 1)
		qsort(table, count, sizeof(NODE *), comp_symbol);	/* Shazzam! */
	table[count] = NULL; /* null terminate the list */
	return table;
}


/* variable_list --- list of global variables */

NODE **
variable_list()
{
	return get_symbols(VARIABLE, true);
}

/* function_list --- list of functions */

NODE **
function_list(int sort)
{
	return get_symbols(FUNCTION, sort);
}

/* print_vars --- print names and values of global variables */ 

void
print_vars(NODE **table, int (*print_func)(FILE *, const char *, ...), FILE *fp)
{
	int i;
	NODE *r;

	assert(table != NULL);

	for (i = 0; (r = table[i]) != NULL; i++) {
		if (r->type == Node_func || r->type == Node_ext_func)
			continue;
		print_func(fp, "%s: ", r->vname);
		if (r->type == Node_var_array)
			print_func(fp, "array, %ld elements\n", r->table_size);
		else if (r->type == Node_var_new)
			print_func(fp, "untyped variable\n");
		else if (r->type == Node_var)
			valinfo(r->var_value, print_func, fp);
	}
}


/* foreach_func --- execute given function for each awk function in table. */

int
foreach_func(NODE **table, int (*pfunc)(INSTRUCTION *, void *), void *data)
{
	int i;
	NODE *r;
	int ret = 0;

	assert(table != NULL);

	for (i = 0; (r = table[i]) != NULL; i++) {
		if ((ret = pfunc(r->code_ptr, data)) != 0)
			break;
	}
	return ret;
}

/* release_all_vars --- free all variable memory */

void
release_all_vars()
{
	int i;
	NODE *hp, *r, *next;

	for (i = 0; i < HASHSIZE; i++)
		for (hp = variables[i]; hp != NULL; hp = next) {
			next = hp->hnext;
			if (hp->type != Node_hashnode)
				continue;
			r = hp->hvalue;
			if (r->type == Node_func || r->type == Node_ext_func)
				continue;
			if (r->type == Node_var_array)
				assoc_clear(r);
			else if (r->type == Node_var)
				unref(r->var_value);
			efree(r->vname);
			freenode(r);
			freenode(hp);
		}
}


/* append_symbol --- append symbol to the list of symbols
 *	installed in the symbol table.
 */

void
append_symbol(NODE *r)
{
	NODE *hp;

	getnode(hp);
	hp->lnode = r;
	hp->rnode = symbol_list->rnode;
	symbol_list->rnode = hp;
}

/* release_symbol --- free symbol list and optionally remove symbol from symbol table */

void
release_symbols(NODE *symlist, int keep_globals)
{
	NODE *hp, *next;

	for (hp = symlist->rnode; hp != NULL; hp = next) {
		if (! keep_globals) {
			/* destroys globals, function, and params
			 * if still in symbol table
			 */
			destroy_symbol(hp->lnode);
		}
		next = hp->rnode;
		freenode(hp);
	}
	symlist->rnode = NULL;
}

/* load_symbols --- fill in symbols' information */

void
load_symbols()
{
	NODE *hp, *r;
	NODE *tmp;
	NODE *sym_array;
	NODE **aptr;
	long i;
	NODE *user, *extension, *variable, *scalar,*array;

	if (PROCINFO_node == NULL)
		return;

	tmp = make_string("identifiers", 11);
	aptr = assoc_lookup(PROCINFO_node, tmp);

	getnode(sym_array);
	init_array(sym_array);

	unref(*aptr);
	*aptr = sym_array;

	sym_array->parent_array = PROCINFO_node;
	sym_array->vname = estrdup("identifiers", 11);
	make_aname(sym_array);

	user = make_string("user", 4);
	extension = make_string("extension", 9);
	scalar = make_string("scalar", 6);
	variable = make_string("variable", 8);
	array = make_string("array", 5);

	for (i = 0; i < HASHSIZE; i++) {
		for (hp = variables[i]; hp != NULL; hp = hp->hnext) {
			if (hp->type != Node_hashnode)
				continue;

			r = hp->hvalue;
			if (   r->type == Node_ext_func
			    || r->type == Node_func
			    || r->type == Node_var
			    || r->type == Node_var_array
			    || r->type == Node_var_new) {
				tmp = make_string(r->vname, strlen(r->vname));
				aptr = assoc_lookup(sym_array, tmp);
				unref(tmp);
				unref(*aptr);
				switch (r->type) {
				case Node_ext_func:
					*aptr = dupnode(extension);
					break;
				case Node_func:
					*aptr = dupnode(user);
					break;
				case Node_var:
					*aptr = dupnode(scalar);
					break;
				case Node_var_array:
					*aptr = dupnode(array);
					break;
				case Node_var_new:
					*aptr = dupnode(variable);
					break;
				default:
					cant_happen();
					break;
				}
			}
		}
	}

	unref(user);
	unref(extension);
	unref(scalar);
	unref(variable);
	unref(array);
}

#define pool_size	d.dl
#define freei		x.xi
static INSTRUCTION *pool_list;

/* INSTR_CHUNK must be > largest code size (3) */
#define INSTR_CHUNK 127

/* bcfree --- deallocate instruction */

void
bcfree(INSTRUCTION *cp)
{
	cp->opcode = 0;
	cp->nexti = pool_list->freei;
	pool_list->freei = cp;
}	

/* bcalloc --- allocate a new instruction */

INSTRUCTION *
bcalloc(OPCODE op, int size, int srcline)
{
	INSTRUCTION *cp;

	if (size > 1) {
		/* wide instructions Op_rule, Op_func_call .. */
		emalloc(cp, INSTRUCTION *, (size + 1) * sizeof(INSTRUCTION), "bcalloc");
		cp->pool_size = size;
		cp->nexti = pool_list->nexti;
		pool_list->nexti = cp++;
	} else {
		INSTRUCTION *pool;

		pool = pool_list->freei;
		if (pool == NULL) {
			INSTRUCTION *last;
			emalloc(cp, INSTRUCTION *, (INSTR_CHUNK + 1) * sizeof(INSTRUCTION), "bcalloc");

			cp->pool_size = INSTR_CHUNK;
			cp->nexti = pool_list->nexti;
			pool_list->nexti = cp;
			pool = ++cp;
			last = &pool[INSTR_CHUNK - 1];
			for (; cp <= last; cp++) {
				cp->opcode = 0;
				cp->nexti = cp + 1;
			}
			--cp;
			cp->nexti = NULL;
		}
		cp = pool;
		pool_list->freei = cp->nexti;
	}

	memset(cp, 0, size * sizeof(INSTRUCTION));
	cp->opcode = op;
	cp->source_line = srcline;
	return cp;
}

/* new_context --- create a new execution context. */

AWK_CONTEXT *
new_context()
{
	AWK_CONTEXT *ctxt;

	emalloc(ctxt, AWK_CONTEXT *, sizeof(AWK_CONTEXT), "new_context");
	memset(ctxt, 0, sizeof(AWK_CONTEXT));
	ctxt->srcfiles.next = ctxt->srcfiles.prev = & ctxt->srcfiles;
	ctxt->rule_list.opcode = Op_list;
	ctxt->rule_list.lasti = & ctxt->rule_list;
	return ctxt;
}

/* set_context --- change current execution context. */

static void
set_context(AWK_CONTEXT *ctxt)
{
	pool_list = & ctxt->pools;
	symbol_list = & ctxt->symbols;
	srcfiles = & ctxt->srcfiles;
	rule_list = & ctxt->rule_list;
	install_func = ctxt->install_func;
	curr_ctxt = ctxt;
}

/*
 * push_context:
 *
 * Switch to the given context after saving the current one. The set
 * of active execution contexts forms a stack; the global or main context
 * is at the bottom of the stack.
 */

void
push_context(AWK_CONTEXT *ctxt)
{
	ctxt->prev = curr_ctxt;
	/* save current source and sourceline */
	if (curr_ctxt != NULL) {
		curr_ctxt->sourceline = sourceline;
		curr_ctxt->source = source;
	}
	sourceline = 0;
	source = NULL;
	set_context(ctxt);
	ctxt_level++;
}

/* pop_context --- switch to previous execution context. */ 

void
pop_context()
{
	AWK_CONTEXT *ctxt;

	assert(curr_ctxt != NULL);
	if (curr_ctxt->prev == NULL)
		fatal(_("can not pop main context"));
	ctxt = curr_ctxt->prev;
	/* restore source and sourceline */
	sourceline = ctxt->sourceline;
	source = ctxt->source;
	set_context(ctxt);
	ctxt_level--;
}

/* in_main_context --- are we in the main context ? */

int
in_main_context()
{
	assert(ctxt_level > 0);
	return (ctxt_level == 1);
}

/* free_context --- free context structure and related data. */ 

void
free_context(AWK_CONTEXT *ctxt, bool keep_globals)
{
	SRCFILE *s, *sn;

	if (ctxt == NULL)
		return;

	assert(curr_ctxt != ctxt);

 	/* free all code including function codes */

	free_bcpool(& ctxt->pools);

	/* free symbols */

	release_symbols(& ctxt->symbols, keep_globals);

	/* free srcfiles */

	for (s = & ctxt->srcfiles; s != & ctxt->srcfiles; s = sn) {
		sn = s->next;
		if (s->stype != SRC_CMDLINE && s->stype != SRC_STDIN)
			efree(s->fullpath);
		efree(s->src);
		efree(s);
	}

	efree(ctxt);
}

/* free_bc_internal --- free internal memory of an instruction. */ 

static void
free_bc_internal(INSTRUCTION *cp)
{
	NODE *m;

	switch(cp->opcode) {
	case Op_func_call:
		if (cp->func_name != NULL)
			efree(cp->func_name);
		break;
	case Op_push_re:
	case Op_match_rec:
	case Op_match:
	case Op_nomatch:
		m = cp->memory;
		if (m->re_reg != NULL)
			refree(m->re_reg);
		if (m->re_exp != NULL)
			unref(m->re_exp);
		if (m->re_text != NULL)
			unref(m->re_text);
		freenode(m);
		break;   		
	case Op_token:
		/* token lost during error recovery in yyparse */
		if (cp->lextok != NULL)
			efree(cp->lextok);
		break;
	case Op_push_i:
		m = cp->memory;
		unref(m);
		break;
	case Op_store_var:
		m = cp->initval;
		if (m != NULL)
			unref(m);
		break;
	case Op_illegal:
		cant_happen();
	default:
		break;	
	}
}

/* free_bcpool --- free list of instruction memory pools */

static void
free_bcpool(INSTRUCTION *pl)
{
	INSTRUCTION *pool, *tmp;

	for (pool = pl->nexti; pool != NULL; pool = tmp) {
		INSTRUCTION *cp, *last;
		long psiz;
		psiz = pool->pool_size;
		if (psiz == INSTR_CHUNK)
			last = pool + psiz;
		else
			last = pool + 1;
		for (cp = pool + 1; cp <= last ; cp++) {
			if (cp->opcode != 0)
				free_bc_internal(cp);
		}
		tmp = pool->nexti;
		efree(pool);
	}
	memset(pl, 0, sizeof(INSTRUCTION));
}
