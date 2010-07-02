/*
 * gawk -- GNU version of awk
 * Copyright (C) 1986 Free Software Foundation
 *   Written by Paul Rubin, August 1986
 *
 * $Log:	awk.y,v $
 * Revision 1.30  89/03/24  15:52:15  david
 * add getline production to rexp
 * merge HASHNODE with NODE
 * 
 * Revision 1.29  89/03/21  11:57:49  david
 * substantial cleanup and code movement from awk1.c
 * this and previous two changes represent a major reworking of the grammar
 * to fix a number of bugs;  two general problems were in I/O redirection
 * specifications and in the handling of whitespace -- the general strategies
 * in fixing these problems were to define some more specific grammatical 
 * elements (e.g. simp_exp and rexp) and use these in particular places; 
 * also got rid of want_concat and want_redirect kludges
 * 
 * Revision 1.28  89/03/15  21:58:01  david
 * more grammar changes (explanation to come) plus changes from Arnold:
 * new case stuff added and old removed
 * tolower and toupper added
 * fix vararg stuff
 * add new escape sequences
 * fix bug in reporting unterminated regexps
 * fix to allow -f -
 * /dev/fd/N etc special files added
 * 
 * Revision 1.27  89/03/02  21:10:09  david
 * intermediate step in major revision -- description later
 * 
 * Revision 1.26  89/01/18  20:39:58  david
 * allow regexp && regexp as pattern and get rid of remaining reduce/reduce conflicts
 * 
 * Revision 1.25  89/01/04  21:53:21  david
 * purge obstack remnants
 * 
 * Revision 1.24  88/12/15  12:52:58  david
 * changes from Jay to get rid of some reduce/reduce conflicts - some remain
 * 
 * Revision 1.23  88/12/07  19:59:25  david
 * changes for incorporating source filename in error messages
 * 
 * Revision 1.22  88/11/23  21:37:24  david
 * Arnold: refinements of AWKPATH code
 * 
 * Revision 1.21  88/11/22  13:46:45  david
 * Arnold: changes for case-insensitive matching
 * 
 * Revision 1.20  88/11/15  10:13:37  david
 * Arnold: allow multiple -f options and search in directories for awk libraries,
 * directories specified by AWKPATH env. variable; cleanupo of comments and
 * #includes
 * 
 * Revision 1.19  88/11/14  21:51:30  david
 * Arnold: added error message for BEGIN or END without any action at all;
 * unlink temporary source file right after creation so it goes away on bomb
 * 
 * Revision 1.18  88/10/19  22:00:56  david
 * generalize (and correct) what pattern can be in pattern {action}; this
 * introduces quite a few new conflicts that should be checked thoroughly
 * at some point, but they don't seem to do any harm at first glance
 * replace malloc with emalloc
 * 
 * Revision 1.17  88/10/17  19:52:01  david
 * Arnold: cleanup, purge FAST
 * 
 * Revision 1.16  88/10/13  22:02:16  david
 * cleanup of yyerror and other error messages
 * 
 * Revision 1.15  88/10/06  23:24:57  david
 * accept     var space ++var
 * accept underscore as first character of a variable name
 * 
 * Revision 1.14  88/06/13  18:01:46  david
 * delete \a (change from Arnold)
 * 
 * Revision 1.13  88/06/08  00:29:42  david
 * better attempt at keeping track of line numbers
 * change grammar to properly handle newlines after && or ||
 * 
 * Revision 1.12  88/06/07  23:39:02  david
 * little delint
 * 
 * Revision 1.11  88/06/05  22:17:40  david
 * make_name() becomes make_param() (again!)
 * func_level goes away, param_counter makes entrance
 * 
 * Revision 1.10  88/05/30  09:49:02  david
 * obstack_free was being called at end of function definition, freeing
 * memory that might be part of global variables referenced only inside
 * functions; commented out for now, will have to selectively free later.
 * cleanup: regexp now returns a NODE *
 * 
 * Revision 1.9  88/05/27  11:04:53  david
 * added print[f] '(' ... ')'     (optional parentheses)
 * for some reason want_redirect wasn't getting set for PRINT, so I set it in 
 * yylex()
 * 
 * Revision 1.8  88/05/26  22:52:14  david
 * fixed cmd | getline
 * added compound patterns (they got lost somewhere along the line)
 * fixed error message in yylex()
 * added null statement 
 * 
 * Revision 1.7  88/05/13  22:05:29  david
 * moved BEGIN and END block merging here
 * BEGIN, END and function defs. are no longer incorporated into main parse tree
 * fixed    command | getline
 * fixed function install and definition
 * 
 * Revision 1.6  88/05/09  17:47:50  david
 * Arnold's coded binary search
 * 
 * Revision 1.5  88/05/04  12:31:13  david
 * be a bit more careful about types
 * make_for_loop() now returns a NODE *
 * keyword search now uses bsearch() -- need a public domain version of this
 * added back stuff in yylex() that got lost somewhere along the line
 * malloc() tokens in yylex() since they were previously just pointers into
 *  current line that got overwritten by the next fgets() -- these need to get
 *  freed at some point
 * fixed backslash line continuation interaction with CONCAT
 * 
 * Revision 1.4  88/04/14  17:03:51  david
 * reinstalled a fix to do with line continuation
 * 
 * Revision 1.3  88/04/14  14:41:01  david
 * Arnold's changes to yylex to read program from a file
 * 
 * Revision 1.5  88/03/18  21:00:07  david
 * Baseline -- hoefully all the functionality of the new awk added.
 * Just debugging and tuning to do.
 * 
 * Revision 1.4  87/11/19  14:37:20  david
 * added a bunch of ew builtin functions
 * added new rules for getline to provide new functionality
 * minor cleanup of redirection handling
 * generalized make_param into make_name
 * 
 * Revision 1.3  87/11/09  21:22:33  david
 * added macinery for user-defined functions (including return)
 * added delete, do-while and system
 * reformatted and revised grammer to improve error-handling
 * changes to yyerror to give improved error messages
 * 
 * Revision 1.2  87/10/29  21:33:28  david
 * added test for membership in an array, as in:  if ("yes" in answers) ...
 * 
 * Revision 1.1  87/10/27  15:23:21  david
 * Initial revision
 * 
 */

/*
GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GAWK General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GAWK,
but only under the conditions described in the GAWK General Public
License.  A copy of this license is supposed to have been given to you
along with GAWK so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GAWK, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

%{
#define YYDEBUG 12
#define YYIMPROVE

#include "awk.h"

extern void msg();
extern struct re_pattern_buffer *mk_re_parse();

NODE *node();
NODE *lookup();
NODE *install();

static NODE *snode();
static NODE *mkrangenode();
static FILE *pathopen();
static NODE *make_for_loop();
static NODE *append_right();
static void func_install();
static NODE *make_param();
static int hashf();
static void pop_params();
static void pop_var();
static int yylex ();
static void yyerror();

static int want_regexp;		/* lexical scanning kludge */
static int lineno = 1;		/* for error msgs */
static char *lexptr;		/* pointer to next char during parsing */
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static int curinfile = -1;	/* index into sourcefiles[] */

extern int errcount;
extern NODE *begin_block;
extern NODE *end_block;
extern int param_counter;
%}

%union {
	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval)();
}

%type <nodeval> function_prologue function_body
%type <nodeval> rexp exp start program rule simp_exp
%type <nodeval> simp_pattern pattern 
%type <nodeval>	action variable param_list
%type <nodeval>	rexpression_list opt_rexpression_list
%type <nodeval>	expression_list opt_expression_list
%type <nodeval>	statements statement if_statement opt_param_list 
%type <nodeval> opt_exp opt_variable regexp p_regexp
%type <nodeval> input_redir output_redir
%type <nodetypeval> r_paren comma nls opt_nls print

%type <sval> func_name
%token <sval> FUNC_CALL NAME REGEXP YSTRING
%token <lval> ERROR INCDEC
%token <fval> NUMBER
%token <nodetypeval> RELOP APPEND_OP
%token <nodetypeval> ASSIGNOP MATCHOP NEWLINE CONCAT_OP
%token <nodetypeval> LEX_BEGIN LEX_END LEX_IF LEX_ELSE LEX_RETURN LEX_DELETE
%token <nodetypeval> LEX_WHILE LEX_DO LEX_FOR LEX_BREAK LEX_CONTINUE
%token <nodetypeval> LEX_PRINT LEX_PRINTF LEX_NEXT LEX_EXIT LEX_FUNCTION
%token <nodetypeval> LEX_GETLINE LEX_SUB LEX_MATCH
%token <nodetypeval> LEX_IN
%token <lval> LEX_AND LEX_OR INCREMENT DECREMENT
%token <ptrval> LEX_BUILTIN

/* these are just yylval numbers */

/* Lowest to highest */
%right ASSIGNOP
%right '?' ':'
%left LEX_OR
%left LEX_AND
%left LEX_GETLINE
%left NUMBER
%left FUNC_CALL LEX_SUB LEX_BUILTIN LEX_MATCH
%nonassoc MATCHOP
%nonassoc RELOP '<' '>' '|' APPEND_OP
%left NAME
%nonassoc LEX_IN
%left YSTRING
%left '(' ')'
%left CONCAT_OP
%left '+' '-'
%left '*' '/' '%'
%right '!' UNARY
%right '^'
%left INCREMENT DECREMENT
%left '$'

%%

start
	: opt_nls program
		{ expression_value = $2; }
	;

program
	: rule
		{ 
			if ($1 != NULL)
				$$ = node ($1, Node_rule_list,(NODE *) NULL);
			else
				$$ = NULL;
			yyerrok;
		}
	| program rule
		/* add the rule to the tail of list */
		{
			if ($2 == NULL)
				$$ = $1;
			else if ($1 == NULL)
				$$ = node($2, Node_rule_list,(NODE *) NULL);
			else
				$$ = append_right ($1,
				   node($2, Node_rule_list,(NODE *) NULL));
			yyerrok;
		}
	| error	{ $$ = NULL; }
	| program error
	;

rule
	: LEX_BEGIN action
	  {
		if (begin_block)
			append_right (begin_block, node(
			    node((NODE *)NULL, Node_rule_node, $2),
			    Node_rule_list, (NODE *)NULL) );
		else
			begin_block = node(node((NODE *)NULL,Node_rule_node,$2),
			    Node_rule_list, (NODE *)NULL);
		$$ = NULL;
		yyerrok;
	  }
	| LEX_END action
	  {
		if (end_block)
			append_right (end_block, node(
			    node((NODE *)NULL, Node_rule_node, $2),
			    Node_rule_list, (NODE *)NULL));
		else
			end_block = node(node((NODE *)NULL, Node_rule_node, $2),
			    Node_rule_list, (NODE *)NULL);
		$$ = NULL;
		yyerrok;
	  }
	| LEX_BEGIN statement_term
	  {
		msg ("error near line %d: BEGIN blocks must have an action part", lineno);
		errcount++;
		yyerrok;
	  }
	| LEX_END statement_term
	  {
		msg ("error near line %d: END blocks must have an action part", lineno);
		errcount++;
		yyerrok;
	  }
	| pattern action
		{ $$ = node ($1, Node_rule_node, $2); yyerrok; }
	| action
		{ $$ = node ((NODE *)NULL, Node_rule_node, $1); yyerrok; }
	| pattern statement_term
		{ if($1) $$ = node ($1, Node_rule_node, (NODE *)NULL); yyerrok; }
	| function_prologue function_body
		{
			func_install($1, $2);
			$$ = NULL;
			yyerrok;
		}
	;

func_name
	: NAME
	| FUNC_CALL
	;
		
function_prologue
	: LEX_FUNCTION 
		{
			param_counter = 0;
		}
	  func_name '(' opt_param_list r_paren
		{
			$$ = append_right(make_param($3), $5);
		}
	;

function_body
	: l_brace statements r_brace
		{ $$ = $2; }
	;


simp_pattern
	: exp
	| p_regexp
	| p_regexp LEX_AND simp_pattern
		{ $$ = node ($1, Node_and, $3); }
	| p_regexp LEX_OR simp_pattern
		{ $$ = node ($1, Node_or, $3); }
	| '!' p_regexp %prec UNARY
		{ $$ = node ($2, Node_not,(NODE *) NULL); }
	| '(' p_regexp r_paren
		{ $$ = $2; }
	;

pattern
	: simp_pattern
	| simp_pattern comma simp_pattern
		{ $$ = mkrangenode ( node($1, Node_cond_pair, $3) ); }
	;

p_regexp
	: regexp
		{ 
		  $$ = node(
		       node(make_number((AWKNUM)0),Node_field_spec,(NODE*)NULL),
		       Node_match, $1);
		}
	;

regexp
	/*
	 * In this rule, want_regexp tells yylex that the next thing
	 * is a regexp so it should read up to the closing slash.
	 */
	: '/'
		{ ++want_regexp; }
	   REGEXP '/'
		{
		  want_regexp = 0;
		  $$ = node((NODE *)NULL,Node_regex,(NODE *)mk_re_parse($3, 0));
		  $$ -> re_case = 0;
		  emalloc ($$ -> re_text, char *, strlen($3)+1, "regexp");
		  strcpy ($$ -> re_text, $3);
		}
	;

action
	: l_brace r_brace opt_semi
		{
			/* empty actions are different from missing actions */
			$$ = node ((NODE *) NULL, Node_illegal, (NODE *) NULL);
		}
	| l_brace statements r_brace opt_semi
		{ $$ = $2 ; }
	;

statements
	: statement
		{ $$ = node ($1, Node_statement_list, (NODE *)NULL); }
	| statements statement
		{
	    		$$ = append_right($1,
				node( $2, Node_statement_list, (NODE *)NULL));
	    		yyerrok;
		}
	| error
		{ $$ = NULL; }
	| statements error
	;

statement_term
	: nls
		{ $<nodetypeval>$ = Node_illegal; }
	| semi opt_nls
		{ $<nodetypeval>$ = Node_illegal; }
	;

	
statement
	: semi opt_nls
		{ $$ = NULL; }
	| l_brace statements r_brace
		{ $$ = $2; }
	| if_statement
		{ $$ = $1; }
	| LEX_WHILE '(' exp r_paren opt_nls statement
		{ $$ = node ($3, Node_K_while, $6); }
	| LEX_DO opt_nls statement LEX_WHILE '(' exp r_paren opt_nls
		{ $$ = node ($6, Node_K_do, $3); }
	| LEX_FOR '(' NAME LEX_IN NAME r_paren opt_nls statement
	  {
		$$ = node ($8, Node_K_arrayfor, make_for_loop(variable($3),
			(NODE *)NULL, variable($5)));
	  }
	| LEX_FOR '(' opt_exp semi exp semi opt_exp r_paren opt_nls statement
	  {
		$$ = node($10, Node_K_for, (NODE *)make_for_loop($3, $5, $7));
	  }
	| LEX_FOR '(' opt_exp semi semi opt_exp r_paren opt_nls statement
	  {
		$$ = node ($9, Node_K_for,
			(NODE *)make_for_loop($3, (NODE *)NULL, $6));
	  }
	| LEX_BREAK statement_term
	   /* for break, maybe we'll have to remember where to break to */
		{ $$ = node ((NODE *)NULL, Node_K_break, (NODE *)NULL); }
	| LEX_CONTINUE statement_term
	   /* similarly */
		{ $$ = node ((NODE *)NULL, Node_K_continue, (NODE *)NULL); }
	| print '(' expression_list r_paren output_redir statement_term
		{ $$ = node ($3, $1, $5); }
	| print opt_rexpression_list output_redir statement_term
		{ $$ = node ($2, $1, $3); }
	| LEX_NEXT statement_term
		{ $$ = node ((NODE *)NULL, Node_K_next, (NODE *)NULL); }
	| LEX_EXIT opt_exp statement_term
		{ $$ = node ($2, Node_K_exit, (NODE *)NULL); }
	| LEX_RETURN opt_exp statement_term
		{ $$ = node ($2, Node_K_return, (NODE *)NULL); }
	| LEX_DELETE NAME '[' expression_list ']' statement_term
		{ $$ = node (variable($2), Node_K_delete, $4); }
	| exp statement_term
		{ $$ = $1; }
	;

print
	: LEX_PRINT
	| LEX_PRINTF
	;

if_statement
	: LEX_IF '(' exp r_paren opt_nls statement
	  {
		$$ = node($3, Node_K_if, 
			node($6, Node_if_branches, (NODE *)NULL));
	  }
	| LEX_IF '(' exp r_paren opt_nls statement
	     LEX_ELSE opt_nls statement
		{ $$ = node ($3, Node_K_if,
				node ($6, Node_if_branches, $9)); }
	;

nls
	: NEWLINE
		{ $$ = NULL; }
	| nls NEWLINE
		{ $$ = NULL; }
	;

opt_nls
	: /* empty */
		{ $$ = NULL; }
	| nls
		{ $$ = NULL; }
	;

input_redir
	: /* empty */
		{ $$ = NULL; }
	| '<' simp_exp
		{ $$ = node ($2, Node_redirect_input, (NODE *)NULL); }
	;

output_redir
	: /* empty */
		{ $$ = NULL; }
	| '>' simp_exp
		{ $$ = node ($2, Node_redirect_output, (NODE *)NULL); }
	| APPEND_OP simp_exp
		{ $$ = node ($2, Node_redirect_append, (NODE *)NULL); }
	| '|' simp_exp
		{ $$ = node ($2, Node_redirect_pipe, (NODE *)NULL); }
	;

opt_param_list
	: /* empty */
		{ $$ = NULL; }
	| param_list
		/* $$ = $1 */
	;

param_list
	: NAME
		{ $$ = make_param($1); }
	| param_list comma NAME
		{ $$ = append_right($1, make_param($3)); yyerrok; }
	| error
		{ $$ = NULL; }
	| param_list error
	| param_list comma error
	;

/* optional expression, as in for loop */
opt_exp
	: /* empty */
		{ $$ = NULL; }
	| exp
	;

opt_rexpression_list
	: /* empty */
		{ $$ = NULL; }
	| rexpression_list
		{ $$ = $1; }
	;

rexpression_list
	: rexp
		{ $$ = node ($1, Node_expression_list, (NODE *)NULL); }
	| rexpression_list comma rexp
	  {
		$$ = append_right($1,
			node( $3, Node_expression_list, (NODE *)NULL));
		yyerrok;
	  }
	| error
		{ $$ = NULL; }
	| rexpression_list error
	| rexpression_list error rexp
	| rexpression_list comma error
	;

opt_expression_list
	: /* empty */
		{ $$ = NULL; }
	| expression_list
		{ $$ = $1; }
	;

expression_list
	: exp
		{ $$ = node ($1, Node_expression_list, (NODE *)NULL); }
	| expression_list comma exp
		{
			$$ = append_right($1,
				node( $3, Node_expression_list, (NODE *)NULL));
			yyerrok;
		}
	| error
		{ $$ = NULL; }
	| expression_list error
	| expression_list error exp
	| expression_list comma error
	;

/* Expressions, not including the comma operator.  */
exp	: variable ASSIGNOP exp
		{ $$ = node ($1, $2, $3); }
	| '(' expression_list r_paren LEX_IN NAME
		{ $$ = node (variable($5), Node_in_array, $2); }
	| exp '|' LEX_GETLINE opt_variable
		{
		  $$ = node ($4, Node_K_getline,
			 node ($1, Node_redirect_pipein, (NODE *)NULL));
		}
	| LEX_GETLINE opt_variable input_redir
		{
		  $$ = node ($2, Node_K_getline, $3);
		}
	| exp LEX_AND exp
		{ $$ = node ($1, Node_and, $3); }
	| exp LEX_OR exp
		{ $$ = node ($1, Node_or, $3); }
	| exp MATCHOP regexp
		 { $$ = node ($1, $2, $3); }
	| exp MATCHOP exp
		 { $$ = node ($1, $2, $3); }
	| exp LEX_IN NAME
		{ $$ = node (variable($3), Node_in_array, $1); }
	| exp RELOP exp
		{ $$ = node ($1, $2, $3); }
	| exp '<' exp
		{ $$ = node ($1, Node_less, $3); }
	| exp '>' exp
		{ $$ = node ($1, Node_greater, $3); }
	| exp '?' exp ':' exp
		{ $$ = node($1, Node_cond_exp, node($3, Node_if_branches, $5));}
	| exp exp %prec CONCAT_OP
		{ $$ = node ($1, Node_concat, $2); }
	| simp_exp
	;

rexp	
	: variable ASSIGNOP rexp
		{ $$ = node ($1, $2, $3); }
	| rexp LEX_AND rexp
		{ $$ = node ($1, Node_and, $3); }
	| rexp LEX_OR rexp
		{ $$ = node ($1, Node_or, $3); }
	| LEX_GETLINE opt_variable input_redir
		{
		  $$ = node ($2, Node_K_getline, $3);
		}
	| rexp MATCHOP regexp
		 { $$ = node ($1, $2, $3); }
	| rexp MATCHOP rexp
		 { $$ = node ($1, $2, $3); }
	| rexp LEX_IN NAME
		{ $$ = node (variable($3), Node_in_array, $1); }
	| rexp RELOP rexp
		{ $$ = node ($1, $2, $3); }
	| rexp '?' rexp ':' rexp
		{ $$ = node($1, Node_cond_exp, node($3, Node_if_branches, $5));}
	| rexp rexp %prec CONCAT_OP
		{ $$ = node ($1, Node_concat, $2); }
	| simp_exp
	;

simp_exp
	: '!' simp_exp %prec UNARY
		{ $$ = node ($2, Node_not,(NODE *) NULL); }
	| '(' exp r_paren
		{ $$ = $2; }
	| LEX_BUILTIN '(' opt_expression_list r_paren
		{ $$ = snode ($3, Node_builtin, $1); }
	| LEX_BUILTIN
		{ $$ = snode ((NODE *)NULL, Node_builtin, $1); }
	| LEX_SUB '(' regexp comma expression_list r_paren 
		{ $$ = node($5, $1, $3); }
	| LEX_SUB '(' exp comma expression_list r_paren 
		{ $$ = node($5, $1, $3); }
	| LEX_MATCH '(' exp comma regexp r_paren
		{ $$ = node($3, $1, $5); }
	| LEX_MATCH '(' exp comma exp r_paren
		{ $$ = node($3, $1, $5); }
	| FUNC_CALL '(' opt_expression_list r_paren
	  {
		$$ = node ($3, Node_func_call, make_string($1, strlen($1)));
	  }
	| INCREMENT variable
		{ $$ = node ($2, Node_preincrement, (NODE *)NULL); }
	| DECREMENT variable
		{ $$ = node ($2, Node_predecrement, (NODE *)NULL); }
	| variable INCREMENT
		{ $$ = node ($1, Node_postincrement, (NODE *)NULL); }
	| variable DECREMENT
		{ $$ = node ($1, Node_postdecrement, (NODE *)NULL); }
	| variable
		{ $$ = $1; }
	| NUMBER
		{ $$ = make_number ($1); }
	| YSTRING
		{ $$ = make_string ($1, -1); }

	/* Binary operators in order of decreasing precedence.  */
	| simp_exp '^' simp_exp
		{ $$ = node ($1, Node_exp, $3); }
	| simp_exp '*' simp_exp
		{ $$ = node ($1, Node_times, $3); }
	| simp_exp '/' simp_exp
		{ $$ = node ($1, Node_quotient, $3); }
	| simp_exp '%' simp_exp
		{ $$ = node ($1, Node_mod, $3); }
	| simp_exp '+' simp_exp
		{ $$ = node ($1, Node_plus, $3); }
	| simp_exp '-' simp_exp
		{ $$ = node ($1, Node_minus, $3); }
	| '-' simp_exp    %prec UNARY
		{ $$ = node ($2, Node_unary_minus, (NODE *)NULL); }
	| '+' simp_exp    %prec UNARY
		{ $$ = $2; }
	;

opt_variable
	: /* empty */
		{ $$ = NULL; }
	| variable
	;

variable
	: NAME
		{ $$ = variable ($1); }
	| NAME '[' expression_list ']'
		{ $$ = node (variable($1), Node_subscript, $3); }
	| '$' simp_exp
		{ $$ = node ($2, Node_field_spec, (NODE *)NULL); }
	;

l_brace
	: '{' opt_nls
	;

r_brace
	: '}' opt_nls	{ yyerrok; }
	;

r_paren
	: ')' { $<nodetypeval>$ = Node_illegal; yyerrok; }
	;

opt_semi
	: /* empty */
	| semi
	;

semi
	: ';'	{ yyerrok; }
	;

comma	: ',' opt_nls	{ $<nodetypeval>$ = Node_illegal; yyerrok; }
	;

%%

struct token {
	char *operator;		/* text to match */
	NODETYPE value;		/* node type */
	int class;		/* lexical class */
	short nostrict;		/* ignore if in strict compatibility mode */
	NODE *(*ptr) ();	/* function that implements this keyword */
};

#define NULL 0

extern NODE
	*do_exp(),	*do_getline(),	*do_index(),	*do_length(),
	*do_sqrt(),	*do_log(),	*do_sprintf(),	*do_substr(),
	*do_split(),	*do_system(),	*do_int(),	*do_close(),
	*do_atan2(),	*do_sin(),	*do_cos(),	*do_rand(),
	*do_srand(),	*do_match(),	*do_tolower(),	*do_toupper();

/* Special functions for debugging */
#ifdef DEBUG
NODE *do_prvars(), *do_bp();
#endif

/* Tokentab is sorted ascii ascending order, so it can be binary searched. */

static struct token tokentab[] = {
	{ "BEGIN",	Node_illegal,		LEX_BEGIN,	0,	0 },
	{ "END",	Node_illegal,		LEX_END,	0,	0 },
	{ "atan2",	Node_builtin,		LEX_BUILTIN,	0,	do_atan2 },
#ifdef DEBUG
	{ "bp",		Node_builtin,		LEX_BUILTIN,	0,	do_bp },
#endif
	{ "break",	Node_K_break,		LEX_BREAK,	0,	0 },
	{ "close",	Node_builtin,		LEX_BUILTIN,	0,	do_close },
	{ "continue",	Node_K_continue,	LEX_CONTINUE,	0,	0 },
	{ "cos",	Node_builtin,		LEX_BUILTIN,	0,	do_cos },
	{ "delete",	Node_K_delete,		LEX_DELETE,	0,	0 },
	{ "do",		Node_K_do,		LEX_DO,		0,	0 },
	{ "else",	Node_illegal,		LEX_ELSE,	0,	0 },
	{ "exit",	Node_K_exit,		LEX_EXIT,	0,	0 },
	{ "exp",	Node_builtin,		LEX_BUILTIN,	0,	do_exp },
	{ "for",	Node_K_for,		LEX_FOR,	0,	0 },
	{ "func",	Node_K_function,	LEX_FUNCTION,	0,	0 },
	{ "function",	Node_K_function,	LEX_FUNCTION,	0,	0 },
	{ "getline",	Node_K_getline,		LEX_GETLINE,	0,	0 },
	{ "gsub",	Node_gsub,		LEX_SUB,	0,	0 },
	{ "if",		Node_K_if,		LEX_IF,		0,	0 },
	{ "in",		Node_illegal,		LEX_IN,		0,	0 },
	{ "index",	Node_builtin,		LEX_BUILTIN,	0,	do_index },
	{ "int",	Node_builtin,		LEX_BUILTIN,	0,	do_int },
	{ "length",	Node_builtin,		LEX_BUILTIN,	0,	do_length },
	{ "log",	Node_builtin,		LEX_BUILTIN,	0,	do_log },
	{ "match",	Node_K_match,		LEX_MATCH,	0,	0 },
	{ "next",	Node_K_next,		LEX_NEXT,	0,	0 },
	{ "print",	Node_K_print,		LEX_PRINT,	0,	0 },
	{ "printf",	Node_K_printf,		LEX_PRINTF,	0,	0 },
#ifdef DEBUG
	{ "prvars",	Node_builtin,		LEX_BUILTIN,	0,	do_prvars },
#endif
	{ "rand",	Node_builtin,		LEX_BUILTIN,	0,	do_rand },
	{ "return",	Node_K_return,		LEX_RETURN,	0,	0 },
	{ "sin",	Node_builtin,		LEX_BUILTIN,	0,	do_sin },
	{ "split",	Node_builtin,		LEX_BUILTIN,	0,	do_split },
	{ "sprintf",	Node_builtin,		LEX_BUILTIN,	0,	do_sprintf },
	{ "sqrt",	Node_builtin,		LEX_BUILTIN,	0,	do_sqrt },
	{ "srand",	Node_builtin,		LEX_BUILTIN,	0,	do_srand },
	{ "sub",	Node_sub,		LEX_SUB,	0,	0 },
	{ "substr",	Node_builtin,		LEX_BUILTIN,	0,	do_substr },
	{ "system",	Node_builtin,		LEX_BUILTIN,	0,	do_system },
	{ "tolower",	Node_builtin,		LEX_BUILTIN,	1,	do_tolower },
	{ "toupper",	Node_builtin,		LEX_BUILTIN,	1,	do_toupper },
	{ "while",	Node_K_while,		LEX_WHILE,	0,	0 },
};

/* VARARGS0 */
static void
yyerror(va_alist)
va_dcl
{
	va_list args;
	char *mesg;
	char *a1;
	register char *ptr, *beg;
	static int list = 0;
	char *scan;

	errcount++;
	va_start(args);
	mesg = va_arg(args, char *);
	if (! list)
		a1 = va_arg(args, char *);
	va_end(args);
	if (mesg || !list) {
		/* Find the current line in the input file */
		if (!lexptr) {
			beg = "(END OF FILE)";
			ptr = beg + 13;
		} else {
			if (*lexptr == '\n' && lexptr != lexptr_begin)
				--lexptr;
			for (beg = lexptr; beg != lexptr_begin && *beg != '\n'; --beg)
				;
			/* NL isn't guaranteed */
			for (ptr = lexptr; *ptr && *ptr != '\n'; ptr++)
				;
			if (beg != lexptr_begin)
				beg++;
		}
		msg("syntax error near line %d:\n%.*s", lineno, ptr - beg, beg);
		scan = beg;
		while (scan <= lexptr)
			if (*scan++ == '\t')
				putc('\t', stderr);
			else
				putc(' ', stderr);
		putc('^', stderr);
		putc(' ', stderr);
		if (mesg) {
			vfprintf(stderr, mesg, args);
		        putc('\n', stderr);
			exit(1);
		} else {
			if (a1) {
				fputs("expecting: ", stderr);
				fputs(a1, stderr);
				list = 1;
				return;
			}
		}
		return;
	}
	if (a1) {
		fputs(" or ", stderr);
		fputs(a1, stderr);
		putc('\n', stderr);
		return;
	}
	putc('\n', stderr);
	list = 0;
}

/*
 * Parse a C escape sequence.  STRING_PTR points to a variable containing a
 * pointer to the string to parse.  That pointer is updated past the
 * characters we use.  The value of the escape sequence is returned. 
 *
 * A negative value means the sequence \ newline was seen, which is supposed to
 * be equivalent to nothing at all. 
 *
 * If \ is followed by a null character, we return a negative value and leave
 * the string pointer pointing at the null character. 
 *
 * If \ is followed by 000, we return 0 and leave the string pointer after the
 * zeros.  A value of 0 does not mean end of string.  
 */

static int
parse_escape(string_ptr)
char **string_ptr;
{
	register int c = *(*string_ptr)++;
	register int i;

	switch (c) {
	case 'a':
		if (strict)
			goto def;
		else
			return BELL;
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		if (strict)
			goto def;
		else
			return '\v';
	case '\n':
		return -2;
	case 0:
		(*string_ptr)--;
		return 0;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		{
			register int i = c - '0';
			register int count = 0;

			while (++count < 3) {
				if ((c = *(*string_ptr)++) >= '0' && c <= '7') {
					i *= 8;
					i += c - '0';
				} else {
					(*string_ptr)--;
					break;
				}
			}
			return i;
		}
	case 'x':
		if (strict)
			goto def;

		i = 0;
		while (1) {
			if (isxdigit((c = *(*string_ptr)++))) {
				if (isdigit(c))
					i += c - '0';
				else if (isupper(c))
					i += c - 'A' + 10;
				else
					i += c - 'a' + 10;
			} else {
				(*string_ptr)--;
				break;
			}
		}
		return i;
	default:
	def:
		return c;
	}
}

/*
 * Read the input and turn it into tokens. Input is now read from a file
 * instead of from malloc'ed memory. The main program takes a program
 * passed as a command line argument and writes it to a temp file. Otherwise
 * the file name is made available in an external variable.
 */

static int
yylex()
{
	register int c;
	register int namelen;
	register char *tokstart;
	register struct token *tokptr;
	char *tokkey;
	static did_newline = 0;	/* the grammar insists that actions end
				 * with newlines.  This was easier than
				 * hacking the grammar. */
	int seen_e = 0;		/* These are for numbers */
	int seen_point = 0;
	extern char **sourcefile;
	extern int tempsource, numfiles;
	static int file_opened = 0;
	static FILE *fin;
	static char cbuf[BUFSIZ];
	int low, mid, high;
	extern int debugging;

	if (! file_opened) {
		file_opened = 1;
#ifdef DEBUG
		if (debugging) {
			int i;

			for (i = 0; i <= numfiles; i++)
				fprintf (stderr, "sourcefile[%d] = %s\n", i,
						sourcefile[i]);
		}
#endif
	nextfile:
		if ((fin = pathopen (sourcefile[++curinfile])) == NULL)
			fatal("cannot open `%s' for reading (%s)",
				sourcefile[curinfile],
				sys_errlist[errno]);
		*(lexptr = cbuf) = '\0';
		/*
		 * immediately unlink the tempfile so that it will
		 * go away cleanly if we bomb.
		 */
		if (tempsource && curinfile == 0)
			(void) unlink (sourcefile[curinfile]);
	}

retry:
	if (! *lexptr)
		if (fgets (cbuf, sizeof cbuf, fin) == NULL) {
			if (fin != NULL)
				fclose (fin);	/* be neat and clean */
			if (curinfile < numfiles)
				goto nextfile;
			return 0;
		} else
			lexptr = lexptr_begin = cbuf;

	if (want_regexp) {
		want_regexp = 0;

		/*
		 * there is a potential bug if a regexp is followed by an
		 * equal sign: "/foo/=bar" would result in assign_quotient
		 * being returned as the next token.  Nothing is done about
		 * it since it is not valid awk, but maybe something should
		 * be done anyway. 
		 */

		tokstart = lexptr;
		while (c = *lexptr++) {
			switch (c) {
			case '\\':
				if (*lexptr++ == '\0') {
					yyerror("unterminated regexp ends with \\");
					return ERROR;
				} else if (lexptr[-1] == '\n')
					goto retry;
				break;
			case '/':	/* end of the regexp */
				lexptr--;
				yylval.sval = tokstart;
				return REGEXP;
			case '\n':
				lineno++;
			case '\0':
				lexptr--;	/* so error messages work */
				yyerror("unterminated regexp");
				return ERROR;
			}
		}
	}

	if (*lexptr == '\n') {
		lexptr++;
		lineno++;
		return NEWLINE;
	}

	while (*lexptr == ' ' || *lexptr == '\t')
		lexptr++;

	tokstart = lexptr;

	switch (c = *lexptr++) {
	case 0:
		return 0;

	case '\n':
		lineno++;
		return NEWLINE;

	case '#':		/* it's a comment */
		while (*lexptr != '\n' && *lexptr != '\0')
			lexptr++;
		goto retry;

	case '\\':
		if (*lexptr == '\n') {
			lineno++;
			lexptr++;
			goto retry;
		} else
			break;
	case ')':
	case ']':
	case '(':	
	case '[':
	case '$':
	case ';':
	case ':':
	case '?':

		/*
		 * set node type to ILLEGAL because the action should set it
		 * to the right thing 
		 */
		yylval.nodetypeval = Node_illegal;
		return c;

	case '{':
	case ',':
		yylval.nodetypeval = Node_illegal;
		return c;

	case '*':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_times;
			lexptr++;
			return ASSIGNOP;
		} else if (*lexptr == '*') {	/* make ** and **= aliases
						 * for ^ and ^= */
			if (lexptr[1] == '=') {
				yylval.nodetypeval = Node_assign_exp;
				lexptr += 2;
				return ASSIGNOP;
			} else {
				yylval.nodetypeval = Node_illegal;
				lexptr++;
				return '^';
			}
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '/':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_quotient;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '%':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_mod;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '^':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_exp;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '+':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_plus;
			lexptr++;
			return ASSIGNOP;
		}
		if (*lexptr == '+') {
			yylval.nodetypeval = Node_illegal;
			lexptr++;
			return INCREMENT;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '!':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_notequal;
			lexptr++;
			return RELOP;
		}
		if (*lexptr == '~') {
			yylval.nodetypeval = Node_nomatch;
			lexptr++;
			return MATCHOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '<':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_leq;
			lexptr++;
			return RELOP;
		}
		yylval.nodetypeval = Node_less;
		return c;

	case '=':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_equal;
			lexptr++;
			return RELOP;
		}
		yylval.nodetypeval = Node_assign;
		return ASSIGNOP;

	case '>':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_geq;
			lexptr++;
			return RELOP;
		} else if (*lexptr == '>') {
			yylval.nodetypeval = Node_redirect_append;
			lexptr++;
			return APPEND_OP;
		}
		yylval.nodetypeval = Node_greater;
		return c;

	case '~':
		yylval.nodetypeval = Node_match;
		return MATCHOP;

	case '}':
		/*
		 * Added did newline stuff.  Easier than
		 * hacking the grammar
		 */
		if (did_newline) {
			did_newline = 0;
			return c;
		}
		did_newline++;
		--lexptr;
		return NEWLINE;

	case '"':
		while (*lexptr != '\0') {
			switch (*lexptr++) {
			case '\\':
				if (*lexptr++ != '\0')
					break;
				/* fall through */
			case '\n':
				yyerror("unterminated string");
				return ERROR;
			case '\"':
				/* Skip the doublequote */
				yylval.sval = tokstart + 1;
				return YSTRING;
			}
		}
		return ERROR;

	case '-':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_minus;
			lexptr++;
			return ASSIGNOP;
		}
		if (*lexptr == '-') {
			yylval.nodetypeval = Node_illegal;
			lexptr++;
			return DECREMENT;
		}

		/*
		 * It looks like space tab comma and newline are the legal
		 * places for a UMINUS.  Have we missed any? 
		 */
		if ((! isdigit(*lexptr) && *lexptr != '.') ||
			(lexptr > lexptr_begin + 1 &&
				    ! index(" \t,\n", lexptr[-2]))) {

			/*
			 * set node type to ILLEGAL because the action should
			 * set it to the right thing 
			 */
			yylval.nodetypeval = Node_illegal;
			return c;
		}
		/* FALL through into number code */
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
	case '.':
		/* It's a number */
		if (c == '-')
			namelen = 1;
		else
			namelen = 0;
		for (; (c = tokstart[namelen]) != '\0'; namelen++) {
			switch (c) {
			case '.':
				if (seen_point)
					goto got_number;
				++seen_point;
				break;
			case 'e':
			case 'E':
				if (seen_e)
					goto got_number;
				++seen_e;
				if (tokstart[namelen + 1] == '-' ||
				    tokstart[namelen + 1] == '+')
					namelen++;
				break;
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
				goto got_number;
			}
		}

got_number:
		lexptr = tokstart + namelen;
		yylval.fval = atof(tokstart);
		return NUMBER;

	case '&':
		if (*lexptr == '&') {
			yylval.nodetypeval = Node_and;
			while (c = *++lexptr) {
				if (c == '#')
					while ((c = *++lexptr) != '\n'
					       && c != '\0')
						;
				if (c == '\n')
					lineno++;
				else if (!isspace(c))
					break;
			}
			return LEX_AND;
		}
		return ERROR;

	case '|':
		if (*lexptr == '|') {
			yylval.nodetypeval = Node_or;
			while (c = *++lexptr) {
				if (c == '#')
					while ((c = *++lexptr) != '\n'
					       && c != '\0')
						;
				if (c == '\n')
					lineno++;
				else if (!isspace(c))
					break;
			}
			return LEX_OR;
		} else {
			yylval.nodetypeval = Node_illegal;
			return c;
		}
		break;
	}

	if (c != '_' && !isalpha(c)) {
		yyerror("Invalid char '%c' in expression\n", c);
		return ERROR;
	}

	/* it's some type of name-type-thing.  Find its length */
	for (namelen = 0; is_identchar(tokstart[namelen]); namelen++)
		/* null */ ;
	emalloc(tokkey, char *, namelen+1, "yylex");
	strncpy (tokkey, tokstart, namelen);
	tokkey[namelen] = '\0';

	/* See if it is a special token.  */
	low = 0;
	high = (sizeof (tokentab) / sizeof (tokentab[0])) - 1;
	while (low <= high) {
		int i, c;

		mid = (low + high) / 2;

	compare:
		c = *tokstart - tokentab[mid].operator[0];
		i = c ? c : strcmp (tokkey, tokentab[mid].operator);

		if (i < 0) {		/* token < mid */
			high = mid - 1;
		} else if (i > 0) {	/* token > mid */
			low = mid + 1;
		} else {
			lexptr = tokstart + namelen;
			if (strict && tokentab[mid].nostrict)
				break;
			if (tokentab[mid].class == LEX_BUILTIN)
				yylval.ptrval = tokentab[mid].ptr;
			else
				yylval.nodetypeval = tokentab[mid].value;
			return tokentab[mid].class;
		}
	}

	/* It's a name.  See how long it is.  */
	yylval.sval = tokkey;
	lexptr = tokstart + namelen;
	if (*lexptr == '(')
		return FUNC_CALL;
	else
		return NAME;
}

#ifndef DEFPATH
#define DEFPATH	".:/usr/lib/awk:/usr/local/lib/awk"
#endif

static FILE *
pathopen (file)
char *file;
{
	static char defpath[] = DEFPATH;
	static char *savepath;
	static int first = 1;
	char *awkpath, *cp;
	char trypath[BUFSIZ];
	FILE *fp, *devopen();
	extern char *getenv ();
	extern int debugging;

	if (strcmp (file, "-") == 0)
		return (stdin);

	if (strict)
		return (fopen (file, "r"));

	if (first) {
		first = 0;
		if ((awkpath = getenv ("AWKPATH")) == NULL || ! *awkpath)
			awkpath = defpath;
		savepath = awkpath;	/* savepath used for restarting */
	} else
		awkpath = savepath;

	if (index (file, '/') != NULL)	/* some kind of path name, no search */
		return (devopen (file, "r"));

	do {
		for (cp = trypath; *awkpath && *awkpath != ':'; )
			*cp++ = *awkpath++;
		*cp++ = '/';
		*cp = '\0';	/* clear left over junk */
		strcat (cp, file);
		if ((fp = devopen (trypath, "r")) != NULL)
			return (fp);

		/* no luck, keep going */
		awkpath++;	/* skip colon */
	} while (*awkpath);
	return (NULL);
}

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, regular files */

FILE *
devopen (name, mode)
char *name, *mode;
{
	int openfd = -1;
	FILE *fdopen ();
	char *cp;

#if defined(STRICT) || defined(NO_DEV_FD)
	return (fopen (name, mode));
#else
	if (strict)
		return (fopen (name, mode));

	if (name[0] != '/' || !STREQN (name, "/dev/", 5))
		return (fopen (name, mode));
	else
		cp = name + 5;
		
	/* XXX - first three tests ignore mode */
	if (STREQ(cp, "stdin"))
		return (stdin);
	else if (STREQ(cp, "stdout"))
		return (stdout);
	else if (STREQ(cp, "stderr"))
		return (stderr);
	else if (STREQN(cp, "fd/", 3)) {
		if (sscanf (cp, "%d", & openfd) == 1 && openfd >= 0)
			/* got something */
			return (fdopen (openfd, mode));
		else
			return (NULL);
	} else
		return (fopen (name, mode));
#endif
}

static NODE *
node_common(op)
NODETYPE op;
{
	register NODE *r;
	extern int numfiles;
	extern int tempsource;
	extern char **sourcefile;

	r = newnode(op);
	r->source_line = lineno;
	if (numfiles > 1 && !tempsource)
		r->source_file = sourcefile[curinfile];
	else
		r->source_file = NULL;
	return r;
}

/*
 * This allocates a node with defined lnode and rnode. 
 * This should only be used by yyparse+co while reading in the program 
 */
NODE *
node(left, op, right)
NODE *left, *right;
NODETYPE op;
{
	register NODE *r;

	r = node_common(op);
	r->lnode = left;
	r->rnode = right;
	return r;
}

/*
 * This allocates a node with defined subnode and proc
 * Otherwise like node()
 */
static NODE *
snode(subn, op, procp)
NODETYPE op;
NODE *(*procp) ();
NODE *subn;
{
	register NODE *r;

	r = node_common(op);
	r->subnode = subn;
	r->proc = procp;
	return r;
}

/*
 * This allocates a Node_line_range node with defined condpair and
 * zeroes the trigger word to avoid the temptation of assuming that calling
 * 'node( foo, Node_line_range, 0)' will properly initialize 'triggered'. 
 */
/* Otherwise like node() */
static NODE *
mkrangenode(cpair)
NODE *cpair;
{
	register NODE *r;

	r = newnode(Node_line_range);
	r->condpair = cpair;
	r->triggered = 0;
	return r;
}

/* Build a for loop */
static NODE *
make_for_loop(init, cond, incr)
NODE *init, *cond, *incr;
{
	register FOR_LOOP_HEADER *r;
	NODE *n;

	emalloc(r, FOR_LOOP_HEADER *, sizeof(FOR_LOOP_HEADER), "make_for_loop");
	n = newnode(Node_illegal);
	r->init = init;
	r->cond = cond;
	r->incr = incr;
	n->sub.nodep.r.hd = r;
	return n;
}

/*
 * Install a name in the hash table specified, even if it is already there.
 * Name stops with first non alphanumeric. Caller must check against
 * redefinition if that is desired. 
 */
NODE *
install(table, name, value)
NODE **table;
char *name;
NODE *value;
{
	register NODE *hp;
	register int i, len, bucket;
	register char *p;

	len = 0;
	p = name;
	while (is_identchar(*p))
		p++;
	len = p - name;

	hp = newnode(Node_hashnode);
	bucket = hashf(name, len, HASHSIZE);
	hp->hnext = table[bucket];
	table[bucket] = hp;
	hp->hlength = len;
	hp->hvalue = value;
	emalloc(hp->hname, char *, len + 1, "install");
	bcopy(name, hp->hname, len);
	hp->hname[len] = '\0';
	hp->hvalue->varname = hp->hname;
	return hp->hvalue;
}

/*
 * find the most recent hash node for name name (ending with first
 * non-identifier char) installed by install 
 */
NODE *
lookup(table, name)
NODE **table;
char *name;
{
	register char *bp;
	register NODE *bucket;
	register int len;

	for (bp = name; is_identchar(*bp); bp++)
		;
	len = bp - name;
	bucket = table[hashf(name, len, HASHSIZE)];
	while (bucket) {
		if (bucket->hlength == len && STREQN(bucket->hname, name, len))
			return bucket->hvalue;
		bucket = bucket->hnext;
	}
	return NULL;
}

#define HASHSTEP(old, c) ((old << 1) + c)
#define MAKE_POS(v) (v & ~0x80000000)	/* make number positive */

/*
 * return hash function on name.
 */
static int
hashf(name, len, hashsize)
register char *name;
register int len;
int hashsize;
{
	register int r = 0;

	while (len--)
		r = HASHSTEP(r, *name++);

	r = MAKE_POS(r) % hashsize;
	return r;
}

/*
 * Add new to the rightmost branch of LIST.  This uses n^2 time, but doesn't
 * get used enough to make optimizing worth it. . . 
 */
/* You don't believe me?  Profile it yourself! */
static NODE *
append_right(list, new)
NODE *list, *new;
{
	register NODE *oldlist;

	oldlist = list;
	while (list->rnode != NULL)
		list = list->rnode;
	list->rnode = new;
	return oldlist;
}

/*
 * check if name is already installed;  if so, it had better have Null value,
 * in which case def is added as the value. Otherwise, install name with def
 * as value. 
 */
static void
func_install(params, def)
NODE *params;
NODE *def;
{
	NODE *r;

	pop_params(params->rnode);
	pop_var(params, 0);
	r = lookup(variables, params->param);
	if (r != NULL) {
		fatal("function name `%s' previously defined", params->param);
	} else
		(void) install(variables, params->param,
			node(params, Node_func, def));
}

static void
pop_var(np, freeit)
NODE *np;
int freeit;
{
	register char *bp;
	register NODE *bucket, **save;
	register int len;
	char *name;

	name = np->param;
	for (bp = name; is_identchar(*bp); bp++)
		;
	len = bp - name;
	save = &(variables[hashf(name, len, HASHSIZE)]);
	for (bucket = *save; bucket; bucket = bucket->hnext) {
		if (len == bucket->hlength && STREQN(bucket->hname, name, len)) {
			*save = bucket->hnext;
			freenode(bucket);
			free(bucket->hname);
			if (freeit)
				free(np->param);
			return;
		}
		save = &(bucket->hnext);
	}
}

static void
pop_params(params)
NODE *params;
{
	register NODE *np;

	for (np = params; np != NULL; np = np->rnode)
		pop_var(np, 1);
}

static NODE *
make_param(name)
char *name;
{
	NODE *r;

	r = newnode(Node_param_list);
	r->param = name;
	r->rnode = NULL;
	r->param_cnt = param_counter++;
	return (install(variables, name, r));
}
