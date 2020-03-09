/*
 * array.c - routines for awk arrays.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2014, 2016, 2018, 2019, 2020,
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

extern FILE *output_fp;
extern NODE **fmt_list;          /* declared in eval.c */

NODE *success_node;

static size_t SUBSEPlen;
static char *SUBSEP;
static char indent_char[] = "    ";

static int sort_up_value_type(const void *p1, const void *p2);
static NODE **null_lookup(NODE *symbol, NODE *subs);
static NODE **null_dump(NODE *symbol, NODE *subs);
static const array_funcs_t null_array_func = {
	"null",
	(afunc_t) 0,
	(afunc_t) 0,
	null_lookup,
	null_afunc,
	null_afunc,
	null_afunc,
	null_afunc,
	null_afunc,
	null_dump,
	(afunc_t) 0,
};

#define MAX_ATYPE 10

static const array_funcs_t *array_types[MAX_ATYPE];
static int num_array_types = 0;

/* register_array_func --- add routines to handle arrays */

static int
register_array_func(const array_funcs_t *afunc)
{
	if (afunc && num_array_types < MAX_ATYPE) {
		if (afunc != & str_array_func && afunc->type_of == NULL)
			return false;
		array_types[num_array_types++] = afunc;
		if (afunc->init)	/* execute init routine if any */
			(void) (*afunc->init)(NULL, NULL);
		return true;
	}
	return false;
}


/* array_init --- register all builtin array types */

void
array_init()
{
	(void) register_array_func(& str_array_func);	/* the default */
	if (! do_mpfr) {
		(void) register_array_func(& int_array_func);
		(void) register_array_func(& cint_array_func);
	}
}


/* make_array --- create an array node */

NODE *
make_array()
{
	NODE *array;
	getnode(array);
	memset(array, '\0', sizeof(NODE));
	array->type = Node_var_array;
	array->array_funcs = & null_array_func;
	/* vname, flags, and parent_array not set here */

	return array;
}


/* null_array --- force symbol to be an empty typeless array */

void
null_array(NODE *symbol)
{
	symbol->type = Node_var_array;
	symbol->array_funcs = & null_array_func;
	symbol->buckets = NULL;
	symbol->table_size = symbol->array_size = 0;
	symbol->array_capacity = 0;
	symbol->flags = 0;

	assert(symbol->xarray == NULL);

	/* vname, parent_array not (re)initialized */
}


/* null_lookup --- assign type to an empty array. */

static NODE **
null_lookup(NODE *symbol, NODE *subs)
{
	int i;
	const array_funcs_t *afunc = NULL;

	assert(symbol->table_size == 0);

	/*
	 * Check which array type wants to accept this sub; traverse
	 * array type list in reverse order.
	 */
	for (i = num_array_types - 1; i >= 1; i--) {
		afunc = array_types[i];
		if (afunc->type_of(symbol, subs) != NULL)
			break;
	}
	if (i == 0 || afunc == NULL)
		afunc = array_types[0];	/* default is str_array_func */
	symbol->array_funcs = afunc;

	/* We have the right type of array; install the subscript */
	return symbol->alookup(symbol, subs);
}


/* null_afunc --- default function for array interface */

NODE **
null_afunc(NODE *symbol ATTRIBUTE_UNUSED, NODE *subs ATTRIBUTE_UNUSED)
{
	return NULL;
}

/* null_dump --- dump function for an empty array */

static NODE **
null_dump(NODE *symbol, NODE *subs ATTRIBUTE_UNUSED)
{
	fprintf(output_fp, "array `%s' is empty\n", array_vname(symbol));
	return NULL;
}


/* assoc_copy --- duplicate input array "symbol" */

NODE *
assoc_copy(NODE *symbol, NODE *newsymb)
{
	assert(newsymb->vname != NULL);

	assoc_clear(newsymb);
	(void) symbol->acopy(symbol, newsymb);
	newsymb->array_funcs = symbol->array_funcs;
	newsymb->flags = symbol->flags;
	return newsymb;
}


/* assoc_dump --- dump array */

void
assoc_dump(NODE *symbol, NODE *ndump)
{
	if (symbol->adump)
		(void) symbol->adump(symbol, ndump);
}


/* make_aname --- construct a 'vname' for a (sub)array */

const char *
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
}
#undef SLEN


/*
 * array_vname --- print the name of the array
 *
 * Returns a pointer to a statically maintained dynamically allocated string.
 * It's appropriate for printing the name once; if the caller wants
 * to save it, they have to make a copy.
 */

const char *
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
 *  force_array --- proceed to the actual Node_var_array,
 *	change Node_var_new to an array.
 *	If canfatal and type isn't good, die fatally,
 *	otherwise return the final actual value.
 */

NODE *
force_array(NODE *symbol, bool canfatal)
{
	NODE *save_symbol = symbol;
	bool isparam = false;

	if (symbol->type == Node_param_list) {
		save_symbol = symbol = GET_PARAM(symbol->param_cnt);
		isparam = true;
		if (symbol->type == Node_array_ref)
			symbol = symbol->orig_array;
	}

	switch (symbol->type) {
	case Node_var_new:
		symbol->xarray = NULL;	/* make sure union is as it should be */
		null_array(symbol);
		symbol->parent_array = NULL;	/* main array has no parent */
		/* fall through */
	case Node_var_array:
		break;

	case Node_array_ref:
	default:
		/* notably Node_var but catches also e.g. a[1] = "x"; a[1][1] = "y" */
		if (canfatal) {
			if (symbol->type == Node_val)
				fatal(_("attempt to use a scalar value as array"));
			if (isparam)
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
	SUBSEP_node->var_value = force_string(SUBSEP_node->var_value);
	SUBSEP = SUBSEP_node->var_value->stptr;
	SUBSEPlen = SUBSEP_node->var_value->stlen;
}


/* concat_exp --- concatenate expression list into a single string */

NODE *
concat_exp(int nargs, bool do_subsep)
{
	/* do_subsep is false for Op_concat */
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
		r = TOP();
		if (r->type == Node_var_array) {
			while (--i > 0)
				DEREF(args_array[i]);	/* avoid memory leak */
			fatal(_("attempt to use array `%s' in a scalar context"), array_vname(r));
		}
		r = POP_STRING();
		args_array[i] = r;
		len += r->stlen;
	}
	len += (nargs - 1) * subseplen;

	emalloc(str, char *, len + 1, "concat_exp");

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


/*
 * adjust_fcall_stack: remove subarray(s) of symbol[] from
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
	pcount = func->param_cnt;
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
			/*
			 * 'symbol' is a subarray, and 'r' is the same subarray:
			 *
			 *   function f(c, d) { delete c[0]; .. }
			 *   BEGIN { a[0][0] = 1; f(a, a[0]); .. }
			 *
			 * But excludes cases like (nsubs = 0):
			 *
			 *   function f(c, d) { delete c; ..}
			 *   BEGIN { a[0][0] = 1; f(a[0], a[0]); ...}
			 */

			null_array(r);
			r->parent_array = NULL;
			continue;
		}

		/* Case 2 */
		for (n = n->parent_array; n != NULL; n = n->parent_array) {
			assert(n->type == Node_var_array);
			if (n == symbol) {
				/*
				 * 'r' is a subarray of 'symbol':
				 *
				 *    function f(c, d) { delete c; .. use d as array .. }
				 *    BEGIN { a[0][0] = 1; f(a, a[0]); .. }
				 *	OR
				 *    BEGIN { a[0][0][0][0] = 1; f(a[0], a[0][0][0]); .. }
				 *
				 */
				null_array(r);
				r->parent_array = NULL;
				break;
			}
		}
	}
}


/* do_delete --- perform `delete array[s]' */

/*
 * `symbol' is array
 * `nsubs' is no of subscripts
 */

void
do_delete(NODE *symbol, int nsubs)
{
	NODE *val, *subs;
	int i;

	assert(symbol->type == Node_var_array);
	subs = val = NULL;	/* silence the compiler */

	/*
	 * The force_string() call is needed to make sure that
	 * the string subscript is reasonable.  For example, with it:
	 *
	 * $ ./gawk --posix 'BEGIN { CONVFMT="%ld"; delete a[1.233]}'
	 * gawk: cmd. line:1: fatal: `%l' is not permitted in POSIX awk formats
	 *
	 * Without it, the code does not fail.
	 */

#define free_subs(n)    do {                                    \
    NODE *s = PEEK(n - 1);                                      \
    if (s->type == Node_val) {                                  \
        (void) force_string(s);	/* may have side effects. */    \
        DEREF(s);                                               \
    }                                                           \
} while (--n > 0)

	if (nsubs == 0) {
		/* delete array */

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

		val = in_array(symbol, subs);
		if (val == NULL) {
			if (do_lint) {
				subs = force_string(subs);
				lintwarn(_("delete: index `%.*s' not in array `%s'"),
					(int) subs->stlen, subs->stptr, array_vname(symbol));
			}
			/* avoid memory leak, free all subs */
			free_subs(i);
			return;
		}

		if (i > 1) {
			if (val->type != Node_var_array) {
				/* e.g.: a[1] = 1; delete a[1][1] */

				free_subs(i);
				subs = force_string(subs);
				fatal(_("attempt to use scalar `%s[\"%.*s\"]' as an array"),
					array_vname(symbol),
					(int) subs->stlen,
					subs->stptr);
			}
			symbol = val;
			DEREF(subs);
		}
	}

	if (val->type == Node_var_array) {
		adjust_fcall_stack(val, nsubs);  /* fix function call stack; See above. */
		assoc_clear(val);
		/* cleared a sub-array, free Node_var_array */
		efree(val->vname);
		freenode(val);
	} else
		unref(val);

	(void) assoc_remove(symbol, subs);
	DEREF(subs);
	if (assoc_empty(symbol))
		/* last element was removed, so reset array type to null */
		null_array(symbol);

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
	NODE **list;
	NODE akind;

	akind.flags = AINDEX|ADELETE;	/* need a single index */
	list = symbol->alist(symbol, & akind);

	if (assoc_empty(symbol))
		return;

	unref(*lhs);
	*lhs = list[0];
	efree(list);

	/* blast the array in one shot */
	adjust_fcall_stack(symbol, 0);
	assoc_clear(symbol);
}


/* value_info --- print scalar node info */

static void
value_info(NODE *n)
{

#define PREC_NUM -1

	if (n == Nnull_string || n == Null_field) {
		fprintf(output_fp, "<(null)>");
		return;
	}

	if ((n->flags & (STRING|STRCUR)) != 0) {
		fprintf(output_fp, "<");
		fprintf(output_fp, "\"%.*s\"", (int) n->stlen, n->stptr);
		if ((n->flags & (NUMBER|NUMCUR)) != 0) {
#ifdef HAVE_MPFR
			if (is_mpg_float(n))
				fprintf(output_fp, ":%s",
					mpg_fmt("%.*R*g", PREC_NUM, ROUND_MODE, n->mpg_numbr));
			else if (is_mpg_integer(n))
				fprintf(output_fp, ":%s", mpg_fmt("%Zd", n->mpg_i));
			else
#endif
			fprintf(output_fp, ":%.*g", PREC_NUM, n->numbr);
		}
		fprintf(output_fp, ">");
	} else {
#ifdef HAVE_MPFR
		if (is_mpg_float(n))
			fprintf(output_fp, "<%s>",
				mpg_fmt("%.*R*g", PREC_NUM, ROUND_MODE, n->mpg_numbr));
		else if (is_mpg_integer(n))
			fprintf(output_fp, "<%s>", mpg_fmt("%Zd", n->mpg_i));
		else
#endif
		fprintf(output_fp, "<%.*g>", PREC_NUM, n->numbr);
	}

	fprintf(output_fp, ":%s", flags2str(n->flags));

	if ((n->flags & MALLOC) != 0)
		fprintf(output_fp, ":%ld", n->valref);
	else
		fprintf(output_fp, ":");

	if ((n->flags & (STRING|STRCUR)) == STRCUR) {
		size_t len;

		fprintf(output_fp, "][");
		fprintf(output_fp, "stfmt=%d, ", n->stfmt);
		/*
		 * If not STFMT_UNUSED, could be CONVFMT or OFMT if last
		 * used in a print statement. If immutable, could be that it
		 * was originally set as a string, or it's a number that has
		 * an integer value.
		 */
		len = fmt_list[n->stfmt]->stlen;
		fmt_list[n->stfmt]->stptr[len] = '\0';
		fprintf(output_fp, "FMT=\"%s\"",
					n->stfmt == STFMT_UNUSED ? "<unused>"
					: fmt_list[n->stfmt]->stptr);
#ifdef HAVE_MPFR
		fprintf(output_fp, ", RNDMODE=\"%c\"", n->strndmode);
#endif
	}

#undef PREC_NUM
}


void
indent(int indent_level)
{
	int i;
	for (i = 0; i < indent_level; i++)
		fprintf(output_fp, "%s", indent_char);
}

/* assoc_info --- print index, value info */

void
assoc_info(NODE *subs, NODE *val, NODE *ndump, const char *aname)
{
	int indent_level = ndump->alevel;

	indent_level++;
	indent(indent_level);
	fprintf(output_fp, "I: [%s:", aname);
	if ((subs->flags & (MPFN|MPZN|INTIND)) == INTIND)
		fprintf(output_fp, "<%ld>", (long) subs->numbr);
	else
		value_info(subs);
	fprintf(output_fp, "]\n");

	indent(indent_level);
	if (val->type == Node_val) {
		fprintf(output_fp, "V: [scalar: ");
		value_info(val);
	} else {
		fprintf(output_fp, "V: [");
		ndump->alevel++;
		ndump->adepth--;
		assoc_dump(val, ndump);
		ndump->adepth++;
		ndump->alevel--;
		indent(indent_level);
	}
	fprintf(output_fp, "]\n");
}


/* do_adump --- dump an array: interface to assoc_dump */

NODE *
do_adump(int nargs)
{
	NODE *symbol, *tmp;
	static NODE ndump;
	long depth = 0;

	/*
	 * depth < 0, no index and value info.
	 *       = 0, main array index and value info; does not descend into sub-arrays.
	 *       > 0, descends into 'depth' sub-arrays, and prints index and value info.
	 */

	if (nargs == 2) {
		tmp = POP_NUMBER();
		depth = get_number_si(tmp);
		DEREF(tmp);
	}
	symbol = POP_PARAM();
	if (symbol->type != Node_var_array)
		fatal(_("%s: first argument is not an array"), "adump");

	ndump.type = Node_dump_array;
	ndump.adepth = depth;
	ndump.alevel = 0;
	assoc_dump(symbol, & ndump);
	return make_number((AWKNUM) 0);
}


/* asort_actual --- do the actual work to sort the input array */

static NODE *
asort_actual(int nargs, sort_context_t ctxt)
{
	NODE *array, *dest = NULL, *result;
	NODE *r, *subs, *s;
	NODE **list = NULL, **ptr;
	unsigned long num_elems, i;
	const char *sort_str;
	char save;

	if (nargs == 3)  /* 3rd optional arg */
		s = POP_STRING();
	else
		s = dupnode(Nnull_string);	/* "" => default sorting */

	s = force_string(s);
	sort_str = s->stptr;
	save = s->stptr[s->stlen];
	s->stptr[s->stlen] = '\0';
	if (s->stlen == 0) {		/* default sorting */
		if (ctxt == ASORT)
			sort_str = "@val_type_asc";
		else
			sort_str = "@ind_str_asc";
	}

	if (nargs >= 2) {  /* 2nd optional arg */
		dest = POP_PARAM();
		if (dest->type != Node_var_array) {
			fatal(_("%s: second argument is not an array"),
				ctxt == ASORT ? "asort" : "asorti");
		}
	}

	array = POP_PARAM();
	if (array->type != Node_var_array) {
		fatal(_("%s: first argument is not an array"),
			ctxt == ASORT ? "asort" : "asorti");
	}
	else if (array == symbol_table)
		fatal(_("%s: first argument cannot be SYMTAB"),
			ctxt == ASORT ? "asort" : "asorti");
	else if (array == func_table)
		fatal(_("%s: first argument cannot be FUNCTAB"),
			ctxt == ASORT ? "asort" : "asorti");

	if (dest != NULL) {
		for (r = dest->parent_array; r != NULL; r = r->parent_array) {
			if (r == array)
				fatal(_("%s: cannot use a subarray of first argument for second argument"),
					ctxt == ASORT ? "asort" : "asorti");
		}
		for (r = array->parent_array; r != NULL; r = r->parent_array) {
			if (r == dest)
				fatal(_("%s: cannot use a subarray of second argument for first argument"),
					ctxt == ASORT ? "asort" : "asorti");
		}
	}

	/* sorting happens inside assoc_list */
	list = assoc_list(array, sort_str, ctxt);
	s->stptr[s->stlen] = save;
	DEREF(s);

	num_elems = assoc_length(array);
	if (num_elems == 0 || list == NULL) {
 		/* source array is empty */
 		if (dest != NULL && dest != array)
 			assoc_clear(dest);
		if (list != NULL)
			efree(list);
 		return make_number((AWKNUM) 0);
 	}

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
		result = make_array();
		result->vname = array->vname;
		result->parent_array = array->parent_array;
	}

	if (ctxt == ASORTI) {
		/* We want the indices of the source array. */

		for (i = 1, ptr = list; i <= num_elems; i++, ptr += 2) {
			subs = make_number(i);
			assoc_set(result, subs, *ptr);
		}
	} else {
		/* We want the values of the source array. */

		for (i = 1, ptr = list; i <= num_elems; i++) {
			subs = make_number(i);

			/* free index node */
			r = *ptr++;
			unref(r);

			/* value node */
			r = *ptr++;

			NODE *value;

			if (r->type == Node_val)
				value = dupnode(r);
			else {
				NODE *arr;
				arr = make_array();
				subs = force_string(subs);
				arr->vname = subs->stptr;
				arr->vname[subs->stlen] = '\0';
				subs->stptr = NULL;
				subs->flags &= ~STRCUR;
				arr->parent_array = array; /* actual parent, not the temporary one. */

				value = assoc_copy(r, arr);
			}
			assoc_set(result, subs, value);
		}
	}

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


/*
 * cmp_strings --- compare two strings; logic similar to cmp_nodes() in eval.c
 *	except the extra case-sensitive comparison when the case-insensitive
 *	result is a match.
 */

static int
cmp_strings(const NODE *n1, const NODE *n2)
{
	char *s1, *s2;
	size_t len1, len2;
	int ret;

	s1 = n1->stptr;
	len1 = n1->stlen;
	s2 =  n2->stptr;
	len2 = n2->stlen;

	if (len1 == 0)
		return len2 == 0 ? 0 : -1;
	if (len2 == 0)
		return 1;

	/* len1 > 0 && len2 > 0 */
	// make const to ensure it doesn't change if we
	// need to call memcmp(), below
	const size_t lmin = len1 < len2 ? len1 : len2;

	if (IGNORECASE) {
		const unsigned char *cp1 = (const unsigned char *) s1;
		const unsigned char *cp2 = (const unsigned char *) s2;

		if (gawk_mb_cur_max > 1) {
			ret = strncasecmpmbs((const unsigned char *) cp1,
					     (const unsigned char *) cp2, lmin);
		} else {
			size_t count = lmin;

			for (ret = 0; count-- > 0 && ret == 0; cp1++, cp2++)
				ret = casetable[*cp1] - casetable[*cp2];
		}
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
	return cmp_strings(t1, t2);
}


/* sort_down_index_str --- qsort comparison function; descending index strings. */

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
	const NODE *t1, *t2;
	int ret;

	t1 = *((const NODE *const *) p1);
	t2 = *((const NODE *const *) p2);

	ret = cmp_numbers(t1, t2);
	if (ret != 0)
		return ret;

	/* break a tie with the index string itself */
	t1 = force_string((NODE *) t1);
	t2 = force_string((NODE *) t2);
	return cmp_strings(t1, t2);
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
	int ret;

	t1 = *((const NODE *const *) p1 + 1);
	t2 = *((const NODE *const *) p2 + 1);

	if (t1->type != Node_val || t2->type != Node_val)
		return sort_up_value_type(p1, p2);

	/* t1 and t2 both have string values */
	ret = cmp_strings(t1, t2);
	if (ret != 0)
		return ret;
	return sort_up_index_string(p1, p2);
}


/* sort_down_value_string --- qsort comparison function; descending value string */

static int
sort_down_value_string(const void *p1, const void *p2)
{
	return -sort_up_value_string(p1, p2);
}


/* sort_up_value_number --- qsort comparison function; ascending value number */

static int
sort_up_value_number(const void *p1, const void *p2)
{
	NODE *t1, *t2;
	int ret;

	t1 = *((NODE *const *) p1 + 1);
	t2 = *((NODE *const *) p2 + 1);

	if (t1->type != Node_val || t2->type != Node_val)
		return sort_up_value_type(p1, p2);

	ret = cmp_numbers(t1, t2);
	if (ret != 0)
		return ret;

	/*
	 * Use string value to guarantee same sort order on all
	 * versions of qsort().
	 */
	ret = cmp_strings(force_string(t1), force_string(t2));
	if (ret != 0)
		return ret;
	return sort_up_index_string(p1, p2);
}


/* sort_down_value_number --- qsort comparison function; descending value number */

static int
sort_down_value_number(const void *p1, const void *p2)
{
	return -sort_up_value_number(p1, p2);
}


/* do_sort_up_value_type --- backend comparison on ascending value type */

static int
do_sort_up_value_type(const void *p1, const void *p2)
{
	NODE *n1, *n2;

	static const NODETYPE element_types[] = {
		Node_builtin_func,
		Node_func,
		Node_ext_func,
		Node_var_new,
		Node_var,
		Node_var_array,
		Node_val,
		Node_illegal
	};

	/* we want to compare the element values */
	n1 = *((NODE *const *) p1 + 1);
	n2 = *((NODE *const *) p2 + 1);

	if (n1->type == Node_var && n2->type == Node_var) {
		/* compare the values of the variables */
		n1 = n1->var_value;
		n2 = n2->var_value;
	}

	/* 1. Arrays vs. everything else, everything else is less than array */
	if (n1->type == Node_var_array) {
		/* return 0 if n2 is a sub-array too, else return 1 */
		return (n2->type != Node_var_array);
	}
	if (n2->type == Node_var_array) {
		return -1;              /* n1 (non-array) < n2 (sub-array) */
	}

	/* 2. Non scalars */
	if (n1->type != Node_val || n2->type != Node_val) {
		int n1_pos, n2_pos, i;

		n1_pos = n2_pos = -1;
		for (i = 0; element_types[i] != Node_illegal; i++) {
			if (n1->type == element_types[i])
				n1_pos = i;

			if (n2->type == element_types[i])
				n2_pos = i;
		}

		assert(n1_pos != -1 && n2_pos != -1);
		return (n1_pos - n2_pos);
	}

	/* two scalars */
	(void) fixtype(n1);
	(void) fixtype(n2);

	if ((n1->flags & NUMBER) != 0 && (n2->flags & NUMBER) != 0) {
		return cmp_numbers(n1, n2);
	}

	/* 3. All numbers are less than all strings. This is aribitrary. */
	if ((n1->flags & NUMBER) != 0 && (n2->flags & STRING) != 0) {
		return -1;
	} else if ((n1->flags & STRING) != 0 && (n2->flags & NUMBER) != 0) {
		return 1;
	}

	/* 4. Two strings */
	return cmp_strings(n1, n2);
}

/* sort_up_value_type --- qsort comparison function; ascending value type */

static int
sort_up_value_type(const void *p1, const void *p2)
{
	int rc = do_sort_up_value_type(p1, p2);

	/* use a tie-breaker if do_sort_up_value_type has no opinion */
	return rc ? rc : sort_up_index_string(p1, p2);
}

/* sort_down_value_type --- qsort comparison function; descending value type */

static int
sort_down_value_type(const void *p1, const void *p2)
{
	return -sort_up_value_type(p1, p2);
}

/* sort_user_func --- user defined qsort comparison function */

static int
sort_user_func(const void *p1, const void *p2)
{
	NODE *idx1, *idx2, *val1, *val2, *r;
	int ret;
	INSTRUCTION *code;

	idx1 = *((NODE *const *) p1);
	idx2 = *((NODE *const *) p2);
	val1 = *((NODE *const *) p1 + 1);
	val2 = *((NODE *const *) p2 + 1);

	code = TOP()->code_ptr;	/* comparison function call instructions */

	/* setup 4 arguments to comp_func() */
	UPREF(idx1);
	PUSH(idx1);
	if (val1->type == Node_val)
		UPREF(val1);
	PUSH(val1);

	UPREF(idx2);
	PUSH(idx2);
	if (val2->type == Node_val)
		UPREF(val2);
	PUSH(val2);

	/* execute the comparison function */
	(void) (*interpret)(code);

	/* return value of the comparison function */
	r = POP_NUMBER();
#ifdef HAVE_MPFR
	/*
	 * mpfr_sgn(mpz_sgn): Returns a positive value if op > 0,
	 * zero if op = 0, and a negative value if op < 0.
	 */
	if (is_mpg_float(r))
		ret = mpfr_sgn(r->mpg_numbr);
	else if (is_mpg_integer(r))
		ret = mpz_sgn(r->mpg_i);
	else
#endif
		ret = (r->numbr < 0.0) ? -1 : (r->numbr > 0.0);
	DEREF(r);
	return ret;
}


/* assoc_list -- construct, and optionally sort, a list of array elements */

NODE **
assoc_list(NODE *symbol, const char *sort_str, sort_context_t sort_ctxt)
{
	typedef int (*qsort_compfunc)(const void *, const void *);

	static const struct qsort_funcs {
		const char *name;
		qsort_compfunc comp_func;
		assoc_kind_t kind;
	} sort_funcs[] = {
{ "@ind_str_asc",	sort_up_index_string,	AINDEX|AISTR|AASC },
{ "@ind_num_asc",	sort_up_index_number,	AINDEX|AINUM|AASC },
{ "@val_str_asc",	sort_up_value_string,	AVALUE|AVSTR|AASC },
{ "@val_num_asc",	sort_up_value_number,	AVALUE|AVNUM|AASC },
{ "@ind_str_desc",	sort_down_index_string,	AINDEX|AISTR|ADESC },
{ "@ind_num_desc",	sort_down_index_number,	AINDEX|AINUM|ADESC },
{ "@val_str_desc",	sort_down_value_string,	AVALUE|AVSTR|ADESC },
{ "@val_num_desc",	sort_down_value_number,	AVALUE|AVNUM|ADESC },
{ "@val_type_asc",	sort_up_value_type,	AVALUE|AASC },
{ "@val_type_desc",	sort_down_value_type,	AVALUE|ADESC },
{ "@unsorted",		0,			AINDEX },
};

	/*
	 * N.B.: AASC and ADESC are hints to the specific array types.
	 *	See cint_list() in cint_array.c.
	 */

	NODE **list;
	NODE akind;
	unsigned long num_elems, j;
	int elem_size, qi;
	qsort_compfunc cmp_func = 0;
	INSTRUCTION *code = NULL;
	extern int currule;
	int save_rule = 0;
	assoc_kind_t assoc_kind = ANONE;

	elem_size = 1;

	for (qi = 0, j = sizeof(sort_funcs)/sizeof(sort_funcs[0]); qi < j; qi++) {
		if (strcmp(sort_funcs[qi].name, sort_str) == 0)
			break;
	}

	if (qi < j) {
		cmp_func = sort_funcs[qi].comp_func;
		assoc_kind = sort_funcs[qi].kind;

		if (symbol->array_funcs != & cint_array_func)
			assoc_kind &= ~(AASC|ADESC);

		if (sort_ctxt != SORTED_IN || (assoc_kind & AVALUE) != 0) {
			/* need index and value pair in the list */

			assoc_kind |= (AINDEX|AVALUE);
			elem_size = 2;
		}

	} else {	/* unrecognized */
		NODE *f;
		const char *sp;

		for (sp = sort_str; *sp != '\0' && ! isspace((unsigned char) *sp); sp++)
			continue;

		/* empty string or string with space(s) not valid as function name */
		if (sp == sort_str || *sp != '\0')
			fatal(_("`%s' is invalid as a function name"), sort_str);

		f = lookup(sort_str);
		if (f == NULL || f->type != Node_func)
			fatal(_("sort comparison function `%s' is not defined"), sort_str);

		cmp_func = sort_user_func;

		/* need index and value pair in the list */
		assoc_kind |= (AVALUE|AINDEX);
		elem_size = 2;

		/* make function call instructions */
		code = bcalloc(Op_func_call, 2, 0);
		code->func_body = f;
		code->func_name = NULL;		/* not needed, func_body already assigned */
		(code + 1)->expr_count = 4;	/* function takes 4 arguments */
		code->nexti = bcalloc(Op_stop, 1, 0);

		/*
		 * make non-redirected getline, exit, `next' and `nextfile' fatal in
		 * callback function by setting currule in interpret()
		 * to undefined (0).
		 */

		save_rule = currule;	/* save current rule */
		currule = 0;

		PUSH_CODE(code);
	}

	akind.flags = (unsigned int) assoc_kind;	/* kludge */
	list = symbol->alist(symbol, & akind);
	assoc_kind = (assoc_kind_t) akind.flags;	/* symbol->alist can modify it */

	/* check for empty list or unsorted, or list already sorted */
	if (list != NULL && cmp_func != NULL && (assoc_kind & (AASC|ADESC)) == 0) {
		num_elems = assoc_length(symbol);

		qsort(list, num_elems, elem_size * sizeof(NODE *), cmp_func); /* shazzam! */

		if (sort_ctxt == SORTED_IN && (assoc_kind & (AINDEX|AVALUE)) == (AINDEX|AVALUE)) {
			/* relocate all index nodes to the first half of the list. */
			for (j = 1; j < num_elems; j++)
				list[j] = list[2 * j];

			/* give back extra memory */

			erealloc(list, NODE **, num_elems * sizeof(NODE *), "assoc_list");
		}
	}

	if (cmp_func == sort_user_func) {
		code = POP_CODE();
		currule = save_rule;            /* restore current rule */
		bcfree(code->nexti);            /* Op_stop */
		bcfree(code);                   /* Op_func_call */
	}

	return list;
}
