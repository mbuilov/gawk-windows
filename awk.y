
/*
 * gawk -- GNU version of awk
 * Copyright (C) 1986 Free Software Foundation
 *   Written by Paul Rubin, August 1986
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

#include <stdio.h>
#include "awk.h"

  static int yylex ();


  /*
   * The following variable is used for a very sickening thing.
   * The awk language uses white space as the string concatenation
   * operator, but having a white space token that would have to appear
   * everywhere in all the grammar rules would be unbearable.
   * It turns out we can return CONCAT_OP exactly when there really
   * is one, just from knowing what kinds of other tokens it can appear
   * between (namely, constants, variables, or close parentheses).
   * This is because concatenation has the lowest priority of all
   * operators.  want_concat_token is used to remember that something
   * that could be the left side of a concat has just been returned.
   *
   * If anyone knows a cleaner way to do this (don't look at the Un*x
   * code to find one, though), please suggest it.
   */
  static int want_concat_token;

  /* Two more horrible kludges.  The same comment applies to these two too */
  static int want_regexp;	/* lexical scanning kludge */
  static int want_redirect;	/* similarly */
  int lineno = 1;	/* JF for error msgs */

/* During parsing of a gawk program, the pointer to the next character
   is in this variable.  */
  char *lexptr;		/* JF moved it up here */
  char *lexptr_begin;	/* JF for error msgs */
%}

%union {
  long lval;
  AWKNUM fval;
  NODE *nodeval;
  NODETYPE nodetypeval;
  char *sval;
  NODE *(*ptrval)();
}

%type <nodeval> exp start program rule pattern conditional
%type <nodeval>	action variable redirection expression_list
%type <nodeval>	statements statement if_statement
%type <nodeval> opt_exp v_exp
%type <nodetypeval> whitespace

%token <sval> NAME REGEXP YSTRING
%token <lval> ERROR INCDEC
%token <fval> NUMBER
%token <nodetypeval> ASSIGNOP RELOP MATCHOP NEWLINE REDIRECT_OP CONCAT_OP
%token <nodetypeval> LEX_BEGIN LEX_END LEX_IF LEX_ELSE
%token <nodetypeval> LEX_WHILE LEX_FOR LEX_BREAK LEX_CONTINUE
%token <nodetypeval> LEX_PRINT LEX_PRINTF LEX_NEXT LEX_EXIT
%token  LEX_IN
%token <lval> LEX_AND LEX_OR INCREMENT DECREMENT
%token <ptrval> LEX_BUILTIN

/* these are just yylval numbers */
/* %token <lval> CHAR JF this isn't used anymore */

/* Lowest to highest */
%left LEX_OR
%left LEX_AND
%right ASSIGNOP
%left CONCAT_OP
%left '+' '-'
%left '*' '/' '%'
%right UNARY
%nonassoc MATCHOP RELOP

%%

start   :  optional_newlines program
		{ expression_value = $2; }
	;


program	: rule
		{ $$ = node ($1, Node_rule_list,(NODE *) NULL); }
	| program rule
		/* cons the rule onto the tail of list */
		{ $$ = append_right ($1, node($2, Node_rule_list,(NODE *) NULL)); }
	;

rule	: pattern action NEWLINE optional_newlines
		{ $$ = node ($1, Node_rule_node, $2); }
	;


pattern	: /* empty */
		{ $$ = NULL; }
	| conditional
		{ $$ = $1; }
	| conditional ',' conditional
		{ $$ = mkrangenode ( node($1, Node_cond_pair, $3) ); } /*jfw*/
	;


conditional :
	  LEX_BEGIN
		{ $$ = node ((NODE *)NULL, Node_K_BEGIN,(NODE *) NULL); }
	| LEX_END
		{ $$ = node ((NODE *)NULL, Node_K_END,(NODE *) NULL); }
	| '!' conditional %prec UNARY
		{ $$ = node ($2, Node_not,(NODE *) NULL); }
	| conditional LEX_AND conditional
		{ $$ = node ($1, Node_and, $3); }
	| conditional LEX_OR conditional
		{ $$ = node ($1, Node_or, $3); }
	| '(' conditional ')'
		{
		  $$ = $2;
		  want_concat_token = 0;
		}

	/* In these rules, want_regexp tells yylex that the next thing
		is a regexp so it should read up to the closing slash. */

	| '/'
		{ ++want_regexp; }
	   REGEXP '/'
		{ want_regexp = 0;
		  $$ = node (node (make_number ((AWKNUM)0), Node_field_spec, (NODE *)NULL),
			     Node_match, (NODE *)make_regexp ($3));
		}
	| exp MATCHOP '/'
		 { ++want_regexp; }
	   REGEXP '/'
		 { want_regexp = 0;
		   $$ = node ($1, $2, (NODE *)make_regexp($5));
		 }
	| exp RELOP exp
		{ $$ = node ($1, $2, $3); }
	| exp	/* JF */
		{ $$ = $1; }
	;


action	: /* empty */
		{ $$ = NULL; }
	|	'{' whitespace statements '}'
		{ $$ = $3; }
	;


statements :			/* EMPTY */
		{ $$ = NULL; }
	| statement
		{ $$ = node ($1, Node_statement_list, (NODE *)NULL); }
	| statements statement
		{ $$ = append_right($1, node( $2, Node_statement_list, (NODE *)NULL)); }
	;

statement_term :
	NEWLINE optional_newlines
		{ $<nodetypeval>$ = Node_illegal; }
	| ';' optional_newlines
		{ $<nodetypeval>$ = Node_illegal; }
	;

whitespace :
		/* blank */
		{ $$ = Node_illegal; }
	|  CONCAT_OP
	| NEWLINE
	| whitespace CONCAT_OP
	| whitespace NEWLINE
	;
statement :
	'{' whitespace statements '}' whitespace
		{ $$ = $3; }
	| if_statement
		{ $$ = $1; }
	| LEX_WHILE '(' conditional ')' whitespace statement
		{ $$ = node ($3, Node_K_while, $6); }
	| LEX_FOR '(' opt_exp ';' conditional ';' opt_exp ')' whitespace statement
		{ $$ = node ($10, Node_K_for, (NODE *)make_for_loop ($3, $5, $7)); }
	| LEX_FOR '(' opt_exp ';' ';' opt_exp ')' whitespace statement
		{ $$ = node ($9, Node_K_for, (NODE *)make_for_loop ($3, (NODE *)NULL, $6)); }
	| LEX_FOR '(' NAME CONCAT_OP LEX_IN NAME ')' whitespace statement
		{ $$ = node ($9, Node_K_arrayfor, (NODE *)make_for_loop(variable($3), (NODE *)NULL, variable($6))); }
	| LEX_BREAK statement_term
	   /* for break, maybe we'll have to remember where to break to */
		{ $$ = node ((NODE *)NULL, Node_K_break, (NODE *)NULL); }
	| LEX_CONTINUE statement_term
	   /* similarly */
		{ $$ = node ((NODE *)NULL, Node_K_continue, (NODE *)NULL); }
	| LEX_PRINT
		{ ++want_redirect; }
	    expression_list redirection statement_term
		{
		  want_redirect = 0;
		  /* $4->lnode = NULL; */
		  $$ = node ($3, Node_K_print, $4);
		}
	| LEX_PRINTF
		{ ++want_redirect; }
	    expression_list redirection statement_term
		{
		  want_redirect = 0;
		  /* $4->lnode = NULL; */
		  $$ = node ($3, Node_K_printf, $4);
		}
	| LEX_PRINTF '(' expression_list ')'
		{ ++want_redirect;
		  want_concat_token = 0; }
	    redirection statement_term
		{
		  want_redirect = 0;
		  $$ = node ($3, Node_K_printf, $6);
		}
	| LEX_NEXT statement_term
		{ $$ = node ((NODE *)NULL, Node_K_next, (NODE *)NULL); }
	| LEX_EXIT statement_term
		{ $$ = node ((NODE *)NULL, Node_K_exit, (NODE *)NULL); }
	| LEX_EXIT '(' exp ')' statement_term
		{ $$ = node ($3, Node_K_exit, (NODE *)NULL); }
	| exp statement_term
		{ $$ = $1; }
	;


if_statement:
	LEX_IF '(' conditional ')' whitespace statement
		{ $$ = node ($3, Node_K_if,
				node ($6, Node_if_branches, (NODE *)NULL)); }
	| LEX_IF '(' conditional ')' whitespace statement
	     LEX_ELSE whitespace statement
		{ $$ = node ($3, Node_K_if,
				node ($6, Node_if_branches, $9)); }
	;

optional_newlines :
	  /* empty */
	| optional_newlines NEWLINE
		{ $<nodetypeval>$ = Node_illegal; }
	;

redirection :
	  /* empty */
		{ $$ = NULL; /* node (NULL, Node_redirect_nil, NULL); */ }
	/* | REDIRECT_OP NAME
		{ $$ = node ($2, $1, NULL); } */
	| REDIRECT_OP exp
		{ $$ = node ($2, $1, (NODE *)NULL); }
	;


/* optional expression, as in for loop */
opt_exp :
		{ $$ = NULL; /* node(NULL, Node_builtin, NULL); */ }
	| exp
		{ $$ = $1; }
	;

expression_list :
	  /* empty */
		{ $$ = NULL; }
	| exp
		{ $$ = node ($1, Node_expression_list, (NODE *)NULL); }
	| expression_list ',' exp
		{ $$ = append_right($1, node( $3, Node_expression_list, (NODE *)NULL)); }
	;


/* Expressions, not including the comma operator.  */
exp	:	LEX_BUILTIN '(' expression_list ')'
			{ $$ = snode ($3, Node_builtin, $1); }
	|	LEX_BUILTIN
			{ $$ = snode ((NODE *)NULL, Node_builtin, $1); }
	|	'(' exp ')'
			{ $$ = $2; }
	|	'-' exp    %prec UNARY
			{ $$ = node ($2, Node_unary_minus, (NODE *)NULL); }
	|	INCREMENT variable %prec UNARY
			{ $$ = node ($2, Node_preincrement, (NODE *)NULL); }
	|	DECREMENT variable %prec UNARY
			{ $$ = node ($2, Node_predecrement, (NODE *)NULL); }
	|	variable INCREMENT  %prec UNARY
			{ $$ = node ($1, Node_postincrement, (NODE *)NULL); }
	|	variable DECREMENT  %prec UNARY
			{ $$ = node ($1, Node_postdecrement, (NODE *)NULL); }
	|	variable
			{ $$ = $1; }	/* JF was variable($1) */
	|	NUMBER
			{ $$ = make_number ($1); }
	|	YSTRING
			{ $$ = make_string ($1, -1); }

/* Binary operators in order of decreasing precedence.  */
	|	exp '*' exp
			{ $$ = node ($1, Node_times, $3); }
	|	exp '/' exp
			{ $$ = node ($1, Node_quotient, $3); }
	|	exp '%' exp
			{ $$ = node ($1, Node_mod, $3); }
	|	exp '+' exp
			{ $$ = node ($1, Node_plus, $3); }
	|	exp '-' exp
			{ $$ = node ($1, Node_minus, $3); }
		/* Empty operator.  See yylex for disgusting details. */
	|	exp CONCAT_OP exp
			{ $$ = node ($1, Node_concat, $3); }
	|	variable ASSIGNOP exp
			{ $$ = node ($1, $2, $3); }
	;

v_exp	:	LEX_BUILTIN '(' expression_list ')'
			{ $$ = snode ($3, Node_builtin, $1); }
	|	LEX_BUILTIN
			{ $$ = snode ((NODE *)NULL, Node_builtin, $1); }
	|	'(' exp ')'
			{ $$ = $2; }
	|	'-' exp    %prec UNARY
			{ $$ = node ($2, Node_unary_minus, (NODE *)NULL); }
	|	INCREMENT variable %prec UNARY
			{ $$ = node ($2, Node_preincrement, (NODE *)NULL); }
	|	DECREMENT variable %prec UNARY
			{ $$ = node ($2, Node_predecrement, (NODE *)NULL); }
	|	variable INCREMENT  %prec UNARY
			{ $$ = node ($1, Node_postincrement, (NODE *)NULL); }
	|	variable DECREMENT  %prec UNARY
			{ $$ = node ($1, Node_postdecrement, (NODE *)NULL); }
	|	variable
			{ $$ = $1; }	/* JF was variable($1) */
	|	NUMBER
			{ $$ = make_number ($1); }
	|	YSTRING
			{ $$ = make_string ($1, -1); }

/* Binary operators in order of decreasing precedence.  */
	|	v_exp '*' exp
			{ $$ = node ($1, Node_times, $3); }
	|	v_exp '/' exp
			{ $$ = node ($1, Node_quotient, $3); }
	|	v_exp '%' exp
			{ $$ = node ($1, Node_mod, $3); }
	|	v_exp '+' exp
			{ $$ = node ($1, Node_plus, $3); }
	|	v_exp '-' exp
			{ $$ = node ($1, Node_minus, $3); }
		/* Empty operator.  See yylex for disgusting details. */
	|	v_exp CONCAT_OP exp
			{ $$ = node ($1, Node_concat, $3); }
	;

variable :
	 	NAME
			{ $$ = variable ($1); }
	|	NAME '[' exp ']'
			{ $$ = node (variable($1), Node_subscript, $3); }
	|	'$' v_exp	  %prec UNARY
			{ $$ = node ($2, Node_field_spec, (NODE *)NULL); }
	;

%%


struct token {
  char *operator;
  NODETYPE value;
  int class;
  NODE *(*ptr)();
};

#define NULL 0

NODE	*do_exp(),	*do_getline(),	*do_index(),	*do_length(),
	*do_sqrt(),	*do_log(),	*do_sprintf(),	*do_substr(),
	*do_split(),	*do_int();

	/* Special functions for debugging */
#ifndef FAST
NODE	*do_prvars(),	*do_bp();
#endif

/* Tokentab is sorted ascii ascending order, so it can be binary searched. */
/* (later.  Right now its just sort of linear search (SLOW!!) */

static struct token tokentab[] = {
  {"BEGIN",	Node_illegal,		LEX_BEGIN,	0},
  {"END",	Node_illegal,		LEX_END,	0},
#ifndef FAST
  {"bp",	Node_builtin,		LEX_BUILTIN,	do_bp},
#endif
  {"break",	Node_K_break,		LEX_BREAK,	0},
  {"continue",	Node_K_continue,	LEX_CONTINUE,	0},
  {"else",	Node_illegal,		LEX_ELSE,	0},
  {"exit",	Node_K_exit,		LEX_EXIT,	0},
  {"exp",	Node_builtin,		LEX_BUILTIN,	do_exp},
  {"for",	Node_K_for,		LEX_FOR,	0},
  {"getline",	Node_builtin,		LEX_BUILTIN,	do_getline},
  {"if",	Node_K_if,		LEX_IF,		0},
  {"in",	Node_illegal,		LEX_IN,		0},
  {"index",	Node_builtin,		LEX_BUILTIN,	do_index},
  {"int",	Node_builtin,		LEX_BUILTIN,	do_int},
  {"length",	Node_builtin,		LEX_BUILTIN,	do_length},
  {"log",	Node_builtin,		LEX_BUILTIN,	do_log},
  {"next",	Node_K_next,		LEX_NEXT,	0},
  {"print",	Node_K_print,		LEX_PRINT,	0},
  {"printf",	Node_K_printf,		LEX_PRINTF,	0},
#ifndef FAST
  {"prvars",	Node_builtin,		LEX_BUILTIN,	do_prvars},
#endif
  {"split",	Node_builtin,		LEX_BUILTIN,	do_split},
  {"sprintf",	Node_builtin,		LEX_BUILTIN,	do_sprintf},
  {"sqrt",	Node_builtin,		LEX_BUILTIN,	do_sqrt},
  {"substr",	Node_builtin,		LEX_BUILTIN,	do_substr},
  {"while",	Node_K_while,		LEX_WHILE,	0},
  {NULL,	Node_illegal,		ERROR,		0}
};

/* Read one token, getting characters through lexptr.  */

static int
yylex ()
{
  register int c;
  register int namelen;
  register char *tokstart;
  register struct token *toktab;
  double atof();	/* JF know what happens if you forget this? */


  static did_newline = 0;	/* JF the grammar insists that actions end
  				   with newlines.  This was easier than hacking
				   the grammar. */
  int do_concat;

  int	seen_e = 0;		/* These are for numbers */
  int	seen_point = 0;

  retry:

  if(!lexptr)
    return 0;

  if (want_regexp) {
    want_regexp = 0;
    /* there is a potential bug if a regexp is followed by an equal sign:
       "/foo/=bar" would result in assign_quotient being returned as the
       next token.  Nothing is done about it since it is not valid awk,
       but maybe something should be done anyway. */

    tokstart = lexptr;
    while (c = *lexptr++) {
      switch (c) {
      case '\\':
	if (*lexptr++ == '\0') {
	  yyerror ("unterminated regexp ends with \\");
	  return ERROR;
	}
	break;
      case '/':			/* end of the regexp */
	lexptr--;
	yylval.sval = tokstart;
	return REGEXP;
      case '\n':
      case '\0':
	yyerror ("unterminated regexp");
	return ERROR;
      }
    }
  }
  do_concat=want_concat_token;
  want_concat_token=0;

  if(*lexptr=='\0') {
    lexptr=0;
    return NEWLINE;
  }

  /* if lexptr is at white space between two terminal tokens or parens,
     it is a concatenation operator. */
  if(do_concat && (*lexptr==' ' || *lexptr=='\t')) {
    while (*lexptr == ' ' || *lexptr == '\t')
      lexptr++;
    if (isalnum(*lexptr) || *lexptr == '\"' || *lexptr == '('
        || *lexptr == '.' || *lexptr == '$') /* the '.' is for decimal pt */
      return CONCAT_OP;
  }

  while (*lexptr == ' ' || *lexptr == '\t')
    lexptr++;

  tokstart = lexptr;	/* JF */

  switch (c = *lexptr++) {
  case 0:
    return 0;

  case '\n':
    lineno++;
    return NEWLINE;

  case '#':			/* it's a comment */
    while (*lexptr != '\n' && *lexptr != '\0')
      lexptr++;
    goto retry;

  case '\\':
    if(*lexptr=='\n') {
      lexptr++;
      goto retry;
    } else break;  
  case ')':
  case ']':
    ++want_concat_token;
    /* fall through */
  case '(':	/* JF these were above, but I don't see why they should turn on concat. . . &*/
  case '[':

  case '{':
  case ',':		/* JF */
  case '$':
  case ';':
    /* set node type to ILLEGAL because the action should set it to
       the right thing */
    yylval.nodetypeval = Node_illegal;
    return c;

  case '*':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_assign_times;
      lexptr++;
      return ASSIGNOP;
    }
    yylval.nodetypeval=Node_illegal;
    return c;

  case '/':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_assign_quotient;
      lexptr++;
      return ASSIGNOP;
    }
    yylval.nodetypeval=Node_illegal;
    return c;

  case '%':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_assign_mod;
      lexptr++;
      return ASSIGNOP;
    }
    yylval.nodetypeval=Node_illegal;
    return c;

  case '+':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_assign_plus;
      lexptr++;
      return ASSIGNOP;
    }
    if(*lexptr=='+') {
      yylval.nodetypeval=Node_illegal;
      lexptr++;
      return INCREMENT;
    }
    yylval.nodetypeval=Node_illegal;
    return c;

  case '!':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_notequal;
      lexptr++;
      return RELOP;
    }
    if(*lexptr=='~') {
      yylval.nodetypeval=Node_nomatch;
      lexptr++;
      return MATCHOP;
    }
    yylval.nodetypeval=Node_illegal;
    return c;

  case '<':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_leq;
      lexptr++;
      return RELOP;
    }
    yylval.nodetypeval=Node_less;
    return RELOP;

  case '=':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_equal;
      lexptr++;
      return RELOP;
    }
    yylval.nodetypeval=Node_assign;
    return ASSIGNOP;

  case '>':
    if(want_redirect) {
      if (*lexptr == '>') {
	yylval.nodetypeval = Node_redirect_append;
	lexptr++;
      } else 
        yylval.nodetypeval = Node_redirect_output;
      return REDIRECT_OP;
    }
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_geq;
      lexptr++;
      return RELOP;
    }
    yylval.nodetypeval=Node_greater;
    return RELOP;

  case '~':
    yylval.nodetypeval=Node_match;
    return MATCHOP;

  case '}':		/* JF added did newline stuff.  Easier than hacking the grammar */
    if(did_newline) {
      did_newline=0;
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
	yyerror ("unterminated string");
	return ERROR;
      case '\"':
	yylval.sval = tokstart + 1;	/* JF Skip the doublequote */
	++want_concat_token;
	return YSTRING;
      }
    }
    return ERROR;	/* JF this was one level up, wrong? */

  case '-':
    if(*lexptr=='=') {
      yylval.nodetypeval=Node_assign_minus;
      lexptr++;
      return ASSIGNOP;
    }
    if(*lexptr=='-') {
      yylval.nodetypeval=Node_illegal;
      lexptr++;
      return DECREMENT;
    }
    /* JF I think space tab comma and newline are the legal places for
       a UMINUS.  Have I missed any? */
    if((!isdigit(*lexptr) && *lexptr!='.') || (lexptr>lexptr_begin+1 &&
 !index(" \t,\n",lexptr[-2]))) {
    /* set node type to ILLEGAL because the action should set it to
       the right thing */
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
    if(c=='-') namelen=1;
    else namelen=0;
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
	if (tokstart[namelen+1] == '-' || tokstart[namelen+1] == '+')
	  namelen++;
	break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9': 
	break;
      default:
	goto got_number;
      }
    }

got_number:
    lexptr = tokstart + namelen;
    yylval.fval = atof(tokstart);
    ++want_concat_token;
    return NUMBER;

  case '&':
    if(*lexptr=='&') {
      yylval.nodetypeval=Node_and;
      lexptr++;
      return LEX_AND;
    }
    return ERROR;

  case '|':
    if(want_redirect) {
      lexptr++;
      yylval.nodetypeval = Node_redirect_pipe;
      return REDIRECT_OP;
    }
    if(*lexptr=='|') {
      yylval.nodetypeval=Node_or;
      lexptr++;
      return LEX_OR;
    }
    return ERROR;
  }
  
  if (!isalpha(c)) {
    yyerror ("Invalid char '%c' in expression\n", c);
    return ERROR;
  }

  /* its some type of name-type-thing.  Find its length */
  for (namelen = 0; is_identchar(tokstart[namelen]); namelen++)
    ;


  /* See if it is a special token.  */
  for (toktab = tokentab; toktab->operator != NULL; toktab++) {
    if(*tokstart==toktab->operator[0] &&
       !strncmp(tokstart,toktab->operator,namelen) &&
       toktab->operator[namelen]=='\0') {
      lexptr=tokstart+namelen;
      if(toktab->class == LEX_BUILTIN)
        yylval.ptrval = toktab->ptr;
      else
        yylval.nodetypeval = toktab->value;
      return toktab->class;
    }
  }

  /* It's a name.  See how long it is.  */
  yylval.sval = tokstart;
  lexptr = tokstart+namelen;
  ++want_concat_token;
  return NAME;
}

/*VARARGS1*/
yyerror (mesg,a1,a2,a3,a4,a5,a6,a7,a8)
     char *mesg;
{
  register char *ptr,*beg;

	/* Find the current line in the input file */
  if(!lexptr) {
    beg="(END OF FILE)";
    ptr=beg+13;
  } else {
    if (*lexptr == '\n' && lexptr!=lexptr_begin)
      --lexptr;
    for (beg = lexptr;beg!=lexptr_begin && *beg != '\n';--beg)
      ;
    for (ptr = lexptr;*ptr && *ptr != '\n';ptr++) /*jfw: NL isn't guaranteed*/
      ;
    if(beg!=lexptr_begin)
      beg++;
  }
  fprintf (stderr, "Error near line %d,  '%.*s'\n",lineno, ptr-beg, beg);
  /* figure out line number, etc. later */
  fprintf (stderr, mesg, a1, a2, a3, a4, a5, a6, a7, a8);
  fprintf (stderr,"\n");
  exit (1);
}

/* Parse a C escape sequence.  STRING_PTR points to a variable
   containing a pointer to the string to parse.  That pointer
   is updated past the characters we use.  The value of the
   escape sequence is returned.

   A negative value means the sequence \ newline was seen,
   which is supposed to be equivalent to nothing at all.

   If \ is followed by a null character, we return a negative
   value and leave the string pointer pointing at the null character.

   If \ is followed by 000, we return 0 and leave the string pointer
   after the zeros.  A value of 0 does not mean end of string.  */

static int
parse_escape (string_ptr)
     char **string_ptr;
{
  register int c = *(*string_ptr)++;
  switch (c)
    {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'e':
      return 033;
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case '\n':
      return -2;
    case 0:
      (*string_ptr)--;
      return 0;
    case '^':
      c = *(*string_ptr)++;
      if (c == '\\')
	c = parse_escape (string_ptr);
      if (c == '?')
	return 0177;
      return (c & 0200) | (c & 037);
      
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
	while (++count < 3)
	  {
	    if ((c = *(*string_ptr)++) >= '0' && c <= '7')
	      {
		i *= 8;
		i += c - '0';
	      }
	    else
	      {
		(*string_ptr)--;
		break;
	      }
	  }
	return i;
      }
    default:
      return c;
    }
}
