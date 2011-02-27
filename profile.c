/*
 * profile.c - gawk bytecode pretty-printer with counts
 */

/* 
 * Copyright (C) 1999-2011 the Free Software Foundation, Inc.
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

static void pprint(INSTRUCTION *startp, INSTRUCTION *endp, int in_for_header);
static void pp_parenthesize(NODE *n);
static void parenthesize(int type, NODE *left, NODE *right);
static char *pp_list(int nargs, const char *paren, const char *delim);
static char *pp_concat(const char *s1, const char *s2, const char *s3);
static int is_binary(int type);
static int prec_level(int type);
static void pp_push(int type, char *s, int flag);
static NODE *pp_pop(void);
static void pp_free(NODE *n);
const char *redir2str(int redirtype);

#define pp_str	hname
#define pp_len	hlength

#define DONT_FREE 1
#define CAN_FREE  2

#ifdef PROFILING
static RETSIGTYPE dump_and_exit(int signum) ATTRIBUTE_NORETURN;
static RETSIGTYPE just_dump(int signum);
#endif

/* pretty printing related functions and variables */

static NODE *pp_stack = NULL;
static char **fparms;	/* function parameter names */
static FILE *prof_fp;	/* where to send the profile */

static long indent_level = 0;


#define SPACEOVER	0

/* init_profiling --- do needed initializations, see also main.c */

void
init_profiling(int *flag ATTRIBUTE_UNUSED, const char *def_file ATTRIBUTE_UNUSED)
{
#ifdef PROFILING
	if (*flag == FALSE) {
		*flag = TRUE;
		set_prof_file(def_file);
	}
#endif
}

/* set_prof_file --- set the output file for profiling */

void
set_prof_file(const char *file)
{
	assert(file != NULL);
	prof_fp = fopen(file, "w");
	if (prof_fp == NULL) {
		warning(_("could not open `%s' for writing: %s"),
				file, strerror(errno));
		warning(_("sending profile to standard error"));
		prof_fp = stderr;
	}
}

/* init_profiling_signals --- set up signal handling for pgawk */

void
init_profiling_signals()
{
#ifdef PROFILING
#ifdef __DJGPP__
	signal(SIGINT, dump_and_exit);
	signal(SIGQUIT, just_dump);
#else  /* !__DJGPP__ */
#ifdef SIGHUP
	signal(SIGHUP, dump_and_exit);
#endif
#ifdef SIGUSR1
	signal(SIGUSR1, just_dump);
#endif
#endif /* !__DJGPP__ */
#endif /* PROFILING */
}

/* indent --- print out enough tabs */

static void
indent(long count)
{
	int i;

	if (count == 0)
		fprintf(prof_fp, "\t");
	else
		fprintf(prof_fp, "%6ld  ", count);

	assert(indent_level >= 0);
	for (i = 0; i < indent_level; i++)
		fprintf(prof_fp, "\t");
}

/* indent_in --- increase the level, with error checking */

static void
indent_in(void)
{
	assert(indent_level >= 0);
	indent_level++;
}

/* indent_out --- decrease the level, with error checking */

static void
indent_out(void)
{
	indent_level--;
	assert(indent_level >= 0);
}

static void
pp_push(int type, char *s, int flag)
{
	NODE *n;
	getnode(n);
	n->pp_str = s;
	n->pp_len = strlen(s);
	n->flags = flag;
	n->type = type;
	n->hnext = pp_stack;
	pp_stack = n;
}

static NODE *
pp_pop()
{
	NODE *n;
	n = pp_stack;
	pp_stack = n->hnext;
	return n;
}

static void
pp_free(NODE *n)
{
	if ((n->flags & CAN_FREE) != 0)
		efree(n->pp_str);
	freenode(n);
}

/*
 * pprint --- pretty print a program segment
 */

static void
pprint(INSTRUCTION *startp, INSTRUCTION *endp, int in_for_header)
{
	INSTRUCTION *pc;
	NODE *t1;
	char *str;
	NODE *t2;
	INSTRUCTION *ip;
	NODE *m;
	char *tmp;
	int rule;
	static int rule_count[MAXRULE];

	for (pc = startp; pc != endp; pc = pc->nexti) {
		if (pc->source_line > 0)
			sourceline = pc->source_line;

		switch (pc->opcode) {
		case Op_rule:
			source = pc->source_file;
			rule = pc->in_rule;

			if (rule != Rule) {
				if (! rule_count[rule]++)
					fprintf(prof_fp, _("\t# %s block(s)\n\n"), ruletab[rule]);
				fprintf(prof_fp, "\t%s {\n", ruletab[rule]);
				ip = (pc + 1)->firsti;
			} else {
				if (! rule_count[rule]++)
					fprintf(prof_fp, _("\t# Rule(s)\n\n"));
				ip = pc->nexti;
				indent(ip->exec_count);
				if (ip != (pc + 1)->firsti) {		/* non-empty pattern */
					pprint(ip->nexti, (pc + 1)->firsti, FALSE);
					t1 = pp_pop();
					fprintf(prof_fp, "%s {", t1->pp_str);
					pp_free(t1);
					ip = (pc + 1)->firsti;
#ifdef PROFILING
					if (ip->exec_count > 0)
						fprintf(prof_fp, " # %ld", ip->exec_count);
#endif
					fprintf(prof_fp, "\n");
				} else {
					fprintf(prof_fp, "{\n");
					ip = (pc + 1)->firsti;
				}
				ip = ip->nexti;
			}
			indent_in();
			pprint(ip, (pc + 1)->lasti, FALSE);
			indent_out();
			fprintf(prof_fp, "\t}\n\n");
			pc = (pc + 1)->lasti;
			break;

		case Op_atexit:
			break;

		case Op_stop:
			memset(rule_count, 0, MAXRULE * sizeof(int));
			break;

		case Op_push_i:
			m = pc->memory;
			if (m == Nnull_string)	/* optional return or exit value; don't print 0 or "" */
				pp_push(pc->opcode, m->stptr, DONT_FREE);
			else if ((m->flags & NUMBER) != 0)
				pp_push(pc->opcode, pp_number(m->numbr), CAN_FREE);
			else {
				str = pp_string(m->stptr, m->stlen, '"');
				if ((m->flags & INTLSTR) != 0) {
					char *tmp = str;
					str = pp_concat("_", tmp, "");
					efree(tmp);
				}
				pp_push(pc->opcode, str, CAN_FREE);
			}
			break;

		case Op_store_var:
		case Op_store_sub:
		case Op_assign_concat:
		case Op_push_lhs:
		case Op_push_param:
		case Op_push_array:
		case Op_push:
		case Op_push_arg:
			m = pc->memory;
			switch (m->type) {
			case Node_param_list:
				pp_push(pc->opcode, fparms[m->param_cnt], DONT_FREE);
				break;

			case Node_var:
			case Node_var_new:
			case Node_var_array:
				if (m->vname != NULL)
					pp_push(pc->opcode, m->vname, DONT_FREE);
 				else
					fatal(_("internal error: %s with null vname"),
							nodetype2str(m->type));
				break;

			default:
				cant_happen();
			}

			switch (pc->opcode) {		
			case Op_store_var:
				t2 = pp_pop(); /* l.h.s. */
				t1 = pp_pop(); /* r.h.s. */
				fprintf(prof_fp, "%s%s%s", t2->pp_str, op2str(pc->opcode), t1->pp_str);
				goto cleanup;

			case Op_store_sub:
				t1 = pp_pop();	/* array */
				tmp = pp_list(pc->expr_count, op2str(Op_subscript), ", "); /*subscript*/
				t2 = pp_pop(); /* r.h.s. */
				fprintf(prof_fp, "%s%s%s%s", t1->pp_str, tmp,
									op2str(pc->opcode), t2->pp_str);
				efree(tmp);
				goto cleanup;

			case Op_assign_concat:
				t2 = pp_pop(); /* l.h.s. */
				t1 = pp_pop();
				tmp = pp_concat(t2->pp_str, op2str(Op_concat), t1->pp_str);
				fprintf(prof_fp, "%s%s%s", t2->pp_str, op2str(Op_assign), tmp);
				efree(tmp);
cleanup:
				pp_free(t2);
				pp_free(t1);
				if (! in_for_header)
					fprintf(prof_fp, "\n");
				break;

			default:
				break;
			}
			break;

		case Op_sub_array:
		case Op_subscript_lhs:
		case Op_subscript:
			tmp = pp_list(pc->sub_count, op2str(pc->opcode), ", ");
			t1 = pp_pop();
			str = pp_concat(t1->pp_str, tmp, "");
			efree(tmp);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
			break;	

		case Op_and:
		case Op_or:
			pprint(pc->nexti, pc->target_jmp, in_for_header);
			t2 = pp_pop();
			t1 = pp_pop();
			parenthesize(pc->opcode, t1, t2);
			str = pp_concat(t1->pp_str, op2str(pc->opcode), t2->pp_str);
			pp_free(t1);
			pp_free(t2);
			pp_push(pc->opcode, str, CAN_FREE);
			pc = pc->target_jmp;
			break;

		case Op_plus_i:
		case Op_minus_i:
		case Op_times_i:
		case Op_exp_i:
		case Op_quotient_i:
		case Op_mod_i:
			m = pc->memory;
			t1 = pp_pop();
			if (prec_level(pc->opcode) > prec_level(t1->type)
					&& is_binary(t1->type))  /* (a - b) * 1 */
				pp_parenthesize(t1);
			if ((m->flags & NUMBER) != 0)
				tmp = pp_number(m->numbr);
			else
				tmp = pp_string(m->stptr, m->stlen, '"');
			str = pp_concat(t1->pp_str, op2str(pc->opcode), tmp);
			efree(tmp);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_plus:
		case Op_minus:
		case Op_times:
		case Op_exp:
		case Op_quotient:
		case Op_mod:
		case Op_equal:
		case Op_notequal:
		case Op_less:
		case Op_greater:
		case Op_leq:
		case Op_geq:
			t2 = pp_pop();
			t1 = pp_pop();
			parenthesize(pc->opcode, t1, t2);
			str = pp_concat(t1->pp_str, op2str(pc->opcode), t2->pp_str);
			pp_free(t1);
			pp_free(t2);
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_preincrement:
		case Op_predecrement:
		case Op_postincrement:
		case Op_postdecrement:
			t1 = pp_pop();
			if (pc->opcode == Op_preincrement || pc->opcode == Op_predecrement)
				str = pp_concat(op2str(pc->opcode), t1->pp_str, "");
			else
				str = pp_concat(t1->pp_str, op2str(pc->opcode), "");
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_field_spec:
		case Op_field_spec_lhs:
		case Op_unary_minus:
		case Op_not:
			t1 = pp_pop();
			if (is_binary(t1->type))
				pp_parenthesize(t1);

			/* optypes table (eval.c) includes space after ! */
			str = pp_concat(op2str(pc->opcode), t1->pp_str, "");
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_assign:
		case Op_assign_plus:
		case Op_assign_minus:
		case Op_assign_times:
		case Op_assign_quotient:
		case Op_assign_mod:
		case Op_assign_exp:
			t2 = pp_pop(); /* l.h.s. */
			t1 = pp_pop();
			str = pp_concat(t2->pp_str, op2str(pc->opcode), t1->pp_str);
			pp_free(t2);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_store_field:
			t1 = pp_pop(); /* field num */
			if (is_binary(t1->type))
				pp_parenthesize(t1);
			t2 = pp_pop(); /* r.h.s. */
			fprintf(prof_fp, "$%s%s%s", t1->pp_str, op2str(pc->opcode), t2->pp_str);
			pp_free(t2);
			pp_free(t1);
			if (! in_for_header)
				fprintf(prof_fp, "\n");
			break; 

		case Op_concat:
 			str = pp_list(pc->expr_count, NULL,
							(pc->concat_flag & CSUBSEP) ? ", " : op2str(Op_concat));
			pp_push(Op_concat, str, CAN_FREE);
			break;

		case Op_K_delete:
		{
			char *array;
			t1 = pp_pop();
			array = t1->pp_str;
			if (pc->expr_count > 0) {
				char *sub;
				sub = pp_list(pc->expr_count, NULL, ", ");
				fprintf(prof_fp, "%s %s[%s]", op2str(Op_K_delete), array, sub);
				efree(sub);
			} else 				
				fprintf(prof_fp, "%s %s", op2str(Op_K_delete), array);
			if (! in_for_header)
				fprintf(prof_fp, "\n");
			pp_free(t1);
		}
			break;

		case Op_K_delete_loop:
			/* Efficency hack not in effect because of exec_count instruction */
			cant_happen();
			break;
		
		case Op_in_array:
		{
			char *array, *sub;
			t1 = pp_pop();
			array = t1->pp_str;
			if (pc->expr_count > 1) {
				sub = pp_list(pc->expr_count, "()", ", ");
				str = pp_concat(sub, op2str(Op_in_array), array);
				efree(sub);
			} else {
				t2 = pp_pop();
				sub = t2->pp_str;
				str = pp_concat(sub, op2str(Op_in_array), array);
				pp_free(t2);
			}
			pp_free(t1);
			pp_push(Op_in_array, str, CAN_FREE);
		}
			break;

		case Op_var_update:
		case Op_var_assign:
		case Op_field_assign:
		case Op_arrayfor_init:
		case Op_arrayfor_incr: 
		case Op_arrayfor_final:
		case Op_newfile:
		case Op_get_record:
		case Op_lint:
		case Op_jmp:
		case Op_jmp_false:
		case Op_jmp_true:
		case Op_no_op:
		case Op_and_final:
		case Op_or_final:
		case Op_cond_pair:
		case Op_after_beginfile:
		case Op_after_endfile:
			break;

		case Op_builtin:
		{
			static char *ext_func = "extension_function()";
			const char *fname = getfname(pc->builtin);
			if (fname != NULL) {
				if (pc->expr_count > 0) {
					tmp = pp_list(pc->expr_count, "()", ", ");
					str = pp_concat(fname, tmp, "");
					efree(tmp);
				} else
					str = pp_concat(fname, "()", "");
				pp_push(Op_builtin, str, CAN_FREE);
			} else
				pp_push(Op_builtin, ext_func, DONT_FREE);
		}
			break;
	
		case Op_K_print:
		case Op_K_printf:
		case Op_K_print_rec:
			if (pc->opcode == Op_K_print_rec)
				tmp = pp_concat(" ", op2str(Op_field_spec), "0");
			else if (pc->redir_type != 0)
				tmp = pp_list(pc->expr_count, "()", ", ");
			else {
				tmp = pp_list(pc->expr_count, "  ", ", ");
				tmp[strlen(tmp) - 1] = '\0';	/* remove trailing space */
			}	

			if (pc->redir_type != 0) {
				t1 = pp_pop();
				if (is_binary(t1->type))
					pp_parenthesize(t1);
				fprintf(prof_fp, "%s%s%s%s", op2str(pc->opcode),
							tmp, redir2str(pc->redir_type), t1->pp_str);
				pp_free(t1);
			} else
				fprintf(prof_fp, "%s%s", op2str(pc->opcode), tmp);
			efree(tmp);
			if (! in_for_header)
				fprintf(prof_fp, "\n");
			break;

		case Op_push_re:
			if (pc->memory->type != Node_regex)
				break;
			/* else 
				fall through */
		case Op_match_rec:
		{
			NODE *re = pc->memory->re_exp;
			str = pp_string(re->stptr, re->stlen, '/');
			pp_push(pc->opcode, str, CAN_FREE);
		}
			break;

		case Op_nomatch:
		case Op_match:
		{
			char *restr, *txt;
			t1 = pp_pop();
			if (is_binary(t1->type))
				pp_parenthesize(t1);
			txt = t1->pp_str;
			m = pc->memory;
			if (m->type == Node_dynregex) {
				restr = txt;
				t2 = pp_pop();
				if (is_binary(t2->type))
					pp_parenthesize(t2);
				txt = t2->pp_str;
				str = pp_concat(txt, op2str(pc->opcode), restr);
				pp_free(t2);
			} else {
				NODE *re = m->re_exp;
				restr = pp_string(re->stptr, re->stlen, '/');
				str = pp_concat(txt, op2str(pc->opcode), restr);
				efree(restr);
			}
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE);
		}
			break;

		case Op_K_getline:
		case Op_K_getline_redir:
			if (pc->into_var) {
				t1 = pp_pop();
				tmp = pp_concat(op2str(Op_K_getline), " ", t1->pp_str);
				pp_free(t1);
			} else
				tmp = pp_concat(op2str(Op_K_getline), "", "");

			if (pc->redir_type != 0) {
				int before = (pc->redir_type == redirect_pipein
							|| pc->redir_type == redirect_twoway);

				t2 = pp_pop();
				if (is_binary(t2->type))
					pp_parenthesize(t2);
				if (before)
					str = pp_concat(t2->pp_str, redir2str(pc->redir_type), tmp);
				else
					str = pp_concat(tmp, redir2str(pc->redir_type), t2->pp_str);
				efree(tmp);
				pp_free(t2);
			} else
				str = tmp;
			pp_push(pc->opcode, str, CAN_FREE);
			break;

		case Op_indirect_func_call:
		case Op_func_call:
		{
			char *fname = pc->func_name;
			char *pre;
 			int pcount;

			if (pc->opcode == Op_indirect_func_call)
				pre = "@";
			else
				pre = "";		
			pcount = (pc + 1)->expr_count;
			if (pcount > 0) {
				tmp = pp_list(pcount, "()", ", ");
				str = pp_concat(pre, fname, tmp);
				efree(tmp);
			} else
				str = pp_concat(pre, fname, "()");
			if (pc->opcode == Op_indirect_func_call) {
				t1 = pp_pop();	/* indirect var */
				pp_free(t1);
			}
			pp_push(pc->opcode, str, CAN_FREE);
		}
			break;

		case Op_K_continue:
		case Op_K_break:
		case Op_K_nextfile:
		case Op_K_next:
			fprintf(prof_fp, "%s\n", op2str(pc->opcode));
			break;

		case Op_K_return:
		case Op_K_exit:
			t1 = pp_pop();
			if (is_binary(t1->type))
				pp_parenthesize(t1);
			if (pc->source_line > 0)	/* don't print implicit 'return' at end of function */
				fprintf(prof_fp, "%s %s\n", op2str(pc->opcode), t1->pp_str);
			pp_free(t1);
			break;

		case Op_pop:
			t1 = pp_pop();
			fprintf(prof_fp, "%s", t1->pp_str);
			if (! in_for_header)
				fprintf(prof_fp, "\n");
			pp_free(t1);
			break;

		case Op_line_range:
			ip = pc + 1;
			pprint(pc->nexti, ip->condpair_left, FALSE);
			pprint(ip->condpair_left->nexti, ip->condpair_right, FALSE);
			t2 = pp_pop();
			t1 = pp_pop();
			str = pp_concat(t1->pp_str, ", ", t2->pp_str);
			pp_free(t1);
			pp_free(t2);
			pp_push(Op_line_range, str, CAN_FREE);
			pc = ip->condpair_right;
			break;

		case Op_K_while:
			ip = pc + 1;
			indent(ip->while_body->exec_count);
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, ip->while_body, FALSE);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {\n", t1->pp_str);
			pp_free(t1);
			indent_in();
			pprint(ip->while_body->nexti, pc->target_break, FALSE);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");
			pc = pc->target_break;
			break;

		case Op_K_do:
			ip = pc + 1;
			indent(pc->nexti->exec_count);
			fprintf(prof_fp, "%s {\n", op2str(pc->opcode));
			indent_in();
			pprint(pc->nexti->nexti, ip->doloop_cond, FALSE);
			indent_out();
			pprint(ip->doloop_cond, pc->target_break, FALSE);
			indent(SPACEOVER);
			t1 = pp_pop();
			fprintf(prof_fp, "} %s (%s)\n", op2str(Op_K_while), t1->pp_str);
			pp_free(t1);
			pc = pc->target_break;
			break;

		case Op_K_for:
			ip = pc + 1;
			indent(ip->forloop_body->exec_count);
			fprintf(prof_fp, "%s (", op2str(pc->opcode));	
			pprint(pc->nexti, ip->forloop_cond, TRUE);
			fprintf(prof_fp, "; ");

			if (ip->forloop_cond->opcode == Op_no_op &&
					ip->forloop_cond->nexti == ip->forloop_body)
				fprintf(prof_fp, "; ");
			else {
				pprint(ip->forloop_cond, ip->forloop_body, TRUE);
				t1 = pp_pop();
				fprintf(prof_fp, "%s; ", t1->pp_str);
				pp_free(t1);
			}

			pprint(pc->target_continue, pc->target_break, TRUE);
			fprintf(prof_fp, ") {\n");
			indent_in();
			pprint(ip->forloop_body->nexti, pc->target_continue, FALSE);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");
			pc = pc->target_break;
			break;

		case Op_K_arrayfor:
		{
			char *array, *item;

			ip = pc + 1;
			t1 = pp_pop();
			array = t1->pp_str;
			m = ip->forloop_cond->array_var;
			if (m->type == Node_param_list)
				item = fparms[m->param_cnt];
			else
				item = m->vname;
			indent(ip->forloop_body->exec_count);
			fprintf(prof_fp, "%s (%s%s%s) {\n", op2str(Op_K_arrayfor),
						item, op2str(Op_in_array), array);
			indent_in();
			pp_free(t1);
			pprint(ip->forloop_body->nexti, pc->target_break, FALSE);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");			
			pc = pc->target_break;
		}
			break;

		case Op_K_switch:
			ip = pc + 1;
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, ip->switch_start, FALSE);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {\n", t1->pp_str);
			pp_free(t1);
			pprint(ip->switch_start, ip->switch_end, FALSE);
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");
			pc = pc->target_break;
			break;

		case Op_K_case:
		case Op_K_default:
			indent(pc->stmt_start->exec_count);
			if (pc->opcode == Op_K_case) {
				t1 = pp_pop();
				fprintf(prof_fp, "%s %s:\n", op2str(pc->opcode), t1->pp_str);
				pp_free(t1);
			} else
				fprintf(prof_fp, "%s:\n", op2str(pc->opcode));
			indent_in();
			pprint(pc->stmt_start->nexti, pc->stmt_end->nexti, FALSE);
			indent_out();
			break;

		case Op_K_if:
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, pc->branch_if, FALSE);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {", t1->pp_str);
			pp_free(t1);

			ip = pc->branch_if;
			if (ip->exec_count > 0)
				fprintf(prof_fp, " # %ld", ip->exec_count);
			fprintf(prof_fp, "\n");
			indent_in();
			pprint(ip->nexti, pc->branch_else, FALSE);
			indent_out();
			pc = pc->branch_else;
			if (pc->nexti->opcode == Op_no_op) {
				indent(SPACEOVER);
				fprintf(prof_fp, "}\n");
			}
			break;

		case Op_K_else:
			fprintf(prof_fp, "} %s {\n", op2str(pc->opcode));
			indent_in();
			pprint(pc->nexti, pc->branch_end, FALSE);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");
			pc = pc->branch_end;
			break;

		case Op_cond_exp:
		{
			NODE *f, *t, *cond;
			size_t len;

			pprint(pc->nexti, pc->branch_if, FALSE);
			ip = pc->branch_if;
			pprint(ip->nexti, pc->branch_else, FALSE);
			ip = pc->branch_else->nexti;

			pc = ip->nexti;
			assert(pc->opcode == Op_cond_exp);
			pprint(pc->nexti, pc->branch_end, FALSE);	

			f = pp_pop();
			t = pp_pop();
			cond = pp_pop();

			len =  f->pp_len + t->pp_len + cond->pp_len + 12;
			emalloc(str, char *, len, "pprint");
			sprintf(str, "(%s ? %s : %s)", cond->pp_str, t->pp_str, f->pp_str);

			pp_free(cond);
			pp_free(t);
			pp_free(f);
			pp_push(Op_cond_exp, str, CAN_FREE);
			pc = pc->branch_end;
		}
			break;			

		case Op_exec_count:
			if (! in_for_header)
				indent(pc->exec_count);
			break;

		default:
			cant_happen();
		}

		if (pc == endp)
			break;
	}
}

/* pp_string_fp --- printy print a string to the fp */

/*
 * This routine concentrates string pretty printing in one place,
 * so that it can be called from multiple places within gawk.
 */

void
pp_string_fp(Func_print print_func, FILE *fp, const char *in_str,
		size_t len, int delim, int breaklines)
{
	char *s = pp_string(in_str, len, delim);
	int count;
	size_t slen;
	const char *str = (const char *) s;
#define BREAKPOINT	70 /* arbitrary */

	slen = strlen(str);
	for (count = 0; slen > 0; slen--, str++) {
		if (++count >= BREAKPOINT && breaklines) {
			print_func(fp, "%c\n%c", delim, delim);
			count = 0;
		} else
			print_func(fp, "%c", *str);
	}
	efree(s);
}

#ifdef PROFILING
/* just_dump --- dump the profile and function stack and keep going */

static RETSIGTYPE
just_dump(int signum)
{
	extern INSTRUCTION *code_block;

	dump_prog(code_block);
	dump_funcs();
	dump_fcall_stack(prof_fp);
	fflush(prof_fp);
	signal(signum, just_dump);	/* for OLD Unix systems ... */
}

/* dump_and_exit --- dump the profile, the function stack, and exit */

static RETSIGTYPE
dump_and_exit(int signum)
{
	just_dump(signum);
	exit(EXIT_FAILURE);
}

#endif

/* dump_prog --- dump the program */

/*
 * XXX: I am not sure it is right to have the strings in the dump
 * be translated, but I'll leave it alone for now.
 */

void
dump_prog(INSTRUCTION *code)
{
	time_t now;

	(void) time(& now);
	/* \n on purpose, with \n in ctime() output */
	fprintf(prof_fp, _("\t# gawk profile, created %s\n"), ctime(& now));
	pprint(code, NULL, FALSE);
}

/* prec_level --- return the precedence of an operator, for paren tests */

static int
prec_level(int type)
{
	switch (type) {
	case Op_push_lhs:
	case Op_push_param:
	case Op_push_array:
	case Op_push:
	case Op_push_i:
	case Op_push_re:
	case Op_match_rec:
	case Op_subscript:
	case Op_subscript_lhs:
	case Op_func_call:
	case Op_K_delete_loop:
	case Op_builtin:
		return 15;

	case Op_field_spec:
	case Op_field_spec_lhs:
		return 14;

	case Op_exp:
	case Op_exp_i:
		return 13;

	case Op_preincrement:
	case Op_predecrement:
	case Op_postincrement:
	case Op_postdecrement:
		return 12;

	case Op_unary_minus:
	case Op_not:
		return 11;

	case Op_times:
	case Op_times_i:
	case Op_quotient:
	case Op_quotient_i:
	case Op_mod:
	case Op_mod_i:
		return 10;

	case Op_plus:
	case Op_plus_i:
	case Op_minus:
	case Op_minus_i:
		return 9;

	case Op_concat:
	case Op_assign_concat:
		return 8;

	case Op_equal:
	case Op_notequal:
	case Op_greater:
	case Op_leq:
	case Op_geq:
	case Op_match:
	case Op_nomatch:
		return 7;

	case Op_K_getline:
	case Op_K_getline_redir:
		return 6;

	case Op_less:
		return 5;

	case Op_in_array:
		return 5;

	case Op_and:
		return 4;

	case Op_or:
		return 3;

	case Op_cond_exp:
		return 2;

	case Op_assign:
	case Op_assign_times:
	case Op_assign_quotient:
	case Op_assign_mod:
	case Op_assign_plus:
	case Op_assign_minus:
	case Op_assign_exp:
		return 1;

	default:
		return 0;
	}
}

static int
is_binary(int type)
{
	switch (type) {
	case Op_geq:
	case Op_leq:
	case Op_greater:
	case Op_less:
	case Op_notequal:
	case Op_equal:
	case Op_exp:
	case Op_times:
	case Op_quotient:
	case Op_mod:
	case Op_plus:
	case Op_minus:
	case Op_exp_i:
	case Op_times_i:
	case Op_quotient_i:
	case Op_mod_i:
	case Op_plus_i:
	case Op_minus_i:
	case Op_concat:
	case Op_assign_concat:
	case Op_match:
	case Op_nomatch:
	case Op_assign:
	case Op_assign_times:
	case Op_assign_quotient:
	case Op_assign_mod:
	case Op_assign_plus:
	case Op_assign_minus:
	case Op_assign_exp:
	case Op_cond_exp:
	case Op_and:
	case Op_or:
	case Op_in_array:
	case Op_K_getline_redir:	/* sometimes */
	case Op_K_getline:
		return TRUE;

	default:
		return FALSE;
	}
}

/* parenthesize --- parenthesize an expression in stack */

static void
pp_parenthesize(NODE *sp)
{
	char *p = sp->pp_str;
	size_t len = sp->pp_len;

	emalloc(p, char *, len + 3, "pp_parenthesize");
	*p = '(';
	memcpy(p + 1, sp->pp_str, len);
	p[len + 1] = ')';
	p[len + 2] = '\0';
	if ((sp->flags & CAN_FREE) != 0)
		efree(sp->pp_str);
	sp->pp_str = p;
	sp->pp_len += 2;
	sp->flags |= CAN_FREE;
}

static void
parenthesize(int type, NODE *left, NODE *right)
{
	int rprec = prec_level(right->type);
	int lprec = prec_level(left->type);
	int prec = prec_level(type);

	if (prec > lprec) {
		if (is_binary(left->type))		/* (a - b) * c */
			pp_parenthesize(left);
		if (prec >= rprec && is_binary(right->type))	/* (a - b) * (c - d) */
			pp_parenthesize(right);
	} else {
		if (prec >= rprec && is_binary(right->type)) /* a - b - (c - d) */
			pp_parenthesize(right);
	}
}

/* pp_string --- pretty format a string or regex constant */

char *
pp_string(const char *in_str, size_t len, int delim)
{
	static char str_escapes[] = "\a\b\f\n\r\t\v\\";
	static char str_printables[] = "abfnrtv\\";
	static char re_escapes[] = "\a\b\f\n\r\t\v";
	static char re_printables[] = "abfnrtv";
	char *escapes;
	char *printables;
	char *cp;
	int i;
	const unsigned char *str = (const unsigned char *) in_str;
	size_t ofre, osiz;
	char *obuf, *obufout;

	assert(delim == '"' || delim == '/');

	if (delim == '/') {
		escapes = re_escapes;
		printables = re_printables;
	} else {
		escapes = str_escapes;
		printables = str_printables;
	}

/* make space for something l big in the buffer */
#define chksize(l)  if ((l) > ofre) { \
		long olen = obufout - obuf; \
		erealloc(obuf, char *, osiz * 2, "pp_string"); \
		obufout = obuf + olen; \
		ofre += osiz; \
		osiz *= 2; \
} ofre -= (l)

	osiz = len + 3 + 2; 	/* initial size; 3 for delim + terminating null */
	emalloc(obuf, char *, osiz, "pp_string");
	obufout = obuf;
	ofre = osiz - 1;

	*obufout++ = delim;
	for (; len > 0; len--, str++) {
		chksize(2);		/* make space for 2 chars */
		if (delim != '/' && *str == delim) {
			*obufout++ = '\\';
			*obufout++ = delim;
		} else if ((cp = strchr(escapes, *str)) != NULL) {
			i = cp - escapes;
			*obufout++ = '\\';
			*obufout++ = printables[i];
		/* NB: Deliberate use of lower-case versions. */
		} else if (isascii(*str) && isprint(*str)) {
			*obufout++ = *str;
			ofre += 1;
		} else {
			size_t len;

			chksize(8);		/* total available space is 10 */

			sprintf(obufout, "\\%03o", *str & 0xff);
			len = strlen(obufout);
			ofre += (10 - len);	 /* adjust free space count */
			obufout += len;
		}
	}
	chksize(1);
	*obufout++ = delim;
	*obufout = '\0';
	return obuf;
#undef chksize
}

/* pp_number --- pretty format a number */

char *
pp_number(AWKNUM d)
{
#define PP_PRECISION 6
	char *str;

	emalloc(str, char *, PP_PRECISION + 10, "pp_number");
	sprintf(str, "%0.*g", PP_PRECISION, d);
	return str;
#undef PP_PRECISION
}

/* pp_node --- pretty format a node */

char *
pp_node(NODE *n)
{
	if ((n->flags & NUMBER) != 0)
		return pp_number(n->numbr);
	return pp_string(n->stptr, n->stlen, '"');
}

static NODE **pp_args = NULL;
static int npp_args;

static char *
pp_list(int nargs, const char *paren, const char *delim)
{
	NODE *r;
 	char *str, *s;
	size_t len;
	size_t delimlen;
	int i;

	if (pp_args == NULL) {
		npp_args = nargs;
		emalloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_list");
	} else if (nargs > npp_args) {
		npp_args = nargs;
		erealloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_list");
	}

	delimlen = strlen(delim);
	len = -delimlen;
	for (i = 1; i <= nargs; i++) {
		r = pp_args[i] = pp_pop();
		len += r->pp_len + delimlen;
	}
	if (paren != NULL) {
		assert(strlen(paren) == 2);
		len += 2;
	}

	emalloc(str, char *, len + 1, "pp_list");
	s = str;
	if (paren != NULL)
		*s++ = paren[0];  
	r = pp_args[nargs];
	memcpy(s, r->pp_str, r->pp_len);
	s += r->pp_len;
	pp_free(r);
	for (i = nargs - 1; i > 0; i--) {
		if (delimlen > 0) {
			memcpy(s, delim, delimlen);
			s += delimlen;
		}
		r = pp_args[i];
		memcpy(s, r->pp_str, r->pp_len);
		s += r->pp_len;
		pp_free(r);
	}
	if (paren != NULL)
		*s++ = paren[1];
	*s = '\0';
	return str;					
}

static char *
pp_concat(const char *s1, const char *s2, const char *s3)
{
	size_t len1, len2, len3, l;
	char *str, *s;

	len1 = strlen(s1);
	len2 = strlen(s2);
	len3 = strlen(s3);
	l = len1 + len2 + len3 + 2;
	emalloc(str, char *, l, "pp_concat");
	s = str;
	if (len1 > 0) {
		memcpy(s, s1, len1);
		s += len1;
	}
	if (len2 > 0) {
		memcpy(s, s2, len2);
		s += len2;
	}
	if (len3 > 0) {
		memcpy(s, s3, len3);
		s += len3;
	}
	*s = '\0';
	return str;
}

/* pp_func --- pretty print a function */

int
pp_func(INSTRUCTION *pc, void *data ATTRIBUTE_UNUSED)
{
	int j;
	char **pnames;
	NODE *f;
	static int first = TRUE;
	int pcount;

	if (first) {
		first = FALSE;
		fprintf(prof_fp, _("\n\t# Functions, listed alphabetically\n"));
	}

	f = pc->func_body;
	fprintf(prof_fp, "\n");
	indent(pc->nexti->exec_count);
	fprintf(prof_fp, "%s %s(", op2str(Op_K_function), f->lnode->param);
	pnames = f->parmlist;
	fparms = pnames;
	pcount = f->lnode->param_cnt;
	for (j = 0; j < pcount; j++) {
		fprintf(prof_fp, "%s", pnames[j]);
		if (j < pcount - 1)
			fprintf(prof_fp, ", ");
	}
	fprintf(prof_fp, ")\n\t{\n");
	indent_in();
	pprint(pc->nexti->nexti, NULL, FALSE);	/* function body */
	indent_out();
	fprintf(prof_fp, "\t}\n");
	return 0;
}

/* redir2str --- convert a redirection type into a printable value */

const char *
redir2str(int redirtype)
{
	static const char *const redirtab[] = {
		"",
		" > ",	/* redirect_output */
		" >> ",	/* redirect_append */
		" | ",	/* redirect_pipe */
		" | ",	/* redirect_pipein */
		" < "	/* redirect_input */
		" |& "	/* redirect_twoway */
	};

	if (redirtype < 0 || redirtype > redirect_twoway)
		fatal(_("redir2str: unknown redirection type %d"), redirtype);
	return redirtab[redirtype];
}

	
