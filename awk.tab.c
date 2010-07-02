
/*  A Bison parser, made from awk.y  */

#define	NAME	258
#define	REGEXP	259
#define	YSTRING	260
#define	ERROR	261
#define	INCDEC	262
#define	NUMBER	263
#define	ASSIGNOP	264
#define	RELOP	265
#define	MATCHOP	266
#define	NEWLINE	267
#define	REDIRECT_OP	268
#define	CONCAT_OP	269
#define	LEX_BEGIN	270
#define	LEX_END	271
#define	LEX_IF	272
#define	LEX_ELSE	273
#define	LEX_WHILE	274
#define	LEX_FOR	275
#define	LEX_BREAK	276
#define	LEX_CONTINUE	277
#define	LEX_PRINT	278
#define	LEX_PRINTF	279
#define	LEX_NEXT	280
#define	LEX_EXIT	281
#define	LEX_IN	282
#define	LEX_AND	283
#define	LEX_OR	284
#define	INCREMENT	285
#define	DECREMENT	286
#define	LEX_BUILTIN	287
#define	UNARY	288

#line 27 "awk.y"

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

#line 64 "awk.y"
typedef union {
  long lval;
  AWKNUM fval;
  NODE *nodeval;
  NODETYPE nodetypeval;
  char *sval;
  NODE *(*ptrval)();
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
 {
      int timestamp;
      int first_line;
      int first_column;
 int last_line;
      int last_column;
      char *text;
   }
 yyltype;

#define YYLTYPE yyltype
#endif

#define	YYACCEPT	return(0)
#define	YYABORT	return(1)
#define	YYERROR	return(1)
#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		200
#define	YYFLAG		-32768
#define	YYNTBASE	49

#define YYTRANSLATE(x) (yytranslate[x])

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    40,     2,     2,    48,    37,     2,     2,    41,
    42,    35,    33,    39,    34,     2,    36,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    45,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    46,     2,    47,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    43,     2,    44,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    38
};

static const short yyrline[] = {     0,
   105,   110,   112,   117,   122,   124,   126,   131,   134,   136,
   138,   140,   142,   151,   153,   158,   160,   164,   166,   171,
   173,   178,   180,   182,   186,   189,   193,   196,   197,   198,
   199,   201,   204,   206,   208,   210,   212,   214,   217,   220,
   222,   228,   230,   236,   239,   244,   246,   248,   250,   255,
   259,   265,   267,   271,   276,   282,   284,   288,   291,   293,
   299,   301,   303,   305,   307,   309,   311,   313,   315,   317,
   319,   323,   325,   327,   329,   331,   334,   336,   340,   342,
   344,   346,   348,   350,   352,   354,   356,   358,   360,   364,
   366,   368,   370,   372,   375,   379,   382,   384
};

static const char * yytname[] = {     0,
"error","$illegal.","NAME","REGEXP","YSTRING","ERROR","INCDEC","NUMBER","ASSIGNOP","RELOP",
"MATCHOP","NEWLINE","REDIRECT_OP","CONCAT_OP","LEX_BEGIN","LEX_END","LEX_IF","LEX_ELSE","LEX_WHILE","LEX_FOR",
"LEX_BREAK","LEX_CONTINUE","LEX_PRINT","LEX_PRINTF","LEX_NEXT","LEX_EXIT","LEX_IN","LEX_AND","LEX_OR","INCREMENT",
"DECREMENT","LEX_BUILTIN","'+'","'-'","'*'","'/'","'%'","UNARY","','","'!'",
"'('","')'","'{'","'}'","';'","'['","']'","'$'","start"
};

static const short yyr1[] = {     0,
    49,    50,    50,    51,    52,    52,    52,    53,    53,    53,
    53,    53,    53,    54,    53,    55,    53,    53,    53,    56,
    56,    57,    57,    57,    58,    58,    59,    59,    59,    59,
    59,    60,    60,    60,    60,    60,    60,    60,    60,    61,
    60,    62,    60,    63,    60,    60,    60,    60,    60,    64,
    64,    65,    65,    66,    66,    67,    67,    68,    68,    68,
    69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
    69,    69,    69,    69,    69,    69,    69,    69,    70,    70,
    70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
    70,    70,    70,    70,    70,    71,    71,    71
};

static const short yyr2[] = {     0,
     2,     1,     2,     4,     0,     1,     3,     1,     1,     2,
     3,     3,     3,     0,     4,     0,     6,     3,     1,     0,
     4,     0,     1,     2,     2,     2,     0,     1,     1,     2,
     2,     5,     1,     6,    10,     9,     9,     2,     2,     0,
     5,     0,     5,     0,     7,     2,     2,     5,     2,     6,
     9,     0,     2,     0,     2,     0,     1,     0,     1,     3,
     4,     1,     3,     2,     2,     2,     2,     2,     1,     1,
     1,     3,     3,     3,     3,     3,     3,     3,     4,     1,
     3,     2,     2,     2,     2,     2,     1,     1,     1,     3,
     3,     3,     3,     3,     3,     1,     4,     2
};

static const short yydefact[] = {    52,
     5,    96,    71,    70,    53,     8,     9,     0,     0,    62,
     0,    14,     0,     0,     0,     5,     2,    20,     6,    19,
    69,     0,    65,    66,    58,     0,    64,     0,    10,     0,
    19,    89,    88,     0,     0,    80,     0,     0,    98,    87,
     3,    27,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    67,    68,     0,     0,    59,
     0,     0,    13,    63,    83,    84,    58,    82,     0,     0,
     0,     0,     0,     0,     0,    85,    86,    29,    28,    22,
    52,    11,    12,     7,    18,    16,    77,    75,    76,    72,
    73,    74,    78,    97,     0,    61,    15,     0,    81,    95,
    93,    94,    90,    91,    92,    31,    30,     0,     0,     0,
     0,     0,    40,    42,     0,     0,    27,     0,    23,    33,
     0,     4,     0,    60,    79,     0,     0,    56,    52,    52,
    38,    39,    58,    58,    58,    46,     0,    47,    22,    21,
    24,    49,     0,     0,     0,    96,     0,    57,    25,    26,
    54,     0,    54,     0,     0,    17,    27,    27,     0,     0,
     0,     0,    44,     0,     0,    27,     0,     0,     0,    56,
     0,    55,    41,    54,    43,    48,    32,    50,    34,     0,
     0,    56,     0,    27,    27,    27,     0,    45,     0,     0,
     0,    27,    51,    37,    36,     0,    35,     0,     0,     0
};

static const short yydefgoto[] = {   198,
    16,    17,    18,    19,    28,   123,    43,   118,   131,    80,
   119,   133,   135,   174,   120,     1,   162,   147,    59,   121,
    39,    21
};

static const short yypact[] = {-32768,
   262,   -29,-32768,-32768,-32768,-32768,-32768,    11,    11,     6,
   313,-32768,   326,   326,   345,   142,-32768,    17,   243,   399,
    31,   313,-32768,-32768,   313,   313,-32768,    61,-32768,    35,
   129,-32768,-32768,    11,    11,    39,   313,   313,-32768,   157,
-32768,    99,    81,   326,   326,   326,   313,    64,   313,   313,
   313,   313,   313,   313,   313,-32768,-32768,    72,   -20,   254,
    87,   105,-32768,-32768,-32768,-32768,   313,-32768,   369,   313,
   313,   313,   313,   313,   313,-32768,-32768,-32768,-32768,   172,
-32768,-32768,   107,    89,   254,-32768,   336,   164,   164,-32768,
-32768,-32768,   254,-32768,   313,-32768,-32768,    46,-32768,   336,
   164,   164,-32768,-32768,-32768,-32768,-32768,   111,   114,   115,
    -6,    -6,-32768,   120,    -6,    71,    99,   202,-32768,-32768,
    -7,   156,   166,   254,-32768,   326,   326,   360,-32768,-32768,
-32768,-32768,   313,   313,   313,-32768,   313,-32768,   172,-32768,
-32768,-32768,   143,   109,   125,    -5,   140,   254,   156,   156,
     3,    53,     3,   381,   232,-32768,    99,    99,   181,   292,
   313,    -6,-32768,    -6,    -6,    99,   172,   172,   206,   313,
   -25,   254,-32768,   198,-32768,-32768,   132,   194,-32768,   174,
   175,   313,    -6,    99,    99,    99,   176,-32768,   172,   172,
   172,    99,-32768,-32768,-32768,   172,-32768,   214,   229,-32768
};

static const short yypgoto[] = {-32768,
-32768,   215,-32768,   -12,-32768,-32768,-32768,    91,   -34,   -82,
  -100,-32768,-32768,-32768,-32768,   -73,   -95,  -159,   -36,    -1,
-32768,   304
};


#define	YYLAST		436


static const short yytable[] = {    20,
    29,    30,    44,    45,   129,   129,    49,   122,   159,    27,
   181,    20,    31,     2,    20,   161,    22,   141,    95,   182,
    58,    96,   187,    60,    61,    50,    51,    52,    53,    54,
    98,    82,    83,    84,   139,    68,    69,   130,   130,    55,
    22,    95,    20,    20,    20,    85,    25,    87,    88,    89,
    90,    91,    92,    93,   141,   149,   150,   164,    15,    42,
    56,    57,    44,    45,    62,    60,   178,   179,   100,   101,
   102,   103,   104,   105,   167,   168,    63,   132,   183,    67,
   136,   138,   129,   177,    95,    49,   142,   125,   193,   194,
   195,    95,    81,   124,   163,   197,   151,   152,   153,    86,
    49,   189,   190,   191,    50,    51,    52,    53,    54,   196,
    78,   137,    79,   144,   145,   130,    44,    45,    94,    50,
    51,    52,    53,    54,    20,    20,   148,   173,    64,   175,
   176,    60,    60,    60,    44,   154,    44,    45,    47,    48,
    97,    -1,    49,   106,     2,   107,     3,   171,   188,     4,
   157,   126,    44,    45,   127,   128,     6,     7,    20,   172,
   134,    50,    51,    52,    53,    54,   158,     5,   148,   143,
    64,     8,     9,    10,     2,    11,     3,    12,   156,     4,
   148,    13,    14,   106,   160,   107,    76,    77,   108,    15,
   109,   110,   111,   112,   113,   114,   115,   116,    52,    53,
    54,     8,     9,    10,     2,    11,     3,   169,   180,     4,
   161,   184,    26,   199,   117,   185,   186,   192,   108,    15,
   109,   110,   111,   112,   113,   114,   115,   116,   200,   155,
    41,     8,     9,    10,     2,    11,     3,     0,     0,     4,
     0,     0,    26,     0,   117,   140,     0,     0,   108,    15,
   109,   110,   111,   112,   113,   114,   115,   116,     0,     0,
     0,     8,     9,    10,     2,    11,     3,    49,     0,     4,
    44,    45,    26,     5,   117,   166,     6,     7,     0,    15,
     0,    46,     0,     0,     0,     0,    50,    51,    52,    53,
    54,     8,     9,    10,     2,    11,     3,    12,     0,     4,
     0,    13,    14,     0,     0,     0,     6,     7,     0,    15,
     0,    23,    24,     0,     0,     2,     0,     3,    40,     0,
     4,     8,     9,    10,     0,    11,     0,    12,     2,     0,
     3,    13,    14,     4,     0,     0,   170,    65,    66,    15,
     6,     7,     8,     9,    10,     0,    11,     2,     0,    32,
     0,     0,    33,    26,     0,     8,     9,    10,     0,    11,
    15,    12,   146,     0,     3,    13,    14,     4,    50,    51,
    52,    53,    54,    15,    34,    35,    36,     0,    37,     0,
     0,     0,    49,     0,     0,    38,     0,     0,     0,     8,
     9,    10,    15,    11,    49,     0,     0,     0,     0,     0,
    26,    50,    51,    52,    53,    54,     0,    15,    47,    48,
    99,     0,    49,    50,    51,    52,    53,    54,     0,     0,
     0,     0,   165,     0,     0,     0,     0,     0,     0,     0,
     0,    50,    51,    52,    53,    54
};

static const short yycheck[] = {     1,
    13,    14,    28,    29,    12,    12,    14,    81,    14,    11,
   170,    13,    14,     3,    16,    13,    46,   118,    39,    45,
    22,    42,   182,    25,    26,    33,    34,    35,    36,    37,
    67,    44,    45,    46,   117,    37,    38,    45,    45,     9,
    46,    39,    44,    45,    46,    47,    41,    49,    50,    51,
    52,    53,    54,    55,   155,   129,   130,   153,    48,    43,
    30,    31,    28,    29,     4,    67,   167,   168,    70,    71,
    72,    73,    74,    75,   157,   158,    42,   112,   174,    41,
   115,   116,    12,   166,    39,    14,   121,    42,   189,   190,
   191,    39,    12,    95,    42,   196,   133,   134,   135,    36,
    14,   184,   185,   186,    33,    34,    35,    36,    37,   192,
    12,    41,    14,   126,   127,    45,    28,    29,    47,    33,
    34,    35,    36,    37,   126,   127,   128,   162,    42,   164,
   165,   133,   134,   135,    28,   137,    28,    29,    10,    11,
    36,     0,    14,    12,     3,    14,     5,   160,   183,     8,
    42,    41,    28,    29,    41,    41,    15,    16,   160,   161,
    41,    33,    34,    35,    36,    37,    42,    12,   170,     4,
    42,    30,    31,    32,     3,    34,     5,    36,    36,     8,
   182,    40,    41,    12,    45,    14,    30,    31,    17,    48,
    19,    20,    21,    22,    23,    24,    25,    26,    35,    36,
    37,    30,    31,    32,     3,    34,     5,    27,     3,     8,
    13,    18,    41,     0,    43,    42,    42,    42,    17,    48,
    19,    20,    21,    22,    23,    24,    25,    26,     0,   139,
    16,    30,    31,    32,     3,    34,     5,    -1,    -1,     8,
    -1,    -1,    41,    -1,    43,    44,    -1,    -1,    17,    48,
    19,    20,    21,    22,    23,    24,    25,    26,    -1,    -1,
    -1,    30,    31,    32,     3,    34,     5,    14,    -1,     8,
    28,    29,    41,    12,    43,    44,    15,    16,    -1,    48,
    -1,    39,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
    37,    30,    31,    32,     3,    34,     5,    36,    -1,     8,
    -1,    40,    41,    -1,    -1,    -1,    15,    16,    -1,    48,
    -1,     8,     9,    -1,    -1,     3,    -1,     5,    15,    -1,
     8,    30,    31,    32,    -1,    34,    -1,    36,     3,    -1,
     5,    40,    41,     8,    -1,    -1,    45,    34,    35,    48,
    15,    16,    30,    31,    32,    -1,    34,     3,    -1,     5,
    -1,    -1,     8,    41,    -1,    30,    31,    32,    -1,    34,
    48,    36,     3,    -1,     5,    40,    41,     8,    33,    34,
    35,    36,    37,    48,    30,    31,    32,    -1,    34,    -1,
    -1,    -1,    14,    -1,    -1,    41,    -1,    -1,    -1,    30,
    31,    32,    48,    34,    14,    -1,    -1,    -1,    -1,    -1,
    41,    33,    34,    35,    36,    37,    -1,    48,    10,    11,
    42,    -1,    14,    33,    34,    35,    36,    37,    -1,    -1,
    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    33,    34,    35,    36,    37
};
#define YYPURE 1

#line 2 "bison.simple"

/* Skeleton output parser for bison,
   copyright (C) 1984 Bob Corbett and Richard Stallman

   Permission is granted to anyone to make or distribute verbatim copies of this program
   provided that the copyright notice and this permission notice are preserved;
   and provided that the recipient is not asked to waive or limit his right to
   redistribute copies as permitted by this permission notice;
   and provided that anyone possessing an executable copy
   is granted access to copy the source code, in machine-readable form,
   in some reasonable manner.

   Permission is granted to distribute derived works or enhanced versions of
   this program under the above conditions with the additional condition
   that the entire derivative or enhanced work
   must be covered by a permission notice identical to this one.

   Anything distributed as part of a package containing portions derived
   from this program, which cannot in current practice perform its function usefully
   in the absense of what was derived directly from this program,
   is to be considered as forming, together with the latter,
   a single work derived from this program,
   which must be entirely covered by a permission notice identical to this one
   in order for distribution of the package to be permitted.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYFAIL		goto yyerrlab;

#define YYTERROR	1

#ifndef YYIMPURE
#define YYLEX		yylex()
#endif

#ifndef YYPURE
#define YYLEX		yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/

int yydebug = 0;		/*  nonzero means print parse trace	*/

#endif  /* YYIMPURE */


/*  YYMAXDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYMAXDEPTH
#define YYMAXDEPTH 200
#endif

/*  YYMAXLIMIT is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXLIMIT
#define YYMAXLIMIT 10000
#endif


#line 87 "bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  YYLTYPE *yylsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYMAXDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYMAXDEPTH];	/*  the semantic value stack		*/
  YYLTYPE yylsa[YYMAXDEPTH];	/*  the location stack			*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */
  YYLTYPE *yyls = yylsa;

  int yymaxdepth = YYMAXDEPTH;

#ifndef YYPURE

  int yychar;
  YYSTYPE yylval;
  YYLTYPE yylloc;

  extern int yydebug;

#endif


  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

  if (yydebug)
    fprintf(stderr, "Starting parse\n");

  yystate = 0;
  yyerrstatus = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
  yylsp = yyls;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      YYLTYPE *yyls1 = yyls;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yymaxdepth);

      yyss = yyss1; yyvs = yyvs1; yyls = yyls1;
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yymaxdepth >= YYMAXLIMIT)
	yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > YYMAXLIMIT)
	yymaxdepth = YYMAXLIMIT;
      yyss = (short *) alloca (yymaxdepth * sizeof (*yyssp));
      bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyls = (YYLTYPE *) alloca (yymaxdepth * sizeof (*yylsp));
      bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      yyvs = (YYSTYPE *) alloca (yymaxdepth * sizeof (*yyvsp));
      bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yylsp = yyls + size - 1;
      yyvsp = yyvs + size - 1;

      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yymaxdepth);

      if (yyssp >= yyss + yymaxdepth - 1)
	YYERROR;
    }

  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
yyresume:

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

      if (yydebug)
	fprintf(stderr, "Parsing next token; it is %d (%s)\n", yychar, yytname[yychar1]);
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

  if (yydebug)
    {
      if (yylen == 1)
	fprintf (stderr, "Reducing 1 value via line %d, ",
		 yyrline[yyn]);
      else
	fprintf (stderr, "Reducing %d values via line %d, ",
		 yylen, yyrline[yyn]);
    }


  switch (yyn) {

case 1:
#line 106 "awk.y"
{ expression_value = yyvsp[0].nodeval; ;
    break;}
case 2:
#line 111 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_rule_list,(NODE *) NULL); ;
    break;}
case 3:
#line 114 "awk.y"
{ yyval.nodeval = append_right (yyvsp[-1].nodeval, node(yyvsp[0].nodeval, Node_rule_list,(NODE *) NULL)); ;
    break;}
case 4:
#line 118 "awk.y"
{ yyval.nodeval = node (yyvsp[-3].nodeval, Node_rule_node, yyvsp[-2].nodeval); ;
    break;}
case 5:
#line 123 "awk.y"
{ yyval.nodeval = NULL; ;
    break;}
case 6:
#line 125 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 7:
#line 127 "awk.y"
{ yyval.nodeval = mkrangenode ( node(yyvsp[-2].nodeval, Node_cond_pair, yyvsp[0].nodeval) ); ;
    break;}
case 8:
#line 133 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_BEGIN,(NODE *) NULL); ;
    break;}
case 9:
#line 135 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_END,(NODE *) NULL); ;
    break;}
case 10:
#line 137 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_not,(NODE *) NULL); ;
    break;}
case 11:
#line 139 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_and, yyvsp[0].nodeval); ;
    break;}
case 12:
#line 141 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_or, yyvsp[0].nodeval); ;
    break;}
case 13:
#line 143 "awk.y"
{
		  yyval.nodeval = yyvsp[-1].nodeval;
		  want_concat_token = 0;
		;
    break;}
case 14:
#line 152 "awk.y"
{ ++want_regexp; ;
    break;}
case 15:
#line 154 "awk.y"
{ want_regexp = 0;
		  yyval.nodeval = node (node (make_number ((AWKNUM)0), Node_field_spec, (NODE *)NULL),
			     Node_match, (NODE *)make_regexp (yyvsp[-1].sval));
		;
    break;}
case 16:
#line 159 "awk.y"
{ ++want_regexp; ;
    break;}
case 17:
#line 161 "awk.y"
{ want_regexp = 0;
		   yyval.nodeval = node (yyvsp[-5].nodeval, yyvsp[-4].nodetypeval, (NODE *)make_regexp(yyvsp[-1].sval));
		 ;
    break;}
case 18:
#line 165 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, yyvsp[-1].nodetypeval, yyvsp[0].nodeval); ;
    break;}
case 19:
#line 167 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 20:
#line 172 "awk.y"
{ yyval.nodeval = NULL; ;
    break;}
case 21:
#line 174 "awk.y"
{ yyval.nodeval = yyvsp[-1].nodeval; ;
    break;}
case 22:
#line 179 "awk.y"
{ yyval.nodeval = NULL; ;
    break;}
case 23:
#line 181 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_statement_list, (NODE *)NULL); ;
    break;}
case 24:
#line 183 "awk.y"
{ yyval.nodeval = append_right(yyvsp[-1].nodeval, node( yyvsp[0].nodeval, Node_statement_list, (NODE *)NULL)); ;
    break;}
case 25:
#line 188 "awk.y"
{ yyval.nodetypeval = Node_illegal; ;
    break;}
case 26:
#line 190 "awk.y"
{ yyval.nodetypeval = Node_illegal; ;
    break;}
case 27:
#line 195 "awk.y"
{ yyval.nodetypeval = Node_illegal; ;
    break;}
case 32:
#line 203 "awk.y"
{ yyval.nodeval = yyvsp[-2].nodeval; ;
    break;}
case 33:
#line 205 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 34:
#line 207 "awk.y"
{ yyval.nodeval = node (yyvsp[-3].nodeval, Node_K_while, yyvsp[0].nodeval); ;
    break;}
case 35:
#line 209 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_K_for, (NODE *)make_for_loop (yyvsp[-7].nodeval, yyvsp[-5].nodeval, yyvsp[-3].nodeval)); ;
    break;}
case 36:
#line 211 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_K_for, (NODE *)make_for_loop (yyvsp[-6].nodeval, (NODE *)NULL, yyvsp[-3].nodeval)); ;
    break;}
case 37:
#line 213 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_K_arrayfor, (NODE *)make_for_loop(variable(yyvsp[-6].sval), (NODE *)NULL, variable(yyvsp[-3].sval))); ;
    break;}
case 38:
#line 216 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_break, (NODE *)NULL); ;
    break;}
case 39:
#line 219 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_continue, (NODE *)NULL); ;
    break;}
case 40:
#line 221 "awk.y"
{ ++want_redirect; ;
    break;}
case 41:
#line 223 "awk.y"
{
		  want_redirect = 0;
		  /* $4->lnode = NULL; */
		  yyval.nodeval = node (yyvsp[-2].nodeval, Node_K_print, yyvsp[-1].nodeval);
		;
    break;}
case 42:
#line 229 "awk.y"
{ ++want_redirect; ;
    break;}
case 43:
#line 231 "awk.y"
{
		  want_redirect = 0;
		  /* $4->lnode = NULL; */
		  yyval.nodeval = node (yyvsp[-2].nodeval, Node_K_printf, yyvsp[-1].nodeval);
		;
    break;}
case 44:
#line 237 "awk.y"
{ ++want_redirect;
		  want_concat_token = 0; ;
    break;}
case 45:
#line 240 "awk.y"
{
		  want_redirect = 0;
		  yyval.nodeval = node (yyvsp[-4].nodeval, Node_K_printf, yyvsp[-1].nodeval);
		;
    break;}
case 46:
#line 245 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_next, (NODE *)NULL); ;
    break;}
case 47:
#line 247 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_exit, (NODE *)NULL); ;
    break;}
case 48:
#line 249 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_K_exit, (NODE *)NULL); ;
    break;}
case 49:
#line 251 "awk.y"
{ yyval.nodeval = yyvsp[-1].nodeval; ;
    break;}
case 50:
#line 257 "awk.y"
{ yyval.nodeval = node (yyvsp[-3].nodeval, Node_K_if,
				node (yyvsp[0].nodeval, Node_if_branches, (NODE *)NULL)); ;
    break;}
case 51:
#line 261 "awk.y"
{ yyval.nodeval = node (yyvsp[-6].nodeval, Node_K_if,
				node (yyvsp[-3].nodeval, Node_if_branches, yyvsp[0].nodeval)); ;
    break;}
case 53:
#line 268 "awk.y"
{ yyval.nodetypeval = Node_illegal; ;
    break;}
case 54:
#line 273 "awk.y"
{ yyval.nodeval = NULL; /* node (NULL, Node_redirect_nil, NULL); */ ;
    break;}
case 55:
#line 277 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, yyvsp[-1].nodetypeval, (NODE *)NULL); ;
    break;}
case 56:
#line 283 "awk.y"
{ yyval.nodeval = NULL; /* node(NULL, Node_builtin, NULL); */ ;
    break;}
case 57:
#line 285 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 58:
#line 290 "awk.y"
{ yyval.nodeval = NULL; ;
    break;}
case 59:
#line 292 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_expression_list, (NODE *)NULL); ;
    break;}
case 60:
#line 294 "awk.y"
{ yyval.nodeval = append_right(yyvsp[-2].nodeval, node( yyvsp[0].nodeval, Node_expression_list, (NODE *)NULL)); ;
    break;}
case 61:
#line 300 "awk.y"
{ yyval.nodeval = snode (yyvsp[-1].nodeval, Node_builtin, yyvsp[-3].ptrval); ;
    break;}
case 62:
#line 302 "awk.y"
{ yyval.nodeval = snode ((NODE *)NULL, Node_builtin, yyvsp[0].ptrval); ;
    break;}
case 63:
#line 304 "awk.y"
{ yyval.nodeval = yyvsp[-1].nodeval; ;
    break;}
case 64:
#line 306 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_unary_minus, (NODE *)NULL); ;
    break;}
case 65:
#line 308 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_preincrement, (NODE *)NULL); ;
    break;}
case 66:
#line 310 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_predecrement, (NODE *)NULL); ;
    break;}
case 67:
#line 312 "awk.y"
{ yyval.nodeval = node (yyvsp[-1].nodeval, Node_postincrement, (NODE *)NULL); ;
    break;}
case 68:
#line 314 "awk.y"
{ yyval.nodeval = node (yyvsp[-1].nodeval, Node_postdecrement, (NODE *)NULL); ;
    break;}
case 69:
#line 316 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 70:
#line 318 "awk.y"
{ yyval.nodeval = make_number (yyvsp[0].fval); ;
    break;}
case 71:
#line 320 "awk.y"
{ yyval.nodeval = make_string (yyvsp[0].sval, -1); ;
    break;}
case 72:
#line 324 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_times, yyvsp[0].nodeval); ;
    break;}
case 73:
#line 326 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_quotient, yyvsp[0].nodeval); ;
    break;}
case 74:
#line 328 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_mod, yyvsp[0].nodeval); ;
    break;}
case 75:
#line 330 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_plus, yyvsp[0].nodeval); ;
    break;}
case 76:
#line 332 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_minus, yyvsp[0].nodeval); ;
    break;}
case 77:
#line 335 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_concat, yyvsp[0].nodeval); ;
    break;}
case 78:
#line 337 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, yyvsp[-1].nodetypeval, yyvsp[0].nodeval); ;
    break;}
case 79:
#line 341 "awk.y"
{ yyval.nodeval = snode (yyvsp[-1].nodeval, Node_builtin, yyvsp[-3].ptrval); ;
    break;}
case 80:
#line 343 "awk.y"
{ yyval.nodeval = snode ((NODE *)NULL, Node_builtin, yyvsp[0].ptrval); ;
    break;}
case 81:
#line 345 "awk.y"
{ yyval.nodeval = yyvsp[-1].nodeval; ;
    break;}
case 82:
#line 347 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_unary_minus, (NODE *)NULL); ;
    break;}
case 83:
#line 349 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_preincrement, (NODE *)NULL); ;
    break;}
case 84:
#line 351 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_predecrement, (NODE *)NULL); ;
    break;}
case 85:
#line 353 "awk.y"
{ yyval.nodeval = node (yyvsp[-1].nodeval, Node_postincrement, (NODE *)NULL); ;
    break;}
case 86:
#line 355 "awk.y"
{ yyval.nodeval = node (yyvsp[-1].nodeval, Node_postdecrement, (NODE *)NULL); ;
    break;}
case 87:
#line 357 "awk.y"
{ yyval.nodeval = yyvsp[0].nodeval; ;
    break;}
case 88:
#line 359 "awk.y"
{ yyval.nodeval = make_number (yyvsp[0].fval); ;
    break;}
case 89:
#line 361 "awk.y"
{ yyval.nodeval = make_string (yyvsp[0].sval, -1); ;
    break;}
case 90:
#line 365 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_times, yyvsp[0].nodeval); ;
    break;}
case 91:
#line 367 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_quotient, yyvsp[0].nodeval); ;
    break;}
case 92:
#line 369 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_mod, yyvsp[0].nodeval); ;
    break;}
case 93:
#line 371 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_plus, yyvsp[0].nodeval); ;
    break;}
case 94:
#line 373 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_minus, yyvsp[0].nodeval); ;
    break;}
case 95:
#line 376 "awk.y"
{ yyval.nodeval = node (yyvsp[-2].nodeval, Node_concat, yyvsp[0].nodeval); ;
    break;}
case 96:
#line 381 "awk.y"
{ yyval.nodeval = variable (yyvsp[0].sval); ;
    break;}
case 97:
#line 383 "awk.y"
{ yyval.nodeval = node (variable(yyvsp[-3].sval), Node_subscript, yyvsp[-1].nodeval); ;
    break;}
case 98:
#line 385 "awk.y"
{ yyval.nodeval = node (yyvsp[0].nodeval, Node_field_spec, (NODE *)NULL); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 303 "bison.simple"

  yyvsp -= yylen;
  yylsp -= yylen;
  yyssp -= yylen;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

  *++yyvsp = yyval;

  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      yyerror("parse error");
    }

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYERROR;

      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYERROR;
  yyvsp--;
  yylsp--;
  yystate = *--yyssp;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  if (yydebug)
    fprintf(stderr, "Shifting error token, ");

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  yystate = yyn;
  goto yynewstate;
}
#line 388 "awk.y"



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
