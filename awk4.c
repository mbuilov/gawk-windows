/*
 * awk4 -- Code for features in new AWK, System V compatibility.
 *
 * Copyright (C) 1988 Free Software Foundation
 * Written by David Trueman, 1988
 *
 * $Log:	awk4.c,v $
 * Revision 1.27  88/12/14  10:53:49  david
 * malloc structures in func_call and free them on return
 * 
 * Revision 1.26  88/12/13  22:29:15  david
 * minor change
 * 
 * Revision 1.25  88/12/08  10:52:01  david
 * small correction to #ifdef'ing
 * 
 * Revision 1.24  88/11/30  15:18:21  david
 * fooling around with memory allocation in func_call() but this new code remains
 * #ifdef'd out
 * correction to creasting private copy of string in do_sub()
 * 
 * Revision 1.23  88/11/29  09:55:48  david
 * corrections to code that tracks value of NF -- this needs cleanup
 * 
 * Revision 1.22  88/11/28  20:30:10  david
 * bug fix for do_sub when third arg. not specified
 * 
 * Revision 1.21  88/11/22  13:51:10  david
 * Arnold: delinting
 * 
 * Revision 1.20  88/11/15  10:25:14  david
 * Arnold: minor cleanup
 * 
 * Revision 1.19  88/11/14  22:00:09  david
 * Arnold: error message on bad regexp; correction to handling of RSTART in
 * match() on failure; return arg handling to previous behaviour: var=val
 * arg is processed whne it is encountered.
 * 
 * Revision 1.18  88/11/14  21:28:09  david
 * moved concat_exp(); update NF on assign to field > NF; temporarily aborted
 * mods. in func_call(); flag misnamed as TMP becomes PERM (don't free)
 * 
 * Revision 1.17  88/11/01  12:19:37  david
 * cleanup and substantial changes to do_sub()
 * 
 * Revision 1.16  88/10/19  21:59:20  david
 * replace malloc and realloc with error checking versions
 * 
 * Revision 1.15  88/10/17  20:54:54  david
 * SYSV --> USG
 * 
 * Revision 1.14  88/10/13  22:00:44  david
 * purge FAST and cleanup error messages
 * 
 * Revision 1.13  88/10/11  09:29:12  david
 * retrieve parameters from the stack in func_call
 * 
 * Revision 1.12  88/10/06  21:53:15  david
 * added FSF copyleft; Arnold's changes to command line processing
 * 
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

NODE *ARGC_node, *ARGV_node;
extern NODE **fields_arr;

jmp_buf func_tag;
NODE **stack_ptr;

NODE *
func_call(name, arg_list)
NODE *name;		/* name is a Node_val giving function name */
NODE *arg_list;		/* Node_expression_list of calling args. */
{
	register NODE *arg, *argp, *r;
	NODE *n, *f, *p;
	jmp_buf func_tag_stack;
	NODE *ret_node_stack;
	NODE **local_stack;
	NODE **sp;
	static int func_tag_valid = 0;
	int count;
	extern NODE *lookup(), *install();
	extern NODE *pop_var();
	extern NODE *ret_node;

	/*
	 * retrieve function definition node
	 */
	f = lookup(variables, name->stptr);
	if (f->type != Node_func)
		fatal("function `%s' not defined", name->stptr);
	/*
	 * mark stack for variables allocated during life of function
	 */
	count = f->lnode->param_cnt;
	emalloc(local_stack, NODE **, count * sizeof(NODE *), "func_call");
	sp = local_stack;

	/*
	 * for each calling arg. add NODE * on stack
	 */
	for (argp = arg_list; count && argp != NULL; argp = argp->rnode) {
		arg = argp->lnode;
		r = newnode(Node_var);
		/*
		 * call by reference for arrays; see below also
		 */
		if (arg->type == Node_param_list)
			arg = stack_ptr[arg->param_cnt];
		if (arg->type == Node_var_array)
			*r = *arg;
		else {
			n = dupnode(tree_eval(arg));
			r->lnode = n;
			r->rnode = (NODE *) NULL;
  		}
		*sp++ = r;
		count--;
	}
	if (argp != NULL)	/* left over calling args. */
		warning(
		    "function `%s' called with more arguments than declared",
		    name->stptr);
	/*
	 * add remaining params. on stack with null value
	 */
	while (count-- > 0) {
		r = newnode(Node_var);
		r->lnode = Nnull_string;
		r->rnode = (NODE *) NULL;
		*sp++ = r;
	}

	/*
	 * execute function body, saving context, as a return statement
	 * will longjmp back here
	 */
	sp = local_stack;
	local_stack = stack_ptr;
	stack_ptr = sp;
	PUSH_BINDING(func_tag_stack, func_tag, func_tag_valid);
	ret_node_stack = ret_node;
	if (_setjmp(func_tag) == 0)
		(void) interpret(f->rnode);
	r = ret_node;
	ret_node = ret_node_stack;
	RESTORE_BINDING(func_tag_stack, func_tag, func_tag_valid);
	sp = stack_ptr;
	stack_ptr = local_stack;
	local_stack = sp;

	/*
	 * here, we pop each parameter and check whether
	 * it was an array.  If so, and if the arg. passed in was
	 * a simple variable, then the value should be copied back.
	 * This achieves "call-by-reference" for arrays.
	 */
	count = f->lnode->param_cnt;
	for (argp = arg_list; count-- > 0 && argp != NULL; argp = argp->rnode) {
		arg = argp->lnode;
		n = *sp++;
		if (arg->type == Node_var && n->type == Node_var_array) {
			arg->var_array = n->var_array;
			arg->type = Node_var_array;
		}
		deref = n->lnode;
		do_deref();
		free((char *) n);
	}
	while (count-- > 0) {
		n = *sp++;
		deref = n->lnode;
		do_deref();
		free((char *) n);
	}
	free((char *) local_stack);
	return r;
}

NODE *
do_match(tree)
NODE *tree;
{
	NODE *t1;
	int rstart;
	struct re_registers reregs;
	struct re_pattern_buffer *rp;
	extern NODE *RSTART_node, *RLENGTH_node;

	t1 = force_string(tree_eval(tree->lnode));
	if (tree->rnode->type == Node_regex)
		rp = tree->rnode->rereg;
	else {
		rp = make_regexp(force_string(tree_eval(tree->rnode)));
		if (rp == NULL)
			cant_happen();
	}
	rstart = re_search(rp, t1->stptr, t1->stlen, 0, t1->stlen, &reregs);
	free_temp(t1);
	if (rstart >= 0) {
		rstart++;	/* 1-based indexing */
		/* RSTART set to rstart below */
		RLENGTH_node->var_value->numbr =
			(AWKNUM) (reregs.end[0] - reregs.start[0]);
	} else {
		/*
		 * Match failed. Set RSTART to 0, RLENGTH to -1.
		 * Return the value of RSTART.
		 */
		rstart = 0;	/* used as return value */
		RLENGTH_node->var_value->numbr = -1.0;
	}
	RSTART_node->var_value->numbr = (AWKNUM) rstart;
	return tmp_number((AWKNUM) rstart);
}

NODE *
do_sub(tree)
NODE *tree;
{
	register int len;
	register char *scan;
	register char *bp, *cp;
	int search_start = 0;
	int match_length;
	int matches = 0;
	char *buf;
	int global;
	struct re_pattern_buffer *rp;
	NODE *s;		/* subst. pattern */
	NODE *t;		/* string to make sub. in; $0 if none given */
	struct re_registers reregs;
	unsigned int saveflags;
	NODE *tmp;
	NODE **lhs;
	char *lastbuf;

	global = (tree->type == Node_gsub);

	if (tree->rnode->type == Node_regex)
		rp = tree->rnode->rereg;
	else {
		rp = make_regexp(force_string(tree_eval(tree->rnode)));
		if (rp == NULL)
			cant_happen();
	}
	tree = tree->lnode;
	s = force_string(tree_eval(tree->lnode));
	tree = tree->rnode;
	deref = 0;
	if (tree == NULL) {
		t = WHOLELINE;
		lhs = &fields_arr[0];
		field_num = 0;
		deref = t;
	} else {
		t = tree->lnode;
		lhs = get_lhs(t);
		t = force_string(tree_eval(t));
	}
	/*
	 * create a private copy of the string
	 */
	if (t->stref > 1 || (t->flags & PERM)) {
		saveflags = t->flags;
		t->flags &= ~MALLOC;
		tmp = dupnode(t);
		t->flags = saveflags;
		do_deref();
		t = tmp;
		if (lhs)
			*lhs = tmp;
	}
	lastbuf = t->stptr;
	do {
		if (re_search(rp, t->stptr, t->stlen, search_start,
		    t->stlen-search_start, &reregs) == -1
		    || reregs.start[0] == reregs.end[0])
			break;
		matches++;

		/*
		 * first, make a pass through the sub. pattern, to calculate
		 * the length of the string after substitution 
		 */
		match_length = reregs.end[0] - reregs.start[0];
		len = t->stlen - match_length;
		for (scan = s->stptr; scan < s->stptr + s->stlen; scan++)
			if (*scan == '&')
				len += match_length;
			else if (*scan == '\\' && *(scan+1) == '&') {
				scan++;
				len++;
			} else
				len++;
		emalloc(buf, char *, len + 1, "do_sub");
		bp = buf;

		/*
		 * now, create the result, copying in parts of the original
		 * string 
		 */
		for (scan = t->stptr; scan < t->stptr + reregs.start[0]; scan++)
			*bp++ = *scan;
		for (scan = s->stptr; scan < s->stptr + s->stlen; scan++)
			if (*scan == '&')
				for (cp = t->stptr + reregs.start[0]; cp < t->stptr + reregs.end[0]; cp++)
					*bp++ = *cp;
			else if (*scan == '\\' && *(scan+1) == '&') {
				scan++;
				*bp++ = *scan;
			} else
				*bp++ = *scan;
		search_start = bp - buf;
		for (scan = t->stptr + reregs.end[0]; scan < t->stptr + t->stlen; scan++)
			*bp++ = *scan;
		*bp = '\0';
		free(lastbuf);
		t->stptr = buf;
		lastbuf = buf;
		t->stlen = len;
	} while (global && search_start < t->stlen);

	free_temp(s);
	if (matches > 0) {
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		else if (field_num > 0) {
			node0_valid = 0;
			if (NF_node->var_value->numbr == -1 &&
			    field_num > NF_node->var_value->numbr)
				assign_number(&(NF_node->var_value),
					(AWKNUM) field_num);
		}
		t->flags &= ~NUM;
	}
	field_num = -1;
	return tmp_number((AWKNUM) matches);
}

init_args(argc0, argc, argv0, argv)
int argc0, argc;
char *argv0;
char **argv;
{
	int i, j;
	NODE **aptr;
	extern NODE **assoc_lookup();
	extern NODE *spc_var();
	extern NODE *make_string();
	extern NODE *make_number();
	extern NODE *tmp_number();

	ARGV_node = spc_var("ARGV", Nnull_string);
	aptr = assoc_lookup(ARGV_node, tmp_number(0.0));
	*aptr = make_string(argv0, strlen(argv0));
	for (i = argc0, j = 1; i < argc; i++) {
		aptr = assoc_lookup(ARGV_node, tmp_number((AWKNUM) j));
		*aptr = make_string(argv[i], strlen(argv[i]));
		j++;
	}
	ARGC_node = spc_var("ARGC", make_number((AWKNUM) j));
}

#ifdef USG
int 
bcopy (src, dst, length)
register char *src, *dst;
register int length;
{
	(void) memcpy (dst, src, length);
}

int 
bzero (b, length)
register char *b;
register int length;
{
	(void) memset (b, '\0', length);
}
#endif
