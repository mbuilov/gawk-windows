/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FUNC_CALL = 258,
     NAME = 259,
     REGEXP = 260,
     ERROR = 261,
     YNUMBER = 262,
     YSTRING = 263,
     RELOP = 264,
     IO_OUT = 265,
     IO_IN = 266,
     ASSIGNOP = 267,
     ASSIGN = 268,
     MATCHOP = 269,
     CONCAT_OP = 270,
     LEX_BEGIN = 271,
     LEX_END = 272,
     LEX_IF = 273,
     LEX_ELSE = 274,
     LEX_RETURN = 275,
     LEX_DELETE = 276,
     LEX_WHILE = 277,
     LEX_DO = 278,
     LEX_FOR = 279,
     LEX_BREAK = 280,
     LEX_CONTINUE = 281,
     LEX_PRINT = 282,
     LEX_PRINTF = 283,
     LEX_NEXT = 284,
     LEX_EXIT = 285,
     LEX_FUNCTION = 286,
     LEX_GETLINE = 287,
     LEX_NEXTFILE = 288,
     LEX_IN = 289,
     LEX_AND = 290,
     LEX_OR = 291,
     INCREMENT = 292,
     DECREMENT = 293,
     LEX_BUILTIN = 294,
     LEX_LENGTH = 295,
     NEWLINE = 296,
     SLASH_BEFORE_EQUAL = 297,
     UNARY = 298
   };
#endif
#define FUNC_CALL 258
#define NAME 259
#define REGEXP 260
#define ERROR 261
#define YNUMBER 262
#define YSTRING 263
#define RELOP 264
#define IO_OUT 265
#define IO_IN 266
#define ASSIGNOP 267
#define ASSIGN 268
#define MATCHOP 269
#define CONCAT_OP 270
#define LEX_BEGIN 271
#define LEX_END 272
#define LEX_IF 273
#define LEX_ELSE 274
#define LEX_RETURN 275
#define LEX_DELETE 276
#define LEX_WHILE 277
#define LEX_DO 278
#define LEX_FOR 279
#define LEX_BREAK 280
#define LEX_CONTINUE 281
#define LEX_PRINT 282
#define LEX_PRINTF 283
#define LEX_NEXT 284
#define LEX_EXIT 285
#define LEX_FUNCTION 286
#define LEX_GETLINE 287
#define LEX_NEXTFILE 288
#define LEX_IN 289
#define LEX_AND 290
#define LEX_OR 291
#define INCREMENT 292
#define DECREMENT 293
#define LEX_BUILTIN 294
#define LEX_LENGTH 295
#define NEWLINE 296
#define SLASH_BEFORE_EQUAL 297
#define UNARY 298




/* Copy the first part of user declarations.  */
#line 26 "awkgram.y"

#ifdef GAWKDEBUG
#define YYDEBUG 12
#endif

#include "awk.h"

#define CAN_FREE	TRUE
#define DONT_FREE	FALSE

#if defined(HAVE_STDARG_H) && defined(__STDC__) && __STDC__
static void yyerror(const char *m, ...) ATTRIBUTE_PRINTF_1;
#else
static void yyerror(); /* va_alist */
#endif
static char *get_src_buf P((void));
static int yylex P((void));
static NODE *node_common P((NODETYPE op));
static NODE *snode P((NODE *subn, NODETYPE op, int sindex));
static NODE *make_for_loop P((NODE *init, NODE *cond, NODE *incr));
static NODE *append_right P((NODE *list, NODE *new));
static inline NODE *append_pattern P((NODE **list, NODE *patt));
static void func_install P((NODE *params, NODE *def));
static void pop_var P((NODE *np, int freeit));
static void pop_params P((NODE *params));
static NODE *make_param P((char *name));
static NODE *mk_rexp P((NODE *exp));
static int dup_parms P((NODE *func));
static void param_sanity P((NODE *arglist));
static int parms_shadow P((const char *fname, NODE *func));
static int isnoeffect P((NODETYPE t));
static int isassignable P((NODE *n));
static void dumpintlstr P((const char *str, size_t len));
static void dumpintlstr2 P((const char *str1, size_t len1, const char *str2, size_t len2));
static void count_args P((NODE *n));

enum defref { FUNC_DEFINE, FUNC_USE };
static void func_use P((const char *name, enum defref how));
static void check_funcs P((void));

static int want_regexp;		/* lexical scanning kludge */
static int can_return;		/* parsing kludge */
static int begin_or_end_rule = FALSE;	/* parsing kludge */
static int parsing_end_rule = FALSE; /* for warnings */
static int in_print = FALSE;	/* lexical scanning kludge for print */
static int in_parens = 0;	/* lexical scanning kludge for print */
static char *lexptr;		/* pointer to next char during parsing */
static char *lexend;
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static char *lexeme;		/* beginning of lexeme for debugging */
static char *thisline = NULL;
#define YYDEBUG_LEXER_TEXT (lexeme)
static int param_counter;
static char *tokstart = NULL;
static char *tok = NULL;
static char *tokend;

static long func_count;		/* total number of functions */

#define HASHSIZE	1021	/* this constant only used here */
NODE *variables[HASHSIZE];
static int var_count;		/* total number of global variables */

extern char *source;
extern int sourceline;
extern struct src *srcfiles;
extern int numfiles;
extern int errcount;
extern NODE *begin_block;
extern NODE *end_block;

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


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 109 "awkgram.y"
typedef union YYSTYPE {
	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval) P((void));
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 253 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 265 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#undef YYSTACK_USE_ALLOCA	/* Gawk: nuke alloca once and for all */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   878

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  48
/* YYNRULES -- Number of rules. */
#define YYNRULES  142
/* YYNRULES -- Number of states. */
#define YYNSTATES  264

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   298

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    53,     2,     2,    56,    52,     2,     2,
      57,    58,    50,    48,    45,    49,     2,    51,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    44,    63,
      46,     2,    47,    43,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,     2,    60,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    61,     2,    62,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    54
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     7,     8,    11,    14,    17,    20,    23,
      24,    26,    30,    32,    34,    40,    42,    44,    46,    48,
      50,    51,    59,    60,    64,    66,    68,    69,    72,    75,
      77,    80,    83,    87,    89,    96,   105,   114,   127,   139,
     142,   145,   148,   151,   155,   156,   161,   164,   165,   170,
     176,   179,   184,   186,   187,   189,   191,   193,   195,   201,
     202,   203,   207,   214,   224,   226,   229,   230,   232,   233,
     236,   237,   239,   241,   245,   247,   250,   254,   255,   257,
     258,   260,   262,   266,   268,   271,   275,   279,   283,   287,
     291,   295,   299,   303,   309,   311,   313,   315,   318,   320,
     322,   324,   326,   328,   331,   337,   339,   342,   344,   348,
     352,   356,   360,   364,   368,   372,   377,   380,   383,   386,
     390,   395,   400,   402,   407,   409,   412,   415,   417,   419,
     422,   425,   426,   428,   430,   435,   438,   441,   444,   446,
     447,   449,   451
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      65,     0,    -1,    90,    66,    90,    -1,    -1,    66,    67,
      -1,    66,     1,    -1,    68,    69,    -1,    68,    78,    -1,
      72,    69,    -1,    -1,    97,    -1,    97,    45,    97,    -1,
      16,    -1,    17,    -1,   106,    77,   107,   109,    90,    -1,
       4,    -1,     3,    -1,    71,    -1,    39,    -1,    40,    -1,
      -1,    31,    73,    70,    57,    92,   108,    90,    -1,    -1,
      76,    75,     5,    -1,    51,    -1,    42,    -1,    -1,    77,
      79,    -1,    77,     1,    -1,    89,    -1,   110,    90,    -1,
     110,    90,    -1,   106,    77,   107,    -1,    88,    -1,    22,
      57,    97,   108,    90,    79,    -1,    23,    90,    79,    22,
      57,    97,   108,    90,    -1,    24,    57,     4,    34,     4,
     108,    90,    79,    -1,    24,    57,    83,   110,    90,    97,
     110,    90,    83,   108,    90,    79,    -1,    24,    57,    83,
     110,    90,   110,    90,    83,   108,    90,    79,    -1,    25,
      78,    -1,    26,    78,    -1,    29,    78,    -1,    33,    78,
      -1,    30,    94,    78,    -1,    -1,    20,    80,    94,    78,
      -1,    81,    78,    -1,    -1,    84,    82,    85,    86,    -1,
      21,     4,    59,    96,    60,    -1,    21,     4,    -1,    21,
      57,     4,    58,    -1,    97,    -1,    -1,    81,    -1,    27,
      -1,    28,    -1,    95,    -1,    57,    97,   111,    96,   108,
      -1,    -1,    -1,    10,    87,   101,    -1,    18,    57,    97,
     108,    90,    79,    -1,    18,    57,    97,   108,    90,    79,
      19,    90,    79,    -1,    41,    -1,    89,    41,    -1,    -1,
      89,    -1,    -1,    46,   102,    -1,    -1,    93,    -1,     4,
      -1,    93,   111,     4,    -1,     1,    -1,    93,     1,    -1,
      93,   111,     1,    -1,    -1,    97,    -1,    -1,    96,    -1,
      97,    -1,    96,   111,    97,    -1,     1,    -1,    96,     1,
      -1,    96,     1,    97,    -1,    96,   111,     1,    -1,   105,
      98,    97,    -1,    97,    35,    97,    -1,    97,    36,    97,
      -1,    97,    14,    97,    -1,    97,    34,     4,    -1,    97,
     100,    97,    -1,    97,    43,    97,    44,    97,    -1,   101,
      -1,    13,    -1,    12,    -1,    42,    13,    -1,     9,    -1,
      46,    -1,    99,    -1,    47,    -1,    74,    -1,    53,    74,
      -1,    57,    96,   108,    34,     4,    -1,   102,    -1,   101,
     102,    -1,   103,    -1,   102,    55,   102,    -1,   102,    50,
     102,    -1,   102,    51,   102,    -1,   102,    52,   102,    -1,
     102,    48,   102,    -1,   102,    49,   102,    -1,    32,   104,
      91,    -1,   102,    11,    32,   104,    -1,   105,    37,    -1,
     105,    38,    -1,    53,   102,    -1,    57,    97,   108,    -1,
      39,    57,    95,   108,    -1,    40,    57,    95,   108,    -1,
      40,    -1,     3,    57,    95,   108,    -1,   105,    -1,    37,
     105,    -1,    38,   105,    -1,     7,    -1,     8,    -1,    49,
     102,    -1,    48,   102,    -1,    -1,   105,    -1,     4,    -1,
       4,    59,    96,    60,    -1,    56,   103,    -1,    61,    90,
      -1,    62,    90,    -1,    58,    -1,    -1,   110,    -1,    63,
      -1,    45,    90,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   170,   170,   176,   178,   183,   196,   200,   215,   226,
     229,   233,   243,   248,   256,   261,   263,   265,   276,   277,
     282,   281,   305,   304,   327,   328,   333,   334,   352,   357,
     358,   362,   364,   366,   368,   370,   372,   412,   416,   421,
     424,   427,   436,   456,   459,   458,   465,   477,   477,   508,
     510,   524,   539,   545,   546,   551,   552,   560,   561,   567,
     572,   572,   583,   588,   595,   596,   599,   601,   606,   607,
     613,   614,   619,   621,   623,   625,   627,   634,   635,   641,
     642,   647,   649,   655,   657,   659,   661,   666,   672,   674,
     676,   682,   684,   690,   692,   697,   699,   701,   706,   708,
     712,   713,   718,   720,   728,   730,   732,   737,   739,   741,
     743,   745,   747,   749,   751,   757,   762,   764,   769,   771,
     773,   776,   778,   786,   794,   795,   797,   799,   801,   804,
     812,   824,   825,   830,   832,   842,   847,   851,   855,   858,
     860,   864,   867
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FUNC_CALL", "NAME", "REGEXP", "ERROR", 
  "YNUMBER", "YSTRING", "RELOP", "IO_OUT", "IO_IN", "ASSIGNOP", "ASSIGN", 
  "MATCHOP", "CONCAT_OP", "LEX_BEGIN", "LEX_END", "LEX_IF", "LEX_ELSE", 
  "LEX_RETURN", "LEX_DELETE", "LEX_WHILE", "LEX_DO", "LEX_FOR", 
  "LEX_BREAK", "LEX_CONTINUE", "LEX_PRINT", "LEX_PRINTF", "LEX_NEXT", 
  "LEX_EXIT", "LEX_FUNCTION", "LEX_GETLINE", "LEX_NEXTFILE", "LEX_IN", 
  "LEX_AND", "LEX_OR", "INCREMENT", "DECREMENT", "LEX_BUILTIN", 
  "LEX_LENGTH", "NEWLINE", "SLASH_BEFORE_EQUAL", "'?'", "':'", "','", 
  "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "UNARY", "'^'", 
  "'$'", "'('", "')'", "'['", "']'", "'{'", "'}'", "';'", "$accept", 
  "start", "program", "rule", "pattern", "action", "func_name", 
  "lex_builtin", "function_prologue", "@1", "regexp", "@2", "a_slash", 
  "statements", "statement_term", "statement", "@3", "simple_stmt", "@4", 
  "opt_simple_stmt", "print", "print_expression_list", "output_redir", 
  "@5", "if_statement", "nls", "opt_nls", "input_redir", "opt_param_list", 
  "param_list", "opt_exp", "opt_expression_list", "expression_list", 
  "exp", "assign_operator", "relop_or_less", "a_relop", "common_exp", 
  "simp_exp", "non_post_simp_exp", "opt_variable", "variable", "l_brace", 
  "r_brace", "r_paren", "opt_semi", "semi", "comma", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    63,    58,    44,    60,    62,    43,    45,
      42,    47,    37,    33,   298,    94,    36,    40,    41,    91,
      93,   123,   125,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    64,    65,    66,    66,    66,    67,    67,    67,    68,
      68,    68,    68,    68,    69,    70,    70,    70,    71,    71,
      73,    72,    75,    74,    76,    76,    77,    77,    77,    78,
      78,    79,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,    79,    79,    80,    79,    79,    82,    81,    81,
      81,    81,    81,    83,    83,    84,    84,    85,    85,    86,
      87,    86,    88,    88,    89,    89,    90,    90,    91,    91,
      92,    92,    93,    93,    93,    93,    93,    94,    94,    95,
      95,    96,    96,    96,    96,    96,    96,    97,    97,    97,
      97,    97,    97,    97,    97,    98,    98,    98,    99,    99,
     100,   100,   101,   101,   101,   101,   101,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   104,   104,   105,   105,   105,   106,   107,   108,   109,
     109,   110,   111
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     3,     0,     2,     2,     2,     2,     2,     0,
       1,     3,     1,     1,     5,     1,     1,     1,     1,     1,
       0,     7,     0,     3,     1,     1,     0,     2,     2,     1,
       2,     2,     3,     1,     6,     8,     8,    12,    11,     2,
       2,     2,     2,     3,     0,     4,     2,     0,     4,     5,
       2,     4,     1,     0,     1,     1,     1,     1,     5,     0,
       0,     3,     6,     9,     1,     2,     0,     1,     0,     2,
       0,     1,     1,     3,     1,     2,     3,     0,     1,     0,
       1,     1,     3,     1,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     2,     5,     1,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     4,     2,     2,     2,     3,
       4,     4,     1,     4,     1,     2,     2,     1,     1,     2,
       2,     0,     1,     1,     4,     2,     2,     2,     1,     0,
       1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      66,    64,     0,    67,     3,     1,    65,     0,     5,     0,
     133,   127,   128,    12,    13,    20,   131,     0,     0,     0,
     122,    25,     0,     0,    24,     0,     0,     0,     4,     0,
       0,   102,    22,     2,    10,    94,   105,   107,   124,     0,
       0,     0,    68,   132,   125,   126,     0,     0,     0,     0,
     130,   124,   129,   103,   118,   135,   124,    83,     0,    81,
      66,   141,     6,     7,    29,    26,    66,     8,     0,    98,
       0,     0,     0,     0,     0,     0,    99,   101,   100,     0,
     106,     0,     0,     0,     0,     0,     0,     0,    96,    95,
     116,   117,     0,     0,     0,     0,    81,     0,    16,    15,
      18,    19,     0,    17,     0,   114,     0,     0,     0,    84,
      66,   138,     0,     0,   119,   136,     0,    30,    23,    90,
      91,    88,    89,     0,    11,    92,   131,   112,   113,   109,
     110,   111,   108,    97,    87,   123,   134,     0,    69,   120,
     121,    85,   142,     0,    86,    82,    28,     0,    44,     0,
       0,    66,     0,     0,     0,    55,    56,     0,    77,     0,
      66,    27,     0,    47,    33,    52,    26,   139,    66,     0,
     115,    74,    72,     0,     0,   104,     0,    77,    50,     0,
       0,     0,    53,    39,    40,    41,     0,    78,    42,   137,
      46,     0,     0,    66,   140,    31,    93,    66,    75,     0,
       0,     0,     0,     0,     0,     0,   133,    54,     0,    43,
       0,    59,    57,    32,    14,    21,    76,    73,    66,    45,
       0,    51,    66,     0,     0,    66,    81,    60,    48,     0,
      49,     0,     0,     0,     0,     0,     0,    62,    34,     0,
      66,     0,    66,     0,    61,    66,    66,     0,    66,    53,
      58,     0,    35,    36,    53,     0,    63,     0,    66,    66,
       0,     0,    38,    37
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     2,     7,    28,    29,    62,   102,   103,    30,    41,
      31,    68,    32,   116,    63,   161,   177,   162,   191,   208,
     163,   211,   228,   236,   164,     3,     4,   105,   173,   174,
     186,    94,    95,   165,    93,    78,    79,    35,    36,    37,
      42,    38,   166,   167,   114,   193,   168,   113
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -238
static const short yypact[] =
{
     -26,  -238,     1,   -19,  -238,  -238,  -238,   262,  -238,   -33,
     -32,  -238,  -238,  -238,  -238,  -238,     0,     0,     0,   -23,
     -13,  -238,   783,   783,  -238,   710,   821,   564,  -238,   -31,
     -24,  -238,  -238,  -238,   737,   783,   145,  -238,   369,   506,
     564,   144,   -15,  -238,  -238,  -238,   506,   506,   783,   761,
     -14,    55,   -14,  -238,   -14,  -238,  -238,  -238,     7,   613,
     -26,  -238,  -238,  -238,   -19,  -238,   -26,  -238,    41,  -238,
     761,    47,   761,   761,   761,   761,  -238,  -238,  -238,   761,
     145,    53,   783,   783,   783,   783,   783,   783,  -238,  -238,
    -238,  -238,    75,   761,    36,    37,   465,    19,  -238,  -238,
    -238,  -238,    33,  -238,   783,  -238,    36,    36,   613,   761,
     -26,  -238,    65,   586,  -238,  -238,   313,  -238,  -238,    82,
    -238,   155,   358,   664,   465,   225,     0,   116,   116,   -14,
     -14,   -14,   -14,  -238,   465,  -238,  -238,    25,   237,  -238,
    -238,   465,  -238,   100,  -238,   465,  -238,    49,  -238,    17,
      50,   -26,    51,   -30,   -30,  -238,  -238,   -30,   761,   -30,
     -26,  -238,   -30,  -238,  -238,   465,  -238,    68,   -26,   761,
    -238,  -238,  -238,    36,    39,  -238,   761,   761,    54,   129,
     761,   433,   637,  -238,  -238,  -238,   -30,   465,  -238,  -238,
    -238,   376,   313,   -26,  -238,  -238,   465,   -26,  -238,    15,
     613,   -30,   564,    76,   613,   117,   -20,  -238,    68,  -238,
     564,   135,  -238,  -238,  -238,  -238,  -238,  -238,   -26,  -238,
      27,  -238,   -26,    92,   146,   -26,   354,  -238,  -238,   433,
    -238,   433,   761,    36,   484,   564,   761,   133,  -238,   613,
     -26,   210,   -26,     7,   783,   -26,   -26,   433,   -26,   688,
    -238,   433,  -238,  -238,   688,    36,  -238,    36,   -26,   -26,
     433,   433,  -238,  -238
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -238,  -238,  -238,  -238,  -238,   125,  -238,  -238,  -238,  -238,
     132,  -238,  -238,    -3,    63,   -69,  -238,  -173,  -238,  -237,
    -238,  -238,  -238,  -238,  -238,   -16,    -7,  -238,  -238,  -238,
      -2,   -40,   -22,    -4,  -238,  -238,  -238,   -57,    38,   173,
      77,    32,    72,    13,     4,  -238,   -27,  -149
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -81
static const short yytable[] =
{
      33,     5,    66,    34,    10,    58,   106,   107,   109,   207,
       1,     1,   255,    64,   224,     1,   216,   257,    97,   217,
     109,   178,     6,    59,    39,   199,   171,    40,   109,   172,
      60,   104,    61,    61,    46,    96,    96,    60,   109,    40,
     198,    87,    96,    96,    47,   108,   118,   -80,    43,    44,
      45,   120,   110,   115,    51,    51,    26,    51,    56,   117,
      50,    52,   112,    54,   110,   111,   119,    51,   121,   122,
     123,   124,   110,    80,   179,   125,   207,   235,   -80,   136,
      51,   207,   110,   -70,   110,   126,    54,   230,   133,   134,
     137,    69,    90,    91,   111,   -80,   -81,   -71,   135,   143,
     -80,    65,    65,   142,   175,   141,   176,   180,   182,   145,
     139,   140,   205,   202,    51,    51,    51,    51,    51,    51,
     127,   128,   129,   130,   131,   132,    66,    66,    76,    77,
      66,    61,    66,   203,   221,    66,    51,    64,    64,   223,
     194,    64,   138,    64,   181,   227,    64,    98,    99,   232,
     233,   212,   245,   189,   187,    67,    81,    53,    43,    66,
     237,   195,   238,   192,    69,   196,    84,    85,    86,    70,
      64,    87,   200,   187,    66,   201,   204,   197,   253,   244,
     220,   225,   256,   100,   101,    64,   214,    96,    58,    71,
     215,   262,   263,    82,    83,    84,    85,    86,    96,    55,
      87,    76,    77,   170,   218,   213,   226,   242,   222,     0,
       0,   229,     0,   243,   248,   231,   183,   184,   234,    69,
     185,     0,   188,     0,    70,   190,     0,     0,   239,     0,
     241,    96,     0,   247,   -81,   249,     0,   240,   251,   252,
       0,   254,     0,   246,    71,    72,    73,   250,   -81,   209,
       0,   260,   261,    74,     0,     0,    76,    77,     0,   258,
       0,   259,   -66,     8,   219,     9,    10,     0,    51,    11,
      12,   -81,   -81,    61,     0,     0,    51,     0,    13,    14,
       0,     0,    80,     0,     0,    82,    83,    84,    85,    86,
       0,     0,    87,    15,    16,     0,     0,     0,     0,    17,
      18,    19,    20,     1,    21,     0,     0,     0,     0,     0,
      22,    23,     0,    24,   146,    25,     9,    10,    26,    27,
      11,    12,     0,    -9,     0,    -9,     0,     0,     0,     0,
       0,   147,     0,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,     0,    16,   159,     0,     0,     0,
      17,    18,    19,    20,     0,    21,     0,     0,     0,     0,
       0,    22,    23,    69,    24,     0,    25,    69,    70,    26,
      27,     0,    70,     0,    60,   160,    61,    57,     0,     9,
      10,    88,    89,    11,    12,     0,   -79,     0,    71,    72,
      73,     0,    71,    72,     0,     0,     0,    74,     0,   110,
      76,    77,     0,     0,    76,    77,    90,    91,    16,     0,
       0,    92,   111,    17,    18,    19,    20,   -79,    21,     0,
       0,     0,     0,     0,    22,    23,     0,    24,     0,    25,
       0,     0,    26,   210,   -79,     0,     9,    10,     0,   -79,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,     0,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,     0,    16,   159,     0,     0,     0,
      17,    18,    19,    20,    69,    21,     0,     0,     0,    70,
       0,    22,    23,     0,    24,     0,    25,     9,    10,    26,
      27,    11,    12,     0,    60,     0,    61,     0,     0,    71,
      72,    73,     0,     0,     0,     0,     0,    57,    74,     9,
      10,    76,    77,    11,    12,     0,    16,     0,     0,     0,
       0,    17,    18,    19,    20,     0,    21,     0,     0,     0,
       0,     0,    22,    23,     0,    24,     0,    25,    16,     0,
      26,    27,     0,    17,    18,    19,    20,    61,    21,     0,
       0,     0,     0,     0,    22,    23,     0,    24,     0,    25,
       0,     0,    26,    27,   -79,    57,     0,     9,    10,     0,
       0,    11,    12,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   144,     0,     9,
      10,     0,     0,    11,    12,     0,    16,     0,     0,     0,
       0,    17,    18,    19,    20,     0,    21,     0,     0,     0,
       0,     0,    22,    23,     0,    24,     0,    25,    16,     0,
      26,    27,    69,    17,    18,    19,    20,    70,    21,     0,
       0,     0,     0,     0,    22,    23,     0,    24,     0,    25,
       9,   206,    26,    27,    11,    12,     0,    71,    72,    73,
       0,     0,     0,     0,     0,     0,    74,     0,   149,    76,
      77,     0,     0,     0,   155,   156,     0,     0,     0,    16,
       0,   111,     0,    69,    17,    18,    19,    20,    70,    21,
       0,     0,     0,     0,     0,    22,    23,     0,    24,     0,
      25,     9,    10,    26,    27,    11,    12,     0,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,   169,   149,
      76,    77,     0,     9,    10,   155,   156,    11,    12,     0,
      16,     0,     0,     0,     0,    17,    18,    19,    20,     0,
      21,     0,     0,     0,     0,     0,    22,    23,     0,    24,
       0,    25,    16,     0,    26,    27,    69,    17,    18,    19,
      20,    70,    21,     0,     0,     0,     0,     0,    22,    23,
       0,    24,     0,    48,     9,    10,    26,    49,    11,    12,
       0,    71,    72,    73,     0,     0,     0,     0,     0,     0,
      74,     0,    75,    76,    77,     0,     9,    10,     0,     0,
      11,    12,     0,    16,     0,     0,     0,     0,    17,    18,
      19,    20,     0,    21,     0,     0,     0,     0,     0,    22,
      23,     0,    24,     0,    25,    16,     0,    26,    27,     0,
      17,    18,    19,    20,     9,    10,     0,     0,    11,    12,
       0,    22,    23,     0,     0,     0,    48,     0,     0,    26,
      49,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,    18,
      19,    20,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,     0,     0,    48,     0,     0,    26,    49
};

static const short yycheck[] =
{
       7,     0,    29,     7,     4,    27,    46,    47,     1,   182,
      41,    41,   249,    29,    34,    41,     1,   254,    40,     4,
       1,     4,    41,    27,    57,   174,     1,    59,     1,     4,
      61,    46,    63,    63,    57,    39,    40,    61,     1,    59,
       1,    55,    46,    47,    57,    49,     5,    10,    16,    17,
      18,     4,    45,    60,    22,    23,    56,    25,    26,    66,
      22,    23,    58,    25,    45,    58,    70,    35,    72,    73,
      74,    75,    45,    35,    57,    79,   249,   226,    41,    60,
      48,   254,    45,    58,    45,    32,    48,    60,    13,    93,
      57,     9,    37,    38,    58,    58,    14,    58,    94,    34,
      63,    29,    30,   110,     4,   109,    57,    57,    57,   113,
     106,   107,   181,    59,    82,    83,    84,    85,    86,    87,
      82,    83,    84,    85,    86,    87,   153,   154,    46,    47,
     157,    63,   159,     4,    58,   162,   104,   153,   154,    22,
     167,   157,   104,   159,   151,    10,   162,     3,     4,    57,
       4,   191,    19,   160,   158,    30,    11,    25,   126,   186,
     229,   168,   231,   166,     9,   169,    50,    51,    52,    14,
     186,    55,   176,   177,   201,   177,   180,   173,   247,   236,
     202,   208,   251,    39,    40,   201,   193,   191,   210,    34,
     197,   260,   261,    48,    49,    50,    51,    52,   202,    26,
      55,    46,    47,   126,   200,   192,   210,   234,   204,    -1,
      -1,   218,    -1,   235,   241,   222,   153,   154,   225,     9,
     157,    -1,   159,    -1,    14,   162,    -1,    -1,   232,    -1,
     234,   235,    -1,   240,     9,   242,    -1,   233,   245,   246,
      -1,   248,    -1,   239,    34,    35,    36,   243,    11,   186,
      -1,   258,   259,    43,    -1,    -1,    46,    47,    -1,   255,
      -1,   257,     0,     1,   201,     3,     4,    -1,   236,     7,
       8,    46,    47,    63,    -1,    -1,   244,    -1,    16,    17,
      -1,    -1,   244,    -1,    -1,    48,    49,    50,    51,    52,
      -1,    -1,    55,    31,    32,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,
      48,    49,    -1,    51,     1,    53,     3,     4,    56,    57,
       7,     8,    -1,    61,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    33,    -1,    -1,    -1,
      37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    48,    49,     9,    51,    -1,    53,     9,    14,    56,
      57,    -1,    14,    -1,    61,    62,    63,     1,    -1,     3,
       4,    12,    13,     7,     8,    -1,    10,    -1,    34,    35,
      36,    -1,    34,    35,    -1,    -1,    -1,    43,    -1,    45,
      46,    47,    -1,    -1,    46,    47,    37,    38,    32,    -1,
      -1,    42,    58,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
      -1,    -1,    56,    57,    58,    -1,     3,     4,    -1,    63,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    32,    33,    -1,    -1,    -1,
      37,    38,    39,    40,     9,    42,    -1,    -1,    -1,    14,
      -1,    48,    49,    -1,    51,    -1,    53,     3,     4,    56,
      57,     7,     8,    -1,    61,    -1,    63,    -1,    -1,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,     1,    43,     3,
       4,    46,    47,     7,     8,    -1,    32,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    32,    -1,
      56,    57,    -1,    37,    38,    39,    40,    63,    42,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
      -1,    -1,    56,    57,    58,     1,    -1,     3,     4,    -1,
      -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,     3,
       4,    -1,    -1,     7,     8,    -1,    32,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    48,    49,    -1,    51,    -1,    53,    32,    -1,
      56,    57,     9,    37,    38,    39,    40,    14,    42,    -1,
      -1,    -1,    -1,    -1,    48,    49,    -1,    51,    -1,    53,
       3,     4,    56,    57,     7,     8,    -1,    34,    35,    36,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    21,    46,
      47,    -1,    -1,    -1,    27,    28,    -1,    -1,    -1,    32,
      -1,    58,    -1,     9,    37,    38,    39,    40,    14,    42,
      -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    51,    -1,
      53,     3,     4,    56,    57,     7,     8,    -1,    34,    35,
      36,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    21,
      46,    47,    -1,     3,     4,    27,    28,     7,     8,    -1,
      32,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    48,    49,    -1,    51,
      -1,    53,    32,    -1,    56,    57,     9,    37,    38,    39,
      40,    14,    42,    -1,    -1,    -1,    -1,    -1,    48,    49,
      -1,    51,    -1,    53,     3,     4,    56,    57,     7,     8,
      -1,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,    -1,
      43,    -1,    45,    46,    47,    -1,     3,     4,    -1,    -1,
       7,     8,    -1,    32,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    51,    -1,    53,    32,    -1,    56,    57,    -1,
      37,    38,    39,    40,     3,     4,    -1,    -1,     7,     8,
      -1,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    -1,    -1,    -1,    53,    -1,    -1,    56,    57
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    41,    65,    89,    90,     0,    41,    66,     1,     3,
       4,     7,     8,    16,    17,    31,    32,    37,    38,    39,
      40,    42,    48,    49,    51,    53,    56,    57,    67,    68,
      72,    74,    76,    90,    97,   101,   102,   103,   105,    57,
      59,    73,   104,   105,   105,   105,    57,    57,    53,    57,
     102,   105,   102,    74,   102,   103,   105,     1,    96,    97,
      61,    63,    69,    78,    89,   106,   110,    69,    75,     9,
      14,    34,    35,    36,    43,    45,    46,    47,    99,   100,
     102,    11,    48,    49,    50,    51,    52,    55,    12,    13,
      37,    38,    42,    98,    95,    96,    97,    96,     3,     4,
      39,    40,    70,    71,    46,    91,    95,    95,    97,     1,
      45,    58,   108,   111,   108,    90,    77,    90,     5,    97,
       4,    97,    97,    97,    97,    97,    32,   102,   102,   102,
     102,   102,   102,    13,    97,   108,    60,    57,   102,   108,
     108,    97,    90,    34,     1,    97,     1,    18,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    33,
      62,    79,    81,    84,    88,    97,   106,   107,   110,    44,
     104,     1,     4,    92,    93,     4,    57,    80,     4,    57,
      57,    90,    57,    78,    78,    78,    94,    97,    78,    90,
      78,    82,    77,   109,   110,    90,    97,   108,     1,   111,
      97,    94,    59,     4,    97,    79,     4,    81,    83,    78,
      57,    85,    95,   107,    90,    90,     1,     4,   108,    78,
      96,    58,   108,    22,    34,   110,    97,    10,    86,    90,
      60,    90,    57,     4,    90,   111,    87,    79,    79,    97,
     108,    97,   110,    96,   101,    19,   108,    90,   110,    90,
     108,    90,    90,    79,    90,    83,    79,    83,   108,   108,
      90,    90,    79,    79
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 171 "awkgram.y"
    {
			check_funcs();
		}
    break;

  case 4:
#line 179 "awkgram.y"
    {
		begin_or_end_rule = parsing_end_rule = FALSE;
		yyerrok;
	  }
    break;

  case 5:
#line 184 "awkgram.y"
    {
		begin_or_end_rule = parsing_end_rule = FALSE;
  		yyerrok;
		/*
		 * If errors, give up, don't produce an infinite
		 * stream of syntax error message.
		 */
		return;
  	  }
    break;

  case 6:
#line 197 "awkgram.y"
    {
		yyvsp[-1].nodeval->rnode = yyvsp[0].nodeval;
	  }
    break;

  case 7:
#line 201 "awkgram.y"
    {
		if (yyvsp[-1].nodeval->lnode != NULL) {
			/* pattern rule with non-empty pattern */
			yyvsp[-1].nodeval->rnode = node(NULL, Node_K_print_rec, NULL);
		} else {
			/* an error */
			if (begin_or_end_rule)
				warning(_("%s blocks must have an action part"),
					(parsing_end_rule ? "END" : "BEGIN"));
			else
				warning(_("each rule must have a pattern or an action part"));
			errcount++;
		}
	  }
    break;

  case 8:
#line 216 "awkgram.y"
    {
		can_return = FALSE;
		if (yyvsp[-1].nodeval)
			func_install(yyvsp[-1].nodeval, yyvsp[0].nodeval);
		yyerrok;
	  }
    break;

  case 9:
#line 226 "awkgram.y"
    {
		yyval.nodeval = append_pattern(&expression_value, (NODE *) NULL);
	  }
    break;

  case 10:
#line 230 "awkgram.y"
    {
		yyval.nodeval = append_pattern(&expression_value, yyvsp[0].nodeval);
	  }
    break;

  case 11:
#line 234 "awkgram.y"
    {
		NODE *r;

		getnode(r);
		r->type = Node_line_range;
		r->condpair = node(yyvsp[-2].nodeval, Node_cond_pair, yyvsp[0].nodeval);
		r->triggered = FALSE;
		yyval.nodeval = append_pattern(&expression_value, r);
	  }
    break;

  case 12:
#line 244 "awkgram.y"
    {
		begin_or_end_rule = TRUE;
		yyval.nodeval = append_pattern(&begin_block, (NODE *) NULL);
	  }
    break;

  case 13:
#line 249 "awkgram.y"
    {
		begin_or_end_rule = parsing_end_rule = TRUE;
		yyval.nodeval = append_pattern(&end_block, (NODE *) NULL);
	  }
    break;

  case 14:
#line 257 "awkgram.y"
    { yyval.nodeval = yyvsp[-3].nodeval; }
    break;

  case 15:
#line 262 "awkgram.y"
    { yyval.sval = yyvsp[0].sval; }
    break;

  case 16:
#line 264 "awkgram.y"
    { yyval.sval = yyvsp[0].sval; }
    break;

  case 17:
#line 266 "awkgram.y"
    {
		yyerror(_("`%s' is a built-in function, it cannot be redefined"),
			tokstart);
		errcount++;
		yyval.sval = builtin_func;
		/* yyerrok; */
	  }
    break;

  case 20:
#line 282 "awkgram.y"
    {
			param_counter = 0;
		}
    break;

  case 21:
#line 286 "awkgram.y"
    {
			NODE *t;

			t = make_param(yyvsp[-4].sval);
			t->flags |= FUNC;
			yyval.nodeval = append_right(t, yyvsp[-2].nodeval);
			can_return = TRUE;
			/* check for duplicate parameter names */
			if (dup_parms(yyval.nodeval))
				errcount++;
		}
    break;

  case 22:
#line 305 "awkgram.y"
    { ++want_regexp; }
    break;

  case 23:
#line 307 "awkgram.y"
    {
		  NODE *n;
		  size_t len = strlen(yyvsp[0].sval);

		  if (do_lint && (yyvsp[0].sval)[0] == '*') {
			/* possible C comment */
			if ((yyvsp[0].sval)[len-1] == '*')
				lintwarn(_("regexp constant `/%s/' looks like a C comment, but is not"), tokstart);
		  }
		  getnode(n);
		  n->type = Node_regex;
		  n->re_exp = make_string(yyvsp[0].sval, len);
		  n->re_reg = make_regexp(yyvsp[0].sval, len, FALSE);
		  n->re_text = NULL;
		  n->re_flags = CONST;
		  yyval.nodeval = n;
		}
    break;

  case 26:
#line 333 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 27:
#line 335 "awkgram.y"
    {
		if (yyvsp[0].nodeval == NULL)
			yyval.nodeval = yyvsp[-1].nodeval;
		else {
			if (do_lint && isnoeffect(yyvsp[0].nodeval->type))
				lintwarn(_("statement may have no effect"));
			if (yyvsp[-1].nodeval == NULL)
				yyval.nodeval = yyvsp[0].nodeval;
			else
	    			yyval.nodeval = append_right(
					(yyvsp[-1].nodeval->type == Node_statement_list ? yyvsp[-1].nodeval
					  : node(yyvsp[-1].nodeval, Node_statement_list, (NODE *) NULL)),
					(yyvsp[0].nodeval->type == Node_statement_list ? yyvsp[0].nodeval
					  : node(yyvsp[0].nodeval, Node_statement_list, (NODE *) NULL)));
		}
	    	yyerrok;
	  }
    break;

  case 28:
#line 353 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 31:
#line 363 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 32:
#line 365 "awkgram.y"
    { yyval.nodeval = yyvsp[-1].nodeval; }
    break;

  case 33:
#line 367 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 34:
#line 369 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-3].nodeval, Node_K_while, yyvsp[0].nodeval); }
    break;

  case 35:
#line 371 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_K_do, yyvsp[-5].nodeval); }
    break;

  case 36:
#line 373 "awkgram.y"
    {
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
		if (yyvsp[0].nodeval != NULL && yyvsp[0].nodeval->type == Node_K_delete) {
			NODE *arr, *sub;

			assert(yyvsp[0].nodeval->rnode->type == Node_expression_list);
			arr = yyvsp[0].nodeval->lnode;	/* array var */
			sub = yyvsp[0].nodeval->rnode->lnode;	/* index var */

			if (   (arr->type == Node_var
				|| arr->type == Node_var_array
				|| arr->type == Node_param_list)
			    && (sub->type == Node_var || sub->type == Node_param_list)
			    && strcmp(yyvsp[-5].sval, sub->vname) == 0
			    && strcmp(yyvsp[-3].sval, arr->vname) == 0) {
				yyvsp[0].nodeval->type = Node_K_delete_loop;
				yyval.nodeval = yyvsp[0].nodeval;
			}
			else
				goto regular_loop;
		} else {
	regular_loop:
			yyval.nodeval = node(yyvsp[0].nodeval, Node_K_arrayfor,
				make_for_loop(variable(yyvsp[-5].sval, CAN_FREE, Node_var),
				(NODE *) NULL, variable(yyvsp[-3].sval, CAN_FREE, Node_var_array)));
		}
	  }
    break;

  case 37:
#line 413 "awkgram.y"
    {
		yyval.nodeval = node(yyvsp[0].nodeval, Node_K_for, (NODE *) make_for_loop(yyvsp[-9].nodeval, yyvsp[-6].nodeval, yyvsp[-3].nodeval));
	  }
    break;

  case 38:
#line 417 "awkgram.y"
    {
		yyval.nodeval = node(yyvsp[0].nodeval, Node_K_for,
			(NODE *) make_for_loop(yyvsp[-8].nodeval, (NODE *) NULL, yyvsp[-3].nodeval));
	  }
    break;

  case 39:
#line 423 "awkgram.y"
    { yyval.nodeval = node((NODE *) NULL, Node_K_break, (NODE *) NULL); }
    break;

  case 40:
#line 426 "awkgram.y"
    { yyval.nodeval = node((NODE *) NULL, Node_K_continue, (NODE *) NULL); }
    break;

  case 41:
#line 428 "awkgram.y"
    { NODETYPE type;

		  if (begin_or_end_rule)
			yyerror(_("`%s' used in %s action"), "next",
				(parsing_end_rule ? "END" : "BEGIN"));
		  type = Node_K_next;
		  yyval.nodeval = node((NODE *) NULL, type, (NODE *) NULL);
		}
    break;

  case 42:
#line 437 "awkgram.y"
    {
		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`nextfile' is a gawk extension"));
		  }
		  if (do_lint)
			lintwarn(_("`nextfile' is a gawk extension"));
		  if (begin_or_end_rule) {
			/* same thing */
			errcount++;
			error(_("`%s' used in %s action"), "nextfile",
				(parsing_end_rule ? "END" : "BEGIN"));
		  }
		  yyval.nodeval = node((NODE *) NULL, Node_K_nextfile, (NODE *) NULL);
		}
    break;

  case 43:
#line 457 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-1].nodeval, Node_K_exit, (NODE *) NULL); }
    break;

  case 44:
#line 459 "awkgram.y"
    {
		  if (! can_return)
			yyerror(_("`return' used outside function context"));
		}
    break;

  case 45:
#line 464 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-1].nodeval, Node_K_return, (NODE *) NULL); }
    break;

  case 47:
#line 477 "awkgram.y"
    { in_print = TRUE; in_parens = 0; }
    break;

  case 48:
#line 478 "awkgram.y"
    {
		/*
		 * Optimization: plain `print' has no expression list, so $3 is null.
		 * If $3 is an expression list with one element (rnode == null)
		 * and lnode is a field spec for field 0, we have `print $0'.
		 * For both, use Node_K_print_rec, which is faster for these two cases.
		 */
		if (yyvsp[-3].nodetypeval == Node_K_print &&
		    (yyvsp[-1].nodeval == NULL
		     || (yyvsp[-1].nodeval->type == Node_expression_list
			&& yyvsp[-1].nodeval->rnode == NULL
			&& yyvsp[-1].nodeval->lnode->type == Node_field_spec
			&& yyvsp[-1].nodeval->lnode->lnode->type == Node_val
			&& yyvsp[-1].nodeval->lnode->lnode->numbr == 0.0))
		) {
			static int warned = FALSE;

			yyval.nodeval = node(NULL, Node_K_print_rec, yyvsp[0].nodeval);

			if (do_lint && yyvsp[-1].nodeval == NULL && begin_or_end_rule && ! warned) {
				warned = TRUE;
				lintwarn(
	_("plain `print' in BEGIN or END rule should probably be `print \"\"'"));
			}
		} else {
			yyval.nodeval = node(yyvsp[-1].nodeval, yyvsp[-3].nodetypeval, yyvsp[0].nodeval);
			if (yyval.nodeval->type == Node_K_printf)
				count_args(yyval.nodeval);
		}
	  }
    break;

  case 49:
#line 509 "awkgram.y"
    { yyval.nodeval = node(variable(yyvsp[-3].sval, CAN_FREE, Node_var_array), Node_K_delete, yyvsp[-1].nodeval); }
    break;

  case 50:
#line 511 "awkgram.y"
    {
		  if (do_lint)
			lintwarn(_("`delete array' is a gawk extension"));
		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`delete array' is a gawk extension"));
		  }
		  yyval.nodeval = node(variable(yyvsp[0].sval, CAN_FREE, Node_var_array), Node_K_delete, (NODE *) NULL);
		}
    break;

  case 51:
#line 525 "awkgram.y"
    {
		  /* this is for tawk compatibility. maybe the warnings should always be done. */
		  if (do_lint)
			lintwarn(_("`delete(array)' is a non-portable tawk extension"));
		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`delete(array)' is a non-portable tawk extension"));
		  }
		  yyval.nodeval = node(variable(yyvsp[-1].sval, CAN_FREE, Node_var_array), Node_K_delete, (NODE *) NULL);
		}
    break;

  case 52:
#line 540 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 53:
#line 545 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 54:
#line 547 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 58:
#line 562 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-3].nodeval, Node_expression_list, yyvsp[-1].nodeval); }
    break;

  case 59:
#line 567 "awkgram.y"
    {
		in_print = FALSE;
		in_parens = 0;
		yyval.nodeval = NULL;
	  }
    break;

  case 60:
#line 572 "awkgram.y"
    { in_print = FALSE; in_parens = 0; }
    break;

  case 61:
#line 573 "awkgram.y"
    {
		yyval.nodeval = node(yyvsp[0].nodeval, yyvsp[-2].nodetypeval, (NODE *) NULL);
		if (yyvsp[-2].nodetypeval == Node_redirect_twoway
		    && yyvsp[0].nodeval->type == Node_K_getline
		    && yyvsp[0].nodeval->rnode->type == Node_redirect_twoway)
			yyerror(_("multistage two-way pipelines don't work"));
	  }
    break;

  case 62:
#line 584 "awkgram.y"
    {
		yyval.nodeval = node(yyvsp[-3].nodeval, Node_K_if, 
			node(yyvsp[0].nodeval, Node_if_branches, (NODE *) NULL));
	  }
    break;

  case 63:
#line 590 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-6].nodeval, Node_K_if,
				node(yyvsp[-3].nodeval, Node_if_branches, yyvsp[0].nodeval)); }
    break;

  case 68:
#line 606 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 69:
#line 608 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_redirect_input, (NODE *) NULL); }
    break;

  case 70:
#line 613 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 71:
#line 615 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 72:
#line 620 "awkgram.y"
    { yyval.nodeval = make_param(yyvsp[0].sval); }
    break;

  case 73:
#line 622 "awkgram.y"
    { yyval.nodeval = append_right(yyvsp[-2].nodeval, make_param(yyvsp[0].sval)); yyerrok; }
    break;

  case 74:
#line 624 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 75:
#line 626 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 76:
#line 628 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 77:
#line 634 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 78:
#line 636 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 79:
#line 641 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 80:
#line 643 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 81:
#line 648 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_expression_list, (NODE *) NULL); }
    break;

  case 82:
#line 650 "awkgram.y"
    {
			yyval.nodeval = append_right(yyvsp[-2].nodeval,
				node(yyvsp[0].nodeval, Node_expression_list, (NODE *) NULL));
			yyerrok;
		}
    break;

  case 83:
#line 656 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 84:
#line 658 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 85:
#line 660 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 86:
#line 662 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 87:
#line 667 "awkgram.y"
    {
		  if (do_lint && yyvsp[0].nodeval->type == Node_regex)
			lintwarn(_("regular expression on right of assignment"));
		  yyval.nodeval = node(yyvsp[-2].nodeval, yyvsp[-1].nodetypeval, yyvsp[0].nodeval);
		}
    break;

  case 88:
#line 673 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_and, yyvsp[0].nodeval); }
    break;

  case 89:
#line 675 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_or, yyvsp[0].nodeval); }
    break;

  case 90:
#line 677 "awkgram.y"
    {
		  if (yyvsp[-2].nodeval->type == Node_regex)
			warning(_("regular expression on left of `~' or `!~' operator"));
		  yyval.nodeval = node(yyvsp[-2].nodeval, yyvsp[-1].nodetypeval, mk_rexp(yyvsp[0].nodeval));
		}
    break;

  case 91:
#line 683 "awkgram.y"
    { yyval.nodeval = node(variable(yyvsp[0].sval, CAN_FREE, Node_var_array), Node_in_array, yyvsp[-2].nodeval); }
    break;

  case 92:
#line 685 "awkgram.y"
    {
		  if (do_lint && yyvsp[0].nodeval->type == Node_regex)
			lintwarn(_("regular expression on right of comparison"));
		  yyval.nodeval = node(yyvsp[-2].nodeval, yyvsp[-1].nodetypeval, yyvsp[0].nodeval);
		}
    break;

  case 93:
#line 691 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-4].nodeval, Node_cond_exp, node(yyvsp[-2].nodeval, Node_if_branches, yyvsp[0].nodeval));}
    break;

  case 94:
#line 693 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 95:
#line 698 "awkgram.y"
    { yyval.nodetypeval = yyvsp[0].nodetypeval; }
    break;

  case 96:
#line 700 "awkgram.y"
    { yyval.nodetypeval = yyvsp[0].nodetypeval; }
    break;

  case 97:
#line 702 "awkgram.y"
    { yyval.nodetypeval = Node_assign_quotient; }
    break;

  case 98:
#line 707 "awkgram.y"
    { yyval.nodetypeval = yyvsp[0].nodetypeval; }
    break;

  case 99:
#line 709 "awkgram.y"
    { yyval.nodetypeval = Node_less; }
    break;

  case 101:
#line 714 "awkgram.y"
    { yyval.nodetypeval = Node_greater; }
    break;

  case 102:
#line 719 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 103:
#line 721 "awkgram.y"
    {
		  yyval.nodeval = node(node(make_number(0.0),
				 Node_field_spec,
				 (NODE *) NULL),
		            Node_nomatch,
			    yyvsp[0].nodeval);
		}
    break;

  case 104:
#line 729 "awkgram.y"
    { yyval.nodeval = node(variable(yyvsp[0].sval, CAN_FREE, Node_var_array), Node_in_array, yyvsp[-3].nodeval); }
    break;

  case 105:
#line 731 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 106:
#line 733 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-1].nodeval, Node_concat, yyvsp[0].nodeval); }
    break;

  case 108:
#line 740 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_exp, yyvsp[0].nodeval); }
    break;

  case 109:
#line 742 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_times, yyvsp[0].nodeval); }
    break;

  case 110:
#line 744 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_quotient, yyvsp[0].nodeval); }
    break;

  case 111:
#line 746 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_mod, yyvsp[0].nodeval); }
    break;

  case 112:
#line 748 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_plus, yyvsp[0].nodeval); }
    break;

  case 113:
#line 750 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-2].nodeval, Node_minus, yyvsp[0].nodeval); }
    break;

  case 114:
#line 752 "awkgram.y"
    {
		  if (do_lint && parsing_end_rule && yyvsp[0].nodeval == NULL)
			lintwarn(_("non-redirected `getline' undefined inside END action"));
		  yyval.nodeval = node(yyvsp[-1].nodeval, Node_K_getline, yyvsp[0].nodeval);
		}
    break;

  case 115:
#line 758 "awkgram.y"
    {
		  yyval.nodeval = node(yyvsp[0].nodeval, Node_K_getline,
			 node(yyvsp[-3].nodeval, yyvsp[-2].nodetypeval, (NODE *) NULL));
		}
    break;

  case 116:
#line 763 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-1].nodeval, Node_postincrement, (NODE *) NULL); }
    break;

  case 117:
#line 765 "awkgram.y"
    { yyval.nodeval = node(yyvsp[-1].nodeval, Node_postdecrement, (NODE *) NULL); }
    break;

  case 118:
#line 770 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_not, (NODE *) NULL); }
    break;

  case 119:
#line 772 "awkgram.y"
    { yyval.nodeval = yyvsp[-1].nodeval; }
    break;

  case 120:
#line 775 "awkgram.y"
    { yyval.nodeval = snode(yyvsp[-1].nodeval, Node_builtin, (int) yyvsp[-3].lval); }
    break;

  case 121:
#line 777 "awkgram.y"
    { yyval.nodeval = snode(yyvsp[-1].nodeval, Node_builtin, (int) yyvsp[-3].lval); }
    break;

  case 122:
#line 779 "awkgram.y"
    {
		if (do_lint)
			lintwarn(_("call of `length' without parentheses is not portable"));
		yyval.nodeval = snode((NODE *) NULL, Node_builtin, (int) yyvsp[0].lval);
		if (do_posix)
			warning(_("call of `length' without parentheses is deprecated by POSIX"));
	  }
    break;

  case 123:
#line 787 "awkgram.y"
    {
		yyval.nodeval = node(yyvsp[-1].nodeval, Node_func_call, make_string(yyvsp[-3].sval, strlen(yyvsp[-3].sval)));
		yyval.nodeval->funcbody = NULL;
		func_use(yyvsp[-3].sval, FUNC_USE);
		param_sanity(yyvsp[-1].nodeval);
		free(yyvsp[-3].sval);
	  }
    break;

  case 125:
#line 796 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_preincrement, (NODE *) NULL); }
    break;

  case 126:
#line 798 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_predecrement, (NODE *) NULL); }
    break;

  case 127:
#line 800 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 128:
#line 802 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 129:
#line 805 "awkgram.y"
    {
		  if (yyvsp[0].nodeval->type == Node_val && (yyvsp[0].nodeval->flags & (STRCUR|STRING)) == 0) {
			yyvsp[0].nodeval->numbr = -(force_number(yyvsp[0].nodeval));
			yyval.nodeval = yyvsp[0].nodeval;
		  } else
			yyval.nodeval = node(yyvsp[0].nodeval, Node_unary_minus, (NODE *) NULL);
		}
    break;

  case 130:
#line 813 "awkgram.y"
    {
		  /*
		   * was: $$ = $2
		   * POSIX semantics: force a conversion to numeric type
		   */
		  yyval.nodeval = node (make_number(0.0), Node_plus, yyvsp[0].nodeval);
		}
    break;

  case 131:
#line 824 "awkgram.y"
    { yyval.nodeval = NULL; }
    break;

  case 132:
#line 826 "awkgram.y"
    { yyval.nodeval = yyvsp[0].nodeval; }
    break;

  case 133:
#line 831 "awkgram.y"
    { yyval.nodeval = variable(yyvsp[0].sval, CAN_FREE, Node_var); }
    break;

  case 134:
#line 833 "awkgram.y"
    {
		if (yyvsp[-1].nodeval == NULL) {
			fatal(_("invalid subscript expression"));
		} else if (yyvsp[-1].nodeval->rnode == NULL) {
			yyval.nodeval = node(variable(yyvsp[-3].sval, CAN_FREE, Node_var_array), Node_subscript, yyvsp[-1].nodeval->lnode);
			freenode(yyvsp[-1].nodeval);
		} else
			yyval.nodeval = node(variable(yyvsp[-3].sval, CAN_FREE, Node_var_array), Node_subscript, yyvsp[-1].nodeval);
		}
    break;

  case 135:
#line 843 "awkgram.y"
    { yyval.nodeval = node(yyvsp[0].nodeval, Node_field_spec, (NODE *) NULL); }
    break;

  case 137:
#line 851 "awkgram.y"
    { yyerrok; }
    break;

  case 138:
#line 855 "awkgram.y"
    { yyerrok; }
    break;

  case 141:
#line 864 "awkgram.y"
    { yyerrok; }
    break;

  case 142:
#line 867 "awkgram.y"
    { yyerrok; }
    break;


    }

/* Line 991 of yacc.c.  */
#line 2414 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 870 "awkgram.y"


struct token {
	const char *operator;		/* text to match */
	NODETYPE value;		/* node type */
	int class;		/* lexical class */
	unsigned flags;		/* # of args. allowed and compatability */
#	define	ARGS	0xFF	/* 0, 1, 2, 3 args allowed (any combination */
#	define	A(n)	(1<<(n))
#	define	VERSION_MASK	0xFF00	/* old awk is zero */
#	define	NOT_OLD		0x0100	/* feature not in old awk */
#	define	NOT_POSIX	0x0200	/* feature not in POSIX */
#	define	GAWKX		0x0400	/* gawk extension */
#	define	RESX		0x0800	/* Bell Labs Research extension */
	NODE *(*ptr) P((NODE *));	/* function that implements this keyword */
};

/* Tokentab is sorted ascii ascending order, so it can be binary searched. */
/* Function pointers come from declarations in awk.h. */

static const struct token tokentab[] = {
{"BEGIN",	Node_illegal,	 LEX_BEGIN,	0,		0},
{"END",		Node_illegal,	 LEX_END,	0,		0},
#ifdef ARRAYDEBUG
{"adump",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_adump},
#endif
{"and",		Node_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_and},
{"asort",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2),	do_asort},
{"asorti",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2),	do_asorti},
{"atan2",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2),	do_atan2},
{"bindtextdomain",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2),	do_bindtextdomain},
{"break",	Node_K_break,	 LEX_BREAK,	0,		0},
{"close",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1)|A(2),	do_close},
{"compl",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_compl},
{"continue",	Node_K_continue, LEX_CONTINUE,	0,		0},
{"cos",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_cos},
{"dcgettext",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_dcgettext},
{"dcngettext",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3)|A(4)|A(5),	do_dcngettext},
{"delete",	Node_K_delete,	 LEX_DELETE,	NOT_OLD,	0},
{"do",		Node_K_do,	 LEX_DO,	NOT_OLD,	0},
{"else",	Node_illegal,	 LEX_ELSE,	0,		0},
{"exit",	Node_K_exit,	 LEX_EXIT,	0,		0},
{"exp",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_exp},
{"extension",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(2),	do_ext},
{"fflush",	Node_builtin,	 LEX_BUILTIN,	RESX|A(0)|A(1), do_fflush},
{"for",		Node_K_for,	 LEX_FOR,	0,		0},
{"func",	Node_K_function, LEX_FUNCTION,	NOT_POSIX|NOT_OLD,	0},
{"function",	Node_K_function, LEX_FUNCTION,	NOT_OLD,	0},
{"gensub",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(3)|A(4), do_gensub},
{"getline",	Node_K_getline,	 LEX_GETLINE,	NOT_OLD,	0},
{"gsub",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_gsub},
{"if",		Node_K_if,	 LEX_IF,	0,		0},
{"in",		Node_illegal,	 LEX_IN,	0,		0},
{"index",	Node_builtin,	 LEX_BUILTIN,	A(2),		do_index},
{"int",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_int},
{"length",	Node_builtin,	 LEX_LENGTH,	A(0)|A(1),	do_length},
{"log",		Node_builtin,	 LEX_BUILTIN,	A(1),		do_log},
{"lshift",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_lshift},
{"match",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_match},
{"mktime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1),	do_mktime},
{"next",	Node_K_next,	 LEX_NEXT,	0,		0},
{"nextfile",	Node_K_nextfile, LEX_NEXTFILE,	GAWKX,		0},
{"or",		Node_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_or},
{"print",	Node_K_print,	 LEX_PRINT,	0,		0},
{"printf",	Node_K_printf,	 LEX_PRINTF,	0,		0},
{"rand",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0),	do_rand},
{"return",	Node_K_return,	 LEX_RETURN,	NOT_OLD,	0},
{"rshift",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_rshift},
{"sin",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_sin},
{"split",	Node_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_split},
{"sprintf",	Node_builtin,	 LEX_BUILTIN,	0,		do_sprintf},
{"sqrt",	Node_builtin,	 LEX_BUILTIN,	A(1),		do_sqrt},
{"srand",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(0)|A(1), do_srand},
#if defined(GAWKDEBUG) || defined(ARRAYDEBUG) /* || ... */
{"stopme",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(0),	stopme},
#endif
{"strftime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(0)|A(1)|A(2), do_strftime},
{"strtonum",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_strtonum},
{"sub",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_sub},
{"substr",	Node_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_substr},
{"system",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_system},
{"systime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(0),	do_systime},
{"tolower",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_tolower},
{"toupper",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_toupper},
{"while",	Node_K_while,	 LEX_WHILE,	0,		0},
{"xor",		Node_builtin,    LEX_BUILTIN,	GAWKX|A(2),	do_xor},
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
#endif /* MBS_SUPPORT */

/* getfname --- return name of a builtin function (for pretty printing) */

const char *
getfname(register NODE *(*fptr)(NODE *))
{
	register int i, j;

	j = sizeof(tokentab) / sizeof(tokentab[0]);
	/* linear search, no other way to do it */
	for (i = 0; i < j; i++) 
		if (tokentab[i].ptr == fptr)
			return tokentab[i].operator;

	return NULL;
}

/* yyerror --- print a syntax error message, show where */

/*
 * Function identifier purposely indented to avoid mangling
 * by ansi2knr.  Sigh.
 */

static void
#if defined(HAVE_STDARG_H) && defined(__STDC__) && __STDC__
  yyerror(const char *m, ...)
#else
/* VARARGS0 */
  yyerror(va_alist)
  va_dcl
#endif
{
	va_list args;
	const char *mesg = NULL;
	register char *bp, *cp;
	char *scan;
	char *buf;
	int count;
	static char end_of_file_line[] = "(END OF FILE)";
	char save;

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
	 */
	save = *bp;
	*bp = '\0';

	msg("%.*s", (int) (bp - thisline), thisline);

	*bp = save;

#if defined(HAVE_STDARG_H) && defined(__STDC__) && __STDC__
	va_start(args, m);
	if (mesg == NULL)
		mesg = m;
#else
	va_start(args);
	if (mesg == NULL)
		mesg = va_arg(args, char *);
#endif
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
	free(buf);
}

/* get_src_buf --- read the next buffer of source program */

static char *
get_src_buf()
{
	static int samefile = FALSE;
	static int nextfile = 0;
	static char *buf = NULL;
	static int fd;
	int n;
	register char *scan;
	static size_t len = 0;
	static int did_newline = FALSE;
	int newfile;
	struct stat sbuf;

#	define	SLOP	128	/* enough space to hold most source lines */

again:
	newfile = FALSE;
	if (nextfile > numfiles)
		return NULL;

	if (srcfiles[nextfile].stype == CMDLINE) {
		if (len == 0) {
			len = strlen(srcfiles[nextfile].val);
			if (len == 0) {
				/*
				 * Yet Another Special case:
				 *	gawk '' /path/name
				 * Sigh.
				 */
				static int warned = FALSE;

				if (do_lint && ! warned) {
					warned = TRUE;
					lintwarn(_("empty program text on command line"));
				}
				++nextfile;
				goto again;
			}
			sourceline = 1;
			lexptr = lexptr_begin = srcfiles[nextfile].val;
			lexend = lexptr + len;
		} else if (! did_newline && *(lexptr-1) != '\n') {
			/*
			 * The following goop is to ensure that the source
			 * ends with a newline and that the entire current
			 * line is available for error messages.
			 */
			int offset;

			did_newline = TRUE;
			offset = lexptr - lexeme;
			for (scan = lexeme; scan > lexptr_begin; scan--)
				if (*scan == '\n') {
					scan++;
					break;
				}
			len = lexptr - scan;
			emalloc(buf, char *, len+1, "get_src_buf");
			memcpy(buf, scan, len);
			thisline = buf;
			lexptr = buf + len;
			*lexptr = '\n';
			lexeme = lexptr - offset;
			lexptr_begin = buf;
			lexend = lexptr + 1;
		} else {
			len = 0;
			lexeme = lexptr = lexptr_begin = NULL;
		}
		if (lexptr == NULL && ++nextfile <= numfiles)
			goto again;
		return lexptr;
	}
	if (! samefile) {
		source = srcfiles[nextfile].val;
		if (source == NULL) {
			if (buf != NULL) {
				free(buf);
				buf = NULL;
			}
			len = 0;
			return lexeme = lexptr = lexptr_begin = NULL;
		}
		fd = pathopen(source);
		if (fd <= INVALID_HANDLE) {
			char *in;

			/* suppress file name and line no. in error mesg */
			in = source;
			source = NULL;
			fatal(_("can't open source file `%s' for reading (%s)"),
				in, strerror(errno));
		}
		len = optimal_bufsize(fd, & sbuf);
		newfile = TRUE;
		if (buf != NULL)
			free(buf);
		emalloc(buf, char *, len + SLOP, "get_src_buf");
		lexptr_begin = buf + SLOP;
		samefile = TRUE;
		sourceline = 1;
	} else {
		/*
		 * Here, we retain the current source line (up to length SLOP)
		 * in the beginning of the buffer that was overallocated above
		 */
		int offset;
		int linelen;

		offset = lexptr - lexeme;
		for (scan = lexeme; scan > lexptr_begin; scan--)
			if (*scan == '\n') {
				scan++;
				break;
			}
		linelen = lexptr - scan;
		if (linelen > SLOP)
			linelen = SLOP;
		thisline = buf + SLOP - linelen;
		memcpy(thisline, scan, linelen);
		lexeme = buf + SLOP - offset;
		lexptr_begin = thisline;
	}
	n = read(fd, buf + SLOP, len);
	if (n == -1)
		fatal(_("can't read sourcefile `%s' (%s)"),
			source, strerror(errno));
	if (n == 0) {
		if (newfile) {
			static int warned = FALSE;

			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn(_("source file `%s' is empty"), source);
			}
		}
		if (fd != fileno(stdin)) /* safety */
			close(fd);
		samefile = FALSE;
		nextfile++;
		if (lexeme)
			*lexeme = '\0';
		len = 0;
		goto again;
	}
	lexptr = buf + SLOP;
	lexend = lexptr + n;
	return buf;
}

/* tokadd --- add a character to the token buffer */

#define	tokadd(x) (*tok++ = (x), tok == tokend ? tokexpand() : tok)

/* tokexpand --- grow the token buffer */

char *
tokexpand()
{
	static int toksize = 60;
	int tokoffset;

	tokoffset = tok - tokstart;
	toksize *= 2;
	if (tokstart != NULL)
		erealloc(tokstart, char *, toksize, "tokexpand");
	else
		emalloc(tokstart, char *, toksize, "tokexpand");
	tokend = tokstart + toksize;
	tok = tokstart + tokoffset;
	return tok;
}

/* nextc --- get the next input character */

#ifdef MBS_SUPPORT

static int
nextc(void)
{
	if (gawk_mb_cur_max > 1)	{
		/* Update the buffer index.  */
		cur_ring_idx = (cur_ring_idx == RING_BUFFER_SIZE - 1)? 0 :
			cur_ring_idx + 1;

		/* Did we already check the current character?  */
		if (cur_char_ring[cur_ring_idx] == 0) {
			/* No, we need to check the next character on the buffer.  */
			int idx, work_ring_idx = cur_ring_idx;
			mbstate_t tmp_state;
			size_t mbclen;

			if (!lexptr || lexptr >= lexend)
				if (!get_src_buf()) {
					return EOF;
				}

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
	}
	else {
		int c;

		if (lexptr && lexptr < lexend)
			c = (int) (unsigned char) *lexptr++;
		else if (get_src_buf())
			c = (int) (unsigned char) *lexptr++;
		else
			c = EOF;

		return c;
	}
}

#else /* MBS_SUPPORT */

#if GAWKDEBUG
int
nextc(void)
{
	int c;

	if (lexptr && lexptr < lexend)
		c = (int) (unsigned char) *lexptr++;
	else if (get_src_buf())
		c = (int) (unsigned char) *lexptr++;
	else
		c = EOF;

	return c;
}
#else
#define	nextc()	((lexptr && lexptr < lexend) ? \
		    ((int) (unsigned char) *lexptr++) : \
		    (get_src_buf() ? ((int) (unsigned char) *lexptr++) : EOF) \
		)
#endif

#endif /* MBS_SUPPORT */

/* pushback --- push a character back on the input */

#ifdef MBS_SUPPORT

static void
pushback(void)
{
	if (gawk_mb_cur_max > 1) {
		cur_ring_idx = (cur_ring_idx == 0)? RING_BUFFER_SIZE - 1 :
			cur_ring_idx - 1;
		(lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr);
	} else
		(lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr);
}

#else

#define pushback() (lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr)

#endif /* MBS_SUPPORT */

/* allow_newline --- allow newline after &&, ||, ? and : */

static void
allow_newline(void)
{
	int c;

	for (;;) {
		c = nextc();
		if (c == EOF)
			break;
		if (c == '#') {
			while ((c = nextc()) != '\n' && c != EOF)
				continue;
			if (c == EOF)
				break;
		}
		if (c == '\n')
			sourceline++;
		if (! ISSPACE(c)) {
			pushback();
			break;
		}
	}
}

/* yylex --- Read the input and turn it into tokens. */

static int
yylex(void)
{
	register int c;
	int seen_e = FALSE;		/* These are for numbers */
	int seen_point = FALSE;
	int esc_seen;		/* for literal strings */
	int low, mid, high;
	static int did_newline = FALSE;
	char *tokkey;
	static int lasttok = 0, eof_warned = FALSE;
	int inhex = FALSE;
	int intlstr = FALSE;

	if (nextc() == EOF) {
		if (lasttok != NEWLINE) {
			lasttok = NEWLINE;
			if (do_lint && ! eof_warned) {
				lintwarn(_("source file does not end in newline"));
				eof_warned = TRUE;
			}
			return NEWLINE;	/* fake it */
		}
		return 0;
	}
	pushback();
#if defined OS2 || defined __EMX__
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
#ifdef MBS_SUPPORT
			if (gawk_mb_cur_max == 1 || nextc_is_1stbyte)
#endif
			switch (c) {
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
				if ((c = nextc()) == EOF) {
					yyerror(_("unterminated regexp ends with `\\' at end of file"));
					return lasttok = REGEXP; /* kludge */
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

				tokadd('\0');
				yylval.sval = tokstart;
				return lasttok = REGEXP;
			case '\n':
				pushback();
				yyerror(_("unterminated regexp"));
				return lasttok = REGEXP;	/* kludge */
			case EOF:
				yyerror(_("unterminated regexp at end of file"));
				return lasttok = REGEXP;	/* kludge */
			}
			tokadd(c);
		}
	}
retry:
	while ((c = nextc()) == ' ' || c == '\t')
		continue;

	lexeme = lexptr ? lexptr - 1 : lexptr;
	thisline = NULL;
	tok = tokstart;
	yylval.nodetypeval = Node_illegal;

#ifdef MBS_SUPPORT
	if (gawk_mb_cur_max == 1 || nextc_is_1stbyte)
#endif
	switch (c) {
	case EOF:
		if (lasttok != NEWLINE) {
			lasttok = NEWLINE;
			if (do_lint && ! eof_warned) {
				lintwarn(_("source file does not end in newline"));
				eof_warned = TRUE;
			}
			return NEWLINE;	/* fake it */
		}
		return 0;

	case '\n':
		sourceline++;
		return lasttok = NEWLINE;

	case '#':		/* it's a comment */
		while ((c = nextc()) != '\n') {
			if (c == EOF) {
				if (lasttok != NEWLINE) {
					lasttok = NEWLINE;
					if (do_lint && ! eof_warned) {
						lintwarn(
				_("source file does not end in newline"));
						eof_warned = TRUE;
					}
					return NEWLINE;	/* fake it */
				}
				return 0;
			}
		}
		sourceline++;
		return lasttok = NEWLINE;

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
			while ((c = nextc()) == ' ' || c == '\t')
				continue;
			if (c == '#') {
				if (do_lint)
					lintwarn(
		_("use of `\\ #...' line continuation is not portable"));
				while ((c = nextc()) != '\n')
					if (c == EOF)
						break;
			}
			pushback();
		}
#endif /* RELAXED_CONTINUATION */
		if (nextc() == '\n') {
			sourceline++;
			goto retry;
		} else {
			yyerror(_("backslash not last character on line"));
			exit(1);
		}
		break;

	case ':':
	case '?':
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
		/* FALL THROUGH */
	case '$':
	case ';':
	case '{':
	case ',':
	case '[':
	case ']':
		return lasttok = c;

	case '*':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_times;
			return lasttok = ASSIGNOP;
		} else if (do_posix) {
			pushback();
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
				yylval.nodetypeval = Node_assign_exp;
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
				return lasttok = '^';
			}
		}
		pushback();
		return lasttok = '*';

	case '/':
		if (nextc() == '=') {
			pushback();
			return lasttok = SLASH_BEFORE_EQUAL;
		}
		pushback();
		return lasttok = '/';

	case '%':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_assign_mod;
			return lasttok = ASSIGNOP;
		}
		pushback();
		return lasttok = '%';

	case '^':
	{
		static int did_warn_op = FALSE, did_warn_assgn = FALSE;

		if (nextc() == '=') {
			if (do_lint_old && ! did_warn_assgn) {
				did_warn_assgn = TRUE;
				warning(_("operator `^=' is not supported in old awk"));
			}
			yylval.nodetypeval = Node_assign_exp;
			return lasttok = ASSIGNOP;
		}
		pushback();
		if (do_lint_old && ! did_warn_op) {
			did_warn_op = TRUE;
			warning(_("operator `^' is not supported in old awk"));
		}
		return lasttok = '^';
	}

	case '+':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_plus;
			return lasttok = ASSIGNOP;
		}
		if (c == '+')
			return lasttok = INCREMENT;
		pushback();
		return lasttok = '+';

	case '!':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_notequal;
			return lasttok = RELOP;
		}
		if (c == '~') {
			yylval.nodetypeval = Node_nomatch;
			return lasttok = MATCHOP;
		}
		pushback();
		return lasttok = '!';

	case '<':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_leq;
			return lasttok = RELOP;
		}
		yylval.nodetypeval = Node_less;
		pushback();
		return lasttok = '<';

	case '=':
		if (nextc() == '=') {
			yylval.nodetypeval = Node_equal;
			return lasttok = RELOP;
		}
		yylval.nodetypeval = Node_assign;
		pushback();
		return lasttok = ASSIGN;

	case '>':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_geq;
			return lasttok = RELOP;
		} else if (c == '>') {
			yylval.nodetypeval = Node_redirect_append;
			return lasttok = IO_OUT;
		}
		pushback();
		if (in_print && in_parens == 0) {
			yylval.nodetypeval = Node_redirect_output;
			return lasttok = IO_OUT;
		}
		yylval.nodetypeval = Node_greater;
		return lasttok = '>';

	case '~':
		yylval.nodetypeval = Node_match;
		return lasttok = MATCHOP;

	case '}':
		/*
		 * Added did newline stuff.  Easier than
		 * hacking the grammar.
		 */
		if (did_newline) {
			did_newline = FALSE;
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
				exit(1);
			}
#ifdef MBS_SUPPORT
			if (gawk_mb_cur_max == 1 || nextc_is_1stbyte)
#endif
			if (c == '\\') {
				c = nextc();
				if (c == '\n') {
					sourceline++;
					continue;
				}
				esc_seen = TRUE;
				tokadd('\\');
			}
			if (c == EOF) {
				pushback();
				yyerror(_("unterminated string"));
				exit(1);
			}
			tokadd(c);
		}
		yylval.nodeval = make_str_node(tokstart,
					tok - tokstart, esc_seen ? SCAN : 0);
		yylval.nodeval->flags |= PERM;
		if (intlstr) {
			yylval.nodeval->flags |= INTLSTR;
			intlstr = FALSE;
			if (do_intl)
				dumpintlstr(yylval.nodeval->stptr,
						yylval.nodeval->stlen);
 		}
		return lasttok = YSTRING;

	case '-':
		if ((c = nextc()) == '=') {
			yylval.nodetypeval = Node_assign_minus;
			return lasttok = ASSIGNOP;
		}
		if (c == '-')
			return lasttok = DECREMENT;
		pushback();
		return lasttok = '-';

	case '.':
		c = nextc();
		pushback();
		if (! ISDIGIT(c))
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
				if (tok == tokstart + 2)
					inhex = TRUE;
				break;
			case '.':
				if (seen_point) {
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
				if ((c = nextc()) == '-' || c == '+')
					tokadd(c);
				else
					pushback();
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
		if (c != EOF)
			pushback();
		else if (do_lint && ! eof_warned) {
			lintwarn(_("source file does not end in newline"));
			eof_warned = TRUE;
		}
		tokadd('\0');
		if (! do_traditional && isnondecimal(tokstart)) {
			static short warned = FALSE;
			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn("numeric constant `%.*s' treated as octal or hexadecimal",
					strlen(tokstart)-1, tokstart);
			}
			yylval.nodeval = make_number(nondec2awknum(tokstart, strlen(tokstart)));
		} else
			yylval.nodeval = make_number(atof(tokstart));
		yylval.nodeval->flags |= PERM;
		return lasttok = YNUMBER;

	case '&':
		if ((c = nextc()) == '&') {
			yylval.nodetypeval = Node_and;
			allow_newline();
			return lasttok = LEX_AND;
		}
		pushback();
		return lasttok = '&';

	case '|':
		if ((c = nextc()) == '|') {
			yylval.nodetypeval = Node_or;
			allow_newline();
			return lasttok = LEX_OR;
		} else if (! do_traditional && c == '&') {
			yylval.nodetypeval = Node_redirect_twoway;
			return lasttok = (in_print && in_parens == 0 ? IO_OUT : IO_IN);
		}
		pushback();
		if (in_print && in_parens == 0) {
			yylval.nodetypeval = Node_redirect_pipe;
			return lasttok = IO_OUT;
		} else {
			yylval.nodetypeval = Node_redirect_pipein;
			return lasttok = IO_IN;
		}
	}

	if (c != '_' && ! ISALPHA(c)) {
		yyerror(_("invalid char '%c' in expression"), c);
		exit(1);
	}

	/*
	 * Lots of fog here.  Consider:
	 *
	 * print "xyzzy"$_"foo"
	 *
	 * Without the check for ` lasttok != '$'' ', this is parsed as
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
	while (is_identchar(c)) {
		tokadd(c);
		c = nextc();
	}
	tokadd('\0');
	emalloc(tokkey, char *, tok - tokstart, "yylex");
	memcpy(tokkey, tokstart, tok - tokstart);
	if (c != EOF)
		pushback();
	else if (do_lint && ! eof_warned) {
		lintwarn(_("source file does not end in newline"));
		eof_warned = TRUE;
	}

	/* See if it is a special token. */
	low = 0;
	high = (sizeof(tokentab) / sizeof(tokentab[0])) - 1;
	while (low <= high) {
		int i;

		mid = (low + high) / 2;
		c = *tokstart - tokentab[mid].operator[0];
		i = c ? c : strcmp(tokstart, tokentab[mid].operator);

		if (i < 0)		/* token < mid */
			high = mid - 1;
		else if (i > 0)		/* token > mid */
			low = mid + 1;
		else {
			if (do_lint) {
				if (tokentab[mid].flags & GAWKX)
					lintwarn(_("`%s' is a gawk extension"),
						tokentab[mid].operator);
				if (tokentab[mid].flags & RESX)
					lintwarn(_("`%s' is a Bell Labs extension"),
						tokentab[mid].operator);
				if (tokentab[mid].flags & NOT_POSIX)
					lintwarn(_("POSIX does not allow `%s'"),
						tokentab[mid].operator);
			}
			if (do_lint_old && (tokentab[mid].flags & NOT_OLD))
				warning(_("`%s' is not supported in old awk"),
						tokentab[mid].operator);
			if ((do_traditional && (tokentab[mid].flags & GAWKX))
			    || (do_posix && (tokentab[mid].flags & NOT_POSIX)))
				break;
			if (tokentab[mid].class == LEX_BUILTIN
			    || tokentab[mid].class == LEX_LENGTH
			   )
				yylval.lval = mid;
			else
				yylval.nodetypeval = tokentab[mid].value;

			free(tokkey);
			return lasttok = tokentab[mid].class;
		}
	}

	yylval.sval = tokkey;
	if (*lexptr == '(')
		return lasttok = FUNC_CALL;
	else {
		static short goto_warned = FALSE;

#define SMART_ALECK	1
		if (SMART_ALECK && do_lint
		    && ! goto_warned && strcasecmp(tokkey, "goto") == 0) {
			goto_warned = TRUE;
			lintwarn(_("`goto' considered harmful!\n"));
		}
		return lasttok = NAME;
	}
}

/* node_common --- common code for allocating a new node */

static NODE *
node_common(NODETYPE op)
{
	register NODE *r;

	getnode(r);
	r->type = op;
	r->flags = MALLOC;
	if (r->type == Node_var)
		r->flags |= UNINITIALIZED;
	/* if lookahead is NL, lineno is 1 too high */
	if (lexeme && *lexeme == '\n')
		r->source_line = sourceline - 1;
	else
		r->source_line = sourceline;
	r->source_file = source;
	return r;
}

/* node --- allocates a node with defined lnode and rnode. */

NODE *
node(NODE *left, NODETYPE op, NODE *right)
{
	register NODE *r;

	r = node_common(op);
	r->lnode = left;
	r->rnode = right;
	return r;
}

/* snode ---	allocate a node with defined subnode and builtin for builtin
		functions. Checks for arg. count and supplies defaults where
		possible. */

static NODE *
snode(NODE *subn, NODETYPE op, int idx)
{
	register NODE *r;
	register NODE *n;
	int nexp = 0;
	int args_allowed;

	r = node_common(op);

	/* traverse expression list to see how many args. given */
	for (n = subn; n != NULL; n = n->rnode) {
		nexp++;
		if (nexp > 5)
			break;
	}

	/* check against how many args. are allowed for this builtin */
	args_allowed = tokentab[idx].flags & ARGS;
	if (args_allowed && (args_allowed & A(nexp)) == 0)
		fatal(_("%d is invalid as number of arguments for %s"),
				nexp, tokentab[idx].operator);

	r->builtin = tokentab[idx].ptr;

	/* special case processing for a few builtins */
	if (nexp == 0 && r->builtin == do_length) {
		subn = node(node(make_number(0.0), Node_field_spec, (NODE *) NULL),
		            Node_expression_list,
			    (NODE *) NULL);
	} else if (r->builtin == do_match) {
		static short warned = FALSE;

		if (subn->rnode->lnode->type != Node_regex)
			subn->rnode->lnode = mk_rexp(subn->rnode->lnode);

		if (subn->rnode->rnode != NULL) {	/* 3rd argument there */
			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn(_("match: third argument is a gawk extension"));
			}
			if (do_traditional)
				fatal(_("match: third argument is a gawk extension"));
		}
	} else if (r->builtin == do_sub || r->builtin == do_gsub) {
		if (subn->lnode->type != Node_regex)
			subn->lnode = mk_rexp(subn->lnode);
		if (nexp == 2)
			append_right(subn, node(node(make_number(0.0),
						     Node_field_spec,
						     (NODE *) NULL),
					        Node_expression_list,
						(NODE *) NULL));
		else if (subn->rnode->rnode->lnode->type == Node_val) {
			if (do_lint)
				lintwarn(_("%s: string literal as last arg of substitute has no effect"),
					(r->builtin == do_sub) ? "sub" : "gsub");
		} else if (! isassignable(subn->rnode->rnode->lnode)) {
			yyerror(_("%s third parameter is not a changeable object"),
				(r->builtin == do_sub) ? "sub" : "gsub");
		}
	} else if (r->builtin == do_gensub) {
		if (subn->lnode->type != Node_regex)
			subn->lnode = mk_rexp(subn->lnode);
		if (nexp == 3)
			append_right(subn, node(node(make_number(0.0),
						     Node_field_spec,
						     (NODE *) NULL),
					        Node_expression_list,
						(NODE *) NULL));
	} else if (r->builtin == do_split) {
		if (nexp == 2)
			append_right(subn,
			    node(FS_node, Node_expression_list, (NODE *) NULL));
		n = subn->rnode->rnode->lnode;
		if (n->type != Node_regex)
			subn->rnode->rnode->lnode = mk_rexp(n);
		if (nexp == 2)
			subn->rnode->rnode->lnode->re_flags |= FS_DFLT;
	} else if (r->builtin == do_close) {
		static short warned = FALSE;

		if ( nexp == 2) {
			if (do_lint && nexp == 2 && ! warned) {
				warned = TRUE;
				lintwarn(_("close: second argument is a gawk extension"));
			}
			if (do_traditional)
				fatal(_("close: second argument is a gawk extension"));
		}
	} else if (do_intl					/* --gen-po */
			&& r->builtin == do_dcgettext		/* dcgettext(...) */
			&& subn->lnode->type == Node_val	/* 1st arg is constant */
			&& (subn->lnode->flags & STRCUR) != 0) {	/* it's a string constant */
		/* ala xgettext, dcgettext("some string" ...) dumps the string */
		NODE *str = subn->lnode;

		if ((str->flags & INTLSTR) != 0)
			warning(_("use of dcgettext(_\"...\") is incorrect: remove leading underscore"));
			/* don't dump it, the lexer already did */
		else
			dumpintlstr(str->stptr, str->stlen);
	} else if (do_intl					/* --gen-po */
			&& r->builtin == do_dcngettext	/* dcngettext(...) */
			&& subn->lnode->type == Node_val	/* 1st arg is constant */
			&& (subn->lnode->flags & STRCUR) != 0	/* it's a string constant */
			&& subn->rnode->lnode->type == Node_val	/* 2nd arg is constant too */
			&& (subn->rnode->lnode->flags & STRCUR) != 0) {	/* it's a string constant */
		/* ala xgettext, dcngettext("some string", "some plural" ...) dumps the string */
		NODE *str1 = subn->lnode;
		NODE *str2 = subn->rnode->lnode;

		if (((str1->flags | str2->flags) & INTLSTR) != 0)
			warning(_("use of dcngettext(_\"...\") is incorrect: remove leading underscore"));
		else
			dumpintlstr2(str1->stptr, str1->stlen, str2->stptr, str2->stlen);
	}

	r->subnode = subn;
	if (r->builtin == do_sprintf) {
		count_args(r);
		r->lnode->printf_count = r->printf_count; /* hack */
	}
	return r;
}

/* make_for_loop --- build a for loop */

static NODE *
make_for_loop(NODE *init, NODE *cond, NODE *incr)
{
	register FOR_LOOP_HEADER *r;
	NODE *n;

	emalloc(r, FOR_LOOP_HEADER *, sizeof(FOR_LOOP_HEADER), "make_for_loop");
	getnode(n);
	n->type = Node_illegal;
	r->init = init;
	r->cond = cond;
	r->incr = incr;
	n->sub.nodep.r.hd = r;
	return n;
}

/* dup_parms --- return TRUE if there are duplicate parameters */

static int
dup_parms(NODE *func)
{
	register NODE *np;
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
			free(names);
			return TRUE;
		}
		names[i++] = np->param;
	}

	dups = 0;
	for (i = 1; i < count; i++) {
		for (j = 0; j < i; j++) {
			if (strcmp(names[i], names[j]) == 0) {
				dups++;
				error(
	_("function `%s': parameter #%d, `%s', duplicates parameter #%d"),
					fname, i+1, names[j], j+1);
			}
		}
	}

	free(names);
	return (dups > 0 ? TRUE : FALSE);
}

/* parms_shadow --- check if parameters shadow globals */

static int
parms_shadow(const char *fname, NODE *func)
{
	int count, i;
	int ret = FALSE;

	if (fname == NULL || func == NULL)	/* error earlier */
		return FALSE;

	count = func->lnode->param_cnt;

	if (count == 0)		/* no args, no problem */
		return FALSE;

	/*
	 * Use warning() and not lintwarn() so that can warn
	 * about all shadowed parameters.
	 */
	for (i = 0; i < count; i++) {
		if (lookup(func->parmlist[i]) != NULL) {
			warning(
	_("function `%s': parameter `%s' shadows global variable"),
					fname, func->parmlist[i]);
			ret = TRUE;
		}
	}

	return ret;
}

/*
 * install:
 * Install a name in the symbol table, even if it is already there.
 * Caller must check against redefinition if that is desired. 
 */

NODE *
install(char *name, NODE *value)
{
	register NODE *hp;
	register size_t len;
	register int bucket;

	var_count++;
	len = strlen(name);
	bucket = hash(name, len, (unsigned long) HASHSIZE);
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

/* lookup --- find the most recent hash node for name installed by install */

NODE *
lookup(const char *name)
{
	register NODE *bucket;
	register size_t len;

	len = strlen(name);
	for (bucket = variables[hash(name, len, (unsigned long) HASHSIZE)];
			bucket != NULL; bucket = bucket->hnext)
		if (bucket->hlength == len && STREQN(bucket->hname, name, len))
			return bucket->hvalue;

	return NULL;
}

/* var_comp --- compare two variable names */

static int
var_comp(const void *v1, const void *v2)
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

static void
valinfo(NODE *n, FILE *fp)
{
	if (n->flags & STRING) {
		fprintf(fp, "string (");
		pp_string_fp(fp, n->stptr, n->stlen, '"', FALSE);
		fprintf(fp, ")\n");
	} else if (n->flags & NUMBER)
		fprintf(fp, "number (%.17g)\n", n->numbr);
	else if (n->flags & STRCUR) {
		fprintf(fp, "string value (");
		pp_string_fp(fp, n->stptr, n->stlen, '"', FALSE);
		fprintf(fp, ")\n");
	} else if (n->flags & NUMCUR)
		fprintf(fp, "number value (%.17g)\n", n->numbr);
	else
		fprintf(fp, "?? flags %s\n", flags2str(n->flags));
}


/* dump_vars --- dump the symbol table */

void
dump_vars(const char *fname)
{
	int i, j;
	NODE **table;
	NODE *p;
	FILE *fp;

	emalloc(table, NODE **, var_count * sizeof(NODE *), "dump_vars");

	if (fname == NULL)
		fp = stderr;
	else if ((fp = fopen(fname, "w")) == NULL) {
		warning(_("could not open `%s' for writing (%s)"), fname, strerror(errno));
		warning(_("sending profile to standard error"));
		fp = stderr;
	}

	for (i = j = 0; i < HASHSIZE; i++)
		for (p = variables[i]; p != NULL; p = p->hnext)
			table[j++] = p;

	assert(j == var_count);

	/* Shazzam! */
	qsort(table, j, sizeof(NODE *), var_comp);

	for (i = 0; i < j; i++) {
		p = table[i];
		if (p->hvalue->type == Node_func)
			continue;
		fprintf(fp, "%.*s: ", (int) p->hlength, p->hname);
		if (p->hvalue->type == Node_var_array)
			fprintf(fp, "array, %ld elements\n", p->hvalue->table_size);
		else if (p->hvalue->type == Node_var)
			valinfo(p->hvalue->var_value, fp);
		else {
			NODE **lhs = get_lhs(p->hvalue, NULL, FALSE);

			valinfo(*lhs, fp);
		}
	}

	if (fp != stderr && fclose(fp) != 0)
		warning(_("%s: close failed (%s)"), fname, strerror(errno));

	free(table);
}

/* release_all_vars --- free all variable memory */

void
release_all_vars()
{
	int i;
	NODE *p, *next;

	for (i = 0; i < HASHSIZE; i++)
		for (p = variables[i]; p != NULL; p = next) {
			next = p->hnext;

			if (p->hvalue->type == Node_func)
				continue;
			else if (p->hvalue->type == Node_var_array)
				assoc_clear(p->hvalue);
			else if (p->hvalue->type == Node_var)
				unref(p->hvalue->var_value);
			else {
				NODE **lhs = get_lhs(p->hvalue, NULL, FALSE);

				unref((*lhs)->var_value);
			}
			unref(p);
	}
}

/* finfo --- for use in comparison and sorting of function names */

struct finfo {
	const char *name;
	size_t nlen;
	NODE *func;
};

/* fcompare --- comparison function for qsort */

static int
fcompare(const void *p1, const void *p2)
{
	const struct finfo *f1, *f2;
	int minlen;

	f1 = (const struct finfo *) p1;
	f2 = (const struct finfo *) p2;

	if (f1->nlen > f2->nlen)
		minlen = f2->nlen;
	else
		minlen = f1->nlen;

	return strncmp(f1->name, f2->name, minlen);
}

/* dump_funcs --- print all functions */

void
dump_funcs()
{
	int i, j;
	NODE *p;
	static struct finfo *tab = NULL;

	if (func_count == 0)
		return;

	/*
	 * Walk through symbol table countng functions.
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

	if (tab == NULL)
		emalloc(tab, struct finfo *, j * sizeof(struct finfo), "dump_funcs");

	/* now walk again, copying info */
	for (i = j = 0; i < HASHSIZE; i++) {
		for (p = variables[i]; p != NULL; p = p->hnext) {
			if (p->hvalue->type == Node_func) {
				tab[j].name = p->hname;
				tab[j].nlen = p->hlength;
				tab[j].func = p->hvalue;
				j++;
			}
		}
	}


	/* Shazzam! */
	qsort(tab, j, sizeof(struct finfo), fcompare);

	for (i = 0; i < j; i++)
		pp_func(tab[i].name, tab[i].nlen, tab[i].func);

	free(tab);
}

/* shadow_funcs --- check all functions for parameters that shadow globals */

void
shadow_funcs()
{
	int i, j;
	NODE *p;
	struct finfo *tab;
	static int calls = 0;
	int shadow = FALSE;

	if (func_count == 0)
		return;

	if (calls++ != 0)
		fatal(_("shadow_funcs() called twice!"));

	emalloc(tab, struct finfo *, func_count * sizeof(struct finfo), "shadow_funcs");

	for (i = j = 0; i < HASHSIZE; i++) {
		for (p = variables[i]; p != NULL; p = p->hnext) {
			if (p->hvalue->type == Node_func) {
				tab[j].name = p->hname;
				tab[j].nlen = p->hlength;
				tab[j].func = p->hvalue;
				j++;
			}
		}
	}

	assert(j == func_count);

	/* Shazzam! */
	qsort(tab, func_count, sizeof(struct finfo), fcompare);

	for (i = 0; i < j; i++)
		shadow |= parms_shadow(tab[i].name, tab[i].func);

	free(tab);

	/* End with fatal if the user requested it.  */
	if (shadow && lintfunc != warning)
		lintwarn(_("there were shadowed variables."));
}

/*
 * append_right:
 * Add new to the rightmost branch of LIST.  This uses n^2 time, so we make
 * a simple attempt at optimizing it.
 */

static NODE *
append_right(NODE *list, NODE *new)
{
	register NODE *oldlist;
	static NODE *savefront = NULL, *savetail = NULL;

	if (list == NULL || new == NULL)
		return list;

	oldlist = list;
	if (savefront == oldlist) {
		savetail = savetail->rnode = new;
		return oldlist;
	} else
		savefront = oldlist;
	while (list->rnode != NULL)
		list = list->rnode;
	savetail = list->rnode = new;
	return oldlist;
}

/*
 * append_pattern:
 * A wrapper around append_right, used for rule lists.
 */
static inline NODE *
append_pattern(NODE **list, NODE *patt)
{
	NODE *n = node(patt, Node_rule_node, (NODE *) NULL);

	if (*list == NULL)
		*list = n;
	else {
		NODE *n1 = node(n, Node_rule_list, (NODE *) NULL);
		if ((*list)->type != Node_rule_list)
			*list = node(*list, Node_rule_list, n1);
		else
			(void) append_right(*list, n1);
	}
	return n;
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

static void
func_install(NODE *params, NODE *def)
{
	NODE *r, *n, *thisfunc;
	char **pnames, *names, *sp;
	size_t pcount = 0, space = 0;
	int i;

	/* check for function foo(foo) { ... }.  bleah. */
	for (n = params->rnode; n != NULL; n = n->rnode) {
		if (strcmp(n->param, params->param) == 0)
			fatal(_("function `%s': can't use function name as parameter name"),
					params->param); 
	}

	thisfunc = NULL;	/* turn off warnings */

	/* symbol table managment */
	pop_var(params, FALSE);
	r = lookup(params->param);
	if (r != NULL) {
		fatal(_("function name `%s' previously defined"), params->param);
	} else if (params->param == builtin_func)	/* not a valid function name */
		goto remove_params;

	/* install the function */
	thisfunc = node(params, Node_func, def);
	(void) install(params->param, thisfunc);

	/* figure out amount of space to allocate for variable names */
	for (n = params->rnode; n != NULL; n = n->rnode) {
		pcount++;
		space += strlen(n->param) + 1;
	}

	/* allocate it and fill it in */
	if (pcount != 0) {
		emalloc(names, char *, space, "func_install");
		emalloc(pnames, char **, pcount * sizeof(char *), "func_install");
		sp = names;
		for (i = 0, n = params->rnode; i < pcount; i++, n = n->rnode) {
			pnames[i] = sp;
			strcpy(sp, n->param);
			sp += strlen(n->param) + 1;
		}
		thisfunc->parmlist = pnames;
	} else {
		thisfunc->parmlist = NULL;
	}

	/* update lint table info */
	func_use(params->param, FUNC_DEFINE);

	func_count++;	/* used by profiling / pretty printer */

remove_params:
	/* remove params from symbol table */
	pop_params(params->rnode);
}

/* pop_var --- remove a variable from the symbol table */

static void
pop_var(NODE *np, int freeit)
{
	register NODE *bucket, **save;
	register size_t len;
	char *name;

	name = np->param;
	len = strlen(name);
	save = &(variables[hash(name, len, (unsigned long) HASHSIZE)]);
	for (bucket = *save; bucket != NULL; bucket = bucket->hnext) {
		if (len == bucket->hlength && STREQN(bucket->hname, name, len)) {
			var_count--;
			*save = bucket->hnext;
			freenode(bucket);
			if (freeit)
				free(np->param);
			return;
		}
		save = &(bucket->hnext);
	}
}

/* pop_params --- remove list of function parameters from symbol table */

/*
 * pop parameters out of the symbol table. do this in reverse order to
 * avoid reading freed memory if there were duplicated parameters.
 */
static void
pop_params(NODE *params)
{
	if (params == NULL)
		return;
	pop_params(params->rnode);
	pop_var(params, TRUE);
}

/* make_param --- make NAME into a function parameter */

static NODE *
make_param(char *name)
{
	NODE *r;

	getnode(r);
	r->type = Node_param_list;
	r->rnode = NULL;
	r->param = name;
	r->param_cnt = param_counter++;
	return (install(name, r));
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
	ind = hash(name, len, HASHSIZE);

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
				lintwarn(_("function `%s' defined but never called"),
					fp->name);
			}
		}
	}

	/* now let's free all the memory */
	for (i = 0; i < HASHSIZE; i++) {
		for (fp = ftable[i]; fp != NULL; fp = next) {
			next = fp->next;
			free(fp->name);
			free(fp);
		}
	}
}

/* param_sanity --- look for parameters that are regexp constants */

static void
param_sanity(NODE *arglist)
{
	NODE *argp, *arg;
	int i;

	for (i = 1, argp = arglist; argp != NULL; argp = argp->rnode, i++) {
		arg = argp->lnode;
		if (arg->type == Node_regex)
			warning(_("regexp constant for parameter #%d yields boolean value"), i);
	}
}

/* variable --- make sure NAME is in the symbol table */

NODE *
variable(char *name, int can_free, NODETYPE type)
{
	register NODE *r;

	if ((r = lookup(name)) != NULL) {
		if (r->type == Node_func)
			fatal(_("function `%s' called with space between name and `(',\n%s"),
				r->vname,
				_("or used as a variable or an array"));
	} else {
		/* not found */
		if (! do_traditional && STREQ(name, "PROCINFO"))
			r = load_procinfo();
		else if (STREQ(name, "ENVIRON"))
			r = load_environ();
		else {
			/*
			 * This is the only case in which we may not free the string.
			 */
			return install(name, node(Nnull_string, type, (NODE *) NULL));
		}
	}
	if (can_free)
		free(name);
	return r;
}

/* mk_rexp --- make a regular expression constant */

static NODE *
mk_rexp(NODE *exp)
{
	NODE *n;

	if (exp->type == Node_regex)
		return exp;

	getnode(n);
	n->type = Node_dynregex;
	n->re_exp = exp;
	n->re_text = NULL;
	n->re_reg = NULL;
	n->re_flags = 0;
	return n;
}

/* isnoeffect --- when used as a statement, has no side effects */

/*
 * To be completely general, we should recursively walk the parse
 * tree, to make sure that all the subexpressions also have no effect.
 * Instead, we just weaken the actual warning that's printed, up above
 * in the grammar.
 */

static int
isnoeffect(NODETYPE type)
{
	switch (type) {
	case Node_times:
	case Node_quotient:
	case Node_mod:
	case Node_plus:
	case Node_minus:
	case Node_subscript:
	case Node_concat:
	case Node_exp:
	case Node_unary_minus:
	case Node_field_spec:
	case Node_and:
	case Node_or:
	case Node_equal:
	case Node_notequal:
	case Node_less:
	case Node_greater:
	case Node_leq:
	case Node_geq:
	case Node_match:
	case Node_nomatch:
	case Node_not:
	case Node_val:
	case Node_in_array:
	case Node_NF:
	case Node_NR:
	case Node_FNR:
	case Node_FS:
	case Node_RS:
	case Node_FIELDWIDTHS:
	case Node_IGNORECASE:
	case Node_OFS:
	case Node_ORS:
	case Node_OFMT:
	case Node_CONVFMT:
	case Node_BINMODE:
	case Node_LINT:
		return TRUE;
	default:
		break;	/* keeps gcc -Wall happy */
	}

	return FALSE;
}

/* isassignable --- can this node be assigned to? */

static int
isassignable(register NODE *n)
{
	switch (n->type) {
	case Node_var:
	case Node_FIELDWIDTHS:
	case Node_RS:
	case Node_FS:
	case Node_FNR:
	case Node_NR:
	case Node_NF:
	case Node_IGNORECASE:
	case Node_OFMT:
	case Node_CONVFMT:
	case Node_ORS:
	case Node_OFS:
	case Node_LINT:
	case Node_BINMODE:
	case Node_field_spec:
	case Node_subscript:
		return TRUE;
	case Node_param_list:
		return ((n->flags & FUNC) == 0);  /* ok if not func name */
	default:
		break;	/* keeps gcc -Wall happy */
	}
	return FALSE;
}

/* stopme --- for debugging */

NODE *
stopme(NODE *tree ATTRIBUTE_UNUSED)
{
	return 0;
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
	pp_string_fp(stdout, str, len, '"', TRUE);
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
	pp_string_fp(stdout, str1, len1, '"', TRUE);
	putchar('\n');
	printf("msgid_plural ");
	pp_string_fp(stdout, str2, len2, '"', TRUE);
	putchar('\n');
	printf("msgstr[0] \"\"\nmsgstr[1] \"\"\n\n");
	fflush(stdout);
}

/* count_args --- count the number of printf arguments */

static void
count_args(NODE *tree)
{
	size_t count = 0;
	NODE *save_tree;

	assert(tree->type == Node_K_printf
		|| (tree->type == Node_builtin && tree->builtin == do_sprintf));
	save_tree = tree;

	tree = tree->lnode;	/* printf format string */

	for (count = 0; tree != NULL; tree = tree->rnode)
		count++;

	save_tree->printf_count = count;
}


