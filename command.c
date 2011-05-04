/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.3"

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

/* Substitute the variable and function names.  */
#define yyparse         zzparse
#define yylex           zzlex
#define yyerror         zzerror
#define yylval          zzlval
#define yychar          zzchar
#define yydebug         zzdebug
#define yynerrs         zznerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 26 "command.y"

#include "awk.h"
#include "cmd.h"

#if 0
#define YYDEBUG 12
int yydebug = 2;
#endif

static int yylex(void);
static void yyerror(const char *mesg, ...);

static int find_command(const char *token, size_t toklen);

static int want_nodeval = FALSE;

static int cmd_idx = -1;			/* index of current command in cmd table */
static int repeat_idx = -1;			/* index of last repeatable command in command table */
static CMDARG *arg_list = NULL;		/* list of arguments */ 
static long errcount = 0;
static char *lexptr_begin = NULL;
static int in_commands = FALSE;
static int num_dim;

static int in_eval = FALSE;
static const char start_EVAL[] = "function @eval(){";
static const char end_EVAL[] = "}";	
static CMDARG *append_statement(CMDARG *alist, char *stmt);
static char *next_word(char *p, int len, char **endp);
static NODE *concat_args(CMDARG *a, int count);

#ifdef HAVE_LIBREADLINE
static void history_expand_line(char **line);
static char *command_generator(const char *text, int state);
static char *srcfile_generator(const char *text, int state);
static char *argument_generator(const char *text, int state);
static char *variable_generator(const char *text, int state);
extern char *option_generator(const char *text, int state);
static int this_cmd = D_illegal;
#else
#define history_expand_line(p)	/* nothing */
static int rl_inhibit_completion;	/* dummy variable */
#endif

struct argtoken {
	const char *name;
	enum argtype cmd;
	enum nametypeval value;
};

/*
 * These two should be static, but there are some compilers that
 * don't like the static keyword with an empty size. Therefore give
 * them names that are less likely to conflict with the rest of gawk.
 */
#define argtab zz_debug_argtab
#define cmdtab zz_debug_cmdtab

extern struct argtoken argtab[];
extern struct cmdtoken cmdtab[];

static CMDARG *mk_cmdarg(enum argtype type);
static void append_cmdarg(CMDARG *arg);
static int find_argument(CMDARG *arg);
#define YYSTYPE CMDARG *


/* Line 189 of yacc.c  */
#line 148 "command.c"

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
     D_BACKTRACE = 258,
     D_BREAK = 259,
     D_CLEAR = 260,
     D_CONTINUE = 261,
     D_DELETE = 262,
     D_DISABLE = 263,
     D_DOWN = 264,
     D_ENABLE = 265,
     D_FINISH = 266,
     D_FRAME = 267,
     D_HELP = 268,
     D_IGNORE = 269,
     D_INFO = 270,
     D_LIST = 271,
     D_NEXT = 272,
     D_NEXTI = 273,
     D_PRINT = 274,
     D_PRINTF = 275,
     D_QUIT = 276,
     D_RETURN = 277,
     D_RUN = 278,
     D_SET = 279,
     D_STEP = 280,
     D_STEPI = 281,
     D_TBREAK = 282,
     D_UP = 283,
     D_UNTIL = 284,
     D_DISPLAY = 285,
     D_UNDISPLAY = 286,
     D_WATCH = 287,
     D_UNWATCH = 288,
     D_DUMP = 289,
     D_TRACE = 290,
     D_INT = 291,
     D_STRING = 292,
     D_NODE = 293,
     D_VARIABLE = 294,
     D_OPTION = 295,
     D_COMMANDS = 296,
     D_END = 297,
     D_SILENT = 298,
     D_SOURCE = 299,
     D_SAVE = 300,
     D_EVAL = 301,
     D_CONDITION = 302,
     D_STATEMENT = 303
   };
#endif
/* Tokens.  */
#define D_BACKTRACE 258
#define D_BREAK 259
#define D_CLEAR 260
#define D_CONTINUE 261
#define D_DELETE 262
#define D_DISABLE 263
#define D_DOWN 264
#define D_ENABLE 265
#define D_FINISH 266
#define D_FRAME 267
#define D_HELP 268
#define D_IGNORE 269
#define D_INFO 270
#define D_LIST 271
#define D_NEXT 272
#define D_NEXTI 273
#define D_PRINT 274
#define D_PRINTF 275
#define D_QUIT 276
#define D_RETURN 277
#define D_RUN 278
#define D_SET 279
#define D_STEP 280
#define D_STEPI 281
#define D_TBREAK 282
#define D_UP 283
#define D_UNTIL 284
#define D_DISPLAY 285
#define D_UNDISPLAY 286
#define D_WATCH 287
#define D_UNWATCH 288
#define D_DUMP 289
#define D_TRACE 290
#define D_INT 291
#define D_STRING 292
#define D_NODE 293
#define D_VARIABLE 294
#define D_OPTION 295
#define D_COMMANDS 296
#define D_END 297
#define D_SILENT 298
#define D_SOURCE 299
#define D_SAVE 300
#define D_EVAL 301
#define D_CONDITION 302
#define D_STATEMENT 303




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 286 "command.c"

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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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


#if (! defined yyoverflow      && (! defined __cplusplus 	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   203

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNRULES -- Number of states.  */
#define YYNSTATES  203

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   303

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      58,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    57,     2,     2,     2,
       2,     2,     2,    53,    50,    54,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,     2,
       2,    49,     2,     2,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    55,     2,    56,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    12,    15,    17,    19,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    46,    51,    52,    53,    58,    62,    66,
      69,    71,    73,    75,    78,    81,    84,    88,    91,    92,
      96,    97,   101,   104,   107,   110,   113,   114,   120,   123,
     124,   128,   129,   133,   134,   139,   142,   145,   148,   151,
     154,   156,   158,   161,   162,   167,   169,   171,   173,   175,
     176,   178,   180,   183,   187,   189,   190,   192,   194,   196,
     197,   199,   203,   205,   206,   208,   210,   214,   218,   219,
     220,   224,   226,   227,   233,   237,   238,   240,   241,   243,
     244,   246,   247,   249,   251,   254,   256,   259,   263,   265,
     268,   272,   274,   276,   278,   280,   284,   286,   287,   289,
     291,   293,   295,   297,   301,   305,   309,   313,   314,   316,
     318,   320,   322,   325,   328,   330,   334,   336,   340,   344,
     346,   349,   351,   354,   357,   359,   362,   365,   366,   368,
     369,   371,   373,   376,   378,   381,   384
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      60,     0,    -1,    -1,    60,    61,    -1,   113,    -1,    71,
     113,    -1,     1,   113,    -1,     6,    -1,    17,    -1,    18,
      -1,    25,    -1,    26,    -1,    31,    -1,    33,    -1,     8,
      -1,     7,    -1,    28,    -1,     9,    -1,     3,    -1,    12,
      -1,     4,    -1,    27,    -1,    -1,    46,    66,    81,   113,
      -1,    -1,    -1,    68,    48,    69,   113,    -1,    67,    68,
      42,    -1,    46,    66,    84,    -1,    13,    94,    -1,    21,
      -1,    23,    -1,    11,    -1,    62,   109,    -1,    64,   110,
      -1,    15,    37,    -1,    14,   111,    36,    -1,    10,    95,
      -1,    -1,    19,    72,    97,    -1,    -1,    20,    73,    99,
      -1,    16,   100,    -1,    29,    87,    -1,     5,    87,    -1,
      65,    88,    -1,    -1,    24,    74,   107,    49,   108,    -1,
      40,    85,    -1,    -1,    22,    75,    93,    -1,    -1,    30,
      76,    91,    -1,    -1,    32,    77,   107,    79,    -1,    63,
     102,    -1,    34,    92,    -1,    44,    37,    -1,    45,    37,
      -1,    41,    80,    -1,    42,    -1,    43,    -1,    35,    37,
      -1,    -1,    47,   111,    78,    79,    -1,    70,    -1,    83,
      -1,   109,    -1,     1,    -1,    -1,    82,    -1,    39,    -1,
      82,    39,    -1,    82,    50,    39,    -1,     1,    -1,    -1,
      84,    -1,     1,    -1,    38,    -1,    -1,    37,    -1,    37,
      49,    37,    -1,    37,    -1,    -1,   111,    -1,    86,    -1,
      37,    51,   111,    -1,    37,    51,    86,    -1,    -1,    -1,
     111,    89,    79,    -1,    86,    -1,    -1,    37,    51,   111,
      90,    79,    -1,    37,    51,    86,    -1,    -1,   107,    -1,
      -1,    37,    -1,    -1,   108,    -1,    -1,    37,    -1,   102,
      -1,    37,   102,    -1,   107,    -1,    52,    39,    -1,    52,
      39,   106,    -1,    96,    -1,    97,    96,    -1,    97,    50,
      96,    -1,     1,    -1,    38,    -1,   107,    -1,    98,    -1,
      99,    50,    98,    -1,     1,    -1,    -1,    53,    -1,    54,
      -1,   111,    -1,    86,    -1,   101,    -1,    37,    51,   111,
      -1,    37,    51,    86,    -1,    37,    51,   101,    -1,   111,
      54,   111,    -1,    -1,   103,    -1,     1,    -1,   111,    -1,
     101,    -1,   103,   111,    -1,   103,   101,    -1,   108,    -1,
     104,    50,   108,    -1,     1,    -1,    55,   104,    56,    -1,
      55,   104,     1,    -1,   105,    -1,   106,   105,    -1,    39,
      -1,    57,    38,    -1,    39,   106,    -1,    38,    -1,    53,
      38,    -1,    54,    38,    -1,    -1,   111,    -1,    -1,   112,
      -1,    36,    -1,    53,    36,    -1,    36,    -1,    53,    36,
      -1,    54,    36,    -1,    58,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   106,   106,   108,   126,   127,   177,   184,   185,   186,
     187,   188,   192,   193,   194,   195,   199,   200,   201,   202,
     206,   207,   212,   216,   235,   242,   242,   249,   265,   279,
     280,   281,   282,   283,   284,   290,   302,   303,   304,   304,
     305,   305,   306,   307,   308,   309,   310,   310,   311,   312,
     312,   313,   313,   314,   314,   315,   316,   317,   322,   327,
     353,   363,   368,   380,   380,   388,   402,   415,   416,   422,
     423,   427,   428,   429,   430,   436,   437,   438,   443,   454,
     455,   460,   468,   485,   486,   487,   488,   489,   494,   495,
     495,   496,   497,   497,   498,   503,   504,   509,   510,   515,
     516,   519,   521,   525,   526,   541,   542,   547,   555,   556,
     557,   558,   562,   563,   567,   568,   569,   574,   575,   577,
     584,   585,   586,   587,   588,   589,   593,   606,   607,   608,
     612,   613,   614,   615,   619,   621,   623,   627,   642,   646,
     648,   653,   654,   663,   673,   675,   682,   695,   696,   702,
     703,   708,   714,   723,   725,   727,   735
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "D_BACKTRACE", "D_BREAK", "D_CLEAR",
  "D_CONTINUE", "D_DELETE", "D_DISABLE", "D_DOWN", "D_ENABLE", "D_FINISH",
  "D_FRAME", "D_HELP", "D_IGNORE", "D_INFO", "D_LIST", "D_NEXT", "D_NEXTI",
  "D_PRINT", "D_PRINTF", "D_QUIT", "D_RETURN", "D_RUN", "D_SET", "D_STEP",
  "D_STEPI", "D_TBREAK", "D_UP", "D_UNTIL", "D_DISPLAY", "D_UNDISPLAY",
  "D_WATCH", "D_UNWATCH", "D_DUMP", "D_TRACE", "D_INT", "D_STRING",
  "D_NODE", "D_VARIABLE", "D_OPTION", "D_COMMANDS", "D_END", "D_SILENT",
  "D_SOURCE", "D_SAVE", "D_EVAL", "D_CONDITION", "D_STATEMENT", "'='",
  "','", "':'", "'@'", "'+'", "'-'", "'['", "']'", "'$'", "'\\n'",
  "$accept", "input", "line", "control_cmd", "d_cmd", "frame_cmd",
  "break_cmd", "set_want_nodeval", "eval_prologue", "statement_list", "@1",
  "eval_cmd", "command", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "condition_exp", "commands_arg", "opt_param_list", "param_list",
  "opt_string_node", "string_node", "option_args", "func_name", "location",
  "break_args", "$@9", "$@10", "opt_variable", "opt_string", "opt_node",
  "help_args", "enable_args", "print_exp", "print_args", "printf_exp",
  "printf_args", "list_args", "integer_range", "opt_integer_list",
  "integer_list", "exp_list", "subscript", "subscript_list", "variable",
  "node", "opt_plus_integer", "opt_integer", "plus_integer", "integer",
  "nls", 0
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,    61,
      44,    58,    64,    43,    45,    91,    93,    36,    10
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    59,    60,    60,    61,    61,    61,    62,    62,    62,
      62,    62,    63,    63,    63,    63,    64,    64,    64,    64,
      65,    65,    66,    67,    68,    69,    68,    70,    70,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    72,    71,
      73,    71,    71,    71,    71,    71,    74,    71,    71,    75,
      71,    76,    71,    77,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    78,    71,    71,    79,    80,    80,    81,
      81,    82,    82,    82,    82,    83,    83,    83,    84,    85,
      85,    85,    86,    87,    87,    87,    87,    87,    88,    89,
      88,    88,    90,    88,    88,    91,    91,    92,    92,    93,
      93,    94,    94,    95,    95,    96,    96,    96,    97,    97,
      97,    97,    98,    98,    99,    99,    99,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   101,   102,   102,   102,
     103,   103,   103,   103,   104,   104,   104,   105,   105,   106,
     106,   107,   107,   107,   108,   108,   108,   109,   109,   110,
     110,   111,   111,   112,   112,   112,   113
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     4,     0,     0,     4,     3,     3,     2,
       1,     1,     1,     2,     2,     2,     3,     2,     0,     3,
       0,     3,     2,     2,     2,     2,     0,     5,     2,     0,
       3,     0,     3,     0,     4,     2,     2,     2,     2,     2,
       1,     1,     2,     0,     4,     1,     1,     1,     1,     0,
       1,     1,     2,     3,     1,     0,     1,     1,     1,     0,
       1,     3,     1,     0,     1,     1,     3,     3,     0,     0,
       3,     1,     0,     5,     3,     0,     1,     0,     1,     0,
       1,     0,     1,     1,     2,     1,     2,     3,     1,     2,
       3,     1,     1,     1,     1,     3,     1,     0,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     0,     1,     1,
       1,     1,     2,     2,     1,     3,     1,     3,     3,     1,
       2,     1,     2,     2,     1,     2,     2,     0,     1,     0,
       1,     1,     2,     1,     2,     2,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,    18,    20,    83,     7,    15,    14,
      17,     0,    32,    19,   101,     0,     0,   117,     8,     9,
      38,    40,    30,    49,    31,    46,    10,    11,    21,    16,
      83,    51,    12,    53,    13,    97,     0,    79,     0,    60,
      61,     0,     0,    22,     0,   156,     3,   147,     0,   149,
      88,    24,    65,     0,     4,     6,   151,    82,     0,    85,
      44,    84,   129,     0,    37,   131,   103,   128,   130,   102,
      29,     0,    35,    82,   118,   119,   121,    42,   122,   120,
       0,     0,    99,     0,    43,    95,     0,    98,    56,    62,
      80,    48,    68,    59,    67,   148,    57,    58,     0,    63,
      33,    55,   153,     0,     0,    34,   150,    82,    91,    45,
      89,     0,     5,     0,   152,   104,   133,   132,     0,    36,
       0,   111,   141,     0,     0,   108,    39,   105,   116,   112,
     114,    41,   113,   144,     0,     0,    50,   100,     0,    52,
      96,     0,     0,    74,    78,    71,     0,    70,    28,     0,
     154,   155,     0,     0,    27,    25,    82,    87,    86,   126,
     124,   125,   123,     0,   139,   143,   106,   142,     0,   109,
       0,   145,   146,     0,    77,    54,    66,    76,    81,    23,
      72,     0,    64,    94,    92,    90,     0,   136,     0,   134,
     140,   107,   110,   115,    47,    73,     0,    26,   138,     0,
     137,    93,   135
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    46,    47,    48,    49,    50,    98,    51,   111,
     186,    52,    53,    80,    81,    83,    82,    85,    86,   149,
     175,    93,   146,   147,   176,   177,    91,    59,    60,   109,
     153,   196,   139,    88,   136,    70,    64,   125,   126,   130,
     131,    77,    65,    66,    67,   188,   164,   165,   127,   137,
      94,   105,    68,   106,    54
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -151
static const yytype_int16 yypact[] =
{
    -151,   145,  -151,   -34,  -151,  -151,    50,  -151,  -151,  -151,
    -151,    10,  -151,  -151,   -10,    59,    -9,    43,  -151,  -151,
    -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,
      50,  -151,  -151,  -151,  -151,    -8,    -6,    14,    12,  -151,
    -151,    22,    23,  -151,    59,  -151,  -151,    59,    13,    36,
      64,  -151,  -151,   -34,  -151,  -151,  -151,    24,    47,  -151,
    -151,  -151,  -151,    13,  -151,  -151,  -151,    59,    48,  -151,
    -151,    80,  -151,    67,    47,  -151,  -151,  -151,  -151,    48,
       4,    19,    69,   -20,  -151,   -20,   -20,  -151,  -151,  -151,
      70,  -151,  -151,  -151,  -151,  -151,  -151,  -151,    16,  -151,
    -151,  -151,  -151,    84,    85,  -151,  -151,    73,  -151,  -151,
    -151,    40,  -151,    74,  -151,  -151,  -151,    48,    59,  -151,
      74,  -151,    71,    89,    91,  -151,    42,  -151,  -151,  -151,
    -151,    81,  -151,  -151,    92,    94,  -151,  -151,    86,  -151,
    -151,     6,    96,  -151,  -151,  -151,   -34,    75,  -151,     6,
    -151,  -151,    74,     6,  -151,  -151,  -151,  -151,  -151,  -151,
    -151,  -151,    48,    31,  -151,    71,    71,  -151,    52,  -151,
     -17,  -151,  -151,    69,  -151,  -151,  -151,  -151,  -151,  -151,
    -151,    95,  -151,  -151,  -151,  -151,   -34,  -151,    17,  -151,
    -151,    71,  -151,  -151,  -151,  -151,     6,  -151,  -151,    69,
    -151,  -151,  -151
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,
    -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,  -151,
    -119,  -151,  -151,  -151,  -151,    38,  -151,   -15,   108,  -151,
    -151,  -151,  -151,  -151,  -151,  -151,  -151,   -90,  -151,   -31,
    -151,  -151,   -14,   -25,  -151,  -151,  -150,   -26,   -77,  -147,
      97,  -151,    -5,  -151,    -3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -148
static const yytype_int16 yytable[] =
{
      55,    61,    76,    78,   132,   121,   138,   174,   140,   141,
      71,    62,    79,    92,    62,   190,   189,   143,   198,   122,
     128,   129,   122,   101,    45,    61,   194,    69,    72,    87,
     182,    89,   187,    95,   185,   108,   169,   124,   115,    99,
     124,   190,    95,   122,   144,   110,    56,    63,    56,    56,
     112,    90,   202,   116,   144,   145,   123,   129,   122,    96,
      97,   124,   117,    58,   -75,    58,    58,   199,  -127,   133,
    -147,  -127,   102,   200,   -69,   113,   124,   201,   192,    56,
      73,   122,   154,   114,   134,   135,    56,    57,   155,   103,
     104,   122,   168,   132,   123,    56,    74,    75,   157,   124,
      56,   107,   118,    58,   123,   160,   161,   133,   158,   124,
      56,   156,    58,   159,   180,   162,   119,    58,   120,   142,
     150,   151,   134,   135,   152,   181,   163,    58,   166,   167,
     171,   170,   172,   178,   195,   173,   148,   183,    84,   193,
     191,     0,     0,   179,   100,     2,     3,   184,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,     0,     0,   197,     0,    37,    38,    39,    40,    41,
      42,    43,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    45
};

static const yytype_int16 yycheck[] =
{
       3,     6,    17,    17,    81,     1,    83,     1,    85,    86,
      15,     1,    17,     1,     1,   165,   163,     1,     1,    39,
       1,    38,    39,    48,    58,    30,   173,    37,    37,    37,
     149,    37,     1,    38,   153,    50,   126,    57,    63,    44,
      57,   191,    47,    39,    38,    50,    36,    37,    36,    36,
      53,    37,   199,    67,    38,    39,    52,    38,    39,    37,
      37,    57,    67,    53,    58,    53,    53,    50,    58,    38,
      58,    58,    36,    56,    58,    51,    57,   196,   168,    36,
      37,    39,    42,    36,    53,    54,    36,    37,    48,    53,
      54,    39,    50,   170,    52,    36,    53,    54,   113,    57,
      36,    37,    54,    53,    52,   120,   120,    38,   113,    57,
      36,    37,    53,   118,    39,   120,    36,    53,    51,    49,
      36,    36,    53,    54,    51,    50,    55,    53,    39,    38,
      38,    50,    38,    37,    39,    49,    98,   152,    30,   170,
     166,    -1,    -1,   146,    47,     0,     1,   152,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    -1,   186,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    58
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    60,     0,     1,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    40,    41,    42,
      43,    44,    45,    46,    47,    58,    61,    62,    63,    64,
      65,    67,    70,    71,   113,   113,    36,    37,    53,    86,
      87,   111,     1,    37,    95,   101,   102,   103,   111,    37,
      94,   111,    37,    37,    53,    54,    86,   100,   101,   111,
      72,    73,    75,    74,    87,    76,    77,    37,    92,    37,
      37,    85,     1,    80,   109,   111,    37,    37,    66,   111,
     109,   102,    36,    53,    54,   110,   112,    37,    86,    88,
     111,    68,   113,    51,    36,   102,   101,   111,    54,    36,
      51,     1,    39,    52,    57,    96,    97,   107,     1,    38,
      98,    99,   107,    38,    53,    54,    93,   108,   107,    91,
     107,   107,    49,     1,    38,    39,    81,    82,    84,    78,
      36,    36,    51,    89,    42,    48,    37,    86,   111,   111,
      86,   101,   111,    55,   105,   106,    39,    38,    50,    96,
      50,    38,    38,    49,     1,    79,    83,    84,    37,   113,
      39,    50,    79,    86,   111,    79,    69,     1,   104,   108,
     105,   106,    96,    98,   108,    39,    90,   113,     1,    50,
      56,    79,   108
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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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

#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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

#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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

#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__      || defined __cplusplus || defined _MSC_VER)
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
        case 3:

/* Line 1464 of yacc.c  */
#line 109 "command.y"
    {
		cmd_idx = -1;
		want_nodeval = FALSE;
		if (lexptr_begin != NULL) {
			if (input_from_tty && lexptr_begin[0] != '\0')
				add_history(lexptr_begin);
			efree(lexptr_begin);
			lexptr_begin = NULL;
		}
		if (arg_list != NULL) {
			free_cmdarg(arg_list);
			arg_list = NULL;
		}
	  }
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 128 "command.y"
    {
		if (errcount == 0 && cmd_idx >= 0) {
			Func_cmd cmdfunc;
			int terminate = FALSE;
			CMDARG *args;
			int ctype = 0;
			
			ctype = cmdtab[cmd_idx].type;

			/* a blank line repeats previous command
			 * (list, next, nexti, step, stepi and continue without arguments).
			 * save the index in the command table; used in yylex
			 */
			if ((ctype == D_list
					|| ctype == D_next
					|| ctype == D_step
					|| ctype == D_nexti
					|| ctype == D_stepi
					|| ctype == D_continue)
				&& arg_list == NULL
				&& ! in_commands
				&& input_from_tty
			)
				repeat_idx = cmd_idx;
			else
				repeat_idx = -1;

			/* call the command handler; reset the globals arg_list, cmd_idx,
			 * since this handler could invoke yyparse again.
			 * call do_commands for the list of commands in `commands';
			 * arg_list isn't freed on return.
			 */

			cmdfunc = cmdtab[cmd_idx].cf_ptr;
			if (in_commands)
				cmdfunc = do_commands;
			cmd_idx = -1;
			want_nodeval = FALSE;

			args = arg_list;
			arg_list = NULL;

			terminate = (*cmdfunc)(args, ctype);
			if (! in_commands || ctype == D_commands)
				free_cmdarg(args);
			if (terminate)
				YYACCEPT;
		}
	  }
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 178 "command.y"
    {
		yyerrok;
	  }
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 212 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 217 "command.y"
    {
		if (errcount == 0) {
			/* don't free arg_list;	passed on to statement_list
			 * non-terminal (empty rule action). See below.
			 */
			if (input_from_tty) {
				dPrompt = eval_Prompt;
				fprintf(out_fp, _("Type (g)awk statement(s). End with the command \"end\"\n"));
				rl_inhibit_completion = 1;
			}
			cmd_idx = -1;
			in_eval = TRUE;
		}
	  }
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 235 "command.y"
    {
		(yyval) = append_statement(arg_list, (char *) start_EVAL);
		if (read_a_line == read_commands_string)	/* unserializing 'eval' in 'commands' */
			(yyval)->a_string[0] = '\0';
		free_cmdarg(arg_list);
		arg_list = NULL;
	  }
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 242 "command.y"
    { (yyval) = append_statement((yyvsp[(1) - (2)]), lexptr_begin); }
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 243 "command.y"
    {
		(yyval) = (yyvsp[(3) - (4)]);
	  }
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 250 "command.y"
    {
		arg_list = append_statement((yyvsp[(2) - (3)]), (char *) end_EVAL);
		if (read_a_line == read_commands_string) {	/* unserializing 'eval' in 'commands' */
			char *str = arg_list->a_string;
			size_t len = strlen(str);
			assert(len > 2 && str[len - 2] == '}');
			str[len - 2] = '\0';
		}
		if (input_from_tty) {
			dPrompt = in_commands ? commands_Prompt : dgawk_Prompt;
			rl_inhibit_completion = 0;
		}
		cmd_idx = find_command("eval", 4);
		in_eval = FALSE;
	  }
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 266 "command.y"
    {
		NODE *n;
		CMDARG *arg;
		n = (yyvsp[(3) - (3)])->a_node;
		arg = append_statement(NULL, (char *) start_EVAL);
		(void) append_statement(arg, n->stptr);
		(void) append_statement(arg, (char *) end_EVAL);
		free_cmdarg(arg_list);
		arg_list = arg;
	  }
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 285 "command.y"
    {
		if (cmdtab[cmd_idx].class == D_FRAME
				&& (yyvsp[(2) - (2)]) != NULL && (yyvsp[(2) - (2)])->a_int < 0)
			yyerror(_("invalid frame number: %d"), (yyvsp[(2) - (2)])->a_int);
	  }
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 291 "command.y"
    {
		int idx = find_argument((yyvsp[(2) - (2)]));
		if (idx < 0)
			yyerror(_("info: invalid option - \"%s\""), (yyvsp[(2) - (2)])->a_string);
		else {
			efree((yyvsp[(2) - (2)])->a_string);
			(yyvsp[(2) - (2)])->a_string = NULL;
			(yyvsp[(2) - (2)])->type = D_argument;
			(yyvsp[(2) - (2)])->a_argument = argtab[idx].value;
		}
	  }
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 304 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 305 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 310 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 312 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 313 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 314 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 318 "command.y"
    {
		if (in_cmd_src((yyvsp[(2) - (2)])->a_string))
			yyerror(_("source \"%s\": already sourced."), (yyvsp[(2) - (2)])->a_string);
	  }
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 323 "command.y"
    {
		if (! input_from_tty)
			yyerror(_("save \"%s\": command not permitted."), (yyvsp[(2) - (2)])->a_string);
	  }
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 328 "command.y"
    {
		int type = 0;
		int num;

		if ((yyvsp[(2) - (2)]) != NULL)
			num = (yyvsp[(2) - (2)])->a_int;

		if (errcount != 0)
			;
		else if (in_commands)
			yyerror(_("Can't use command `commands' for breakpoint/watchpoint commands"));
		else if ((yyvsp[(2) - (2)]) == NULL &&  ! (type = has_break_or_watch_point(&num, TRUE)))
			yyerror(_("no breakpoint/watchpoint has been set yet"));
		else if ((yyvsp[(2) - (2)]) != NULL && ! (type = has_break_or_watch_point(&num, FALSE)))
			yyerror(_("invalid breakpoint/watchpoint number"));
		if (type) {
			in_commands = TRUE;
			if (input_from_tty) {
				dPrompt = commands_Prompt; 
				fprintf(out_fp, _("Type commands for when %s %d is hit, one per line.\n"),
								(type == D_break) ? "breakpoint" : "watchpoint", num);
				fprintf(out_fp, _("End with the command \"end\"\n"));
			}
		}
	  }
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 354 "command.y"
    {
		if (! in_commands)
			yyerror(_("`end' valid only in command `commands' or `eval'"));
		else {
			if (input_from_tty)
				dPrompt = dgawk_Prompt;	
			in_commands = FALSE;
		}
	  }
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 364 "command.y"
    {
		if (! in_commands)
			yyerror(_("`silent' valid only in command `commands'"));
	  }
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 369 "command.y"
    {
		int idx = find_argument((yyvsp[(2) - (2)]));
		if (idx < 0)
			yyerror(_("trace: invalid option - \"%s\""), (yyvsp[(2) - (2)])->a_string);
		else {
			efree((yyvsp[(2) - (2)])->a_string);
			(yyvsp[(2) - (2)])->a_string = NULL;
			(yyvsp[(2) - (2)])->type = D_argument;
			(yyvsp[(2) - (2)])->a_argument = argtab[idx].value;
		}
	  }
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 380 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 381 "command.y"
    {
		int type;
		int num = (yyvsp[(2) - (4)])->a_int;
		type = has_break_or_watch_point(&num, FALSE);
		if (! type)
			yyerror(_("condition: invalid breakpoint/watchpoint number"));
	  }
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 389 "command.y"
    {
		if (in_commands) {
			/* Prepend command 'eval' to argument list */
			CMDARG *arg;
			arg = mk_cmdarg(D_string);
			arg->a_string = estrdup("eval", 4);
			arg->next = arg_list;
			arg_list = arg;
		}
	  }
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 403 "command.y"
    {
		if ((yyvsp[(1) - (1)]) != NULL) {
			NODE *n = (yyvsp[(1) - (1)])->a_node;
			(yyvsp[(1) - (1)])->type = D_string;
			(yyvsp[(1) - (1)])->a_string = n->stptr;
			freenode(n);
		}
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 417 "command.y"
    {	(yyval) = NULL; }
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 422 "command.y"
    { (yyval) = NULL; }
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 431 "command.y"
    { (yyval) = NULL; }
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 436 "command.y"
    { (yyval) = NULL; }
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 439 "command.y"
    { (yyval) = NULL; }
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 444 "command.y"
    {
		NODE *n;
		n = (yyvsp[(1) - (1)])->a_node;
		if ((n->flags & STRING) == 0)
			yyerror(_("argument not a string"));
	  }
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 454 "command.y"
    { (yyval) = NULL; }
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 456 "command.y"
    {
		if (find_option((yyvsp[(1) - (1)])->a_string) < 0)
			yyerror(_("option: invalid parameter - \"%s\""), (yyvsp[(1) - (1)])->a_string);
 	  }
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 461 "command.y"
    {
		if (find_option((yyvsp[(1) - (3)])->a_string) < 0)
			yyerror(_("option: invalid parameter - \"%s\""), (yyvsp[(1) - (3)])->a_string);
 	  }
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 469 "command.y"
    {
		NODE *n;
		n = lookup((yyvsp[(1) - (1)])->a_string);
		if (n == NULL || n->type != Node_func)
			yyerror(_("no such function - \"%s\""), (yyvsp[(1) - (1)])->a_string);
		else {
			(yyvsp[(1) - (1)])->type = D_func;
			efree((yyvsp[(1) - (1)])->a_string);
			(yyvsp[(1) - (1)])->a_string = NULL;
			(yyvsp[(1) - (1)])->a_node = n;
		}
	  }
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 485 "command.y"
    { (yyval) = NULL; }
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 494 "command.y"
    { (yyval) = NULL; }
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 495 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 497 "command.y"
    { want_nodeval = TRUE; }
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 503 "command.y"
    { (yyval) = NULL; }
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 509 "command.y"
    { (yyval) = NULL; }
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 515 "command.y"
    { (yyval) = NULL; }
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 527 "command.y"
    {
		int idx = find_argument((yyvsp[(1) - (2)]));
		if (idx < 0)
			yyerror(_("enable: invalid option - \"%s\""), (yyvsp[(1) - (2)])->a_string);
		else {
			efree((yyvsp[(1) - (2)])->a_string);
			(yyvsp[(1) - (2)])->a_string = NULL;
			(yyvsp[(1) - (2)])->type = D_argument;
			(yyvsp[(1) - (2)])->a_argument = argtab[idx].value;
		}
	  }
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 543 "command.y"
    {
		(yyvsp[(2) - (2)])->type = D_array;	/* dump all items */
		(yyvsp[(2) - (2)])->a_count = 0;
	  }
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 548 "command.y"
    {
		(yyvsp[(2) - (3)])->type = D_array;
		(yyvsp[(2) - (3)])->a_count = num_dim;
	  }
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 574 "command.y"
    { (yyval) = NULL; }
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 576 "command.y"
    { (yyval) = NULL; }
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 578 "command.y"
    {
		CMDARG *a;
		a = mk_cmdarg(D_int);
		a->a_int = -1;
		append_cmdarg(a);
	  }
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 594 "command.y"
    {
		if ((yyvsp[(1) - (3)])->a_int > (yyvsp[(3) - (3)])->a_int)
			yyerror(_("invalid range specification: %d - %d"),
				(yyvsp[(1) - (3)])->a_int, (yyvsp[(3) - (3)])->a_int);
		else
			(yyvsp[(1) - (3)])->type = D_range;
		(yyval) = (yyvsp[(1) - (3)]);
	  }
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 606 "command.y"
    { (yyval) = NULL; }
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 620 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 622 "command.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 628 "command.y"
    {
		CMDARG *a;
		NODE *subs;
		int count = 0;
		
		for (a = (yyvsp[(2) - (3)]); a != NULL; a = a->next)
			count++;
		subs = concat_args((yyvsp[(2) - (3)]), count);
		free_cmdarg((yyvsp[(2) - (3)])->next);
		(yyvsp[(2) - (3)])->next = NULL;
		(yyvsp[(2) - (3)])->type = D_node;
		(yyvsp[(2) - (3)])->a_node = subs;
		(yyval) = (yyvsp[(2) - (3)]);
	  }
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 647 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); num_dim = 1; }
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 649 "command.y"
    {	(yyval) = (yyvsp[(1) - (2)]); num_dim++; }
    break;

  case 142:

/* Line 1464 of yacc.c  */
#line 655 "command.y"
    {
		NODE *n = (yyvsp[(2) - (2)])->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value for field number"));
		else
			(yyvsp[(2) - (2)])->type = D_field;
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 143:

/* Line 1464 of yacc.c  */
#line 664 "command.y"
    {
		/* a_string is array name, a_count is dimension count */
		(yyvsp[(1) - (2)])->type = D_subscript;
		(yyvsp[(1) - (2)])->a_count = num_dim;
		(yyval) = (yyvsp[(1) - (2)]);
	  }
    break;

  case 144:

/* Line 1464 of yacc.c  */
#line 674 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 145:

/* Line 1464 of yacc.c  */
#line 676 "command.y"
    { 
		NODE *n = (yyvsp[(2) - (2)])->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 146:

/* Line 1464 of yacc.c  */
#line 683 "command.y"
    { 
		NODE *n = (yyvsp[(2) - (2)])->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		else
			(yyvsp[(2) - (2)])->a_node->numbr = - n->numbr;
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 147:

/* Line 1464 of yacc.c  */
#line 695 "command.y"
    { (yyval) = NULL; }
    break;

  case 148:

/* Line 1464 of yacc.c  */
#line 697 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 149:

/* Line 1464 of yacc.c  */
#line 702 "command.y"
    { (yyval) = NULL; }
    break;

  case 150:

/* Line 1464 of yacc.c  */
#line 704 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 151:

/* Line 1464 of yacc.c  */
#line 709 "command.y"
    {
		if ((yyvsp[(1) - (1)])->a_int == 0)
			yyerror(_("non-zero integer value"));
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 152:

/* Line 1464 of yacc.c  */
#line 715 "command.y"
    {
		if ((yyvsp[(2) - (2)])->a_int == 0)
			yyerror(_("non-zero integer value"));
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 153:

/* Line 1464 of yacc.c  */
#line 724 "command.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 154:

/* Line 1464 of yacc.c  */
#line 726 "command.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 155:

/* Line 1464 of yacc.c  */
#line 728 "command.y"
    {
		(yyvsp[(2) - (2)])->a_int = - (yyvsp[(2) - (2)])->a_int;
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 156:

/* Line 1464 of yacc.c  */
#line 736 "command.y"
    {
		if (lexptr_begin != NULL) {
			if (input_from_tty && lexptr_begin[0] != '\0')
				add_history(lexptr_begin);
			efree(lexptr_begin);
			lexptr_begin = NULL;
		}
	  }
    break;



/* Line 1464 of yacc.c  */
#line 2519 "command.c"
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
#line 746 "command.y"



/* append_statement --- append 'stmt' to the list of eval awk statements */ 

static CMDARG *
append_statement(CMDARG *alist, char *stmt) 
{
	CMDARG *a, *arg; 
	char *s;
	int len, slen, ssize;

#define EVALSIZE	512

	if (stmt == start_EVAL) {
		len = sizeof(start_EVAL);
		for (a = alist; a != NULL; a = a->next)
			len += strlen(a->a_string) + 1;	/* 1 for ',' */
		len += EVALSIZE;

		emalloc(s, char *, (len + 2) * sizeof(char), "append_statement");
		arg = mk_cmdarg(D_string);
		arg->a_string = s;
		arg->a_count = len;	/* kludge */

		slen = sizeof("function @eval(") - 1;
		memcpy(s, start_EVAL, slen);

		for (a = alist; a != NULL; a = a->next) {
			len = strlen(a->a_string);
			memcpy(s + slen, a->a_string, len);
			slen += len;
			if (a->next != NULL)
				s[slen++] = ',';
		}
		s[slen++] = ')';
		s[slen++] = '{';
		s[slen] = '\0';
		return arg;
	}
		 
	len = strlen(stmt) + 1;	/* 1 for newline */
	s = alist->a_string;
	slen = strlen(s);
	ssize = alist->a_count;
	if (len > ssize - slen) {
		ssize = slen + len + EVALSIZE;
		erealloc(s, char *, (ssize + 2) * sizeof(char), "append_statement");
		alist->a_string = s;
		alist->a_count = ssize;
	}
	memcpy(s + slen, stmt, len);
	slen += len;
	if (slen >= 2 && s[slen - 2] != '\n') {
		s[slen - 1] = '\n';
		s[slen] = '\0';
	}

	if (stmt == end_EVAL)
		erealloc(alist->a_string, char *, slen + 2, "append_statement");
	return alist;

#undef EVALSIZE
}


/* command names sorted in ascending order */

struct cmdtoken cmdtab[] = {
{ "backtrace", "bt", D_backtrace, D_BACKTRACE, do_backtrace,
	gettext_noop("backtrace [N] - print trace of all or N innermost (outermost if N < 0) frames.") },
{ "break", "b", D_break, D_BREAK, do_breakpoint,
	gettext_noop("break [[filename:]N|function] - set breakpoint at the specified location.") },
{ "clear", "", D_clear, D_CLEAR, do_clear,
	gettext_noop("clear [[filename:]N|function] - delete breakpoints previously set.") },
{ "commands", "", D_commands, D_COMMANDS, do_commands,
	gettext_noop("commands [num] - starts a list of commands to be executed at a breakpoint(watchpoint) hit.") },
{ "condition", "", D_condition, D_CONDITION, do_condition,
	gettext_noop("condition num [expr] - set or clear breakpoint or watchpoint condition.") },
{ "continue", "c", D_continue, D_CONTINUE, do_continue,
	gettext_noop("continue [COUNT] - continue program being debugged.") },
{ "delete", "d", D_delete, D_DELETE, do_delete_breakpoint,
	gettext_noop("delete [breakpoints] [range] - delete specified breakpoints.") },
{ "disable", "", D_disable, D_DISABLE, do_disable_breakpoint,
	gettext_noop("disable [breakpoints] [range] - disable specified breakpoints.") },
{ "display", "", D_display, D_DISPLAY, do_display,
	gettext_noop("display [var] - print value of variable each time the program stops.") },
{ "down", "", D_down, D_DOWN, do_down,
	gettext_noop("down [N] - move N frames down the stack.") },
{ "dump", "", D_dump, D_DUMP, do_dump_instructions,
	gettext_noop("dump [filename] - dump instructions to file or stdout.") },
{ "enable", "e", D_enable, D_ENABLE, do_enable_breakpoint,
	gettext_noop("enable [once|del] [breakpoints] [range] - enable specified breakpoints.") },
{ "end", "", D_end, D_END, do_commands,
	gettext_noop("end - end a list of commands or awk statements.") },
{ "eval", "", D_eval, D_EVAL, do_eval,
	gettext_noop("eval stmt|[p1, p2, ...] - evaluate awk statement(s).") },
{ "finish", "", D_finish, D_FINISH, do_finish,
	gettext_noop("finish - execute until selected stack frame returns.") },
{ "frame", "f", D_frame, D_FRAME, do_frame,
	gettext_noop("frame [N] - select and print stack frame number N.") },
{ "help", "h", D_help, D_HELP, do_help,
	gettext_noop("help [command] - print list of commands or explanation of command.") },
{ "ignore", "", D_ignore, D_IGNORE, do_ignore_breakpoint,
	gettext_noop("ignore N COUNT - set ignore-count of breakpoint number N to COUNT.") },
{ "info", "i", D_info, D_INFO, do_info,
	gettext_noop("info topic - source|sources|variables|functions|break|frame|args|locals|display|watch.") },
{ "list", "l", D_list, D_LIST, do_list,
	gettext_noop("list [-|+|[filename:]lineno|function|range] - list specified line(s).") },
{ "next", "n", D_next, D_NEXT, do_next,
	gettext_noop("next [COUNT] - step program, proceeding through subroutine calls.") },
{ "nexti", "ni", D_nexti, D_NEXTI, do_nexti,
	gettext_noop("nexti [COUNT] - step one instruction, but proceed through subroutine calls.") },
{ "option", "o", D_option, D_OPTION, do_option,
	gettext_noop("option [name[=value]] - set or display debugger option(s).") },
{ "print", "p", D_print, D_PRINT, do_print_var,
	gettext_noop("print var [var] - print value of a variable or array.") },
{ "printf", "", D_printf, D_PRINTF, do_print_f,
	gettext_noop("printf format, [arg], ... - formatted output.") },
{ "quit", "q", D_quit, D_QUIT, do_quit,
	gettext_noop("quit - exit debugger.") },
{ "return", "", D_return, D_RETURN, do_return,
	gettext_noop("return [value] - make selected stack frame return to its caller.") },
{ "run", "r", D_run, D_RUN, do_run,
	gettext_noop("run - start or restart executing program.") },
#ifdef HAVE_LIBREADLINE
{ "save", "", D_save, D_SAVE, do_save,
	gettext_noop("save filename - save commands from the session to file.") },
#endif
{ "set", "", D_set, D_SET, do_set_var,
	gettext_noop("set var = value - assign value to a scalar variable.") },
{ "silent", "", D_silent, D_SILENT, do_commands,
	gettext_noop("silent - suspends usual message when stopped at a breakpoint/watchpoint.") },
{ "source", "", D_source, D_SOURCE, do_source,
	gettext_noop("source file - execute commands from file.") },
{ "step", "s", D_step, D_STEP, do_step,
	gettext_noop("step [COUNT] - step program until it reaches a different source line.") },
{ "stepi", "si", D_stepi, D_STEPI, do_stepi,
	gettext_noop("stepi [COUNT] - step one instruction exactly.") },
{ "tbreak", "t", D_tbreak, D_TBREAK, do_tmp_breakpoint,
	gettext_noop("tbreak [[filename:]N|function] - set a temporary breakpoint.") },
{ "trace", "", D_trace, D_TRACE, do_trace_instruction,
	gettext_noop("trace on|off - print instruction before executing.") },
{ "undisplay",	"", D_undisplay, D_UNDISPLAY, do_undisplay,
	gettext_noop("undisplay [N] - remove variable(s) from automatic display list.") },
{ "until", "u", D_until, D_UNTIL, do_until,
	gettext_noop("until [[filename:]N|function] - execute until program reaches a different line or line N within current frame.") },
{ "unwatch", "", D_unwatch, D_UNWATCH, do_unwatch,
	gettext_noop("unwatch [N] - remove variable(s) from watch list.") },
{ "up",	"", D_up, D_UP, do_up,
	gettext_noop("up [N] - move N frames up the stack.") },
{ "watch", "w", D_watch, D_WATCH, do_watch,
	gettext_noop("watch var - set a watchpoint for a variable.") },
{ NULL, NULL, D_illegal, 0, (Func_cmd) 0,
	 NULL },
};

struct argtoken argtab[] = {
	{ "args", D_info, A_ARGS },
	{ "break", D_info, A_BREAK },
	{ "del", D_enable, A_DEL },
	{ "display", D_info, A_DISPLAY },
	{ "frame", D_info, A_FRAME },
	{ "functions", D_info, A_FUNCTIONS },
	{ "locals", D_info, A_LOCALS },
	{ "off", D_trace, A_TRACE_OFF },
	{ "on", D_trace, A_TRACE_ON },
	{ "once", D_enable, A_ONCE },
	{ "source", D_info, A_SOURCE },
	{ "sources", D_info, A_SOURCES },
	{ "variables", D_info, A_VARIABLES },
	{ "watch", D_info, A_WATCH },
	{ NULL, D_illegal, 0 },
};


/* get_command --- return command handler function */

Func_cmd
get_command(int ctype)
{
	int i;
	for (i = 0; cmdtab[i].name != NULL; i++) {
		if (cmdtab[i].type == ctype)
			return cmdtab[i].cf_ptr;
	}
	return (Func_cmd) 0;
}

/* get_command_name --- return command name given it's type */

const char *
get_command_name(int ctype)
{
	int i;
	for (i = 0; cmdtab[i].name != NULL; i++) {
		if (cmdtab[i].type == ctype)
			return cmdtab[i].name;
	}
	return NULL;
} 

/* mk_cmdarg --- make an argument for command */

static CMDARG *
mk_cmdarg(enum argtype type)
{
	CMDARG *arg;
	emalloc(arg, CMDARG *, sizeof(CMDARG), "mk_cmdarg");
	memset(arg, 0, sizeof(CMDARG));
	arg->type = type;
	return arg;
}

/* append_cmdarg --- append ARG to the list of arguments for the current command */
 
static void
append_cmdarg(CMDARG *arg)
{
	static CMDARG *savetail;

	if (arg_list == NULL)
		arg_list = arg;
	else
		savetail->next = arg;
	savetail = arg;
}

/* free_cmdarg --- free all arguments in LIST */

void
free_cmdarg(CMDARG *list)
{
	CMDARG *arg, *nexta;

	for (arg = list; arg != NULL; arg = nexta) {
		nexta = arg->next;

		switch (arg->type) {
		case D_variable:
		case D_subscript:
		case D_array:
		case D_string:
			if (arg->a_string != NULL)
				efree(arg->a_string);
			break;
		case D_node:
		case D_field:
			unref(arg->a_node);
			break;
		default:
			break;
		}
		efree(arg);
	}
}

/* yyerror --- print a syntax error message */

static void
yyerror(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	fprintf(out_fp, _("error: "));
	vfprintf(out_fp, mesg, args);
	fprintf(out_fp, "\n");
	va_end(args);
	errcount++;
	repeat_idx = -1;
}


/* yylex --- read a command and turn it into tokens */

static int
yylex(void)
{
	static char *lexptr = NULL;
	static char *lexend;
	int c;
	char *tokstart;
	size_t toklen; 

	yylval = (CMDARG *) NULL;

	if (errcount > 0 && lexptr_begin == NULL) {
		/* fake a new line */
		errcount = 0;
		return '\n';
	}

	if (lexptr_begin == NULL) {
again:
		lexptr_begin = read_a_line(dPrompt);
		if (lexptr_begin == NULL) {	/* EOF or error */
			if (get_eof_status() == EXIT_FATAL) 
				exit(EXIT_FATAL);
			if (get_eof_status() == EXIT_FAILURE) {
				static int seen_eof = 0;

				/* force a quit, and let do_quit (in debug.c) exit */
				if (! seen_eof) {
					if (errno != 0)	{
						fprintf(stderr, _("can't read command (%s)\n"), strerror(errno));
						exit_val = EXIT_FAILURE;
					} /* else
						exit_val = EXIT_SUCCESS; */

					seen_eof = 1;
					return '\n';	/* end current command if any */
				} else if (seen_eof++ == 1) {
					cmd_idx = find_command("quit", 4);
					return D_QUIT;	/* 'quit' token */
				} else
					return '\n';	/* end command 'quit' */
			}
			if (errno != 0)
				d_error(_("can't read command (%s)"), strerror(errno));
			if (pop_cmd_src() == 0)
				goto again;
			exit(EXIT_FATAL);	/* shouldn't happen */
		}

		if (! in_commands && ! in_eval	/* history expansion off in 'commands' and 'eval' */
				&& input_from_tty
		)
			history_expand_line(&lexptr_begin);
	
		lexptr = lexptr_begin;
		lexend = lexptr + strlen(lexptr);
		if (*lexptr == '\0'		/* blank line */
				&& repeat_idx >= 0
				&& input_from_tty
				&& ! in_eval
		) {
#ifdef HAVE_LIBREADLINE
			HIST_ENTRY *h;
			h = previous_history();
			if (h != NULL)
				add_history(h->line);
#endif
			cmd_idx = repeat_idx;
			return cmdtab[cmd_idx].class;	/* repeat last command */
		}
		repeat_idx = -1;
	}
	
	c = *lexptr;

	while (c == ' ' || c == '\t')
		c = *++lexptr;

	if (! input_from_tty && c == '#')
		return '\n'; 

	tokstart = lexptr;
	if (lexptr >= lexend)
		return '\n';

	if (cmd_idx < 0) {	/* need a command */
		if (c == '?' && tokstart[1] == '\0'	&& ! in_eval) {
			lexptr++;
			cmd_idx = find_command("help", 4);
			return D_HELP;
		}

		while (c != '\0' && c != ' ' && c != '\t') {
			if (! isalpha(c) && ! in_eval) {
				yyerror(_("invalid character in command"));
				return '\n';
			}
			c = *++lexptr;
		}

		toklen = lexptr - tokstart;

		if (in_eval) {
			if (toklen == 3
					&& tokstart[3] == '\0'
					&& tokstart[0] == 'e'
					&& tokstart[1] == 'n'
					&& tokstart[2] == 'd'
			) {
				cmd_idx = find_command(tokstart, toklen);
				return D_END;
			}
			lexptr = lexend;
			return D_STATEMENT;
		}

		cmd_idx = find_command(tokstart, toklen);
		if (cmd_idx >= 0) {
			if (in_commands && cmdtab[cmd_idx].type != D_eval) {
				/* add the actual command string (lexptr_begin) to
				 * arg_list; command string for 'eval' prepended to the arg_list
				 * in the grammer above (see eval_cmd non-terminal).
				 */
				CMDARG *arg;
				arg = mk_cmdarg(D_string);
				arg->a_string = estrdup(lexptr_begin, lexend - lexptr_begin);
				append_cmdarg(arg);
			}
			return cmdtab[cmd_idx].class;
		} else {
			yyerror(_("unknown command - \"%.*s\", try help"), toklen, tokstart);
			return '\n';
		}
	}

	c = *lexptr;
	
	if (cmdtab[cmd_idx].type == D_option) {
		if (c == '=')
			return *lexptr++;
	} else if (c == '-' || c == '+' || c == ':' || c == '|')
		return *lexptr++;

	if (c == '"') {
		char *str, *p;
		int flags = ALREADY_MALLOCED;
		int esc_seen = FALSE;

		toklen = lexend - lexptr;
		emalloc(str, char *, toklen + 2, "yylex");
		p = str;

		while ((c = *++lexptr) != '"') {
			if (lexptr == lexend) {
err:
				efree(str);
				yyerror(_("unterminated string"));
				return '\n';
			}
			if (c == '\\') {
				c = *++lexptr;
				esc_seen = TRUE;
				if (want_nodeval || c != '"')
					*p++ = '\\';
			}
			if (lexptr == lexend)
				goto err;
			*p++ = c;
		}
		lexptr++;
		*p = '\0';

		if (! want_nodeval) {
			yylval = mk_cmdarg(D_string);
			yylval->a_string = estrdup(str, p - str);
			append_cmdarg(yylval);
			return D_STRING;
		} else {	/* awk string */
			if (esc_seen)
				flags |= SCAN;
			yylval = mk_cmdarg(D_node);
			yylval->a_node = make_str_node(str, p - str, flags);
			append_cmdarg(yylval);
			return D_NODE;
		}
	}

	if (! want_nodeval) {
		while ((c = *++lexptr) != '\0' && c != ':' && c != '-'
					&& c != ' ' && c != '\t' && c != '=')
			;

		/* Is it an integer? */
		if (isdigit((unsigned char) tokstart[0]) && cmdtab[cmd_idx].type != D_option) {
			char *end;
			long l;

			errno = 0;
			l = strtol(tokstart, &end, 0);
			if (errno != 0) {
				yyerror(_("%s"), strerror(errno));
				errno = 0;
				return '\n';
			}

			if (lexptr == end) {
				yylval = mk_cmdarg(D_int);
				yylval->a_int = l;
				append_cmdarg(yylval);
				return D_INT;
			}
		}

		/* Must be string */
		yylval = mk_cmdarg(D_string);
		yylval->a_string = estrdup(tokstart, lexptr - tokstart);
		append_cmdarg(yylval);
		return D_STRING;
	}

	/* assert(want_nodval == TRUE); */

	/* look for awk number */

	if (isdigit((unsigned char) tokstart[0])) {
		double d;

		errno = 0;
		d = strtod(tokstart, &lexptr);
		if (errno != 0) {
			yyerror(strerror(errno));
			errno = 0;
			return '\n';
		}
		yylval = mk_cmdarg(D_node);
		yylval->a_node = make_number(d);
		append_cmdarg(yylval);
		return D_NODE;
	}

	c = *lexptr;
	if (c == '$' || c == '@'
			|| c == '[' || c == ']'
			|| c == ',' || c == '=')
		return *lexptr++;

	if (c != '_' && ! isalpha(c)) {
		yyerror(_("invalid character"));
		return '\n';
	}

	while (isalnum(c) || c == '_')
		c = *++lexptr;
	toklen = lexptr - tokstart;

	/* awk variable */
	yylval = mk_cmdarg(D_variable);
	yylval->a_string = estrdup(tokstart, toklen);
	append_cmdarg(yylval);
	return D_VARIABLE;
}

/* find_argument --- find index in 'argtab' for a command option */

static int
find_argument(CMDARG *arg)
{
	/* non-number argument */
	int idx;
	char *name, *p;
	size_t len;
	assert(cmd_idx >= 0);
	name = arg->a_string;
	len = strlen(name);
	for (idx = 0; (p = (char *) argtab[idx].name) != NULL; idx++) {
		if (cmdtab[cmd_idx].type == argtab[idx].cmd
				&& *p == *name
				&& strlen(p) == len
				&& strncmp(p, name, len) == 0
		)
			return idx;
	}
	return -1;	/* invalid option */
}

/* concat_args --- concatenate argument strings into a single string NODE */

static NODE *
concat_args(CMDARG *arg, int count)
{
	NODE *n;
	NODE **tmp;
	char *str, *subsep, *p;
	long len, subseplen;
	int i;

	if (count == 1) {
		n = force_string(arg->a_node);
		return dupnode(n);
	}
	
	emalloc(tmp, NODE **, count * sizeof(NODE *), "concat_args");
	subseplen = SUBSEP_node->var_value->stlen;
	subsep = SUBSEP_node->var_value->stptr;
	len = -subseplen;

	for (i = 0; i < count; i++) {
		n = force_string(arg->a_node);
		len += n->stlen + subseplen;
		tmp[i] = n;
		arg = arg->next;
	}

	emalloc(str, char *, len + 2, "concat_args");
	n = tmp[0];
	memcpy(str, n->stptr, n->stlen);
	p = str + n->stlen;
	for (i = 1; i < count; i++) {
		if (subseplen == 1)
			*p++ = *subsep;
		else if (subseplen > 0) {
			memcpy(p, subsep, subseplen);
			p += subseplen;
		}

		n = tmp[i];
		memcpy(p, n->stptr, n->stlen);
		p += n->stlen;
	}
	str[len] = '\0';
	efree(tmp);
	return make_str_node(str, len, ALREADY_MALLOCED);
}

/* find_command --- find the index in 'cmdtab' using exact,
 *                  abbreviation or unique partial match 
 */

static int
find_command(const char *token, size_t toklen)
{
	char *name, *abrv;
	int i, k;
	int try_exact = TRUE;
	int abrv_match = -1;
	int partial_match = -1;

#if 'a' == 0x81 /* it's EBCDIC */
	/* make sure all lower case characters in token (sorting
	 * isn't the solution in this case)
	 */
	for (i = 0; i < toklen; i++) {
		if (token[i] != tolower(token[i]))
			return -1;
	}
#endif

	k = sizeof(cmdtab)/sizeof(cmdtab[0]) - 1;
	for (i = 0; i < k; i++) {
		name = (char *) cmdtab[i].name;
		if (try_exact && *token == *name
				&& toklen == strlen(name)
				&& strncmp(name, token, toklen) == 0
		)
			return i;
		if (*name > *token)
			try_exact = FALSE;
		if (abrv_match < 0) {
			abrv = cmdtab[i].abbrvn;
			if (abrv[0] == token[0]) {
				if (toklen == 1 && ! abrv[1])
					abrv_match = i;
				else if (toklen == 2 && abrv[1] == token[1])
					abrv_match = i;
			}
		}
		if (! try_exact && abrv_match >= 0)
			return abrv_match;
		if (partial_match < 0) {
			if (*token == *name
					&& toklen < strlen(name)
					&& strncmp(name, token, toklen) == 0
			) {
				if ((i == k - 1 || strncmp(cmdtab[i + 1].name, token, toklen) != 0)
					&& (i == 0 || strncmp(cmdtab[i - 1].name, token, toklen) != 0)
				)
					partial_match = i;
			}
		}
	}
	return partial_match;
}

/* do_help -- help command */

int
do_help(CMDARG *arg, int cmd)
{
	int i;
	if (arg == NULL) {
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			for (i = 0; cmdtab[i].name != NULL; i++) {
				gprintf(out_fp, "%s:\n", cmdtab[i].name);
				gprintf(out_fp, "\t%s\n", _(cmdtab[i].help_txt));
			}
		}
	} else if (arg->type == D_string) {
		char *name;
		name = arg->a_string;
		i = find_command(name, strlen(name));
		if (i >= 0) {
			fprintf(out_fp, "%s\n", cmdtab[i].help_txt);
			if (STREQ(cmdtab[i].name, "option"))
				option_help();
		} else
			fprintf(out_fp, _("undefined command: %s\n"), name);
	}

	return FALSE;
}


/* next_word --- find the next word in a line to complete 
 *               (word seperation characters are space and tab).
 */
   
static char *
next_word(char *p, int len, char **endp)
{
	char *q;
	int i;

	if (p == NULL || len <= 0)
		return NULL;
	for (i = 0; i < len; i++, p++)
		if (*p != ' ' && *p != '\t')
			break;
	if (i == len)
		return NULL;
	if (endp != NULL) {
		for (i++, q = p + 1; i < len; i++, q++)
			if (*q == ' ' || *q == '\t')
				break;
		*endp = q;
	}
	return p;
}

#ifdef HAVE_LIBREADLINE

/* command_completion --- attempt to complete based on the word number in line;
 *    try to complete on command names if this is the first word; for the next
 *    word(s), the type of completion depends on the command name (first word).
 */

#ifndef RL_READLINE_VERSION		/* < 4.2a */
#define rl_completion_matches(x, y) completion_matches((char *) (x), (y))
#endif


char **
command_completion(const char *text, int start, int end)
{
	char *cmdtok, *e;
	int idx;
	int len;

	rl_attempted_completion_over = TRUE;	/* no default filename completion please */

	this_cmd = D_illegal;
	len = start;
	if ((cmdtok = next_word(rl_line_buffer, len, &e)) == NULL)	/* no first word yet */
		return  rl_completion_matches(text, command_generator);
	len -= (e - rl_line_buffer);

	idx = find_command(cmdtok, e - cmdtok);
	if (idx < 0)
		return NULL;
	this_cmd = cmdtab[idx].type;

	if (! next_word(e, len, NULL)) {
		switch (this_cmd) {
		case D_break:
		case D_list:
		case D_until:
		case D_tbreak:
		case D_clear:
			return rl_completion_matches(text, srcfile_generator);
		case D_info:
		case D_enable:
		case D_trace:
		case D_help:
			return rl_completion_matches(text, argument_generator);
		case D_option:
			return rl_completion_matches(text, option_generator);
		case D_print:
		case D_printf:
		case D_set:
		case D_display:
		case D_watch:
			return rl_completion_matches(text, variable_generator);
		default:
			return NULL;
		}
	}
	if (this_cmd == D_print || this_cmd == D_printf)
		return rl_completion_matches(text, variable_generator);
	return NULL;
}	

/* command_generator --- generator function for command completion */
 
static char *
command_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx = 0;
	char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}
	while ((name = (char *) cmdtab[idx].name) != NULL) {
		idx++;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}
	return NULL;
}

/* srcfile_generator --- generator function for source file completion */

static char *
srcfile_generator(const char *text, int state)
{
	static size_t textlen;
	static SRCFILE *s;
	char *name;
	extern SRCFILE *srcfiles;

	if (! state) {	/* first time */
		textlen = strlen(text);
		s = srcfiles->next;
	}
	while (s != srcfiles) {
		if (s->stype != SRC_FILE && s->stype != SRC_INC) {
			s = s->next;
			continue;
		}
		name = s->src;
		s = s->next;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}
	return NULL;
}

/* argument_generator --- generator function for non-number argument completion */

static char *
argument_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx;
	char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}

	if (this_cmd == D_help) {
		while ((name = (char *) cmdtab[idx++].name) != NULL) {
			if (strncmp(name, text, textlen) == 0)
				return estrdup(name, strlen(name));
		}
	} else {
		while ((name = (char *) argtab[idx].name) != NULL) {
			if (this_cmd != argtab[idx++].cmd)
				continue;
			if (strncmp(name, text, textlen) == 0)
				return estrdup(name, strlen(name));
		}
	}		
	return NULL;
}

/* variable_generator --- generator function for variable name completion */

static char *
variable_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx = 0;
	static char **pnames = NULL;
	static NODE **var_table = NULL;
	char *name;
	NODE *hp;

	if (! state) {	/* first time */
		textlen = strlen(text);
		if (var_table != NULL)
			efree(var_table);
		var_table = get_varlist();
		idx = 0;
		pnames = get_parmlist();  /* names of function params in
		                           * current context; the array
		                           * is NULL terminated in
		                           * awkgram.y (func_install).
		                           */
	}

	/* function params */
	while (pnames != NULL) {
		name = pnames[idx];
		if (name == NULL) {
			pnames = NULL;	/* don't try to match params again */
			idx = 0;
			break;
		}
		idx++;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}

	/* globals */
	while ((hp = var_table[idx]) != NULL) {
		idx++;
		if (hp->hvalue->type == Node_func)
			continue;
		if (strncmp(hp->hname, text, textlen) == 0)
			return estrdup(hp->hname, hp->hlength);
	}
	return NULL;
}

/* history_expand_line ---  history expand the LINE */

static void
history_expand_line(char **line)
{
	int ret;
	char *expansion;

	if (! *line || input_fd != 0 || ! input_from_tty)
		return;
	using_history();
	ret = history_expand(*line, &expansion);
	if (ret < 0 || ret == 2)
		efree(expansion);
	else {
		efree(*line);
		*line = expansion;
	}
}

#endif


