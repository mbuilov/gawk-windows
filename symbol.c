/*
 * symbol.c - routines for symbol table management and code allocation
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2015, 2017-2020,
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

extern SRCFILE *srcfiles;
extern INSTRUCTION *rule_list;

#define HASHSIZE	1021

static int func_count;	/* total number of functions */
static int var_count;	/* total number of global variables and functions */

static NODE *symbol_list;
static void (*install_func)(NODE *) = NULL;
static NODE *make_symbol(const char *name, NODETYPE type);
static NODE *install(const char *name, NODE *parm, NODETYPE type);
static void free_bcpool(INSTRUCTION_POOL *pl);

static AWK_CONTEXT *curr_ctxt = NULL;
static int ctxt_level;

static NODE *global_table, *param_table;
NODE *symbol_table, *func_table;

/* Use a flag to avoid a strcmp() call inside install() */
static bool installing_specials = false;

/* init_symbol_table --- make sure the symbol tables are initialized */

void
init_symbol_table()
{
	getnode(global_table);
	memset(global_table, '\0', sizeof(NODE));
	null_array(global_table);

	getnode(param_table);
	memset(param_table, '\0', sizeof(NODE));
	null_array(param_table);

	installing_specials = true;
	func_table = install_symbol(estrdup("FUNCTAB", 7), Node_var_array);

	symbol_table = install_symbol(estrdup("SYMTAB", 6), Node_var_array);
	installing_specials = false;
}

/*
 * install_symbol:
 * Install a global name in the symbol table, even if it is already there.
 * Caller must check against redefinition if that is desired.
 */

NODE *
install_symbol(const char *name, NODETYPE type)
{
	return install(name, NULL, type);
}


/*
 * lookup --- find the most recent global or param node for name
 *	installed by install_symbol
 */

NODE *
lookup(const char *name)
{
	NODE *n;
	NODE *tmp;
	NODE *tables[5];	/* manual init below, for z/OS */
	int i;

	/* ``It's turtles, all the way down.'' */
	tables[0] = param_table;	/* parameters shadow everything */
	tables[1] = global_table;	/* SYMTAB and FUNCTAB found first, can't be redefined */
	tables[2] = func_table;		/* then functions */
	tables[3] = symbol_table;	/* then globals */
	tables[4] = NULL;

	if (strncmp(name, "awk::", 5) == 0)
		tmp = make_string(name + 5, strlen(name) - 5);
	else
		tmp = make_string(name, strlen(name));

	n = NULL;
	for (i = 0; tables[i] != NULL; i++) {
		if (assoc_empty(tables[i]))
			continue;

		if ((do_posix || do_traditional) && tables[i] == global_table)
			continue;

		n = in_array(tables[i], tmp);
		if (n != NULL)
			break;
	}

	unref(tmp);
	if (n == NULL || n->type == Node_val)	/* non-variable in SYMTAB */
		return NULL;
	return n;	/* new place */
}

/* make_params --- allocate function parameters for the symbol table */

NODE *
make_params(char **pnames, int pcount)
{
	NODE *p, *parms;
	int i;

	if (pcount <= 0 || pnames == NULL)
		return NULL;

	ezalloc(parms, NODE *, pcount * sizeof(NODE), "make_params");

	for (i = 0, p = parms; i < pcount; i++, p++) {
		p->type = Node_param_list;
		p->param = pnames[i];	/* shadows pname and vname */
		p->param_cnt = i;
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

	if (   (pcount = func->param_cnt) <= 0
	    || (parms = func->fparms) == NULL)
		return;

	for (i = 0; i < pcount; i++)
		(void) install(parms[i].param, parms + i, Node_param_list);
}


/*
 * remove_params --- remove function parameters out of the symbol table.
 */

void
remove_params(NODE *func)
{
	NODE *parms, *p;
	int i, pcount;

	if (func == NULL)
		return;

	assert(func->type == Node_func);

	if (   (pcount = func->param_cnt) <= 0
	    || (parms = func->fparms) == NULL)
		return;

	for (i = pcount - 1; i >= 0; i--) {
		NODE *tmp;
		NODE *tmp2;

		p = parms + i;
		assert(p->type == Node_param_list);
		tmp = make_string(p->vname, strlen(p->vname));
		tmp2 = in_array(param_table, tmp);
		if (tmp2 != NULL && tmp2->dup_ent != NULL)
			tmp2->dup_ent = tmp2->dup_ent->dup_ent;
		else
			(void) assoc_remove(param_table, tmp);

		unref(tmp);
	}

	assoc_clear(param_table);	/* shazzam! */
}


/* remove_symbol --- remove a symbol from the symbol table */

NODE *
remove_symbol(NODE *r)
{
	NODE *n = in_array(symbol_table, r);

	if (n == NULL)
		return n;

	n = dupnode(n);

	(void) assoc_remove(symbol_table, r);

	return n;
}


/*
 * destroy_symbol --- remove a symbol from symbol table
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
		break;	/* use break so that storage is freed */
	}

	efree(r->vname);
	freenode(r);
}


/* make_symbol --- allocates a global symbol for the symbol table. */

static NODE *
make_symbol(const char *name, NODETYPE type)
{
	NODE *r;

	getnode(r);
	memset(r, '\0', sizeof(NODE));
	if (type == Node_var_array)
		null_array(r);
	else if (type == Node_var)
		r->var_value = dupnode(Nnull_string);
	r->vname = (char *) name;
	r->type = type;

	return r;
}

/* install --- install a global name or function parameter in the symbol table */

static NODE *
install(const char *name, NODE *parm, NODETYPE type)
{
	NODE *r;
	NODE *table;
	NODE *n_name;
	NODE *prev;

	if (strncmp(name, "awk::", 5) == 0)
		n_name = make_string(name + 5, strlen(name) - 5);
	else
		n_name = make_string(name, strlen(name));

	table = symbol_table;

	if (type == Node_param_list) {
		table = param_table;
	} else if (   type == Node_func
		   || type == Node_ext_func
		   || type == Node_builtin_func) {
		table = func_table;
	} else if (installing_specials) {
		table = global_table;
	}

	if (parm != NULL)
		r = parm;
	else {
		/* global symbol */
		r = make_symbol(name, type);
		if (type == Node_func)
			func_count++;
		if (type != Node_ext_func && type != Node_builtin_func && table != global_table)
			var_count++;	/* total, includes Node_func */
	}

	if (type == Node_param_list) {
		prev = in_array(table, n_name);
		if (prev == NULL)
			goto simple;
		r->dup_ent = prev->dup_ent;
		prev->dup_ent = r;
		unref(n_name);
	} else {
simple:
		/* the simple case */
		assoc_set(table, n_name, r);
	}

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
get_symbols(SYMBOL_TYPE what, bool sort)
{
	int i;
	NODE **table;
	NODE **list;
	NODE *r;
	long count = 0;
	long max;
	NODE *the_table;

	/*
	 * assoc_list() returns an array with two elements per awk array
	 * element. Elements i and i+1 in the C array represent the key
	 * and value of element j in the awk array. Thus the loops use += 2
	 * to go through the awk array.
	 */

	if (what == FUNCTION) {
		the_table = func_table;
		max = the_table->table_size * 2;

		list = assoc_list(the_table, "@unsorted", ASORTI);
		emalloc(table, NODE **, (func_count + 1) * sizeof(NODE *), "get_symbols");

		for (i = count = 0; i < max; i += 2) {
			r = list[i+1];
			if (r->type == Node_ext_func || r->type == Node_builtin_func)
				continue;
			assert(r->type == Node_func);
			table[count++] = r;
		}
	} else {	/* what == VARIABLE */
		update_global_values();

		the_table = symbol_table;
		max = the_table->table_size * 2;

		list = assoc_list(the_table, "@unsorted", ASORTI);
		/* add three: one for FUNCTAB, one for SYMTAB, and one for a final NULL */
		emalloc(table, NODE **, (var_count + 1 + 1 + 1) * sizeof(NODE *), "get_symbols");

		for (i = count = 0; i < max; i += 2) {
			r = list[i+1];
			if (r->type == Node_val)	/* non-variable in SYMTAB */
				continue;
			table[count++] = r;
		}

		table[count++] = func_table;
		table[count++] = symbol_table;
	}

	efree(list);

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
function_list(bool sort)
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
			print_func(fp, "array, %ld elements\n", assoc_length(r));
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
	assoc_clear(symbol_table);
	assoc_clear(func_table);
	assoc_clear(global_table);
}


/* append_symbol --- append symbol to the list of symbols
 *	installed in the symbol table.
 */

void
append_symbol(NODE *r)
{
	NODE *p;

	getnode(p);
	p->lnode = r;
	p->rnode = symbol_list->rnode;
	symbol_list->rnode = p;
}

/* release_symbols --- free symbol list and optionally remove symbol from symbol table */

void
release_symbols(NODE *symlist, int keep_globals)
{
	NODE *p, *next;

	for (p = symlist->rnode; p != NULL; p = next) {
		if (! keep_globals) {
			/*
			 * destroys globals, function, and params
			 * if still in symbol table
			 */
			destroy_symbol(p->lnode);
		}
		next = p->rnode;
		freenode(p);
	}
	symlist->rnode = NULL;
}

/* load_symbols --- fill in symbols' information */

void
load_symbols()
{
	NODE *r;
	NODE *tmp;
	NODE *sym_array;
	NODE **aptr;
	long i, j, max;
	NODE *user, *extension, *untyped, *scalar, *array, *built_in;
	NODE **list;
	NODE *tables[4];

	if (PROCINFO_node == NULL)
		return;

	tables[0] = func_table;
	tables[1] = symbol_table;
	tables[2] = global_table;
	tables[3] = NULL;

	tmp = make_string("identifiers", 11);
	aptr = assoc_lookup(PROCINFO_node, tmp);

	getnode(sym_array);
	memset(sym_array, '\0', sizeof(NODE));	/* PPC Mac OS X wants this */
	null_array(sym_array);

	unref(tmp);
	unref(*aptr);
	*aptr = sym_array;

	sym_array->parent_array = PROCINFO_node;
	sym_array->vname = estrdup("identifiers", 11);

	user = make_string("user", 4);
	extension = make_string("extension", 9);
	scalar = make_string("scalar", 6);
	untyped = make_string("untyped", 7);
	array = make_string("array", 5);
	built_in = make_string("builtin", 7);

	for (i = 0; tables[i] != NULL; i++) {
		list = assoc_list(tables[i], "@unsorted", ASORTI);
		max = tables[i]->table_size * 2;
		if (max == 0)
			continue;
		for (j = 0; j < max; j += 2) {
			r = list[j+1];
			if (   r->type == Node_ext_func
			    || r->type == Node_func
			    || r->type == Node_builtin_func
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
				case Node_builtin_func:
					*aptr = dupnode(built_in);
					break;
				case Node_var:
					*aptr = dupnode(scalar);
					break;
				case Node_var_array:
					*aptr = dupnode(array);
					break;
				case Node_var_new:
					*aptr = dupnode(untyped);
					break;
				default:
					cant_happen();
					break;
				}
			}
		}
		efree(list);
	}

	unref(user);
	unref(extension);
	unref(scalar);
	unref(untyped);
	unref(array);
}

/* check_param_names --- make sure no parameter is the name of a function */

bool
check_param_names(void)
{
	int i, j;
	NODE **list;
	NODE *f;
	long max;
	bool result = true;
	NODE n;

	if (assoc_empty(func_table))
		return result;

	max = func_table->table_size * 2;

	memset(& n, 0, sizeof n);
	n.type = Node_val;
	n.flags = STRING|STRCUR;
	n.stfmt = STFMT_UNUSED;
#ifdef HAVE_MPFR
	n.strndmode = MPFR_round_mode;
#endif

	/*
	 * assoc_list() returns an array with two elements per awk array
	 * element. Elements i and i+1 in the C array represent the key
	 * and value of element j in the awk array. Thus the loops use += 2
	 * to go through the awk array.
	 *
	 * In this case, the name is in list[i], and the function is
	 * in list[i+1]. Just what we need.
	 */

	list = assoc_list(func_table, "@unsorted", ASORTI);

	for (i = 0; i < max; i += 2) {
		f = list[i+1];
		if (f->type == Node_builtin_func || f->param_cnt == 0)
			continue;

		/* loop over each param in function i */
		for (j = 0; j < f->param_cnt; j++) {
			/* compare to function names */

			/* use a fake node to avoid malloc/free of make_string */
			n.stptr = f->fparms[j].param;
			n.stlen = strlen(f->fparms[j].param);

			if (in_array(func_table, & n)) {
				error(
			_("function `%s': cannot use function `%s' as a parameter name"),
					list[i]->stptr,
					f->fparms[j].param);
				result = false;
			}
		}
	}

	efree(list);
	return result;
}

static INSTRUCTION_POOL *pools;

/*
 * For best performance, the INSTR_CHUNK value should be divisible by all
 * possible sizes, i.e. 1 through MAX_INSTRUCTION_ALLOC. Otherwise, there
 * will be wasted space at the end of the block.
 */
#define INSTR_CHUNK (2*3*21)

struct instruction_block {
	struct instruction_block *next;
	INSTRUCTION i[INSTR_CHUNK];
};

/* bcfree --- deallocate instruction */

void
bcfree(INSTRUCTION *cp)
{
	assert(cp->pool_size >= 1 && cp->pool_size <= MAX_INSTRUCTION_ALLOC);

	cp->opcode = Op_illegal;
	cp->nexti = pools->pool[cp->pool_size - 1].free_list;
	pools->pool[cp->pool_size - 1].free_list = cp;
}

/* bcalloc --- allocate a new instruction */

INSTRUCTION *
bcalloc(OPCODE op, int size, int srcline)
{
	INSTRUCTION *cp;
	struct instruction_mem_pool *pool;

	assert(size >= 1 && size <= MAX_INSTRUCTION_ALLOC);
	pool = &pools->pool[size - 1];

	if (pool->free_list != NULL) {
		cp = pool->free_list;
		pool->free_list = cp->nexti;
	} else if (pool->free_space && pool->free_space + size <= & pool->block_list->i[INSTR_CHUNK]) {
		cp = pool->free_space;
		pool->free_space += size;
	} else {
		struct instruction_block *block;
		emalloc(block, struct instruction_block *, sizeof(struct instruction_block), "bcalloc");
		block->next = pool->block_list;
		pool->block_list = block;
		cp = &block->i[0];
		pool->free_space = &block->i[size];
	}

	memset(cp, 0, size * sizeof(INSTRUCTION));
	cp->pool_size = size;
	cp->opcode = op;
	cp->source_line = srcline;
	return cp;
}

/* new_context --- create a new execution context. */

AWK_CONTEXT *
new_context()
{
	AWK_CONTEXT *ctxt;

	ezalloc(ctxt, AWK_CONTEXT *, sizeof(AWK_CONTEXT), "new_context");
	ctxt->srcfiles.next = ctxt->srcfiles.prev = & ctxt->srcfiles;
	ctxt->rule_list.opcode = Op_list;
	ctxt->rule_list.lasti = & ctxt->rule_list;
	return ctxt;
}

/* set_context --- change current execution context. */

static void
set_context(AWK_CONTEXT *ctxt)
{
	pools = & ctxt->pools;
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
		fatal(_("cannot pop main context"));
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
		if (m->re_reg[0] != NULL)
			refree(m->re_reg[0]);
		if (m->re_reg[1] != NULL)
			refree(m->re_reg[1]);
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

/* free_bc_mempool --- free a single pool */

static void
free_bc_mempool(struct instruction_mem_pool *pool, int size)
{
	bool first = true;
	struct instruction_block *block, *next;

	for (block = pool->block_list; block; block = next) {
		INSTRUCTION *cp, *end;

		end = (first ? pool->free_space : & block->i[INSTR_CHUNK]);
		for (cp = & block->i[0]; cp + size <= end; cp += size) {
			if (cp->opcode != Op_illegal)
				free_bc_internal(cp);
		}
		next = block->next;
		efree(block);
		first = false;
	}
}


/* free_bcpool --- free list of instruction memory pools */

static void
free_bcpool(INSTRUCTION_POOL *pl)
{
	int i;

	for (i = 0; i < MAX_INSTRUCTION_ALLOC; i++)
		free_bc_mempool(& pl->pool[i], i + 1);
}

/* is_all_upper --- return true if name is all uppercase letters */

/*
 * DON'T use isupper(), it's locale aware!
 */

bool
is_all_upper(const char *name)
{
	for (; *name != '\0'; name++) {
		switch (*name) {
		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z':
			break;
		default:
			return false;
		}
	}

	return true;
}
