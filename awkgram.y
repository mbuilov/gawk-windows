/*
 * awkgram.y --- yacc/bison parser
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2020 the Free Software Foundation, Inc.
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

%{
#ifdef GAWKDEBUG
#define YYDEBUG 12
#endif

#include "awk.h"

#if defined(__STDC__) && __STDC__ < 1	/* VMS weirdness, maybe elsewhere */
#define signed /**/
#endif

static void yyerror(const char *m, ...) ATTRIBUTE_PRINTF_1;
static void error_ln(int line, const char *m, ...) ATTRIBUTE_PRINTF_2;
static void lintwarn_ln(int line, const char *m, ...) ATTRIBUTE_PRINTF_2;
static void warning_ln(int line, const char *m, ...) ATTRIBUTE_PRINTF_2;
static char *get_src_buf(void);
static int yylex(void);
int	yyparse(void);
static INSTRUCTION *snode(INSTRUCTION *subn, INSTRUCTION *op);
static char **check_params(char *fname, int pcount, INSTRUCTION *list);
static int install_function(char *fname, INSTRUCTION *fi, INSTRUCTION *plist);
static NODE *mk_rexp(INSTRUCTION *exp);
static void param_sanity(INSTRUCTION *arglist);
static int parms_shadow(INSTRUCTION *pc, bool *shadow);
#ifndef NO_LINT
static int isnoeffect(OPCODE type);
#endif
static INSTRUCTION *make_assignable(INSTRUCTION *ip);
static void dumpintlstr(const char *str, size_t len);
static void dumpintlstr2(const char *str1, size_t len1, const char *str2, size_t len2);
static bool include_source(INSTRUCTION *file, void **srcfile_p);
static bool load_library(INSTRUCTION *file, void **srcfile_p);
static void set_namespace(INSTRUCTION *ns, INSTRUCTION *comment);
static void next_sourcefile(void);
static char *tokexpand(void);
static NODE *set_profile_text(NODE *n, const char *str, size_t len);
static int check_qualified_special(char *token);
static char *qualify_name(const char *name, size_t len);
static INSTRUCTION *trailing_comment;
static INSTRUCTION *outer_comment;
static INSTRUCTION *interblock_comment;
static INSTRUCTION *pending_comment;
static INSTRUCTION *namespace_chain;

#ifdef DEBUG_COMMENTS
static void
debug_print_comment_s(const char *name, INSTRUCTION *comment, int line)
{
	if (comment != NULL)
		fprintf(stderr, "%d: %s: <%.*s>\n", line, name,
				(int) (comment->memory->stlen - 1),
				comment->memory->stptr);
}
#define debug_print_comment(comment) \
	 debug_print_comment_s(# comment, comment, __LINE__)
#endif

#define instruction(t)	bcalloc(t, 1, 0)

static INSTRUCTION *mk_program(void);
static INSTRUCTION *append_rule(INSTRUCTION *pattern, INSTRUCTION *action);
static INSTRUCTION *mk_function(INSTRUCTION *fi, INSTRUCTION *def);
static INSTRUCTION *mk_condition(INSTRUCTION *cond, INSTRUCTION *ifp, INSTRUCTION *true_branch,
		INSTRUCTION *elsep,	INSTRUCTION *false_branch);
static INSTRUCTION *mk_expression_list(INSTRUCTION *list, INSTRUCTION *s1);
static INSTRUCTION *mk_for_loop(INSTRUCTION *forp, INSTRUCTION *init, INSTRUCTION *cond,
		INSTRUCTION *incr, INSTRUCTION *body);
static void fix_break_continue(INSTRUCTION *list, INSTRUCTION *b_target, INSTRUCTION *c_target);
static INSTRUCTION *mk_binary(INSTRUCTION *s1, INSTRUCTION *s2, INSTRUCTION *op);
static INSTRUCTION *mk_boolean(INSTRUCTION *left, INSTRUCTION *right, INSTRUCTION *op);
static INSTRUCTION *mk_assignment(INSTRUCTION *lhs, INSTRUCTION *rhs, INSTRUCTION *op);
static INSTRUCTION *mk_getline(INSTRUCTION *op, INSTRUCTION *opt_var, INSTRUCTION *redir, int redirtype);
static int count_expressions(INSTRUCTION **list, bool isarg);
static INSTRUCTION *optimize_assignment(INSTRUCTION *exp);
static void add_lint(INSTRUCTION *list, LINTTYPE linttype);

enum defref { FUNC_DEFINE, FUNC_USE, FUNC_EXT };
static void func_use(const char *name, enum defref how);
static void check_funcs(void);

static ssize_t read_one_line(int fd, void *buffer, size_t count);
static int one_line_close(int fd);
static void merge_comments(INSTRUCTION *c1, INSTRUCTION *c2);
static INSTRUCTION *make_braced_statements(INSTRUCTION *lbrace, INSTRUCTION *stmts, INSTRUCTION *rbrace);
static void add_sign_to_num(NODE *n, char sign);

static bool at_seen = false;
static bool want_source = false;
static bool want_regexp = false;	/* lexical scanning kludge */
static enum {
	FUNC_HEADER,
	FUNC_BODY,
	DONT_CHECK
} want_param_names = DONT_CHECK;	/* ditto */
static bool in_function;		/* parsing kludge */
static int rule = 0;

const char *const ruletab[] = {
	"?",
	"BEGIN",
	"Rule",
	"END",
	"BEGINFILE",
	"ENDFILE",
};

static bool in_print = false;	/* lexical scanning kludge for print */
static int in_parens = 0;	/* lexical scanning kludge for print */
static int sub_counter = 0;	/* array dimension counter for use in delete */
static char *lexptr;		/* pointer to next char during parsing */
static char *lexend;		/* end of buffer */
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static char *lexeme;		/* beginning of lexeme for debugging */
static bool lexeof;		/* seen EOF for current source? */
static char *thisline = NULL;
static int in_braces = 0;	/* count braces for firstline, lastline in an 'action' */
static int lastline = 0;
static int firstline = 0;
static SRCFILE *sourcefile = NULL;	/* current program source */
static int lasttok = 0;
static bool eof_warned = false;	/* GLOBAL: want warning for each file */
static int break_allowed;	/* kludge for break */
static int continue_allowed;	/* kludge for continue */

#define END_FILE	-1000
#define END_SRC  	-2000

#define YYDEBUG_LEXER_TEXT (lexeme)
static char *tokstart = NULL;
static char *tok = NULL;
static char *tokend;
int errcount = 0;

extern char *source;
extern int sourceline;
extern SRCFILE *srcfiles;
extern INSTRUCTION *rule_list;
extern int max_args;
extern NODE **args_array;

const char awk_namespace[] = "awk";
const char *current_namespace = awk_namespace;
bool namespace_changed = false;

static INSTRUCTION *rule_block[sizeof(ruletab)];

static INSTRUCTION *ip_rec;
static INSTRUCTION *ip_newfile;
static INSTRUCTION *ip_atexit = NULL;
static INSTRUCTION *ip_end;
static INSTRUCTION *ip_endfile;
static INSTRUCTION *ip_beginfile;
INSTRUCTION *main_beginfile;
static bool called_from_eval = false;

static inline INSTRUCTION *list_create(INSTRUCTION *x);
static inline INSTRUCTION *list_append(INSTRUCTION *l, INSTRUCTION *x);
static inline INSTRUCTION *list_prepend(INSTRUCTION *l, INSTRUCTION *x);
static inline INSTRUCTION *list_merge(INSTRUCTION *l1, INSTRUCTION *l2);

extern double fmod(double x, double y);

#define YYSTYPE INSTRUCTION *
%}

%token FUNC_CALL NAME REGEXP FILENAME
%token YNUMBER YSTRING TYPED_REGEXP
%token RELOP IO_OUT IO_IN
%token ASSIGNOP ASSIGN MATCHOP CONCAT_OP
%token SUBSCRIPT
%token LEX_BEGIN LEX_END LEX_IF LEX_ELSE LEX_RETURN LEX_DELETE
%token LEX_SWITCH LEX_CASE LEX_DEFAULT LEX_WHILE LEX_DO LEX_FOR LEX_BREAK LEX_CONTINUE
%token LEX_PRINT LEX_PRINTF LEX_NEXT LEX_EXIT LEX_FUNCTION
%token LEX_BEGINFILE LEX_ENDFILE
%token LEX_GETLINE LEX_NEXTFILE
%token LEX_IN
%token LEX_AND LEX_OR INCREMENT DECREMENT
%token LEX_BUILTIN LEX_LENGTH
%token LEX_EOF
%token LEX_INCLUDE LEX_EVAL LEX_LOAD LEX_NAMESPACE
%token NEWLINE

/* Lowest to highest */
%right ASSIGNOP ASSIGN SLASH_BEFORE_EQUAL
%right '?' ':'
%left LEX_OR
%left LEX_AND
%left LEX_GETLINE
%nonassoc LEX_IN
%left FUNC_CALL LEX_BUILTIN LEX_LENGTH
%nonassoc ','
%left MATCHOP
%nonassoc RELOP '<' '>' IO_IN IO_OUT
%left CONCAT_OP
%left YSTRING YNUMBER TYPED_REGEXP
%left '+' '-'
%left '*' '/' '%'
%right '!' UNARY
%right '^'
%left INCREMENT DECREMENT
%left '$'
%left '(' ')'
%%

program
	: /* empty */
	  { $$ = NULL; }
	| program rule
	  {
		rule = 0;
		yyerrok;
	  }
	| program nls
	  {
		if ($2 != NULL) {
			if ($1 == NULL)
				outer_comment = $2;
			else
				interblock_comment = $2;
		}
		$$ = $1;
	  }
	| program LEX_EOF
	  {
		next_sourcefile();
	  }
	| program error
	  {
		rule = 0;
		/*
		 * If errors, give up, don't produce an infinite
		 * stream of syntax error messages.
		 */
  		/* yyerrok; */
	  }
	;

rule
	: pattern action
	  {
		(void) append_rule($1, $2);
		if (pending_comment != NULL) {
			interblock_comment = pending_comment;
			pending_comment = NULL;
		}
	  }
	| pattern statement_term
	  {
		if (rule != Rule) {
			msg(_("%s blocks must have an action part"), ruletab[rule]);
			errcount++;
		} else if ($1 == NULL) {
			msg(_("each rule must have a pattern or an action part"));
			errcount++;
		} else {	/* pattern rule with non-empty pattern */
			if ($2 != NULL)
				list_append($1, $2);
			(void) append_rule($1, NULL);
		}
	  }
	| function_prologue action
	  {
		in_function = false;
		(void) mk_function($1, $2);
		want_param_names = DONT_CHECK;
		if (pending_comment != NULL) {
			interblock_comment = pending_comment;
			pending_comment = NULL;
		}
		yyerrok;
	  }
	| '@' LEX_INCLUDE source statement_term
	  {
		want_source = false;
		at_seen = false;
		if ($3 != NULL && $4 != NULL) {
			SRCFILE *s = (SRCFILE *) $3;
			s->comment = $4;
		}
		yyerrok;
	  }
	| '@' LEX_LOAD library statement_term
	  {
		want_source = false;
		at_seen = false;
		if ($3 != NULL && $4 != NULL) {
			SRCFILE *s = (SRCFILE *) $3;
			s->comment = $4;
		}
		yyerrok;
	  }
	| '@' LEX_NAMESPACE namespace statement_term
	  {
		want_source = false;
		at_seen = false;

		// this frees $3 storage in all cases
		set_namespace($3, $4);

		yyerrok;
	  }
	;

source
	: FILENAME
	  {
		void *srcfile = NULL;

		if (! include_source($1, & srcfile))
			YYABORT;
		efree($1->lextok);
		bcfree($1);
		$$ = (INSTRUCTION *) srcfile;
	  }
	| FILENAME error
	  { $$ = NULL; }
	| error
	  { $$ = NULL; }
	;

library
	: FILENAME
	  {
		void *srcfile;

		if (! load_library($1, & srcfile))
			YYABORT;
		efree($1->lextok);
		bcfree($1);
		$$ = (INSTRUCTION *) srcfile;
	  }
	| FILENAME error
	  { $$ = NULL; }
	| error
	  { $$ = NULL; }
	;

namespace
	: FILENAME
	  { $$ = $1; }
	| FILENAME error
	  { $$ = NULL; }
	| error
	  { $$ = NULL; }
	;

pattern
	: /* empty */
	  {
		rule = Rule;
		$$ = NULL;
	  }
	| exp
	  {
		rule = Rule;
	  }

	| exp comma exp
	  {
		INSTRUCTION *tp;

		add_lint($1, LINT_assign_in_cond);
		add_lint($3, LINT_assign_in_cond);

		tp = instruction(Op_no_op);
		list_prepend($1, bcalloc(Op_line_range, !!do_pretty_print + 1, 0));
		$1->nexti->triggered = false;
		$1->nexti->target_jmp = $3->nexti;

		list_append($1, instruction(Op_cond_pair));
		$1->lasti->line_range = $1->nexti;
		$1->lasti->target_jmp = tp;

		list_append($3, instruction(Op_cond_pair));
		$3->lasti->line_range = $1->nexti;
		$3->lasti->target_jmp = tp;
		if (do_pretty_print) {
			($1->nexti + 1)->condpair_left = $1->lasti;
			($1->nexti + 1)->condpair_right = $3->lasti;
		}
		/* Put any comments in front of the range expression */
		if ($2 != NULL)
			$$ = list_append(list_merge(list_prepend($1, $2), $3), tp);
		else
			$$ = list_append(list_merge($1, $3), tp);
		rule = Rule;
	  }
	| LEX_BEGIN
	  {
		static int begin_seen = 0;

		if (do_lint_old && ++begin_seen == 2)
			lintwarn_ln($1->source_line,
				_("old awk does not support multiple `BEGIN' or `END' rules"));

		$1->in_rule = rule = BEGIN;
		$1->source_file = source;
		$$ = $1;
	  }
	| LEX_END
	  {
		static int end_seen = 0;

		if (do_lint_old && ++end_seen == 2)
			lintwarn_ln($1->source_line,
				_("old awk does not support multiple `BEGIN' or `END' rules"));

		$1->in_rule = rule = END;
		$1->source_file = source;
		$$ = $1;
	  }
	| LEX_BEGINFILE
	  {
		$1->in_rule = rule = BEGINFILE;
		$1->source_file = source;
		$$ = $1;
	  }
	| LEX_ENDFILE
	  {
		$1->in_rule = rule = ENDFILE;
		$1->source_file = source;
		$$ = $1;
	  }
	;

action
	: l_brace statements r_brace opt_semi opt_nls
	  {
		INSTRUCTION *ip = make_braced_statements($1, $2, $3);

		if ($3 != NULL && $5 != NULL) {
			merge_comments($3, $5);
			pending_comment = $3;
		} else if ($3 != NULL) {
			pending_comment = $3;
		} else if ($5 != NULL) {
			pending_comment = $5;
		}

		$$ = ip;
	  }
	;

func_name
	: NAME
	| FUNC_CALL
	  {
		const char *name = $1->lextok;
		char *qname = qualify_name(name, strlen(name));

		if (qname != name) {
			efree((void *)name);
			$1->lextok = qname;
		}
		$$ = $1;
	  }
	| lex_builtin
	  {
		yyerror(_("`%s' is a built-in function, it cannot be redefined"),
					tokstart);
		YYABORT;
	  }
	| '@' LEX_EVAL
	  {
		$$ = $2;
		at_seen = false;
	  }
	;

lex_builtin
	: LEX_BUILTIN
	| LEX_LENGTH
	;

function_prologue
	: LEX_FUNCTION func_name '(' { want_param_names = FUNC_HEADER; } opt_param_list r_paren opt_nls
	  {
		INSTRUCTION *func_comment = NULL;
		// Merge any comments found in the parameter list with those
		// following the function header, associate the whole shebang
		// with the function as one block comment.
		if ($5 != NULL && $5->comment != NULL) {
			if ($7 != NULL) {
				merge_comments($5->comment, $7);
			}
			func_comment = $5->comment;
		} else if ($7 != NULL) {
			func_comment = $7;
		}

		$1->source_file = source;
		$1->comment = func_comment;
		if (install_function($2->lextok, $1, $5) < 0)
			YYABORT;
		in_function = true;
		$2->lextok = NULL;
		bcfree($2);
		/* $5 already free'd in install_function */
		$$ = $1;
		want_param_names = FUNC_BODY;
	  }
	;

regexp
	/*
	 * In this rule, want_regexp tells yylex that the next thing
	 * is a regexp so it should read up to the closing slash.
	 */
	: a_slash
		{ want_regexp = true; }
	  REGEXP	/* The terminating '/' is consumed by yylex(). */
		{
		  NODE *n, *exp;
		  char *re;
		  size_t len;

		  re = $3->lextok;
		  $3->lextok = NULL;
		  len = strlen(re);
		  if (do_lint) {
			if (len == 0)
				lintwarn_ln($3->source_line,
					_("regexp constant `//' looks like a C++ comment, but is not"));
			else if (re[0] == '*' && re[len-1] == '*')
				/* possible C comment */
				lintwarn_ln($3->source_line,
					_("regexp constant `/%s/' looks like a C comment, but is not"), re);
		  }

		  exp = make_str_node(re, len, ALREADY_MALLOCED);
		  n = make_regnode(Node_regex, exp);
		  if (n == NULL) {
			unref(exp);
			YYABORT;
		  }
		  $$ = $3;
		  $$->opcode = Op_match_rec;
		  $$->memory = n;
		}
	;

typed_regexp
	: TYPED_REGEXP
		{
		  char *re;
		  size_t len;

		  re = $1->lextok;
		  $1->lextok = NULL;
		  len = strlen(re);

		  $$ = $1;
		  $$->opcode = Op_push_re;
		  $$->memory = make_typed_regex(re, len);
		}

a_slash
	: '/'
	  { bcfree($1); }
	| SLASH_BEFORE_EQUAL
	;

statements
	: /* empty */
	  { $$ = NULL; }
	| statements statement
	  {
		if ($2 == NULL) {
			$$ = $1;
		} else {
			add_lint($2, LINT_no_effect);
			if ($1 == NULL) {
				$$ = $2;
			} else {
				$$ = list_merge($1, $2);
			}
		}

		if (trailing_comment != NULL) {
			$$ = list_append($$, trailing_comment);
			trailing_comment = NULL;
		}

		yyerrok;
	  }
	| statements error
	  {	$$ = NULL; }
	;

statement_term
	: nls		{ $$ = $1; }
	| semi opt_nls	{ $$ = $2; }
	;

statement
	: semi opt_nls
	  {
		if ($2 != NULL) {
			INSTRUCTION *ip;

			merge_comments($2, NULL);
			ip = list_create(instruction(Op_no_op));
			$$ = list_append(ip, $2); 
		} else
			$$ = NULL;
	  }
	| l_brace statements r_brace
	  {
		trailing_comment = $3;	// NULL or comment
		$$ = make_braced_statements($1, $2, $3);
	  }
	| if_statement
	  {
		if (do_pretty_print)
			$$ = list_prepend($1, instruction(Op_exec_count));
		else
			$$ = $1;
 	  }
	| LEX_SWITCH '(' exp r_paren opt_nls l_brace case_statements opt_nls r_brace
	  {
		INSTRUCTION *dflt, *curr = NULL, *cexp, *cstmt;
		INSTRUCTION *ip, *nextc, *tbreak;
		const char **case_values = NULL;
		int maxcount = 128;
		int case_count = 0;
		int i;

		tbreak = instruction(Op_no_op);
		cstmt = list_create(tbreak);
		cexp = list_create(instruction(Op_pop));
		dflt = instruction(Op_jmp);
		dflt->target_jmp = tbreak;	/* if no case match and no explicit default */

		if ($7 != NULL) {
			curr = $7->nexti;
			bcfree($7);	/* Op_list */
		}
		/*  else
			curr = NULL; */

		for (; curr != NULL; curr = nextc) {
			INSTRUCTION *caseexp = curr->case_exp;
			INSTRUCTION *casestmt = curr->case_stmt;

			nextc = curr->nexti;
			if (curr->opcode == Op_K_case) {
				if (caseexp->opcode == Op_push_i) {
					/* a constant scalar */
					char *caseval;
					caseval = force_string(caseexp->memory)->stptr;
					for (i = 0; i < case_count; i++) {
						if (strcmp(caseval, case_values[i]) == 0)
							error_ln(curr->source_line,
								_("duplicate case values in switch body: %s"), caseval);
					}

					if (case_values == NULL)
						emalloc(case_values, const char **, sizeof(char *) * maxcount, "statement");
					else if (case_count >= maxcount) {
						maxcount += 128;
						erealloc(case_values, const char **, sizeof(char*) * maxcount, "statement");
					}
					case_values[case_count++] = caseval;
				} else {
					/* match a constant regex against switch expression. */
					(curr + 1)->match_exp = true;
				}
				curr->stmt_start = casestmt->nexti;
				curr->stmt_end	= casestmt->lasti;
				(void) list_prepend(cexp, curr);
				(void) list_prepend(cexp, caseexp);
			} else {
				if (dflt->target_jmp != tbreak)
					error_ln(curr->source_line,
						_("duplicate `default' detected in switch body"));
				else
					dflt->target_jmp = casestmt->nexti;

				if (do_pretty_print) {
					curr->stmt_start = casestmt->nexti;
					curr->stmt_end = casestmt->lasti;
					(void) list_prepend(cexp, curr);
				} else
					bcfree(curr);
			}

			cstmt = list_merge(casestmt, cstmt);
		}

		if (case_values != NULL)
			efree(case_values);

		ip = $3;
		if (do_pretty_print) {
			// first merge comments
			INSTRUCTION *head_comment = NULL;

			if ($5 != NULL && $6 != NULL) {
				merge_comments($5, $6);
				head_comment = $5;
			} else if ($5 != NULL)
				head_comment = $5;
			else
				head_comment = $6;

			$1->comment = head_comment;

			(void) list_prepend(ip, $1);
			(void) list_prepend(ip, instruction(Op_exec_count));
			$1->target_break = tbreak;
			($1 + 1)->switch_start = cexp->nexti;
			($1 + 1)->switch_end = cexp->lasti;
			($1 + 1)->switch_end->comment = $9;
		}
		/* else
			$1 is NULL */

		(void) list_append(cexp, dflt);
		(void) list_merge(ip, cexp);
		if ($8 != NULL)
			(void) list_append(cstmt, $8);
		$$ = list_merge(ip, cstmt);

		break_allowed--;
		fix_break_continue(ip, tbreak, NULL);
	  }
	| LEX_WHILE '(' exp r_paren opt_nls statement
	  {
		/*
		 *    -----------------
		 * tc:
		 *         cond
		 *    -----------------
		 *    [Op_jmp_false tb   ]
		 *    -----------------
		 *         body
		 *    -----------------
		 *    [Op_jmp      tc    ]
		 * tb:[Op_no_op          ]
		 */

		INSTRUCTION *ip, *tbreak, *tcont;

		tbreak = instruction(Op_no_op);
		add_lint($3, LINT_assign_in_cond);
		tcont = $3->nexti;
		ip = list_append($3, instruction(Op_jmp_false));
		ip->lasti->target_jmp = tbreak;

		if (do_pretty_print) {
			(void) list_append(ip, instruction(Op_exec_count));
			$1->target_break = tbreak;
			$1->target_continue = tcont;
			($1 + 1)->while_body = ip->lasti;
			(void) list_prepend(ip, $1);
		}
		/* else
			$1 is NULL */

		if ($5 != NULL) {
			if ($6 == NULL)
				$6 = list_create(instruction(Op_no_op));

			$5->memory->comment_type = BLOCK_COMMENT;
			$6 = list_prepend($6, $5);
		}

		if ($6 != NULL)
			(void) list_merge(ip, $6);
		(void) list_append(ip, instruction(Op_jmp));
		ip->lasti->target_jmp = tcont;
		$$ = list_append(ip, tbreak);

		break_allowed--;
		continue_allowed--;
		fix_break_continue(ip, tbreak, tcont);
	  }
	| LEX_DO opt_nls statement LEX_WHILE '(' exp r_paren opt_nls
	  {
		/*
		 *    -----------------
		 * z:
		 *         body
		 *    -----------------
		 * tc:
		 *         cond
		 *    -----------------
		 *    [Op_jmp_true | z  ]
		 * tb:[Op_no_op         ]
		 */

		INSTRUCTION *ip, *tbreak, *tcont;

		tbreak = instruction(Op_no_op);
		tcont = $6->nexti;
		add_lint($6, LINT_assign_in_cond);
		if ($3 != NULL)
			ip = list_merge($3, $6);
		else
			ip = list_prepend($6, instruction(Op_no_op));

		if ($2 != NULL)
			(void) list_prepend(ip, $2);

		if (do_pretty_print)
			(void) list_prepend(ip, instruction(Op_exec_count));

		(void) list_append(ip, instruction(Op_jmp_true));
		ip->lasti->target_jmp = ip->nexti;
		$$ = list_append(ip, tbreak);

		break_allowed--;
		continue_allowed--;
		fix_break_continue(ip, tbreak, tcont);

		if (do_pretty_print) {
			$1->target_break = tbreak;
			$1->target_continue = tcont;
			($1 + 1)->doloop_cond = tcont;
			$$ = list_prepend(ip, $1);
			bcfree($4);
			if ($8 != NULL)
				$1->comment = $8;
		}
		/* else
			$1 and $4 are NULLs */
	  }
	| LEX_FOR '(' NAME LEX_IN simple_variable r_paren opt_nls statement
	  {
		INSTRUCTION *ip;
		char *var_name = $3->lextok;

		if ($8 != NULL
				&& $8->lasti->opcode == Op_K_delete
				&& $8->lasti->expr_count == 1
				&& $8->nexti->opcode == Op_push
				&& ($8->nexti->memory->type != Node_var || !($8->nexti->memory->var_update))
				&& strcmp($8->nexti->memory->vname, var_name) == 0
		) {

		/*
		 * Efficiency hack.  Recognize the special case of
		 *
		 * 	for (iggy in foo)
		 * 		delete foo[iggy]
		 *
		 * and treat it as if it were
		 *
		 * 	delete foo
		 *
		 * Check that the body is a `delete a[i]' statement,
		 * and that both the loop var and array names match.
		 */
			NODE *arr = NULL;

			ip = $8->nexti->nexti;
			if ($5->nexti->opcode == Op_push && $5->lasti == $5->nexti)
				arr = $5->nexti->memory;
			if (arr != NULL
					&& ip->opcode == Op_no_op
					&& ip->nexti->opcode == Op_push_array
					&& strcmp(ip->nexti->memory->vname, arr->vname) == 0
					&& ip->nexti->nexti == $8->lasti
			) {
				(void) make_assignable($8->nexti);
				$8->lasti->opcode = Op_K_delete_loop;
				$8->lasti->expr_count = 0;
				if ($1 != NULL)
					bcfree($1);
				efree(var_name);
				bcfree($3);
				bcfree($4);
				bcfree($5);
				if ($7 != NULL) {
					merge_comments($7, NULL);
					$8 = list_prepend($8, $7);
				}
				$$ = $8;
			} else
				goto regular_loop;
		} else {
			INSTRUCTION *tbreak, *tcont;

			/*    [ Op_push_array a       ]
			 *    [ Op_arrayfor_init | ib ]
			 * ic:[ Op_arrayfor_incr | ib ]
			 *    [ Op_var_assign if any  ]
			 *
			 *              body
			 *
			 *    [Op_jmp | ic            ]
			 * ib:[Op_arrayfor_final      ]
			 */
regular_loop:
			ip = $5;
			ip->nexti->opcode = Op_push_array;

			tbreak = instruction(Op_arrayfor_final);
			$4->opcode = Op_arrayfor_incr;
			$4->array_var = variable($3->source_line, var_name, Node_var);
			$4->target_jmp = tbreak;
			tcont = $4;
			$3->opcode = Op_arrayfor_init;
			$3->target_jmp = tbreak;
			(void) list_append(ip, $3);

			if (do_pretty_print) {
				$1->opcode = Op_K_arrayfor;
				$1->target_continue = tcont;
				$1->target_break = tbreak;
				(void) list_append(ip, $1);
			}
			/* else
				$1 is NULL */

			/* add update_FOO instruction if necessary */
			if ($4->array_var->type == Node_var && $4->array_var->var_update) {
				(void) list_append(ip, instruction(Op_var_update));
				ip->lasti->update_var = $4->array_var->var_update;
			}
			(void) list_append(ip, $4);

			/* add set_FOO instruction if necessary */
			if ($4->array_var->type == Node_var && $4->array_var->var_assign) {
				(void) list_append(ip, instruction(Op_var_assign));
				ip->lasti->assign_var = $4->array_var->var_assign;
			}

			if (do_pretty_print) {
				(void) list_append(ip, instruction(Op_exec_count));
				($1 + 1)->forloop_cond = $4;
				($1 + 1)->forloop_body = ip->lasti;
			}

			if ($7 != NULL)
				merge_comments($7, NULL);

			if ($8 != NULL) {
				if ($7 != NULL)
					$8 = list_prepend($8, $7);
				(void) list_merge(ip, $8);
			} else if ($7 != NULL)
				(void) list_append(ip, $7);

			(void) list_append(ip, instruction(Op_jmp));
			ip->lasti->target_jmp = $4;
			$$ = list_append(ip, tbreak);
			fix_break_continue(ip, tbreak, tcont);
		}

		break_allowed--;
		continue_allowed--;
	  }
	| LEX_FOR '(' opt_simple_stmt semi opt_nls exp semi opt_nls opt_simple_stmt r_paren opt_nls statement
	  {
		if ($5 != NULL) {
			merge_comments($5, NULL);
			$1->comment = $5;
		}
		if ($8 != NULL) {
			merge_comments($8, NULL);
			if ($1->comment == NULL) {
				$8->memory->comment_type = FOR_COMMENT;
				$1->comment = $8;
			} else
				$1->comment->comment = $8;
		}
		if ($11 != NULL)
			$12 = list_prepend($12, $11);
		add_lint($6, LINT_assign_in_cond);
		$$ = mk_for_loop($1, $3, $6, $9, $12);

		break_allowed--;
		continue_allowed--;
	  }
	| LEX_FOR '(' opt_simple_stmt semi opt_nls semi opt_nls opt_simple_stmt r_paren opt_nls statement
	  {
		if ($5 != NULL) {
			merge_comments($5, NULL);
			$1->comment = $5;
		}
		if ($7 != NULL) {
			merge_comments($7, NULL);
			if ($1->comment == NULL) {
				$7->memory->comment_type = FOR_COMMENT;
				$1->comment = $7;
			} else
				$1->comment->comment = $7;
		}
		if ($10 != NULL)
			$11 = list_prepend($11, $10);
		$$ = mk_for_loop($1, $3, (INSTRUCTION *) NULL, $8, $11);

		break_allowed--;
		continue_allowed--;
	  }
	| non_compound_stmt
	  {
		if (do_pretty_print)
			$$ = list_prepend($1, instruction(Op_exec_count));
		else
			$$ = $1;
	  }
	;

non_compound_stmt
	: LEX_BREAK statement_term
	  {
		if (! break_allowed)
			error_ln($1->source_line,
				_("`break' is not allowed outside a loop or switch"));
		$1->target_jmp = NULL;
		$$ = list_create($1);
		if ($2 != NULL)
			$$ = list_append($$, $2);
	  }
	| LEX_CONTINUE statement_term
	  {
		if (! continue_allowed)
			error_ln($1->source_line,
				_("`continue' is not allowed outside a loop"));
		$1->target_jmp = NULL;
		$$ = list_create($1);
		if ($2 != NULL)
			$$ = list_append($$, $2);
	  }
	| LEX_NEXT statement_term
	  {
		/* if inside function (rule = 0), resolve context at run-time */
		if (rule && rule != Rule)
			error_ln($1->source_line,
				_("`next' used in %s action"), ruletab[rule]);
		$1->target_jmp = ip_rec;
		$$ = list_create($1);
		if ($2 != NULL)
			$$ = list_append($$, $2);
	  }
	| LEX_NEXTFILE statement_term
	  {
		/* if inside function (rule = 0), resolve context at run-time */
		if (rule == BEGIN || rule == END || rule == ENDFILE)
			error_ln($1->source_line,
				_("`nextfile' used in %s action"), ruletab[rule]);

		$1->target_newfile = ip_newfile;
		$1->target_endfile = ip_endfile;
		$$ = list_create($1);
		if ($2 != NULL)
			$$ = list_append($$, $2);
	  }
	| LEX_EXIT opt_exp statement_term
	  {
		/* Initialize the two possible jump targets, the actual target
		 * is resolved at run-time.
		 */
		$1->target_end = ip_end;	/* first instruction in end_block */
		$1->target_atexit = ip_atexit;	/* cleanup and go home */

		if ($2 == NULL) {
			$$ = list_create($1);
			(void) list_prepend($$, instruction(Op_push_i));
			$$->nexti->memory = dupnode(Nnull_string);
		} else
			$$ = list_append($2, $1);
		if ($3 != NULL)
			$$ = list_append($$, $3);
	  }
	| LEX_RETURN
	  {
		if (! in_function)
			yyerror(_("`return' used outside function context"));
	  } opt_exp statement_term {
		if (called_from_eval)
			$1->opcode = Op_K_return_from_eval;

		if ($3 == NULL) {
			$$ = list_create($1);
			(void) list_prepend($$, instruction(Op_push_i));
			$$->nexti->memory = dupnode(Nnull_string);
		} else
			$$ = list_append($3, $1);
		if ($4 != NULL)
			$$ = list_append($$, $4);
	  }
	| simple_stmt statement_term
	  {
		if ($2 != NULL)
			$$ = list_append($1, $2);
		else
			$$ = $1;
	  }
	;

	/*
	 * A simple_stmt exists to satisfy a constraint in the POSIX
	 * grammar allowing them to occur as the 1st and 3rd parts
	 * in a `for (...;...;...)' loop.  This is a historical oddity
	 * inherited from Unix awk, not at all documented in the AK&W
	 * awk book.  We support it, as this was reported as a bug.
	 * We don't bother to document it though. So there.
	 */
simple_stmt
	: print { in_print = true; in_parens = 0; } print_expression_list output_redir
	  {
		/*
		 * Optimization: plain `print' has no expression list, so $3 is null.
		 * If $3 is NULL or is a bytecode list for $0 use Op_K_print_rec,
		 * which is faster for these two cases.
		 */

		if (do_optimize && $1->opcode == Op_K_print &&
			($3 == NULL
				|| ($3->lasti->opcode == Op_field_spec
					&& $3->nexti->nexti->nexti == $3->lasti
					&& $3->nexti->nexti->opcode == Op_push_i
					&& $3->nexti->nexti->memory->type == Node_val)
			)
		) {
			static bool warned = false;
			/*   -----------------
			 *      output_redir
			 *    [ redirect exp ]
			 *   -----------------
			 *     expression_list
			 *   ------------------
			 *    [Op_K_print_rec | NULL | redir_type | expr_count]
			 */

			if ($3 != NULL) {
				NODE *n = $3->nexti->nexti->memory;

				if (! iszero(n))
					goto regular_print;

				bcfree($3->lasti);			/* Op_field_spec */
				unref(n);				/* Node_val */
				bcfree($3->nexti->nexti);		/* Op_push_i */
				bcfree($3->nexti);			/* Op_list */
				bcfree($3);				/* Op_list */
			} else {
				if (do_lint && (rule == BEGIN || rule == END) && ! warned) {
					warned = true;
					lintwarn_ln($1->source_line,
		_("plain `print' in BEGIN or END rule should probably be `print \"\"'"));
				}
			}

			$1->expr_count = 0;
			$1->opcode = Op_K_print_rec;
			if ($4 == NULL) {    /* no redircetion */
				$1->redir_type = redirect_none;
				$$ = list_create($1);
			} else {
				INSTRUCTION *ip;
				ip = $4->nexti;
				$1->redir_type = ip->redir_type;
				$4->nexti = ip->nexti;
				bcfree(ip);
				$$ = list_append($4, $1);
			}
		} else {
			/*   -----------------
			 *    [ output_redir    ]
			 *    [ redirect exp    ]
			 *   -----------------
			 *    [ expression_list ]
			 *   ------------------
			 *    [$1 | NULL | redir_type | expr_count]
			 *
			 */
regular_print:
			if ($4 == NULL) {		/* no redirection */
				if ($3 == NULL)	{	/* print/printf without arg */
					$1->expr_count = 0;
					if ($1->opcode == Op_K_print)
						$1->opcode = Op_K_print_rec;
					$1->redir_type = redirect_none;
					$$ = list_create($1);
				} else {
					INSTRUCTION *t = $3;
					$1->expr_count = count_expressions(&t, false);
					$1->redir_type = redirect_none;
					$$ = list_append(t, $1);
				}
			} else {
				INSTRUCTION *ip;
				ip = $4->nexti;
				$1->redir_type = ip->redir_type;
				$4->nexti = ip->nexti;
				bcfree(ip);
				if ($3 == NULL) {
					$1->expr_count = 0;
					if ($1->opcode == Op_K_print)
						$1->opcode = Op_K_print_rec;
					$$ = list_append($4, $1);
				} else {
					INSTRUCTION *t = $3;
					$1->expr_count = count_expressions(&t, false);
					$$ = list_append(list_merge($4, t), $1);
				}
			}
		}
	  }

	| LEX_DELETE NAME { sub_counter = 0; } delete_subscript_list
	  {
		char *arr = $2->lextok;

		$2->opcode = Op_push_array;
		$2->memory = variable($2->source_line, arr, Node_var_new);

		if (! do_posix && ! do_traditional) {
			if ($2->memory == symbol_table)
				fatal(_("`delete' is not allowed with SYMTAB"));
			else if ($2->memory == func_table)
				fatal(_("`delete' is not allowed with FUNCTAB"));
		}

		if ($4 == NULL) {
			/*
			 * As of September 2012, POSIX has added support
			 * for `delete array'. See:
			 * http://austingroupbugs.net/view.php?id=544
			 *
			 * Thanks to Nathan Weeks for the initiative.
			 *
			 * Thus we no longer warn or check do_posix.
			 * Also, since BWK awk supports it, we don't have to
			 * check do_traditional either.
			 */
			$1->expr_count = 0;
			$$ = list_append(list_create($2), $1);
		} else {
			$1->expr_count = sub_counter;
			$$ = list_append(list_append($4, $2), $1);
		}
	  }
	| LEX_DELETE '(' NAME ')'
		  /*
		   * this is for tawk compatibility. maybe the warnings
		   * should always be done.
		   */
	  {
		static bool warned = false;
		char *arr = $3->lextok;

		if (do_lint && ! warned) {
			warned = true;
			lintwarn_ln($1->source_line,
				_("`delete(array)' is a non-portable tawk extension"));
		}
		if (do_traditional) {
			error_ln($1->source_line,
				_("`delete(array)' is a non-portable tawk extension"));
		}
		$3->memory = variable($3->source_line, arr, Node_var_new);
		$3->opcode = Op_push_array;
		$1->expr_count = 0;
		$$ = list_append(list_create($3), $1);

		if (! do_posix && ! do_traditional) {
			if ($3->memory == symbol_table)
				fatal(_("`delete' is not allowed with SYMTAB"));
			else if ($3->memory == func_table)
				fatal(_("`delete' is not allowed with FUNCTAB"));
		}
	  }
	| exp
	  {
		$$ = optimize_assignment($1);
	  }
	;

opt_simple_stmt
	: /* empty */
	  { $$ = NULL; }
	| simple_stmt
	  { $$ = $1; }
	;

case_statements
	: /* empty */
	  { $$ = NULL; }
	| case_statements case_statement
	  {
		if ($1 == NULL)
			$$ = list_create($2);
		else
			$$ = list_prepend($1, $2);
	  }
	| case_statements error
	  { $$ = NULL; }
	;

case_statement
	: LEX_CASE case_value colon opt_nls statements
	  {
		INSTRUCTION *casestmt = $5;
		if ($5 == NULL)
			casestmt = list_create(instruction(Op_no_op));
		if (do_pretty_print)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		$1->case_exp = $2;
		$1->case_stmt = casestmt;
		$1->comment = $4;
		bcfree($3);
		$$ = $1;
	  }
	| LEX_DEFAULT colon opt_nls statements
	  {
		INSTRUCTION *casestmt = $4;
		if ($4 == NULL)
			casestmt = list_create(instruction(Op_no_op));
		if (do_pretty_print)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		bcfree($2);
		$1->case_stmt = casestmt;
		$1->comment = $3;
		$$ = $1;
	  }
	;

case_value
	: YNUMBER
	  {	$$ = $1; }
	| '-' YNUMBER    %prec UNARY
	  {
		NODE *n = $2->memory;
		(void) force_number(n);
		negate_num(n);
		bcfree($1);
		$$ = $2;
	  }
	| '+' YNUMBER    %prec UNARY
	  {
		NODE *n = $2->lasti->memory;
		bcfree($1);
		add_sign_to_num(n, '+');
		$$ = $2;
	  }
	| YSTRING
	  {	$$ = $1; }
	| regexp
	  {
		if ($1->memory->type == Node_regex)
			$1->opcode = Op_push_re;
		else
			$1->opcode = Op_push;
		$$ = $1;
	  }
	| typed_regexp
	  {
		assert(($1->memory->flags & REGEX) == REGEX);
		$1->opcode = Op_push_re;
		$$ = $1;
	  }
	;

print
	: LEX_PRINT
	  { $$ = $1; }
	| LEX_PRINTF
	  { $$ = $1; }
	;

	/*
	 * Note: ``print(x)'' is already parsed by the first rule,
	 * so there is no good in covering it by the second one too.
	 */
print_expression_list
	: opt_expression_list
	| '(' expression_list r_paren
	  {
		$$ = $2;
	  }
	;

output_redir
	: /* empty */
	  {
		in_print = false;
		in_parens = 0;
		$$ = NULL;
	  }
	| IO_OUT { in_print = false; in_parens = 0; } common_exp
	  {
		if ($1->redir_type == redirect_twoway
		    	&& $3->lasti->opcode == Op_K_getline_redir
		   	 	&& $3->lasti->redir_type == redirect_twoway)
			yyerror(_("multistage two-way pipelines don't work"));
		if (do_lint && $1->redir_type == redirect_output && $3->lasti->opcode == Op_concat)
			lintwarn(_("concatenation as I/O `>' redirection target is ambiguous"));
		$$ = list_prepend($3, $1);
	  }
	;

if_statement
	: LEX_IF '(' exp r_paren opt_nls statement
	  {
		if ($5 != NULL)
			$1->comment = $5;
		add_lint($3, LINT_assign_in_cond);
		$$ = mk_condition($3, $1, $6, NULL, NULL);
	  }
	| LEX_IF '(' exp r_paren opt_nls statement
	     LEX_ELSE opt_nls statement
	  {
		if ($5 != NULL)
			$1->comment = $5;
		if ($8 != NULL)
			$7->comment = $8;
		add_lint($3, LINT_assign_in_cond);
		$$ = mk_condition($3, $1, $6, $7, $9);
	  }
	;

nls
	: NEWLINE
	  {
		$$ = $1;
	  }
	| nls NEWLINE
	  {
		if ($1 != NULL && $2 != NULL) {
			if ($1->memory->comment_type == EOL_COMMENT) {
				assert($2->memory->comment_type == BLOCK_COMMENT);
				$1->comment = $2;	// chain them
			} else {
				merge_comments($1, $2);
			}

			$$ = $1;
		} else if ($1 != NULL) {
			$$ = $1;
		} else if ($2 != NULL) {
			$$ = $2;
		} else
			$$ = NULL;
	  }
	;

opt_nls
	: /* empty */
	  { $$ = NULL; }
	| nls
	  { $$ = $1; }
	;

input_redir
	: /* empty */
	  { $$ = NULL; }
	| '<' simp_exp
	  {
		bcfree($1);
		$$ = $2;
	  }
	;

opt_param_list
	: /* empty */
	  { $$ = NULL; }
	| param_list
	  { $$ = $1; }
	;

param_list
	: NAME
	  {
		$1->param_count = 0;
		$$ = list_create($1);
	  }
	| param_list comma NAME
	  {
		if ($1 != NULL && $3 != NULL) {
			$3->param_count = $1->lasti->param_count + 1;
			$$ = list_append($1, $3);
			yyerrok;

			// newlines are allowed after commas, catch any comments
			if ($2 != NULL) {
				if ($1->comment != NULL)
					merge_comments($1->comment, $2);
				else
					$1->comment = $2;
			}
		} else
			$$ = NULL;
	  }
	| error
	  { $$ = NULL; }
	| param_list error
	  { $$ = $1; }
	| param_list comma error
	  { $$ = $1; }
	;

/* optional expression, as in for loop */
opt_exp
	: /* empty */
	  { $$ = NULL; }
	| exp
	  { $$ = $1; }
	;

opt_expression_list
	: /* empty */
	  { $$ = NULL; }
	| expression_list
	  { $$ = $1; }
	;

expression_list
	: exp
	  {	$$ = mk_expression_list(NULL, $1); }
	| expression_list comma exp
	  {
		if ($2 != NULL)
			$1->lasti->comment = $2;
		$$ = mk_expression_list($1, $3);
		yyerrok;
	  }
	| error
	  { $$ = NULL; }
	| expression_list error
	  {
		/*
		 * Returning the expression list instead of NULL lets
		 * snode get a list of arguments that it can count.
		 */
		$$ = $1;
	  }
	| expression_list error exp
	  {
		/* Ditto */
		$$ = mk_expression_list($1, $3);
	  }
	| expression_list comma error
	  {
		/* Ditto */
		if ($2 != NULL)
			$1->lasti->comment = $2;
		$$ = $1;
	  }
	;

opt_fcall_expression_list
	: /* empty */
	  { $$ = NULL; }
	| fcall_expression_list
	  { $$ = $1; }
	;

fcall_expression_list
	: fcall_exp
	  {	$$ = mk_expression_list(NULL, $1); }
	| fcall_expression_list comma fcall_exp
	  {
		if ($2 != NULL)
			$1->lasti->comment = $2;
		$$ = mk_expression_list($1, $3);
		yyerrok;
	  }
	| error
	  { $$ = NULL; }
	| fcall_expression_list error
	  {
		/*
		 * Returning the expression list instead of NULL lets
		 * snode get a list of arguments that it can count.
		 */
		$$ = $1;
	  }
	| fcall_expression_list error fcall_exp
	  {
		/* Ditto */
		$$ = mk_expression_list($1, $3);
	  }
	| fcall_expression_list comma error
	  {
		/* Ditto */
		if ($2 != NULL)
			$1->comment = $2;
		$$ = $1;
	  }
	;

fcall_exp
	: exp { $$ = $1; }
	| typed_regexp { $$ = list_create($1); }
	;

/* Expressions, not including the comma operator.  */
exp
	: variable assign_operator exp %prec ASSIGNOP
	  {
		if (do_lint && $3->lasti->opcode == Op_match_rec)
			lintwarn_ln($2->source_line,
				_("regular expression on right of assignment"));
		$$ = mk_assignment($1, $3, $2);
	  }
	| variable ASSIGN typed_regexp %prec ASSIGNOP
	  {
		$$ = mk_assignment($1, list_create($3), $2);
	  }
	| exp LEX_AND exp
	  {	$$ = mk_boolean($1, $3, $2); }
	| exp LEX_OR exp
	  {	$$ = mk_boolean($1, $3, $2); }
	| exp MATCHOP typed_regexp
	  {
		if ($1->lasti->opcode == Op_match_rec)
			warning_ln($2->source_line,
				_("regular expression on left of `~' or `!~' operator"));

		assert($3->opcode == Op_push_re
			&& ($3->memory->flags & REGEX) != 0);
		/* RHS is @/.../ */
		$2->memory = $3->memory;
		bcfree($3);
		$$ = list_append($1, $2);
	  }
	| exp MATCHOP exp
	  {
		if ($1->lasti->opcode == Op_match_rec)
			warning_ln($2->source_line,
				_("regular expression on left of `~' or `!~' operator"));

		if ($3->lasti == $3->nexti && $3->nexti->opcode == Op_match_rec) {
			/* RHS is /.../ */
			$2->memory = $3->nexti->memory;
			bcfree($3->nexti);	/* Op_match_rec */
			bcfree($3);			/* Op_list */
			$$ = list_append($1, $2);
		} else {
			$2->memory = make_regnode(Node_dynregex, NULL);
			$$ = list_append(list_merge($1, $3), $2);
		}
	  }
	| exp LEX_IN simple_variable
	  {
		if (do_lint_old)
			lintwarn_ln($2->source_line,
				_("old awk does not support the keyword `in' except after `for'"));
		$3->nexti->opcode = Op_push_array;
		$2->opcode = Op_in_array;
		$2->expr_count = 1;
		$$ = list_append(list_merge($1, $3), $2);
	  }
	| exp a_relop exp %prec RELOP
	  {
		if (do_lint && $3->lasti->opcode == Op_match_rec)
			lintwarn_ln($2->source_line,
				_("regular expression on right of comparison"));
		$$ = list_append(list_merge($1, $3), $2);
	  }
	| exp '?' exp ':' exp
	  { $$ = mk_condition($1, $2, $3, $4, $5); }
	| common_exp
	  { $$ = $1; }
	;

assign_operator
	: ASSIGN
	  { $$ = $1; }
	| ASSIGNOP
	  { $$ = $1; }
	| SLASH_BEFORE_EQUAL ASSIGN   /* `/=' */
	  {
		$2->opcode = Op_assign_quotient;
		$$ = $2;
	  }
	;

relop_or_less
	: RELOP
	  { $$ = $1; }
	| '<'
	  { $$ = $1; }
	;

a_relop
	: relop_or_less
	  { $$ = $1; }
	| '>'
	  { $$ = $1; }
	;

common_exp
	: simp_exp
	  { $$ = $1; }
	| simp_exp_nc
	  { $$ = $1; }
	| common_exp simp_exp %prec CONCAT_OP
	  {
		int count = 2;
		bool is_simple_var = false;

		if ($1->lasti->opcode == Op_concat) {
			/* multiple (> 2) adjacent strings optimization */
			is_simple_var = ($1->lasti->concat_flag & CSVAR) != 0;
			count = $1->lasti->expr_count + 1;
			$1->lasti->opcode = Op_no_op;
		} else {
			is_simple_var = ($1->nexti->opcode == Op_push
					&& $1->lasti == $1->nexti); /* first exp. is a simple
					                             * variable?; kludge for use
					                             * in Op_assign_concat.
		 			                             */
		}

		if (do_optimize
			&& $1->nexti == $1->lasti && $1->nexti->opcode == Op_push_i
			&& $2->nexti == $2->lasti && $2->nexti->opcode == Op_push_i
		) {
			NODE *n1 = $1->nexti->memory;
			NODE *n2 = $2->nexti->memory;
			size_t nlen;

			// 1.5 ""   # can't fold this if program mucks with CONVFMT.
			// See test #12 in test/posix.awk.
			// Also can't fold if one or the other is translatable.
			if ((n1->flags & (NUMBER|NUMINT|INTLSTR)) != 0 || (n2->flags & (NUMBER|NUMINT|INTLSTR)) != 0)
				goto plain_concat;

			n1 = force_string(n1);
			n2 = force_string(n2);
			nlen = n1->stlen + n2->stlen;
			erealloc(n1->stptr, char *, nlen + 1, "constant fold");
			memcpy(n1->stptr + n1->stlen, n2->stptr, n2->stlen);
			n1->stlen = nlen;
			n1->stptr[nlen] = '\0';
			n1->flags &= ~(NUMCUR|NUMBER|NUMINT);
			n1->flags |= (STRING|STRCUR);
			unref(n2);
			bcfree($2->nexti);
			bcfree($2);
			$$ = $1;
		} else {
	plain_concat:
			$$ = list_append(list_merge($1, $2), instruction(Op_concat));
			$$->lasti->concat_flag = (is_simple_var ? CSVAR : 0);
			$$->lasti->expr_count = count;
			if (count > max_args)
				max_args = count;
		}
	  }
	;

simp_exp
	: non_post_simp_exp
	/* Binary operators in order of decreasing precedence.  */
	| simp_exp '^' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp '*' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp '/' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp '%' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp '+' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp '-' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| LEX_GETLINE opt_variable input_redir
	  {
		/*
		 * In BEGINFILE/ENDFILE, allow `getline [var] < file'
		 */

		if ((rule == BEGINFILE || rule == ENDFILE) && $3 == NULL)
			error_ln($1->source_line,
				 _("non-redirected `getline' invalid inside `%s' rule"), ruletab[rule]);
		if (do_lint && rule == END && $3 == NULL)
			lintwarn_ln($1->source_line,
				_("non-redirected `getline' undefined inside END action"));
		$$ = mk_getline($1, $2, $3, redirect_input);
	  }
	| variable INCREMENT
	  {
		$2->opcode = Op_postincrement;
		$$ = mk_assignment($1, NULL, $2);
	  }
	| variable DECREMENT
	  {
		$2->opcode = Op_postdecrement;
		$$ = mk_assignment($1, NULL, $2);
	  }
	| '(' expression_list r_paren LEX_IN simple_variable
	  {
		if (do_lint_old) {
		    /* first one is warning so that second one comes out if warnings are fatal */
		    warning_ln($4->source_line,
				_("old awk does not support the keyword `in' except after `for'"));
		    lintwarn_ln($4->source_line,
				_("old awk does not support multidimensional arrays"));
		}
		$5->nexti->opcode = Op_push_array;
		$4->opcode = Op_in_array;
		if ($2 == NULL) {	/* error */
			errcount++;
			$4->expr_count = 0;
			$$ = list_merge($5, $4);
		} else {
			INSTRUCTION *t = $2;
			$4->expr_count = count_expressions(&t, false);
			$$ = list_append(list_merge(t, $5), $4);
		}
	  }
	;

/* Expressions containing "| getline" lose the ability to be on the
   right-hand side of a concatenation. */
simp_exp_nc
	: common_exp IO_IN LEX_GETLINE opt_variable
		{
		  $$ = mk_getline($3, $4, $1, $2->redir_type);
		  bcfree($2);
		}
	/* Binary operators in order of decreasing precedence.  */
	| simp_exp_nc '^' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp_nc '*' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp_nc '/' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp_nc '%' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp_nc '+' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	| simp_exp_nc '-' simp_exp
	  { $$ = mk_binary($1, $3, $2); }
	;

non_post_simp_exp
	: regexp
	  {
		$$ = list_create($1);
	  }
	| '!' simp_exp %prec UNARY
	  {
		if ($2->opcode == Op_match_rec) {
			$2->opcode = Op_nomatch;
			$1->opcode = Op_push_i;
			$1->memory = set_profile_text(make_number(0.0), "0", 1);
			$$ = list_append(list_append(list_create($1),
						instruction(Op_field_spec)), $2);
		} else {
			if (do_optimize && $2->nexti == $2->lasti
					&& $2->nexti->opcode == Op_push_i
					&& ($2->nexti->memory->flags & (MPFN|MPZN|INTLSTR)) == 0
			) {
				NODE *n = $2->nexti->memory;
				if ((n->flags & STRING) != 0) {
					n->numbr = (AWKNUM) (n->stlen == 0);
					n->flags &= ~(STRCUR|STRING);
					n->flags |= (NUMCUR|NUMBER);
					efree(n->stptr);
					n->stptr = NULL;
					n->stlen = 0;
				} else
					n->numbr = (AWKNUM) (n->numbr == 0.0);
				bcfree($1);
				$$ = $2;
			} else {
				$1->opcode = Op_not;
				add_lint($2, LINT_assign_in_cond);
				$$ = list_append($2, $1);
			}
		}
	   }
	| '(' exp r_paren
	  {
		// Always include. Allows us to lint warn on
		// print "foo" > "bar" 1
		// but not warn on
		// print "foo" > ("bar" 1)
		$$ = list_append($2, bcalloc(Op_parens, 1, sourceline));
	  }
	| LEX_BUILTIN '(' opt_fcall_expression_list r_paren
	  {
		$$ = snode($3, $1);
		if ($$ == NULL)
			YYABORT;
	  }
	| LEX_LENGTH '(' opt_fcall_expression_list r_paren
	  {
		$$ = snode($3, $1);
		if ($$ == NULL)
			YYABORT;
	  }
	| LEX_LENGTH
	  {
		static bool warned = false;

		if (do_lint && ! warned) {
			warned = true;
			lintwarn_ln($1->source_line,
				_("call of `length' without parentheses is not portable"));
		}
		$$ = snode(NULL, $1);
		if ($$ == NULL)
			YYABORT;
	  }
	| func_call
	| variable
	| INCREMENT variable
	  {
		$1->opcode = Op_preincrement;
		$$ = mk_assignment($2, NULL, $1);
	  }
	| DECREMENT variable
	  {
		$1->opcode = Op_predecrement;
		$$ = mk_assignment($2, NULL, $1);
	  }
	| YNUMBER
	  {
		$$ = list_create($1);
	  }
	| YSTRING
	  {
		$$ = list_create($1);
	  }
	| '-' simp_exp    %prec UNARY
	  {
		if ($2->lasti->opcode == Op_push_i
			&& ($2->lasti->memory->flags & STRING) == 0
		) {
			NODE *n = $2->lasti->memory;
			(void) force_number(n);
			negate_num(n);
			$$ = $2;
			bcfree($1);
		} else {
			$1->opcode = Op_unary_minus;
			$$ = list_append($2, $1);
		}
	  }
	| '+' simp_exp    %prec UNARY
	  {
		if ($2->lasti->opcode == Op_push_i
			&& ($2->lasti->memory->flags & STRING) == 0
			&& ($2->lasti->memory->flags & NUMCONSTSTR) != 0) {
			NODE *n = $2->lasti->memory;
			add_sign_to_num(n, '+');
			$$ = $2;
			bcfree($1);
		} else {
			/*
			 * was: $$ = $2
			 * POSIX semantics: force a conversion to numeric type
			 */
			$1->opcode = Op_unary_plus;
			$$ = list_append($2, $1);
		}
	  }
	;

func_call
	: direct_func_call
	  {
		func_use($1->lasti->func_name, FUNC_USE);
		$$ = $1;
	  }
	| '@' direct_func_call
	  {
		/* indirect function call */
		INSTRUCTION *f, *t;
		char *name;
		NODE *indirect_var;
		static bool warned = false;
		const char *msg = _("indirect function calls are a gawk extension");

		if (do_traditional || do_posix)
			yyerror("%s", msg);
		else if (do_lint_extensions && ! warned) {
			warned = true;
			lintwarn("%s", msg);
		}

		f = $2->lasti;
		f->opcode = Op_indirect_func_call;
		name = estrdup(f->func_name, strlen(f->func_name));
		if (is_std_var(name))
			yyerror(_("cannot use special variable `%s' for indirect function call"), name);
		indirect_var = variable(f->source_line, name, Node_var_new);
		t = instruction(Op_push);
		t->memory = indirect_var;

		/* prepend indirect var instead of appending to arguments (opt_expression_list),
		 * and pop it off in setup_frame (eval.c) (left to right evaluation order); Test case:
		 *		f = "fun"
		 *		@f(f="real_fun")
		 */

		$$ = list_prepend($2, t);
		at_seen = false;
	  }
	;

direct_func_call
	: FUNC_CALL '(' opt_fcall_expression_list r_paren
	  {
		NODE *n;
		char *name = $1->func_name;
		char *qname = qualify_name(name, strlen(name));

		if (qname != name) {
			efree((char *) name);
			$1->func_name = qname;
		}

		if (! at_seen) {
			n = lookup($1->func_name);
			if (n != NULL && n->type != Node_func
			    && n->type != Node_ext_func) {
				error_ln($1->source_line,
					_("attempt to use non-function `%s' in function call"),
						$1->func_name);
			}
		}

		param_sanity($3);
		$1->opcode = Op_func_call;
		$1->func_body = NULL;
		if ($3 == NULL) {	/* no argument or error */
			($1 + 1)->expr_count = 0;
			$$ = list_create($1);
		} else {
			INSTRUCTION *t = $3;
			($1 + 1)->expr_count = count_expressions(&t, true);
			$$ = list_append(t, $1);
		}
	  }
	;

opt_variable
	: /* empty */
	  { $$ = NULL; }
	| variable
	  { $$ = $1; }
	;

delete_subscript_list
	: /* empty */
	  { $$ = NULL; }
	| delete_subscript SUBSCRIPT
	  { $$ = $1; }
	;

delete_subscript
	: delete_exp_list
	  {	$$ = $1; }
	| delete_subscript delete_exp_list
	  {
		$$ = list_merge($1, $2);
	  }
	;

delete_exp_list
	: bracketed_exp_list
	  {
		INSTRUCTION *ip = $1->lasti;
		int count = ip->sub_count;	/* # of SUBSEP-seperated expressions */
		if (count > 1) {
			/* change Op_subscript or Op_sub_array to Op_concat */
			ip->opcode = Op_concat;
			ip->concat_flag = CSUBSEP;
			ip->expr_count = count;
		} else
			ip->opcode = Op_no_op;
		sub_counter++;	/* count # of dimensions */
		$$ = $1;
	  }
	;

bracketed_exp_list
	: '[' expression_list ']'
  	  {
		INSTRUCTION *t = $2;
		if ($2 == NULL) {
			error_ln($3->source_line,
				_("invalid subscript expression"));
			/* install Null string as subscript. */
			t = list_create(instruction(Op_push_i));
			t->nexti->memory = dupnode(Nnull_string);
			$3->sub_count = 1;
		} else
			$3->sub_count = count_expressions(&t, false);
		$$ = list_append(t, $3);
	  }
	;

subscript
	: bracketed_exp_list
	  {	$$ = $1; }
	| subscript bracketed_exp_list
	  {
		$$ = list_merge($1, $2);
	  }
	;

subscript_list
	: subscript SUBSCRIPT
	  { $$ = $1; }
	;

simple_variable
	: NAME
	  {
		$1->opcode = Op_push;
		$1->memory = variable($1->source_line, $1->lextok, Node_var_new);
		$$ = list_create($1);
	  }
	| NAME subscript_list
	  {
		char *arr = $1->lextok;

		$1->memory = variable($1->source_line, arr, Node_var_new);
		$1->opcode = Op_push_array;
		$$ = list_prepend($2, $1);
	  }
	;

variable
	: simple_variable
	  {
		INSTRUCTION *ip = $1->nexti;
		if (ip->opcode == Op_push
			&& ip->memory->type == Node_var
			&& ip->memory->var_update
		) {
			$$ = list_prepend($1, instruction(Op_var_update));
			$$->nexti->update_var = ip->memory->var_update;
		} else
			$$ = $1;
	  }
	| '$' non_post_simp_exp opt_incdec
	  {
		$$ = list_append($2, $1);
		if ($3 != NULL)
			mk_assignment($2, NULL, $3);
	  }
	;

opt_incdec
	: INCREMENT
	  {
		$1->opcode = Op_postincrement;
	  }
	| DECREMENT
	  {
		$1->opcode = Op_postdecrement;
	  }
	| /* empty */
	  { $$ = NULL; }
	;

l_brace
	: '{' opt_nls { $$ = $2; }
	;

r_brace
	: '}' opt_nls	{ $$ = $2; yyerrok; }
	;

r_paren
	: ')' { yyerrok; }
	;

opt_semi
	: /* empty */
	  { $$ = NULL; }
	| semi
	;

semi
	: ';'	{ yyerrok; }
	;

colon
	: ':'	{ $$ = $1; yyerrok; }
	;

comma
	: ',' opt_nls	{ $$ = $2; yyerrok; }
	;
%%

struct token {
	const char *operator;	/* text to match */
	OPCODE value;			/*  type */
	int class;				/* lexical class */
	unsigned flags;			/* # of args. allowed and compatability */
#	define	ARGS	0xFF	/* 0, 1, 2, 3 args allowed (any combination */
#	define	A(n)	(1<<(n))
#	define	VERSION_MASK	0xFF00	/* old awk is zero */
#	define	NOT_OLD		0x0100	/* feature not in old awk */
#	define	NOT_POSIX	0x0200	/* feature not in POSIX */
#	define	GAWKX		0x0400	/* gawk extension */
#	define	BREAK		0x0800	/* break allowed inside */
#	define	CONTINUE	0x1000	/* continue allowed inside */
#	define	DEBUG_USE	0x2000	/* for use by developers */

	NODE *(*ptr)(int);	/* function that implements this keyword */
	NODE *(*ptr2)(int);	/* alternate arbitrary-precision function */
};

#ifdef USE_EBCDIC
/* tokcompare --- lexicographically compare token names for sorting */

static int
tokcompare(const void *l, const void *r)
{
	struct token *lhs, *rhs;

	lhs = (struct token *) l;
	rhs = (struct token *) r;

	return strcmp(lhs->operator, rhs->operator);
}
#endif

/*
 * Tokentab is sorted ASCII ascending order, so it can be binary searched.
 * See check_special(), which sorts the table on EBCDIC systems.
 * Function pointers come from declarations in awk.h.
 */

#ifdef HAVE_MPFR
#define MPF(F) do_mpfr_##F
#else
#define MPF(F) 0
#endif

static const struct token tokentab[] = {
{"BEGIN",	Op_rule,	 LEX_BEGIN,	0,		0,	0},
{"BEGINFILE",	Op_rule,	 LEX_BEGINFILE,	GAWKX,		0,	0},
{"END",		Op_rule,	 LEX_END,	0,		0,	0},
{"ENDFILE",	Op_rule,	 LEX_ENDFILE,	GAWKX,		0,	0},
#ifdef ARRAYDEBUG
{"adump",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1)|A(2)|DEBUG_USE,	do_adump,	0},
#endif
{"and",		Op_builtin,    LEX_BUILTIN,	GAWKX,		do_and,	MPF(and)},
{"asort",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_asort,	0},
{"asorti",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_asorti,	0},
{"atan2",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2),	do_atan2,	MPF(atan2)},
{"bindtextdomain",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2),	do_bindtextdomain,	0},
{"break",	Op_K_break,	 LEX_BREAK,	0,		0,	0},
{"case",	Op_K_case,	 LEX_CASE,	GAWKX,		0,	0},
{"close",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1)|A(2),	do_close,	0},
{"compl",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_compl,	MPF(compl)},
{"continue",	Op_K_continue, LEX_CONTINUE,	0,		0,	0},
{"cos",		Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_cos,	MPF(cos)},
{"dcgettext",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_dcgettext,	0},
{"dcngettext",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3)|A(4)|A(5),	do_dcngettext,	0},
{"default",	Op_K_default,	 LEX_DEFAULT,	GAWKX,		0,	0},
{"delete",	Op_K_delete,	 LEX_DELETE,	NOT_OLD,	0,	0},
{"do",		Op_K_do,	 LEX_DO,	NOT_OLD|BREAK|CONTINUE,	0,	0},
{"else",	Op_K_else,	 LEX_ELSE,	0,		0,	0},
{"eval",	Op_symbol,	 LEX_EVAL,	0,		0,	0},
{"exit",	Op_K_exit,	 LEX_EXIT,	0,		0,	0},
{"exp",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_exp,	MPF(exp)},
{"fflush",	Op_builtin,	 LEX_BUILTIN,	A(0)|A(1), do_fflush,	0},
{"for",		Op_K_for,	 LEX_FOR,	BREAK|CONTINUE,	0,	0},
{"func",	Op_func, 	LEX_FUNCTION,	NOT_POSIX|NOT_OLD,	0,	0},
{"function",	Op_func, 	LEX_FUNCTION,	NOT_OLD,	0,	0},
{"gensub",	Op_sub_builtin,	 LEX_BUILTIN,	GAWKX|A(3)|A(4), 0,	0},
{"getline",	Op_K_getline_redir,	 LEX_GETLINE,	NOT_OLD,	0,	0},
{"gsub",	Op_sub_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), 0,	0},
{"if",		Op_K_if,	 LEX_IF,	0,		0,	0},
{"in",		Op_symbol,	 LEX_IN,	0,		0,	0},
{"include",	Op_symbol,	 LEX_INCLUDE,	GAWKX,	0,	0},
{"index",	Op_builtin,	 LEX_BUILTIN,	A(2),		do_index,	0},
{"int",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_int,	MPF(int)},
#ifdef SUPPLY_INTDIV
{"intdiv0",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(3),	do_intdiv,	MPF(intdiv)},
#endif
{"isarray",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1),	do_isarray,	0},
{"length",	Op_builtin,	 LEX_LENGTH,	A(0)|A(1),	do_length,	0},
{"load",  	Op_symbol,	 LEX_LOAD,	GAWKX,		0,	0},
{"log",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_log,	MPF(log)},
{"lshift",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_lshift,	MPF(lshift)},
{"match",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_match,	0},
{"mktime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2), do_mktime, 0},
{"namespace",  	Op_symbol,	 LEX_NAMESPACE,	GAWKX,		0,	0},
{"next",	Op_K_next,	 LEX_NEXT,	0,		0,	0},
{"nextfile",	Op_K_nextfile, LEX_NEXTFILE,	0,		0,	0},
{"or",		Op_builtin,    LEX_BUILTIN,	GAWKX,		do_or,	MPF(or)},
{"patsplit",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2)|A(3)|A(4), do_patsplit,	0},
{"print",	Op_K_print,	 LEX_PRINT,	0,		0,	0},
{"printf",	Op_K_printf,	 LEX_PRINTF,	0,		0,	0},
{"rand",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0),	do_rand,	MPF(rand)},
{"return",	Op_K_return,	 LEX_RETURN,	NOT_OLD,	0,	0},
{"rshift",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_rshift,	MPF(rshift)},
{"sin",		Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_sin,	MPF(sin)},
{"split",	Op_builtin,	 LEX_BUILTIN,	A(2)|A(3)|A(4),	do_split,	0},
{"sprintf",	Op_builtin,	 LEX_BUILTIN,	0,		do_sprintf,	0},
{"sqrt",	Op_builtin,	 LEX_BUILTIN,	A(1),		do_sqrt,	MPF(sqrt)},
{"srand",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0)|A(1), do_srand,	MPF(srand)},
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG) /* || ... */
{"stopme",	Op_builtin,	LEX_BUILTIN,	GAWKX|A(0)|DEBUG_USE,	stopme,		0},
#endif
{"strftime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(0)|A(1)|A(2)|A(3), do_strftime,	0},
{"strtonum",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_strtonum, MPF(strtonum)},
{"sub",		Op_sub_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), 0,	0},
{"substr",	Op_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_substr,	0},
{"switch",	Op_K_switch,	 LEX_SWITCH,	GAWKX|BREAK,	0,	0},
{"system",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_system,	0},
{"systime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(0),	do_systime,	0},
{"tolower",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_tolower,	0},
{"toupper",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_toupper,	0},
{"typeof",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2), do_typeof,	0},
{"while",	Op_K_while,	 LEX_WHILE,	BREAK|CONTINUE,	0,	0},
{"xor",		Op_builtin,    LEX_BUILTIN,	GAWKX,		do_xor,	MPF(xor)},
};

/* Variable containing the current shift state.  */
static mbstate_t cur_mbstate;
/* Ring buffer containing current characters.  */
#define MAX_CHAR_IN_RING_BUFFER 8
#define RING_BUFFER_SIZE (MAX_CHAR_IN_RING_BUFFER * MB_LEN_MAX)
static char cur_char_ring[RING_BUFFER_SIZE];
/* Index for ring buffers.  */
static int cur_ring_idx;
/* This macro means that last nextc() return a singlebyte character
   or 1st byte of a multibyte character.  */
#define nextc_is_1stbyte (cur_char_ring[cur_ring_idx] == 1)

/* getfname --- return name of a builtin function (for pretty printing) */

const char *
getfname(NODE *(*fptr)(int), bool prepend_awk)
{
	int i, j;
	static char buf[100];

	j = sizeof(tokentab) / sizeof(tokentab[0]);
	/* linear search, no other way to do it */
	for (i = 0; i < j; i++) {
		if (tokentab[i].ptr == fptr || tokentab[i].ptr2 == fptr) {
			if (prepend_awk && (tokentab[i].flags & GAWKX) != 0) {
				sprintf(buf, "awk::%s", tokentab[i].operator);
				return buf;
			}
			return tokentab[i].operator;
		}
	}

	return NULL;
}

/* negate_num --- negate a number in NODE */

void
negate_num(NODE *n)
{
#ifdef HAVE_MPFR
	int tval = 0;
#endif

	add_sign_to_num(n, '-');

	if (! is_mpg_number(n)) {
		n->numbr = -n->numbr;
		return;
	}

#ifdef HAVE_MPFR
	if (is_mpg_integer(n)) {
		if (! iszero(n)) {
			mpz_neg(n->mpg_i, n->mpg_i);
			return;
		}

		/*
		 * 0 --> -0 conversion. Requires turning the MPG integer
		 * into an MPFR float.
		 */

		mpz_clear(n->mpg_i);	/* release the integer storage */

		/* Convert and fall through. */
		tval = mpfr_set_d(n->mpg_numbr, 0.0, ROUND_MODE);
		IEEE_FMT(n->mpg_numbr, tval);
		n->flags &= ~MPZN;
		n->flags |= MPFN;
	}

	/* mpfr float case */
	tval = mpfr_neg(n->mpg_numbr, n->mpg_numbr, ROUND_MODE);
	IEEE_FMT(n->mpg_numbr, tval);
#endif
}

/* add_sign_to_num --- make a constant unary plus or minus for profiling */

static void
add_sign_to_num(NODE *n, char sign)
{
	if ((n->flags & NUMCONSTSTR) != 0) {
		char *s;

		s = n->stptr;
		memmove(& s[1], & s[0], n->stlen + 1);
		s[0] = sign;
		n->stlen++;
	}
}

/* print_included_from --- print `Included from ..' file names and locations */

static void
print_included_from()
{
	int saveline, line;
	SRCFILE *s;

	/* suppress current file name, line # from `.. included from ..' msgs */
	saveline = sourceline;
	sourceline = 0;

	for (s = sourcefile; s != NULL && s->stype == SRC_INC; ) {
		s = s->next;
		if (s == NULL || s->fd <= INVALID_HANDLE)
			continue;
		line = s->srclines;

		/* if last token is NEWLINE, line number is off by 1. */
		if (s->lasttok == NEWLINE)
			line--;
		msg("%s %s:%d%c",
			s->prev == sourcefile ? "In file included from"
					  : "                 from",
			(s->stype == SRC_INC ||
				 s->stype == SRC_FILE) ? s->src : "cmd. line",
			line,
			s->stype == SRC_INC ? ',' : ':'
		);
	}
	sourceline = saveline;
}

/* warning_ln --- print a warning message with location */

static void
warning_ln(int line, const char *mesg, ...)
{
	va_list args;
	int saveline;

	saveline = sourceline;
	sourceline = line;
	print_included_from();
	va_start(args, mesg);
	err(false, _("warning: "), mesg, args);
	va_end(args);
	sourceline = saveline;
}

/* lintwarn_ln --- print a lint warning and location */

static void
lintwarn_ln(int line, const char *mesg, ...)
{
	va_list args;
	int saveline;

	saveline = sourceline;
	sourceline = line;
	print_included_from();
	va_start(args, mesg);
	if (lintfunc == r_fatal)
		err(true, _("fatal: "), mesg, args);
	else
		err(false, _("warning: "), mesg, args);
	va_end(args);
	sourceline = saveline;
	if (lintfunc == r_fatal)
		gawk_exit(EXIT_FATAL);
}

/* error_ln --- print an error message and location */

static void
error_ln(int line, const char *m, ...)
{
	va_list args;
	int saveline;

	saveline = sourceline;
	sourceline = line;
	print_included_from();
	errcount++;
	va_start(args, m);
	err(false, "error: ", m, args);
	va_end(args);
	sourceline = saveline;
}

/* yyerror --- print a syntax error message, show where */

static void
yyerror(const char *m, ...)
{
	va_list args;
	const char *mesg = NULL;
	char *bp, *cp;
	char *scan;
	char *buf;
	int count;
	static char end_of_file_line[] = "(END OF FILE)";
	static char syntax_error[] = "syntax error";
	static size_t syn_err_len = sizeof(syntax_error) - 1;
	bool generic_error = (strncmp(m, syntax_error, syn_err_len) == 0);

	print_included_from();

	errcount++;
	/* Find the current line in the input file */
	if (lexptr && lexeme) {
		if (thisline == NULL) {
			cp = lexeme;
			if (*cp == '\n') {
				if (cp > lexptr_begin)
					cp--;
				mesg = _("unexpected newline or end of string");
			}
			for (; cp != lexptr_begin && *cp != '\n'; --cp)
				continue;
			if (*cp == '\n')
				cp++;
			thisline = cp;
		}
		/* NL isn't guaranteed */
		bp = lexeme;
		if (bp < thisline)
			bp = thisline + 1;
		while (bp < lexend && *bp && *bp != '\n')
			bp++;
	} else {
		thisline = end_of_file_line;
		bp = thisline + strlen(thisline);
	}

	if (lexeof && mesg == NULL && generic_error) {
		msg("%s", end_of_file_line);
		mesg = _("source files / command-line arguments must contain complete functions or rules");
	} else
		msg("%.*s", (int) (bp - thisline), thisline);

	va_start(args, m);
	if (mesg == NULL)
		mesg = m;

	count = strlen(mesg) + 1;
	if (lexptr != NULL)
		count += (lexeme - thisline) + 2;
	ezalloc(buf, char *, count+1, "yyerror");

	bp = buf;

	if (lexptr != NULL) {
		scan = thisline;
		while (scan < lexeme)
			if (*scan++ == '\t')
				*bp++ = '\t';
			else
				*bp++ = ' ';
		*bp++ = '^';
		*bp++ = ' ';
	}
	strcpy(bp, mesg);
	err(false, "", buf, args);
	va_end(args);
	efree(buf);
}

/* mk_program --- create a single list of instructions */

static INSTRUCTION *
mk_program()
{
	INSTRUCTION *cp, *tmp;

#define begin_block         rule_block[BEGIN]
#define end_block           rule_block[END]
#define prog_block          rule_block[Rule]
#define beginfile_block     rule_block[BEGINFILE]
#define endfile_block       rule_block[ENDFILE]

	if (end_block == NULL)
		end_block = list_create(ip_end);
	else
		(void) list_prepend(end_block, ip_end);

	if (! in_main_context()) {
		if (begin_block != NULL && prog_block != NULL)
			cp = list_merge(begin_block, prog_block);
		else
			cp = (begin_block != NULL) ? begin_block : prog_block;

		if (cp != NULL)
			(void) list_merge(cp, end_block);
		else
			cp = end_block;

		(void) list_append(cp, instruction(Op_stop));
		goto out;
	}

	if (endfile_block == NULL)
		endfile_block = list_create(ip_endfile);
	else {
		ip_rec->has_endfile = true;
		(void) list_prepend(endfile_block, ip_endfile);
	}

	if (beginfile_block == NULL)
		beginfile_block = list_create(ip_beginfile);
	else
		(void) list_prepend(beginfile_block, ip_beginfile);

	if (prog_block == NULL) {
		if (end_block->nexti == end_block->lasti
				&& beginfile_block->nexti == beginfile_block->lasti
				&& endfile_block->nexti == endfile_block->lasti
		) {
			/* no pattern-action and (real) end, beginfile or endfile blocks */
			bcfree(ip_rec);
			bcfree(ip_newfile);
			ip_rec = ip_newfile = NULL;

			list_append(beginfile_block, instruction(Op_after_beginfile));
			(void) list_append(endfile_block, instruction(Op_after_endfile));

			if (begin_block == NULL)     /* no program at all */
				cp = end_block;
			else
				cp = list_merge(begin_block, end_block);

			if (interblock_comment != NULL) {
				(void) list_append(cp, interblock_comment);
				interblock_comment = NULL;
			}

			(void) list_append(cp, ip_atexit);
			(void) list_append(cp, instruction(Op_stop));

			/* append beginfile_block and endfile_block for sole use
			 * in getline without redirection (Op_K_getline).
			 */

			(void) list_merge(cp, beginfile_block);
			(void) list_merge(cp, endfile_block);

			if (outer_comment != NULL) {
				cp = list_merge(list_create(outer_comment), cp);
				outer_comment = NULL;
			}

			if (interblock_comment != NULL) {
				(void) list_append(cp, interblock_comment);
				interblock_comment = NULL;
			}

			goto out;

		} else {
			/* install a do-nothing prog block */
			prog_block = list_create(instruction(Op_no_op));
		}
	}

	(void) list_append(endfile_block, instruction(Op_after_endfile));
	(void) list_prepend(prog_block, ip_rec);
	(void) list_append(prog_block, instruction(Op_jmp));
	prog_block->lasti->target_jmp = ip_rec;

	list_append(beginfile_block, instruction(Op_after_beginfile));

	cp = list_merge(beginfile_block, prog_block);
	(void) list_prepend(cp, ip_newfile);
	(void) list_merge(cp, endfile_block);
	(void) list_merge(cp, end_block);
	if (begin_block != NULL)
		cp = list_merge(begin_block, cp);

	if (outer_comment != NULL) {
		cp = list_merge(list_create(outer_comment), cp);
		outer_comment = NULL;
	}

	if (interblock_comment != NULL) {
		(void) list_append(cp, interblock_comment);
		interblock_comment = NULL;
	}

	(void) list_append(cp, ip_atexit);
	(void) list_append(cp, instruction(Op_stop));

out:
	/* delete the Op_list, not needed */
	tmp = cp->nexti;
	bcfree(cp);
	return tmp;

#undef begin_block
#undef end_block
#undef prog_block
#undef beginfile_block
#undef endfile_block
}

/* parse_program --- read in the program and convert into a list of instructions */

int
parse_program(INSTRUCTION **pcode, bool from_eval)
{
	int ret;

	called_from_eval = from_eval;

	/* pre-create non-local jump targets
	 * ip_end (Op_no_op) -- used as jump target for `exit'
	 * outside an END block.
	 */
	ip_end = instruction(Op_no_op);

	if (! in_main_context())
		ip_newfile = ip_rec = ip_atexit = ip_beginfile = ip_endfile = NULL;
	else {
		ip_endfile = instruction(Op_no_op);
		main_beginfile = ip_beginfile = instruction(Op_no_op);
		ip_rec = instruction(Op_get_record); /* target for `next', also ip_newfile */
		ip_newfile = bcalloc(Op_newfile, 2, 0); /* target for `nextfile' */
		ip_newfile->target_jmp = ip_end;
		ip_newfile->target_endfile = ip_endfile;
		(ip_newfile + 1)->target_get_record = ip_rec;
		ip_rec->target_newfile = ip_newfile;
		ip_atexit = instruction(Op_atexit);	/* target for `exit' in END block */
	}

	for (sourcefile = srcfiles->next; sourcefile->stype == SRC_EXTLIB;
			sourcefile = sourcefile->next)
		;

	lexeof = false;
	lexptr = NULL;
	lasttok = 0;
	memset(rule_block, 0, sizeof(ruletab) * sizeof(INSTRUCTION *));
	errcount = 0;
	tok = tokstart != NULL ? tokstart : tokexpand();

	ret = yyparse();
	*pcode = mk_program();

	/* avoid false source indications */
	source = NULL;
	sourceline = 0;
	if (ret == 0)	/* avoid spurious warning if parser aborted with YYABORT */
		check_funcs();

	if (do_posix && ! check_param_names())
		errcount++;

	if (args_array == NULL)
		emalloc(args_array, NODE **, (max_args + 2) * sizeof(NODE *), "parse_program");
	else
		erealloc(args_array, NODE **, (max_args + 2) * sizeof(NODE *), "parse_program");

	return (ret || errcount);
}

/* free_srcfile --- free a SRCFILE struct */

void
free_srcfile(SRCFILE *thisfile)
{
	efree(thisfile->src);
	efree(thisfile);
}

/* do_add_srcfile --- add one item to srcfiles */

static SRCFILE *
do_add_srcfile(enum srctype stype, char *src, char *path, SRCFILE *thisfile)
{
	SRCFILE *s;

	ezalloc(s, SRCFILE *, sizeof(SRCFILE), "do_add_srcfile");
	s->src = estrdup(src, strlen(src));
	s->fullpath = path;
	s->stype = stype;
	s->fd = INVALID_HANDLE;
	s->next = thisfile;
	s->prev = thisfile->prev;
	thisfile->prev->next = s;
	thisfile->prev = s;
	return s;
}

/* add_srcfile --- add one item to srcfiles after checking if
 *				a source file exists and not already in list.
 */

SRCFILE *
add_srcfile(enum srctype stype, char *src, SRCFILE *thisfile, bool *already_included, int *errcode)
{
	SRCFILE *s;
	struct stat sbuf;
	char *path;
	int errno_val = 0;

	if (already_included)
		*already_included = false;
	if (errcode)
		*errcode = 0;
	if (stype == SRC_CMDLINE || stype == SRC_STDIN)
		return do_add_srcfile(stype, src, NULL, thisfile);

	path = find_source(src, & sbuf, & errno_val, stype == SRC_EXTLIB);
	if (path == NULL) {
		if (errcode) {
			*errcode = errno_val;
			return NULL;
		}
		/* use full messages to ease translation */
		fatal(stype != SRC_EXTLIB
			? _("cannot open source file `%s' for reading: %s")
			: _("cannot open shared library `%s' for reading: %s"),
				src,
				errno_val ? strerror(errno_val) : _("reason unknown"));
	}

	/* N.B. We do not eliminate duplicate SRC_FILE (-f) programs. */
	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if ((s->stype == SRC_FILE || s->stype == SRC_INC || s->stype == SRC_EXTLIB) && files_are_same(path, s)) {
			if (stype == SRC_INC || stype == SRC_EXTLIB) {
				/* eliminate duplicates */
				if ((stype == SRC_INC) && (s->stype == SRC_FILE))
					fatal(_("cannot include `%s' and use it as a program file"), src);

				if (do_lint) {
					int line = sourceline;
					/* Kludge: the line number may be off for `@include file'.
					 * Since, this function is also used for '-f file' in main.c,
					 * sourceline > 1 check ensures that the call is at
					 * parse time.
					 */
					if (sourceline > 1 && lasttok == NEWLINE)
						line--;
					lintwarn_ln(line,
						    stype != SRC_EXTLIB
						      ? _("already included source file `%s'")
						      : _("already loaded shared library `%s'"),
						    src);
				}
				efree(path);
				if (already_included)
					*already_included = true;
				return NULL;
			} else {
				/* duplicates are allowed for -f */
				if (s->stype == SRC_INC)
					fatal(_("cannot include `%s' and use it as a program file"), src);
				/* no need to scan for further matches, since
				 * they must be of homogeneous type */
				break;
			}
		}
	}

	s = do_add_srcfile(stype, src, path, thisfile);
	s->sbuf = sbuf;
	s->mtime = sbuf.st_mtime;
	return s;
}

/* include_source --- read program from source included using `@include' */

static bool
include_source(INSTRUCTION *file, void **srcfile_p)
{
	SRCFILE *s;
	char *src = file->lextok;
	int errcode;
	bool already_included;

	*srcfile_p = NULL;

	if (do_traditional || do_posix) {
		error_ln(file->source_line, _("@include is a gawk extension"));
		return false;
	}

	if (strlen(src) == 0) {
		if (do_lint)
			lintwarn_ln(file->source_line, _("empty filename after @include"));
		return true;
	}

	s = add_srcfile(SRC_INC, src, sourcefile, &already_included, &errcode);
	if (s == NULL) {
		if (already_included)
			return true;
		error_ln(file->source_line,
			_("cannot open source file `%s' for reading: %s"),
			src, errcode ? strerror(errcode) : _("reason unknown"));
		return false;
	}

	/* save scanner state for the current sourcefile */
	sourcefile->srclines = sourceline;
	sourcefile->lexptr = lexptr;
	sourcefile->lexend = lexend;
	sourcefile->lexptr_begin = lexptr_begin;
	sourcefile->lexeme = lexeme;
	sourcefile->lasttok = lasttok;
	sourcefile->namespace = current_namespace;

	/* included file becomes the current source */
	sourcefile = s;
	lexptr = NULL;
	sourceline = 0;
	source = NULL;
	lasttok = 0;
	lexeof = false;
	eof_warned = false;
	current_namespace = awk_namespace;
	*srcfile_p = (void *) s;
	return true;
}

/* load_library --- load a shared library */

static bool
load_library(INSTRUCTION *file, void **srcfile_p)
{
	SRCFILE *s;
	char *src = file->lextok;
	int errcode;
	bool already_included;

	*srcfile_p = NULL;

	if (do_traditional || do_posix) {
		error_ln(file->source_line, _("@load is a gawk extension"));
		return false;
	}


	if (strlen(src) == 0) {
		if (do_lint)
			lintwarn_ln(file->source_line, _("empty filename after @load"));
		return true;
	}

	if (do_pretty_print && ! do_profile) {
		// create a fake one, don't try to open the file
		s = do_add_srcfile(SRC_EXTLIB, src, src, sourcefile);
	} else {
		s = add_srcfile(SRC_EXTLIB, src, sourcefile, &already_included, &errcode);
		if (s == NULL) {
			if (already_included)
				return true;
			error_ln(file->source_line,
				_("cannot open shared library `%s' for reading: %s"),
				src, errcode ? strerror(errcode) : _("reason unknown"));
			return false;
		}

		load_ext(s->fullpath);
	}

	*srcfile_p = (void *) s;
	return true;
}

/* next_sourcefile --- read program from the next source in srcfiles */

static void
next_sourcefile()
{
	static int (*closefunc)(int fd) = NULL;

	if (closefunc == NULL) {
		char *cp = getenv("AWKREADFUNC");

		/* If necessary, one day, test value for different functions.  */
		if (cp == NULL)
			closefunc = close;
		else
			closefunc = one_line_close;
	}

	/*
	 * This won't be true if there's an invalid character in
	 * the source file or source string (e.g., user typo).
	 * Previous versions of gawk did not core dump in such a
	 * case.
	 *
	 * assert(lexeof == true);
	 */

	lexeof = false;
	eof_warned = false;
	sourcefile->srclines = sourceline;	/* total no of lines in current file */
	if (sourcefile->fd > INVALID_HANDLE) {
		if (sourcefile->fd != fileno(stdin))  /* safety */
			(*closefunc)(sourcefile->fd);
		sourcefile->fd = INVALID_HANDLE;
	}
	if (sourcefile->buf != NULL) {
		efree(sourcefile->buf);
		sourcefile->buf = NULL;
		sourcefile->lexptr_begin = NULL;
	}

	while ((sourcefile = sourcefile->next) != NULL) {
		if (sourcefile == srcfiles)
			return;
		if (sourcefile->stype != SRC_EXTLIB)
			break;
	}

	if (sourcefile->lexptr_begin != NULL) {
		/* resume reading from already opened file (postponed to process '@include') */
		lexptr = sourcefile->lexptr;
		lexend = sourcefile->lexend;
		lasttok = sourcefile->lasttok;
		lexptr_begin = sourcefile->lexptr_begin;
		lexeme = sourcefile->lexeme;
		sourceline = sourcefile->srclines;
		source = sourcefile->src;
		set_current_namespace(sourcefile->namespace);
	} else {
		lexptr = NULL;
		sourceline = 0;
		source = NULL;
		lasttok = 0;
		set_current_namespace(awk_namespace);
	}
}

/* get_src_buf --- read the next buffer of source program */

static char *
get_src_buf()
{
	int n;
	char *scan;
	bool newfile;
	int savelen;
	struct stat sbuf;

	/*
	 * No argument prototype on readfunc on purpose,
	 * avoids problems with some ancient systems where
	 * the types of arguments to read() aren't up to date.
	 */
	static ssize_t (*readfunc)() = 0;

	if (readfunc == NULL) {
		char *cp = getenv("AWKREADFUNC");

		/* If necessary, one day, test value for different functions.  */
		if (cp == NULL)
			/*
			 * cast is to remove warnings on systems with
			 * different return types for read.
			 */
			readfunc = ( ssize_t(*)() ) read;
		else
			readfunc = read_one_line;
	}

	newfile = false;
	if (sourcefile == srcfiles)
		return NULL;

	if (sourcefile->stype == SRC_CMDLINE) {
		if (sourcefile->bufsize == 0) {
			sourcefile->bufsize = strlen(sourcefile->src);
			lexptr = lexptr_begin = lexeme = sourcefile->src;
			lexend = lexptr + sourcefile->bufsize;
			sourceline = 1;
			if (sourcefile->bufsize == 0) {
				/*
				 * Yet Another Special case:
				 *	gawk '' /path/name
				 * Sigh.
				 */
				static bool warned = false;

				if (do_lint && ! warned) {
					warned = true;
					lintwarn(_("empty program text on command line"));
				}
				lexeof = true;
			}
		} else if (sourcefile->buf == NULL  && *(lexptr-1) != '\n') {
			/*
			 * The following goop is to ensure that the source
			 * ends with a newline and that the entire current
			 * line is available for error messages.
			 */
			int offset;
			char *buf;

			offset = lexptr - lexeme;
			for (scan = lexeme; scan > lexptr_begin; scan--)
				if (*scan == '\n') {
					scan++;
					break;
				}
			savelen = lexptr - scan;
			emalloc(buf, char *, savelen + 1, "get_src_buf");
			memcpy(buf, scan, savelen);
			thisline = buf;
			lexptr = buf + savelen;
			*lexptr = '\n';
			lexeme = lexptr - offset;
			lexptr_begin = buf;
			lexend = lexptr + 1;
			sourcefile->buf = buf;
		} else
			lexeof = true;
		return lexptr;
	}

	if (sourcefile->fd <= INVALID_HANDLE) {
		int fd;
		int l;

		source = sourcefile->src;
		if (source == NULL)
			return NULL;
		fd = srcopen(sourcefile);
		if (fd <= INVALID_HANDLE) {
			char *in;

			/* suppress file name and line no. in error mesg */
			in = source;
			source = NULL;
			error(_("cannot open source file `%s' for reading: %s"),
				in, strerror(errno));
			errcount++;
			lexeof = true;
			return sourcefile->src;
		}

		sourcefile->fd = fd;
		l = optimal_bufsize(fd, &sbuf);
		/*
		 * Make sure that something silly like
		 * 	AWKBUFSIZE=8 make check
		 * works ok.
		 */
#define A_DECENT_BUFFER_SIZE	128
		if (l < A_DECENT_BUFFER_SIZE)
			l = A_DECENT_BUFFER_SIZE;
#undef A_DECENT_BUFFER_SIZE
		sourcefile->bufsize = l;
		newfile = true;
		emalloc(sourcefile->buf, char *, sourcefile->bufsize, "get_src_buf");
		memset(sourcefile->buf, '\0', sourcefile->bufsize);	// keep valgrind happy
		lexptr = lexptr_begin = lexeme = sourcefile->buf;
		savelen = 0;
		sourceline = 1;
		thisline = NULL;
	} else {
		/*
		 * Here, we retain the current source line in the beginning of the buffer.
		 */
		int offset;
		for (scan = lexeme; scan > lexptr_begin; scan--)
			if (*scan == '\n') {
				scan++;
				break;
			}

		savelen = lexptr - scan;
		offset = lexptr - lexeme;

		if (savelen > 0) {
			/*
			 * Need to make sure we have room left for reading new text;
			 * grow the buffer (by doubling, an arbitrary choice), if the retained line
			 * takes up more than a certain percentage (50%, again an arbitrary figure)
			 * of the available space.
			 */

			if (savelen > sourcefile->bufsize / 2) { /* long line or token  */
				sourcefile->bufsize *= 2;
				erealloc(sourcefile->buf, char *, sourcefile->bufsize, "get_src_buf");
				scan = sourcefile->buf + (scan - lexptr_begin);
				lexptr_begin = sourcefile->buf;
			}

			thisline = lexptr_begin;
			memmove(thisline, scan, savelen);
			lexptr = thisline + savelen;
			lexeme = lexptr - offset;
		} else {
			savelen = 0;
			lexptr = lexeme = lexptr_begin;
			thisline = NULL;
		}
	}

	n = (*readfunc)(sourcefile->fd, lexptr, sourcefile->bufsize - savelen);
	if (n == -1) {
		error(_("cannot read source file `%s': %s"),
				source, strerror(errno));
		errcount++;
		lexeof = true;
	} else {
		lexend = lexptr + n;
		if (n == 0) {
			static bool warned = false;
			if (do_lint && newfile && ! warned) {
				warned = true;
				sourceline = 0;
				lintwarn(_("source file `%s' is empty"), source);
			}
			lexeof = true;
		}
	}
	return sourcefile->buf;
}

/* tokadd --- add a character to the token buffer */

#define	tokadd(x) (*tok++ = (x), tok == tokend ? tokexpand() : tok)

/* tokexpand --- grow the token buffer */

static char *
tokexpand()
{
	static int toksize;
	int tokoffset;

	if (tokstart != NULL) {
		tokoffset = tok - tokstart;
		toksize *= 2;
		erealloc(tokstart, char *, toksize, "tokexpand");
		tok = tokstart + tokoffset;
	} else {
		toksize = 60;
		emalloc(tokstart, char *, toksize, "tokexpand");
		tok = tokstart;
	}
	tokend = tokstart + toksize;
	return tok;
}

/* check_bad_char --- fatal if c isn't allowed in gawk source code */

/*
 * The error message was inspired by someone who decided to put
 * a physical \0 byte into the source code to see what would
 * happen and then filed a bug report about it.  Sigh.
 */

static void
check_bad_char(int c)
{
	/* allow escapes. needed for autoconf. bleah. */
	switch (c) {
	case '\a':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
		return;
	default:
		break;
	}

	if (iscntrl(c) && ! isspace(c))
		// This is a PEBKAC error, but we'll be nice and not say so.
		fatal(_("error: invalid character '\\%03o' in source code"), c & 0xFF);
}

/* nextc --- get the next input character */

// For namespaces, -e chunks must be syntactic units.
#define NO_CONTINUE_SOURCE_STRINGS	1

static int
nextc(bool check_for_bad)
{
	if (gawk_mb_cur_max > 1) {
again:
#ifdef NO_CONTINUE_SOURCE_STRINGS
		if (lexeof)
			return END_FILE;
#else
		if (lexeof) {
			if (sourcefile->next == srcfiles)
				return END_FILE;
			else
				next_sourcefile();
		}
#endif
		if (lexptr == NULL || lexptr >= lexend) {
			if (get_src_buf())
				goto again;
			return END_SRC;
		}

		/* Update the buffer index.  */
		cur_ring_idx = (cur_ring_idx == RING_BUFFER_SIZE - 1)? 0 :
			cur_ring_idx + 1;

		/* Did we already check the current character?  */
		if (cur_char_ring[cur_ring_idx] == 0) {
			/* No, we need to check the next character on the buffer.  */
			int idx, work_ring_idx = cur_ring_idx;
			mbstate_t tmp_state;
			size_t mbclen;

			for (idx = 0; lexptr + idx < lexend; idx++) {
				tmp_state = cur_mbstate;
				mbclen = mbrlen(lexptr, idx + 1, &tmp_state);

				if (mbclen == 1 || mbclen == (size_t)-1 || mbclen == 0) {
					/* It is a singlebyte character, non-complete multibyte
					   character or EOF.  We treat it as a singlebyte
					   character.  */
					cur_char_ring[work_ring_idx] = 1;
					break;
				} else if (mbclen == (size_t)-2) {
					/* It is not a complete multibyte character.  */
					cur_char_ring[work_ring_idx] = idx + 1;
				} else {
					/* mbclen > 1 */
					cur_char_ring[work_ring_idx] = mbclen;
					break;
				}
				work_ring_idx = (work_ring_idx == RING_BUFFER_SIZE - 1)?
					0 : work_ring_idx + 1;
			}
			cur_mbstate = tmp_state;

			/* Put a mark on the position on which we write next character.  */
			work_ring_idx = (work_ring_idx == RING_BUFFER_SIZE - 1)?
				0 : work_ring_idx + 1;
			cur_char_ring[work_ring_idx] = 0;
		}
		if (check_for_bad || *lexptr == '\0')
			check_bad_char(*lexptr);

		return (int) (unsigned char) *lexptr++;
	} else {
		do {
#ifdef NO_CONTINUE_SOURCE_STRINGS
			if (lexeof)
				return END_FILE;
#else
			if (lexeof) {
				if (sourcefile->next == srcfiles)
					return END_FILE;
				else
					next_sourcefile();
			}
#endif
			if (lexptr && lexptr < lexend) {
				if (check_for_bad || *lexptr == '\0')
					check_bad_char(*lexptr);
				return ((int) (unsigned char) *lexptr++);
			}
		} while (get_src_buf());
		return END_SRC;
	}
}
#undef NO_CONTINUE_SOURCE_STRINGS

/* pushback --- push a character back on the input */

static inline void
pushback(void)
{
	if (gawk_mb_cur_max > 1)
		cur_ring_idx = (cur_ring_idx == 0)? RING_BUFFER_SIZE - 1 :
			cur_ring_idx - 1;
	(! lexeof && lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr);
}

/*
 * get_comment --- collect comment text.
 * 	Flag = EOL_COMMENT for end-of-line comments.
 * 	Flag = BLOCK_COMMENT for self-contained comments.
 */

static int
get_comment(enum commenttype flag, INSTRUCTION **comment_instruction)
{
	int c;
	int sl;
	char *p1;
	char *p2;

	tok = tokstart;
	tokadd('#');
	sl = sourceline;

	while (true) {
		while ((c = nextc(false)) != '\n' && c != END_FILE) {
			/* ignore \r characters */
			if (c != '\r')
				tokadd(c);
		}
		if (flag == EOL_COMMENT) {
			/* comment at end of line.  */
			if (c == '\n')
				tokadd(c);
			break;
		}
		if (c == '\n') {
			tokadd(c);
			sourceline++;
			do {
				c = nextc(false);
				if (c == '\n') {
					sourceline++;
					tokadd(c);
				}
			} while (isspace(c) && c != END_FILE);
			if (c == END_FILE)
				break;
			else if (c != '#') {
				pushback();
				sourceline--;
				break;
			} else
				tokadd(c);
		} else
			break;
	}

	/* remove any trailing blank lines (consecutive \n) from comment */
	p1 = tok - 1;
	p2 = tok - 2;
	while (*p1 == '\n' && *p2 == '\n') {
		p1--;
		p2--;
		tok--;
	}

	(*comment_instruction) = bcalloc(Op_comment, 1, sl);
	(*comment_instruction)->source_file = source;
	(*comment_instruction)->memory = make_str_node(tokstart, tok - tokstart, 0);
	(*comment_instruction)->memory->comment_type = flag;

	return c;
}

/* allow_newline --- allow newline after &&, ||, ? and : */

static void
allow_newline(INSTRUCTION **new_comment)
{
	int c;

	for (;;) {
		c = nextc(true);
		if (c == END_FILE) {
			pushback();
			break;
		}
		if (c == '#') {
			if (do_pretty_print && ! do_profile) {
				/* collect comment byte code iff doing pretty print but not profiling.  */
				c = get_comment(EOL_COMMENT, new_comment);
			} else {
				while ((c = nextc(false)) != '\n' && c != END_FILE)
					continue;
			}
			if (c == END_FILE) {
				pushback();
				break;
			}
		}
		if (c == '\n')
			sourceline++;
		if (! isspace(c)) {
			pushback();
			break;
		}
	}
}

/* newline_eof --- return newline or EOF as needed and adjust variables */

/*
 * This routine used to be a macro, however GCC 4.6.2 warned about
 * the result of a computation not being used.  Converting to a function
 * removes the warnings.
 */

static int
newline_eof()
{
	/* NB: a newline at end does not start a source line. */
	if (lasttok != NEWLINE) {
                pushback();
		if (do_lint && ! eof_warned) {
        		lintwarn(_("source file does not end in newline"));
			eof_warned = true;
		}
		sourceline++;
		return NEWLINE;
	}

	sourceline--;
	eof_warned = false;
	return LEX_EOF;
}

/* yylex --- Read the input and turn it into tokens. */

static int
#ifdef USE_EBCDIC
yylex_ebcdic(void)
#else
yylex(void)
#endif
{
	int c;
	bool seen_e = false;		/* These are for numbers */
	bool seen_point = false;
	bool esc_seen;		/* for literal strings */
	int mid;
	int base;
	static bool did_newline = false;
	char *tokkey;
	bool inhex = false;
	bool intlstr = false;
	AWKNUM d;
	bool collecting_typed_regexp = false;
	static int qm_col_count = 0;

#define GET_INSTRUCTION(op) bcalloc(op, 1, sourceline)

#define NEWLINE_EOF newline_eof()

	yylval = (INSTRUCTION *) NULL;
	if (lasttok == SUBSCRIPT) {
		lasttok = 0;
		return SUBSCRIPT;
	}

	if (lasttok == LEX_EOF)		/* error earlier in current source, must give up !! */
		return 0;

	c = nextc(! want_regexp);
	if (c == END_SRC)
		return 0;
	if (c == END_FILE)
		return lasttok = NEWLINE_EOF;
	pushback();

#if defined __EMX__
	/*
	 * added for OS/2's extproc feature of cmd.exe
	 * (like #! in BSD sh)
	 */
	if (strncasecmp(lexptr, "extproc ", 8) == 0) {
		while (*lexptr && *lexptr != '\n')
			lexptr++;
	}
#endif

	lexeme = lexptr;
	thisline = NULL;

collect_regexp:
	if (want_regexp) {
		int in_brack = 0;	/* count brackets, [[:alnum:]] allowed */
		int b_index = -1;
		int cur_index = 0;

		/*
		 * Here is what's ok with brackets:
		 *
		 * [..[..] []] [^]] [.../...]
		 * [...\[...] [...\]...] [...\/...]
		 *
		 * (Remember that all of the above are inside /.../)
		 *
		 * The code for \ handles \[, \] and \/.
		 *
		 * Otherwise, track the first open [ position, and if
		 * an embedded ] occurs, allow it to pass through
		 * if it's right after the first [ or after [^.
		 *
		 * Whew!
		 */

		want_regexp = false;
		tok = tokstart;
		for (;;) {
			c = nextc(false);

			cur_index = tok - tokstart;
			if (gawk_mb_cur_max == 1 || nextc_is_1stbyte) switch (c) {
			case '[':
				if (nextc(false) == ':' || in_brack == 0) {
					in_brack++;
					if (in_brack == 1)
						b_index = tok - tokstart;
				}
				pushback();
				break;
			case ']':
				if (in_brack > 0
				    && (cur_index == b_index + 1
					|| (cur_index == b_index + 2 && tok[-1] == '^')))
					; /* do nothing */
				else {
					in_brack--;
					if (in_brack == 0)
						b_index = -1;
				}
				break;
			case '\\':
				if ((c = nextc(false)) == END_FILE) {
					pushback();
					yyerror(_("unterminated regexp ends with `\\' at end of file"));
					goto end_regexp; /* kludge */
				}
				if (c == '\r')	/* allow MS-DOS files. bleah */
					c = nextc(true);
				if (c == '\n') {
					sourceline++;
					continue;
				} else {
					tokadd('\\');
					tokadd(c);
					continue;
				}
				break;
			case '/':	/* end of the regexp */
				if (in_brack > 0)
					break;
end_regexp:
				yylval = GET_INSTRUCTION(Op_token);
				yylval->lextok = estrdup(tokstart, tok - tokstart);
				if (do_lint) {
					int peek = nextc(true);

					pushback();
					if (peek == 'i' || peek == 's') {
						if (source)
							lintwarn(
						_("%s: %d: tawk regex modifier `/.../%c' doesn't work in gawk"),
								source, sourceline, peek);
						else
							lintwarn(
						_("tawk regex modifier `/.../%c' doesn't work in gawk"),
								peek);
					}
				}
				if (collecting_typed_regexp) {
					collecting_typed_regexp = false;
					lasttok = TYPED_REGEXP;
				} else
					lasttok = REGEXP;

				return lasttok;
			case '\n':
				pushback();
				yyerror(_("unterminated regexp"));
				goto end_regexp;	/* kludge */
			case END_FILE:
				pushback();
				yyerror(_("unterminated regexp at end of file"));
				goto end_regexp;	/* kludge */
			}
			tokadd(c);
		}
	}
retry:

	/* skipping \r is a hack, but windows is just too pervasive. sigh. */
	while ((c = nextc(true)) == ' ' || c == '\t' || c == '\r')
		continue;

	lexeme = lexptr ? lexptr - 1 : lexptr;
	thisline = NULL;
	tok = tokstart;

	if (gawk_mb_cur_max == 1 || nextc_is_1stbyte)
	switch (c) {
	case END_SRC:
		return 0;

	case END_FILE:
		return lasttok = NEWLINE_EOF;

	case '\n':
		sourceline++;
		return lasttok = NEWLINE;

	case '#':		/* it's a comment */
		yylval = NULL;
		if (do_pretty_print && ! do_profile) {
			/*
			 * Collect comment byte code iff doing pretty print
			 * but not profiling.
			 */
			INSTRUCTION *new_comment;

			if (lasttok == NEWLINE || lasttok == 0)
				c = get_comment(BLOCK_COMMENT, & new_comment);
			else
				c = get_comment(EOL_COMMENT, & new_comment);

			yylval = new_comment;

			if (c == END_FILE) {
				pushback();
				return lasttok = NEWLINE;
			}
		} else {
			while ((c = nextc(false)) != '\n') {
				if (c == END_FILE)
					return lasttok = NEWLINE_EOF;
			}
		}
		sourceline++;
		return lasttok = NEWLINE;

	case '@':
		c = nextc(true);
		if (c == '/') {
			want_regexp = true;
			collecting_typed_regexp = true;
			goto collect_regexp;
		}
		pushback();
		at_seen = true;
		return lasttok = '@';

	case '\\':
#ifdef RELAXED_CONTINUATION
		/*
		 * This code purports to allow comments and/or whitespace
		 * after the `\' at the end of a line used for continuation.
		 * Use it at your own risk. We think it's a bad idea, which
		 * is why it's not on by default.
		 */
		yylval = NULL;
		if (! do_traditional) {
			INSTRUCTION *new_comment;

			/* strip trailing white-space and/or comment */
			while ((c = nextc(true)) == ' ' || c == '\t' || c == '\r')
				continue;
			if (c == '#') {
				static bool warned = false;

				if (do_lint && ! warned) {
					warned = true;
					lintwarn(
		_("use of `\\ #...' line continuation is not portable"));
				}
				if (do_pretty_print && ! do_profile) {
					c = get_comment(EOL_COMMENT, & new_comment);
					yylval = new_comment;
					return lasttok = c;
				} else {
					while ((c = nextc(false)) != '\n')
						if (c == END_FILE)
							break;
				}
			}
			pushback();
		}
#endif /* RELAXED_CONTINUATION */
		c = nextc(true);
		if (c == '\r')	/* allow MS-DOS files. bleah */
			c = nextc(true);
		if (c == '\n') {
			sourceline++;
			goto retry;
		} else {
			yyerror(_("backslash not last character on line"));
			return lasttok = LEX_EOF;
		}
		break;

	case '?':
		qm_col_count++;
		// fall through
	case ':':
		yylval = GET_INSTRUCTION(Op_cond_exp);
		if (qm_col_count > 0) {
			if (! do_posix) {
				INSTRUCTION *new_comment = NULL;
				allow_newline(& new_comment);
				yylval->comment = new_comment;
			}
			if (c == ':')
				qm_col_count--;
		}
		return lasttok = c;

		/*
		 * in_parens is undefined unless we are parsing a print
		 * statement (in_print), but why bother with a check?
		 */
	case ')':
		in_parens--;
		return lasttok = c;

	case '(':
		in_parens++;
		return lasttok = c;
	case '$':
		yylval = GET_INSTRUCTION(Op_field_spec);
		return lasttok = c;
	case '{':
		if (++in_braces == 1)
			firstline = sourceline;
	case ';':
	case ',':
	case '[':
			return lasttok = c;
	case ']':
		c = nextc(true);
		pushback();
		if (c == '[') {
			if (do_traditional)
				fatal(_("multidimensional arrays are a gawk extension"));
			if (do_lint_extensions)
				lintwarn(_("multidimensional arrays are a gawk extension"));
			yylval = GET_INSTRUCTION(Op_sub_array);
			lasttok = ']';
		} else {
			yylval = GET_INSTRUCTION(Op_subscript);
			lasttok = SUBSCRIPT;	/* end of subscripts */
		}
		return ']';

	case '*':
		if ((c = nextc(true)) == '=') {
			yylval = GET_INSTRUCTION(Op_assign_times);
			return lasttok = ASSIGNOP;
		} else if (do_posix) {
			pushback();
			yylval = GET_INSTRUCTION(Op_times);
			return lasttok = '*';
		} else if (c == '*') {
			/* make ** and **= aliases for ^ and ^= */
			static bool did_warn_op = false, did_warn_assgn = false;

			if (nextc(true) == '=') {
				if (! did_warn_assgn) {
					did_warn_assgn = true;
					if (do_lint)
						lintwarn(_("POSIX does not allow operator `%s'"), "**=");
					if (do_lint_old)
						lintwarn(_("operator `%s' is not supported in old awk"), "**=");
				}
				yylval = GET_INSTRUCTION(Op_assign_exp);
				return ASSIGNOP;
			} else {
				pushback();
				if (! did_warn_op) {
					did_warn_op = true;
					if (do_lint)
						lintwarn(_("POSIX does not allow operator `%s'"), "**");
					if (do_lint_old)
						lintwarn(_("operator `%s' is not supported in old awk"), "**");
				}
				yylval = GET_INSTRUCTION(Op_exp);
				return lasttok = '^';
			}
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_times);
		return lasttok = '*';

	case '/':
		if (nextc(false) == '=') {
			pushback();
			return lasttok = SLASH_BEFORE_EQUAL;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_quotient);
		return lasttok = '/';

	case '%':
		if (nextc(true) == '=') {
			yylval = GET_INSTRUCTION(Op_assign_mod);
			return lasttok = ASSIGNOP;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_mod);
		return lasttok = '%';

	case '^':
	{
		static bool did_warn_op = false, did_warn_assgn = false;

		if (nextc(true) == '=') {
			if (do_lint_old && ! did_warn_assgn) {
				did_warn_assgn = true;
				lintwarn(_("operator `%s' is not supported in old awk"), "^=");
			}
			yylval = GET_INSTRUCTION(Op_assign_exp);
			return lasttok = ASSIGNOP;
		}
		pushback();
		if (do_lint_old && ! did_warn_op) {
			did_warn_op = true;
			lintwarn(_("operator `%s' is not supported in old awk"), "^");
		}
		yylval = GET_INSTRUCTION(Op_exp);
		return lasttok = '^';
	}

	case '+':
		if ((c = nextc(true)) == '=') {
			yylval = GET_INSTRUCTION(Op_assign_plus);
			return lasttok = ASSIGNOP;
		}
		if (c == '+') {
			yylval = GET_INSTRUCTION(Op_symbol);
			return lasttok = INCREMENT;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_plus);
		return lasttok = '+';

	case '!':
		if ((c = nextc(true)) == '=') {
			yylval = GET_INSTRUCTION(Op_notequal);
			return lasttok = RELOP;
		}
		if (c == '~') {
			yylval = GET_INSTRUCTION(Op_nomatch);
			return lasttok = MATCHOP;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_symbol);
		return lasttok = '!';

	case '<':
		if (nextc(true) == '=') {
			yylval = GET_INSTRUCTION(Op_leq);
			return lasttok = RELOP;
		}
		yylval = GET_INSTRUCTION(Op_less);
		pushback();
		return lasttok = '<';

	case '=':
		if (nextc(true) == '=') {
			yylval = GET_INSTRUCTION(Op_equal);
			return lasttok = RELOP;
		}
		yylval = GET_INSTRUCTION(Op_assign);
		pushback();
		return lasttok = ASSIGN;

	case '>':
		if ((c = nextc(true)) == '=') {
			yylval = GET_INSTRUCTION(Op_geq);
			return lasttok = RELOP;
		} else if (c == '>') {
			yylval = GET_INSTRUCTION(Op_symbol);
			yylval->redir_type = redirect_append;
			return lasttok = IO_OUT;
		}
		pushback();
		if (in_print && in_parens == 0) {
			yylval = GET_INSTRUCTION(Op_symbol);
			yylval->redir_type = redirect_output;
			return lasttok = IO_OUT;
		}
		yylval = GET_INSTRUCTION(Op_greater);
		return lasttok = '>';

	case '~':
		yylval = GET_INSTRUCTION(Op_match);
		return lasttok = MATCHOP;

	case '}':
		/*
		 * Added did newline stuff.  Easier than
		 * hacking the grammar.
		 */
		if (did_newline) {
			did_newline = false;
			if (--in_braces == 0)
				lastline = sourceline;
			return lasttok = c;
		}
		did_newline = true;
		--lexptr;	/* pick up } next time */
		return lasttok = NEWLINE;

	case '"':
	string:
		esc_seen = false;
		/*
		 * Allow any kind of junk in quoted string,
		 * so pass false to nextc().
		 */
		while ((c = nextc(false)) != '"') {
			if (c == '\n') {
				pushback();
				yyerror(_("unterminated string"));
				return lasttok = LEX_EOF;
			}
			if ((gawk_mb_cur_max == 1 || nextc_is_1stbyte) &&
			    c == '\\') {
				c = nextc(true);
				if (c == '\r')	/* allow MS-DOS files. bleah */
					c = nextc(true);
				if (c == '\n') {
					if (do_posix)
						fatal(_("POSIX does not allow physical newlines in string values"));
					else if (do_lint)
						lintwarn(_("backslash string continuation is not portable"));
					sourceline++;
					continue;
				}
				esc_seen = true;
				if (! want_source || c != '"')
					tokadd('\\');
			}
			if (c == END_FILE) {
				pushback();
				yyerror(_("unterminated string"));
				return lasttok = LEX_EOF;
			}
			tokadd(c);
		}
		yylval = GET_INSTRUCTION(Op_token);
		if (want_source) {
			yylval->lextok = estrdup(tokstart, tok - tokstart);
			return lasttok = FILENAME;
		}

		yylval->opcode = Op_push_i;
		yylval->memory = make_str_node(tokstart,
					tok - tokstart, esc_seen ? SCAN : 0);
		if (intlstr) {
			yylval->memory->flags |= INTLSTR;
			intlstr = false;
			if (do_intl)
				dumpintlstr(yylval->memory->stptr, yylval->memory->stlen);
		}
		return lasttok = YSTRING;

	case '-':
		if ((c = nextc(true)) == '=') {
			yylval = GET_INSTRUCTION(Op_assign_minus);
			return lasttok = ASSIGNOP;
		}
		if (c == '-') {
			yylval = GET_INSTRUCTION(Op_symbol);
			return lasttok = DECREMENT;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_minus);
		return lasttok = '-';

	case '.':
		c = nextc(true);
		pushback();
		if (! isdigit(c))
			return lasttok = '.';
		else
			c = '.';
		/* FALL THROUGH */
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		/* It's a number */
		for (;;) {
			bool gotnumber = false;

			tokadd(c);
			switch (c) {
			case 'x':
			case 'X':
				if (do_traditional)
					goto done;
				if (tok == tokstart + 2) {
					int peek = nextc(true);

					if (isxdigit(peek)) {
						inhex = true;
						pushback();	/* following digit */
					} else {
						pushback();	/* x or X */
						goto done;
					}
				}
				break;
			case '.':
				/* period ends exponent part of floating point number */
				if (seen_point || seen_e) {
					gotnumber = true;
					break;
				}
				seen_point = true;
				break;
			case 'e':
			case 'E':
				if (inhex)
					break;
				if (seen_e) {
					gotnumber = true;
					break;
				}
				seen_e = true;
				if ((c = nextc(true)) == '-' || c == '+') {
					int c2 = nextc(true);

					if (isdigit(c2)) {
						tokadd(c);
						tokadd(c2);
					} else {
						pushback();	/* non-digit after + or - */
						pushback();	/* + or - */
						pushback();	/* e or E */
					}
				} else if (! isdigit(c)) {
					pushback();	/* character after e or E */
					pushback();	/* e or E */
				} else {
					pushback();	/* digit */
				}
				break;
			case 'a':
			case 'A':
			case 'b':
			case 'B':
			case 'c':
			case 'C':
			case 'D':
			case 'd':
			case 'f':
			case 'F':
				if (do_traditional || ! inhex)
					goto done;
				/* fall through */
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			default:
			done:
				gotnumber = true;
			}
			if (gotnumber)
				break;
			c = nextc(true);
		}
		pushback();

		tokadd('\0');
		yylval = GET_INSTRUCTION(Op_push_i);

		base = 10;
		if (! do_traditional) {
			base = get_numbase(tokstart, strlen(tokstart)-1, false);
			if (do_lint) {
				if (base == 8)
					lintwarn("numeric constant `%.*s' treated as octal",
						(int) strlen(tokstart)-1, tokstart);
				else if (base == 16)
					lintwarn("numeric constant `%.*s' treated as hexadecimal",
						(int) strlen(tokstart)-1, tokstart);
			}
		}

#ifdef HAVE_MPFR
		if (do_mpfr) {
			NODE *r;

			if (! seen_point && ! seen_e) {
				r = mpg_integer();
				mpg_strtoui(r->mpg_i, tokstart, strlen(tokstart), NULL, base);
				errno = 0;
			} else {
				int tval;
				r = mpg_float();
				tval = mpfr_strtofr(r->mpg_numbr, tokstart, NULL, base, ROUND_MODE);
				errno = 0;
				IEEE_FMT(r->mpg_numbr, tval);
			}
			yylval->memory = set_profile_text(r, tokstart, strlen(tokstart)-1);
			return lasttok = YNUMBER;
		}
#endif
		if (base != 10)
			d = nondec2awknum(tokstart, strlen(tokstart)-1, NULL);
		else
			d = atof(tokstart);
		yylval->memory = set_profile_text(make_number(d), tokstart, strlen(tokstart) - 1);
		if (d <= INT32_MAX && d >= INT32_MIN && d == (int32_t) d)
			yylval->memory->flags |= NUMINT;
		return lasttok = YNUMBER;

	case '&':
		if ((c = nextc(true)) == '&') {
			yylval = GET_INSTRUCTION(Op_and);
			INSTRUCTION *new_comment = NULL;
			allow_newline(& new_comment);
			yylval->comment = new_comment;

			return lasttok = LEX_AND;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_symbol);
		return lasttok = '&';

	case '|':
		if ((c = nextc(true)) == '|') {
			yylval = GET_INSTRUCTION(Op_or);
			INSTRUCTION *new_comment = NULL;
			allow_newline(& new_comment);
			yylval->comment = new_comment;

			return lasttok = LEX_OR;
		} else if (! do_traditional && c == '&') {
			yylval = GET_INSTRUCTION(Op_symbol);
			yylval->redir_type = redirect_twoway;

			return lasttok = (in_print && in_parens == 0 ? IO_OUT : IO_IN);
		}
		pushback();
		if (in_print && in_parens == 0) {
			yylval = GET_INSTRUCTION(Op_symbol);
			yylval->redir_type = redirect_pipe;
			return lasttok = IO_OUT;
		} else {
			yylval = GET_INSTRUCTION(Op_symbol);
			yylval->redir_type = redirect_pipein;
			return lasttok = IO_IN;
		}
	}

	if (! is_letter(c)) {
		yyerror(_("invalid char '%c' in expression"), c);
		return lasttok = LEX_EOF;
	}

	/*
	 * Lots of fog here.  Consider:
	 *
	 * print "xyzzy"$_"foo"
	 *
	 * Without the check for ` lasttok != '$' ', this is parsed as
	 *
	 * print "xxyzz" $(_"foo")
	 *
	 * With the check, it is "correctly" parsed as three
	 * string concatenations.  Sigh.  This seems to be
	 * "more correct", but this is definitely one of those
	 * occasions where the interactions are funny.
	 */
	if (! do_traditional && c == '_' && lasttok != '$') {
		if ((c = nextc(true)) == '"') {
			intlstr = true;
			goto string;
		}
		pushback();
		c = '_';
	}

	/* it's some type of name-type-thing.  Find its length. */
	tok = tokstart;
	while (c != END_FILE && is_identchar(c)) {
		tokadd(c);
		c = nextc(true);

		if (! do_traditional && c == ':') {
			int peek = nextc(true);

			if (peek == ':') {	// saw identifier::
				tokadd(c);
				tokadd(c);
				c = nextc(true);
			} else
				pushback();
				// then continue around the loop, c == ':'
		}
	}
	tokadd('\0');
	pushback();

	(void) validate_qualified_name(tokstart);

	/* See if it is a special token. */
	if ((mid = check_qualified_special(tokstart)) >= 0) {
		static int warntab[sizeof(tokentab) / sizeof(tokentab[0])];
		int class = tokentab[mid].class;

		switch (class) {
		case LEX_EVAL:
		case LEX_INCLUDE:
		case LEX_LOAD:
		case LEX_NAMESPACE:
			if (lasttok != '@')
				goto out;
		default:
			break;
		}

		/* allow parameter names to shadow the names of gawk extension built-ins */
		if ((tokentab[mid].flags & GAWKX) != 0) {
			NODE *f;

			switch (want_param_names) {
			case FUNC_HEADER:
				/* in header, defining parameter names */
				goto out;
			case FUNC_BODY:
				/* in body, name must be in symbol table for it to be a parameter */
				if ((f = lookup(tokstart)) != NULL) {
					if (f->type == Node_builtin_func)
						break;
					else
						goto out;
				}
				/* else
					fall through */
			case DONT_CHECK:
				/* regular code */
				break;
			default:
				cant_happen();
				break;
			}
		}

		if (do_lint) {
			if (do_lint_extensions && (tokentab[mid].flags & GAWKX) != 0 && (warntab[mid] & GAWKX) == 0) {
				lintwarn(_("`%s' is a gawk extension"),
					tokentab[mid].operator);
				warntab[mid] |= GAWKX;
			}
			if ((tokentab[mid].flags & NOT_POSIX) != 0 && (warntab[mid] & NOT_POSIX) == 0) {
				lintwarn(_("POSIX does not allow `%s'"),
					tokentab[mid].operator);
				warntab[mid] |= NOT_POSIX;
			}
		}
		if (do_lint_old && (tokentab[mid].flags & NOT_OLD) != 0
				 && (warntab[mid] & NOT_OLD) == 0
		) {
			lintwarn(_("`%s' is not supported in old awk"),
					tokentab[mid].operator);
			warntab[mid] |= NOT_OLD;
		}

		if ((tokentab[mid].flags & BREAK) != 0)
			break_allowed++;
		if ((tokentab[mid].flags & CONTINUE) != 0)
			continue_allowed++;

		switch (class) {
		case LEX_NAMESPACE:
		case LEX_INCLUDE:
		case LEX_LOAD:
			want_source = true;
			break;
		case LEX_EVAL:
			if (in_main_context())
				goto out;
			emalloc(tokkey, char *, tok - tokstart + 1, "yylex");
			tokkey[0] = '@';
			memcpy(tokkey + 1, tokstart, tok - tokstart);
			yylval = GET_INSTRUCTION(Op_token);
			yylval->lextok = tokkey;
			break;

		case LEX_FUNCTION:
		case LEX_BEGIN:
		case LEX_END:
		case LEX_BEGINFILE:
		case LEX_ENDFILE:
			yylval = bcalloc(tokentab[mid].value, 4, sourceline);
			break;

		case LEX_FOR:
		case LEX_WHILE:
		case LEX_DO:
		case LEX_SWITCH:
			if (! do_pretty_print)
				return lasttok = class;
			/* fall through */
		case LEX_CASE:
			yylval = bcalloc(tokentab[mid].value, 2, sourceline);
			break;

		/*
		 * These must be checked here, due to the LALR nature of the parser,
		 * the rules for continue and break may not be reduced until after
		 * a token that increments the xxx_allowed varibles is seen. Bleah.
		 */
		case LEX_CONTINUE:
			if (! continue_allowed) {
				error_ln(sourceline,
					_("`continue' is not allowed outside a loop"));
				errcount++;
			}
			goto make_instruction;

		case LEX_BREAK:
			if (! break_allowed) {
				error_ln(sourceline,
					_("`break' is not allowed outside a loop or switch"));
				errcount++;
			}
			goto make_instruction;

		default:
make_instruction:
			yylval = GET_INSTRUCTION(tokentab[mid].value);
			if (class == LEX_BUILTIN || class == LEX_LENGTH)
				yylval->builtin_idx = mid;
			break;
		}
		return lasttok = class;
	}
out:
	if (want_param_names == FUNC_HEADER)
		tokkey = estrdup(tokstart, tok - tokstart - 1);
	else
		tokkey = qualify_name(tokstart, tok - tokstart - 1);

	if (*lexptr == '(') {
		yylval = bcalloc(Op_token, 2, sourceline);
		yylval->lextok = tokkey;
		return lasttok = FUNC_CALL;
	} else {
		static bool goto_warned = false;

		yylval = GET_INSTRUCTION(Op_token);
		yylval->lextok = tokkey;

#define SMART_ALECK	1
		if (SMART_ALECK
		    && do_lint
		    && ! goto_warned
		    && tolower(tokkey[0]) == 'g'
		    && strcasecmp(tokkey, "goto") == 0) {
			goto_warned = true;
			lintwarn(_("`goto' considered harmful!"));
		}
		return lasttok = NAME;
	}

#undef GET_INSTRUCTION
#undef NEWLINE_EOF
}

/* It's EBCDIC in a Bison grammar, run for the hills!

   Or, convert single-character tokens coming out of yylex() from EBCDIC to
   ASCII values on-the-fly so that the parse tables need not be regenerated
   for EBCDIC systems.  */
#ifdef USE_EBCDIC
static int
yylex(void)
{
	static char etoa_xlate[256];
	static bool do_etoa_init = true;
	int tok;

	if (do_etoa_init)
	{
		for (tok = 0; tok < 256; tok++)
			etoa_xlate[tok] = (char) tok;
#ifdef HAVE___ETOA_L
		/* IBM helpfully provides this function.  */
		__etoa_l(etoa_xlate, sizeof(etoa_xlate));
#else
# error "An EBCDIC-to-ASCII translation function is needed for this system"
#endif
		do_etoa_init = false;
	}

	tok = yylex_ebcdic();

	if (tok >= 0 && tok <= 0xFF)
		tok = etoa_xlate[tok];

	return tok;
}
#endif /* USE_EBCDIC */

/* snode --- instructions for builtin functions. Checks for arg. count
             and supplies defaults where possible. */

static INSTRUCTION *
snode(INSTRUCTION *subn, INSTRUCTION *r)
{
	INSTRUCTION *arg;
	INSTRUCTION *ip;
	NODE *n;
	int nexp = 0;
	int args_allowed;
	int idx = r->builtin_idx;

	if (subn != NULL) {
		INSTRUCTION *tp;
		for (tp = subn->nexti; tp; tp = tp->nexti) {
			tp = tp->lasti;
			nexp++;
		}
		assert(nexp > 0);
	}

	/* check against how many args. are allowed for this builtin */
	args_allowed = tokentab[idx].flags & ARGS;
	if (args_allowed && (args_allowed & A(nexp)) == 0) {
		yyerror(_("%d is invalid as number of arguments for %s"),
				nexp, tokentab[idx].operator);
		return NULL;
	}

	/* special processing for sub, gsub and gensub */

	if (tokentab[idx].value == Op_sub_builtin) {
		const char *operator = tokentab[idx].operator;

		r->sub_flags = 0;

		arg = subn->nexti;		/* first arg list */
		(void) mk_rexp(arg);

		if (strcmp(operator, "gensub") != 0) {
			/* sub and gsub */

			if (strcmp(operator, "gsub") == 0)
				r->sub_flags |= GSUB;

			arg = arg->lasti->nexti;	/* 2nd arg list */
			if (nexp == 2) {
				INSTRUCTION *expr;

				expr = list_create(instruction(Op_push_i));
				expr->nexti->memory = set_profile_text(make_number(0.0), "0", 1);
				(void) mk_expression_list(subn,
						list_append(expr, instruction(Op_field_spec)));
			}

			arg = arg->lasti->nexti; 	/* third arg list */
			ip = arg->lasti;
			if (ip->opcode == Op_push_i) {
				if (do_lint)
					lintwarn(_("%s: string literal as last argument of substitute has no effect"),
						operator);
				r->sub_flags |=	LITERAL;
			} else {
				if (make_assignable(ip) == NULL)
					yyerror(_("%s third parameter is not a changeable object"),
						operator);
				else
					ip->do_reference = true;
			}

			r->expr_count = count_expressions(&subn, false);
			ip = subn->lasti;

			(void) list_append(subn, r);

			/* add after_assign code */
			if (ip->opcode == Op_push_lhs && ip->memory->type == Node_var && ip->memory->var_assign) {
				(void) list_append(subn, instruction(Op_var_assign));
				subn->lasti->assign_ctxt = Op_sub_builtin;
				subn->lasti->assign_var = ip->memory->var_assign;
			} else if (ip->opcode == Op_field_spec_lhs) {
				(void) list_append(subn, instruction(Op_field_assign));
				subn->lasti->assign_ctxt = Op_sub_builtin;
				subn->lasti->field_assign = (Func_ptr) 0;
				ip->target_assign = subn->lasti;
			} else if (ip->opcode == Op_subscript_lhs) {
				(void) list_append(subn, instruction(Op_subscript_assign));
				subn->lasti->assign_ctxt = Op_sub_builtin;
			}

			return subn;

		} else {
			/* gensub */

			r->sub_flags |= GENSUB;
			if (nexp == 3) {
				ip = instruction(Op_push_i);
				ip->memory = set_profile_text(make_number(0.0), "0", 1);
				(void) mk_expression_list(subn,
						list_append(list_create(ip), instruction(Op_field_spec)));
			}

			r->expr_count = count_expressions(&subn, false);
			return list_append(subn, r);
		}
	}

#ifdef HAVE_MPFR
	/* N.B.: If necessary, add special processing for alternate builtin, below */
	if (do_mpfr && tokentab[idx].ptr2)
		r->builtin =  tokentab[idx].ptr2;
	else
#endif
		r->builtin = tokentab[idx].ptr;

	/* special case processing for a few builtins */

	if (r->builtin == do_length) {
		if (nexp == 0) {
		    /* no args. Use $0 */

			INSTRUCTION *list;
			r->expr_count = 1;
			list = list_create(r);
			(void) list_prepend(list, instruction(Op_field_spec));
			(void) list_prepend(list, instruction(Op_push_i));
			list->nexti->memory = set_profile_text(make_number(0.0), "0", 1);
			return list;
		} else {
			arg = subn->nexti;
			if (arg->nexti == arg->lasti && arg->nexti->opcode == Op_push)
				arg->nexti->opcode = Op_push_arg;	/* argument may be array */
 		}
	} else if (r->builtin == do_isarray) {
		arg = subn->nexti;
		if (arg->nexti == arg->lasti && arg->nexti->opcode == Op_push)
			arg->nexti->opcode = Op_push_arg_untyped;	/* argument may be untyped */
	} else if (r->builtin == do_typeof) {
		arg = subn->nexti;
		if (arg->nexti == arg->lasti && arg->nexti->opcode == Op_push)
			arg->nexti->opcode = Op_push_arg_untyped;	/* argument may be untyped */
		if (nexp == 2) {	/* 2nd argument there */
			arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
			ip = arg->lasti;
			if (ip->opcode == Op_push)
				ip->opcode = Op_push_array;
		}
#ifdef SUPPLY_INTDIV
	} else if (r->builtin == do_intdiv
#ifdef HAVE_MPFR
		   || r->builtin == MPF(intdiv)
#endif
			) {
		arg = subn->nexti->lasti->nexti->lasti->nexti;	/* 3rd arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
#endif /* SUPPLY_INTDIV */
	} else if (r->builtin == do_match) {
		static bool warned = false;

		arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
		(void) mk_rexp(arg);

		if (nexp == 3) {	/* 3rd argument there */
			if (do_lint_extensions && ! warned) {
				warned = true;
				lintwarn(_("match: third argument is a gawk extension"));
			}
			if (do_traditional) {
				yyerror(_("match: third argument is a gawk extension"));
				return NULL;
			}

			arg = arg->lasti->nexti; 	/* third arg list */
			ip = arg->lasti;
			if (/*ip == arg->nexti  && */ ip->opcode == Op_push)
				ip->opcode = Op_push_array;
		}
	} else if (r->builtin == do_split) {
		arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
		if (nexp == 2) {
			INSTRUCTION *expr;
			expr = list_create(instruction(Op_push));
			expr->nexti->memory = FS_node;
			(void) mk_expression_list(subn, expr);
		}
		arg = arg->lasti->nexti;
		n = mk_rexp(arg);
		if (nexp == 2)
			n->re_flags |= FS_DFLT;
		if (nexp == 4) {
			arg = arg->lasti->nexti;
			ip = arg->lasti;
			if (ip->opcode == Op_push)
				ip->opcode = Op_push_array;
		}
	} else if (r->builtin == do_patsplit) {
		arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
		if (nexp == 2) {
			INSTRUCTION *expr;
			expr = list_create(instruction(Op_push));
			expr->nexti->memory = FPAT_node;
			(void) mk_expression_list(subn, expr);
		}
		arg = arg->lasti->nexti;
		n = mk_rexp(arg);
		if (nexp == 4) {
			arg = arg->lasti->nexti;
			ip = arg->lasti;
			if (ip->opcode == Op_push)
				ip->opcode = Op_push_array;
		}
	} else if (r->builtin == do_close) {
		static bool warned = false;
		if (nexp == 2) {
			if (do_lint_extensions && ! warned) {
				warned = true;
				lintwarn(_("close: second argument is a gawk extension"));
			}
			if (do_traditional) {
				yyerror(_("close: second argument is a gawk extension"));
				return NULL;
			}
		}
	} else if (do_intl					/* --gen-po */
			&& r->builtin == do_dcgettext		/* dcgettext(...) */
			&& subn->nexti->lasti->opcode == Op_push_i	/* 1st arg is constant */
			&& (subn->nexti->lasti->memory->flags & STRING) != 0) {	/* it's a string constant */
		/* ala xgettext, dcgettext("some string" ...) dumps the string */
		NODE *str = subn->nexti->lasti->memory;

		if ((str->flags & INTLSTR) != 0)
			warning(_("use of dcgettext(_\"...\") is incorrect: remove leading underscore"));
			/* don't dump it, the lexer already did */
		else
			dumpintlstr(str->stptr, str->stlen);
	} else if (do_intl					/* --gen-po */
			&& r->builtin == do_dcngettext		/* dcngettext(...) */
			&& subn->nexti->lasti->opcode == Op_push_i	/* 1st arg is constant */
			&& (subn->nexti->lasti->memory->flags & STRING) != 0	/* it's a string constant */
			&& subn->nexti->lasti->nexti->lasti->opcode == Op_push_i	/* 2nd arg is constant too */
			&& (subn->nexti->lasti->nexti->lasti->memory->flags & STRING) != 0) {	/* it's a string constant */
		/* ala xgettext, dcngettext("some string", "some plural" ...) dumps the string */
		NODE *str1 = subn->nexti->lasti->memory;
		NODE *str2 = subn->nexti->lasti->nexti->lasti->memory;

		if (((str1->flags | str2->flags) & INTLSTR) != 0)
			warning(_("use of dcngettext(_\"...\") is incorrect: remove leading underscore"));
		else
			dumpintlstr2(str1->stptr, str1->stlen, str2->stptr, str2->stlen);
	} else if (r->builtin == do_asort || r->builtin == do_asorti) {
		arg = subn->nexti;	/* 1st arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
		if (nexp >= 2) {
			arg = ip->nexti;
			ip = arg->lasti;
			if (ip->opcode == Op_push)
				ip->opcode = Op_push_array;
		}
	}
	else if (r->builtin == do_index) {
		arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_match_rec || ip->opcode == Op_push_re)
			fatal(_("index: regexp constant as second argument is not allowed"));
	}
#ifdef ARRAYDEBUG
	else if (r->builtin == do_adump) {
		ip = subn->nexti->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
	}
#endif

	if (subn != NULL) {
		r->expr_count = count_expressions(&subn, false);
		return list_append(subn, r);
	}

	r->expr_count = 0;
	return list_create(r);
}


/* parms_shadow --- check if parameters shadow globals */

static int
parms_shadow(INSTRUCTION *pc, bool *shadow)
{
	int pcount, i;
	bool ret = false;
	NODE *func, *fp;
	char *fname;

	func = pc->func_body;
	fname = func->vname;
	fp = func->fparms;

#if 0	/* can't happen, already exited if error ? */
	if (fname == NULL || func == NULL)	/* error earlier */
		return false;
#endif

	pcount = func->param_cnt;

	if (pcount == 0)		/* no args, no problem */
		return 0;

	source = pc->source_file;
	sourceline = pc->source_line;
	/*
	 * Use warning() and not lintwarn() so that can warn
	 * about all shadowed parameters.
	 */
	for (i = 0; i < pcount; i++) {
		if (lookup(fp[i].param) != NULL) {
			warning(
	_("function `%s': parameter `%s' shadows global variable"),
					fname, fp[i].param);
			ret = true;
		}
	}

	*shadow |= ret;
	return 0;
}

/* valinfo --- dump var info */

void
valinfo(NODE *n, Func_print print_func, FILE *fp)
{
	if (n == Nnull_string)
		print_func(fp, "uninitialized scalar\n");
	else if ((n->flags & REGEX) != 0)
		print_func(fp, "@/%.*s/\n", n->stlen, n->stptr);
	else if ((n->flags & STRING) != 0) {
		pp_string_fp(print_func, fp, n->stptr, n->stlen, '"', false);
		print_func(fp, "\n");
	} else if ((n->flags & NUMBER) != 0) {
#ifdef HAVE_MPFR
		if (is_mpg_float(n))
			print_func(fp, "%s\n", mpg_fmt("%.17R*g", ROUND_MODE, n->mpg_numbr));
		else if (is_mpg_integer(n))
			print_func(fp, "%s\n", mpg_fmt("%Zd", n->mpg_i));
		else
#endif
		print_func(fp, "%.17g\n", n->numbr);
	} else
		print_func(fp, "?? flags %s\n", flags2str(n->flags));
}


/* dump_vars --- dump the symbol table */

void
dump_vars(const char *fname)
{
	FILE *fp;
	NODE **vars;

	if (fname == NULL)
		fp = stderr;
	else if (strcmp(fname, "-") == 0)
		fp = stdout;
	else if ((fp = fopen(fname, "w")) == NULL) {
		warning(_("could not open `%s' for writing: %s"), fname, strerror(errno));
		warning(_("sending variable list to standard error"));
		fp = stderr;
	}

	vars = variable_list();
	print_vars(vars, fprintf, fp);
	efree(vars);
	if (fp != stdout && fp != stderr && fclose(fp) != 0)
		warning(_("%s: close failed: %s"), fname, strerror(errno));
}

/* dump_funcs --- print all functions */

void
dump_funcs()
{
	NODE **funcs;
	funcs = function_list(true);
	(void) foreach_func(funcs, (int (*)(INSTRUCTION *, void *)) pp_func, (void *) 0);
	efree(funcs);
}


/* shadow_funcs --- check all functions for parameters that shadow globals */

void
shadow_funcs()
{
	static int calls = 0;
	bool shadow = false;
	NODE **funcs;

	if (calls++ != 0)
		fatal(_("shadow_funcs() called twice!"));

	funcs = function_list(true);
	(void) foreach_func(funcs, (int (*)(INSTRUCTION *, void *)) parms_shadow, & shadow);
	efree(funcs);

	/* End with fatal if the user requested it.  */
	if (shadow && lintfunc == r_fatal)
		lintwarn(_("there were shadowed variables."));
}


/* mk_function --- finalize function definition node; remove parameters
 *	out of the symbol table.
 */

static INSTRUCTION *
mk_function(INSTRUCTION *fi, INSTRUCTION *def)
{
	NODE *thisfunc;

	thisfunc = fi->func_body;
	assert(thisfunc != NULL);

	/* add any pre-function comment to start of action for profile.c  */

	if (interblock_comment != NULL) {
		interblock_comment->source_line = 0;
		merge_comments(interblock_comment, fi->comment);
		fi->comment = interblock_comment;
		interblock_comment = NULL;
	}

	/*
	 * Add an implicit return at end;
	 * also used by 'return' command in debugger
	 */

	(void) list_append(def, instruction(Op_push_i));
	def->lasti->memory = dupnode(Nnull_string);
	(void) list_append(def, instruction(Op_K_return));

	if (trailing_comment != NULL) {
		(void) list_append(def, trailing_comment);
		trailing_comment = NULL;
	}

	if (do_pretty_print) {
		fi[3].nexti = namespace_chain;
		namespace_chain = NULL;
		(void) list_prepend(def, instruction(Op_exec_count));
	}

	/* fi->opcode = Op_func */
	(fi + 1)->firsti = def->nexti;
	(fi + 1)->lasti = def->lasti;
	(fi + 2)->first_line = fi->source_line;
	(fi + 2)->last_line = lastline;
	fi->nexti = def->nexti;
	bcfree(def);

	(void) list_append(rule_list, fi + 1);	/* debugging */

	/* update lint table info */
	func_use(thisfunc->vname, FUNC_DEFINE);

	/* remove params from symbol table */
	remove_params(thisfunc);
	return fi;
}

/*
 * install_function:
 * install function name in the symbol table.
 * Extra work, build up and install a list of the parameter names.
 */

static int
install_function(char *fname, INSTRUCTION *fi, INSTRUCTION *plist)
{
	NODE *r, *f;
	int pcount = 0;

	r = lookup(fname);
	if (r != NULL) {
		error_ln(fi->source_line, _("function name `%s' previously defined"), fname);
		return -1;
	}

	if (plist != NULL)
		pcount = plist->lasti->param_count + 1;
	f = install_symbol(fname, Node_func);
	if (f->vname != fname) {
		// DON'T free fname, it's done later
		fname = f->vname;
	}

	fi->func_body = f;
	f->param_cnt = pcount;
	f->code_ptr = fi;
	f->fparms = NULL;
	if (pcount > 0) {
		char **pnames;
		pnames = check_params(fname, pcount, plist);	/* frees plist */
		f->fparms = make_params(pnames, pcount);
		efree(pnames);
		install_params(f);
	}
	return 0;
}


/* check_params --- build a list of function parameter names after
 *	making sure that the names are valid and there are no duplicates.
 */

static char **
check_params(char *fname, int pcount, INSTRUCTION *list)
{
	INSTRUCTION *p, *np;
	int i, j;
	char *name;
	char **pnames;

	assert(pcount > 0);

	emalloc(pnames, char **, pcount * sizeof(char *), "check_params");

	for (i = 0, p = list->nexti; p != NULL; i++, p = np) {
		np = p->nexti;
		name = p->lextok;
		p->lextok = NULL;

		if (strcmp(name, fname) == 0) {
			/* check for function foo(foo) { ... }.  bleah. */
			error_ln(p->source_line,
				_("function `%s': cannot use function name as parameter name"), fname);
		} else if (is_std_var(name)) {
			error_ln(p->source_line,
				_("function `%s': cannot use special variable `%s' as a function parameter"),
					fname, name);
		} else if (strchr(name, ':') != NULL)
			error_ln(p->source_line,
				_("function `%s': parameter `%s' cannot contain a namespace"),
					fname, name);

		/* check for duplicate parameters */
		for (j = 0; j < i; j++) {
			if (strcmp(name, pnames[j]) == 0) {
				error_ln(p->source_line,
					_("function `%s': parameter #%d, `%s', duplicates parameter #%d"),
					fname, i + 1, name, j + 1);
			}
		}

		pnames[i] = name;
		bcfree(p);
	}
	bcfree(list);

	return pnames;
}


#ifdef HASHSIZE
undef HASHSIZE
#endif
#define HASHSIZE 1021

static struct fdesc {
	char *name;
	short used;
	short defined;
	short extension;
	struct fdesc *next;
} *ftable[HASHSIZE];

/* func_use --- track uses and definitions of functions */

static void
func_use(const char *name, enum defref how)
{
	struct fdesc *fp;
	int len;
	int ind;

	len = strlen(name);
	ind = hash(name, len, HASHSIZE, NULL);

	for (fp = ftable[ind]; fp != NULL; fp = fp->next)
		if (strcmp(fp->name, name) == 0)
			goto update_value;

	/* not in the table, fall through to allocate a new one */

	ezalloc(fp, struct fdesc *, sizeof(struct fdesc), "func_use");
	emalloc(fp->name, char *, len + 1, "func_use");
	strcpy(fp->name, name);
	fp->next = ftable[ind];
	ftable[ind] = fp;

update_value:
	if (how == FUNC_DEFINE)
		fp->defined++;
	else if (how == FUNC_EXT) {
		fp->defined++;
		fp->extension++;
	} else
		fp->used++;
}

/* track_ext_func --- add an extension function to the table */

void
track_ext_func(const char *name)
{
	func_use(name, FUNC_EXT);
}

/* check_funcs --- verify functions that are called but not defined */

static void
check_funcs()
{
	struct fdesc *fp, *next;
	int i;

	if (! in_main_context())
		goto free_mem;

	for (i = 0; i < HASHSIZE; i++) {
		for (fp = ftable[i]; fp != NULL; fp = fp->next) {
			if (do_lint && ! fp->extension) {
				/*
				 * Making this not a lint check and
				 * incrementing * errcount breaks old code.
				 * Sigh.
				 */
				if (fp->defined == 0)
					lintwarn(_("function `%s' called but never defined"),
						fp->name);

				if (fp->used == 0)
					lintwarn(_("function `%s' defined but never called directly"),
						fp->name);
			}
		}
	}

free_mem:
	/* now let's free all the memory */
	for (i = 0; i < HASHSIZE; i++) {
		for (fp = ftable[i]; fp != NULL; fp = next) {
			next = fp->next;
			efree(fp->name);
			efree(fp);
		}
		ftable[i] = NULL;
	}
}

/* param_sanity --- look for parameters that are regexp constants */

static void
param_sanity(INSTRUCTION *arglist)
{
	INSTRUCTION *argl, *arg;
	int i = 1;

	if (arglist == NULL)
		return;
	for (argl = arglist->nexti; argl; ) {
		arg = argl->lasti;
		if (arg->opcode == Op_match_rec)
			warning_ln(arg->source_line,
				_("regexp constant for parameter #%d yields boolean value"), i);
		argl = arg->nexti;
		i++;
	}
}

/* variable --- make sure NAME is in the symbol table */

NODE *
variable(int location, char *name, NODETYPE type)
{
	NODE *r;

	if ((r = lookup(name)) != NULL) {
		if (r->type == Node_func || r->type == Node_ext_func )
			error_ln(location, _("function `%s' called with space between name and `(',\nor used as a variable or an array"),
				r->vname);
	} else {
		/* not found */
		return install_symbol(name, type);
	}
	efree(name);
	return r;
}

/* make_regnode --- make a regular expression node */

NODE *
make_regnode(int type, NODE *exp)
{
	NODE *n;

	assert(type == Node_regex || type == Node_dynregex);
	getnode(n);
	memset(n, 0, sizeof(NODE));
	n->type = type;
	n->re_cnt = 1;

	if (type == Node_regex) {
		n->re_reg[0] = make_regexp(exp->stptr, exp->stlen, false, true, false);
		if (n->re_reg[0] == NULL) {
			freenode(n);
			return NULL;
		}
		n->re_exp = exp;
		n->re_flags = CONSTANT;
	}
	return n;
}


/* mk_rexp --- make a regular expression constant */

static NODE *
mk_rexp(INSTRUCTION *list)
{
	INSTRUCTION *ip;

	ip = list->nexti;
	if (ip == list->lasti && ip->opcode == Op_match_rec)
		ip->opcode = Op_push_re;
	else if (ip == list->lasti && ip->opcode == Op_push_re)
		; /* do nothing --- @/.../ */
	else {
		ip = instruction(Op_push_re);
		ip->memory = make_regnode(Node_dynregex, NULL);
		ip->nexti = list->lasti->nexti;
		list->lasti->nexti = ip;
		list->lasti = ip;
	}
	return ip->memory;
}

#ifndef NO_LINT
/* isnoeffect --- when used as a statement, has no side effects */

static int
isnoeffect(OPCODE type)
{
	switch (type) {
	case Op_times:
	case Op_times_i:
	case Op_quotient:
	case Op_quotient_i:
	case Op_mod:
	case Op_mod_i:
	case Op_plus:
	case Op_plus_i:
	case Op_minus:
	case Op_minus_i:
	case Op_subscript:
	case Op_concat:
	case Op_exp:
	case Op_exp_i:
	case Op_unary_minus:
	case Op_field_spec:
	case Op_and_final:
	case Op_or_final:
	case Op_equal:
	case Op_notequal:
	case Op_less:
	case Op_greater:
	case Op_leq:
	case Op_geq:
	case Op_match:
	case Op_nomatch:
	case Op_match_rec:
	case Op_not:
	case Op_in_array:
		return true;
	default:
		break;	/* keeps gcc -Wall happy */
	}

	return false;
}
#endif /* NO_LINT */


/* make_assignable --- make this operand an assignable one if posiible */

static INSTRUCTION *
make_assignable(INSTRUCTION *ip)
{
	switch (ip->opcode) {
	case Op_push:
		ip->opcode = Op_push_lhs;
		return ip;
	case Op_field_spec:
		ip->opcode = Op_field_spec_lhs;
		return ip;
	case Op_subscript:
		ip->opcode = Op_subscript_lhs;
		return ip;
	default:
		break;	/* keeps gcc -Wall happy */
	}
	return NULL;
}

/* stopme --- for debugging */

NODE *
stopme(int nargs ATTRIBUTE_UNUSED)
{
	return make_number(0.0);
}

/* dumpintlstr --- write out an initial .po file entry for the string */

static void
dumpintlstr(const char *str, size_t len)
{
	char *cp;

	/* See the GNU gettext distribution for details on the file format */

	if (source != NULL) {
		/* ala the gettext sources, remove leading `./'s */
		for (cp = source; cp[0] == '.' && cp[1] == '/'; cp += 2)
			continue;
		printf("#: %s:%d\n", cp, sourceline);
	}

	printf("msgid ");
	pp_string_fp(fprintf, stdout, str, len, '"', true);
	putchar('\n');
	printf("msgstr \"\"\n\n");
	fflush(stdout);
}

/* dumpintlstr2 --- write out an initial .po file entry for the string and its plural */

static void
dumpintlstr2(const char *str1, size_t len1, const char *str2, size_t len2)
{
	char *cp;

	/* See the GNU gettext distribution for details on the file format */

	if (source != NULL) {
		/* ala the gettext sources, remove leading `./'s */
		for (cp = source; cp[0] == '.' && cp[1] == '/'; cp += 2)
			continue;
		printf("#: %s:%d\n", cp, sourceline);
	}

	printf("msgid ");
	pp_string_fp(fprintf, stdout, str1, len1, '"', true);
	putchar('\n');
	printf("msgid_plural ");
	pp_string_fp(fprintf, stdout, str2, len2, '"', true);
	putchar('\n');
	printf("msgstr[0] \"\"\nmsgstr[1] \"\"\n\n");
	fflush(stdout);
}

/* mk_binary --- instructions for binary operators */

static INSTRUCTION *
mk_binary(INSTRUCTION *s1, INSTRUCTION *s2, INSTRUCTION *op)
{
	INSTRUCTION *ip1,*ip2;
	AWKNUM res;

	ip2 = s2->nexti;
	if (s2->lasti == ip2 && ip2->opcode == Op_push_i) {
	/* do any numeric constant folding */
		ip1 = s1->nexti;
		if (do_optimize
				&& ip1 == s1->lasti && ip1->opcode == Op_push_i
				&& (ip1->memory->flags & (MPFN|MPZN|STRCUR|STRING)) == 0
				&& (ip2->memory->flags & (MPFN|MPZN|STRCUR|STRING)) == 0
		) {
			NODE *n1 = ip1->memory, *n2 = ip2->memory;
			res = force_number(n1)->numbr;
			(void) force_number(n2);
			switch (op->opcode) {
			case Op_times:
				res *= n2->numbr;
				break;
			case Op_quotient:
				if (n2->numbr == 0.0) {
					/* don't fatalize, allow parsing rest of the input */
					error_ln(op->source_line, _("division by zero attempted"));
					goto regular;
				}

				res /= n2->numbr;
				break;
			case Op_mod:
				if (n2->numbr == 0.0) {
					/* don't fatalize, allow parsing rest of the input */
					error_ln(op->source_line, _("division by zero attempted in `%%'"));
					goto regular;
				}
#ifdef HAVE_FMOD
				res = fmod(res, n2->numbr);
#else	/* ! HAVE_FMOD */
				(void) modf(res / n2->numbr, &res);
				res = n1->numbr - res * n2->numbr;
#endif	/* ! HAVE_FMOD */
				break;
			case Op_plus:
				res += n2->numbr;
				break;
			case Op_minus:
				res -= n2->numbr;
				break;
			case Op_exp:
				res = calc_exp(res, n2->numbr);
				break;
			default:
				goto regular;
			}

			op->opcode = Op_push_i;
			// We don't need to call set_profile_text() here since
			// optimizing is disabled when doing pretty printing.
			op->memory = make_number(res);
			unref(n1);
			unref(n2);
			bcfree(ip1);
			bcfree(ip2);
			bcfree(s1);
			bcfree(s2);
			return list_create(op);
		} else {
		/* do basic arithmetic optimisation */
		/* convert (Op_push_i Node_val) + (Op_plus) to (Op_plus_i Node_val) */
			switch (op->opcode) {
			case Op_times:
				op->opcode = Op_times_i;
				break;
			case Op_quotient:
				op->opcode = Op_quotient_i;
				break;
			case Op_mod:
				op->opcode = Op_mod_i;
				break;
			case Op_plus:
				op->opcode = Op_plus_i;
				break;
			case Op_minus:
				op->opcode = Op_minus_i;
				break;
			case Op_exp:
				op->opcode = Op_exp_i;
				break;
			default:
				goto regular;
			}

			op->memory = ip2->memory;
			bcfree(ip2);
			bcfree(s2);	/* Op_list */
			return list_append(s1, op);
		}
	}

regular:
	/* append lists s1, s2 and add `op' bytecode */
	(void) list_merge(s1, s2);
	return list_append(s1, op);
}

/* mk_boolean --- instructions for boolean and, or */

static INSTRUCTION *
mk_boolean(INSTRUCTION *left, INSTRUCTION *right, INSTRUCTION *op)
{
	INSTRUCTION *tp;
	OPCODE opc, final_opc;

	opc = op->opcode;		/* Op_and or Op_or */
	final_opc = (opc == Op_or) ? Op_or_final : Op_and_final;

	add_lint(right, LINT_assign_in_cond);

	tp = left->lasti;

	if (tp->opcode != final_opc) {	/* x || y */
		list_append(right, instruction(final_opc));
		add_lint(left, LINT_assign_in_cond);
		(void) list_append(left, op);
		left->lasti->target_jmp = right->lasti;

		/* NB: target_stmt points to previous Op_and(Op_or) in a chain;
		 *     target_stmt only used in the parser (see below).
		 */

		left->lasti->target_stmt = left->lasti;
		right->lasti->target_stmt = left->lasti;
	} else {		/* optimization for x || y || z || ... */
		INSTRUCTION *ip;

		op->opcode = final_opc;
		(void) list_append(right, op);
		op->target_stmt = tp;
		tp->opcode = opc;
		tp->target_jmp = op;

		/* update jump targets */
		for (ip = tp->target_stmt; ; ip = ip->target_stmt) {
			assert(ip->opcode == opc);
			assert(ip->target_jmp == tp);
			/* if (ip->opcode == opc &&  ip->target_jmp == tp) */
			ip->target_jmp = op;
			if (ip->target_stmt == ip)
				break;
		}
	}

	return list_merge(left, right);
}

/* mk_condition --- if-else and conditional */

static INSTRUCTION *
mk_condition(INSTRUCTION *cond, INSTRUCTION *ifp, INSTRUCTION *true_branch,
		INSTRUCTION *elsep, INSTRUCTION *false_branch)
{
	/*
	 *    ----------------
	 *       cond
	 *    ----------------
	 * t: [Op_jmp_false f ]
	 *    ----------------
	 *       true_branch
	 *
	 *    ----------------
	 *    [Op_jmp y]
	 *    ----------------
	 * f:
	 *      false_branch
	 *    ----------------
	 * y: [Op_no_op]
	 *    ----------------
	 */

	INSTRUCTION *ip;
	bool setup_else_part = true;

	if (false_branch == NULL) {
		false_branch = list_create(instruction(Op_no_op));
		if (elsep == NULL) {		/* else { } */
			setup_else_part = false;
		}
	} else {
		/* assert(elsep != NULL); */

		/* avoid a series of no_op's: if .. else if .. else if .. */
		if (false_branch->lasti->opcode != Op_no_op)
			(void) list_append(false_branch, instruction(Op_no_op));
	}

	if (setup_else_part) {
		if (do_pretty_print) {
			(void) list_prepend(false_branch, elsep);
			false_branch->nexti->branch_end = false_branch->lasti;
			(void) list_prepend(false_branch, instruction(Op_exec_count));
		} else
			bcfree(elsep);
	}

	(void) list_prepend(false_branch, instruction(Op_jmp));
	false_branch->nexti->target_jmp = false_branch->lasti;

	add_lint(cond, LINT_assign_in_cond);
	ip = list_append(cond, instruction(Op_jmp_false));
	ip->lasti->target_jmp = false_branch->nexti->nexti;

	if (do_pretty_print) {
		(void) list_prepend(ip, ifp);
		(void) list_append(ip, instruction(Op_exec_count));
		ip->nexti->branch_if = ip->lasti;
		ip->nexti->branch_else = false_branch->nexti;
	} else
		bcfree(ifp);

	if (true_branch != NULL)
		list_merge(ip, true_branch);
	return list_merge(ip, false_branch);
}

enum defline { FIRST_LINE, LAST_LINE };

/* find_line -- find the first(last) line in a list of (pattern) instructions */

static int
find_line(INSTRUCTION *pattern, enum defline what)
{
	INSTRUCTION *ip;
	int lineno = 0;

	for (ip = pattern->nexti; ip; ip = ip->nexti) {
		if (what == LAST_LINE) {
			if (ip->source_line > lineno)
				lineno = ip->source_line;
		} else {	/* FIRST_LINE */
			if (ip->source_line > 0
					&& (lineno == 0 || ip->source_line < lineno))
				lineno = ip->source_line;
		}
		if (ip == pattern->lasti)
			break;
	}
	assert(lineno > 0);
	return lineno;
}

/* append_rule --- pattern-action instructions */

static INSTRUCTION *
append_rule(INSTRUCTION *pattern, INSTRUCTION *action)
{
	/*
	 *    ----------------
	 *       pattern
	 *    ----------------
	 *    [Op_jmp_false f ]
	 *    ----------------
	 *       action
	 *    ----------------
	 * f: [Op_no_op       ]
	 *    ----------------
	 */

	INSTRUCTION *rp;
	INSTRUCTION *tp;
	INSTRUCTION *ip;

	if (rule != Rule) {
		rp = pattern;
		if (do_pretty_print) {
			rp[3].nexti = namespace_chain;
			namespace_chain = NULL;
			(void) list_append(action, instruction(Op_no_op));
		}
		(rp + 1)->firsti = action->nexti;
		(rp + 1)->lasti = action->lasti;
		(rp + 2)->first_line = pattern->source_line;
		(rp + 2)->last_line = lastline;
		ip = list_prepend(action, rp);
		if (interblock_comment != NULL) {
			ip = list_prepend(ip, interblock_comment);
			interblock_comment = NULL;
		}
	} else {
		rp = bcalloc(Op_rule, 4, 0);
		rp->in_rule = Rule;
		rp->source_file = source;
		tp = instruction(Op_no_op);

		if (do_pretty_print) {
			rp[3].nexti = namespace_chain;
			namespace_chain = NULL;
		}

		if (pattern == NULL) {
			/* assert(action != NULL); */
			if (do_pretty_print)
				(void) list_prepend(action, instruction(Op_exec_count));
			(rp + 1)->firsti = action->nexti;
			(rp + 1)->lasti = tp;
			(rp + 2)->first_line = firstline;
			(rp + 2)->last_line = lastline;
			rp->source_line = firstline;
			ip = list_prepend(list_append(action, tp), rp);
		} else {
			(void) list_append(pattern, instruction(Op_jmp_false));
			pattern->lasti->target_jmp = tp;
			(rp + 2)->first_line = find_line(pattern, FIRST_LINE);
			rp->source_line = (rp + 2)->first_line;
			if (action == NULL) {
				(rp + 2)->last_line = find_line(pattern, LAST_LINE);
				action = list_create(instruction(Op_K_print_rec));
				if (do_pretty_print)
					action = list_prepend(action, instruction(Op_exec_count));
			} else
				(rp + 2)->last_line = lastline;

			if (interblock_comment != NULL) {	// was after previous action
				pattern = list_prepend(pattern, interblock_comment);
				interblock_comment = NULL;
			}

			if (do_pretty_print) {
				pattern = list_prepend(pattern, instruction(Op_exec_count));
				action = list_prepend(action, instruction(Op_exec_count));
			}

 			(rp + 1)->firsti = action->nexti;
			(rp + 1)->lasti = tp;
			ip = list_append(
					list_merge(list_prepend(pattern, rp),
						action),
					tp);
		}
	}

	list_append(rule_list, rp + 1);

	if (rule_block[rule] == NULL)
		rule_block[rule] = ip;
	else
		(void) list_merge(rule_block[rule], ip);

	return rule_block[rule];
}

/* mk_assignment --- assignment bytecodes */

static INSTRUCTION *
mk_assignment(INSTRUCTION *lhs, INSTRUCTION *rhs, INSTRUCTION *op)
{
	INSTRUCTION *tp;
	INSTRUCTION *ip;

	tp = lhs->lasti;
	switch (tp->opcode) {
	case Op_field_spec:
		tp->opcode = Op_field_spec_lhs;
		break;
	case Op_subscript:
		tp->opcode = Op_subscript_lhs;
		break;
	case Op_push:
	case Op_push_array:
		tp->opcode = Op_push_lhs;
		break;
	case Op_field_assign:
		yyerror(_("cannot assign a value to the result of a field post-increment expression"));
		break;
	default:
		yyerror(_("invalid target of assignment (opcode %s)"),
				opcode2str(tp->opcode));
		break;
	}

	tp->do_reference = (op->opcode != Op_assign);	/* check for uninitialized reference */

	if (rhs != NULL)
		ip = list_merge(rhs, lhs);
	else
		ip = lhs;

	(void) list_append(ip, op);

	if (tp->opcode == Op_push_lhs
			&& tp->memory->type == Node_var
			&& tp->memory->var_assign
	) {
		tp->do_reference = false; /* no uninitialized reference checking
		                           * for a special variable.
		                           */
		(void) list_append(ip, instruction(Op_var_assign));
		ip->lasti->assign_var = tp->memory->var_assign;
	} else if (tp->opcode == Op_field_spec_lhs) {
		(void) list_append(ip, instruction(Op_field_assign));
		ip->lasti->field_assign = (Func_ptr) 0;
		tp->target_assign = ip->lasti;
	} else if (tp->opcode == Op_subscript_lhs) {
		(void) list_append(ip, instruction(Op_subscript_assign));
	}

	return ip;
}

/* optimize_assignment --- peephole optimization for assignment */

static INSTRUCTION *
optimize_assignment(INSTRUCTION *exp)
{
	INSTRUCTION *i1, *i2, *i3;

	/*
	 * Optimize assignment statements array[subs] = x; var = x; $n = x;
	 * string concatenation of the form s = s t.
	 *
	 * 1) Array element assignment array[subs] = x:
	 *   Replaces Op_push_array + Op_subscript_lhs + Op_assign + Op_pop
	 *   with single instruction Op_store_sub.
	 *	 Limitation: 1 dimension and sub is simple var/value.
	 *
	 * 2) Simple variable assignment var = x:
	 *   Replaces Op_push_lhs + Op_assign + Op_pop with Op_store_var.
	 *
	 * 3) Field assignment $n = x:
	 *   Replaces Op_field_spec_lhs + Op_assign + Op_field_assign + Op_pop
	 *   with Op_store_field.
	 *
	 * 4) Optimization for string concatenation:
	 *   For cases like x = x y, uses realloc to include y in x;
	 *   also eliminates instructions Op_push_lhs and Op_pop.
	 */

	/*
	 * N.B.: do not append Op_pop instruction to the returned
	 * instruction list if optimized. None of these
	 * optimized instructions pushes the r-value of assignment
	 * onto the runtime stack.
	 */

	i2 = NULL;
	i1 = exp->lasti;

	if (   i1->opcode != Op_assign
	    && i1->opcode != Op_field_assign)
		return list_append(exp, instruction(Op_pop));

	for (i2 = exp->nexti; i2 != i1; i2 = i2->nexti) {
		switch (i2->opcode) {
		case Op_concat:
			if (i2->nexti->opcode == Op_push_lhs    /* l.h.s is a simple variable */
				&& (i2->concat_flag & CSVAR) != 0   /* 1st exp in r.h.s is a simple variable;
				                                     * see Op_concat in the grammer above.
				                                     */
				&& i2->nexti->memory == exp->nexti->memory	 /* and the same as in l.h.s */
				&& i2->nexti->nexti == i1
				&& i1->opcode == Op_assign
			) {
				/* s = s ... optimization */

				/* avoid stuff like x = x (x = y) or x = x gsub(/./, "b", x);
				 * check for l-value reference to this variable in the r.h.s.
				 * Also, avoid function calls in general to guard against
				 * global variable assignment.
				 */

				for (i3 = exp->nexti->nexti; i3 != i2; i3 = i3->nexti) {
					if ((i3->opcode == Op_push_lhs && i3->memory == i2->nexti->memory)
							|| i3->opcode == Op_func_call)
						return list_append(exp, instruction(Op_pop)); /* no optimization */
				}

				/* remove the variable from r.h.s */
				i3 = exp->nexti;
				exp->nexti = i3->nexti;
				bcfree(i3);

				if (--i2->expr_count == 1)	/* one less expression in Op_concat */
					i2->opcode = Op_no_op;

				i3 = i2->nexti;
				assert(i3->opcode == Op_push_lhs);
				i3->opcode = Op_assign_concat;	/* change Op_push_lhs to Op_assign_concat */
				i3->nexti = NULL;
				bcfree(i1);          /* Op_assign */
				exp->lasti = i3;     /* update Op_list */
				return exp;
			}
			break;

		case Op_field_spec_lhs:
			if (i2->nexti->opcode == Op_assign
					&& i2->nexti->nexti == i1
					&& i1->opcode == Op_field_assign
			) {
				/* $n = .. */
				i2->opcode = Op_store_field;
				bcfree(i2->nexti);  /* Op_assign */
				i2->nexti = NULL;
				bcfree(i1);          /* Op_field_assign */
				exp->lasti = i2;    /* update Op_list */
				return exp;
			}
			break;

		case Op_push_array:
			if (i2->nexti->nexti->opcode == Op_subscript_lhs) {
				i3 = i2->nexti->nexti;
				if (i3->sub_count == 1
						&& i3->nexti == i1
						&& i1->opcode == Op_assign
				) {
					/* array[sub] = .. */
					i3->opcode = Op_store_sub;
					i3->memory = i2->memory;
					i3->expr_count = 1;  /* sub_count shadows memory,
                                          * so use expr_count instead.
				                          */
					i3->nexti = NULL;
					i2->opcode = Op_no_op;
					bcfree(i1);          /* Op_assign */
					exp->lasti = i3;     /* update Op_list */
					return exp;
				}
			}
			break;

		case Op_push_lhs:
			if (i2->nexti == i1
					&& i1->opcode == Op_assign
			) {
				/* var = .. */
				i2->opcode = Op_store_var;
				i2->nexti = NULL;
				bcfree(i1);          /* Op_assign */
				exp->lasti = i2;     /* update Op_list */

				i3 = exp->nexti;
				if (i3->opcode == Op_push_i
					&& (i3->memory->flags & INTLSTR) == 0
					&& i3->nexti == i2
				) {
					/* constant initializer */
					i2->initval = i3->memory;
					bcfree(i3);
					exp->nexti = i2;
				} else
					i2->initval = NULL;

				return exp;
			}
			break;

		default:
			break;
		}
	}

	/* no optimization  */
	return list_append(exp, instruction(Op_pop));
}


/* mk_getline --- make instructions for getline */

static INSTRUCTION *
mk_getline(INSTRUCTION *op, INSTRUCTION *var, INSTRUCTION *redir, int redirtype)
{
	INSTRUCTION *ip;
	INSTRUCTION *tp;
	INSTRUCTION *asgn = NULL;

	/*
	 *  getline [var] < [file]
	 *
	 *  [ file (simp_exp)]
	 *  [ [ var ] ]
	 *  [ Op_K_getline_redir|NULL|redir_type|into_var]
	 *  [ [var_assign] ]
	 *
	 */

	if (redir == NULL) {
		int sline = op->source_line;
		bcfree(op);
		op = bcalloc(Op_K_getline, 2, sline);
		(op + 1)->target_endfile = ip_endfile;
		(op + 1)->target_beginfile = ip_beginfile;
	}

	if (var != NULL) {
		tp = make_assignable(var->lasti);
		assert(tp != NULL);

		/* check if we need after_assign bytecode */
		if (tp->opcode == Op_push_lhs
				&& tp->memory->type == Node_var
				&& tp->memory->var_assign
		) {
			asgn = instruction(Op_var_assign);
			asgn->assign_ctxt = op->opcode;
			asgn->assign_var = tp->memory->var_assign;
		} else if (tp->opcode == Op_field_spec_lhs) {
			asgn = instruction(Op_field_assign);
			asgn->assign_ctxt = op->opcode;
			asgn->field_assign = (Func_ptr) 0;   /* determined at run time */
			tp->target_assign = asgn;
		} else if (tp->opcode == Op_subscript_lhs) {
			asgn = instruction(Op_subscript_assign);
			asgn->assign_ctxt = op->opcode;
		}

		if (redir != NULL) {
			ip = list_merge(redir, var);
			(void) list_append(ip, op);
		} else
			ip = list_append(var, op);
	} else if (redir != NULL)
		ip = list_append(redir, op);
	else
		ip = list_create(op);
	op->into_var = (var != NULL);
	op->redir_type = (redir != NULL) ? redirtype : redirect_none;

	return (asgn == NULL ? ip : list_append(ip, asgn));
}


/* mk_for_loop --- for loop bytecodes */

static INSTRUCTION *
mk_for_loop(INSTRUCTION *forp, INSTRUCTION *init, INSTRUCTION *cond,
				INSTRUCTION *incr, INSTRUCTION *body)
{
	/*
	 *   ------------------------
	 *        init                 (may be NULL)
	 *   ------------------------
	 * x:
	 *        cond                 (Op_no_op if NULL)
	 *   ------------------------
	 *    [ Op_jmp_false tb      ]
	 *   ------------------------
	 *        body                 (may be NULL)
	 *   ------------------------
	 * tc:
	 *    incr                      (may be NULL)
	 *    [ Op_jmp x             ]
	 *   ------------------------
	 * tb:[ Op_no_op             ]
	 */

	INSTRUCTION *ip, *tbreak, *tcont;
	INSTRUCTION *jmp;
	INSTRUCTION *pp_cond;
	INSTRUCTION *ret;

	tbreak = instruction(Op_no_op);

	if (cond != NULL) {
		add_lint(cond, LINT_assign_in_cond);
		pp_cond = cond->nexti;
		ip = cond;
		(void) list_append(ip, instruction(Op_jmp_false));
		ip->lasti->target_jmp = tbreak;
	} else {
		pp_cond = instruction(Op_no_op);
		ip = list_create(pp_cond);
	}

	if (init != NULL)
		ip = list_merge(init, ip);

	if (do_pretty_print) {
		(void) list_append(ip, instruction(Op_exec_count));
		(forp + 1)->forloop_cond = pp_cond;
		(forp + 1)->forloop_body = ip->lasti;
	}

	if (body != NULL)
		(void) list_merge(ip, body);

	jmp = instruction(Op_jmp);
	jmp->target_jmp = pp_cond;
	if (incr == NULL)
		tcont = jmp;
	else {
		tcont = incr->nexti;
		(void) list_merge(ip, incr);
	}

	(void) list_append(ip, jmp);
	ret = list_append(ip, tbreak);
	fix_break_continue(ret, tbreak, tcont);

	if (do_pretty_print) {
		forp->target_break = tbreak;
		forp->target_continue = tcont;
		ret = list_prepend(ret, forp);
	}
	/* else
		forp is NULL */

	return ret;
}

/* add_lint --- add lint warning bytecode if needed */

static void
add_lint(INSTRUCTION *list, LINTTYPE linttype)
{
#ifndef NO_LINT
	INSTRUCTION *ip;

	switch (linttype) {
	case LINT_assign_in_cond:
		ip = list->lasti;
		if (ip->opcode == Op_var_assign || ip->opcode == Op_field_assign) {
			assert(ip != list->nexti);
			for (ip = list->nexti; ip->nexti != list->lasti; ip = ip->nexti)
				;
		}

		if (ip->opcode == Op_assign || ip->opcode == Op_assign_concat) {
			list_append(list, instruction(Op_lint));
			list->lasti->lint_type = linttype;
		}
		break;

	case LINT_no_effect:
		if (list->lasti->opcode == Op_pop && list->nexti != list->lasti) {
			int line = 0;

			// Get down to the last instruction (FIXME: why?)
			for (ip = list->nexti; ip->nexti != list->lasti; ip = ip->nexti) {
				// along the way track line numbers, we will use the line
				// closest to the opcode if that opcode doesn't have one
				if (ip->source_line != 0)
					line = ip->source_line;
			}

			if (do_lint) {		/* compile-time warning */
				if (isnoeffect(ip->opcode)) {
					if (ip->source_line != 0)
						line = ip->source_line;
					lintwarn_ln(line, ("statement may have no effect"));
				}
			}

			if (ip->opcode == Op_push || ip->opcode == Op_push_i) {		/* run-time warning */
				list_append(list, instruction(Op_lint));
				list->lasti->lint_type = linttype;
			}
		}
		break;

	default:
		break;
	}
#endif
}

/* mk_expression_list --- list of bytecode lists */

static INSTRUCTION *
mk_expression_list(INSTRUCTION *list, INSTRUCTION *s1)
{
	INSTRUCTION *r;

	/* we can't just combine all bytecodes, since we need to
	 * process individual expressions for a few builtins in snode() (-:
	 */

	/* -- list of lists     */
	/* [Op_list| ... ]------
	 *                       |
	 * [Op_list| ... ]   --  |
	 *  ...               |  |
	 *  ...       <-------   |
	 * [Op_list| ... ]   --  |
	 *  ...               |  |
	 *  ...               |  |
	 *  ...       <------- --
	 */

	assert(s1 != NULL && s1->opcode == Op_list);
	if (list == NULL) {
		list = instruction(Op_list);
		list->nexti = s1;
		list->lasti = s1->lasti;
		return list;
	}

	/* append expression to the end of the list */

	r = list->lasti;
	r->nexti = s1;
	list->lasti = s1->lasti;
	return list;
}

/* count_expressions --- fixup expression_list from mk_expression_list.
 *                       returns no of expressions in list. isarg is true
 *                       for function arguments.
 */

static int
count_expressions(INSTRUCTION **list, bool isarg)
{
	INSTRUCTION *expr;
	INSTRUCTION *r = NULL;
	int count = 0;

	if (*list == NULL)	/* error earlier */
		return 0;

	for (expr = (*list)->nexti; expr; ) {
		INSTRUCTION *t1, *t2;
		t1 = expr->nexti;
		t2 = expr->lasti;
		if (isarg && t1 == t2 && t1->opcode == Op_push)
			t1->opcode = Op_push_param;
		if (++count == 1)
			r = expr;
		else
			(void) list_merge(r, expr);
		expr = t2->nexti;
	}

	assert(count > 0);
	if (! isarg && count > max_args)
		max_args = count;
	bcfree(*list);
	*list = r;
	return count;
}

/* fix_break_continue --- fix up break & continue codes in loop bodies */

static void
fix_break_continue(INSTRUCTION *list, INSTRUCTION *b_target, INSTRUCTION *c_target)
{
	INSTRUCTION *ip;

	list->lasti->nexti = NULL;	/* just to make sure */

	for (ip = list->nexti; ip != NULL; ip = ip->nexti) {
		switch (ip->opcode) {
		case Op_K_break:
			if (ip->target_jmp == NULL)
				ip->target_jmp = b_target;
			break;

		case Op_K_continue:
			if (ip->target_jmp == NULL)
				ip->target_jmp = c_target;
			break;

		default:
			/* this is to keep the compiler happy. sheesh. */
			break;
		}
	}
}

static inline INSTRUCTION *
list_create(INSTRUCTION *x)
{
	INSTRUCTION *l;

	l = instruction(Op_list);
	l->nexti = x;
	l->lasti = x;
	return l;
}

static inline INSTRUCTION *
list_append(INSTRUCTION *l, INSTRUCTION *x)
{
#ifdef GAWKDEBUG
	if (l->opcode != Op_list)
		cant_happen();
#endif
	l->lasti->nexti = x;
	l->lasti = x;
	return l;
}

static inline INSTRUCTION *
list_prepend(INSTRUCTION *l, INSTRUCTION *x)
{
#ifdef GAWKDEBUG
	if (l->opcode != Op_list)
		cant_happen();
#endif
	x->nexti = l->nexti;
	l->nexti = x;
	return l;
}

static inline INSTRUCTION *
list_merge(INSTRUCTION *l1, INSTRUCTION *l2)
{
#ifdef GAWKDEBUG
	if (l1->opcode != Op_list)
		cant_happen();
	if (l2->opcode != Op_list)
		cant_happen();
#endif
	l1->lasti->nexti = l2->nexti;
	l1->lasti = l2->lasti;
	bcfree(l2);
	return l1;
}

/* See if name is a special token. */

int
check_special(const char *name)
{
	int low, high, mid;
	int i;
	int non_standard_flags = 0;
#ifdef USE_EBCDIC
	static bool did_sort = false;

	if (! did_sort) {
		qsort((void *) tokentab,
				sizeof(tokentab) / sizeof(tokentab[0]),
				sizeof(tokentab[0]), tokcompare);
		did_sort = true;
	}
#endif

	if (do_traditional)
		non_standard_flags |= GAWKX;
	if (do_posix)
		non_standard_flags |= NOT_POSIX;

	low = 0;
	high = (sizeof(tokentab) / sizeof(tokentab[0])) - 1;
	while (low <= high) {
		mid = (low + high) / 2;
		i = *name - tokentab[mid].operator[0];
		if (i == 0)
			i = strcmp(name, tokentab[mid].operator);

		if (i < 0)		/* token < mid */
			high = mid - 1;
		else if (i > 0)		/* token > mid */
			low = mid + 1;
		else {
			if ((tokentab[mid].flags & non_standard_flags) != 0)
				return -1;
			return mid;
		}
	}
	return -1;
}

/*
 * This provides a private version of functions that act like VMS's
 * variable-length record filesystem, where there was a bug on
 * certain source files.
 */

static FILE *fp = NULL;

/* read_one_line --- return one input line at a time. mainly for debugging. */

static ssize_t
read_one_line(int fd, void *buffer, size_t count)
{
	char buf[BUFSIZ];

	/* Minor potential memory leak here. Too bad. */
	if (fp == NULL) {
		fp = fdopen(fd, "r");
		if (fp == NULL) {
			fprintf(stderr, "ugh. fdopen: %s\n", strerror(errno));
			gawk_exit(EXIT_FAILURE);
		}
	}

	if (fgets(buf, sizeof buf, fp) == NULL)
		return 0;

	memcpy(buffer, buf, strlen(buf));
	return strlen(buf);
}

/* one_line_close --- close the open file being read with read_one_line() */

static int
one_line_close(int fd)
{
	int ret;

	if (fp == NULL || fd != fileno(fp))
		fatal("debugging read/close screwed up!");

	ret = fclose(fp);
	fp = NULL;
	return ret;
}


/* lookup_builtin --- find a builtin function or return NULL */

builtin_func_t
lookup_builtin(const char *name)
{
	if (strncmp(name, "awk::", 5) == 0)
		name += 5;

	int mid = check_special(name);

	if (mid == -1)
		return NULL;

	switch (tokentab[mid].class) {
	case LEX_BUILTIN:
	case LEX_LENGTH:
		break;
	default:
		return NULL;
	}

	/* And another special case... */
	if (tokentab[mid].value == Op_sub_builtin)
		return (builtin_func_t) do_sub;

#ifdef HAVE_MPFR
	if (do_mpfr)
		return tokentab[mid].ptr2;
#endif

	return tokentab[mid].ptr;
}

/* install_builtins --- add built-in functions to FUNCTAB */

void
install_builtins(void)
{
	int i, j;
	int flags_that_must_be_clear = DEBUG_USE;

	if (do_traditional)
		flags_that_must_be_clear |= GAWKX;

	if (do_posix)
		flags_that_must_be_clear |= NOT_POSIX;


	j = sizeof(tokentab) / sizeof(tokentab[0]);
	for (i = 0; i < j; i++) {
		if (   (tokentab[i].class == LEX_BUILTIN
		        || tokentab[i].class == LEX_LENGTH)
		    && (tokentab[i].flags & flags_that_must_be_clear) == 0) {
			(void) install_symbol(tokentab[i].operator, Node_builtin_func);
		}
	}
}

/*
 * 9/2014: Gawk cannot use <ctype.h> isalpha or isalnum when
 * parsing the program since that can let through non-English
 * letters.  So, we supply our own. !@#$%^&*()-ing locales!
 */

/* is_alpha --- return true if c is an English letter */

/*
 * The scene of the murder was grisly to look upon.  When the inspector
 * arrived, the sergeant turned to him and said, "Another programmer stabbed
 * in the back. He never knew what happened."
 *
 * The inspector replied, "Looks like the MO of isalpha, and his even meaner
 * big brother, isalnum. The Locale brothers."  The sergeant merely
 * shuddered in horror.
 */

bool
is_alpha(int c)
{
	switch (c) {
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
		return true;
	}
	return false;
}

/* is_alnum --- return true for alphanumeric, English only letters */

bool
is_alnum(int c)
{
	/* digit test is good for EBCDIC too. so there. */
	return (is_alpha(c) || ('0' <= c && c <= '9'));
}


/*
 * is_letter --- function to check letters
 *	isalpha() isn't good enough since it can look at the locale.
 * Underscore counts as a letter in awk identifiers
 */

bool
is_letter(int c)
{
	return (is_alpha(c) || c == '_');
}

/* is_identchar --- return true if c can be in an identifier */

bool
is_identchar(int c)
{
	return (is_alnum(c) || c == '_');
}

/* set_profile_text --- make a number that can be printed when profiling */

static NODE *
set_profile_text(NODE *n, const char *str, size_t len)
{
	if (do_pretty_print) {
		// two extra bytes: one for NUL termination, and another in
		// case we need to add a leading minus sign in add_sign_to_num
		emalloc(n->stptr, char *, len + 2, "set_profile_text");
		memcpy(n->stptr, str, len);
		n->stptr[len] = '\0';
		n->stlen = len;
		// Set STRCUR and n->stfmt for use when profiling
		// (i.e., actually running the program) so that
		// force_string() on this item will work ok.
		// Thanks and a tip of the hatlo to valgrind.
		n->flags |= (NUMCONSTSTR|STRCUR);
		n->stfmt = STFMT_UNUSED;
#ifdef HAVE_MPFR
		n->strndmode = MPFR_round_mode;
#endif
	}

	return n;
}

/*
 * merge_comments --- merge c2 into c1 and free c2 if successful.
 *	Allow c2 to be NULL, in which case just merged chained
 *	comments in c1.
 */

static void
merge_comments(INSTRUCTION *c1, INSTRUCTION *c2)
{
	assert(c1->opcode == Op_comment);

	if (c1->comment == NULL && c2 == NULL)	// nothing to do
		return;

	size_t total = c1->memory->stlen;
	if (c1->comment != NULL)
		total += 1 /* \n */ + c1->comment->memory->stlen;

	if (c2 != NULL) {
		assert(c2->opcode == Op_comment);
		total += 1 /* \n */ + c2->memory->stlen;
		if (c2->comment != NULL)
			total += c2->comment->memory->stlen + 1;
	}

	char *buffer;
	emalloc(buffer, char *, total + 1, "merge_comments");

	strcpy(buffer, c1->memory->stptr);
	if (c1->comment != NULL) {
		strcat(buffer, "\n");
		strcat(buffer, c1->comment->memory->stptr);
	}

	if (c2 != NULL) {
		strcat(buffer, "\n");
		strcat(buffer, c2->memory->stptr);
		if (c2->comment != NULL) {
			strcat(buffer, "\n");
			strcat(buffer, c2->comment->memory->stptr);
		}

		unref(c2->memory);
		if (c2->comment != NULL) {
			unref(c2->comment->memory);
			bcfree(c2->comment);
			c2->comment = NULL;
		}
		bcfree(c2);
	}

	c1->memory->comment_type = BLOCK_COMMENT;
	free(c1->memory->stptr);
	c1->memory->stptr = buffer;
	c1->memory->stlen = strlen(buffer);

	// now free everything else
	if (c1->comment != NULL) {
		unref(c1->comment->memory);
		bcfree(c1->comment);
		c1->comment = NULL;
	}
}

/* make_braced_statements --- handle `l_brace statements r_brace' with comments */

static INSTRUCTION *
make_braced_statements(INSTRUCTION *lbrace, INSTRUCTION *stmts, INSTRUCTION *rbrace)
{
	INSTRUCTION *ip;

	if (stmts == NULL)
		ip = list_create(instruction(Op_no_op));
	else
		ip = stmts;

	if (lbrace != NULL) {
		INSTRUCTION *comment2 = lbrace->comment;
		if (comment2 != NULL) {
			ip = list_prepend(ip, comment2);
			lbrace->comment = NULL;
		}
		ip = list_prepend(ip, lbrace);
	}

	return ip;
}

/* validate_qualified_name --- make sure that a qualified name is built correctly */

/*
 * This routine returns upon first error, no need to produce multiple, possibly
 * conflicting / confusing error messages.
 */

bool
validate_qualified_name(char *token)
{
	char *cp, *cp2;

	// no colon, by definition it's well formed
	if ((cp = strchr(token, ':')) == NULL)
		return true;

	if (do_traditional || do_posix) {
		error_ln(sourceline, _("identifier %s: qualified names not allowed in traditional / POSIX mode"), token);
		return false;
	}

	if (cp[1] != ':') {	// could happen from command line
		error_ln(sourceline, _("identifier %s: namespace separator is two colons, not one"), token);
		return false;
	}

	if (! is_letter(cp[2])) {
		error_ln(sourceline,
				_("qualified identifier `%s' is badly formed"),
				token);
		return false;
	}

	if ((cp2 = strchr(cp+2, ':')) != NULL) {
		error_ln(sourceline,
			_("identifier `%s': namespace separator can only appear once in a qualified name"),
			token);
		return false;
	}

	return true;
}

/* check_qualified_special --- decide if a name is special or not */

static int
check_qualified_special(char *token)
{
	char *cp;

	if ((cp = strchr(token, ':')) == NULL && current_namespace == awk_namespace)
		return check_special(token);

	/*
	 * Now it's more complicated.  Here are the rules.
	 *
	 * 1. Namespace name cannot be a standard awk reserved word or function.
	 * 2. Subordinate part of the name cannot be standard awk reserved word or function.
	 * 3. If namespace part is explicitly "awk", return result of check_special().
	 * 4. Else return -1 (gawk extensions allowed, we check standard awk in step 2).
	 */

	const struct token *tok;
	int i;
	if (cp == NULL) {	// namespace not awk, but a simple identifier
		i = check_special(token);
		if (i < 0)
			return i;

		tok = & tokentab[i];
		if ((tok->flags & GAWKX) != 0 && tok->class == LEX_BUILTIN)
			return -1;
		else
			return i;
	}

	char *ns, *end, *subname;
	ns = token;
	*(end = cp) = '\0';	// temporarily turn it into standalone string
	subname = end + 2;

	// First check the namespace part
	i = check_special(ns);
	if (i >= 0 && (tokentab[i].flags & GAWKX) == 0) {
		error_ln(sourceline, _("using reserved identifier `%s' as a namespace is not allowed"), ns);
		goto done;
	}

	// Now check the subordinate part
	i = check_special(subname);
	if (i >= 0 && (tokentab[i].flags & GAWKX) == 0 && strcmp(ns, "awk") != 0) {
		error_ln(sourceline, _("using reserved identifier `%s' as second component of a qualified name is not allowed"), subname);
		goto done;
	}

	if (strcmp(ns, "awk") == 0) {
		i = check_special(subname);
		if (i >= 0) {
			if ((tokentab[i].flags & GAWKX) != 0 && tokentab[i].class == LEX_BUILTIN)
				;	// gawk additional builtin function, is ok
			else
				error_ln(sourceline, _("using reserved identifier `%s' as second component of a qualified name is not allowed"), subname);
		}
	} else
		i = -1;
done:
	*end = ':';
	return i;
}

/* set_namespace --- change the current namespace */

static void
set_namespace(INSTRUCTION *ns, INSTRUCTION *comment)
{
	if (ns == NULL)
		return;

	if (do_traditional || do_posix) {
		error_ln(ns->source_line, _("@namespace is a gawk extension"));
		efree(ns->lextok);
		bcfree(ns);
		return;
	}

	if (! is_valid_identifier(ns->lextok)) {
		error_ln(ns->source_line, _("namespace name `%s' must meet identifier naming rules"), ns->lextok);
		efree(ns->lextok);
		bcfree(ns);
		return;
	}

	int mid = check_special(ns->lextok);

	if (mid >= 0) {
		error_ln(ns->source_line, _("using reserved identifier `%s' as a namespace is not allowed"), ns->lextok);
		efree(ns->lextok);
		bcfree(ns);
		return;
	}

	if (strcmp(ns->lextok, current_namespace) == 0)
		;	// nothing to do
	else if (strcmp(ns->lextok, awk_namespace) == 0) {
		set_current_namespace(awk_namespace);
	} else {
		set_current_namespace(estrdup(ns->lextok, strlen(ns->lextok)));
	}
	efree(ns->lextok);

	// save info and push on front of list of namespaces seen
	INSTRUCTION *new_ns = instruction(Op_K_namespace);
	new_ns->comment = comment;
	new_ns->ns_name = estrdup(current_namespace, strlen(current_namespace));
	new_ns->nexti = namespace_chain;
	namespace_chain = new_ns;

	ns->lextok = NULL;
	bcfree(ns);

	namespace_changed = true;

	return;
}

/* qualify_name --- put name into namespace */

static char *
qualify_name(const char *name, size_t len)
{
	if (strchr(name, ':') != NULL)	// already qualified
		return estrdup(name, len);

	NODE *p = lookup(name);
	if (p != NULL && p->type == Node_param_list)
		return estrdup(name, len);

	if (current_namespace != awk_namespace && ! is_all_upper(name)) {
		size_t length = strlen(current_namespace) + 2 + len + 1;
		char *buf;

		emalloc(buf, char *, length, "qualify_name");
		sprintf(buf, "%s::%s", current_namespace, name);

		return buf;
	}

	return estrdup(name, len);
}
