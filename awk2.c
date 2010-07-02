/*
 * awk2 --- gawk parse tree interpreter 
 *
 * Copyright (C) 1986 Free Software Foundation Written by Paul Rubin, August
 * 1986 
 *
 * $Log:	awk2.c,v $
 * Revision 1.47  89/03/22  22:09:50  david
 * a cleaner way to handle assignment to $n where n > 0
 * 
 * Revision 1.46  89/03/22  21:01:14  david
 * delete some obsolete code
 * 
 * Revision 1.45  89/03/21  19:25:37  david
 * minor cleanup
 * 
 * Revision 1.44  89/03/21  18:24:02  david
 * bug fix in cmp_nodes: strings in which one was a prefix of the other compared equal
 * 
 * Revision 1.43  89/03/21  10:55:55  david
 * cleanup and fix of string comparison (0 length was wrong)
 * 
 * Revision 1.42  89/03/15  22:01:17  david
 * old case stuff removed
 * new case stuff added
 * fixed % operator
 * strings with embedded \0 can now be compared
 * 
 * Revision 1.41  89/03/15  21:32:50  david
 * try to free up memory in as many places as possible
 * 
 * Revision 1.40  88/12/15  12:57:31  david
 * make casetable static
 * 
 * Revision 1.39  88/12/14  10:50:51  david
 * dupnode() the return from a function
 * 
 * Revision 1.38  88/12/13  22:27:04  david
 * macro-front-end tree_eval and other optimizations
 * 
 * Revision 1.36  88/12/08  10:51:37  david
 * small correction to source file code
 * 
 * Revision 1.35  88/12/07  20:00:35  david
 * changes for incorporating source filename into error messages
 * 
 * Revision 1.34  88/12/01  15:04:48  david
 * cleanup and additions for source line number printing in error messages
 * 
 * Revision 1.33  88/11/30  15:16:10  david
 * merge FREE_ONE_REFERENCE into do_deref()
 * free more in do_deref
 * in for (i in array) loops, make sure value of i gets freed on each iteration
 * 
 * Revision 1.32  88/11/29  09:55:04  david
 * corrections to code that tracks value of NF -- this needs cleanup
 * 
 * Revision 1.31  88/11/23  21:40:47  david
 * Arnold: comment cleanup
 * 
 * Revision 1.30  88/11/22  13:49:09  david
 * Arnold: changes for case-insensitive matching
 * 
 * Revision 1.29  88/11/15  10:22:42  david
 * Arnold: cleanup of comments and #include's
 * 
 * Revision 1.28  88/11/14  21:55:38  david
 * Arnold: misc. cleanup and error message on bad regexp
 * 
 * Revision 1.27  88/11/14  21:26:52  david
 * update NF on assignment to a field greater than current NF
 * 
 * Revision 1.26  88/11/03  15:26:20  david
 * simplify call to in_array(); extensive revision of cmp_nodes and is_a_number
 * 
 * Revision 1.25  88/11/01  12:11:57  david
 * DEBUG macro becomes DBG_P; added some debugging code; moved all the
 * compound assignments (+= etc.) into op_assign()
 * 
 * Revision 1.24  88/10/25  10:43:05  david
 * intermediate state: more code movement; Node_string et al. -> Node_val;
 * add more debugging code; improve cmp_nodes
 * 
 * Revision 1.22  88/10/19  21:57:41  david
 * replace malloc and realloc with error checking versions
 * start to change handling of $0
 * 
 * Revision 1.21  88/10/17  20:56:13  david
 * Arnold: better error messages for use of a function in the wrong context
 * 
 * Revision 1.20  88/10/13  21:56:41  david
 * cleanup of previous changes
 * change panic() to fatal()
 * detect and bomb on function call with space between name and opening (
 * 
 * Revision 1.19  88/10/11  22:19:20  david
 * cleanup 
 * 
 * Revision 1.18  88/10/04  21:31:33  david
 * minor cleanup
 * 
 * Revision 1.17  88/08/22  14:01:19  david
 * fix to set_field() from Jay Finlayson
 * 
 * Revision 1.16  88/08/09  14:51:34  david
 * removed bad call to obstack_free() -- there is a lot of memory that is
 * not being properly freed -- this area needs major work
 * changed semantics in eval_condition -- if(expr) should test true if
 * expr is a non-null string, even if the num,erical value is zero -- counter-
 * intuitive but that's what's in the book
 * 
 * Revision 1.15  88/06/13  18:02:58  david
 * separate exit value from fact that exit has been called [from Arnold]
 * 
 * Revision 1.14  88/06/07  23:39:48  david
 * insubstantial changes
 * 
 * Revision 1.13  88/06/06  11:26:39  david
 * get rid of some obsolete code
 * change interface of set_field()
 * 
 * Revision 1.12  88/06/05  22:21:36  david
 * local variables are now kept on a stack
 * 
 * Revision 1.11  88/06/01  22:06:50  david
 * make sure incases of Node_param_list that the variable is looked up
 * 
 * Revision 1.10  88/05/31  09:29:47  david
 * expunge Node_local_var
 * 
 * Revision 1.9  88/05/30  09:52:55  david
 * be prepared for NULL return from make_regexp()
 * fix fatal() call
 * 
 * Revision 1.8  88/05/26  22:48:48  david
 * fixed regexp matching code
 * 
 * Revision 1.7  88/05/16  21:27:09  david
 * comment out obstack_free in interpret() -- it is done in do_file() anyway
 * and was definitely free'ing stuff it shouldn't have
 * change call of func_call() a bit
 * allow get_lhs to be called with other Node types -- return 0; used in
 * do_sub()
 * 
 * Revision 1.6  88/05/13  22:00:03  david
 * generalized *_BINDING macros and moved them to awk.h
 * changes to function calling (mostly elsewhere)
 * put into use the Node_var_array type
 * 
 * Revision 1.5  88/05/09  21:22:27  david
 * finally (I hope) got the code right in assign_number
 * 
 * Revision 1.4  88/05/04  12:23:30  david
 * fflush(stdout) on prints if FAST not def'ed
 * all the assign_* cases were returning the wrong thing
 * fixed Node_in_array code
 * code in assign_number was freeing memory it shouldn't have
 * 
 * Revision 1.3  88/04/15  13:12:38  david
 * additional error message
 * 
 * Revision 1.2  88/04/12  16:03:24  david
 * fixed previously intoduced bug: all matches succeeded
 * 
 * Revision 1.1  88/04/08  15:15:01  david
 * Initial revision
 *  Revision 1.7  88/04/08  14:48:33  david changes from
 * Arnold Robbins 
 *
 * Revision 1.6  88/03/28  14:13:50  david *** empty log message *** 
 *
 * Revision 1.5  88/03/23  22:17:37  david mostly delinting -- a couple of bug
 * fixes 
 *
 * Revision 1.4  88/03/18  21:00:10  david Baseline -- hoefully all the
 * functionality of the new awk added. Just debugging and tuning to do. 
 *
 * Revision 1.3  87/11/14  15:16:21  david added user-defined functions with
 * return and do-while loops 
 *
 * Revision 1.2  87/10/29  21:45:44  david added support for array membership
 * test, as in:  if ("yes" in answers) ... this involved one more case: for
 * Node_in_array and rearrangment of the code in assoc_lookup, so that the
 * element can be located without being created 
 *
 * Revision 1.1  87/10/27  15:23:28  david Initial revision 
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

extern void do_print();
extern NODE *do_printf();
extern NODE *func_call();
extern NODE *do_match();
extern NODE *do_sub();
extern NODE *do_getline();
extern int in_array();
extern void do_delete();

extern double pow();

static int eval_condition();
static int is_a_number();
static NODE *op_assign();

NODE *_t;		/* used as a temporary in macros */
NODE *_result;		/* holds result of tree_eval, for possible freeing */
NODE *ret_node;

/* More of that debugging stuff */
#ifdef	DEBUG
#define DBG_P(X) print_debug X
#else
#define DBG_P(X)
#endif

extern jmp_buf func_tag;

/*
 * This table is used by the regexp routines to do case independant
 * matching. Basically, every ascii character maps to itself, except
 * uppercase letters map to lower case ones. This table has 256
 * entries, which may be overkill. Note also that if the system this
 * is compiled on doesn't use 7-bit ascii, casetable[] should not be
 * defined to the linker, so gawk should not load.
 *
 * Do NOT make this array static, it is used in several spots, not
 * just in this file.
 */
#if 'a' == 97	/* it's ascii */
char casetable[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	/* ' '     '!'     '"'     '#'     '$'     '%'     '&'     ''' */
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	/* '('     ')'     '*'     '+'     ','     '-'     '.'     '/' */
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	/* '0'     '1'     '2'     '3'     '4'     '5'     '6'     '7' */
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	/* '8'     '9'     ':'     ';'     '<'     '='     '>'     '?' */
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	/* '@'     'A'     'B'     'C'     'D'     'E'     'F'     'G' */
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	/* 'H'     'I'     'J'     'K'     'L'     'M'     'N'     'O' */
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	/* 'P'     'Q'     'R'     'S'     'T'     'U'     'V'     'W' */
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	/* 'X'     'Y'     'Z'     '['     '\'     ']'     '^'     '_' */
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	/* '`'     'a'     'b'     'c'     'd'     'e'     'f'     'g' */
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	/* 'h'     'i'     'j'     'k'     'l'     'm'     'n'     'o' */
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	/* 'p'     'q'     'r'     's'     't'     'u'     'v'     'w' */
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	/* 'x'     'y'     'z'     '{'     '|'     '}'     '~' */
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
	'\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
	'\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
	'\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};
#else
#include "You lose. You will need a translation table for your character set."
#endif

/*
 * Tree is a bunch of rules to run. Returns zero if it hit an exit()
 * statement 
 */
interpret(tree)
NODE *tree;
{
	register NODE *t;	/* temporary */

	auto jmp_buf loop_tag_stack;	/* shallow binding stack for loop_tag */
	static jmp_buf loop_tag;/* always the current binding */
	static int loop_tag_valid = 0;	/* nonzero when loop_tag valid */

	static jmp_buf rule_tag;/* tag the rule currently being run, for NEXT
				 * and EXIT statements.  It is static because
				 * there are no nested rules */

	register NODE **lhs;	/* lhs == Left Hand Side for assigns, etc */
	register struct search *l;	/* For array_for */


	extern NODE **fields_arr;
	extern int exiting, exit_val;

	/*
	 * clean up temporary strings created by evaluating expressions in
	 * previous recursive calls 
	 */

	if (tree == NULL)
		return 1;
	sourceline = tree->source_line;
	source = tree->source_file;
	switch (tree->type) {
	case Node_rule_list:
		for (t = tree; t != NULL; t = t->rnode) {
			tree = t->lnode;
			switch (_setjmp(rule_tag)) {
			case 0:	/* normal non-jump */
				if (eval_condition(tree->lnode)) { /* pattern */
					DBG_P(("Found a rule", tree->rnode));
					if (tree->rnode == NULL) {
						/*
						 * special case: pattern with
						 * no action is equivalent to
						 * an action of {print}
						 */
						NODE printnode;

						printnode.type = Node_K_print;
						printnode.lnode = NULL;
						printnode.rnode = NULL;
						do_print(&printnode);
					} else if (tree->rnode->type == Node_illegal) {
						/*
						 * An empty statement
						 * (``{ }'') is different
						 * from a missing statement.
						 * A missing statement is
						 * equal to ``{ print }'' as
						 * above, but an empty
						 * statement is as in C, do
						 * nothing.
						 */
					} else
						(void) interpret(t->lnode->rnode);
				}
				break;
			case TAG_CONTINUE:	/* NEXT statement */
				return 1;
			case TAG_BREAK:
				return 0;
			}
		}
		break;

	case Node_statement_list:
		for (t = tree; t != NULL; t = t->rnode) {
			DBG_P(("Statements", t->lnode));
			(void) interpret(t->lnode);
		}
		break;

	case Node_K_if:
		DBG_P(("IF", tree->lnode));
		if (eval_condition(tree->lnode)) {
			DBG_P(("True", tree->rnode->lnode));
			(void) interpret(tree->rnode->lnode);
		} else {
			DBG_P(("False", tree->rnode->rnode));
			(void) interpret(tree->rnode->rnode);
		}
		break;

	case Node_K_while:
		PUSH_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);

		DBG_P(("WHILE", tree->lnode));
		while (eval_condition(tree->lnode)) {
			switch (_setjmp(loop_tag)) {
			case 0:	/* normal non-jump */
				DBG_P(("DO", tree->rnode));
				(void) interpret(tree->rnode);
				break;
			case TAG_CONTINUE:	/* continue statement */
				break;
			case TAG_BREAK:	/* break statement */
				RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
				return 1;
			default:
				cant_happen();
			}
		}
		RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
		break;

	case Node_K_do:
		PUSH_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);

		do {
			switch (_setjmp(loop_tag)) {
			case 0:	/* normal non-jump */
				DBG_P(("DO", tree->rnode));
				(void) interpret(tree->rnode);
				break;
			case TAG_CONTINUE:	/* continue statement */
				break;
			case TAG_BREAK:	/* break statement */
				RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
				return 1;
			default:
				cant_happen();
			}
			DBG_P(("WHILE", tree->lnode));
		} while (eval_condition(tree->lnode));
		RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
		break;

	case Node_K_for:
		PUSH_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);

		DBG_P(("FOR", tree->forloop->init));
		(void) interpret(tree->forloop->init);

		DBG_P(("FOR.WHILE", tree->forloop->cond));
		while (eval_condition(tree->forloop->cond)) {
			switch (_setjmp(loop_tag)) {
			case 0:	/* normal non-jump */
				DBG_P(("FOR.DO", tree->lnode));
				(void) interpret(tree->lnode);
				/* fall through */
			case TAG_CONTINUE:	/* continue statement */
				DBG_P(("FOR.INCR", tree->forloop->incr));
				(void) interpret(tree->forloop->incr);
				break;
			case TAG_BREAK:	/* break statement */
				RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
				return 1;
			default:
				cant_happen();
			}
		}
		RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
		break;

	case Node_K_arrayfor:
#define hakvar forloop->init
#define arrvar forloop->incr
		PUSH_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
		DBG_P(("AFOR.VAR", tree->hakvar));
		lhs = get_lhs(tree->hakvar, 1);
		t = tree->arrvar;
		if (tree->arrvar->type == Node_param_list)
			t = stack_ptr[tree->arrvar->param_cnt];
		for (l = assoc_scan(t); l; l = assoc_next(l)) {
			deref = *lhs;
			do_deref();
			*lhs = dupnode(l->retval);
			if (field_num == 0)
				set_record(fields_arr[0]->stptr,
				    fields_arr[0]->stlen);
			DBG_P(("AFOR.NEXTIS", *lhs));
			switch (_setjmp(loop_tag)) {
			case 0:
				DBG_P(("AFOR.DO", tree->lnode));
				(void) interpret(tree->lnode);
			case TAG_CONTINUE:
				break;

			case TAG_BREAK:
				RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
				field_num = -1;
				return 1;
			default:
				cant_happen();
			}
		}
		field_num = -1;
		RESTORE_BINDING(loop_tag_stack, loop_tag, loop_tag_valid);
		break;

	case Node_K_break:
		DBG_P(("BREAK", NULL));
		if (loop_tag_valid == 0)
			fatal("unexpected break");
		_longjmp(loop_tag, TAG_BREAK);
		break;

	case Node_K_continue:
		DBG_P(("CONTINUE", NULL));
		if (loop_tag_valid == 0)
			fatal("unexpected continue");
		_longjmp(loop_tag, TAG_CONTINUE);
		break;

	case Node_K_print:
		DBG_P(("PRINT", tree));
		do_print(tree);
		break;

	case Node_K_printf:
		DBG_P(("PRINTF", tree));
		(void) do_printf(tree);
		break;

	case Node_K_next:
		DBG_P(("NEXT", NULL));
		_longjmp(rule_tag, TAG_CONTINUE);
		break;

	case Node_K_exit:
		/*
		 * In A,K,&W, p. 49, it says that an exit statement "...
		 * causes the program to behave as if the end of input had
		 * occurred; no more input is read, and the END actions, if
		 * any are executed." This implies that the rest of the rules
		 * are not done. So we immediately break out of the main loop.
		 */
		DBG_P(("EXIT", NULL));
		exiting = 1;
		if (tree)
			exit_val = (int) force_number(tree_eval(tree->lnode));
		free_result();
		_longjmp(rule_tag, TAG_BREAK);
		break;

	case Node_K_return:
		DBG_P(("RETURN", NULL));
		ret_node = dupnode(tree_eval(tree->lnode));
		ret_node->flags |= TEMP;
		_longjmp(func_tag, TAG_RETURN);
		break;

	default:
		/*
		 * Appears to be an expression statement.  Throw away the
		 * value. 
		 */
		DBG_P(("E", NULL));
		(void) tree_eval(tree);
		free_result();
		break;
	}
	return 1;
}

/* evaluate a subtree, allocating strings on a temporary stack. */

NODE *
r_tree_eval(tree)
NODE *tree;
{
	register NODE *r, *t1, *t2;	/* return value & temporary subtrees */
	int i;
	register NODE **lhs;
	int di;
	AWKNUM x, x2;
	long lx;
	struct re_pattern_buffer *rp;
	extern NODE **fields_arr;

	if (tree->type != Node_var)
		source = tree->source_file;
	sourceline = tree->source_line;
	switch (tree->type) {
	case Node_and:
		DBG_P(("AND", tree));
		return tmp_number((AWKNUM) (eval_condition(tree->lnode)
					    && eval_condition(tree->rnode)));

	case Node_or:
		DBG_P(("OR", tree));
		return tmp_number((AWKNUM) (eval_condition(tree->lnode)
					    || eval_condition(tree->rnode)));

	case Node_not:
		DBG_P(("NOT", tree));
		return tmp_number((AWKNUM) ! eval_condition(tree->lnode));

		/* Builtins */
	case Node_builtin:
		DBG_P(("builtin", tree));
		return ((*tree->proc) (tree->subnode));

	case Node_K_getline:
		DBG_P(("GETLINE", tree));
		return (do_getline(tree));

	case Node_in_array:
		DBG_P(("IN_ARRAY", tree));
		return tmp_number((AWKNUM) in_array(tree->lnode, tree->rnode));

	case Node_K_match:
		DBG_P(("MATCH", tree));
		return do_match(tree);

	case Node_sub:
	case Node_gsub:
		DBG_P(("SUB", tree));
		return do_sub(tree);

	case Node_func_call:
		DBG_P(("func_call", tree));
		return func_call(tree->rnode, tree->lnode);

	case Node_K_delete:
		DBG_P(("DELETE", tree));
		do_delete(tree->lnode, tree->rnode);
		return Nnull_string;

		/* unary operations */

	case Node_var:
	case Node_var_array:
	case Node_param_list:
	case Node_subscript:
	case Node_field_spec:
		DBG_P(("var_type ref", tree));
		lhs = get_lhs(tree, 0);
		field_num = -1;
		deref = 0;
		return *lhs;

	case Node_unary_minus:
		DBG_P(("UMINUS", tree));
		x = -force_number(tree_eval(tree->subnode));
		free_result();
		return tmp_number(x);

	case Node_cond_exp:
		DBG_P(("?:", tree));
		if (eval_condition(tree->lnode)) {
			DBG_P(("True", tree->rnode->lnode));
			return tree_eval(tree->rnode->lnode);
		} else {
			DBG_P(("False", tree->rnode->rnode));
			return tree_eval(tree->rnode->rnode);
		}
		break;

	case Node_match:
	case Node_nomatch:
		DBG_P(("ASSIGN_[no]match", tree));
		t1 = force_string(tree_eval(tree->lnode));
		if (tree->rnode->type == Node_regex) {
			rp = tree->rnode->rereg;
			if (!strict && ((IGNORECASE_node->var_value->numbr != 0)
			    ^ (tree->rnode->re_case != 0))) {
				/* recompile since case sensitivity differs */
				rp = tree->rnode->rereg =
					mk_re_parse(tree->rnode->re_text,
					(IGNORECASE_node->var_value->numbr != 0));
				tree->rnode->re_case = (IGNORECASE_node->var_value->numbr != 0);
			}
		} else {
			rp = make_regexp(force_string(tree_eval(tree->rnode)),
					(IGNORECASE_node->var_value->numbr != 0));
			if (rp == NULL)
				cant_happen();
		}
		i = re_search(rp, t1->stptr, t1->stlen, 0, t1->stlen,
			(struct re_registers *) NULL);
		i = (i == -1) ^ (tree->type == Node_match);
		free_temp(t1);
		return tmp_number((AWKNUM) i);

	case Node_func:
		fatal("function `%s' called with space between name and (,\n%s",
			tree->lnode->param,
			"or used in other expression context");

		/* assignments */
	case Node_assign:
		DBG_P(("ASSIGN", tree));
		r = tree_eval(tree->rnode);
		lhs = get_lhs(tree->lnode, 1);
		*lhs = dupnode(r);
		do_deref();
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		field_num = -1;
		return *lhs;

		/* other assignment types are easier because they are numeric */
	case Node_preincrement:
	case Node_predecrement:
	case Node_postincrement:
	case Node_postdecrement:
	case Node_assign_exp:
	case Node_assign_times:
	case Node_assign_quotient:
	case Node_assign_mod:
	case Node_assign_plus:
	case Node_assign_minus:
		return op_assign(tree);
	}

	/*
	 * Note that if TREE is invalid, gawk will probably bomb in one of
	 * these tree_evals here.  
	 */
	/* evaluate subtrees in order to do binary operation, then keep going */
	t1 = tree_eval(tree->lnode);
	t2 = tree_eval(tree->rnode);

	switch (tree->type) {
	case Node_concat:
		DBG_P(("CONCAT", tree));
		t1 = force_string(t1);
		t2 = force_string(t2);

		r = newnode(Node_val);
		r->flags = (STR|TEMP);
		r->stlen = t1->stlen + t2->stlen;
		r->stref = 1;
		emalloc(r->stptr, char *, r->stlen + 1, "tree_eval");
		bcopy(t1->stptr, r->stptr, t1->stlen);
		bcopy(t2->stptr, r->stptr + t1->stlen, t2->stlen);
		r->stptr[r->stlen] = '\0';
		free_temp(t1);
		free_temp(t2);
		return r;

	case Node_geq:
	case Node_leq:
	case Node_greater:
	case Node_less:
	case Node_notequal:
	case Node_equal:
		di = cmp_nodes(t1, t2);
		free_temp(t1);
		free_temp(t2);
		switch (tree->type) {
		case Node_equal:
			DBG_P(("EQUAL", tree));
			return tmp_number((AWKNUM) (di == 0));
		case Node_notequal:
			DBG_P(("NOT_EQUAL", tree));
			return tmp_number((AWKNUM) (di != 0));
		case Node_less:
			DBG_P(("LESS_THAN", tree));
			return tmp_number((AWKNUM) (di < 0));
		case Node_greater:
			DBG_P(("GREATER_THAN", tree));
			return tmp_number((AWKNUM) (di > 0));
		case Node_leq:
			DBG_P(("LESS_THAN_EQUAL", tree));
			return tmp_number((AWKNUM) (di <= 0));
		case Node_geq:
			DBG_P(("GREATER_THAN_EQUAL", tree));
			return tmp_number((AWKNUM) (di >= 0));
		}
		break;
	}

	(void) force_number(t1);
	(void) force_number(t2);

	switch (tree->type) {
	case Node_exp:
		DBG_P(("EXPONENT", tree));
		x = pow((double) t1->numbr, (double) t2->numbr);
		free_temp(t1);
		free_temp(t2);
		return tmp_number(x);

	case Node_times:
		DBG_P(("MULT", tree));
		x = t1->numbr * t2->numbr;
		free_temp(t1);
		free_temp(t2);
		return tmp_number(x);

	case Node_quotient:
		DBG_P(("DIVIDE", tree));
		x = t2->numbr;
		free_temp(t2);
		if (x == (AWKNUM) 0) {
			free_temp(t1);
			return tmp_number((AWKNUM) 0);
		} else {
			x = t1->numbr / x;
			free_temp(t1);
			return tmp_number(x);
		}

	case Node_mod:
		DBG_P(("MODULUS", tree));
		x = t2->numbr;
		free_temp(t2);
		if (x == (AWKNUM) 0) {
			free_temp(t1);
			return tmp_number((AWKNUM) 0);
		}
		lx = t1->numbr / x;	/* assignment to long truncates */
		x2 = lx * x;
		x = t1->numbr - x2;
		free_temp(t1);
		return tmp_number(x);

	case Node_plus:
		DBG_P(("PLUS", tree));
		x = t1->numbr + t2->numbr;
		free_temp(t1);
		free_temp(t2);
		return tmp_number(x);

	case Node_minus:
		DBG_P(("MINUS", tree));
		x = t1->numbr - t2->numbr;
		free_temp(t1);
		free_temp(t2);
		return tmp_number(x);

	default:
		fatal("illegal type (%d) in tree_eval", tree->type);
	}
	return 0;
}

/*
 * This makes numeric operations slightly more efficient. Just change the
 * value of a numeric node, if possible 
 */
void
assign_number(ptr, value)
NODE **ptr;
AWKNUM value;
{
	extern NODE *deref;

#ifdef DEBUG
	if ((*ptr)->type != Node_val)
		cant_happen();
#endif
	if (*ptr == Nnull_string) {
		*ptr = make_number(value);
		deref = 0;
		return;
	}
	if ((*ptr)->stref > 1) {
		*ptr = make_number(value);
		return;
	}
	(*ptr)->numbr = value;
	(*ptr)->flags |= NUM;
	(*ptr)->flags &= ~STR;
	(*ptr)->stref = 0;
	deref = 0;
}


/* Is TREE true or false?  Returns 0==false, non-zero==true */
static int
eval_condition(tree)
NODE *tree;
{
	register NODE *t1;
	int ret;

	if (tree == NULL)	/* Null trees are the easiest kinds */
		return 1;
	if (tree->type == Node_line_range) {
		/*
		 * Node_line_range is kind of like Node_match, EXCEPT: the
		 * lnode field (more properly, the condpair field) is a node
		 * of a Node_cond_pair; whether we evaluate the lnode of that
		 * node or the rnode depends on the triggered word.  More
		 * precisely:  if we are not yet triggered, we tree_eval the
		 * lnode; if that returns true, we set the triggered word. 
		 * If we are triggered (not ELSE IF, note), we tree_eval the
		 * rnode, clear triggered if it succeeds, and perform our
		 * action (regardless of success or failure).  We want to be
		 * able to begin and end on a single input record, so this
		 * isn't an ELSE IF, as noted above.
		 */
		if (!tree->triggered)
			if (!eval_condition(tree->condpair->lnode))
				return 0;
			else
				tree->triggered = 1;
		/* Else we are triggered */
		if (eval_condition(tree->condpair->rnode))
			tree->triggered = 0;
		return 1;
	}

	/*
	 * Could just be J.random expression. in which case, null and 0 are
	 * false, anything else is true 
	 */

	t1 = tree_eval(tree);
	if (t1->flags & STR)
		ret = t1->stlen != 0;
	else
		ret = t1->numbr != 0.0;
	free_temp(t1);
	return ret;
}

/*
 * strtod() would have been better, except (1) real awk is needlessly
 * restrictive in what strings it will consider to be numbers, and (2) I
 * couldn't find the public domain version anywhere handy. 
 */
static int
is_a_number(str)	/* does the string str have pure-numeric syntax? */
char *str;		/* don't convert it, assume that atof is better */
{
	if (*str == 0)
		return 0;	/* null string is not equal to 0 */

	if (*str == '-')
		str++;
	if (*str == 0)
		return 0;
	/* must be either . or digits (.4 is legal) */
	if (*str != '.' && !isdigit(*str))
		return 0;
	while (isdigit(*str))
		str++;
	if (*str == '.') {
		str++;
		while (isdigit(*str))
			str++;
	}

	/*
	 * curiously, real awk DOESN'T consider "1E1" to be equal to 10! Or
	 * even equal to 1E1 for that matter!  For a laugh, try:
	 * awk 'BEGIN {if ("1E1" == 1E1) print "eq"; else print "neq"; exit}'
	 * Since this behavior is QUITE curious, I include the code for the
	 * adventurous. One might also feel like skipping leading whitespace
	 * (awk doesn't) and allowing a leading + (awk doesn't).
	 */
#ifdef Allow_Exponents
	if (*str == 'e' || *str == 'E') {
		str++;
		if (*str == '+' || *str == '-')
			str++;
		if (!isdigit(*str))
			return 0;
		while (isdigit(*str))
			str++;
	}
#endif
	/*
	 * if we have digested the whole string, we are
	 * successful 
	 */
	return (*str == 0);
}

int
cmp_nodes(t1, t2)
NODE *t1, *t2;
{
	AWKNUM d;
	int ret;
	int len1, len2;

	if (t1 == t2)
		return 0;
	if ((t1->flags & NUM)) {
		if ((t2->flags & NUM))
			d = t1->numbr - t2->numbr;
		else if (is_a_number(t2->stptr))
			d = t1->numbr - force_number(t2);
		else {
			t1 = force_string(t1);
			goto strings;
		}
		if (d == 0.0)	/* from profiling, this is most common */
			return 0;
		if (d > 0.0)
			return 1;
		return -1;
	}
	if ((t2->flags & NUM)) {
		if (is_a_number(t1->stptr))
			d = force_number(t1) - t2->numbr;
		else {
			t2 = force_string(t2);
			goto strings;
		}
		if (d == 0.0)	/* from profiling, this is most common */
			return 0;
		if (d > 0.0)
			return 1;
		return -1;
	}
	if (is_a_number(t1->stptr) && is_a_number(t2->stptr)) {
		/*
		 * following two statements are this way because force_number
		 * is a macro
		 */
		d = force_number(t1);
		d = d - force_number(t2);
		if (d == 0.0)	/* from profiling, this is most common */
			return 0;
		if (d > 0.0)
			return 1;
		return -1;
	}

strings:
	len1 = t1->stlen;
	len2 = t2->stlen;
	if (len1 == 0) {
		if (len2 == 0)
			return 0;
		else
			return -1;
	} else if (len2 == 0)
		return 1;
	ret = memcmp(t1->stptr, t2->stptr, len1 <= len2 ? len1 : len2);
	if (ret == 0 && len1 != len2)
		return len1 < len2 ? -1: 1;
	return ret;
}

#ifdef NOMEMCMP
/*
 * memcmp --- compare strings.
 *
 * We use our own routine since it has to act like strcmp() for return
 * value, and the BSD manual says bcmp() only returns zero/non-zero.
 */

static int
memcmp (s1, s2, l)
register char *s1, *s2;
register int l;
{
	for (; l--; s1++, s2++) {
		if (*s1 != *s2)
			return (*s1 - *s2);
	}
	return (*--s1 - *--s2);
}
#endif

static NODE *
op_assign(tree)
NODE *tree;
{
	AWKNUM rval, lval;
	NODE **lhs;
	AWKNUM t1, t2;
	long ltemp;

	lhs = get_lhs(tree->lnode, 1);
	lval = force_number(*lhs);

	switch(tree->type) {
	case Node_preincrement:
	case Node_predecrement:
		DBG_P(("+-X", tree));
		assign_number(lhs,
		    lval + (tree->type == Node_preincrement ? 1.0 : -1.0));
		do_deref();
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		field_num = -1;
		return *lhs;
		break;

	case Node_postincrement:
	case Node_postdecrement:
		DBG_P(("X+-", tree));
		assign_number(lhs,
		    lval + (tree->type == Node_postincrement ? 1.0 : -1.0));
		do_deref();
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		field_num = -1;
		return tmp_number(lval);
	}

	rval = force_number(tree_eval(tree->rnode));
	free_result();
	switch(tree->type) {
	case Node_assign_exp:
		DBG_P(("ASSIGN_exp", tree));
		assign_number(lhs, (AWKNUM) pow((double) lval, (double) rval));
		break;

	case Node_assign_times:
		DBG_P(("ASSIGN_times", tree));
		assign_number(lhs, lval * rval);
		break;

	case Node_assign_quotient:
		DBG_P(("ASSIGN_quotient", tree));
		assign_number(lhs, lval / rval);
		break;

	case Node_assign_mod:
		DBG_P(("ASSIGN_mod", tree));
		ltemp = lval / rval;	/* assignment to long truncates */
		t1 = ltemp * rval;
		t2 = lval - t1;
		assign_number(lhs, t2);
		break;

	case Node_assign_plus:
		DBG_P(("ASSIGN_plus", tree));
		assign_number(lhs, lval + rval);
		break;

	case Node_assign_minus:
		DBG_P(("ASSIGN_minus", tree));
		assign_number(lhs, lval - rval);
		break;
	}
	do_deref();
	if (field_num == 0)
		set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
	field_num = -1;
	return *lhs;
}

