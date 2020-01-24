/*
 * profile.c - gawk bytecode pretty-printer with counts
 */

/*
 * Copyright (C) 1999-2020 the Free Software Foundation, Inc.
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

static void pprint(INSTRUCTION *startp, INSTRUCTION *endp, int flags);
static INSTRUCTION *end_line(INSTRUCTION *ip);
static void pp_parenthesize(NODE *n);
static void parenthesize(int type, NODE *left, NODE *right);
static char *pp_list(int nargs, const char *paren, const char *delim);
static char *pp_group3(const char *s1, const char *s2, const char *s3);
static char *pp_concat(int nargs);
static char *pp_string_or_typed_regex(const char *in_str, size_t len, int delim, bool typed_regex);
static char *pp_typed_regex(const char *in_str, size_t len, int delim);
static bool is_binary(int type);
static bool is_scalar(int type);
static int prec_level(int type);
static void pp_push(int type, char *s, int flag, INSTRUCTION *comment);
static NODE *pp_pop(void);
static void print_comment(INSTRUCTION *pc, long in);
const char *redir2str(int redirtype);
static void pp_namespace(const char *name, INSTRUCTION *comment);
static void pp_namespace_list(INSTRUCTION *list);
static char *adjust_namespace(char *name, bool *malloced);

#define pp_str	vname
#define pp_len	sub.nodep.reserved
#define pp_next	rnode
#define pp_comment	sub.nodep.x.cmnt

#define DONT_FREE 1
#define CAN_FREE  2

static void dump_and_exit(int signum) ATTRIBUTE_NORETURN;
static void just_dump(int signum);

/* pretty printing related functions and variables */

static NODE *pp_stack = NULL;
static NODE *func_params;	/* function parameters */
static FILE *prof_fp;	/* where to send the profile */

static long indent_level = 0;

static const char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
static const size_t tabs_len = sizeof(tabs) - 1;

#define check_indent_level() \
	if (indent_level + 1 > tabs_len) \
		/* We're allowed to be snarky, occasionally. */ \
		fatal(_("Program indentation level too deep. Consider refactoring your code"));


#define SPACEOVER	0

#define NO_PPRINT_FLAGS	0
#define IN_FOR_HEADER	1
#define IN_ELSE_IF	2

/* set_prof_file --- set the output file for profiling or pretty-printing */

void
set_prof_file(const char *file)
{
	int fd;

	assert(file != NULL);
	fd = devopen_simple(file, "w", true);
	if (fd == INVALID_HANDLE)
		prof_fp = NULL;
	else if (fd == fileno(stdout))
		prof_fp = stdout;
	else if (fd == fileno(stderr))
		prof_fp = stderr;
	else
		prof_fp = fdopen(fd, "w");

	if (prof_fp == NULL) {
		/* don't leak file descriptors */
		int e = errno;

		if (   fd != INVALID_HANDLE
		    && fd != fileno(stdout)
		    && fd != fileno(stderr))
			(void) close(fd);

		errno = e;
		warning(_("could not open `%s' for writing: %s"),
				file, strerror(errno));
		warning(_("sending profile to standard error"));
		prof_fp = stderr;
	}
}

/* init_profiling_signals --- set up signal handling for gawk --profile */

void
init_profiling_signals()
{
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
}

/* indent --- print out enough tabs */

static void
indent(long count)
{
	int i;

	if (do_profile) {
		if (count == 0)
			fprintf(prof_fp, "\t");
		else
			fprintf(prof_fp, "%6ld  ", count);
	}

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

/* pp_push --- push a pretty printed string onto the stack */

static void
pp_push(int type, char *s, int flag, INSTRUCTION *comment)
{
	NODE *n;
	getnode(n);
	n->pp_str = s;
	n->pp_len = strlen(s);
	n->flags = flag;
	n->type = type;
	n->pp_next = pp_stack;
	n->pp_comment = comment;
	pp_stack = n;
}

/* pp_pop --- pop a pretty printed string off the stack */

static NODE *
pp_pop()
{
	NODE *n;
	n = pp_stack;
	pp_stack = n->pp_next;
	return n;
}

/* pp_top --- look at what's on the top of the stack */

#define pp_top()	pp_stack

/* pp_free --- release a pretty printed node */

static void
pp_free(NODE *n)
{
	if ((n->flags & CAN_FREE) != 0)
		efree(n->pp_str);
	freenode(n);
}

/* pprint --- pretty print a program segment */

static void
pprint(INSTRUCTION *startp, INSTRUCTION *endp, int flags)
{
	INSTRUCTION *pc;
	NODE *t1;
	char *str;
	NODE *t2;
	INSTRUCTION *ip1;
	INSTRUCTION *ip2;
	NODE *m;
	char *tmp;
	int rule;
	static int rule_count[MAXRULE];
	static bool skip_comment = false;

	for (pc = startp; pc != endp; pc = pc->nexti) {
		if (pc->source_line > 0)
			sourceline = pc->source_line;

		/* skip leading EOL comment as it has already been printed  */
		if (pc->opcode == Op_comment
		    && pc->memory->comment_type == EOL_COMMENT
		    && skip_comment) {
			skip_comment = false;
			continue;
		}
		skip_comment = false;

		switch (pc->opcode) {
		case Op_rule:
			/*
			 * Rules are four instructions long.
			 * See append_rule in awkgram.y.
			 * The first has the Rule Op Code, nexti etc.
			 * The second, (pc + 1) has firsti and lasti:
			 * 	the first/last ACTION instructions for this rule.
			 * The third has first_line and last_line:
			 * 	the first and last source line numbers.
			 * The fourth holds the namespace name if there is one.
			 *	(there should be one if we're in this file)
			 * This can actually be a list in reverse order if
			 * there were several @namespace directives one
			 * after the other.
			 */
			source = pc->source_file;
			rule = pc->in_rule;

			pp_namespace_list(pc[3].nexti);

			if (rule != Rule) {
				/* Allow for pre-non-rule-block comment  */
				if (pc->nexti != (pc+1)->firsti
				    && pc->nexti->opcode == Op_comment
				    && pc->nexti->memory->comment_type == BLOCK_COMMENT)
					print_comment(pc->nexti, -1);
				ip1 = (pc + 1)->firsti;
				ip2 = (pc + 1)->lasti;

				if (do_profile) {
					if (! rule_count[rule]++)
						fprintf(prof_fp, _("\t# %s rule(s)\n\n"), ruletab[rule]);
					indent(0);
				}
				fprintf(prof_fp, "%s {", ruletab[rule]);
				end_line(pc);
				skip_comment = true;
			} else {
				if (do_profile && ! rule_count[rule]++)
					fprintf(prof_fp, _("\t# Rule(s)\n\n"));
				ip1 = pc->nexti;
				indent(ip1->exec_count);
				if (ip1 != (pc + 1)->firsti) {		/* non-empty pattern */
					pprint(ip1->nexti, (pc + 1)->firsti, NO_PPRINT_FLAGS);
					/* Allow for case where the "pattern" is just a comment  */
					if (ip1->nexti->nexti->nexti != (pc +1)->firsti
					    || ip1->nexti->opcode != Op_comment) {
						t1 = pp_pop();
						fprintf(prof_fp, "%s {", t1->pp_str);
						pp_free(t1);
					} else
						fprintf(prof_fp, "{");
					ip1 = (pc + 1)->firsti;
					ip2 = (pc + 1)->lasti;

					if (do_profile && ip1->exec_count > 0)
						fprintf(prof_fp, " # %ld", ip1->exec_count);

					end_line(ip1);
					skip_comment = true;
				} else {
					fprintf(prof_fp, "{\n");
					ip1 = (pc + 1)->firsti;
					ip2 = (pc + 1)->lasti;
				}
				ip1 = ip1->nexti;
			}
			indent_in();
			pprint(ip1, ip2, NO_PPRINT_FLAGS);
			indent_out();
			if (do_profile)
				indent(0);
			fprintf(prof_fp, "}\n\n");
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
				pp_push(pc->opcode, m->stptr, DONT_FREE, pc->comment);
			else if ((m->flags & NUMBER) != 0)
				pp_push(pc->opcode, pp_number(m), CAN_FREE, pc->comment);
			else {
				str = pp_string(m->stptr, m->stlen, '"');
				if ((m->flags & INTLSTR) != 0) {
					char *tmp = str;
					str = pp_group3("_", tmp, "");
					efree(tmp);
				}
				pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			}
			break;

		case Op_store_var:
			if (pc->initval != NULL)
				pp_push(Op_push_i, pp_node(pc->initval), CAN_FREE, pc->comment);
			/* fall through */
		case Op_store_sub:
		case Op_assign_concat:
		case Op_push_lhs:
		case Op_push_param:
		case Op_push_array:
		case Op_push:
		case Op_push_arg:
		case Op_push_arg_untyped:
			m = pc->memory;
			switch (m->type) {
			case Node_param_list:
				pp_push(pc->opcode, func_params[m->param_cnt].param, DONT_FREE, pc->comment);
				break;

			case Node_var:
			case Node_var_new:
			case Node_var_array:
				if (m->vname != NULL) {
					bool malloced = false;
					char *name = adjust_namespace(m->vname, & malloced);

					pp_push(pc->opcode, name, malloced ? CAN_FREE : DONT_FREE, pc->comment);
				} else
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
				tmp = pp_group3(t2->pp_str, op2str(Op_concat), t1->pp_str);
				fprintf(prof_fp, "%s%s%s", t2->pp_str, op2str(Op_assign), tmp);
				efree(tmp);
cleanup:
				pp_free(t2);
				pp_free(t1);
				if ((flags & IN_FOR_HEADER) == 0)
					pc = end_line(pc);
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
			str = pp_group3(t1->pp_str, tmp, "");
			efree(tmp);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_and:
		case Op_or:
			pprint(pc->nexti, pc->target_jmp, flags);
			t2 = pp_pop();
			t1 = pp_pop();
			parenthesize(pc->opcode, t1, t2);
			if (pc->comment == NULL)
				str = pp_group3(t1->pp_str, op2str(pc->opcode), t2->pp_str);
			else {
				check_indent_level();

				size_t len = strlen(t1->pp_str)
						+ strlen(op2str(pc->opcode)) + strlen(t2->pp_str)	// foo && bar
						+ indent_level + 1				// indent
						+ pc->comment->memory->stlen + 3;		// tab comment

				emalloc(str, char *, len, "pprint");
				sprintf(str, "%s%s%s%.*s %s", t1->pp_str, op2str(pc->opcode),
						pc->comment->memory->stptr,
						(int) (indent_level + 1), tabs, t2->pp_str);
			}
			pp_free(t1);
			pp_free(t2);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
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
				tmp = pp_number(m);
			else
				tmp = pp_string(m->stptr, m->stlen, '"');
			str = pp_group3(t1->pp_str, op2str(pc->opcode), tmp);
			efree(tmp);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_parens:
			t1 = pp_pop();
			str = pp_group3("(", t1->pp_str, ")");
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
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
			str = pp_group3(t1->pp_str, op2str(pc->opcode), t2->pp_str);
			pp_free(t1);
			pp_free(t2);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_preincrement:
		case Op_predecrement:
		case Op_postincrement:
		case Op_postdecrement:
			t1 = pp_pop();
			if (pc->opcode == Op_preincrement || pc->opcode == Op_predecrement)
				str = pp_group3(op2str(pc->opcode), t1->pp_str, "");
			else
				str = pp_group3(t1->pp_str, op2str(pc->opcode), "");
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_field_spec:
		case Op_field_spec_lhs:
		case Op_unary_minus:
		case Op_unary_plus:
		case Op_not:
			t1 = pp_pop();
			if (is_binary(t1->type)
			    || (((OPCODE) t1->type) == pc->opcode
				    && (pc->opcode == Op_unary_minus
					    || pc->opcode == Op_unary_plus)))
				pp_parenthesize(t1);

			/* optypes table (eval.c) includes space after ! */
			str = pp_group3(op2str(pc->opcode), t1->pp_str, "");
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
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
			str = pp_group3(t2->pp_str, op2str(pc->opcode), t1->pp_str);
			pp_free(t2);
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_store_field:
			t1 = pp_pop(); /* field num */
			if (is_binary(t1->type))
				pp_parenthesize(t1);
			t2 = pp_pop(); /* r.h.s. */
			fprintf(prof_fp, "$%s%s%s", t1->pp_str, op2str(pc->opcode), t2->pp_str);
			pp_free(t2);
			pp_free(t1);
			if ((flags & IN_FOR_HEADER) == 0)
				pc = end_line(pc);
			break;

		case Op_concat:
			str = pp_concat(pc->expr_count);
			pp_push(Op_concat, str, CAN_FREE, pc->comment);
			break;

		case Op_K_delete:
		{
			char *array;
			t1 = pp_pop();
			array = t1->pp_str;
			if (pc->expr_count > 0) {
				char *sub;
				sub = pp_list(pc->expr_count, NULL, pc->expr_count > 1 ? "][" : ", ");
				fprintf(prof_fp, "%s %s[%s]", op2str(Op_K_delete), array, sub);
				efree(sub);
			} else
				fprintf(prof_fp, "%s %s", op2str(Op_K_delete), array);
			if ((flags & IN_FOR_HEADER) == 0)
				pc = end_line(pc);
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
				str = pp_group3(sub, op2str(Op_in_array), array);
				efree(sub);
			} else {
				t2 = pp_pop();
				if (prec_level(t2->type) < prec_level(Op_in_array)) {
					pp_parenthesize(t2);
				}
				sub = t2->pp_str;
				str = pp_group3(sub, op2str(Op_in_array), array);
				pp_free(t2);
			}
			pp_free(t1);
			pp_push(Op_in_array, str, CAN_FREE, pc->comment);
		}
			break;

		case Op_var_update:
		case Op_var_assign:
		case Op_field_assign:
		case Op_subscript_assign:
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

		case Op_sub_builtin:
		{
			const char *fname = "sub";
			if ((pc->sub_flags & GSUB) != 0)
				fname = "gsub";
			else if ((pc->sub_flags & GENSUB) != 0)
				fname = "gensub";
			tmp = pp_list(pc->expr_count, "()", ", ");
			str = pp_group3(fname, tmp, "");
			efree(tmp);
			pp_push(Op_sub_builtin, str, CAN_FREE, pc->comment);
		}
			break;

		case Op_builtin:
		case Op_ext_builtin:
		{
			const char *fname;
			if (pc->opcode == Op_builtin) {
				bool prepend_awk = (current_namespace != awk_namespace && strcmp(current_namespace, "awk") != 0);
				fname = getfname(pc->builtin, prepend_awk);
			} else
				fname = (pc + 1)->func_name;
			if (fname != NULL) {
				if (pc->expr_count > 0) {
					tmp = pp_list(pc->expr_count, "()", ", ");
					str = pp_group3(fname, tmp, "");
					efree(tmp);
				} else
					str = pp_group3(fname, "()", "");
				pp_push(Op_builtin, str, CAN_FREE, pc->comment);
			} else
				fatal(_("internal error: builtin with null fname"));
		}
			break;

		case Op_K_print:
		case Op_K_printf:
		case Op_K_print_rec:
			if (pc->opcode == Op_K_print_rec)
				// instead of `print $0', just `print'
				tmp = strdup("");
			else if (pc->redir_type != 0) {
				// Avoid turning printf("hello\n") into printf(("hello\n"))
				NODE *n = pp_top();

				if (pc->expr_count == 1
				    && n->pp_str[0] == '(' 
				    && n->pp_str[n->pp_len - 1] == ')') {
					n = pp_pop();

					tmp = strdup(n->pp_str);
					pp_free(n);
				} else
					tmp = pp_list(pc->expr_count, "()", ", ");
			} else {
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
			if ((flags & IN_FOR_HEADER) == 0)
				pc = end_line(pc);
			break;

		case Op_push_re:
			if (pc->memory->type != Node_regex && (pc->memory->flags & REGEX) == 0)
				break;
			/* else
				fall through */
		case Op_match_rec:
		{
			if (pc->memory->type == Node_regex) {
				NODE *re = pc->memory->re_exp;
				str = pp_string(re->stptr, re->stlen, '/');
			} else {
				assert((pc->memory->flags & REGEX) != 0);
				str = pp_typed_regex(pc->memory->stptr, pc->memory->stlen, '/');
			}
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
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
				str = pp_group3(txt, op2str(pc->opcode), restr);
				pp_free(t2);
			} else if (m->type == Node_val && (m->flags & REGEX) != 0) {
				restr = pp_typed_regex(m->stptr, m->stlen, '/');
				str = pp_group3(txt, op2str(pc->opcode), restr);
				efree(restr);
			} else {
				NODE *re = m->re_exp;
				restr = pp_string(re->stptr, re->stlen, '/');
				str = pp_group3(txt, op2str(pc->opcode), restr);
				efree(restr);
			}
			pp_free(t1);
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
		}
			break;

		case Op_K_getline:
		case Op_K_getline_redir:
			if (pc->into_var) {
				t1 = pp_pop();
				tmp = pp_group3(op2str(Op_K_getline), " ", t1->pp_str);
				pp_free(t1);
			} else
				tmp = pp_group3(op2str(Op_K_getline), "", "");

			if (pc->redir_type != 0) {
				int before = (pc->redir_type == redirect_pipein
							|| pc->redir_type == redirect_twoway);

				t2 = pp_pop();
				if (is_binary(t2->type))
					pp_parenthesize(t2);
				if (before)
					str = pp_group3(t2->pp_str, redir2str(pc->redir_type), tmp);
				else
					str = pp_group3(tmp, redir2str(pc->redir_type), t2->pp_str);
				efree(tmp);
				pp_free(t2);
			} else
				str = tmp;
			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			break;

		case Op_indirect_func_call:
		case Op_func_call:
		{
			char *pre;
 			int pcount;
			bool malloced = false;
			char *fname = adjust_namespace(pc->func_name, & malloced);

			if (pc->opcode == Op_indirect_func_call)
				pre = "@";
			else
				pre = "";
			pcount = (pc + 1)->expr_count;
			if (pcount > 0) {
				tmp = pp_list(pcount, "()", ", ");
				str = pp_group3(pre, fname, tmp);
				efree(tmp);
			} else
				str = pp_group3(pre, fname, "()");
			if (pc->opcode == Op_indirect_func_call) {
				t1 = pp_pop();	/* indirect var */
				pp_free(t1);
			}

			pp_push(pc->opcode, str, CAN_FREE, pc->comment);
			if (malloced)
				efree((void *) fname);
		}
			break;

		case Op_K_continue:
		case Op_K_break:
		case Op_K_nextfile:
		case Op_K_next:
			fprintf(prof_fp, "%s", op2str(pc->opcode));
			pc = end_line(pc);
			break;

		case Op_K_return:
		case Op_K_exit:
			t1 = pp_pop();
			if (is_binary(t1->type))
				pp_parenthesize(t1);
			if (pc->source_line > 0) {	/* don't print implicit 'return' at end of function */
				// avoid final trailing space to keep whiny users happy
				if (t1->pp_str[0] != '\0')
					fprintf(prof_fp, "%s %s", op2str(pc->opcode), t1->pp_str);
				else
					fprintf(prof_fp, "%s", op2str(pc->opcode));
				pc = end_line(pc);
			}
			pp_free(t1);
			break;

		case Op_pop:
			t1 = pp_pop();
			fprintf(prof_fp, "%s", t1->pp_str);
			if ((flags & IN_FOR_HEADER) == 0)
				pc = end_line(pc);
			pp_free(t1);
			break;

		case Op_line_range:
			ip1 = pc + 1;
			pprint(pc->nexti, ip1->condpair_left, NO_PPRINT_FLAGS);
			pprint(ip1->condpair_left->nexti, ip1->condpair_right, NO_PPRINT_FLAGS);
			t2 = pp_pop();
			t1 = pp_pop();
			str = pp_group3(t1->pp_str, ", ", t2->pp_str);
			pp_free(t1);
			pp_free(t2);
			pp_push(Op_line_range, str, CAN_FREE, pc->comment);
			pc = ip1->condpair_right;
			break;

		case Op_K_while:
			ip1 = pc + 1;
			indent(ip1->while_body->exec_count);
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, ip1->while_body, NO_PPRINT_FLAGS);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {", t1->pp_str);
			pp_free(t1);
			ip1->while_body = end_line(ip1->while_body);
			indent_in();
			pprint(ip1->while_body->nexti, pc->target_break, NO_PPRINT_FLAGS);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}");
			pc = end_line(pc->target_break);
			break;

		case Op_K_do:
			ip1 = pc + 1;
			indent(pc->nexti->exec_count);
			fprintf(prof_fp, "%s {", op2str(pc->opcode));
			end_line(pc->nexti);
			skip_comment = true;
			indent_in();
			pprint(pc->nexti->nexti, ip1->doloop_cond, NO_PPRINT_FLAGS);
			indent_out();
			pprint(ip1->doloop_cond, pc->target_break, NO_PPRINT_FLAGS);
			indent(SPACEOVER);
			t1 = pp_pop();
			fprintf(prof_fp, "} %s (%s)", op2str(Op_K_while), t1->pp_str);
			if (pc->comment)
				fprintf(prof_fp, "\t%s", pc->comment->memory->stptr);
			else {
				end_line(pc->target_break);
				skip_comment = true;
			}
			pp_free(t1);
			pc = pc->target_break;
			break;

		case Op_K_for:
		{
			INSTRUCTION *comment1 = NULL, *comment2 = NULL;

			if (pc->comment != NULL) {
				comment1 = pc->comment;
				pc->comment = NULL;
				if (comment1 != NULL && comment1->comment != NULL) {
					comment2 = comment1->comment;
					comment1->comment = NULL;
				}
				if (comment2 == NULL && comment1->memory->comment_type == FOR_COMMENT) {
					comment2 = comment1;
					comment2->memory->comment_type = EOL_COMMENT;
					comment1 = NULL;
				}
			}

			ip1 = pc + 1;
			indent(ip1->forloop_body->exec_count);
			fprintf(prof_fp, "%s (", op2str(pc->opcode));

			/* If empty for looop header, print it a little more nicely. */
			if (   pc->nexti->opcode == Op_no_op
			    && ip1->forloop_cond == pc->nexti
			    && pc->target_continue->opcode == Op_jmp
			    && comment1 == NULL && comment2 == NULL) {
				fprintf(prof_fp, ";;");
			} else {
				pprint(pc->nexti, ip1->forloop_cond, IN_FOR_HEADER);
				fprintf(prof_fp, "; ");

				if (comment1 != NULL) {
					print_comment(comment1, 0);
					indent(ip1->forloop_body->exec_count);
					indent(1);
				}

				if (ip1->forloop_cond->opcode == Op_no_op &&
						ip1->forloop_cond->nexti == ip1->forloop_body)
					fprintf(prof_fp, "; ");
				else {
					pprint(ip1->forloop_cond, ip1->forloop_body, IN_FOR_HEADER);
					t1 = pp_pop();
					fprintf(prof_fp, "%s; ", t1->pp_str);
					pp_free(t1);
				}

				if (comment2 != NULL) {
					print_comment(comment2, 0);
					indent(ip1->forloop_body->exec_count);
					indent(1);
				}

				pprint(pc->target_continue, pc->target_break, IN_FOR_HEADER);
			}
			fprintf(prof_fp, ") {");
			end_line(ip1->forloop_body);
			skip_comment = true;
			indent_in();
			pprint(ip1->forloop_body->nexti, pc->target_continue, NO_PPRINT_FLAGS);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}");
			end_line(pc->target_break);
			skip_comment = true;
			pc = pc->target_break;
		}
			break;

		case Op_K_arrayfor:
		{
			char *array;
			const char *item;

			ip1 = pc + 1;
			t1 = pp_pop();
			array = t1->pp_str;
			m = ip1->forloop_cond->array_var;
			if (m->type == Node_param_list)
				item = func_params[m->param_cnt].param;
			else
				item = m->vname;
			indent(ip1->forloop_body->exec_count);
			fprintf(prof_fp, "%s (%s%s%s) {", op2str(Op_K_arrayfor),
						item, op2str(Op_in_array), array);
			end_line(ip1->forloop_body);
			skip_comment = true;
			indent_in();
			pp_free(t1);
			pprint(ip1->forloop_body->nexti, pc->target_break, NO_PPRINT_FLAGS);
			indent_out();
			indent(SPACEOVER);
			fprintf(prof_fp, "}");
			end_line(pc->target_break);
			skip_comment = true;
			pc = pc->target_break;
		}
			break;

		case Op_K_switch:
			ip1 = pc + 1;
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, ip1->switch_start, NO_PPRINT_FLAGS);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {\n", t1->pp_str);
			if (pc->comment)
				print_comment(pc->comment, 0);
			pp_free(t1);
			pprint(ip1->switch_start, ip1->switch_end, NO_PPRINT_FLAGS);
			indent(SPACEOVER);
			fprintf(prof_fp, "}\n");
			if (ip1->switch_end->comment)
				print_comment(ip1->switch_end->comment, 0);
			pc = pc->target_break;
			break;

		case Op_K_case:
		case Op_K_default:
			indent(pc->stmt_start->exec_count);
			if (pc->opcode == Op_K_case) {
				t1 = pp_pop();
				fprintf(prof_fp, "%s %s:", op2str(pc->opcode), t1->pp_str);
				pp_free(t1);
			} else
				fprintf(prof_fp, "%s:", op2str(pc->opcode));

			indent_in();
			if (pc->comment != NULL) {
				if (pc->comment->memory->comment_type == EOL_COMMENT)
					fprintf(prof_fp, "\t%s", pc->comment->memory->stptr);
				else {
					fprintf(prof_fp, "\n");
					print_comment(pc->comment, indent_level);
				}
			} else
				fprintf(prof_fp, "\n");
			pprint(pc->stmt_start->nexti, pc->stmt_end->nexti, NO_PPRINT_FLAGS);
			indent_out();
			break;

		case Op_K_if:
			fprintf(prof_fp, "%s (", op2str(pc->opcode));
			pprint(pc->nexti, pc->branch_if, NO_PPRINT_FLAGS);
			t1 = pp_pop();
			fprintf(prof_fp, "%s) {", t1->pp_str);
			pp_free(t1);

			ip1 = pc->branch_if;
			if (ip1->exec_count > 0)
				fprintf(prof_fp, " # %ld", ip1->exec_count);
			ip1 = end_line(ip1);
			indent_in();
			if (pc->comment != NULL)
				print_comment(pc->comment, indent_level);
			pprint(ip1->nexti, pc->branch_else, NO_PPRINT_FLAGS);
			indent_out();
			pc = pc->branch_else;
			if (pc->nexti->opcode == Op_no_op) {	/* no following else */
				indent(SPACEOVER);
				fprintf(prof_fp, "}");
				if (pc->nexti->nexti->opcode != Op_comment
				    || pc->nexti->nexti->memory->comment_type == BLOCK_COMMENT)
					fprintf(prof_fp, "\n");
				/* else
				 	It will be printed at the top. */
			}
			/*
			 * See next case; turn off the flag so that the
			 * following else is correctly indented.
			 */
			flags &= ~IN_ELSE_IF;
			break;

		case Op_K_else:
			/*
			 * If possible, chain else-if's together on the
			 * same line.
			 *
			 * See awkgram.y:mk_condition to understand
			 * what is being checked here.
			 *
			 * Op_exec_count follows Op_K_else, check the
			 * opcode of the following instruction.
			 * Additionally, check that the subsequent if
			 * terminates where this else does; in that case
			 * it's ok to compact the if to follow the else.
			 */

			fprintf(prof_fp, "} %s ", op2str(pc->opcode));
			if (pc->nexti->nexti->opcode == Op_K_if
			    && pc->branch_end == pc->nexti->nexti->branch_else->lasti) {
				pprint(pc->nexti, pc->branch_end, IN_ELSE_IF);
			} else {
				fprintf(prof_fp, "{");
				end_line(pc);
				skip_comment = true;
				indent_in();
				if (pc->comment != NULL)
					print_comment(pc->comment, indent_level);
				pprint(pc->nexti, pc->branch_end, NO_PPRINT_FLAGS);
				indent_out();
				indent(SPACEOVER);
				fprintf(prof_fp, "}");
				end_line(pc->branch_end);
				skip_comment = true;
			}
			/*
			 * Don't do end_line() here, we get multiple blank lines after
			 * the final else in a chain of else-ifs since they all point
			 * to the same branch_end.
			 */
			pc = pc->branch_end;
			break;

		case Op_cond_exp:
		{
			NODE *f, *t, *cond;
			size_t len;
			INSTRUCTION *qm_comment = NULL, *colon_comment = NULL;

			qm_comment = pc->comment;

			pprint(pc->nexti, pc->branch_if, NO_PPRINT_FLAGS);
			ip1 = pc->branch_if;
			pprint(ip1->nexti, pc->branch_else, NO_PPRINT_FLAGS);
			ip1 = pc->branch_else->nexti;

			pc = ip1->nexti;
			colon_comment = pc->comment;
			assert(pc->opcode == Op_cond_exp);
			pprint(pc->nexti, pc->branch_end, NO_PPRINT_FLAGS);

			f = pp_pop();
			t = pp_pop();
			cond = pp_pop();

			/*
			 * This stuff handles comments that come after a ?, :, or both.
			 * Allowing newlines after ? and : is a gawk extension.
			 * Theoretically this is fragile, since ?: expressions can be nested.
			 * In practice, it's not, since if there was a comment following ? or :
			 * in the original code, then it wasn't nested.
			 */

			len = f->pp_len + t->pp_len + cond->pp_len + 12;
			if (qm_comment == NULL && colon_comment == NULL) {
				// easy case
				emalloc(str, char *, len, "pprint");
				sprintf(str, "%s ? %s : %s", cond->pp_str, t->pp_str, f->pp_str);
			} else if (qm_comment != NULL && colon_comment != NULL) {
				check_indent_level();
				len += qm_comment->memory->stlen +		// comments
					colon_comment->memory->stlen +
					2 * (indent_level + 1) + 3 +		// indentation
					t->pp_len + 6;
				emalloc(str, char *, len, "pprint");
				sprintf(str,
					"%s ? %s"	// cond ? comment
					"%.*s   %s"	// indent true-part
					" : %s"		// : comment
					"%.*s   %s",	// indent false-part
					cond->pp_str,	// condition
					qm_comment->memory->stptr,	// comment
					(int) (indent_level + 1), tabs,		// indent
					t->pp_str,			// true part
					colon_comment->memory->stptr,	// comment
					(int) (indent_level + 1), tabs,		// indent
					f->pp_str			// false part
					);
			} else if (qm_comment != NULL) {
				check_indent_level();
				len += qm_comment->memory->stlen +	// comment
					1 * (indent_level + 1) + 3 +	// indentation
					t->pp_len + 3;
				emalloc(str, char *, len, "pprint");
				sprintf(str,
					"%s ? %s"	// cond ? comment
					"%.*s   %s"	// indent true-part
					" : %s",	// : false-part
					cond->pp_str,	// condition
					qm_comment->memory->stptr,	// comment
					(int) (indent_level + 1), tabs,		// indent
					t->pp_str,			// true part
					f->pp_str			// false part
					);
			} else {
				check_indent_level();
				len += colon_comment->memory->stlen +		// comment
					1 * (indent_level + 1) + 3 +		// indentation
					t->pp_len + 3;
				emalloc(str, char *, len, "pprint");
				sprintf(str,
					"%s ? %s"	// cond ? true-part
					" : %s"		// : comment
					"%.*s   %s",	// indent false-part
					cond->pp_str,			// condition
					t->pp_str,			// true part
					colon_comment->memory->stptr,	// comment
					(int) (indent_level + 1), tabs,		// indent
					f->pp_str			// false part
					);
			}

			pp_free(cond);
			pp_free(t);
			pp_free(f);
			pp_push(Op_cond_exp, str, CAN_FREE, pc->comment);
			pc = pc->branch_end;
		}
			break;

		case Op_exec_count:
			if (flags == NO_PPRINT_FLAGS)
				indent(pc->exec_count);
			break;

		case Op_comment:
			print_comment(pc, 0);
			break;

		case Op_list:
			break;

		default:
			cant_happen();
		}

		if (pc == endp)
			break;
	}
}

/* end_line --- end pretty print line with new line or on-line comment  */

INSTRUCTION *
end_line(INSTRUCTION *ip)
{
	INSTRUCTION *ret = ip;

	if (ip->nexti->opcode == Op_comment
	    && ip->nexti->memory->comment_type == EOL_COMMENT) {
		fprintf(prof_fp, "\t");
		print_comment(ip->nexti, -1);
		ret = ip->nexti;
	}
	else
		fprintf(prof_fp, "\n");

	return ret;
}

/* pp_string_fp --- pretty print a string to the fp */

/*
 * This routine concentrates string pretty printing in one place,
 * so that it can be called from multiple places within gawk.
 */

void
pp_string_fp(Func_print print_func, FILE *fp, const char *in_str,
		size_t len, int delim, bool breaklines)
{
	char *s = pp_string(in_str, len, delim);
	int count;
	size_t slen;
	const char *str = (const char *) s;
#define BREAKPOINT	70 /* arbitrary */

	slen = strlen(str);
	for (count = 0; slen > 0; slen--, str++) {
		print_func(fp, "%c", *str);
		if (++count >= BREAKPOINT && breaklines) {
			print_func(fp, "%c\n%c", delim, delim);
			count = 0;
		}
	}
	efree(s);
}


/* just_dump --- dump the profile and function stack and keep going */

static void
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

static void
dump_and_exit(int signum)
{
	just_dump(signum);
	final_exit(EXIT_FAILURE);
}

/* print_lib_list --- print a list of all libraries loaded */

static void
print_lib_list(FILE *prof_fp)
{
	SRCFILE *s;
	static bool printed_header = false;
	const char *indent = "";
	bool found = false;

	if (do_profile)
		indent = "\t";

	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if (s->stype == SRC_EXTLIB) {
			if (do_profile && ! printed_header) {
				printed_header = true;
				fprintf(prof_fp, _("%s# Loaded extensions (-l and/or @load)\n\n"), indent);
			}
			found = true;
			fprintf(prof_fp, "%s@load \"%s\"", indent, s->src);
			if (s->comment != NULL) {
				fprintf(prof_fp, "\t");
				print_comment(s->comment, indent_level + 1);
			} else
				fprintf(prof_fp, "\n");
		}
	}
	if (found)	/* we found some */
		fprintf(prof_fp, "\n");
}

/* print_include_list --- print a list of all files included */

static void
print_include_list(FILE *prof_fp)
{
	SRCFILE *s;
	static bool printed_header = false;
	bool found = false;

	if (do_profile)
		return;

	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if (s->stype == SRC_INC) {
			if (! printed_header) {
				printed_header = true;
				fprintf(prof_fp, _("\n# Included files (-i and/or @include)\n\n"));
			}
			found = true;
			fprintf(prof_fp, "# @include \"%s\"", s->src);
			if (s->comment != NULL) {
				fprintf(prof_fp, "\t");
				print_comment(s->comment, indent_level + 1);
			} else
				fprintf(prof_fp, "\n");
		}
	}
	if (found)	/* we found some */
		fprintf(prof_fp, "\n");
}

/* print_comment --- print comment text with proper indentation */

static void
print_comment(INSTRUCTION* pc, long in)
{
	char *text;
	size_t count;
	bool after_newline = false;

	count = pc->memory->stlen;
	text = pc->memory->stptr;

	if (in >= 0)
		indent(in);    /* is this correct? Where should comments go?  */
	for (; count > 0; count--, text++) {
		if (after_newline) {
			indent(in);
			after_newline = false;
		}
		putc(*text, prof_fp);
		after_newline = (*text == '\n');
	}

	if (pc->comment) {
		// chaining should only be two deep
		assert(pc->comment->comment == NULL);
		// if first was EOL comment, next must be block comment,
		// it needs to be indented.
		if (pc->memory->comment_type == EOL_COMMENT)
			in++;
		print_comment(pc->comment, in);
	}
}

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
	if (do_profile)
		fprintf(prof_fp, _("\t# gawk profile, created %s\n"), ctime(& now));
	print_lib_list(prof_fp);
	pprint(code, NULL, NO_PPRINT_FLAGS);
	print_include_list(prof_fp);
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
		return 16;

	case Op_field_spec:
	case Op_field_spec_lhs:
		return 15;

	case Op_preincrement:
	case Op_predecrement:
	case Op_postincrement:
	case Op_postdecrement:
		return 14;

	case Op_exp:
	case Op_exp_i:
		return 13;

	case Op_unary_minus:
	case Op_unary_plus:
	case Op_not:
		return 12;

	case Op_times:
	case Op_times_i:
	case Op_quotient:
	case Op_quotient_i:
	case Op_mod:
	case Op_mod_i:
		return 11;

	case Op_plus:
	case Op_plus_i:
	case Op_minus:
	case Op_minus_i:
		return 10;

	case Op_concat:
	case Op_assign_concat:
		return 9;

	case Op_equal:
	case Op_notequal:
	case Op_greater:
	case Op_less:
	case Op_leq:
	case Op_geq:
		return 8;

	case Op_match:
	case Op_nomatch:
		return 7;

	case Op_K_getline:
	case Op_K_getline_redir:
		return 6;

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

/* is_scalar --- return true if scalar, false otherwise */

static bool
is_scalar(int type)
{
	switch (type) {
	case Op_push_lhs:
	case Op_push_param:
	case Op_push_array:
	case Op_push:
	case Op_push_i:
	case Op_push_re:
	case Op_subscript:
	case Op_subscript_lhs:
	case Op_func_call:
	case Op_builtin:
	case Op_field_spec:
	case Op_field_spec_lhs:
	case Op_preincrement:
	case Op_predecrement:
	case Op_postincrement:
	case Op_postdecrement:
	case Op_unary_minus:
	case Op_unary_plus:
	case Op_not:
		return true;

	default:
		return false;
	}
}

/* is_binary --- return true if type represents a binary operator */

static bool
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
		return true;

	default:
		return false;
	}
}

/* pp_parenthesize --- parenthesize an expression in stack */

static void
pp_parenthesize(NODE *sp)
{
	char *p = sp->pp_str;
	size_t len = sp->pp_len;

	if (p[0] == '(')	// already parenthesized
		return;

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

/* parenthesize --- parenthesize two nodes relative to parent node type */

static void
parenthesize(int type, NODE *left, NODE *right)
{
	int rprec = prec_level(right->type);
	int lprec = prec_level(left->type);
	int prec = prec_level(type);

	if (lprec < prec)
		pp_parenthesize(left);
	if (rprec < prec)
		pp_parenthesize(right);
}

/* pp_string --- pretty format a string or regular regex constant */

char *
pp_string(const char *in_str, size_t len, int delim)
{
	return pp_string_or_typed_regex(in_str, len, delim, false);
}

/* pp_typed_regex --- pretty format a hard regex constant */

static char *
pp_typed_regex(const char *in_str, size_t len, int delim)
{
	return pp_string_or_typed_regex(in_str, len, delim, true);
}

/* pp_string_or_typed_regex --- pretty format a string, regex, or typed regex constant */

char *
pp_string_or_typed_regex(const char *in_str, size_t len, int delim, bool typed_regex)
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

	/* initial size; 3 for delim + terminating null, 1 for @ */
	osiz = len + 3 + 1 + (typed_regex == true);
	emalloc(obuf, char *, osiz, "pp_string");
	obufout = obuf;
	ofre = osiz - 1;

	if (typed_regex)
		*obufout++ = '@';

	*obufout++ = delim;
	for (; len > 0; len--, str++) {
		chksize(2);		/* make space for 2 chars */
		if (delim != '/' && *str == delim) {
			*obufout++ = '\\';
			*obufout++ = delim;
		} else if (*str == '\0') {
			*obufout++ = '\\';
			*obufout++ = '0';
			chksize(2);	/* need 2 more chars for this case */
			*obufout++ = '0';
			*obufout++ = '0';
		} else if ((cp = strchr(escapes, *str)) != NULL) {
			i = cp - escapes;
			*obufout++ = '\\';
			*obufout++ = printables[i];
		/* NB: Deliberate use of lower-case versions. */
		} else if (isascii(*str) && isprint(*str)) {
			*obufout++ = *str;
			ofre += 1;	/* used 1 less than expected */
		} else {
			size_t len;

			chksize(8);		/* total available space is 10 */

			sprintf(obufout, "\\%03o", *str & 0xff);
			len = strlen(obufout);
			ofre += (10 - len);	 /* adjust free space count */
			obufout += len;
		}
	}
	chksize(2);
	*obufout++ = delim;
	*obufout = '\0';
	return obuf;
#undef chksize
}

/* pp_number --- pretty format a number */

char *
pp_number(NODE *n)
{
	char *str;

	assert((n->flags & NUMCONSTSTR) != 0);
	emalloc(str, char *, n->stlen + 1, "pp_number");
	strcpy(str, n->stptr);
	return str;
}

/* pp_node --- pretty format a node */

char *
pp_node(NODE *n)
{
	if ((n->flags & NUMBER) != 0)
		return pp_number(n);
	return pp_string(n->stptr, n->stlen, '"');
}

/* pp_list --- pretty print a list, with surrounding characters and separator */

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
	INSTRUCTION *comment = NULL;

	if (pp_args == NULL) {
		npp_args = nargs;
		emalloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_list");
	} else if (nargs > npp_args) {
		npp_args = nargs;
		erealloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_list");
	}

	delimlen = strlen(delim);
	if (nargs == 0)
		len = 2;
	else {
		len = -delimlen;
		for (i = 1; i <= nargs; i++) {
			r = pp_args[i] = pp_pop();
			len += r->pp_len + delimlen;
			if (r->pp_comment != NULL) {
				comment = (INSTRUCTION *) r->pp_comment;
				len += comment->memory->stlen + indent_level + 1;	// comment\n ident
			}
		}
		if (paren != NULL) {
			assert(strlen(paren) == 2);
			len += 2;
		}
	}
	comment = NULL;

	emalloc(str, char *, len + 1, "pp_list");
	s = str;
	if (paren != NULL)
		*s++ = paren[0];

	for (i = nargs; i > 0; i--) {
		// argument
		r = pp_args[i];
		memcpy(s, r->pp_str, r->pp_len);
		s += r->pp_len;

		// delimiter
		if (i > 1 && delimlen > 0) {
			memcpy(s, delim, delimlen);
			s += delimlen;
		}

		// comment if any
		if (r->pp_comment != NULL) {
			check_indent_level();
			comment = (INSTRUCTION *) r->pp_comment;
			memcpy(s, comment->memory->stptr, comment->memory->stlen);
			s += comment->memory->stlen;
			memcpy(s, tabs, indent_level + 1);
			s += indent_level + 1;
		}
		pp_free(r);
	}

	if (paren != NULL)
		*s++ = paren[1];
	*s = '\0';
	return str;
}

/* is_unary_minus --- return true if string starts with unary minus */

static bool
is_unary_minus(const char *str)
{
	return str[0] == '-' && str[1] != '-';
}

/* pp_concat --- handle concatenation and correct parenthesizing of expressions */

static char *
pp_concat(int nargs)
{
	NODE *r;
 	char *str, *s;
	size_t len;
	static const size_t delimlen = 1;	/* " " */
	int i;
	int pl_l, pl_r;

	if (pp_args == NULL) {
		npp_args = nargs;
		emalloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_concat");
	} else if (nargs > npp_args) {
		npp_args = nargs;
		erealloc(pp_args, NODE **, (nargs + 2) * sizeof(NODE *), "pp_concat");
	}

	/*
	 * items are on the stack in reverse order that they
	 * will be printed so pop them off backwards.
	 */

	len = -delimlen;
	for (i = nargs; i >= 1; i--) {
		r = pp_args[i] = pp_pop();
		len += r->pp_len + delimlen + 2;
	}

	emalloc(str, char *, len + 1, "pp_concat");
	s = str;

	/* now copy in */
	for (i = 1; i < nargs; i++) {
		r = pp_args[i];

		if (r->pp_str[0] != '(') {
			pl_l = prec_level(pp_args[i]->type);
			pl_r = prec_level(pp_args[i+1]->type);

			if (i >= 2 && is_unary_minus(r->pp_str)) {
				*s++ = '(';
				memcpy(s, r->pp_str, r->pp_len);
				s += r->pp_len;
				*s++ = ')';
			} else if (is_scalar(pp_args[i]->type) && is_scalar(pp_args[i+1]->type)) {
				memcpy(s, r->pp_str, r->pp_len);
				s += r->pp_len;
			} else if (pl_l <= pl_r || is_scalar(pp_args[i+1]->type)) {
				*s++ = '(';
				memcpy(s, r->pp_str, r->pp_len);
				s += r->pp_len;
				*s++ = ')';
			} else {
				memcpy(s, r->pp_str, r->pp_len);
				s += r->pp_len;
			}
		} else {
			memcpy(s, r->pp_str, r->pp_len);
			s += r->pp_len;
		}

		if (i < nargs) {
			*s++ = ' ';
		}
	}

	pl_l = prec_level(pp_args[nargs-1]->type);
	pl_r = prec_level(pp_args[nargs]->type);
	r = pp_args[nargs];
	if (r->pp_str[0] == '(') {
		memcpy(s, r->pp_str, r->pp_len);
		s += r->pp_len;
	} else if (is_unary_minus(r->pp_str) || ((pl_l >= pl_r && ! is_scalar(pp_args[nargs]->type)))) {
		*s++ = '(';
		memcpy(s, r->pp_str, r->pp_len);
		s += r->pp_len;
		*s++ = ')';
	} else {
		memcpy(s, r->pp_str, r->pp_len);
		s += r->pp_len;
	}

	for (i = nargs; i >= 1; i--) {
		pp_free(pp_args[i]);
	}

	*s = '\0';
	return str;
}

/* pp_group3 --- string together up to 3 strings */

static char *
pp_group3(const char *s1, const char *s2, const char *s3)
{
	size_t len1, len2, len3, l;
	char *str, *s;

	len1 = strlen(s1);
	len2 = strlen(s2);
	len3 = strlen(s3);
	l = len1 + len2 + len3 + 1;
	emalloc(str, char *, l, "pp_group3");
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
	static bool first = true;
	NODE *func;
	int pcount;
	INSTRUCTION *fp;

	if (first) {
		first = false;
		if (do_profile)
			fprintf(prof_fp, _("\n\t# Functions, listed alphabetically\n"));
	}

	pp_namespace_list(pc[3].nexti);

	fp = pc->nexti->nexti;
	func = pc->func_body;
	fprintf(prof_fp, "\n");

	/* print any function comment */
	if (pc->comment != NULL)
		print_comment(pc->comment, -1);	/* -1 ==> don't indent */

	indent(pc->nexti->exec_count);
	
	bool malloced = false;
	char *name = adjust_namespace(func->vname, & malloced);
	fprintf(prof_fp, "%s %s(", op2str(Op_K_function), name);
	if (malloced)
		free(name);
	pcount = func->param_cnt;
	func_params = func->fparms;
	for (j = 0; j < pcount; j++) {
		fprintf(prof_fp, "%s", func_params[j].param);
		if (j < pcount - 1)
			fprintf(prof_fp, ", ");
	}
	if (fp->opcode == Op_comment
		&& fp->memory->comment_type == EOL_COMMENT) {
		fprintf(prof_fp, ")");
		fp = end_line(fp);
	} else
		fprintf(prof_fp, ")\n");
	if (do_profile)
		indent(0);
	fprintf(prof_fp, "{\n");
	indent_in();
	pprint(fp, NULL, NO_PPRINT_FLAGS);	/* function body */
	indent_out();
	if (do_profile)
		indent(0);
	fprintf(prof_fp, "}\n");
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
		" < ",	/* redirect_input */
		" |& ",	/* redirect_twoway */
	};

	if (redirtype < 0 || redirtype > redirect_twoway)
		fatal(_("redir2str: unknown redirection type %d"), redirtype);
	return redirtab[redirtype];
}

/* pp_namespace --- print @namespace directive */

static void
pp_namespace(const char *name, INSTRUCTION *comment)
{
	// Don't print the initial `@namespace "awk"' unless
	// @namespace was used at some point in the program
	if (! namespace_changed)
		return;

	if (strcmp(current_namespace, name) == 0)
		return;

	// don't need to free current_namespace, it comes from
	// info saved in Op_namespace instructions.
	current_namespace = name;

	if (do_profile)
		indent(SPACEOVER);

	fprintf(prof_fp, "@namespace \"%s\"", name);

	if (comment != NULL) {
		putc('\t', prof_fp);
		print_comment(comment, 0);
		putc('\n', prof_fp);
	} else
		fprintf(prof_fp, "\n\n");
}

/* pp_namespace_list --- print the list, back to front, using recursion */

static void
pp_namespace_list(INSTRUCTION *list)
{
	if (list == NULL)
		return;

	pp_namespace_list(list->nexti);
	pp_namespace(list->ns_name, list->comment);
}

/* adjust_namespace --- remove leading namespace or add leading awk:: */

static char *
adjust_namespace(char *name, bool *malloced)
{
	*malloced = false;

	// unadorned name from symbol table, add awk:: if not in awk:: n.s.
	if (strchr(name, ':') == NULL &&
	    current_namespace != awk_namespace &&	// can be equal if namespace never changed
	    strcmp(current_namespace, "awk") != 0 &&
	    ! is_all_upper(name)) {
		char *buf;
		size_t len = 5 + strlen(name) + 1;

		emalloc(buf, char *, len, "adjust_namespace");
		sprintf(buf, "awk::%s", name);
		*malloced = true;

		return buf;
	}

	// qualifed name, remove <ns>:: if in that n.s.
	size_t len = strlen(current_namespace);

	if (strncmp(current_namespace, name, len) == 0) {
		char *ret = name + len + 2;

		return ret;
	}

	return name;
}
