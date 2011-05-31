/*
 * awkgram.y --- yacc/bison parser
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
static int func_install(INSTRUCTION *fp, INSTRUCTION *def);
static void pop_params(NODE *params);
static NODE *make_param(char *pname);
static NODE *mk_rexp(INSTRUCTION *exp);
static void append_param(char *pname);
static int dup_parms(INSTRUCTION *fp, NODE *func);
static void param_sanity(INSTRUCTION *arglist);
static int parms_shadow(INSTRUCTION *pc, int *shadow);
static int isnoeffect(OPCODE type);
static INSTRUCTION *make_assignable(INSTRUCTION *ip);
static void dumpintlstr(const char *str, size_t len);
static void dumpintlstr2(const char *str1, size_t len1, const char *str2, size_t len2);
static int isarray(NODE *n);
static int include_source(INSTRUCTION *file);
static void next_sourcefile(void);
static char *tokexpand(void);

#define instruction(t)	bcalloc(t, 1, 0)

static INSTRUCTION *mk_program(void);
static INSTRUCTION *append_rule(INSTRUCTION *pattern, INSTRUCTION *action);
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
static NODE *make_regnode(int type, NODE *exp);
static int count_expressions(INSTRUCTION **list, int isarg);
static INSTRUCTION *optimize_assignment(INSTRUCTION *exp);
static void add_lint(INSTRUCTION *list, LINTTYPE linttype);

enum defref { FUNC_DEFINE, FUNC_USE };
static void func_use(const char *name, enum defref how);
static void check_funcs(void);
static void free_bcpool(INSTRUCTION *pl);

static ssize_t read_one_line(int fd, void *buffer, size_t count);
static int one_line_close(int fd);

static void (*install_func)(char *) = NULL;

static int want_source = FALSE;
static int want_regexp;		/* lexical scanning kludge */
static int can_return;		/* parsing kludge */
static int rule = 0;

const char *const ruletab[] = {
	"?",
	"BEGIN",
	"Rule",
	"END",
	"BEGINFILE",
	"ENDFILE",
};

static int in_print = FALSE;	/* lexical scanning kludge for print */
static int in_parens = 0;	/* lexical scanning kludge for print */
static int sub_counter = 0;	/* array dimension counter for use in delete */
static char *lexptr = NULL;		/* pointer to next char during parsing */
static char *lexend;
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static char *lexeme;		/* beginning of lexeme for debugging */
static int lexeof;			/* seen EOF for current source? */  
static char *thisline = NULL;
static int in_braces = 0;	/* count braces for firstline, lastline in an 'action' */
static int lastline = 0;
static int firstline = 0;
static SRCFILE *sourcefile = NULL;	/* current program source */
static int lasttok = 0;
static int eof_warned = FALSE;	/* GLOBAL: want warning for each file */
static int break_allowed;	/* kludge for break */
static int continue_allowed;	/* kludge for continue */


#define END_FILE	-1000
#define END_SRC  	-2000

#define YYDEBUG_LEXER_TEXT (lexeme)
static int param_counter;
static NODE *func_params;	/* list of parameters for the current function */
static char *tokstart = NULL;
static char *tok = NULL;
static char *tokend;
static int errcount = 0;

static NODE *symbol_list;
extern void destroy_symbol(char *name); 

static long func_count;		/* total number of functions */

#define HASHSIZE	1021	/* this constant only used here */
NODE *variables[HASHSIZE];
static int var_count;		/* total number of global variables */

extern char *source;
extern int sourceline;
extern SRCFILE *srcfiles;
extern INSTRUCTION *rule_list;
extern int max_args;

static INSTRUCTION *rule_block[sizeof(ruletab)];

static INSTRUCTION *ip_rec;
static INSTRUCTION *ip_newfile;
static INSTRUCTION *ip_atexit = NULL;
static INSTRUCTION *ip_end;
static INSTRUCTION *ip_endfile;
static INSTRUCTION *ip_beginfile;

static inline INSTRUCTION *list_create(INSTRUCTION *x);
static inline INSTRUCTION *list_append(INSTRUCTION *l, INSTRUCTION *x);
static inline INSTRUCTION *list_prepend(INSTRUCTION *l, INSTRUCTION *x);
static inline INSTRUCTION *list_merge(INSTRUCTION *l1, INSTRUCTION *l2);

extern double fmod(double x, double y);
/*
 * This string cannot occur as a real awk identifier.
 * Use it as a special token to make function parsing
 * uniform, but if it's seen, don't install the function.
 * e.g.
 * 	function split(x) { return x }
 * 	function x(a) { return a }
 * should only produce one error message, and not core dump.
 */
static char builtin_func[] = "@builtin";

#define YYSTYPE INSTRUCTION *
%}

%token FUNC_CALL NAME REGEXP FILENAME
%token YNUMBER YSTRING
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
%token LEX_INCLUDE LEX_EVAL
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
%left YSTRING YNUMBER
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
	| program rule
	  {
		rule = 0;
		yyerrok;
	  }
	| program nls
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
	  }
	| pattern statement_term
	  {
		if (rule != Rule) {
			msg(_("%s blocks must have an action part"), ruletab[rule]);
			errcount++;
		} else if ($1 == NULL) {
			msg(_("each rule must have a pattern or an action part"));
			errcount++;
		} else		/* pattern rule with non-empty pattern */
			(void) append_rule($1, NULL);
	  }
	| function_prologue action
	  {
		can_return = FALSE;
		if ($1 && func_install($1, $2) < 0)
			YYABORT;
		func_params = NULL;
		yyerrok;
	  }
	| '@' LEX_INCLUDE source statement_term
	  {
		want_source = FALSE;
		yyerrok;
	  }
	;

source
	: FILENAME
	  {
		if (include_source($1) < 0)
			YYABORT;
		efree($1->lextok);
		bcfree($1);
		$$ = NULL;
	  }
	| FILENAME error
	  { $$ = NULL; }
	| error
	  { $$ = NULL; }
	;

pattern
	: /* empty */
	  {	$$ = NULL; rule = Rule; }
	| exp
	  {	$$ = $1; rule = Rule; }
	| exp ',' opt_nls exp
	  {
		INSTRUCTION *tp;

		add_lint($1, LINT_assign_in_cond);
		add_lint($4, LINT_assign_in_cond);

		tp = instruction(Op_no_op);
		list_prepend($1, bcalloc(Op_line_range, !!do_profiling + 1, 0));
		$1->nexti->triggered = FALSE;
		$1->nexti->target_jmp = $4->nexti;

		list_append($1, instruction(Op_cond_pair));
		$1->lasti->line_range = $1->nexti;
		$1->lasti->target_jmp = tp;

		list_append($4, instruction(Op_cond_pair));
		$4->lasti->line_range = $1->nexti;
		$4->lasti->target_jmp = tp;
		if (do_profiling) {
			($1->nexti + 1)->condpair_left = $1->lasti;
			($1->nexti + 1)->condpair_right = $4->lasti;
		}
		$$ = list_append(list_merge($1, $4), tp);
		rule = Rule;
	  }
	| LEX_BEGIN
	  {
		static int begin_seen = 0;
		if (do_lint_old && ++begin_seen == 2)
			warning_ln($1->source_line,
				_("old awk does not support multiple `BEGIN' or `END' rules"));

		$1->in_rule = rule = BEGIN;
		$1->source_file = source;
		$$ = $1;
	  }
	| LEX_END
	  {
		static int end_seen = 0;
		if (do_lint_old && ++end_seen == 2)
			warning_ln($1->source_line,
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
		if ($2 == NULL)
			$$ = list_create(instruction(Op_no_op));
		else
			$$ = $2;
	  }
	;

func_name
	: NAME
	  { $$ = $1; }
	| FUNC_CALL
	  { $$ = $1; }
	| lex_builtin
	  {
		yyerror(_("`%s' is a built-in function, it cannot be redefined"),
			tokstart);
		$1->opcode = Op_symbol;	/* Op_symbol instead of Op_token so that
		                         * free_bc_internal does not try to free it
		                         */
		$1->lextok = builtin_func;
		$$ = $1;
		/* yyerrok; */
	  }
	| '@' LEX_EVAL
	  { $$ = $2; }
	;

lex_builtin
	: LEX_BUILTIN
	| LEX_LENGTH
	;
		
function_prologue
	: LEX_FUNCTION
	  {
		param_counter = 0;
		func_params = NULL;
	  }
		func_name '(' opt_param_list r_paren opt_nls
	 	{
			NODE *t;

			$1->source_file = source;
			t = make_param($3->lextok);
			$3->lextok = NULL;
			bcfree($3);
			t->flags |= FUNC;
			t->rnode = func_params;
			func_params = t;
			$$ = $1;
			can_return = TRUE;
			/* check for duplicate parameter names */
			if (dup_parms($1, t))
				errcount++;
		}
	;

regexp
	/*
	 * In this rule, want_regexp tells yylex that the next thing
	 * is a regexp so it should read up to the closing slash.
	 */
	: a_slash
		{ ++want_regexp; }
	  REGEXP	/* The terminating '/' is consumed by yylex(). */
		{
		  NODE *n, *exp;
		  char *re;
		  size_t len;

		  re = $3->lextok;
		  len = strlen(re);
		  if (do_lint) {
			if (len == 0)
				lintwarn_ln($3->source_line,
					_("regexp constant `//' looks like a C++ comment, but is not"));
			else if ((re)[0] == '*' && (re)[len-1] == '*')
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

a_slash
	: '/'
	  { bcfree($1); }
	| SLASH_BEFORE_EQUAL
	;

statements
	: /* empty */
	  {	$$ = NULL; }
	| statements statement
	  {
		if ($2 == NULL)
			$$ = $1;
		else {
			add_lint($2, LINT_no_effect);
			if ($1 == NULL)
				$$ = $2;
			else
				$$ = list_merge($1, $2);
		}
	    yyerrok;
	  }
	| statements error
	  {	$$ = NULL; }
	;

statement_term
	: nls
	| semi opt_nls
	;

statement
	: semi opt_nls
	  { $$ = NULL; }
	| l_brace statements r_brace
	  { $$ = $2; }
	| if_statement
	  {
		if (do_profiling)
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
		} /*  else
				curr = NULL; */

		for(; curr != NULL; curr = nextc) {
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
					(curr + 1)->match_exp = TRUE;
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

				if (do_profiling) {
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
		if (do_profiling) {
			(void) list_prepend(ip, $1);
			(void) list_prepend(ip, instruction(Op_exec_count));
			$1->target_break = tbreak;
			($1 + 1)->switch_start = cexp->nexti;
			($1 + 1)->switch_end = cexp->lasti;
		}/* else
				$1 is NULL */

		(void) list_append(cexp, dflt);
		(void) list_merge(ip, cexp);
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

		if (do_profiling) {
			(void) list_append(ip, instruction(Op_exec_count));
			$1->target_break = tbreak;
			$1->target_continue = tcont;
			($1 + 1)->while_body = ip->lasti;
			(void) list_prepend(ip, $1);
		}/* else
				$1 is NULL */

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
		if (do_profiling)
			(void) list_prepend(ip, instruction(Op_exec_count));
		(void) list_append(ip, instruction(Op_jmp_true));
		ip->lasti->target_jmp = ip->nexti;
		$$ = list_append(ip, tbreak);

		break_allowed--;
		continue_allowed--;
		fix_break_continue(ip, tbreak, tcont);

		if (do_profiling) {
			$1->target_break = tbreak;
			$1->target_continue = tcont;
			($1 + 1)->doloop_cond = tcont;
			$$ = list_prepend(ip, $1);
			bcfree($4);
		} /* else
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
		
		/* Efficiency hack.  Recognize the special case of
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
			$4->array_var = variable(var_name, Node_var);
			$4->target_jmp = tbreak;
			tcont = $4;
			$3->opcode = Op_arrayfor_init;
			$3->target_jmp = tbreak;
			(void) list_append(ip, $3);

			if (do_profiling) {
				$1->opcode = Op_K_arrayfor;
				$1->target_continue = tcont;
				$1->target_break = tbreak;
				(void) list_append(ip, $1);
			} /* else
					$1 is NULL */

			/* add update_FOO instruction if necessary */ 
			if ($4->array_var->type == Node_var && $4->array_var->var_update) {
				(void) list_append(ip, instruction(Op_var_update));
				ip->lasti->memory = $4->array_var;
				ip->lasti->update_var = $4->array_var->var_update;
			}
			(void) list_append(ip, $4);

			/* add set_FOO instruction if necessary */
			if ($4->array_var->type == Node_var && $4->array_var->var_assign) {
				(void) list_append(ip, instruction(Op_var_assign));
				ip->lasti->memory = $4->array_var;
				ip->lasti->assign_var = $4->array_var->var_assign;
			}

			if (do_profiling) {
				(void) list_append(ip, instruction(Op_exec_count));
				($1 + 1)->forloop_cond = $4;
				($1 + 1)->forloop_body = ip->lasti; 
			}

			if ($8 != NULL)
				(void) list_merge(ip, $8);

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
		$$ = mk_for_loop($1, $3, $6, $9, $12);

		break_allowed--;
		continue_allowed--;
	  }
	| LEX_FOR '(' opt_simple_stmt semi opt_nls semi opt_nls opt_simple_stmt r_paren opt_nls statement
	  {
		$$ = mk_for_loop($1, $3, (INSTRUCTION *) NULL, $8, $11);

		break_allowed--;
		continue_allowed--;
	  }
	| non_compound_stmt
	  {
		if (do_profiling)
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

	  }
	| LEX_CONTINUE statement_term
	  {
		if (! continue_allowed)
			error_ln($1->source_line,
				_("`continue' is not allowed outside a loop"));
		$1->target_jmp = NULL;
		$$ = list_create($1);

	  }
	| LEX_NEXT statement_term
	  {
		/* if inside function (rule = 0), resolve context at run-time */
		if (rule && rule != Rule)
			error_ln($1->source_line,
				_("`next' used in %s action"), ruletab[rule]);
		$1->target_jmp = ip_rec;
		$$ = list_create($1);
	  }
	| LEX_NEXTFILE statement_term
	  {
		if (do_traditional)
			error_ln($1->source_line,
				_("`nextfile' is a gawk extension"));

		/* if inside function (rule = 0), resolve context at run-time */
		if (rule == BEGIN || rule == END || rule == ENDFILE)
			error_ln($1->source_line,
				_("`nextfile' used in %s action"), ruletab[rule]);

		$1->target_jmp = ip_newfile;
		$1->target_endfile = ip_endfile;
		$$ = list_create($1);
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
			$$->nexti->memory = Nnull_string;
		} else
			$$ = list_append($2, $1);
	  }
	| LEX_RETURN
	  {
		if (! can_return)
			yyerror(_("`return' used outside function context"));
	  } opt_exp statement_term {
		if ($3 == NULL) {
			$$ = list_create($1);
			(void) list_prepend($$, instruction(Op_push_i));
			$$->nexti->memory = Nnull_string;
		} else
			$$ = list_append($3, $1);
	  }
	| simple_stmt statement_term
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
	: print { in_print = TRUE; in_parens = 0; } print_expression_list output_redir
	  {
		/*
		 * Optimization: plain `print' has no expression list, so $3 is null.
		 * If $3 is NULL or is a bytecode list for $0 use Op_K_print_rec,
		 * which is faster for these two cases.
		 */

		if ($1->opcode == Op_K_print &&
				($3 == NULL
					|| ($3->lasti->opcode == Op_field_spec
						&& $3->nexti->nexti->nexti == $3->lasti
						&& $3->nexti->nexti->opcode == Op_push_i
						&& $3->nexti->nexti->memory->type == Node_val
						&& $3->nexti->nexti->memory->numbr == 0.0)
				)
		) {
			static short warned = FALSE;
			/*   -----------------
			 *      output_redir
			 *    [ redirect exp ]
			 *   -----------------
			 *     expression_list
			 *   ------------------
			 *    [Op_K_print_rec | NULL | redir_type | expr_count]
			 */

			if ($3 != NULL) {
				bcfree($3->lasti);				/* Op_field_spec */
				$3->nexti->nexti->memory->flags &= ~PERM;
				$3->nexti->nexti->memory->flags |= MALLOC;			
				unref($3->nexti->nexti->memory);	/* Node_val */
				bcfree($3->nexti->nexti);		/* Op_push_i */
				bcfree($3->nexti);				/* Op_list */
				bcfree($3);						/* Op_list */
			} else {
				if (do_lint && (rule == BEGIN || rule == END) && ! warned) {
					warned = TRUE;
					lintwarn_ln($1->source_line,
		_("plain `print' in BEGIN or END rule should probably be `print \"\"'"));
				}
			}

			$1->expr_count = 0;
			$1->opcode = Op_K_print_rec;
			if ($4 == NULL) {    /* no redircetion */
				$1->redir_type = 0;
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
			 
			if ($4 == NULL) {		/* no redirection */
				if ($3 == NULL)	{	/* printf without arg */
					$1->expr_count = 0;
					$1->redir_type = 0;
					$$ = list_create($1);
				} else {
					INSTRUCTION *t = $3;
					$1->expr_count = count_expressions(&t, FALSE);
					$1->redir_type = 0;
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
					$$ = list_append($4, $1);
				} else {
					INSTRUCTION *t = $3;
					$1->expr_count = count_expressions(&t, FALSE);
					$$ = list_append(list_merge($4, t), $1);
				}
			}
		}
	  }

	| LEX_DELETE NAME { sub_counter = 0; } delete_subscript_list
	  {
		char *arr = $2->lextok;

		$2->opcode = Op_push_array;
		$2->memory = variable(arr, Node_var_new);

		if ($4 == NULL) {
			static short warned = FALSE;

			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn_ln($1->source_line,
					_("`delete array' is a gawk extension"));
			}
			if (do_traditional)
				error_ln($1->source_line,
					_("`delete array' is a gawk extension"));
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
		static short warned = FALSE;
		char *arr = $3->lextok;

		if (do_lint && ! warned) {
			warned = TRUE;
			lintwarn_ln($1->source_line,
				_("`delete(array)' is a non-portable tawk extension"));
		}
		if (do_traditional) {
			error_ln($1->source_line,
				_("`delete array' is a gawk extension"));
		}
		$3->memory = variable(arr, Node_var_new);
		$3->opcode = Op_push_array;
		$1->expr_count = 0;
		$$ = list_append(list_create($3), $1);
	  }
	| exp
	  {	$$ = optimize_assignment($1); }
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
		if (do_profiling)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		$1->case_exp = $2;
		$1->case_stmt = casestmt;
		bcfree($3);
		$$ = $1;
	  }
	| LEX_DEFAULT colon opt_nls statements
	  {
		INSTRUCTION *casestmt = $4;
		if ($4 == NULL)
			casestmt = list_create(instruction(Op_no_op));
		if (do_profiling)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		bcfree($2);
		$1->case_stmt = casestmt;
		$$ = $1;
	  }
	;

case_value
	: YNUMBER
	  {	$$ = $1; }
	| '-' YNUMBER    %prec UNARY
	  { 
		$2->memory->numbr = -(force_number($2->memory));
		bcfree($1);
		$$ = $2;
	  }
	| '+' YNUMBER    %prec UNARY
	  {
		bcfree($1);
		$$ = $2;
	  }
	| YSTRING 
	  {	$$ = $1; }
	| regexp  
	  {
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
		in_print = FALSE;
		in_parens = 0;
		$$ = NULL;
	  }
	| IO_OUT { in_print = FALSE; in_parens = 0; } common_exp
	  {
		if ($1->redir_type == redirect_twoway
		    	&& $3->lasti->opcode == Op_K_getline_redir
		   	 	&& $3->lasti->redir_type == redirect_twoway)
			yyerror(_("multistage two-way pipelines don't work"));
		$$ = list_prepend($3, $1);
	  }
	;

if_statement
	: LEX_IF '(' exp r_paren opt_nls statement
	  {
		$$ = mk_condition($3, $1, $6, NULL, NULL);
	  }
	| LEX_IF '(' exp r_paren opt_nls statement
	     LEX_ELSE opt_nls statement
	  {
		$$ = mk_condition($3, $1, $6, $7, $9);
	  }
	;

nls
	: NEWLINE
	| nls NEWLINE
	;

opt_nls
	: /* empty */
	| nls
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
	| param_list
	;

param_list
	: NAME
	  {
		append_param($1->lextok);
		$1->lextok = NULL;
		bcfree($1);
	  }
	| param_list comma NAME
	  {
		append_param($3->lextok);
		$3->lextok = NULL;
		bcfree($3);
		yyerrok;
	  }
	| error
	  { /* func_params = NULL; */ }
	| param_list error
	  { /* func_params = NULL; */ }
	| param_list comma error
	  { /* func_params = NULL; */ }
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
		$$ = mk_expression_list($1, $3);
		yyerrok;
	  }
	| error
	  { $$ = NULL; }
	| expression_list error
	  { $$ = NULL; }
	| expression_list error exp
	  { $$ = NULL; }
	| expression_list comma error
	  { $$ = NULL; }
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
	| exp LEX_AND exp
	  {	$$ = mk_boolean($1, $3, $2); }
	| exp LEX_OR exp
	  {	$$ = mk_boolean($1, $3, $2); }
	| exp MATCHOP exp
	  {
		if ($1->lasti->opcode == Op_match_rec)
			warning_ln($2->source_line,
				_("regular expression on left of `~' or `!~' operator"));

		if ($3->lasti == $3->nexti && $3->nexti->opcode == Op_match_rec) {
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
		  warning_ln($2->source_line,
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
		int is_simple_var = FALSE;

		if ($1->lasti->opcode == Op_concat) {
			/* multiple (> 2) adjacent strings optimization */
			is_simple_var = ($1->lasti->concat_flag & CSVAR);
			count = $1->lasti->expr_count + 1;
			$1->lasti->opcode = Op_no_op;
		} else {
			is_simple_var = ($1->nexti->opcode == Op_push
						&& $1->lasti == $1->nexti); /* first exp. is a simple
						                             * variable?; kludge for use
						                             * in Op_assign_concat.
			 			                             */
		}

		if (do_optimize > 1
				&& $1->nexti == $1->lasti && $1->nexti->opcode == Op_push_i
				&& $2->nexti == $2->lasti && $2->nexti->opcode == Op_push_i
		) {
			NODE *n1 = $1->nexti->memory;
			NODE *n2 = $2->nexti->memory;
			size_t nlen;

			(void) force_string(n1);
			(void) force_string(n2);
			nlen = n1->stlen + n2->stlen;
			erealloc(n1->stptr, char *, nlen + 2, "constant fold");
			memcpy(n1->stptr + n1->stlen, n2->stptr, n2->stlen);
			n1->stlen = nlen;
			n1->stptr[nlen] = '\0';
			n1->flags &= ~(NUMCUR|NUMBER);
			n1->flags |= (STRING|STRCUR);

			n2->flags &= ~PERM;
			n2->flags |= MALLOC;
			unref(n2);
			bcfree($2->nexti);
			bcfree($2);
			$$ = $1;
		} else {
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
		 * In BEGINFILE/ENDFILE, allow `getline var < file'
		 */

		if (rule == BEGINFILE || rule == ENDFILE) {
			if ($2 != NULL && $3 != NULL)
				;	 /* all  ok */
			else {
				if ($2 != NULL)
					error_ln($1->source_line,
						_("`getline var' invalid inside `%s' rule"), ruletab[rule]);
				else
					error_ln($1->source_line,
						_("`getline' invalid inside `%s' rule"), ruletab[rule]);
			}
		}
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
		    warning_ln($4->source_line,
				_("old awk does not support the keyword `in' except after `for'"));
		    warning_ln($4->source_line,
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
			$4->expr_count = count_expressions(&t, FALSE);
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
			$1->memory = mk_number(0.0, (PERM|NUMCUR|NUMBER));	
			$$ = list_append(list_append(list_create($1),
								instruction(Op_field_spec)), $2);
		} else {
			if (do_optimize > 1 && $2->nexti == $2->lasti
							&& $2->nexti->opcode == Op_push_i
			) {
				NODE *n = $2->nexti->memory;
				if ((n->flags & (STRCUR|STRING)) != 0) {
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
	  { $$ = $2; }
	| LEX_BUILTIN '(' opt_expression_list r_paren
	  {
		$$ = snode($3, $1);
		if ($$ == NULL)
			YYABORT;
	  }
	| LEX_LENGTH '(' opt_expression_list r_paren
	  {
		$$ = snode($3, $1);
		if ($$ == NULL)
			YYABORT;
	  }
	| LEX_LENGTH
	  {
		static short warned1 = FALSE;

		if (do_lint && ! warned1) {
			warned1 = TRUE;
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
				&& ($2->lasti->memory->flags & (STRCUR|STRING)) == 0) {
			$2->lasti->memory->numbr = -(force_number($2->lasti->memory));
			$$ = $2;
			bcfree($1);
		} else {
			$1->opcode = Op_unary_minus;
			$$ = list_append($2, $1);
		}
	  }
	| '+' simp_exp    %prec UNARY
	  {
	    /*
	     * was: $$ = $2
	     * POSIX semantics: force a conversion to numeric type
	     */
		$1->opcode = Op_plus_i;
		$1->memory = mk_number((AWKNUM) 0.0, (PERM|NUMCUR|NUMBER));
		$$ = list_append($2, $1);
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
		static short warned = FALSE;
		const char *msg = _("indirect function calls are a gawk extension");

		if (do_traditional || do_posix)
			yyerror("%s", msg);
		else if (do_lint && ! warned) {
			warned = TRUE;
			lintwarn("%s", msg);
		}
		
		f = $2->lasti;
		f->opcode = Op_indirect_func_call;
		name = estrdup(f->func_name, strlen(f->func_name));
		if (is_std_var(name))
			yyerror(_("can not use special variable `%s' for indirect function call"), name);
		indirect_var = variable(name, Node_var_new);
		t = instruction(Op_push);
		t->memory = indirect_var;

		/* prepend indirect var instead of appending to arguments (opt_expression_list),
		 * and pop it off in setup_frame (eval.c) (left to right evaluation order); Test case:
		 *		f = "fun"
		 *		@f(f="real_fun")
		 */

		$$ = list_prepend($2, t);
	  }
	;

direct_func_call
	: FUNC_CALL '(' opt_expression_list r_paren
	  {
		param_sanity($3);
		$1->opcode = Op_func_call;
		$1->func_body = NULL;
		if ($3 == NULL) {	/* no argument or error */
			($1 + 1)->expr_count = 0;
			$$ = list_create($1);
		} else {
			INSTRUCTION *t = $3;
			($1 + 1)->expr_count = count_expressions(&t, TRUE); 
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
			t->nexti->memory = Nnull_string;
			$3->sub_count = 1;			
		} else
			$3->sub_count = count_expressions(&t, FALSE);
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
		char *var_name = $1->lextok;

		$1->opcode = Op_push;
		$1->memory = variable(var_name, Node_var_new);
		$$ = list_create($1);
	  }
	| NAME subscript_list
	  {
		NODE *n;

		char *arr = $1->lextok;
		if ((n = lookup(arr)) != NULL && ! isarray(n))
			yyerror(_("use of non-array as array"));
		$1->memory = variable(arr, Node_var_new);
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
			$$->nexti->memory = ip->memory;
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
	| /* empty */	{ $$ = NULL; }
	;

l_brace
	: '{' opt_nls
	;

r_brace
	: '}' opt_nls	{ yyerrok; }
	;

r_paren
	: ')' { yyerrok; }
	;

opt_semi
	: /* empty */
	| semi
	;

semi
	: ';'	{ yyerrok; }
	;

colon
	: ':'	{ $$ = $1; yyerrok; }
	;

comma
	: ',' opt_nls	{ yyerrok; }
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
#	define	RESX		0x0800	/* Bell Labs Research extension */
#	define	BREAK		0x1000	/* break allowed inside */
#	define	CONTINUE	0x2000	/* continue allowed inside */
	NODE *(*ptr)(int);	/* function that implements this keyword */
};

#if 'a' == 0x81 /* it's EBCDIC */
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

static const struct token tokentab[] = {
{"BEGIN",	Op_rule,	 LEX_BEGIN,	0,		0},
{"BEGINFILE",	Op_rule,	 LEX_BEGINFILE,	GAWKX,		0},
{"END",		Op_rule,	 LEX_END,	0,		0},
{"ENDFILE",		Op_rule,	 LEX_ENDFILE,	GAWKX,		0},
#ifdef ARRAYDEBUG
{"adump",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_adump},
#endif
{"and",		Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_and},
{"asort",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_asort},
{"asorti",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_asorti},
{"atan2",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2),	do_atan2},
{"bindtextdomain",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2),	do_bindtextdomain},
{"break",	Op_K_break,	 LEX_BREAK,	0,		0},
{"case",	Op_K_case,	 LEX_CASE,	GAWKX,		0},
{"close",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1)|A(2),	do_close},
{"compl",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_compl},
{"continue",	Op_K_continue, LEX_CONTINUE,	0,		0},
{"cos",		Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_cos},
{"dcgettext",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_dcgettext},
{"dcngettext",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3)|A(4)|A(5),	do_dcngettext},
{"default",	Op_K_default,	 LEX_DEFAULT,	GAWKX,		0},
{"delete",	Op_K_delete,	 LEX_DELETE,	NOT_OLD,	0},
{"do",		Op_K_do,	 LEX_DO,	NOT_OLD|BREAK|CONTINUE,	0},
{"else",	Op_K_else,	 LEX_ELSE,	0,		0},
{"eval",	Op_symbol,	 LEX_EVAL,	0,		0},
{"exit",	Op_K_exit,	 LEX_EXIT,	0,		0},
{"exp",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_exp},
{"extension",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(2),	do_ext},
{"fflush",	Op_builtin,	 LEX_BUILTIN,	RESX|A(0)|A(1), do_fflush},
{"for",		Op_K_for,	 LEX_FOR,	BREAK|CONTINUE,	0},
{"func",	Op_func, LEX_FUNCTION,	NOT_POSIX|NOT_OLD,	0},
{"function",Op_func, LEX_FUNCTION,	NOT_OLD,	0},
{"gensub",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(3)|A(4), do_gensub},
{"getline",	Op_K_getline_redir,	 LEX_GETLINE,	NOT_OLD,	0},
{"gsub",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_gsub},
{"if",		Op_K_if,	 LEX_IF,	0,		0},
{"in",		Op_symbol,	 LEX_IN,	0,		0},
{"include",  Op_symbol,	 LEX_INCLUDE,	GAWKX,	0},
{"index",	Op_builtin,	 LEX_BUILTIN,	A(2),		do_index},
{"int",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_int},
{"isarray",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1),	do_isarray},
{"length",	Op_builtin,	 LEX_LENGTH,	A(0)|A(1),	do_length},
{"log",		Op_builtin,	 LEX_BUILTIN,	A(1),		do_log},
{"lshift",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_lshift},
{"match",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_match},
{"mktime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(1),	do_mktime},
{"next",	Op_K_next,	 LEX_NEXT,	0,		0},
{"nextfile",	Op_K_nextfile, LEX_NEXTFILE,	GAWKX,		0},
{"or",		Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_or},
{"patsplit",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2)|A(3)|A(4), do_patsplit},
{"print",	Op_K_print,	 LEX_PRINT,	0,		0},
{"printf",	Op_K_printf,	 LEX_PRINTF,	0,		0},
{"rand",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0),	do_rand},
{"return",	Op_K_return,	 LEX_RETURN,	NOT_OLD,	0},
{"rshift",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_rshift},
{"sin",		Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_sin},
{"split",	Op_builtin,	 LEX_BUILTIN,	A(2)|A(3)|A(4),	do_split},
{"sprintf",	Op_builtin,	 LEX_BUILTIN,	0,		do_sprintf},
{"sqrt",	Op_builtin,	 LEX_BUILTIN,	A(1),		do_sqrt},
{"srand",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0)|A(1), do_srand},
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG) /* || ... */
{"stopme",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(0),	stopme},
#endif
{"strftime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(0)|A(1)|A(2)|A(3), do_strftime},
{"strtonum",	Op_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_strtonum},
{"sub",		Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_sub},
{"substr",	Op_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_substr},
{"switch",	Op_K_switch,	 LEX_SWITCH,	GAWKX|BREAK,	0},
{"system",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_system},
{"systime",	Op_builtin,	 LEX_BUILTIN,	GAWKX|A(0),	do_systime},
{"tolower",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_tolower},
{"toupper",	Op_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_toupper},
{"while",	Op_K_while,	 LEX_WHILE,	BREAK|CONTINUE,	0},
{"xor",		Op_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_xor},
};

#ifdef MBS_SUPPORT
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
#else /* MBS_SUPPORT */
/* a dummy */
#define nextc_is_1stbyte 1
#endif /* MBS_SUPPORT */

/* getfname --- return name of a builtin function (for pretty printing) */

const char *
getfname(NODE *(*fptr)(int))
{
	int i, j;

	j = sizeof(tokentab) / sizeof(tokentab[0]);
	/* linear search, no other way to do it */
	for (i = 0; i < j; i++) 
		if (tokentab[i].ptr == fptr)
			return tokentab[i].operator;

	return NULL;
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
	err(_("warning: "), mesg, args);
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
		err(_("fatal: "), mesg, args);
	else
		err(_("warning: "), mesg, args);
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
	err("error: ", m, args);
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
	char save;

	print_included_from();

	errcount++;
	/* Find the current line in the input file */
	if (lexptr && lexeme) {
		if (thisline == NULL) {
			cp = lexeme;
			if (*cp == '\n') {
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
		while (bp < lexend && *bp && *bp != '\n')
			bp++;
	} else {
		thisline = end_of_file_line;
		bp = thisline + strlen(thisline);
	}

	/*
	 * Saving and restoring *bp keeps valgrind happy,
	 * since the guts of glibc uses strlen, even though
	 * we're passing an explict precision. Sigh.
	 *
	 * 8/2003: We may not need this anymore.
	 */
	save = *bp;
	*bp = '\0';

	msg("%.*s", (int) (bp - thisline), thisline);

	*bp = save;
	va_start(args, m);
	if (mesg == NULL)
		mesg = m;

	count = (bp - thisline) + strlen(mesg) + 2 + 1;
	emalloc(buf, char *, count, "yyerror");

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
	err("", buf, args);
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
		extern int has_endfile;	/* kludge for use in inrec (io.c) */
		has_endfile = TRUE;
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
			(void) list_append(cp, ip_atexit);
			(void) list_append(cp, instruction(Op_stop));

			/* append beginfile_block and endfile_block for sole use
			 * in getline without redirection (Op_K_getline).
			 */

			(void) list_merge(cp, beginfile_block);
			(void) list_merge(cp, endfile_block);

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
parse_program(INSTRUCTION **pcode)
{
	int ret;

	/* pre-create non-local jump targets
	 * ip_end (Op_no_op) -- used as jump target for `exit'
	 * outside an END block.
	 */
	ip_end = instruction(Op_no_op);

	if (! in_main_context())
		ip_newfile = ip_rec = ip_atexit = ip_beginfile = ip_endfile = NULL;
	else {
		ip_endfile = instruction(Op_no_op);
		ip_beginfile = instruction(Op_no_op);
		ip_newfile = instruction(Op_newfile); /* target for `nextfile' */
		ip_newfile->target_jmp = ip_end;
		ip_newfile->target_endfile = ip_endfile;
		ip_rec = instruction(Op_get_record); /* target for `next' */
		ip_atexit = instruction(Op_atexit);	/* target for `exit' in END block */
	}

	sourcefile = srcfiles->next;
	lexeof = FALSE;
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

	return (ret || errcount);
}

/* do_add_srcfile --- add one item to srcfiles */

static SRCFILE *
do_add_srcfile(int stype, char *src, char *path, SRCFILE *thisfile)
{
	SRCFILE *s;

	emalloc(s, SRCFILE *, sizeof(SRCFILE), "do_add_srcfile");
	memset(s, 0, sizeof(SRCFILE));
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
add_srcfile(int stype, char *src, SRCFILE *thisfile, int *already_included, int *errcode)
{
	SRCFILE *s;
	struct stat sbuf;
	char *path;
	int errno_val = 0;

	if (already_included)
		*already_included = FALSE;
	if (errcode)
		*errcode = 0;
	if (stype == SRC_CMDLINE || stype == SRC_STDIN)
		return do_add_srcfile(stype, src, NULL, thisfile);

	path = find_source(src, &sbuf, &errno_val);
	if (path == NULL) {
		if (errcode) {
			*errcode = errno_val;
			return NULL;
		}
		fatal(_("can't open source file `%s' for reading (%s)"),
				src, errno_val ? strerror(errno_val) : _("reason unknown"));
	}

	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if ((s->stype == SRC_FILE || s->stype == SRC_INC)
				&& files_are_same(path, s)
		) {
			if (do_lint) {
				int line = sourceline;
				/* Kludge: the line number may be off for `@include file'.
				 * Since, this function is also used for '-f file' in main.c,
				 * sourceline > 1 check ensures that the call is at
				 * parse time.
				 */
				if (sourceline > 1 && lasttok == NEWLINE)
					line--;
				lintwarn_ln(line, _("already included source file `%s'"), src);
			}
			efree(path);
			if (already_included)
				*already_included = TRUE;
			return NULL;
		}
	}

	s = do_add_srcfile(stype, src, path, thisfile);
	s->sbuf = sbuf;
	s->mtime = sbuf.st_mtime;
	return s;
}

/* include_source --- read program from source included using `@include' */

static int
include_source(INSTRUCTION *file)
{
	SRCFILE *s;
	char *src = file->lextok;
	int errcode;
	int already_included;

	if (do_traditional || do_posix) {
		error_ln(file->source_line, _("@include is a gawk extension"));
		return -1;
	}

	if (strlen(src) == 0) {
		if (do_lint)
			lintwarn_ln(file->source_line, _("empty filename after @include"));
		return 0;
	}

	s = add_srcfile(SRC_INC, src, sourcefile, &already_included, &errcode);
	if (s == NULL) {
		if (already_included)
			return 0;
		error_ln(file->source_line,
			_("can't open source file `%s' for reading (%s)"),
			src, errcode ? strerror(errcode) : _("reason unknown"));
		return -1;
	}

	/* save scanner state for the current sourcefile */
	sourcefile->srclines = sourceline;
	sourcefile->lexptr = lexptr;
	sourcefile->lexend = lexend;
	sourcefile->lexptr_begin = lexptr_begin;        
	sourcefile->lexeme = lexeme;
	sourcefile->lasttok = lasttok;

	/* included file becomes the current source */ 
	sourcefile = s;
	lexptr = NULL;
	sourceline = 0;
	source = NULL;
	lasttok = 0;
	lexeof = FALSE;
	eof_warned = FALSE;
	return 0;
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
	 * assert(lexeof == TRUE);
	 */
	lexeof = FALSE;
	eof_warned = FALSE;
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

	sourcefile = sourcefile->next;
	if (sourcefile == srcfiles)
		return;

	if (sourcefile->lexptr_begin != NULL) {
		/* resume reading from already opened file (postponed to process '@include') */
		lexptr = sourcefile->lexptr;
		lexend = sourcefile->lexend;
		lasttok = sourcefile->lasttok;
		lexptr_begin = sourcefile->lexptr_begin;
		lexeme = sourcefile->lexeme;
		sourceline = sourcefile->srclines;
		source = sourcefile->src;
	} else {
		lexptr = NULL;
		sourceline = 0;
		source = NULL;
		lasttok = 0;
	}
}

/* get_src_buf --- read the next buffer of source program */

static char *
get_src_buf()
{
	int n;
	char *scan;
	int newfile;
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

	newfile = FALSE;
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
				static short warned = FALSE;

				if (do_lint && ! warned) {
					warned = TRUE;
					lintwarn(_("empty program text on command line"));
				}
				lexeof = TRUE;
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
			lexeof = TRUE;
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
			error(_("can't open source file `%s' for reading (%s)"),
				in, strerror(errno));
			errcount++;
			lexeof = TRUE;
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
		newfile = TRUE;
		emalloc(sourcefile->buf, char *, sourcefile->bufsize, "get_src_buf");
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
		error(_("can't read sourcefile `%s' (%s)"),
				source, strerror(errno));
		errcount++;
		lexeof = TRUE;
	} else {
		lexend = lexptr + n;
		if (n == 0) {
			static short warned = FALSE;
			if (do_lint && newfile && ! warned){
				warned = TRUE;
				sourceline = 0;
				lintwarn(_("source file `%s' is empty"), source);
			}
			lexeof = TRUE;
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

/* nextc --- get the next input character */

#ifdef MBS_SUPPORT

static int
nextc(void)
{
	if (gawk_mb_cur_max > 1) {
again:
		if (lexeof)
			return END_FILE;
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
	
			for (idx = 0 ; lexptr + idx < lexend ; idx++) {
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

		return (int) (unsigned char) *lexptr++;
	} else {
		do {
			if (lexeof)
				return END_FILE;
			if (lexptr && lexptr < lexend)
					return ((int) (unsigned char) *lexptr++);
		} while (get_src_buf());
		return END_SRC;
	}
}

#else /* MBS_SUPPORT */

int
nextc()
{
	do {
		if (lexeof)
			return END_FILE;
		if (lexptr && lexptr < lexend)
			return ((int) (unsigned char) *lexptr++);
	} while (get_src_buf());
	return END_SRC;
}

#endif /* MBS_SUPPORT */

/* pushback --- push a character back on the input */

static inline void
pushback(void)
{
#ifdef MBS_SUPPORT
	if (gawk_mb_cur_max > 1)
		cur_ring_idx = (cur_ring_idx == 0)? RING_BUFFER_SIZE - 1 :
			cur_ring_idx - 1;
#endif
	(! lexeof && lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr);
}


/* allow_newline --- allow newline after &&, ||, ? and : */

static void
allow_newline(void)
{
	int c;

	for (;;) {
		c = nextc();
		if (c == END_FILE) {
			pushback();
			break;
		}
		if (c == '#') {
			while ((c = nextc()) != '\n' && c != END_FILE)
				continue;
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

/* yylex --- Read the input and turn it into tokens. */

static int
yylex(void)
{
	int c;
	int seen_e = FALSE;		/* These are for numbers */
	int seen_point = FALSE;
	int esc_seen;		/* for literal strings */
	int mid;
	static int did_newline = FALSE;
	char *tokkey;
	int inhex = FALSE;
	int intlstr = FALSE;

#define GET_INSTRUCTION(op) bcalloc(op, 1, sourceline)

	/* NB: a newline at end does not start a source line. */

#define NEWLINE_EOF                                             \
    (lasttok != NEWLINE  ?                                      \
                (pushback(), do_lint && ! eof_warned &&         \
        (lintwarn(_("source file does not end in newline")),    \
       		eof_warned = TRUE), sourceline++, NEWLINE) :        \
                 (sourceline--, eof_warned = FALSE, LEX_EOF))


	yylval = (INSTRUCTION *) NULL;
	if (lasttok == SUBSCRIPT) {
		lasttok = 0;
		return SUBSCRIPT;
	}
 
	if (lasttok == LEX_EOF)		/* error earlier in current source, must give up !! */
		return 0;

	c = nextc();
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
	if (want_regexp) {
		int in_brack = 0;	/* count brackets, [[:alnum:]] allowed */
		/*
		 * Counting brackets is non-trivial. [[] is ok,
		 * and so is [\]], with a point being that /[/]/ as a regexp
		 * constant has to work.
		 *
		 * Do not count [ or ] if either one is preceded by a \.
		 * A `[' should be counted if
		 *  a) it is the first one so far (in_brack == 0)
		 *  b) it is the `[' in `[:'
		 * A ']' should be counted if not preceded by a \, since
		 * it is either closing `:]' or just a plain list.
		 * According to POSIX, []] is how you put a ] into a set.
		 * Try to handle that too.
		 *
		 * The code for \ handles \[ and \].
		 */

		want_regexp = FALSE;
		tok = tokstart;
		for (;;) {
			c = nextc();

			if (gawk_mb_cur_max == 1 || nextc_is_1stbyte) switch (c) {
			case '[':
				/* one day check for `.' and `=' too */
				if (nextc() == ':' || in_brack == 0)
					in_brack++;
				pushback();
				break;
			case ']':
				if (tokstart[0] == '['
				    && (tok == tokstart + 1
					|| (tok == tokstart + 2
					    && tokstart[1] == '^')))
					/* do nothing */;
				else
					in_brack--;
				break;
			case '\\':
				if ((c = nextc()) == END_FILE) {
					pushback();
					yyerror(_("unterminated regexp ends with `\\' at end of file"));
					goto end_regexp; /* kludge */
				} else if (c == '\n') {
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
					int peek = nextc();

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
				return lasttok = REGEXP;
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
	while ((c = nextc()) == ' ' || c == '\t' || c == '\r')
		continue;

	lexeme = lexptr ? lexptr - 1 : lexptr;
	thisline = NULL;
	tok = tokstart;

#ifdef MBS_SUPPORT
	if (gawk_mb_cur_max == 1 || nextc_is_1stbyte)
#endif
	switch (c) {
	case END_SRC:
		return 0;

	case END_FILE:
		return lasttok = NEWLINE_EOF;

	case '\n':
		sourceline++;
		return lasttok = NEWLINE;

	case '#':		/* it's a comment */
		while ((c = nextc()) != '\n') {
			if (c == END_FILE)
				return lasttok = NEWLINE_EOF;
		}
		sourceline++;
		return lasttok = NEWLINE;

	case '@':
		return lasttok = '@';

	case '\\':
#ifdef RELAXED_CONTINUATION
		/*
		 * This code puports to allow comments and/or whitespace
		 * after the `\' at the end of a line used for continuation.
		 * Use it at your own risk. We think it's a bad idea, which
		 * is why it's not on by default.
		 */
		if (! do_traditional) {
			/* strip trailing white-space and/or comment */
			while ((c = nextc()) == ' ' || c == '\t' || c == '\r')
				continue;
			if (c == '#') {
				static short warned = FALSE;

				if (do_lint && ! warned) {
					warned = TRUE;
					lintwarn(
		_("use of `\\ #...' line continuation is not portable"));
				}
				while ((c = nextc()) != '\n')
					if (c == END_FILE)
						break;
			}
			pushback();
		}
#endif /* RELAXED_CONTINUATION */
		c = nextc();
		if (c == '\r')	/* allow MS-DOS files. bleah */
			c = nextc();
		if (c == '\n') {
			sourceline++;
			goto retry;
		} else {
			yyerror(_("backslash not last character on line"));
			return lasttok = LEX_EOF;
		}
		break;

	case ':':
	case '?':
		yylval = GET_INSTRUCTION(Op_cond_exp);
		if (! do_posix)
			allow_newline();
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
		c = nextc();
		pushback();
		if (c == '[') {
			yylval = GET_INSTRUCTION(Op_sub_array);
			lasttok = ']';
		} else {
			yylval = GET_INSTRUCTION(Op_subscript);
			lasttok = SUBSCRIPT;	/* end of subscripts */
		}
		return ']';

	case '*':
		if ((c = nextc()) == '=') {
			yylval = GET_INSTRUCTION(Op_assign_times);
			return lasttok = ASSIGNOP;
		} else if (do_posix) {
			pushback();
			yylval = GET_INSTRUCTION(Op_times);
			return lasttok = '*';
		} else if (c == '*') {
			/* make ** and **= aliases for ^ and ^= */
			static int did_warn_op = FALSE, did_warn_assgn = FALSE;

			if (nextc() == '=') {
				if (! did_warn_assgn) {
					did_warn_assgn = TRUE;
					if (do_lint)
						lintwarn(_("POSIX does not allow operator `**='"));
					if (do_lint_old)
						warning(_("old awk does not support operator `**='"));
				}
				yylval = GET_INSTRUCTION(Op_assign_exp);
				return ASSIGNOP;
			} else {
				pushback();
				if (! did_warn_op) {
					did_warn_op = TRUE;
					if (do_lint)
						lintwarn(_("POSIX does not allow operator `**'"));
					if (do_lint_old)
						warning(_("old awk does not support operator `**'"));
				}
				yylval = GET_INSTRUCTION(Op_exp);
				return lasttok = '^';
			}
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_times);
		return lasttok = '*';

	case '/':
		if (nextc() == '=') {
			pushback();
			return lasttok = SLASH_BEFORE_EQUAL;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_quotient);
		return lasttok = '/';

	case '%':
		if (nextc() == '=') {
			yylval = GET_INSTRUCTION(Op_assign_mod);
			return lasttok = ASSIGNOP;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_mod);
		return lasttok = '%';

	case '^':
	{
		static int did_warn_op = FALSE, did_warn_assgn = FALSE;

		if (nextc() == '=') {
			if (do_lint_old && ! did_warn_assgn) {
				did_warn_assgn = TRUE;
				warning(_("operator `^=' is not supported in old awk"));
			}
			yylval = GET_INSTRUCTION(Op_assign_exp);
			return lasttok = ASSIGNOP;
		}
		pushback();
		if (do_lint_old && ! did_warn_op) {
			did_warn_op = TRUE;
			warning(_("operator `^' is not supported in old awk"));
		}
		yylval = GET_INSTRUCTION(Op_exp);	
		return lasttok = '^';
	}

	case '+':
		if ((c = nextc()) == '=') {
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
		if ((c = nextc()) == '=') {
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
		if (nextc() == '=') {
			yylval = GET_INSTRUCTION(Op_leq);
			return lasttok = RELOP;
		}
		yylval = GET_INSTRUCTION(Op_less);
		pushback();
		return lasttok = '<';

	case '=':
		if (nextc() == '=') {
			yylval = GET_INSTRUCTION(Op_equal);
			return lasttok = RELOP;
		}
		yylval = GET_INSTRUCTION(Op_assign);
		pushback();
		return lasttok = ASSIGN;

	case '>':
		if ((c = nextc()) == '=') {
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
			did_newline = FALSE;
			if (--in_braces == 0)
				lastline = sourceline;
			return lasttok = c;
		}
		did_newline++;
		--lexptr;	/* pick up } next time */
		return lasttok = NEWLINE;

	case '"':
	string:
		esc_seen = FALSE;
		while ((c = nextc()) != '"') {
			if (c == '\n') {
				pushback();
				yyerror(_("unterminated string"));
				return lasttok = LEX_EOF;
			}
			if ((gawk_mb_cur_max == 1 || nextc_is_1stbyte) &&
			    c == '\\') {
				c = nextc();
				if (c == '\n') {
					sourceline++;
					continue;
				}
				esc_seen = TRUE;
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
		yylval->memory->flags &= ~MALLOC;
		yylval->memory->flags |= PERM;
		if (intlstr) {
			yylval->memory->flags |= INTLSTR;
			intlstr = FALSE;
			if (do_intl)
				dumpintlstr(yylval->memory->stptr, yylval->memory->stlen);
		}
		return lasttok = YSTRING;

	case '-':
		if ((c = nextc()) == '=') {
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
		c = nextc();
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
			int gotnumber = FALSE;

			tokadd(c);
			switch (c) {
			case 'x':
			case 'X':
				if (do_traditional)
					goto done;
				if (tok == tokstart + 2) {
					int peek = nextc();

					if (isxdigit(peek)) {
						inhex = TRUE;
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
					gotnumber = TRUE;
					break;
				}
				seen_point = TRUE;
				break;
			case 'e':
			case 'E':
				if (inhex)
					break;
				if (seen_e) {
					gotnumber = TRUE;
					break;
				}
				seen_e = TRUE;
				if ((c = nextc()) == '-' || c == '+') {
					int c2 = nextc();

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
				gotnumber = TRUE;
			}
			if (gotnumber)
				break;
			c = nextc();
		}
		pushback();

		tokadd('\0');
		yylval = GET_INSTRUCTION(Op_push_i);
		if (! do_traditional && isnondecimal(tokstart, FALSE)) {
			if (do_lint) {
				if (isdigit((unsigned char) tokstart[1]))	/* not an 'x' or 'X' */
					lintwarn("numeric constant `%.*s' treated as octal",
						(int) strlen(tokstart)-1, tokstart);
				else if (tokstart[1] == 'x' || tokstart[1] == 'X')
					lintwarn("numeric constant `%.*s' treated as hexadecimal",
						(int) strlen(tokstart)-1, tokstart);
			}
			yylval->memory = mk_number(nondec2awknum(tokstart, strlen(tokstart)),
											PERM|NUMCUR|NUMBER);
		} else
			yylval->memory = mk_number(atof(tokstart), PERM|NUMCUR|NUMBER);
		return lasttok = YNUMBER;

	case '&':
		if ((c = nextc()) == '&') {
			yylval = GET_INSTRUCTION(Op_and);
			allow_newline();
			return lasttok = LEX_AND;
		}
		pushback();
		yylval = GET_INSTRUCTION(Op_symbol);
		return lasttok = '&';

	case '|':
		if ((c = nextc()) == '|') {
			yylval = GET_INSTRUCTION(Op_or);
			allow_newline();
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

	if (c != '_' && ! isalpha(c)) {
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
		if ((c = nextc()) == '"') {
			intlstr = TRUE;
			goto string;
		}
		pushback();
		c = '_';
	}

	/* it's some type of name-type-thing.  Find its length. */
	tok = tokstart;
	while (c != END_FILE && is_identchar(c)) {
		tokadd(c);
		c = nextc();
	}
	tokadd('\0');
	pushback();

	/* See if it is a special token. */
	if ((mid = check_special(tokstart)) >= 0) {
		static int warntab[sizeof(tokentab) / sizeof(tokentab[0])];
		int class = tokentab[mid].class;

		if ((class == LEX_INCLUDE || class == LEX_EVAL)
				&& lasttok != '@')
			goto out;

		if (do_lint) {
			if ((tokentab[mid].flags & GAWKX) && ! (warntab[mid] & GAWKX)) {
				lintwarn(_("`%s' is a gawk extension"),
					tokentab[mid].operator);
				warntab[mid] |= GAWKX;
			}
			if ((tokentab[mid].flags & RESX) && ! (warntab[mid] & RESX)) {
				lintwarn(_("`%s' is a Bell Labs extension"),
					tokentab[mid].operator);
				warntab[mid] |= RESX;
			}
			if ((tokentab[mid].flags & NOT_POSIX) && ! (warntab[mid] & NOT_POSIX)) {
				lintwarn(_("POSIX does not allow `%s'"),
					tokentab[mid].operator);
				warntab[mid] |= NOT_POSIX;
			}
		}
		if (do_lint_old && (tokentab[mid].flags & NOT_OLD)
				 && ! (warntab[mid] & NOT_OLD)
		) {
			warning(_("`%s' is not supported in old awk"),
					tokentab[mid].operator);
			warntab[mid] |= NOT_OLD;
		}

		if (tokentab[mid].flags & BREAK)
			break_allowed++;
		if (tokentab[mid].flags & CONTINUE)
			continue_allowed++;

		switch (class) {
		case LEX_INCLUDE:
			want_source = TRUE;
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
			yylval = bcalloc(tokentab[mid].value, 3, sourceline);
			break;

		case LEX_FOR:
		case LEX_WHILE:
		case LEX_DO:
		case LEX_SWITCH:
			if (! do_profiling)
				return lasttok = class;
			/* fall through */
		case LEX_CASE:
			yylval = bcalloc(tokentab[mid].value, 2, sourceline);
			break;

		default:
			yylval = GET_INSTRUCTION(tokentab[mid].value);
			if (class == LEX_BUILTIN || class == LEX_LENGTH)
				yylval->builtin_idx = mid;
			break;
		}
		return lasttok = class;
	}
out:
	tokkey = estrdup(tokstart, tok - tokstart);
	if (*lexptr == '(') {
		yylval = bcalloc(Op_token, 2, sourceline);
		yylval->lextok = tokkey;	
		return lasttok = FUNC_CALL;
	} else {
		static short goto_warned = FALSE;

		yylval = GET_INSTRUCTION(Op_token);
		yylval->lextok = tokkey;

#define SMART_ALECK	1
		if (SMART_ALECK && do_lint
		    && ! goto_warned && strcasecmp(tokkey, "goto") == 0) {
			goto_warned = TRUE;
			lintwarn(_("`goto' considered harmful!\n"));
		}
		return lasttok = NAME;
	}

#undef GET_INSTRUCTION
#undef NEWLINE_EOF
}

/* mk_symbol --- allocates a symbol for the symbol table. */

NODE *
mk_symbol(NODETYPE type, NODE *value)
{
	NODE *r;

	getnode(r);
	r->type = type;
	r->flags = MALLOC;
	r->lnode = value;
	r->rnode = NULL;
	r->parent_array = NULL;
	r->var_assign = (Func_ptr) 0;
	return r;
}

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

	r->builtin = tokentab[idx].ptr;

	/* check against how many args. are allowed for this builtin */
	args_allowed = tokentab[idx].flags & ARGS;
	if (args_allowed && (args_allowed & A(nexp)) == 0) {
		yyerror(_("%d is invalid as number of arguments for %s"),
				nexp, tokentab[idx].operator);
		return NULL;
	}

	/* special case processing for a few builtins */
	if (r->builtin == do_length) {
		if (nexp == 0) {		
		    /* no args. Use $0 */

			INSTRUCTION *list;
			r->expr_count = 1;			
			list = list_create(r);
			(void) list_prepend(list, instruction(Op_field_spec));
			(void) list_prepend(list, instruction(Op_push_i));
			list->nexti->memory = mk_number((AWKNUM) 0.0, (PERM|NUMCUR|NUMBER));
			return list; 
		} else {
			arg = subn->nexti;
			if (arg->nexti == arg->lasti && arg->nexti->opcode == Op_push)
				arg->nexti->opcode = Op_push_arg;	/* argument may be array */
 		}
	} else if (r->builtin == do_isarray) {
		arg = subn->nexti;
		if (arg->nexti == arg->lasti && arg->nexti->opcode == Op_push)
			arg->nexti->opcode = Op_push_arg;	/* argument may be array */
	} else if (r->builtin == do_match) {
		static short warned = FALSE;

		arg = subn->nexti->lasti->nexti;	/* 2nd arg list */
		(void) mk_rexp(arg);

		if (nexp == 3) {	/* 3rd argument there */
			if (do_lint && ! warned) {
				warned = TRUE;
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
	} else if (r->builtin == do_sub || r->builtin == do_gsub) {
		int literal = FALSE;
		
		arg = subn->nexti;		/* first arg list */
		(void) mk_rexp(arg);

		arg = arg->lasti->nexti;	/* 2nd arg list */
		if (nexp == 2) {
			INSTRUCTION *expr;
			expr = list_create(instruction(Op_push_i));
			expr->nexti->memory = mk_number((AWKNUM) 0.0, (PERM|NUMCUR|NUMBER));
			(void) mk_expression_list(subn,
					list_append(expr, instruction(Op_field_spec)));
		}

		arg = arg->lasti->nexti; 	/* third arg list */
		ip = arg->lasti;
		if (ip->opcode == Op_push_i) {
			if (do_lint)
				lintwarn(_("%s: string literal as last arg of substitute has no effect"),
					(r->builtin == do_sub) ? "sub" : "gsub");
			literal = TRUE;
		} else {
			if (make_assignable(ip) == NULL)
				yyerror(_("%s third parameter is not a changeable object"),
					(r->builtin == do_sub) ? "sub" : "gsub");
			else
				ip->do_reference = TRUE;
		}

		/* kludge: This is one of the few cases
		 * when we need to know the type of item on stack.
		 * In case of string literal as the last argument,
		 * pass 4 as # of args (See sub_common code in builtin.c).
		 * Other cases like length(array or scalar) seem
		 * to work out ok.
		 */
		
		r->expr_count = count_expressions(&subn, FALSE) + !!literal;
		ip = subn->lasti;

		(void) list_append(subn, r);

		/* add after_assign bytecode(s) */
		if (ip->opcode == Op_push_lhs && ip->memory->type == Node_var && ip->memory->var_assign) {
			(void) list_append(subn, instruction(Op_var_assign));
			subn->lasti->memory = ip->memory;
			subn->lasti->assign_var = ip->memory->var_assign;
		} else if (ip->opcode == Op_field_spec_lhs) {
			(void) list_append(subn, instruction(Op_field_assign));
			subn->lasti->field_assign = (Func_ptr) 0;
			ip->target_assign = subn->lasti;
		}
		return subn;	
	} else if (r->builtin == do_gensub) {
		if (nexp == 3) {
			arg = subn->nexti->lasti->nexti->lasti->nexti;	/* 3rd arg list */
			ip = instruction(Op_push_i);
			ip->memory = mk_number((AWKNUM) 0.0, (PERM|NUMCUR|NUMBER));
			(void) mk_expression_list(subn,
					list_append(list_create(ip),
						instruction(Op_field_spec)));
		}
		arg = subn->nexti;		/* first arg list */
		(void) mk_rexp(arg);
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
		static short warned = FALSE;
		if (nexp == 2) {
			if (do_lint && ! warned) {
				warned = TRUE;
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
			&& (subn->nexti->lasti->memory->flags & STRCUR) != 0) {	/* it's a string constant */
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
			&& (subn->nexti->lasti->memory->flags & STRCUR) != 0	/* it's a string constant */
			&& subn->nexti->lasti->nexti->lasti->opcode == Op_push_i	/* 2nd arg is constant too */
			&& (subn->nexti->lasti->nexti->lasti->memory->flags & STRCUR) != 0) {	/* it's a string constant */
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
#ifdef ARRAYDEBUG
	else if (r->builtin == do_adump) {
		ip = subn->nexti->lasti;
		if (ip->opcode == Op_push)
			ip->opcode = Op_push_array;
	}
#endif		

	if (subn != NULL) {
		r->expr_count = count_expressions(&subn, FALSE);
		return list_append(subn, r);
	}

	r->expr_count = 0;
	return list_create(r);
}

/* append_param --- append PNAME to the list of parameters
 *                  for the current function.
 */

static void
append_param(char *pname)
{
	static NODE *savetail = NULL;
	NODE *p;

	p = make_param(pname);
	if (func_params == NULL) {
		func_params = p;
		savetail = p;
	} else if (savetail != NULL) {
		savetail->rnode = p;
		savetail = p;
	}
}

/* dup_parms --- return TRUE if there are duplicate parameters */

static int
dup_parms(INSTRUCTION *fp, NODE *func)
{
	NODE *np;
	const char *fname, **names;
	int count, i, j, dups;
	NODE *params;

	if (func == NULL)	/* error earlier */
		return TRUE;

	fname = func->param;
	count = func->param_cnt;
	params = func->rnode;

	if (count == 0)		/* no args, no problem */
		return FALSE;

	if (params == NULL)	/* error earlier */
		return TRUE;

	emalloc(names, const char **, count * sizeof(char *), "dup_parms");

	i = 0;
	for (np = params; np != NULL; np = np->rnode) {
		if (np->param == NULL) { /* error earlier, give up, go home */
			efree(names);
			return TRUE;
		}
		names[i++] = np->param;
	}

	dups = 0;
	for (i = 1; i < count; i++) {
		for (j = 0; j < i; j++) {
			if (strcmp(names[i], names[j]) == 0) {
				dups++;
				error_ln(fp->source_line,
	_("function `%s': parameter #%d, `%s', duplicates parameter #%d"),
					fname, i + 1, names[j], j+1);
			}
		}
	}

	efree(names);
	return (dups > 0 ? TRUE : FALSE);
}

/* parms_shadow --- check if parameters shadow globals */

static int
parms_shadow(INSTRUCTION *pc, int *shadow)
{
	int pcount, i;
	int ret = FALSE;
	NODE *func;
	char *fname;

	func = pc->func_body;
	fname = func->lnode->param;
	
#if 0	/* can't happen, already exited if error ? */
	if (fname == NULL || func == NULL)	/* error earlier */
		return FALSE;
#endif

	pcount = func->lnode->param_cnt;

	if (pcount == 0)		/* no args, no problem */
		return 0;

	source = pc->source_file;
	sourceline = pc->source_line;
	/*
	 * Use warning() and not lintwarn() so that can warn
	 * about all shadowed parameters.
	 */
	for (i = 0; i < pcount; i++) {
		if (lookup(func->parmlist[i]) != NULL) {
			warning(
	_("function `%s': parameter `%s' shadows global variable"),
					fname, func->parmlist[i]);
			ret = TRUE;
		}
	}

	*shadow |= ret;
	return 0;
}


/*
 * install_symbol:
 * Install a name in the symbol table, even if it is already there.
 * Caller must check against redefinition if that is desired. 
 */


NODE *
install_symbol(char *name, NODE *value)
{
	NODE *hp;
	size_t len;
	int bucket;

	if (install_func)
		(*install_func)(name);

	var_count++;
	len = strlen(name);
	bucket = hash(name, len, (unsigned long) HASHSIZE, NULL);
	getnode(hp);
	hp->type = Node_hashnode;
	hp->hnext = variables[bucket];
	variables[bucket] = hp;
	hp->hlength = len;
	hp->hvalue = value;
	hp->hname = name;
	hp->hvalue->vname = name;
	return hp->hvalue;
}

/* lookup --- find the most recent hash node for name installed by install_symbol */

NODE *
lookup(const char *name)
{
	NODE *bucket;
	size_t len;

	len = strlen(name);
	for (bucket = variables[hash(name, len, (unsigned long) HASHSIZE, NULL)];
			bucket != NULL; bucket = bucket->hnext)
		if (bucket->hlength == len && STREQN(bucket->hname, name, len))
			return bucket->hvalue;
	return NULL;
}

/* sym_comp --- compare two symbol (variable or function) names */

static int
sym_comp(const void *v1, const void *v2)
{
	const NODE *const *npp1, *const *npp2;
	const NODE *n1, *n2;
	int minlen;

	npp1 = (const NODE *const *) v1;
	npp2 = (const NODE *const *) v2;
	n1 = *npp1;
	n2 = *npp2;

	if (n1->hlength > n2->hlength)
		minlen = n1->hlength;
	else
		minlen = n2->hlength;

	return strncmp(n1->hname, n2->hname, minlen);
}

/* valinfo --- dump var info */

void
valinfo(NODE *n, int (*print_func)(FILE *, const char *, ...), FILE *fp)
{
	if (n == Nnull_string)
		print_func(fp, "uninitialized scalar\n");
	else if (n->flags & STRING) {
		pp_string_fp(print_func, fp, n->stptr, n->stlen, '"', FALSE);
		print_func(fp, "\n");
	} else if (n->flags & NUMBER)
		print_func(fp, "%.17g\n", n->numbr);
	else if (n->flags & STRCUR) {
		pp_string_fp(print_func, fp, n->stptr, n->stlen, '"', FALSE);
		print_func(fp, "\n");
	} else if (n->flags & NUMCUR)
		print_func(fp, "%.17g\n", n->numbr);
	else
		print_func(fp, "?? flags %s\n", flags2str(n->flags));
}

/* get_varlist --- list of global variables */

NODE **
get_varlist()
{
	int i, j;
	NODE **table;
	NODE *p;

	emalloc(table, NODE **, (var_count + 1) * sizeof(NODE *), "get_varlist");
	update_global_values();
	for (i = j = 0; i < HASHSIZE; i++)
		for (p = variables[i]; p != NULL; p = p->hnext)
			table[j++] = p;
	assert(j == var_count);

	/* Shazzam! */
	qsort(table, j, sizeof(NODE *), sym_comp);

	table[j] = NULL;
	return table;
}

/* print_vars --- print names and values of global variables */ 

void
print_vars(int (*print_func)(FILE *, const char *, ...), FILE *fp)
{
	int i;
	NODE **table;
	NODE *p;

	table = get_varlist();
	for (i = 0; (p = table[i]) != NULL; i++) {
		if (p->hvalue->type == Node_func)
			continue;
		print_func(fp, "%.*s: ", (int) p->hlength, p->hname);
		if (p->hvalue->type == Node_var_array)
			print_func(fp, "array, %ld elements\n", p->hvalue->table_size);
		else if (p->hvalue->type == Node_var_new)
			print_func(fp, "untyped variable\n");
		else if (p->hvalue->type == Node_var)
			valinfo(p->hvalue->var_value, print_func, fp);
	}
	efree(table);
}

/* dump_vars --- dump the symbol table */

void
dump_vars(const char *fname)
{
	FILE *fp;

	if (fname == NULL)
		fp = stderr;
	else if ((fp = fopen(fname, "w")) == NULL) {
		warning(_("could not open `%s' for writing (%s)"), fname, strerror(errno));
		warning(_("sending variable list to standard error"));
		fp = stderr;
	}

	print_vars(fprintf, fp);
	if (fp != stderr && fclose(fp) != 0)
		warning(_("%s: close failed (%s)"), fname, strerror(errno));
}

/* release_all_vars --- free all variable memory */

void
release_all_vars()
{
	int i;
	NODE *p, *next;
	
	for (i = 0; i < HASHSIZE; i++) {
		for (p = variables[i]; p != NULL; p = next) {
			next = p->hnext;

			if (p->hvalue->type == Node_func)
				continue;
			else if (p->hvalue->type == Node_var_array)
				assoc_clear(p->hvalue);
			else if (p->hvalue->type != Node_var_new)
				unref(p->hvalue->var_value);

			efree(p->hname);
			freenode(p->hvalue);
			freenode(p);
		}
	}                                                                    
}

/* dump_funcs --- print all functions */

void
dump_funcs()
{
	if (func_count <= 0)
		return;

	(void) foreach_func((int (*)(INSTRUCTION *, void *)) pp_func, TRUE, (void *) 0);
}

/* shadow_funcs --- check all functions for parameters that shadow globals */

void
shadow_funcs()
{
	static int calls = 0;
	int shadow = FALSE;

	if (func_count <= 0)
		return;

	if (calls++ != 0)
		fatal(_("shadow_funcs() called twice!"));

	(void) foreach_func((int (*)(INSTRUCTION *, void *)) parms_shadow, TRUE, &shadow);

	/* End with fatal if the user requested it.  */
	if (shadow && lintfunc != warning)
		lintwarn(_("there were shadowed variables."));
}

/*
 * func_install:
 * check if name is already installed;  if so, it had better have Null value,
 * in which case def is added as the value. Otherwise, install name with def
 * as value. 
 *
 * Extra work, build up and save a list of the parameter names in a table
 * and hang it off params->parmlist. This is used to set the `vname' field
 * of each function parameter during a function call. See eval.c.
 */

static int
func_install(INSTRUCTION *func, INSTRUCTION *def)
{
	NODE *params;
	NODE *r, *n, *thisfunc, *hp;
	char **pnames = NULL;
	char *fname;
	int pcount = 0;
	int i;

	params = func_params;

	/* check for function foo(foo) { ... }.  bleah. */
	for (n = params->rnode; n != NULL; n = n->rnode) {
		if (strcmp(n->param, params->param) == 0) {
			error_ln(func->source_line,
				_("function `%s': can't use function name as parameter name"), params->param);
			return -1;
		} else if (is_std_var(n->param)) {
			error_ln(func->source_line,
				_("function `%s': can't use special variable `%s' as a function parameter"),
					params->param, n->param);
			return -1;
		}
	}

	thisfunc = NULL;        /* turn off warnings */

	fname = params->param;
	/* symbol table management */
	hp = remove_symbol(params->param);  /* remove function name out of symbol table */ 
	if (hp != NULL)
		freenode(hp);
	r = lookup(fname);
	if (r != NULL) {
		error_ln(func->source_line,
			 _("function name `%s' previously defined"), fname);
		return -1;
	} else if (fname == builtin_func)	/* not a valid function name */
		goto remove_params;

	/* add an implicit return at end;
	 * also used by 'return' command in debugger
	 */
      
	(void) list_append(def, instruction(Op_push_i));
	def->lasti->memory = Nnull_string;
	(void) list_append(def, instruction(Op_K_return));

	if (do_profiling)
		(void) list_prepend(def, instruction(Op_exec_count));

	/* func->opcode is Op_func */
	(func + 1)->firsti = def->nexti;
	(func + 1)->lasti = def->lasti;
	(func + 2)->first_line = func->source_line;
	(func + 2)->last_line = lastline;

	func->nexti = def->nexti;
	bcfree(def);

	(void) list_append(rule_list, func + 1);	/* debugging */

	/* install the function */
	thisfunc = mk_symbol(Node_func, params);
	(void) install_symbol(fname, thisfunc);
	thisfunc->code_ptr = func;
	func->func_body = thisfunc;

	for (n = params->rnode; n != NULL; n = n->rnode)
		pcount++;

	if (pcount != 0) {
		emalloc(pnames, char **, (pcount + 1) * sizeof(char *), "func_install");
		for (i = 0, n = params->rnode; i < pcount; i++, n = n->rnode)
			pnames[i] = n->param;
		pnames[pcount] = NULL;
	}
	thisfunc->parmlist = pnames;

	/* update lint table info */
	func_use(fname, FUNC_DEFINE);

	func_count++;	/* used in profiler / pretty printer */

remove_params:
	/* remove params from symbol table */
	pop_params(params->rnode);
	return 0;
}

/* remove_symbol --- remove a variable from the symbol table */

NODE *
remove_symbol(char *name)
{
	NODE *bucket, **save;
	size_t len;

	len = strlen(name);
	save = &(variables[hash(name, len, (unsigned long) HASHSIZE, NULL)]);
	for (bucket = *save; bucket != NULL; bucket = bucket->hnext) {
		if (len == bucket->hlength && STREQN(bucket->hname, name, len)) {
			var_count--;
			*save = bucket->hnext;
			return bucket;
		}
		save = &(bucket->hnext);
	}
	return NULL;
}

/* pop_params --- remove list of function parameters from symbol table */

/*
 * pop parameters out of the symbol table. do this in reverse order to
 * avoid reading freed memory if there were duplicated parameters.
 */
static void
pop_params(NODE *params)
{
	NODE *hp;
	if (params == NULL)
		return;
	pop_params(params->rnode);
	hp = remove_symbol(params->param);
	if (hp != NULL)
		freenode(hp);
}

/* make_param --- make NAME into a function parameter */

static NODE *
make_param(char *name)
{
	NODE *r;

	getnode(r);
	r->type = Node_param_list;
	r->rnode = NULL;
	r->param_cnt = param_counter++;
	return (install_symbol(name, r));
}

static struct fdesc {
	char *name;
	short used;
	short defined;
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

	for (fp = ftable[ind]; fp != NULL; fp = fp->next) {
		if (strcmp(fp->name, name) == 0) {
			if (how == FUNC_DEFINE)
				fp->defined++;
			else
				fp->used++;
			return;
		}
	}

	/* not in the table, fall through to allocate a new one */

	emalloc(fp, struct fdesc *, sizeof(struct fdesc), "func_use");
	memset(fp, '\0', sizeof(struct fdesc));
	emalloc(fp->name, char *, len + 1, "func_use");
	strcpy(fp->name, name);
	if (how == FUNC_DEFINE)
		fp->defined++;
	else
		fp->used++;
	fp->next = ftable[ind];
	ftable[ind] = fp;
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
#ifdef REALLYMEAN
			/* making this the default breaks old code. sigh. */
			if (fp->defined == 0) {
				error(
		_("function `%s' called but never defined"), fp->name);
				errcount++;
			}
#else
			if (do_lint && fp->defined == 0)
				lintwarn(
		_("function `%s' called but never defined"), fp->name);
#endif
			if (do_lint && fp->used == 0) {
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

/* foreach_func --- execute given function for each awk function in symbol table. */

int
foreach_func(int (*pfunc)(INSTRUCTION *, void *), int sort, void *data)
{
	int i, j;
	NODE *p;
	int ret = 0;

	if (sort) {
		NODE **tab;

		/*
		 * Walk through symbol table counting functions.
		 * Could be more than func_count if there are
		 * extension functions.
		 */
		for (i = j = 0; i < HASHSIZE; i++) {
			for (p = variables[i]; p != NULL; p = p->hnext) {
				if (p->hvalue->type == Node_func) {
					j++;
				}
			}
		}

		if (j == 0)
			return 0;

		emalloc(tab, NODE **, j * sizeof(NODE *), "foreach_func");

		/* now walk again, copying info */
		for (i = j = 0; i < HASHSIZE; i++) {
			for (p = variables[i]; p != NULL; p = p->hnext) {
				if (p->hvalue->type == Node_func) {
					tab[j] = p;
					j++;
				}
			}
		}

		/* Shazzam! */
		qsort(tab, j, sizeof(NODE *), sym_comp);

		for (i = 0; i < j; i++) {
			if ((ret = pfunc(tab[i]->hvalue->code_ptr, data)) != 0)
				break;
		}

		efree(tab);
		return ret;
	}

	/* unsorted */
	for (i = 0; i < HASHSIZE; i++) {
		for (p = variables[i]; p != NULL; p = p->hnext) {
			if (p->hvalue->type == Node_func
					&& (ret = pfunc(p->hvalue->code_ptr, data)) != 0)
				return ret;
		}
	}
	return 0;
}

/* deferred variables --- those that are only defined if needed. */

/*
 * Is there any reason to use a hash table for deferred variables?  At the
 * moment, there are only 1 to 3 such variables, so it may not be worth
 * the overhead.  If more modules start using this facility, it should
 * probably be converted into a hash table.
 */

static struct deferred_variable {
	NODE *(*load_func)(void);
	struct deferred_variable *next;
	char name[1];	/* variable-length array */
} *deferred_variables;

/* register_deferred_variable --- add a var name and loading function to the list */

void
register_deferred_variable(const char *name, NODE *(*load_func)(void))
{
	struct deferred_variable *dv;
	size_t sl = strlen(name);

	emalloc(dv, struct deferred_variable *, sizeof(*dv)+sl,
		"register_deferred_variable");
	dv->load_func = load_func;
	dv->next = deferred_variables;
	memcpy(dv->name, name, sl+1);
	deferred_variables = dv;
}

/* variable --- make sure NAME is in the symbol table */

NODE *
variable(char *name, NODETYPE type)
{
	NODE *r;

	if ((r = lookup(name)) != NULL) {
		if (r->type == Node_func) {
			error(_("function `%s' called with space between name and `(',\nor used as a variable or an array"),
				r->vname);
			errcount++;
			r->type = Node_var_new; /* continue parsing instead of exiting */
		}
	} else {
		/* not found */
		struct deferred_variable *dv;

		for (dv = deferred_variables; TRUE; dv = dv->next) {
			if (dv == NULL) {
			/*
			 * This is the only case in which we may not free the string.
			 */
				if (type == Node_var)
					r = mk_symbol(type, Nnull_string);
				else
					r = mk_symbol(type, (NODE *) NULL);
				return install_symbol(name, r);
			}
			if (STREQ(name, dv->name)) {
				r = (*dv->load_func)();
				break;
			}
		}
	}
	efree(name);
	return r;
}

/* make_regnode --- make a regular expression node */

static NODE *
make_regnode(int type, NODE *exp)
{
	NODE *n;

	getnode(n);
	memset(n, 0, sizeof(NODE));
	n->type = type;
	n->re_cnt = 1;

	if (type == Node_regex) {
		n->re_reg = make_regexp(exp->stptr, exp->stlen, FALSE, TRUE, FALSE);
		if (n->re_reg == NULL) {
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
	else {
		ip = instruction(Op_push_re);
		ip->memory = make_regnode(Node_dynregex, NULL);
		ip->nexti = list->lasti->nexti;
		list->lasti->nexti = ip;
		list->lasti = ip;
	}
	return ip->memory;
}

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
		return TRUE;
	default:
		break;	/* keeps gcc -Wall happy */
	}

	return FALSE;
}

/* make_assignable --- make this operand an assignable one if posiible */

static INSTRUCTION *
make_assignable(INSTRUCTION *ip)
{
	switch (ip->opcode) {
	case Op_push:
		if (ip->memory->type == Node_param_list
				&& (ip->memory->flags & FUNC) != 0)
			return NULL;
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
	return (NODE *) 0;
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
	pp_string_fp(fprintf, stdout, str, len, '"', TRUE);
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
	pp_string_fp(fprintf, stdout, str1, len1, '"', TRUE);
	putchar('\n');
	printf("msgid_plural ");
	pp_string_fp(fprintf, stdout, str2, len2, '"', TRUE);
	putchar('\n');
	printf("msgstr[0] \"\"\nmsgstr[1] \"\"\n\n");
	fflush(stdout);
}

/* isarray --- can this type be subscripted? */

static int
isarray(NODE *n)
{
	switch (n->type) {
	case Node_var_new:
	case Node_var_array:
		return TRUE;
	case Node_param_list:
		return (n->flags & FUNC) == 0;
	case Node_array_ref:
		cant_happen();
		break;
	default:
		break;	/* keeps gcc -Wall happy */
	}

	return FALSE;
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
		if (do_optimize > 1
				&& ip1 == s1->lasti && ip1->opcode == Op_push_i
				&& (ip1->memory->flags & (STRCUR|STRING)) == 0
				&& (ip2->memory->flags & (STRCUR|STRING)) == 0
		) {
			NODE *n1 = ip1->memory, *n2 = ip2->memory;
			res = force_number(n1);
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
			op->memory = mk_number(res, (PERM|NUMCUR|NUMBER));
			n1->flags &= ~PERM;
			n1->flags |= MALLOC;
			n2->flags &= ~PERM;
			n2->flags |= MALLOC;
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

	if (false_branch == NULL) {
		false_branch = list_create(instruction(Op_no_op));
		if (elsep != NULL) {		/* else { } */
			if (do_profiling)
				(void) list_prepend(false_branch, elsep);
			else
				bcfree(elsep);
		}
	} else {
		/* assert(elsep != NULL); */

		/* avoid a series of no_op's: if .. else if .. else if .. */
		if (false_branch->lasti->opcode != Op_no_op)
			(void) list_append(false_branch, instruction(Op_no_op));
		if (do_profiling) {
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

	if (do_profiling) {
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
		if (do_profiling)
			(void) list_append(action, instruction(Op_no_op));
		(rp + 1)->firsti = action->nexti;
		(rp + 1)->lasti = action->lasti;
		(rp + 2)->first_line = pattern->source_line;
		(rp + 2)->last_line = lastline;
		ip = list_prepend(action, rp);

	} else {
		rp = bcalloc(Op_rule, 3, 0);
		rp->in_rule = Rule;
		rp->source_file = source;
		tp = instruction(Op_no_op);

		if (pattern == NULL) {
			/* assert(action != NULL); */
			if (do_profiling)
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
				if (do_profiling)
					(void) list_prepend(action, instruction(Op_exec_count));
			} else
				(rp + 2)->last_line = lastline;

			if (do_profiling) {
				(void) list_prepend(pattern, instruction(Op_exec_count));
				(void) list_prepend(action, instruction(Op_exec_count));
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
	default:
		cant_happen();
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
		tp->do_reference = FALSE; /* no uninitialized reference checking
		                           * for a special variable.
		                           */
		(void) list_append(ip, instruction(Op_var_assign));
		ip->lasti->memory = tp->memory;
		ip->lasti->assign_var = tp->memory->var_assign;
	} else if (tp->opcode == Op_field_spec_lhs) {
		(void) list_append(ip, instruction(Op_field_assign));
		ip->lasti->field_assign = (Func_ptr) 0;
		tp->target_assign = ip->lasti;
	}

	return ip;
}

/* optimize_assignment --- peephole optimization for assignment */

static INSTRUCTION *
optimize_assignment(INSTRUCTION *exp)
{
	INSTRUCTION *i1;
	INSTRUCTION *i2;
	INSTRUCTION *i3;

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

	if (   ! do_optimize
	    || (   i1->opcode != Op_assign
		&& i1->opcode != Op_field_assign)
	)
		return list_append(exp, instruction(Op_pop));

	for (i2 = exp->nexti; i2 != i1; i2 = i2->nexti) {
		switch (i2->opcode) {
		case Op_concat:
			if (i2->nexti->opcode == Op_push_lhs    /* l.h.s is a simple variable */
				&& (i2->concat_flag & CSVAR)        /* 1st exp in r.h.s is a simple variable;
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
			asgn->memory = tp->memory;
			asgn->assign_var = tp->memory->var_assign;
		} else if (tp->opcode == Op_field_spec_lhs) {
			asgn = instruction(Op_field_assign);
			asgn->field_assign = (Func_ptr) 0;   /* determined at run time */
			tp->target_assign = asgn;
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
	op->redir_type = (redir != NULL) ? redirtype : 0;

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

	if (do_profiling) {
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

	if (do_profiling) {
		forp->target_break = tbreak;
		forp->target_continue = tcont;
		ret = list_prepend(ret, forp);
	} /* else
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
			for (ip = list->nexti; ip->nexti != list->lasti; ip = ip->nexti)
				;

			if (do_lint) {		/* compile-time warning */
				if (isnoeffect(ip->opcode))
					lintwarn_ln(ip->source_line, ("statement may have no effect"));
			}

			if (ip->opcode == Op_push) {		/* run-time warning */
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
count_expressions(INSTRUCTION **list, int isarg)
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


/* append_symbol --- append symbol to the list of symbols
 *                  installed in the symbol table.
 */

void
append_symbol(char *name)
{
	NODE *hp;

	/* N.B.: func_install removes func name and reinstalls it;
	 * and we get two entries for it here!. destroy_symbol()
	 * will find and destroy the Node_func which is what we want.
	 */

	getnode(hp);
	hp->hname = name;	/* shallow copy */
	hp->hnext = symbol_list->hnext;
	symbol_list->hnext = hp;
}

/* release_symbol --- free symbol list and optionally remove symbol from symbol table */

void
release_symbols(NODE *symlist, int keep_globals)
{
	NODE *hp, *n;

	for (hp = symlist->hnext; hp != NULL; hp = n) {
		if (! keep_globals) {
			/* destroys globals, function, and params
			 * if still in symbol table and not removed by func_install
			 * due to parse error.
			 */
			destroy_symbol(hp->hname);
		}
		n = hp->hnext;
		freenode(hp);
	}
	symlist->hnext = NULL;
}

/* destroy_symbol --- remove a symbol from symbol table
*                     and free all associated memory.
*/

void
destroy_symbol(char *name)
{
	NODE *symbol, *hp;

	symbol = lookup(name);
	if (symbol == NULL)
		return;

	if (symbol->type == Node_func) {
		char **varnames;
		NODE *func, *n;
				
		func = symbol;
		varnames = func->parmlist;
		if (varnames != NULL)
			efree(varnames);

		/* function parameters of type Node_param_list */				
		for (n = func->lnode->rnode; n != NULL; ) {
			NODE *np;
			np = n->rnode;
			efree(n->param);
			freenode(n);
			n = np;
		}		
		freenode(func->lnode);
		func_count--;

	} else if (symbol->type == Node_var_array)
		assoc_clear(symbol);
	else if (symbol->type == Node_var) 
		unref(symbol->var_value);

	/* remove from symbol table */
	hp = remove_symbol(name);
	efree(hp->hname);
	freenode(hp->hvalue);
	freenode(hp);
}

#define pool_size	d.dl
#define freei		x.xi
static INSTRUCTION *pool_list;
static AWK_CONTEXT *curr_ctxt = NULL;

/* new_context --- create a new execution context. */

AWK_CONTEXT *
new_context()
{
	AWK_CONTEXT *ctxt;

	emalloc(ctxt, AWK_CONTEXT *, sizeof(AWK_CONTEXT), "new_context");
	memset(ctxt, 0, sizeof(AWK_CONTEXT));
	ctxt->srcfiles.next = ctxt->srcfiles.prev = &ctxt->srcfiles;
	ctxt->rule_list.opcode = Op_list;
	ctxt->rule_list.lasti = &ctxt->rule_list;
	return ctxt;
}

/* set_context --- change current execution context. */

static void
set_context(AWK_CONTEXT *ctxt)
{
	pool_list = &ctxt->pools;
	symbol_list = &ctxt->symbols;
	srcfiles = &ctxt->srcfiles;
	rule_list = &ctxt->rule_list;
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
}

/* pop_context --- switch to previous execution context. */ 

void
pop_context()
{
	AWK_CONTEXT *ctxt;

	assert(curr_ctxt != NULL);
	ctxt = curr_ctxt->prev;
	/* restore source and sourceline */
	sourceline = ctxt->sourceline;
	source = ctxt->source;
	set_context(ctxt);
}

/* in_main_context --- are we in the main context ? */

int
in_main_context()
{
	assert(curr_ctxt != NULL);
	return (curr_ctxt->prev == NULL);
}

/* free_context --- free context structure and related data. */ 

void
free_context(AWK_CONTEXT *ctxt, int keep_globals)
{
	SRCFILE *s, *sn;

	if (ctxt == NULL)
		return;

	assert(curr_ctxt != ctxt);

 	/* free all code including function codes */
	free_bcpool(&ctxt->pools);
	/* free symbols */
	release_symbols(&ctxt->symbols, keep_globals);
	/* free srcfiles */
	for (s = &ctxt->srcfiles; s != &ctxt->srcfiles; s = sn) {
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
		if (cp->func_name != NULL
				&& cp->func_name != builtin_func
		)
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
	case Op_token:	/* token lost during error recovery in yyparse */
		if (cp->lextok != NULL)
			efree(cp->lextok);
		break;
	case Op_illegal:
		cant_happen();
	default:
		break;	
	}
}


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
#if 'a' == 0x81 /* it's EBCDIC */
	static int did_sort = FALSE;

	if (! did_sort) {
		qsort((void *) tokentab,
				sizeof(tokentab) / sizeof(tokentab[0]),
				sizeof(tokentab[0]), tokcompare);
		did_sort = TRUE;
	}
#endif

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
			if ((do_traditional && (tokentab[mid].flags & GAWKX))
					|| (do_posix && (tokentab[mid].flags & NOT_POSIX)))
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

