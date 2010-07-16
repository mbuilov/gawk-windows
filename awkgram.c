/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 26 "awkgram.y"

#ifdef GAWKDEBUG
#define YYDEBUG 12
#endif

#include "awk.h"

#if defined(__STDC__) && __STDC__ < 1	/* VMS weirdness, maybe elsewhere */
#define signed /**/
#endif

#define CAN_FREE	TRUE
#define DONT_FREE	FALSE

#ifdef CAN_USE_STDARG_H
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
static int isarray P((NODE *n));

enum defref { FUNC_DEFINE, FUNC_USE };
static void func_use P((const char *name, enum defref how));
static void check_funcs P((void));

static ssize_t read_one_line P((int fd, void *buffer, size_t count));
static int one_line_close P((int fd));

static NODE *constant_fold P((NODE *left, NODETYPE op, NODE *right));
static NODE *optimize_concat P((NODE *left, NODETYPE op, NODE *right));

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
extern long numfiles;
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


/* Line 189 of yacc.c  */
#line 167 "awkgram.c"

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

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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
     LEX_SWITCH = 277,
     LEX_CASE = 278,
     LEX_DEFAULT = 279,
     LEX_WHILE = 280,
     LEX_DO = 281,
     LEX_FOR = 282,
     LEX_BREAK = 283,
     LEX_CONTINUE = 284,
     LEX_PRINT = 285,
     LEX_PRINTF = 286,
     LEX_NEXT = 287,
     LEX_EXIT = 288,
     LEX_FUNCTION = 289,
     LEX_GETLINE = 290,
     LEX_NEXTFILE = 291,
     LEX_IN = 292,
     LEX_AND = 293,
     LEX_OR = 294,
     INCREMENT = 295,
     DECREMENT = 296,
     LEX_BUILTIN = 297,
     LEX_LENGTH = 298,
     NEWLINE = 299,
     SLASH_BEFORE_EQUAL = 300,
     UNARY = 301
   };
#endif
/* Tokens.  */
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
#define LEX_SWITCH 277
#define LEX_CASE 278
#define LEX_DEFAULT 279
#define LEX_WHILE 280
#define LEX_DO 281
#define LEX_FOR 282
#define LEX_BREAK 283
#define LEX_CONTINUE 284
#define LEX_PRINT 285
#define LEX_PRINTF 286
#define LEX_NEXT 287
#define LEX_EXIT 288
#define LEX_FUNCTION 289
#define LEX_GETLINE 290
#define LEX_NEXTFILE 291
#define LEX_IN 292
#define LEX_AND 293
#define LEX_OR 294
#define INCREMENT 295
#define DECREMENT 296
#define LEX_BUILTIN 297
#define LEX_LENGTH 298
#define NEWLINE 299
#define SLASH_BEFORE_EQUAL 300
#define UNARY 301




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 120 "awkgram.y"

	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval) P((void));



/* Line 214 of yacc.c  */
#line 306 "awkgram.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 318 "awkgram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1088

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  165
/* YYNRULES -- Number of states.  */
#define YYNSTATES  308

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   301

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,     2,     2,    59,    55,     2,     2,
      60,    61,    53,    51,    48,    52,     2,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    47,    66,
      49,     2,    50,    46,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,    63,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,     2,    65,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    57
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     8,    11,    14,    17,    20,    23,
      24,    26,    30,    32,    34,    40,    42,    44,    46,    48,
      50,    51,    59,    60,    64,    66,    68,    69,    72,    75,
      77,    80,    83,    87,    89,    99,   106,   115,   124,   137,
     149,   152,   155,   158,   161,   165,   166,   171,   174,   175,
     180,   186,   189,   194,   196,   197,   199,   201,   202,   205,
     208,   214,   219,   221,   224,   227,   229,   231,   233,   235,
     237,   243,   244,   245,   249,   256,   266,   268,   271,   272,
     274,   275,   278,   279,   281,   283,   287,   289,   292,   296,
     297,   299,   300,   302,   304,   308,   310,   313,   317,   321,
     325,   329,   333,   337,   341,   345,   351,   353,   355,   357,
     360,   362,   364,   366,   368,   370,   372,   375,   377,   381,
     385,   389,   393,   397,   401,   405,   408,   411,   417,   422,
     426,   430,   434,   438,   442,   446,   448,   451,   455,   460,
     465,   467,   472,   474,   477,   480,   482,   484,   487,   490,
     491,   493,   495,   500,   502,   506,   508,   510,   511,   514,
     517,   519,   520,   522,   524,   526
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      68,     0,    -1,    97,    69,    97,    -1,    -1,    69,    70,
      -1,    69,     1,    -1,    71,    72,    -1,    71,    81,    -1,
      75,    72,    -1,    -1,   104,    -1,   104,    48,   104,    -1,
      16,    -1,    17,    -1,   116,    80,   117,   119,    97,    -1,
       4,    -1,     3,    -1,    74,    -1,    42,    -1,    43,    -1,
      -1,    34,    76,    73,    60,    99,   118,    97,    -1,    -1,
      79,    78,     5,    -1,    54,    -1,    45,    -1,    -1,    80,
      82,    -1,    80,     1,    -1,    96,    -1,   120,    97,    -1,
     120,    97,    -1,   116,    80,   117,    -1,    95,    -1,    22,
      60,   104,   118,    97,   116,    87,    97,   117,    -1,    25,
      60,   104,   118,    97,    82,    -1,    26,    97,    82,    25,
      60,   104,   118,    97,    -1,    27,    60,     4,    37,     4,
     118,    97,    82,    -1,    27,    60,    86,   120,    97,   104,
     120,    97,    86,   118,    97,    82,    -1,    27,    60,    86,
     120,    97,   120,    97,    86,   118,    97,    82,    -1,    28,
      81,    -1,    29,    81,    -1,    32,    81,    -1,    36,    81,
      -1,    33,   101,    81,    -1,    -1,    20,    83,   101,    81,
      -1,    84,    81,    -1,    -1,    91,    85,    92,    93,    -1,
      21,     4,    62,   103,    63,    -1,    21,     4,    -1,    21,
      60,     4,    61,    -1,   104,    -1,    -1,    84,    -1,    88,
      -1,    -1,    88,    89,    -1,    88,     1,    -1,    23,    90,
     121,    97,    80,    -1,    24,   121,    97,    80,    -1,     7,
      -1,    52,     7,    -1,    51,     7,    -1,     8,    -1,    77,
      -1,    30,    -1,    31,    -1,   102,    -1,    60,   104,   122,
     103,   118,    -1,    -1,    -1,    10,    94,   108,    -1,    18,
      60,   104,   118,    97,    82,    -1,    18,    60,   104,   118,
      97,    82,    19,    97,    82,    -1,    44,    -1,    96,    44,
      -1,    -1,    96,    -1,    -1,    49,   109,    -1,    -1,   100,
      -1,     4,    -1,   100,   122,     4,    -1,     1,    -1,   100,
       1,    -1,   100,   122,     1,    -1,    -1,   104,    -1,    -1,
     103,    -1,   104,    -1,   103,   122,   104,    -1,     1,    -1,
     103,     1,    -1,   103,     1,   104,    -1,   103,   122,     1,
      -1,   113,   105,   104,    -1,   104,    38,   104,    -1,   104,
      39,   104,    -1,   104,    14,   104,    -1,   104,    37,     4,
      -1,   104,   107,   104,    -1,   104,    46,   104,    47,   104,
      -1,   108,    -1,    13,    -1,    12,    -1,    45,    13,    -1,
       9,    -1,    49,    -1,   106,    -1,    50,    -1,   109,    -1,
     110,    -1,   108,   109,    -1,   111,    -1,   109,    58,   109,
      -1,   109,    53,   109,    -1,   109,    54,   109,    -1,   109,
      55,   109,    -1,   109,    51,   109,    -1,   109,    52,   109,
      -1,    35,   112,    98,    -1,   113,    40,    -1,   113,    41,
      -1,    60,   103,   118,    37,     4,    -1,   108,    11,    35,
     112,    -1,   110,    58,   109,    -1,   110,    53,   109,    -1,
     110,    54,   109,    -1,   110,    55,   109,    -1,   110,    51,
     109,    -1,   110,    52,   109,    -1,    77,    -1,    56,   109,
      -1,    60,   104,   118,    -1,    42,    60,   102,   118,    -1,
      43,    60,   102,   118,    -1,    43,    -1,     3,    60,   102,
     118,    -1,   113,    -1,    40,   113,    -1,    41,   113,    -1,
       7,    -1,     8,    -1,    52,   109,    -1,    51,   109,    -1,
      -1,   113,    -1,     4,    -1,     4,    62,   103,    63,    -1,
     114,    -1,    59,   111,   115,    -1,    40,    -1,    41,    -1,
      -1,    64,    97,    -1,    65,    97,    -1,    61,    -1,    -1,
     120,    -1,    66,    -1,    47,    -1,    48,    97,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   181,   181,   187,   189,   194,   206,   210,   225,   236,
     239,   243,   253,   262,   274,   279,   281,   283,   294,   295,
     300,   299,   323,   322,   348,   349,   354,   355,   373,   378,
     379,   383,   385,   387,   389,   391,   393,   395,   439,   443,
     448,   451,   454,   463,   487,   490,   489,   499,   511,   511,
     542,   544,   562,   584,   590,   591,   596,   649,   650,   667,
     672,   674,   679,   681,   686,   688,   690,   695,   696,   704,
     705,   711,   716,   716,   728,   733,   740,   741,   744,   746,
     751,   752,   758,   759,   764,   766,   768,   770,   772,   779,
     780,   786,   787,   792,   794,   800,   802,   804,   806,   811,
     817,   819,   821,   827,   833,   839,   841,   846,   848,   850,
     855,   857,   861,   862,   867,   869,   871,   876,   878,   880,
     882,   884,   886,   888,   890,   896,   898,   900,   913,   918,
     920,   922,   924,   926,   928,   933,   935,   937,   939,   942,
     944,   958,   966,   967,   969,   971,   973,   976,   984,   996,
     997,  1002,  1004,  1019,  1029,  1044,  1045,  1046,  1050,  1054,
    1058,  1061,  1063,  1067,  1071,  1074
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FUNC_CALL", "NAME", "REGEXP", "ERROR",
  "YNUMBER", "YSTRING", "RELOP", "IO_OUT", "IO_IN", "ASSIGNOP", "ASSIGN",
  "MATCHOP", "CONCAT_OP", "LEX_BEGIN", "LEX_END", "LEX_IF", "LEX_ELSE",
  "LEX_RETURN", "LEX_DELETE", "LEX_SWITCH", "LEX_CASE", "LEX_DEFAULT",
  "LEX_WHILE", "LEX_DO", "LEX_FOR", "LEX_BREAK", "LEX_CONTINUE",
  "LEX_PRINT", "LEX_PRINTF", "LEX_NEXT", "LEX_EXIT", "LEX_FUNCTION",
  "LEX_GETLINE", "LEX_NEXTFILE", "LEX_IN", "LEX_AND", "LEX_OR",
  "INCREMENT", "DECREMENT", "LEX_BUILTIN", "LEX_LENGTH", "NEWLINE",
  "SLASH_BEFORE_EQUAL", "'?'", "':'", "','", "'<'", "'>'", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "UNARY", "'^'", "'$'", "'('", "')'", "'['",
  "']'", "'{'", "'}'", "';'", "$accept", "start", "program", "rule",
  "pattern", "action", "func_name", "lex_builtin", "function_prologue",
  "$@1", "regexp", "$@2", "a_slash", "statements", "statement_term",
  "statement", "$@3", "simple_stmt", "$@4", "opt_simple_stmt",
  "switch_body", "case_statements", "case_statement", "case_value",
  "print", "print_expression_list", "output_redir", "$@5", "if_statement",
  "nls", "opt_nls", "input_redir", "opt_param_list", "param_list",
  "opt_exp", "opt_expression_list", "expression_list", "exp",
  "assign_operator", "relop_or_less", "a_relop", "common_exp", "simp_exp",
  "simp_exp_nc", "non_post_simp_exp", "opt_variable", "variable",
  "field_spec", "opt_incdec", "l_brace", "r_brace", "r_paren", "opt_semi",
  "semi", "colon", "comma", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,    63,    58,    44,    60,
      62,    43,    45,    42,    47,    37,    33,   301,    94,    36,
      40,    41,    91,    93,   123,   125,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    69,    69,    69,    70,    70,    70,    71,
      71,    71,    71,    71,    72,    73,    73,    73,    74,    74,
      76,    75,    78,    77,    79,    79,    80,    80,    80,    81,
      81,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    83,    82,    82,    85,    84,
      84,    84,    84,    84,    86,    86,    87,    88,    88,    88,
      89,    89,    90,    90,    90,    90,    90,    91,    91,    92,
      92,    93,    94,    93,    95,    95,    96,    96,    97,    97,
      98,    98,    99,    99,   100,   100,   100,   100,   100,   101,
     101,   102,   102,   103,   103,   103,   103,   103,   103,   104,
     104,   104,   104,   104,   104,   104,   104,   105,   105,   105,
     106,   106,   107,   107,   108,   108,   108,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   112,
     112,   113,   113,   113,   114,   115,   115,   115,   116,   117,
     118,   119,   119,   120,   121,   122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     0,     2,     2,     2,     2,     2,     0,
       1,     3,     1,     1,     5,     1,     1,     1,     1,     1,
       0,     7,     0,     3,     1,     1,     0,     2,     2,     1,
       2,     2,     3,     1,     9,     6,     8,     8,    12,    11,
       2,     2,     2,     2,     3,     0,     4,     2,     0,     4,
       5,     2,     4,     1,     0,     1,     1,     0,     2,     2,
       5,     4,     1,     2,     2,     1,     1,     1,     1,     1,
       5,     0,     0,     3,     6,     9,     1,     2,     0,     1,
       0,     2,     0,     1,     1,     3,     1,     2,     3,     0,
       1,     0,     1,     1,     3,     1,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     1,     1,     1,     2,
       1,     1,     1,     1,     1,     1,     2,     1,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     5,     4,     3,
       3,     3,     3,     3,     3,     1,     2,     3,     4,     4,
       1,     4,     1,     2,     2,     1,     1,     2,     2,     0,
       1,     1,     4,     1,     3,     1,     1,     0,     2,     2,
       1,     0,     1,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      78,    76,     0,    79,     3,     1,    77,     0,     5,     0,
     151,   145,   146,    12,    13,    20,   149,     0,     0,     0,
     140,    25,     0,     0,    24,     0,     0,     0,     4,     0,
       0,   135,    22,     2,    10,   106,   114,   115,   117,   142,
     153,     0,     0,     0,    80,   150,   143,   144,     0,     0,
     148,   142,   147,   136,     0,   157,   142,    95,     0,    93,
      78,   163,     6,     7,    29,    26,    78,     8,     0,   110,
       0,     0,     0,     0,     0,     0,   111,   113,   112,     0,
       0,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   108,   107,   125,   126,     0,     0,
       0,     0,    93,     0,    16,    15,    18,    19,     0,    17,
       0,   124,     0,     0,     0,   155,   156,   154,    96,    78,
     160,     0,     0,   137,   158,     0,    30,    23,   102,   103,
     100,   101,     0,    11,   104,   149,   122,   123,   119,   120,
     121,   118,   133,   134,   130,   131,   132,   129,   109,    99,
     141,   152,     0,    81,   138,   139,    97,   165,     0,    98,
      94,    28,     0,    45,     0,     0,     0,    78,     0,     0,
       0,    67,    68,     0,    89,     0,    78,    27,     0,    48,
      33,    53,    26,   161,    78,     0,   128,    86,    84,     0,
       0,   127,     0,    89,    51,     0,     0,     0,     0,    54,
      40,    41,    42,     0,    90,    43,   159,    47,     0,     0,
      78,   162,    31,   105,    78,    87,     0,     0,     0,     0,
       0,     0,     0,     0,   151,    55,     0,    44,     0,    71,
      69,    32,    14,    21,    88,    85,    78,    46,     0,    52,
      78,    78,     0,     0,    78,    93,    72,    49,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    74,    57,    35,
       0,    78,     0,    78,     0,    73,    78,    78,     0,    78,
       0,    78,    54,    70,     0,     0,    59,     0,     0,    58,
      36,    37,    54,     0,    75,    34,    62,    65,     0,     0,
      66,     0,   164,    78,     0,    78,    64,    63,    78,    26,
      78,     0,    26,     0,     0,    39,     0,    38
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     7,    28,    29,    62,   108,   109,    30,    43,
      31,    68,    32,   125,    63,   177,   193,   178,   208,   226,
     267,   268,   279,   291,   179,   229,   247,   256,   180,     3,
       4,   111,   189,   190,   203,   100,   101,   181,    99,    78,
      79,    35,    36,    37,    38,    44,    39,    40,   117,   182,
     183,   123,   210,   184,   293,   122
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -221
static const yytype_int16 yypact[] =
{
     -32,  -221,    35,    -3,  -221,  -221,  -221,   343,  -221,   -13,
      -4,  -221,  -221,  -221,  -221,  -221,    32,    32,    32,    -6,
      -5,  -221,   974,   974,  -221,   974,  1028,   768,  -221,   -26,
       2,  -221,  -221,  -221,   820,   952,   392,   446,  -221,   125,
    -221,   707,   768,   185,    11,  -221,  -221,  -221,   707,   707,
       3,    -8,     3,     3,   974,    76,  -221,  -221,    42,   148,
     -32,  -221,  -221,  -221,    -3,  -221,   -32,  -221,    87,  -221,
     974,    93,   974,   974,   974,   974,  -221,  -221,  -221,   974,
      69,   392,   974,   974,   974,   974,   974,   974,   974,   974,
     974,   974,   974,   974,  -221,  -221,  -221,  -221,    97,   974,
      50,    41,   746,    36,  -221,  -221,  -221,  -221,    46,  -221,
     974,  -221,    50,    50,   148,  -221,  -221,  -221,   974,   -32,
    -221,    81,   790,  -221,  -221,   505,  -221,  -221,    14,  -221,
     540,   317,   874,   746,     7,    32,    70,    70,     3,     3,
       3,     3,    70,    70,     3,     3,     3,     3,  -221,   746,
    -221,  -221,    20,   392,  -221,  -221,   746,  -221,   115,  -221,
     746,  -221,    80,  -221,    27,    82,    84,   -32,    89,   -18,
     -18,  -221,  -221,   -18,   974,   -18,   -32,  -221,   -18,  -221,
    -221,   746,  -221,    67,   -32,   974,  -221,  -221,  -221,    50,
      48,  -221,   974,   974,    79,   150,   974,   974,   631,   844,
    -221,  -221,  -221,   -18,   746,  -221,  -221,  -221,   571,   505,
     -32,  -221,  -221,   746,   -32,  -221,    26,   148,   -18,   768,
      98,   148,   148,   133,   -12,  -221,    67,  -221,   768,   151,
    -221,  -221,  -221,  -221,  -221,  -221,   -32,  -221,    45,  -221,
     -32,   -32,   111,   172,   -32,   546,  -221,  -221,   631,  -221,
       2,   631,   974,    50,   685,   768,   974,   162,  -221,  -221,
     148,   -32,   325,   -32,    42,   952,   -32,   -32,    21,   -32,
     631,   -32,   898,  -221,   631,   117,  -221,   194,   137,  -221,
    -221,  -221,   898,    50,  -221,  -221,  -221,  -221,   183,   186,
    -221,   137,  -221,   -32,    50,   -32,  -221,  -221,   -32,  -221,
     -32,   631,  -221,   397,   631,  -221,   451,  -221
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -221,  -221,  -221,  -221,  -221,   169,  -221,  -221,  -221,  -221,
     -85,  -221,  -221,  -173,   -39,  -169,  -221,  -184,  -221,  -220,
    -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,     5,
      -7,  -221,  -221,  -221,    13,   -35,   -23,    73,  -221,  -221,
    -221,   -56,   128,  -221,   178,    90,   -15,  -221,  -221,   -24,
    -192,    55,  -221,   127,   -69,  -151
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -93
static const yytype_int16 yytable[] =
{
      33,    45,    46,    47,    58,    65,    65,    51,    51,   209,
      51,    56,     1,   112,   113,   225,   -93,   231,     1,   103,
      51,   187,   276,    69,   188,   243,     1,   234,   -93,   223,
     235,   194,    96,    97,    64,     5,    10,   118,    60,   216,
      61,     6,   118,   118,   277,   278,   118,    41,    61,   215,
      42,   -92,   283,   124,    48,    49,   -93,   -93,    42,   126,
     110,    87,   294,    76,    77,   -56,    60,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,   257,
      34,   -82,   259,   285,   119,   -92,   -56,   195,   225,   119,
     119,    26,   127,   119,   255,    51,   119,   129,   225,   151,
      59,   281,   -92,   120,   135,   284,   152,   -92,   249,   -83,
     148,   120,   157,   121,   102,   102,   115,   116,   158,   191,
      45,   102,   102,    84,    85,    86,   303,   114,    87,   306,
     200,   201,   305,    61,   202,   307,   205,    94,    95,   207,
     192,   219,   196,   128,   197,   130,   131,   132,   133,   199,
      50,    52,   134,    53,   220,   150,    66,    69,   242,   239,
     198,   246,    70,    81,   227,    96,    97,   154,   155,   206,
      98,   252,   149,   230,    64,    64,   253,   212,    64,   237,
      64,   266,   176,    64,   292,    71,    72,    73,   104,   105,
     296,   156,   290,   297,    74,   160,   238,    76,    77,    67,
     265,   286,   287,   232,    55,    58,   218,   233,    64,   120,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   298,    64,     0,   186,   258,   106,   107,   248,
       0,     0,   264,   250,   251,     0,     0,   254,   153,    21,
       0,    51,     0,     0,   214,   288,   289,   204,    24,     0,
      51,     0,     0,     0,   270,     0,   272,     0,   213,   274,
     275,     0,   280,     0,   282,   217,   204,     0,     0,   221,
     222,     0,   236,     0,     0,     0,   240,   241,     0,     0,
       0,   102,     0,     0,     0,     0,   299,     0,   301,     0,
       0,   302,   102,   304,     0,     0,    66,    66,     0,     0,
      66,   245,    66,     0,     0,    66,     0,     0,   261,     0,
     211,     0,     0,     0,     0,   269,     0,     0,     0,   273,
       0,     0,     0,     0,     0,   260,    69,   262,   102,     0,
      66,    70,     0,     0,    69,     0,     0,     0,   295,    70,
       0,     0,     0,   -78,     8,    66,     9,    10,     0,   300,
      11,    12,     0,   244,    71,    72,     0,     0,     0,    13,
      14,     0,    71,    72,    73,     0,    76,    77,     0,     0,
       0,    74,     0,     0,    76,    77,     0,    15,    16,     0,
       0,   263,     0,    17,    18,    19,    20,     1,    21,   271,
       0,    61,     0,    81,    22,    23,     0,    24,   161,    25,
       9,    10,    26,    27,    11,    12,     0,    -9,     0,    -9,
       0,     0,     0,     0,     0,   162,     0,   163,   164,   165,
     -61,   -61,   166,   167,   168,   169,   170,   171,   172,   173,
     174,     0,    16,   175,     0,     0,     0,    17,    18,    19,
      20,   -61,    21,    82,    83,    84,    85,    86,    22,    23,
      87,    24,   161,    25,     9,    10,    26,    27,    11,    12,
       0,    60,   -61,    61,     0,     0,     0,     0,     0,   162,
       0,   163,   164,   165,   -60,   -60,   166,   167,   168,   169,
     170,   171,   172,   173,   174,     0,    16,   175,     0,     0,
       0,    17,    18,    19,    20,   -60,    21,    88,    89,    90,
      91,    92,    22,    23,    93,    24,   161,    25,     9,    10,
      26,    27,    11,    12,     0,    60,   -60,    61,     0,     0,
       0,     0,     0,   162,     0,   163,   164,   165,     0,     0,
     166,   167,   168,   169,   170,   171,   172,   173,   174,     0,
      16,   175,     0,     0,     0,    17,    18,    19,    20,    69,
      21,     0,     0,     0,    70,    69,    22,    23,     0,    24,
      70,    25,     0,     0,    26,    27,     0,     0,     0,    60,
     176,    61,    57,     0,     9,    10,     0,    71,    11,    12,
       0,   -91,     0,    71,    72,    73,     0,     0,     0,    76,
      77,     0,    74,     0,   119,    76,    77,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,   120,     0,     0,
       0,    17,    18,    19,    20,   -91,    21,     0,     0,     0,
       0,     0,    22,    23,     0,    24,     0,    25,     0,     0,
      26,   228,   -91,     0,     9,    10,     0,   -91,    11,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   162,
       0,   163,   164,   165,     0,     0,   166,   167,   168,   169,
     170,   171,   172,   173,   174,     0,    16,   175,     0,     0,
       0,    17,    18,    19,    20,     0,    21,     0,     0,     0,
       0,     0,    22,    23,     0,    24,     0,    25,     9,    10,
      26,    27,    11,    12,     0,    60,     0,    61,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    57,     0,
       9,    10,     0,     0,    11,    12,     0,     0,     0,     0,
      16,     0,     0,     0,     0,    17,    18,    19,    20,     0,
      21,     0,     0,     0,     0,     0,    22,    23,     0,    24,
       0,    25,    16,     0,    26,    27,     0,    17,    18,    19,
      20,    61,    21,     0,     0,    69,     0,     0,    22,    23,
      70,    24,     0,    25,     0,     0,    26,    27,   -91,    57,
       0,     9,    10,     0,     0,    11,    12,     0,     0,     0,
       0,     0,     0,    71,    72,    73,     0,     0,     0,     0,
       0,   159,    74,     9,    10,    76,    77,    11,    12,     0,
       0,     0,     0,    16,     0,     0,     0,     0,    17,    18,
      19,    20,     0,    21,     0,     0,     0,     0,     0,    22,
      23,     0,    24,     0,    25,    16,     0,    26,    27,    69,
      17,    18,    19,    20,    70,    21,     0,     0,     0,     0,
       0,    22,    23,     0,    24,     0,    25,     9,   224,    26,
      27,    11,    12,     0,     0,     0,     0,    71,    72,    73,
       0,     0,     0,     0,     0,   164,    74,     0,    75,    76,
      77,     0,     0,     0,   171,   172,     0,     0,     0,    16,
       0,     0,     0,    69,    17,    18,    19,    20,    70,    21,
       0,     0,     0,     0,     0,    22,    23,     0,    24,     0,
      25,     9,    10,    26,    27,    11,    12,     0,     0,     0,
       0,    71,    72,    73,     0,     0,     0,     0,     0,   164,
      74,   185,     0,    76,    77,     0,     0,     0,   171,   172,
       0,     0,     0,    16,     0,     0,     0,     0,    17,    18,
      19,    20,     0,    21,     0,     0,     0,     0,     0,    22,
      23,     0,    24,     0,    25,     9,    10,    26,    27,    11,
      12,     0,     0,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,    10,     0,
       0,    11,    12,     0,     0,     0,     0,    16,     0,     0,
       0,     0,    17,    18,    19,    20,     0,    21,     0,     0,
       0,     0,     0,    22,    23,     0,    24,     0,    25,    16,
       0,    26,    27,     0,    17,    18,    19,    20,     0,    21,
       0,     0,     0,     0,     0,    22,    23,     0,    24,     0,
      25,     9,    10,    26,    27,    11,    12,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,    18,
      19,    20,     0,    21,     0,     0,     0,     0,     0,    22,
      23,     0,    24,     0,    25,     0,     0,    26,    54
};

static const yytype_int16 yycheck[] =
{
       7,    16,    17,    18,    27,    29,    30,    22,    23,   182,
      25,    26,    44,    48,    49,   199,     9,   209,    44,    42,
      35,     1,     1,     9,     4,    37,    44,     1,    14,   198,
       4,     4,    40,    41,    29,     0,     4,     1,    64,   190,
      66,    44,     1,     1,    23,    24,     1,    60,    66,     1,
      62,    10,   272,    60,    60,    60,    49,    50,    62,    66,
      49,    58,   282,    49,    50,    44,    64,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,   248,
       7,    61,   251,   275,    48,    44,    65,    60,   272,    48,
      48,    59,     5,    48,   245,   110,    48,     4,   282,    63,
      27,   270,    61,    61,    35,   274,    60,    66,    63,    61,
      13,    61,   119,    58,    41,    42,    40,    41,    37,     4,
     135,    48,    49,    53,    54,    55,   299,    54,    58,   302,
     169,   170,   301,    66,   173,   304,   175,    12,    13,   178,
      60,    62,    60,    70,    60,    72,    73,    74,    75,    60,
      22,    23,    79,    25,     4,   100,    29,     9,    25,    61,
     167,    10,    14,    35,   203,    40,    41,   112,   113,   176,
      45,    60,    99,   208,   169,   170,     4,   184,   173,   218,
     175,    19,    65,   178,    47,    37,    38,    39,     3,     4,
       7,   118,   277,     7,    46,   122,   219,    49,    50,    30,
     256,     7,     8,   210,    26,   228,   193,   214,   203,    61,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,   291,   218,    -1,   135,   250,    42,    43,   236,
      -1,    -1,   255,   240,   241,    -1,    -1,   244,   110,    45,
      -1,   256,    -1,    -1,   189,    51,    52,   174,    54,    -1,
     265,    -1,    -1,    -1,   261,    -1,   263,    -1,   185,   266,
     267,    -1,   269,    -1,   271,   192,   193,    -1,    -1,   196,
     197,    -1,   217,    -1,    -1,    -1,   221,   222,    -1,    -1,
      -1,   208,    -1,    -1,    -1,    -1,   293,    -1,   295,    -1,
      -1,   298,   219,   300,    -1,    -1,   169,   170,    -1,    -1,
     173,   228,   175,    -1,    -1,   178,    -1,    -1,   253,    -1,
     183,    -1,    -1,    -1,    -1,   260,    -1,    -1,    -1,   264,
      -1,    -1,    -1,    -1,    -1,   252,     9,   254,   255,    -1,
     203,    14,    -1,    -1,     9,    -1,    -1,    -1,   283,    14,
      -1,    -1,    -1,     0,     1,   218,     3,     4,    -1,   294,
       7,     8,    -1,   226,    37,    38,    -1,    -1,    -1,    16,
      17,    -1,    37,    38,    39,    -1,    49,    50,    -1,    -1,
      -1,    46,    -1,    -1,    49,    50,    -1,    34,    35,    -1,
      -1,   254,    -1,    40,    41,    42,    43,    44,    45,   262,
      -1,    66,    -1,   265,    51,    52,    -1,    54,     1,    56,
       3,     4,    59,    60,     7,     8,    -1,    64,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    18,    -1,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    35,    36,    -1,    -1,    -1,    40,    41,    42,
      43,    44,    45,    51,    52,    53,    54,    55,    51,    52,
      58,    54,     1,    56,     3,     4,    59,    60,     7,     8,
      -1,    64,    65,    66,    -1,    -1,    -1,    -1,    -1,    18,
      -1,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    51,    52,    53,
      54,    55,    51,    52,    58,    54,     1,    56,     3,     4,
      59,    60,     7,     8,    -1,    64,    65,    66,    -1,    -1,
      -1,    -1,    -1,    18,    -1,    20,    21,    22,    -1,    -1,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      35,    36,    -1,    -1,    -1,    40,    41,    42,    43,     9,
      45,    -1,    -1,    -1,    14,     9,    51,    52,    -1,    54,
      14,    56,    -1,    -1,    59,    60,    -1,    -1,    -1,    64,
      65,    66,     1,    -1,     3,     4,    -1,    37,     7,     8,
      -1,    10,    -1,    37,    38,    39,    -1,    -1,    -1,    49,
      50,    -1,    46,    -1,    48,    49,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    61,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    54,    -1,    56,    -1,    -1,
      59,    60,    61,    -1,     3,     4,    -1,    66,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,
      -1,    20,    21,    22,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    -1,    -1,
      -1,    40,    41,    42,    43,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    51,    52,    -1,    54,    -1,    56,     3,     4,
      59,    60,     7,     8,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
       3,     4,    -1,    -1,     7,     8,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,
      -1,    56,    35,    -1,    59,    60,    -1,    40,    41,    42,
      43,    66,    45,    -1,    -1,     9,    -1,    -1,    51,    52,
      14,    54,    -1,    56,    -1,    -1,    59,    60,    61,     1,
      -1,     3,     4,    -1,    -1,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,    -1,
      -1,     1,    46,     3,     4,    49,    50,     7,     8,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,    41,
      42,    43,    -1,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    54,    -1,    56,    35,    -1,    59,    60,     9,
      40,    41,    42,    43,    14,    45,    -1,    -1,    -1,    -1,
      -1,    51,    52,    -1,    54,    -1,    56,     3,     4,    59,
      60,     7,     8,    -1,    -1,    -1,    -1,    37,    38,    39,
      -1,    -1,    -1,    -1,    -1,    21,    46,    -1,    48,    49,
      50,    -1,    -1,    -1,    30,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,     9,    40,    41,    42,    43,    14,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,    -1,
      56,     3,     4,    59,    60,     7,     8,    -1,    -1,    -1,
      -1,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,    21,
      46,    47,    -1,    49,    50,    -1,    -1,    -1,    30,    31,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    40,    41,
      42,    43,    -1,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    54,    -1,    56,     3,     4,    59,    60,     7,
       8,    -1,    -1,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,    -1,
      -1,     7,     8,    -1,    -1,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    40,    41,    42,    43,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    54,    -1,    56,    35,
      -1,    59,    60,    -1,    40,    41,    42,    43,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    54,    -1,
      56,     3,     4,    59,    60,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    41,
      42,    43,    -1,    45,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    54,    -1,    56,    -1,    -1,    59,    60
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    44,    68,    96,    97,     0,    44,    69,     1,     3,
       4,     7,     8,    16,    17,    34,    35,    40,    41,    42,
      43,    45,    51,    52,    54,    56,    59,    60,    70,    71,
      75,    77,    79,    97,   104,   108,   109,   110,   111,   113,
     114,    60,    62,    76,   112,   113,   113,   113,    60,    60,
     109,   113,   109,   109,    60,   111,   113,     1,   103,   104,
      64,    66,    72,    81,    96,   116,   120,    72,    78,     9,
      14,    37,    38,    39,    46,    48,    49,    50,   106,   107,
      11,   109,    51,    52,    53,    54,    55,    58,    51,    52,
      53,    54,    55,    58,    12,    13,    40,    41,    45,   105,
     102,   103,   104,   103,     3,     4,    42,    43,    73,    74,
      49,    98,   102,   102,   104,    40,    41,   115,     1,    48,
      61,   118,   122,   118,    97,    80,    97,     5,   104,     4,
     104,   104,   104,   104,   104,    35,   109,   109,   109,   109,
     109,   109,   109,   109,   109,   109,   109,   109,    13,   104,
     118,    63,    60,   109,   118,   118,   104,    97,    37,     1,
     104,     1,    18,    20,    21,    22,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    36,    65,    82,    84,    91,
      95,   104,   116,   117,   120,    47,   112,     1,     4,    99,
     100,     4,    60,    83,     4,    60,    60,    60,    97,    60,
      81,    81,    81,   101,   104,    81,    97,    81,    85,    80,
     119,   120,    97,   104,   118,     1,   122,   104,   101,    62,
       4,   104,   104,    82,     4,    84,    86,    81,    60,    92,
     102,   117,    97,    97,     1,     4,   118,    81,   103,    61,
     118,   118,    25,    37,   120,   104,    10,    93,    97,    63,
      97,    97,    60,     4,    97,   122,    94,    82,   116,    82,
     104,   118,   104,   120,   103,   108,    19,    87,    88,   118,
      97,   120,    97,   118,    97,    97,     1,    23,    24,    89,
      97,    82,    97,    86,    82,   117,     7,     8,    51,    52,
      77,    90,    47,   121,    86,   118,     7,     7,   121,    97,
     118,    97,    97,    80,    97,    82,    80,    82
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
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
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
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



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

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
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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

/* Line 1464 of yacc.c  */
#line 182 "awkgram.y"
    {
			check_funcs();
		}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 190 "awkgram.y"
    {
		begin_or_end_rule = parsing_end_rule = FALSE;
		yyerrok;
	  }
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 195 "awkgram.y"
    {
		begin_or_end_rule = parsing_end_rule = FALSE;
		/*
		 * If errors, give up, don't produce an infinite
		 * stream of syntax error messages.
		 */
  		/* yyerrok; */
  	  }
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 207 "awkgram.y"
    {
		(yyvsp[(1) - (2)].nodeval)->rnode = (yyvsp[(2) - (2)].nodeval);
	  }
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 211 "awkgram.y"
    {
		if ((yyvsp[(1) - (2)].nodeval)->lnode != NULL) {
			/* pattern rule with non-empty pattern */
			(yyvsp[(1) - (2)].nodeval)->rnode = node(NULL, Node_K_print_rec, NULL);
		} else {
			/* an error */
			if (begin_or_end_rule)
				msg(_("%s blocks must have an action part"),
					(parsing_end_rule ? "END" : "BEGIN"));
			else
				msg(_("each rule must have a pattern or an action part"));
			errcount++;
		}
	  }
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 226 "awkgram.y"
    {
		can_return = FALSE;
		if ((yyvsp[(1) - (2)].nodeval))
			func_install((yyvsp[(1) - (2)].nodeval), (yyvsp[(2) - (2)].nodeval));
		yyerrok;
	  }
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 236 "awkgram.y"
    {
		(yyval.nodeval) = append_pattern(&expression_value, (NODE *) NULL);
	  }
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 240 "awkgram.y"
    {
		(yyval.nodeval) = append_pattern(&expression_value, (yyvsp[(1) - (1)].nodeval));
	  }
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 244 "awkgram.y"
    {
		NODE *r;

		getnode(r);
		r->type = Node_line_range;
		r->condpair = node((yyvsp[(1) - (3)].nodeval), Node_cond_pair, (yyvsp[(3) - (3)].nodeval));
		r->triggered = FALSE;
		(yyval.nodeval) = append_pattern(&expression_value, r);
	  }
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 254 "awkgram.y"
    {
		static int begin_seen = 0;
		if (do_lint_old && ++begin_seen == 2)
			warning(_("old awk does not support multiple `BEGIN' or `END' rules"));

		begin_or_end_rule = TRUE;
		(yyval.nodeval) = append_pattern(&begin_block, (NODE *) NULL);
	  }
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 263 "awkgram.y"
    {
		static int end_seen = 0;
		if (do_lint_old && ++end_seen == 2)
			warning(_("old awk does not support multiple `BEGIN' or `END' rules"));

		begin_or_end_rule = parsing_end_rule = TRUE;
		(yyval.nodeval) = append_pattern(&end_block, (NODE *) NULL);
	  }
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 275 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(2) - (5)].nodeval); }
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 280 "awkgram.y"
    { (yyval.sval) = (yyvsp[(1) - (1)].sval); }
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 282 "awkgram.y"
    { (yyval.sval) = (yyvsp[(1) - (1)].sval); }
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 284 "awkgram.y"
    {
		yyerror(_("`%s' is a built-in function, it cannot be redefined"),
			tokstart);
		errcount++;
		(yyval.sval) = builtin_func;
		/* yyerrok; */
	  }
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 300 "awkgram.y"
    {
			param_counter = 0;
		}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 304 "awkgram.y"
    {
			NODE *t;

			t = make_param((yyvsp[(3) - (7)].sval));
			t->flags |= FUNC;
			(yyval.nodeval) = append_right(t, (yyvsp[(5) - (7)].nodeval));
			can_return = TRUE;
			/* check for duplicate parameter names */
			if (dup_parms((yyval.nodeval)))
				errcount++;
		}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 323 "awkgram.y"
    { ++want_regexp; }
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 325 "awkgram.y"
    {
		  NODE *n;
		  size_t len = strlen((yyvsp[(3) - (3)].sval));

		  if (do_lint) {
			if (len == 0)
				lintwarn(_("regexp constant `//' looks like a C++ comment, but is not"));
			else if (((yyvsp[(3) - (3)].sval))[0] == '*' && ((yyvsp[(3) - (3)].sval))[len-1] == '*')
				/* possible C comment */
				lintwarn(_("regexp constant `/%s/' looks like a C comment, but is not"), tokstart);
		  }
		  getnode(n);
		  n->type = Node_regex;
		  n->re_exp = make_string((yyvsp[(3) - (3)].sval), len);
		  n->re_reg = make_regexp((yyvsp[(3) - (3)].sval), len, FALSE, TRUE);
		  n->re_text = NULL;
		  n->re_flags = CONSTANT;
		  n->re_cnt = 1;
		  (yyval.nodeval) = n;
		}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 354 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 356 "awkgram.y"
    {
		if ((yyvsp[(2) - (2)].nodeval) == NULL)
			(yyval.nodeval) = (yyvsp[(1) - (2)].nodeval);
		else {
			if (do_lint && isnoeffect((yyvsp[(2) - (2)].nodeval)->type))
				lintwarn(_("statement may have no effect"));
			if ((yyvsp[(1) - (2)].nodeval) == NULL)
				(yyval.nodeval) = (yyvsp[(2) - (2)].nodeval);
			else
	    			(yyval.nodeval) = append_right(
					((yyvsp[(1) - (2)].nodeval)->type == Node_statement_list ? (yyvsp[(1) - (2)].nodeval)
					  : node((yyvsp[(1) - (2)].nodeval), Node_statement_list, (NODE *) NULL)),
					((yyvsp[(2) - (2)].nodeval)->type == Node_statement_list ? (yyvsp[(2) - (2)].nodeval)
					  : node((yyvsp[(2) - (2)].nodeval), Node_statement_list, (NODE *) NULL)));
		}
	    	yyerrok;
	  }
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 374 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 384 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 386 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(2) - (3)].nodeval); }
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 388 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 390 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(3) - (9)].nodeval), Node_K_switch, (yyvsp[(7) - (9)].nodeval)); }
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 392 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(3) - (6)].nodeval), Node_K_while, (yyvsp[(6) - (6)].nodeval)); }
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 394 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(6) - (8)].nodeval), Node_K_do, (yyvsp[(3) - (8)].nodeval)); }
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 396 "awkgram.y"
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
		if ((yyvsp[(8) - (8)].nodeval) != NULL && (yyvsp[(8) - (8)].nodeval)->type == Node_K_delete && (yyvsp[(8) - (8)].nodeval)->rnode != NULL) {
			NODE *arr, *sub;

			assert((yyvsp[(8) - (8)].nodeval)->rnode->type == Node_expression_list);
			arr = (yyvsp[(8) - (8)].nodeval)->lnode;	/* array var */
			sub = (yyvsp[(8) - (8)].nodeval)->rnode->lnode;	/* index var */

			if (   (arr->type == Node_var_new
				|| arr->type == Node_var_array
				|| arr->type == Node_param_list)
			    && (sub->type == Node_var_new
				|| sub->type == Node_var
				|| sub->type == Node_param_list)
			    && strcmp((yyvsp[(3) - (8)].sval), sub->vname) == 0
			    && strcmp((yyvsp[(5) - (8)].sval), arr->vname) == 0) {
				(yyvsp[(8) - (8)].nodeval)->type = Node_K_delete_loop;
				(yyval.nodeval) = (yyvsp[(8) - (8)].nodeval);
				free((yyvsp[(3) - (8)].sval));	/* thanks to valgrind for pointing these out */
				free((yyvsp[(5) - (8)].sval));
			}
			else
				goto regular_loop;
		} else {
	regular_loop:
			(yyval.nodeval) = node((yyvsp[(8) - (8)].nodeval), Node_K_arrayfor,
				make_for_loop(variable((yyvsp[(3) - (8)].sval), CAN_FREE, Node_var),
				(NODE *) NULL, variable((yyvsp[(5) - (8)].sval), CAN_FREE, Node_var_array)));
		}
	  }
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 440 "awkgram.y"
    {
		(yyval.nodeval) = node((yyvsp[(12) - (12)].nodeval), Node_K_for, (NODE *) make_for_loop((yyvsp[(3) - (12)].nodeval), (yyvsp[(6) - (12)].nodeval), (yyvsp[(9) - (12)].nodeval)));
	  }
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 444 "awkgram.y"
    {
		(yyval.nodeval) = node((yyvsp[(11) - (11)].nodeval), Node_K_for,
			(NODE *) make_for_loop((yyvsp[(3) - (11)].nodeval), (NODE *) NULL, (yyvsp[(8) - (11)].nodeval)));
	  }
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 450 "awkgram.y"
    { (yyval.nodeval) = node((NODE *) NULL, Node_K_break, (NODE *) NULL); }
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 453 "awkgram.y"
    { (yyval.nodeval) = node((NODE *) NULL, Node_K_continue, (NODE *) NULL); }
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 455 "awkgram.y"
    { NODETYPE type;

		  if (begin_or_end_rule)
			yyerror(_("`%s' used in %s action"), "next",
				(parsing_end_rule ? "END" : "BEGIN"));
		  type = Node_K_next;
		  (yyval.nodeval) = node((NODE *) NULL, type, (NODE *) NULL);
		}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 464 "awkgram.y"
    {
		  static short warned = FALSE;

		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`nextfile' is a gawk extension"));
		  }
		  if (do_lint && ! warned) {
		  	warned = TRUE;
			lintwarn(_("`nextfile' is a gawk extension"));
		  }
		  if (begin_or_end_rule) {
			/* same thing */
			errcount++;
			error(_("`%s' used in %s action"), "nextfile",
				(parsing_end_rule ? "END" : "BEGIN"));
		  }
		  (yyval.nodeval) = node((NODE *) NULL, Node_K_nextfile, (NODE *) NULL);
		}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 488 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (3)].nodeval), Node_K_exit, (NODE *) NULL); }
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 490 "awkgram.y"
    {
		  if (! can_return)
			yyerror(_("`return' used outside function context"));
		}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 495 "awkgram.y"
    {
		  (yyval.nodeval) = node((yyvsp[(3) - (4)].nodeval) == NULL ? Nnull_string : (yyvsp[(3) - (4)].nodeval),
			Node_K_return, (NODE *) NULL);
		}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 511 "awkgram.y"
    { in_print = TRUE; in_parens = 0; }
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 512 "awkgram.y"
    {
		/*
		 * Optimization: plain `print' has no expression list, so $3 is null.
		 * If $3 is an expression list with one element (rnode == null)
		 * and lnode is a field spec for field 0, we have `print $0'.
		 * For both, use Node_K_print_rec, which is faster for these two cases.
		 */
		if ((yyvsp[(1) - (4)].nodetypeval) == Node_K_print &&
		    ((yyvsp[(3) - (4)].nodeval) == NULL
		     || ((yyvsp[(3) - (4)].nodeval)->type == Node_expression_list
			&& (yyvsp[(3) - (4)].nodeval)->rnode == NULL
			&& (yyvsp[(3) - (4)].nodeval)->lnode->type == Node_field_spec
			&& (yyvsp[(3) - (4)].nodeval)->lnode->lnode->type == Node_val
			&& (yyvsp[(3) - (4)].nodeval)->lnode->lnode->numbr == 0.0))
		) {
			static short warned = FALSE;

			(yyval.nodeval) = node(NULL, Node_K_print_rec, (yyvsp[(4) - (4)].nodeval));

			if (do_lint && (yyvsp[(3) - (4)].nodeval) == NULL && begin_or_end_rule && ! warned) {
				warned = TRUE;
				lintwarn(
	_("plain `print' in BEGIN or END rule should probably be `print \"\"'"));
			}
		} else {
			(yyval.nodeval) = node((yyvsp[(3) - (4)].nodeval), (yyvsp[(1) - (4)].nodetypeval), (yyvsp[(4) - (4)].nodeval));
			if ((yyval.nodeval)->type == Node_K_printf)
				count_args((yyval.nodeval));
		}
	  }
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 543 "awkgram.y"
    { (yyval.nodeval) = node(variable((yyvsp[(2) - (5)].sval), CAN_FREE, Node_var_array), Node_K_delete, (yyvsp[(4) - (5)].nodeval)); }
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 545 "awkgram.y"
    {
		  static short warned = FALSE;

		  if (do_lint && ! warned) {
			warned = TRUE;
			lintwarn(_("`delete array' is a gawk extension"));
		  }
		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`delete array' is a gawk extension"));
		  }
		  (yyval.nodeval) = node(variable((yyvsp[(2) - (2)].sval), CAN_FREE, Node_var_array), Node_K_delete, (NODE *) NULL);
		}
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 563 "awkgram.y"
    {
		  /*
		   * this is for tawk compatibility. maybe the warnings
		   * should always be done.
		   */
		  static short warned = FALSE;

		  if (do_lint && ! warned) {
			warned = TRUE;
			lintwarn(_("`delete(array)' is a non-portable tawk extension"));
		  }
		  if (do_traditional) {
			/*
			 * can't use yyerror, since may have overshot
			 * the source line
			 */
			errcount++;
			error(_("`delete(array)' is a non-portable tawk extension"));
		  }
		  (yyval.nodeval) = node(variable((yyvsp[(3) - (4)].sval), CAN_FREE, Node_var_array), Node_K_delete, (NODE *) NULL);
		}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 585 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 590 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 592 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 597 "awkgram.y"
    {
		if ((yyvsp[(1) - (1)].nodeval) == NULL) {
			(yyval.nodeval) = NULL;
		} else {
			NODE *dflt = NULL;
			NODE *head = (yyvsp[(1) - (1)].nodeval);
			NODE *curr;
	
			const char **case_values = NULL;
	
			int maxcount = 128;
			int case_count = 0;
			int i;
	
			emalloc(case_values, const char **, sizeof(char*) * maxcount, "switch_body");
			for (curr = (yyvsp[(1) - (1)].nodeval); curr != NULL; curr = curr->rnode) {
				/* Assure that case statement values are unique. */
				if (curr->lnode->type == Node_K_case) {
					char *caseval;
	
					if (curr->lnode->lnode->type == Node_regex)
						caseval = curr->lnode->lnode->re_exp->stptr;
					else
						caseval = force_string(tree_eval(curr->lnode->lnode))->stptr;
	
					for (i = 0; i < case_count; i++)
						if (strcmp(caseval, case_values[i]) == 0)
							yyerror(_("duplicate case values in switch body: %s"), caseval);
	
					if (case_count >= maxcount) {
						maxcount += 128;
						erealloc(case_values, const char **, sizeof(char*) * maxcount, "switch_body");
					}
					case_values[case_count++] = caseval;
				} else {
					/* Otherwise save a pointer to the default node.  */
					if (dflt != NULL)
						yyerror(_("Duplicate `default' detected in switch body"));
					dflt = curr;
				}
			}
	
			free(case_values);
	
			/* Create the switch body. */
			(yyval.nodeval) = node(head, Node_switch_body, dflt);
		}
	}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 649 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 651 "awkgram.y"
    {
		if ((yyvsp[(2) - (2)].nodeval) == NULL)
			(yyval.nodeval) = (yyvsp[(1) - (2)].nodeval);
		else {
			if (do_lint && isnoeffect((yyvsp[(2) - (2)].nodeval)->type))
				lintwarn(_("statement may have no effect"));
			if ((yyvsp[(1) - (2)].nodeval) == NULL)
				(yyval.nodeval) = node((yyvsp[(2) - (2)].nodeval), Node_case_list, (NODE *) NULL);
			else
				(yyval.nodeval) = append_right(
					((yyvsp[(1) - (2)].nodeval)->type == Node_case_list ? (yyvsp[(1) - (2)].nodeval) : node((yyvsp[(1) - (2)].nodeval), Node_case_list, (NODE *) NULL)),
					((yyvsp[(2) - (2)].nodeval)->type == Node_case_list ? (yyvsp[(2) - (2)].nodeval) : node((yyvsp[(2) - (2)].nodeval), Node_case_list, (NODE *) NULL))
				);
		}
	    	yyerrok;
	  }
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 668 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 673 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (5)].nodeval), Node_K_case, (yyvsp[(5) - (5)].nodeval)); }
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 675 "awkgram.y"
    { (yyval.nodeval) = node((NODE *) NULL, Node_K_default, (yyvsp[(4) - (4)].nodeval)); }
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 680 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 682 "awkgram.y"
    {
		(yyvsp[(2) - (2)].nodeval)->numbr = -(force_number((yyvsp[(2) - (2)].nodeval)));
		(yyval.nodeval) = (yyvsp[(2) - (2)].nodeval);
	  }
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 687 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(2) - (2)].nodeval); }
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 689 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 691 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 706 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (5)].nodeval), Node_expression_list, (yyvsp[(4) - (5)].nodeval)); }
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 711 "awkgram.y"
    {
		in_print = FALSE;
		in_parens = 0;
		(yyval.nodeval) = NULL;
	  }
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 716 "awkgram.y"
    { in_print = FALSE; in_parens = 0; }
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 717 "awkgram.y"
    {
		(yyval.nodeval) = node((yyvsp[(3) - (3)].nodeval), (yyvsp[(1) - (3)].nodetypeval), (NODE *) NULL);
		if ((yyvsp[(1) - (3)].nodetypeval) == Node_redirect_twoway
		    && (yyvsp[(3) - (3)].nodeval)->type == Node_K_getline
		    && (yyvsp[(3) - (3)].nodeval)->rnode != NULL
		    && (yyvsp[(3) - (3)].nodeval)->rnode->type == Node_redirect_twoway)
			yyerror(_("multistage two-way pipelines don't work"));
	  }
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 729 "awkgram.y"
    {
		(yyval.nodeval) = node((yyvsp[(3) - (6)].nodeval), Node_K_if, 
			node((yyvsp[(6) - (6)].nodeval), Node_if_branches, (NODE *) NULL));
	  }
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 735 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(3) - (9)].nodeval), Node_K_if,
				node((yyvsp[(6) - (9)].nodeval), Node_if_branches, (yyvsp[(9) - (9)].nodeval))); }
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 751 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 753 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (2)].nodeval), Node_redirect_input, (NODE *) NULL); }
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 758 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 760 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 765 "awkgram.y"
    { (yyval.nodeval) = make_param((yyvsp[(1) - (1)].sval)); }
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 767 "awkgram.y"
    { (yyval.nodeval) = append_right((yyvsp[(1) - (3)].nodeval), make_param((yyvsp[(3) - (3)].sval))); yyerrok; }
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 769 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 771 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 773 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 779 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 781 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 786 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 788 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 793 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (1)].nodeval), Node_expression_list, (NODE *) NULL); }
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 795 "awkgram.y"
    {
			(yyval.nodeval) = append_right((yyvsp[(1) - (3)].nodeval),
				node((yyvsp[(3) - (3)].nodeval), Node_expression_list, (NODE *) NULL));
			yyerrok;
		}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 801 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 803 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 805 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 807 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 812 "awkgram.y"
    {
		  if (do_lint && (yyvsp[(3) - (3)].nodeval)->type == Node_regex)
			lintwarn(_("regular expression on right of assignment"));
		  (yyval.nodeval) = optimize_concat((yyvsp[(1) - (3)].nodeval), (yyvsp[(2) - (3)].nodetypeval), (yyvsp[(3) - (3)].nodeval));
		}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 818 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_and, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 820 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_or, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 822 "awkgram.y"
    {
		  if ((yyvsp[(1) - (3)].nodeval)->type == Node_regex)
			warning(_("regular expression on left of `~' or `!~' operator"));
		  (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), (yyvsp[(2) - (3)].nodetypeval), mk_rexp((yyvsp[(3) - (3)].nodeval)));
		}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 828 "awkgram.y"
    {
		  if (do_lint_old)
		    warning(_("old awk does not support the keyword `in' except after `for'"));
		  (yyval.nodeval) = node(variable((yyvsp[(3) - (3)].sval), CAN_FREE, Node_var_array), Node_in_array, (yyvsp[(1) - (3)].nodeval));
		}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 834 "awkgram.y"
    {
		  if (do_lint && (yyvsp[(3) - (3)].nodeval)->type == Node_regex)
			lintwarn(_("regular expression on right of comparison"));
		  (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), (yyvsp[(2) - (3)].nodetypeval), (yyvsp[(3) - (3)].nodeval));
		}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 840 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (5)].nodeval), Node_cond_exp, node((yyvsp[(3) - (5)].nodeval), Node_if_branches, (yyvsp[(5) - (5)].nodeval)));}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 842 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 847 "awkgram.y"
    { (yyval.nodetypeval) = (yyvsp[(1) - (1)].nodetypeval); }
    break;

  case 108:

/* Line 1464 of yacc.c  */
#line 849 "awkgram.y"
    { (yyval.nodetypeval) = (yyvsp[(1) - (1)].nodetypeval); }
    break;

  case 109:

/* Line 1464 of yacc.c  */
#line 851 "awkgram.y"
    { (yyval.nodetypeval) = Node_assign_quotient; }
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 856 "awkgram.y"
    { (yyval.nodetypeval) = (yyvsp[(1) - (1)].nodetypeval); }
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 858 "awkgram.y"
    { (yyval.nodetypeval) = Node_less; }
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 863 "awkgram.y"
    { (yyval.nodetypeval) = Node_greater; }
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 868 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 870 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 116:

/* Line 1464 of yacc.c  */
#line 872 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (2)].nodeval), Node_concat, (yyvsp[(2) - (2)].nodeval)); }
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 879 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_exp, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 881 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_times, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 883 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_quotient, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 885 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_mod, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 887 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_plus, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 889 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(1) - (3)].nodeval), Node_minus, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 891 "awkgram.y"
    {
		  if (do_lint && parsing_end_rule && (yyvsp[(3) - (3)].nodeval) == NULL)
			lintwarn(_("non-redirected `getline' undefined inside END action"));
		  (yyval.nodeval) = node((yyvsp[(2) - (3)].nodeval), Node_K_getline, (yyvsp[(3) - (3)].nodeval));
		}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 897 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (2)].nodeval), Node_postincrement, (NODE *) NULL); }
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 899 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (2)].nodeval), Node_postdecrement, (NODE *) NULL); }
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 901 "awkgram.y"
    {
		  if (do_lint_old) {
		    warning(_("old awk does not support the keyword `in' except after `for'"));
		    warning(_("old awk does not support multidimensional arrays"));
		  }
		  (yyval.nodeval) = node(variable((yyvsp[(5) - (5)].sval), CAN_FREE, Node_var_array), Node_in_array, (yyvsp[(2) - (5)].nodeval));
		}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 914 "awkgram.y"
    {
		  (yyval.nodeval) = node((yyvsp[(4) - (4)].nodeval), Node_K_getline,
			 node((yyvsp[(1) - (4)].nodeval), (yyvsp[(2) - (4)].nodetypeval), (NODE *) NULL));
		}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 919 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_exp, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 921 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_times, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 923 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_quotient, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 925 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_mod, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 927 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_plus, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 929 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(1) - (3)].nodeval), Node_minus, (yyvsp[(3) - (3)].nodeval)); }
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 934 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 936 "awkgram.y"
    { (yyval.nodeval) = constant_fold((yyvsp[(2) - (2)].nodeval), Node_not, (NODE *) NULL); }
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 938 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(2) - (3)].nodeval); }
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 941 "awkgram.y"
    { (yyval.nodeval) = snode((yyvsp[(3) - (4)].nodeval), Node_builtin, (int) (yyvsp[(1) - (4)].lval)); }
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 943 "awkgram.y"
    { (yyval.nodeval) = snode((yyvsp[(3) - (4)].nodeval), Node_builtin, (int) (yyvsp[(1) - (4)].lval)); }
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 945 "awkgram.y"
    {
		static short warned1 = FALSE, warned2 = FALSE;

		if (do_lint && ! warned1) {
			warned1 = TRUE;
			lintwarn(_("call of `length' without parentheses is not portable"));
		}
		(yyval.nodeval) = snode((NODE *) NULL, Node_builtin, (int) (yyvsp[(1) - (1)].lval));
		if (do_posix && ! warned2) {
			warned2 = TRUE;
			warning(_("call of `length' without parentheses is deprecated by POSIX"));
		}
	  }
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 959 "awkgram.y"
    {
		(yyval.nodeval) = node((yyvsp[(3) - (4)].nodeval), Node_func_call, make_string((yyvsp[(1) - (4)].sval), strlen((yyvsp[(1) - (4)].sval))));
		(yyval.nodeval)->funcbody = NULL;
		func_use((yyvsp[(1) - (4)].sval), FUNC_USE);
		param_sanity((yyvsp[(3) - (4)].nodeval));
		free((yyvsp[(1) - (4)].sval));
	  }
    break;

  case 143:

/* Line 1464 of yacc.c  */
#line 968 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (2)].nodeval), Node_preincrement, (NODE *) NULL); }
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 970 "awkgram.y"
    { (yyval.nodeval) = node((yyvsp[(2) - (2)].nodeval), Node_predecrement, (NODE *) NULL); }
    break;

  case 145:

/* Line 1464 of yacc.c  */
#line 972 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 974 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 977 "awkgram.y"
    {
		  if ((yyvsp[(2) - (2)].nodeval)->type == Node_val && ((yyvsp[(2) - (2)].nodeval)->flags & (STRCUR|STRING)) == 0) {
			(yyvsp[(2) - (2)].nodeval)->numbr = -(force_number((yyvsp[(2) - (2)].nodeval)));
			(yyval.nodeval) = (yyvsp[(2) - (2)].nodeval);
		  } else
			(yyval.nodeval) = node((yyvsp[(2) - (2)].nodeval), Node_unary_minus, (NODE *) NULL);
		}
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 985 "awkgram.y"
    {
		  /*
		   * was: $$ = $2
		   * POSIX semantics: force a conversion to numeric type
		   */
		  (yyval.nodeval) = node (make_number(0.0), Node_plus, (yyvsp[(2) - (2)].nodeval));
		}
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 996 "awkgram.y"
    { (yyval.nodeval) = NULL; }
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 998 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 1003 "awkgram.y"
    { (yyval.nodeval) = variable((yyvsp[(1) - (1)].sval), CAN_FREE, Node_var_new); }
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 1005 "awkgram.y"
    {
		NODE *n;

		if ((n = lookup((yyvsp[(1) - (4)].sval))) != NULL && ! isarray(n)) {
			yyerror(_("use of non-array as array"));
			(yyval.nodeval) = node(variable((yyvsp[(1) - (4)].sval), CAN_FREE, Node_var_array), Node_subscript, (yyvsp[(3) - (4)].nodeval));
		} else if ((yyvsp[(3) - (4)].nodeval) == NULL) {
			fatal(_("invalid subscript expression"));
		} else if ((yyvsp[(3) - (4)].nodeval)->rnode == NULL) {
			(yyval.nodeval) = node(variable((yyvsp[(1) - (4)].sval), CAN_FREE, Node_var_array), Node_subscript, (yyvsp[(3) - (4)].nodeval)->lnode);
			freenode((yyvsp[(3) - (4)].nodeval));
		} else
			(yyval.nodeval) = node(variable((yyvsp[(1) - (4)].sval), CAN_FREE, Node_var_array), Node_subscript, (yyvsp[(3) - (4)].nodeval));
	  }
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 1019 "awkgram.y"
    { (yyval.nodeval) = (yyvsp[(1) - (1)].nodeval); }
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 1030 "awkgram.y"
    {
		NODE *n = node((yyvsp[(2) - (3)].nodeval), Node_field_spec, (NODE *) NULL);
		if ((yyvsp[(3) - (3)].sval) != NULL) {
			if ((yyvsp[(3) - (3)].sval)[0] == '+')
				(yyval.nodeval) = node(n, Node_postincrement, (NODE *) NULL);
			else
				(yyval.nodeval) = node(n, Node_postdecrement, (NODE *) NULL);
		} else {
			(yyval.nodeval) = n;
		}
	  }
    break;

  case 155:

/* Line 1464 of yacc.c  */
#line 1044 "awkgram.y"
    { (yyval.sval) = "+"; }
    break;

  case 156:

/* Line 1464 of yacc.c  */
#line 1045 "awkgram.y"
    { (yyval.sval) = "-"; }
    break;

  case 157:

/* Line 1464 of yacc.c  */
#line 1046 "awkgram.y"
    { (yyval.sval) = NULL; }
    break;

  case 159:

/* Line 1464 of yacc.c  */
#line 1054 "awkgram.y"
    { yyerrok; }
    break;

  case 160:

/* Line 1464 of yacc.c  */
#line 1058 "awkgram.y"
    { yyerrok; }
    break;

  case 163:

/* Line 1464 of yacc.c  */
#line 1067 "awkgram.y"
    { yyerrok; }
    break;

  case 164:

/* Line 1464 of yacc.c  */
#line 1071 "awkgram.y"
    { yyerrok; }
    break;

  case 165:

/* Line 1464 of yacc.c  */
#line 1074 "awkgram.y"
    { yyerrok; }
    break;



/* Line 1464 of yacc.c  */
#line 3392 "awkgram.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
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
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
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


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 1077 "awkgram.y"


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

#if 'a' == 0x81 /* it's EBCDIC */
/* tokcompare --- lexicographically compare token names for sorting */

static int
tokcompare(void *l, void *r)
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
#ifdef ALLOW_SWITCH
{"case",	Node_K_case,	 LEX_CASE,	GAWKX,		0},
#endif
{"close",	Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1)|A(2),	do_close},
{"compl",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_compl},
{"continue",	Node_K_continue, LEX_CONTINUE,	0,		0},
{"cos",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(1),	do_cos},
{"dcgettext",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3),	do_dcgettext},
{"dcngettext",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(1)|A(2)|A(3)|A(4)|A(5),	do_dcngettext},
#ifdef ALLOW_SWITCH
{"default",	Node_K_default,	 LEX_DEFAULT,	GAWKX,		0},
#endif
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
{"strftime",	Node_builtin,	 LEX_BUILTIN,	GAWKX|A(0)|A(1)|A(2)|A(3), do_strftime},
{"strtonum",	Node_builtin,    LEX_BUILTIN,	GAWKX|A(1),	do_strtonum},
{"sub",		Node_builtin,	 LEX_BUILTIN,	NOT_OLD|A(2)|A(3), do_sub},
{"substr",	Node_builtin,	 LEX_BUILTIN,	A(2)|A(3),	do_substr},
#ifdef ALLOW_SWITCH
{"switch",	Node_K_switch,	 LEX_SWITCH,	GAWKX,		0},
#endif
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
#else /* MBS_SUPPORT */
/* a dummy */
#define nextc_is_1stbyte 1
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
#ifdef CAN_USE_STDARG_H
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
	 *
	 * 8/2003: We may not need this anymore.
	 */
	save = *bp;
	*bp = '\0';

	msg("%.*s", (int) (bp - thisline), thisline);

	*bp = save;

#ifdef CAN_USE_STDARG_H
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
	static size_t buflen = 0;
	static int fd;

	/*
	 * No argument prototype on readfunc on purpose,
	 * avoids problems with some ancient systems where
	 * the types of arguments to read() aren't up to date.
	 */
	static ssize_t (*readfunc)() = 0;
	static int (*closefunc)P((int fd)) = NULL;

	int n;
	register char *scan;
	int newfile;
	struct stat sbuf;
	int readcount = 0;
	int l;
	char *readloc;

	if (readfunc == NULL) {
		char *cp = getenv("AWKREADFUNC");

		/* If necessary, one day, test value for different functions.  */
		if (cp == NULL) {
			readfunc = read;
			closefunc = close;
		} else {
			readfunc = read_one_line;
			closefunc = one_line_close;
		}
	}

again:
	newfile = FALSE;
	if (nextfile > numfiles)
		return NULL;

	if (srcfiles[nextfile].stype == CMDLINE) {
		if ((l = strlen(srcfiles[nextfile].val)) == 0) {
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
			++nextfile;
			goto again;
		}
		if (srcfiles[nextfile].val[l-1] == '\n') {
			/* has terminating newline, can use it directly */
			sourceline = 1;
			source = NULL;
			lexptr = lexptr_begin = srcfiles[nextfile].val;
			/* fall through to pointer adjustment and return, below */
		} else {
			/* copy it into static buffer */

			/* make sure buffer exists and has room */
			if (buflen == 0) {
				emalloc(buf, char *, l+2, "get_src_buf");
				buflen = l + 2;
			} else if (l+2 > buflen) {
				erealloc(buf, char *, l+2, "get_src_buf");
				buflen = l + 2;
			} /* else
				buffer has room, just use it */

			/* copy in data */
			memcpy(buf, srcfiles[nextfile].val, l);
			buf[l] = '\n';
			buf[++l] = '\0';

			/* set vars and return */
			sourceline = 0;
			source = NULL;
			lexptr = lexptr_begin = buf;
		}
		lexend = lexptr + l;
		nextfile++;	/* for next entry to this routine */
		return lexptr;
	}

	if (! samefile) {
		source = srcfiles[nextfile].val;
		if (source == NULL) {	/* read all the source files, all done */
			if (buf != NULL) {
				free(buf);
				buf = NULL;
			}
			buflen = 0;
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
		l = optimal_bufsize(fd, & sbuf);
		/*
		 * Make sure that something silly like
		 * 	AWKBUFSIZE=8 make check
		 * works ok.
		 */
#define A_DECENT_BUFFER_SIZE	128
		if (l < A_DECENT_BUFFER_SIZE)
			l = A_DECENT_BUFFER_SIZE;
#undef A_DECENT_BUFFER_SIZE

		newfile = TRUE;

		/* make sure buffer exists and has room */
		if (buflen == 0) {
			emalloc(buf, char *, l+2, "get_src_buf");
			buflen = l + 2;
		} else if (l+2 > buflen) {
			erealloc(buf, char *, l+2, "get_src_buf");
			buflen = l + 2;
		} /* else
			buffer has room, just use it */

		readcount = l;
		readloc = lexeme = lexptr = lexptr_begin = buf;
		samefile = TRUE;
		sourceline = 1;
	} else {
		/*
		 * In same file, ran off edge of buffer.
		 * Shift current line down to front, adjust
		 * pointers and fill in the rest of the buffer.
		 */

		int lexeme_offset = lexeme - lexptr_begin;
		int lexptr_offset = lexptr - lexptr_begin;
		int lexend_offset = lexend - lexptr_begin;

		/* find beginning of current line */
		for (scan = lexeme; scan >= lexptr_begin; scan--) {
			if (*scan == '\n') {
				scan++;
				break;
			}
		}

		/*
		 * This condition can be read as follows: IF
		 * 1. The beginning of the line is at the beginning of the
		 *    buffer (no newline was found: scan <= buf)
		 * AND:
		 *    2. The start of valid lexical data is into the buffer
		 *       (lexptr_begin > buf)
		 *       OR:
		 *       3. We have scanned past the end of the last data read
		 *          (lexptr == lexend)
		 *          AND:
		 *          4. There's no room left in the buffer
		 *             (lexptr_offset >= buflen - 2)
		 *
		 * If all that's true, grow the buffer to add more to
		 * the current line.
		 */

		if (scan <= buf
		    && (lexptr_begin > buf
			|| (lexptr == lexend
			    && lexptr_offset >= buflen - 2))) {
			/* have to grow the buffer */
			buflen *= 2;
			erealloc(buf, char *, buflen, "get_src_buf");
		} else if (scan > buf) {
			/* Line starts in middle of the buffer, shift things down. */
			memmove(buf, scan, lexend - scan);
			/*
			 * make offsets relative to start of line,
			 * not start of buffer.
			 */
			lexend_offset = lexend - scan;
			lexeme_offset = lexeme - scan;
			lexptr_offset = lexptr - scan;
		}

		/* adjust pointers */
		lexeme = buf + lexeme_offset;
		lexptr = buf + lexptr_offset;
		lexend = buf + lexend_offset;
		lexptr_begin = buf;
		readcount = buflen - (lexend - buf);
		readloc = lexend;
	}

	/* add more data to buffer */
	n = (*readfunc)(fd, readloc, readcount);
	if (n == -1)
		fatal(_("can't read sourcefile `%s' (%s)"),
			source, strerror(errno));
	if (n == 0) {
		if (newfile) {
			static short warned = FALSE;

			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn(_("source file `%s' is empty"), source);
			}
		}
		if (fd != fileno(stdin)) /* safety */
			(*closefunc)(fd);
		samefile = FALSE;
		nextfile++;
		goto again;
	}
	lexend = lexptr + n;
	return lexptr;
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
	if (gawk_mb_cur_max > 1) {
		if (!lexptr || lexptr >= lexend) {
			if (! get_src_buf())
				return EOF;
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

static inline void
pushback(void)
{
#ifdef MBS_SUPPORT
	if (gawk_mb_cur_max > 1)
		cur_ring_idx = (cur_ring_idx == 0)? RING_BUFFER_SIZE - 1 :
			cur_ring_idx - 1;
#endif
	(lexptr && lexptr > lexptr_begin ? lexptr-- : lexptr);
}


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
	register int c;
	int seen_e = FALSE;		/* These are for numbers */
	int seen_point = FALSE;
	int esc_seen;		/* for literal strings */
	int mid;
	static int did_newline = FALSE;
	char *tokkey;
	static int lasttok = 0;
	static short eof_warned = FALSE;
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
				if ((c = nextc()) == EOF) {
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
				tokadd('\0');
				yylval.sval = tokstart;
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
			case EOF:
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
	yylval.nodetypeval = Node_illegal;

	if (gawk_mb_cur_max == 1 || nextc_is_1stbyte) switch (c) {
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
			exit(EXIT_FAILURE);
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
				exit(EXIT_FAILURE);
			}
			if ((gawk_mb_cur_max == 1 || nextc_is_1stbyte) &&
			    c == '\\') {
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
				exit(EXIT_FAILURE);
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
		if (c != EOF)
			pushback();
		else if (do_lint && ! eof_warned) {
			lintwarn(_("source file does not end in newline"));
			eof_warned = TRUE;
		}
		tokadd('\0');
		if (! do_traditional && isnondecimal(tokstart, FALSE)) {
			if (do_lint) {
				if (isdigit(tokstart[1]))	/* not an 'x' or 'X' */
					lintwarn("numeric constant `%.*s' treated as octal",
						(int) strlen(tokstart)-1, tokstart);
				else if (tokstart[1] == 'x' || tokstart[1] == 'X')
					lintwarn("numeric constant `%.*s' treated as hexadecimal",
						(int) strlen(tokstart)-1, tokstart);
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

	if (c != '_' && ! isalpha(c)) {
		yyerror(_("invalid char '%c' in expression"), c);
		exit(EXIT_FAILURE);
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

	if ((mid = check_special(tokstart)) >= 0) {
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
			;
		else {
			if (tokentab[mid].class == LEX_BUILTIN
			    || tokentab[mid].class == LEX_LENGTH)
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
	/* if lookahead is a NL, lineno is 1 too high */
	if (lexeme && lexeme >= lexptr_begin && *lexeme == '\n')
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
			&& r->builtin == do_dcngettext		/* dcngettext(...) */
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
		if (r->lnode != NULL)	/* r->lnode set from subn. guard against syntax errors & check it's valid */
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

/* lookup --- find the most recent hash node for name installed by install */

NODE *
lookup(const char *name)
{
	register NODE *bucket;
	register size_t len;

	len = strlen(name);
	for (bucket = variables[hash(name, len, (unsigned long) HASHSIZE, NULL)];
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
		else if (p->hvalue->type == Node_var_new)
			fprintf(fp, "unused variable\n");
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
			else if (p->hvalue->type != Node_var_new) {
				NODE **lhs = get_lhs(p->hvalue, NULL, FALSE);

				unref(*lhs);
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
	struct finfo *tab = NULL;

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

	if (j == 0)
		return;

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
	if (savefront == oldlist)
		list = savetail; /* Be careful: maybe list->rnode != NULL */
	else
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
		else if (is_std_var(n->param))
			fatal(_("function `%s': can't use special variable `%s' as a function parameter"),
				params->param, n->param);
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
	save = &(variables[hash(name, len, (unsigned long) HASHSIZE, NULL)]);
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
variable(char *name, int can_free, NODETYPE type)
{
	register NODE *r;

	if ((r = lookup(name)) != NULL) {
		if (r->type == Node_func)
			fatal(_("function `%s' called with space between name and `(',\nor used as a variable or an array"),
				r->vname);

	} else {
		/* not found */
		struct deferred_variable *dv;

		for (dv = deferred_variables; TRUE; dv = dv->next) {
			if (dv == NULL) {
				/*
				 * This is the only case in which we may not
				 * free the string.
				 */
				NODE *n;

				if (type == Node_var_array)
					n = node(NULL, type, NULL);
				else
					n = node(Nnull_string, type, NULL);

				return install(name, n);
			}
			if (STREQ(name, dv->name)) {
				r = (*dv->load_func)();
				break;
			}
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
	n->re_cnt = 1;
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
	case Node_SUBSEP:
	case Node_TEXTDOMAIN:
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
	case Node_var_new:
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
	case Node_SUBSEP:
	case Node_TEXTDOMAIN:
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

/* See if name is a special token. */

int
check_special(const char *name)
{
	int low, high, mid;
	int i;
#if 'a' == 0x81 /* it's EBCDIC */
	static int did_sort = FALSE;

	if (! did_sort) {
		qsort(tokentab, sizeof(tokentab) / sizeof(tokentab[0]),
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
		else
			return mid;
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
			exit(EXIT_FAILURE);
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

/* constant_fold --- try to fold constant operations */

static NODE *
constant_fold(NODE *left, NODETYPE op, NODE *right)
{
	AWKNUM result;
	extern double fmod P((double x, double y));

	if (! do_optimize)
		return node(left, op, right);

	/* Unary not */
	if (right == NULL) {
		if (op == Node_not && left->type == Node_val) {
			if ((left->flags & (STRCUR|STRING)) != 0) {
				NODE *ret;
				if (left->stlen == 0) {
					ret = make_number((AWKNUM) 1.0);
				} else {
					ret = make_number((AWKNUM) 0.0);
				}
				unref(left);

				return ret;
			} else {
				if (left->numbr == 0) {
					left->numbr = 1.0;
				} else {
					left->numbr = 0.0;
				}

				return left;
			}
		}

		return node(left, op, right);
	}

	/* String concatentation of two string constants */
	if (op == Node_concat
	    && left->type == Node_val
	    && (left->flags & (STRCUR|STRING)) != 0
	    && right->type == Node_val
	    && (right->flags & (STRCUR|STRING)) != 0) {
		size_t newlen = left->stlen + right->stlen + 2;

		erealloc(left->stptr, char *, newlen, "constant_fold");
		memcpy(left->stptr + left->stlen, right->stptr, right->stlen);
		left->stptr[left->stlen + right->stlen] = '\0';
		left->stlen += right->stlen;

		unref(right);
		return left;
	}

	/*
	 * From here down, numeric operations.
	 * Check for string and bail out if have them.
	 */
	if (left->type != Node_val
	    || (left->flags & (STRCUR|STRING)) != 0
	    || right->type != Node_val
	    || (right->flags & (STRCUR|STRING)) != 0) {
		return node(left, op, right);
	}

	/* Numeric operations: */
	switch (op) {
	case Node_not:
	case Node_exp:
	case Node_times:
	case Node_quotient:
	case Node_mod:
	case Node_plus:
	case Node_minus:
		break;
	default:
		return node(left, op, right);
	}

	left->numbr = force_number(left);
	right->numbr = force_number(right);

	result = left->numbr;
	switch (op) {
	case Node_exp:
		result = calc_exp(left->numbr, right->numbr);
		break;
	case Node_times:
		result *= right->numbr;
		break;
	case Node_quotient:
		if (right->numbr == 0)
			fatal(_("division by zero attempted in `/'"));
		result /= right->numbr;
		break;
	case Node_mod:
		if (right->numbr == 0)
			fatal(_("division by zero attempted in `%%'"));
#ifdef HAVE_FMOD
		result = fmod(result, right->numbr);
#else	/* ! HAVE_FMOD */
		(void) modf(left->numbr / right->numbr, &result);
		result = left->numbr - result * right->numbr;
#endif	/* ! HAVE_FMOD */
		break;
	case Node_plus:
		result += right->numbr;
		break;
	case Node_minus:
		result -= right->numbr;
		break;
	default:
		/* Shut up compiler warnings */
		fatal("can't happen");
		break;
	}

	unref(right);
	left->numbr = result;

	return left;
}

/* optimize_concat --- optimize the general "x = x y z a" case */

static NODE *
optimize_concat(NODE *left, NODETYPE op, NODE *right)
{
	NODE *top, *leftmost;

	if (op != Node_assign)
		return node(left, op, right);

	/*
	 * optimization of `x = x y'.  can save lots of time
	 * if done a lot.
	 */
	if ((    left->type == Node_var
		|| left->type == Node_var_new
		|| left->type == Node_param_list)
	      && right->type == Node_concat) {
		/* find bottom of tree, save it */
		for (top = right; top->lnode != NULL && top->type == Node_concat; top = top->lnode) {
			leftmost = top->lnode;
			if (leftmost->type == Node_concat)
				continue;

			/* at this point, we've run out of concatentation */
			if (leftmost != left)
				return node(left, op, right);

			top->lnode = Nnull_string;
			return node(left, Node_assign_concat, right);
		}
	}
	return node(left, op, right);
}

