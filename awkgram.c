/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.5"

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

/* Line 268 of yacc.c  */
#line 26 "awkgram.y"

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


/* Line 268 of yacc.c  */
#line 225 "awkgram.c"

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
     FILENAME = 261,
     YNUMBER = 262,
     YSTRING = 263,
     RELOP = 264,
     IO_OUT = 265,
     IO_IN = 266,
     ASSIGNOP = 267,
     ASSIGN = 268,
     MATCHOP = 269,
     CONCAT_OP = 270,
     SUBSCRIPT = 271,
     LEX_BEGIN = 272,
     LEX_END = 273,
     LEX_IF = 274,
     LEX_ELSE = 275,
     LEX_RETURN = 276,
     LEX_DELETE = 277,
     LEX_SWITCH = 278,
     LEX_CASE = 279,
     LEX_DEFAULT = 280,
     LEX_WHILE = 281,
     LEX_DO = 282,
     LEX_FOR = 283,
     LEX_BREAK = 284,
     LEX_CONTINUE = 285,
     LEX_PRINT = 286,
     LEX_PRINTF = 287,
     LEX_NEXT = 288,
     LEX_EXIT = 289,
     LEX_FUNCTION = 290,
     LEX_BEGINFILE = 291,
     LEX_ENDFILE = 292,
     LEX_GETLINE = 293,
     LEX_NEXTFILE = 294,
     LEX_IN = 295,
     LEX_AND = 296,
     LEX_OR = 297,
     INCREMENT = 298,
     DECREMENT = 299,
     LEX_BUILTIN = 300,
     LEX_LENGTH = 301,
     LEX_EOF = 302,
     LEX_INCLUDE = 303,
     LEX_EVAL = 304,
     NEWLINE = 305,
     SLASH_BEFORE_EQUAL = 306,
     UNARY = 307
   };
#endif
/* Tokens.  */
#define FUNC_CALL 258
#define NAME 259
#define REGEXP 260
#define FILENAME 261
#define YNUMBER 262
#define YSTRING 263
#define RELOP 264
#define IO_OUT 265
#define IO_IN 266
#define ASSIGNOP 267
#define ASSIGN 268
#define MATCHOP 269
#define CONCAT_OP 270
#define SUBSCRIPT 271
#define LEX_BEGIN 272
#define LEX_END 273
#define LEX_IF 274
#define LEX_ELSE 275
#define LEX_RETURN 276
#define LEX_DELETE 277
#define LEX_SWITCH 278
#define LEX_CASE 279
#define LEX_DEFAULT 280
#define LEX_WHILE 281
#define LEX_DO 282
#define LEX_FOR 283
#define LEX_BREAK 284
#define LEX_CONTINUE 285
#define LEX_PRINT 286
#define LEX_PRINTF 287
#define LEX_NEXT 288
#define LEX_EXIT 289
#define LEX_FUNCTION 290
#define LEX_BEGINFILE 291
#define LEX_ENDFILE 292
#define LEX_GETLINE 293
#define LEX_NEXTFILE 294
#define LEX_IN 295
#define LEX_AND 296
#define LEX_OR 297
#define INCREMENT 298
#define DECREMENT 299
#define LEX_BUILTIN 300
#define LEX_LENGTH 301
#define LEX_EOF 302
#define LEX_INCLUDE 303
#define LEX_EVAL 304
#define NEWLINE 305
#define SLASH_BEFORE_EQUAL 306
#define UNARY 307




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 371 "awkgram.c"

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
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
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1157

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  74
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  185
/* YYNRULES -- Number of states.  */
#define YYNSTATES  330

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    62,     2,     2,    65,    61,     2,     2,
      66,    67,    59,    57,    54,    58,     2,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    53,    73,
      55,     2,    56,    52,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    69,     2,    70,    64,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    71,     2,    72,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    63
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    16,    19,    22,
      25,    30,    32,    35,    37,    38,    40,    45,    47,    49,
      51,    53,    59,    61,    63,    65,    68,    70,    72,    73,
      81,    82,    86,    88,    90,    91,    94,    97,    99,   102,
     105,   109,   111,   121,   128,   137,   146,   159,   171,   173,
     176,   179,   182,   185,   189,   190,   195,   198,   199,   204,
     205,   210,   215,   217,   218,   220,   221,   224,   227,   233,
     238,   240,   243,   246,   248,   250,   252,   254,   256,   260,
     261,   262,   266,   273,   283,   285,   288,   289,   291,   292,
     295,   296,   298,   300,   304,   306,   309,   313,   314,   316,
     317,   319,   321,   325,   327,   330,   334,   338,   342,   346,
     350,   354,   358,   362,   368,   370,   372,   374,   377,   379,
     381,   383,   385,   387,   389,   392,   394,   398,   402,   406,
     410,   414,   418,   422,   425,   428,   434,   439,   443,   447,
     451,   455,   459,   463,   465,   468,   472,   477,   482,   484,
     486,   488,   491,   494,   496,   498,   501,   504,   506,   509,
     514,   515,   517,   518,   521,   523,   526,   528,   532,   534,
     537,   540,   542,   545,   547,   551,   553,   555,   556,   559,
     562,   564,   565,   567,   569,   571
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      75,     0,    -1,    -1,    75,    76,    -1,    75,   104,    -1,
      75,    47,    -1,    75,     1,    -1,    78,    79,    -1,    78,
      88,    -1,    82,    79,    -1,    68,    48,    77,    88,    -1,
       6,    -1,     6,     1,    -1,     1,    -1,    -1,   112,    -1,
     112,    54,   105,   112,    -1,    17,    -1,    18,    -1,    36,
      -1,    37,    -1,   132,    87,   133,   135,   105,    -1,     4,
      -1,     3,    -1,    81,    -1,    68,    49,    -1,    45,    -1,
      46,    -1,    -1,    35,    83,    80,    66,   107,   134,   105,
      -1,    -1,    86,    85,     5,    -1,    60,    -1,    51,    -1,
      -1,    87,    89,    -1,    87,     1,    -1,   104,    -1,   136,
     105,    -1,   136,   105,    -1,   132,    87,   133,    -1,   103,
      -1,    23,    66,   112,   134,   105,   132,    96,   105,   133,
      -1,    26,    66,   112,   134,   105,    89,    -1,    27,   105,
      89,    26,    66,   112,   134,   105,    -1,    28,    66,     4,
      40,   129,   134,   105,    89,    -1,    28,    66,    95,   136,
     105,   112,   136,   105,    95,   134,   105,    89,    -1,    28,
      66,    95,   136,   105,   136,   105,    95,   134,   105,    89,
      -1,    90,    -1,    29,    88,    -1,    30,    88,    -1,    33,
      88,    -1,    39,    88,    -1,    34,   109,    88,    -1,    -1,
      21,    91,   109,    88,    -1,    92,    88,    -1,    -1,    99,
      93,   100,   101,    -1,    -1,    22,     4,    94,   123,    -1,
      22,    66,     4,    67,    -1,   112,    -1,    -1,    92,    -1,
      -1,    96,    97,    -1,    96,     1,    -1,    24,    98,   137,
     105,    87,    -1,    25,   137,   105,    87,    -1,     7,    -1,
      58,     7,    -1,    57,     7,    -1,     8,    -1,    84,    -1,
      31,    -1,    32,    -1,   110,    -1,    66,   111,   134,    -1,
      -1,    -1,    10,   102,   116,    -1,    19,    66,   112,   134,
     105,    89,    -1,    19,    66,   112,   134,   105,    89,    20,
     105,    89,    -1,    50,    -1,   104,    50,    -1,    -1,   104,
      -1,    -1,    55,   117,    -1,    -1,   108,    -1,     4,    -1,
     108,   138,     4,    -1,     1,    -1,   108,     1,    -1,   108,
     138,     1,    -1,    -1,   112,    -1,    -1,   111,    -1,   112,
      -1,   111,   138,   112,    -1,     1,    -1,   111,     1,    -1,
     111,     1,   112,    -1,   111,   138,     1,    -1,   130,   113,
     112,    -1,   112,    41,   112,    -1,   112,    42,   112,    -1,
     112,    14,   112,    -1,   112,    40,   129,    -1,   112,   115,
     112,    -1,   112,    52,   112,    53,   112,    -1,   116,    -1,
      13,    -1,    12,    -1,    51,    13,    -1,     9,    -1,    55,
      -1,   114,    -1,    56,    -1,   117,    -1,   118,    -1,   116,
     117,    -1,   119,    -1,   117,    64,   117,    -1,   117,    59,
     117,    -1,   117,    60,   117,    -1,   117,    61,   117,    -1,
     117,    57,   117,    -1,   117,    58,   117,    -1,    38,   122,
     106,    -1,   130,    43,    -1,   130,    44,    -1,    66,   111,
     134,    40,   129,    -1,   116,    11,    38,   122,    -1,   118,
      64,   117,    -1,   118,    59,   117,    -1,   118,    60,   117,
      -1,   118,    61,   117,    -1,   118,    57,   117,    -1,   118,
      58,   117,    -1,    84,    -1,    62,   117,    -1,    66,   112,
     134,    -1,    45,    66,   110,   134,    -1,    46,    66,   110,
     134,    -1,    46,    -1,   120,    -1,   130,    -1,    43,   130,
      -1,    44,   130,    -1,     7,    -1,     8,    -1,    58,   117,
      -1,    57,   117,    -1,   121,    -1,    68,   121,    -1,     3,
      66,   110,   134,    -1,    -1,   130,    -1,    -1,   124,    16,
      -1,   125,    -1,   124,   125,    -1,   126,    -1,    69,   111,
      70,    -1,   126,    -1,   127,   126,    -1,   127,    16,    -1,
       4,    -1,     4,   128,    -1,   129,    -1,    65,   119,   131,
      -1,    43,    -1,    44,    -1,    -1,    71,   105,    -1,    72,
     105,    -1,    67,    -1,    -1,   136,    -1,    73,    -1,    53,
      -1,    54,   105,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   218,   218,   220,   225,   226,   230,   242,   246,   257,
     265,   273,   281,   283,   289,   290,   292,   318,   329,   340,
     346,   355,   365,   367,   369,   380,   385,   386,   391,   390,
     420,   419,   452,   454,   459,   460,   473,   478,   479,   483,
     485,   487,   494,   584,   626,   668,   783,   790,   797,   807,
     816,   825,   834,   849,   865,   864,   876,   888,   888,   984,
     984,  1009,  1032,  1038,  1039,  1045,  1046,  1053,  1058,  1070,
    1084,  1086,  1092,  1097,  1099,  1107,  1109,  1118,  1119,  1127,
    1132,  1132,  1143,  1147,  1155,  1156,  1159,  1161,  1166,  1167,
    1174,  1176,  1180,  1186,  1193,  1195,  1197,  1204,  1205,  1211,
    1212,  1217,  1219,  1224,  1226,  1228,  1230,  1236,  1243,  1245,
    1247,  1263,  1273,  1280,  1282,  1287,  1289,  1291,  1299,  1301,
    1306,  1308,  1313,  1315,  1317,  1370,  1372,  1374,  1376,  1378,
    1380,  1382,  1384,  1407,  1412,  1417,  1442,  1448,  1450,  1452,
    1454,  1456,  1458,  1463,  1467,  1498,  1500,  1506,  1512,  1525,
    1526,  1527,  1532,  1537,  1541,  1545,  1557,  1570,  1575,  1611,
    1629,  1630,  1636,  1637,  1642,  1644,  1651,  1668,  1685,  1687,
    1694,  1699,  1707,  1721,  1734,  1743,  1747,  1751,  1755,  1759,
    1763,  1766,  1768,  1772,  1776,  1780
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FUNC_CALL", "NAME", "REGEXP",
  "FILENAME", "YNUMBER", "YSTRING", "RELOP", "IO_OUT", "IO_IN", "ASSIGNOP",
  "ASSIGN", "MATCHOP", "CONCAT_OP", "SUBSCRIPT", "LEX_BEGIN", "LEX_END",
  "LEX_IF", "LEX_ELSE", "LEX_RETURN", "LEX_DELETE", "LEX_SWITCH",
  "LEX_CASE", "LEX_DEFAULT", "LEX_WHILE", "LEX_DO", "LEX_FOR", "LEX_BREAK",
  "LEX_CONTINUE", "LEX_PRINT", "LEX_PRINTF", "LEX_NEXT", "LEX_EXIT",
  "LEX_FUNCTION", "LEX_BEGINFILE", "LEX_ENDFILE", "LEX_GETLINE",
  "LEX_NEXTFILE", "LEX_IN", "LEX_AND", "LEX_OR", "INCREMENT", "DECREMENT",
  "LEX_BUILTIN", "LEX_LENGTH", "LEX_EOF", "LEX_INCLUDE", "LEX_EVAL",
  "NEWLINE", "SLASH_BEFORE_EQUAL", "'?'", "':'", "','", "'<'", "'>'",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "UNARY", "'^'", "'$'", "'('",
  "')'", "'@'", "'['", "']'", "'{'", "'}'", "';'", "$accept", "program",
  "rule", "source", "pattern", "action", "func_name", "lex_builtin",
  "function_prologue", "$@1", "regexp", "$@2", "a_slash", "statements",
  "statement_term", "statement", "non_compound_stmt", "$@3", "simple_stmt",
  "$@4", "$@5", "opt_simple_stmt", "case_statements", "case_statement",
  "case_value", "print", "print_expression_list", "output_redir", "$@6",
  "if_statement", "nls", "opt_nls", "input_redir", "opt_param_list",
  "param_list", "opt_exp", "opt_expression_list", "expression_list", "exp",
  "assign_operator", "relop_or_less", "a_relop", "common_exp", "simp_exp",
  "simp_exp_nc", "non_post_simp_exp", "func_call", "direct_func_call",
  "opt_variable", "delete_subscript_list", "delete_subscript",
  "delete_exp_list", "bracketed_exp_list", "subscript", "subscript_list",
  "simple_variable", "variable", "opt_incdec", "l_brace", "r_brace",
  "r_paren", "opt_semi", "semi", "colon", "comma", 0
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,    63,    58,    44,    60,    62,    43,    45,    42,
      47,    37,    33,   307,    94,    36,    40,    41,    64,    91,
      93,   123,   125,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    74,    75,    75,    75,    75,    75,    76,    76,    76,
      76,    77,    77,    77,    78,    78,    78,    78,    78,    78,
      78,    79,    80,    80,    80,    80,    81,    81,    83,    82,
      85,    84,    86,    86,    87,    87,    87,    88,    88,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    90,
      90,    90,    90,    90,    91,    90,    90,    93,    92,    94,
      92,    92,    92,    95,    95,    96,    96,    96,    97,    97,
      98,    98,    98,    98,    98,    99,    99,   100,   100,   101,
     102,   101,   103,   103,   104,   104,   105,   105,   106,   106,
     107,   107,   108,   108,   108,   108,   108,   109,   109,   110,
     110,   111,   111,   111,   111,   111,   111,   112,   112,   112,
     112,   112,   112,   112,   112,   113,   113,   113,   114,   114,
     115,   115,   116,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   117,   117,   118,   118,   118,   118,
     118,   118,   118,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   120,   120,   121,
     122,   122,   123,   123,   124,   124,   125,   126,   127,   127,
     128,   129,   129,   130,   130,   131,   131,   131,   132,   133,
     134,   135,   135,   136,   137,   138
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     2,     2,     2,     2,
       4,     1,     2,     1,     0,     1,     4,     1,     1,     1,
       1,     5,     1,     1,     1,     2,     1,     1,     0,     7,
       0,     3,     1,     1,     0,     2,     2,     1,     2,     2,
       3,     1,     9,     6,     8,     8,    12,    11,     1,     2,
       2,     2,     2,     3,     0,     4,     2,     0,     4,     0,
       4,     4,     1,     0,     1,     0,     2,     2,     5,     4,
       1,     2,     2,     1,     1,     1,     1,     1,     3,     0,
       0,     3,     6,     9,     1,     2,     0,     1,     0,     2,
       0,     1,     1,     3,     1,     2,     3,     0,     1,     0,
       1,     1,     3,     1,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     5,     4,     3,     3,     3,
       3,     3,     3,     1,     2,     3,     4,     4,     1,     1,
       1,     2,     2,     1,     1,     2,     2,     1,     2,     4,
       0,     1,     0,     2,     1,     2,     1,     3,     1,     2,
       2,     1,     2,     1,     3,     1,     1,     0,     2,     2,
       1,     0,     1,     1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     6,     0,   171,   153,   154,    17,    18,
      28,    19,    20,   160,     0,     0,     0,   148,     5,    84,
      33,     0,     0,    32,     0,     0,     0,     0,     3,     0,
       0,   143,    30,     4,    15,   114,   122,   123,   125,   149,
     157,   173,   150,     0,     0,   168,     0,   172,     0,    88,
     161,   151,   152,     0,     0,     0,   156,   150,   155,   144,
       0,   177,   150,   103,     0,   101,     0,   158,    86,   183,
       7,     8,    37,    34,    86,     9,     0,    85,   118,     0,
       0,     0,     0,     0,    86,   119,   121,   120,     0,     0,
     124,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   116,   115,   133,   134,     0,     0,     0,
       0,   101,     0,   170,   169,    23,    22,    26,    27,     0,
       0,    24,     0,   132,     0,     0,     0,   175,   176,   174,
     104,    86,   180,     0,     0,   145,    13,     0,     0,    87,
     178,     0,    38,    31,   110,   111,   108,   109,     0,     0,
     112,   160,   130,   131,   127,   128,   129,   126,   141,   142,
     138,   139,   140,   137,   117,   107,   159,   167,    25,     0,
      89,   146,   147,   105,   185,     0,   106,   102,    12,    10,
      36,     0,    54,     0,     0,     0,    86,     0,     0,     0,
      75,    76,     0,    97,     0,    86,    35,    48,     0,    57,
      41,    62,    34,   181,    86,     0,    16,   136,    94,    92,
       0,     0,   135,     0,    97,    59,     0,     0,     0,     0,
      63,    49,    50,    51,     0,    98,    52,   179,    56,     0,
       0,    86,   182,    39,   113,    86,    95,     0,     0,     0,
     162,     0,     0,     0,     0,   171,    64,     0,    53,     0,
      79,    77,    40,    21,    29,    96,    93,    86,    55,    60,
       0,   164,   166,    61,    86,    86,     0,     0,    86,     0,
      80,    58,     0,   163,   165,     0,     0,     0,     0,     0,
      78,     0,    82,    65,    43,     0,    86,     0,    86,    81,
      86,     0,    86,     0,    86,    63,     0,    67,     0,     0,
      66,     0,    44,    45,    63,     0,    83,    70,    73,     0,
       0,    74,     0,   184,    86,    42,     0,    86,    72,    71,
      86,    34,    86,     0,    34,     0,     0,    47,     0,    46
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    28,   138,    29,    70,   120,   121,    30,    48,
      31,    76,    32,   141,    71,   196,   197,   214,   198,   229,
     240,   247,   291,   300,   312,   199,   250,   271,   281,   200,
     139,   140,   123,   210,   211,   224,   109,   110,   201,   108,
      87,    88,    35,    36,    37,    38,    39,    40,    49,   259,
     260,   261,    45,    46,    47,    41,    42,   129,   202,   203,
     135,   231,   204,   314,   134
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -269
static const yytype_int16 yypact[] =
{
    -269,   335,  -269,  -269,   -31,   -24,  -269,  -269,  -269,  -269,
    -269,  -269,  -269,    12,    12,    12,   -19,   -12,  -269,  -269,
    -269,   978,   978,  -269,   978,  1023,   804,    21,  -269,   115,
     -21,  -269,  -269,     8,  1062,   952,   -20,   330,  -269,  -269,
    -269,  -269,   246,   736,   804,  -269,     2,  -269,   205,    15,
    -269,  -269,  -269,   736,   736,    70,    52,    80,    52,    52,
     978,   147,  -269,  -269,    50,   308,   174,  -269,    64,  -269,
    -269,  -269,     8,  -269,    64,  -269,   129,  -269,  -269,   978,
     143,   978,   978,   978,    64,  -269,  -269,  -269,   978,   112,
     -20,   978,   978,   978,   978,   978,   978,   978,   978,   978,
     978,   978,   978,  -269,  -269,  -269,  -269,   141,   978,    90,
     152,  1101,    48,  -269,  -269,  -269,  -269,  -269,  -269,   111,
     105,  -269,   978,  -269,    90,    90,   308,  -269,  -269,  -269,
     978,    64,  -269,   134,   830,  -269,  -269,    13,   -16,     8,
    -269,   552,  -269,  -269,    53,  -269,   142,   300,  1081,   978,
     103,    12,   185,   185,    52,    52,    52,    52,   185,   185,
      52,    52,    52,    52,  -269,  1101,  -269,  -269,  -269,    63,
     -20,  -269,  -269,  1101,  -269,   143,  -269,  1101,  -269,  -269,
    -269,   121,  -269,     6,   130,   137,    64,   139,   -16,   -16,
    -269,  -269,   -16,   978,   -16,    64,  -269,  -269,   -16,  -269,
    -269,  1101,  -269,   127,    64,   978,  1101,  -269,  -269,  -269,
      90,   118,  -269,   978,   978,  -269,   180,   978,   978,   665,
     875,  -269,  -269,  -269,   -16,  1101,  -269,  -269,  -269,   598,
     552,    64,  -269,  -269,  1101,    64,  -269,    28,   308,   -16,
     -24,   140,   308,   308,   189,   -14,  -269,   127,  -269,   804,
     201,  -269,  -269,  -269,  -269,  -269,  -269,    64,  -269,  -269,
      14,  -269,  -269,  -269,    64,    64,   158,   143,    64,    50,
    -269,  -269,   665,  -269,  -269,   -21,   665,   978,    90,   710,
     134,   978,   198,  -269,  -269,   308,    64,  1056,    64,   952,
      64,    60,    64,   665,    64,   907,   665,  -269,   119,   177,
    -269,   155,  -269,  -269,   907,    90,  -269,  -269,  -269,   224,
     228,  -269,   177,  -269,    64,  -269,    90,    64,  -269,  -269,
      64,  -269,    64,   665,  -269,   406,   665,  -269,   479,  -269
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -269,  -269,  -269,  -269,  -269,   208,  -269,  -269,  -269,  -269,
     -58,  -269,  -269,  -193,    72,  -171,  -269,  -269,  -189,  -269,
    -269,  -268,  -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,
      45,    37,  -269,  -269,  -269,    38,   -48,   -23,    -1,  -269,
    -269,  -269,   -26,    44,  -269,   217,  -269,     1,   102,  -269,
    -269,    -3,   -39,  -269,  -269,   -72,    -2,  -269,   -28,  -213,
     -49,  -269,   -25,   -47,    66
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -101
static const yytype_int16 yytable[] =
{
      34,    73,    73,    64,    74,   124,   125,   114,   145,   230,
     215,    50,    51,    52,   178,   133,     5,   252,   113,    57,
      57,   112,    57,    62,     4,    65,   267,   305,    67,   255,
     273,   246,   256,    57,    19,    43,   316,    91,    92,    93,
      94,    95,   111,   111,    96,    44,    33,    53,   244,   130,
      68,   130,   111,   111,    54,    44,    67,    69,    77,   126,
     166,   297,    78,   -11,   208,    56,    58,   209,    59,    66,
     122,    44,   216,     4,    72,   171,   172,    25,   144,    90,
     146,   147,   148,    44,   298,   299,   -11,   150,   315,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,   282,   131,   212,   131,   284,   246,   165,    85,    86,
      19,   142,  -101,    74,    19,   246,    96,   132,   167,   236,
      57,   149,   303,   105,   106,   306,   307,   308,   325,   173,
     -90,   328,   -86,   177,   143,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,     5,   206,    50,
     151,    78,   327,   130,   164,   329,    79,   132,  -101,  -101,
     168,   235,  -100,    74,    74,    19,   170,    74,   174,    74,
      20,   169,   131,    74,   175,   136,   309,   310,   232,    23,
     137,   251,    80,    72,   241,   -91,    68,   213,    69,   257,
     127,   128,   225,   264,   265,   278,   217,    85,    86,    74,
      69,   262,  -100,   218,   234,   220,   131,   263,   115,   116,
     179,   270,   238,   225,    74,   266,   242,   243,   290,  -100,
     280,   262,   268,   219,   277,  -100,   269,   195,   111,   286,
     313,   318,   227,    72,    72,   319,   292,    72,    75,    72,
     311,   233,    61,    72,    93,    94,    95,   283,    65,    96,
     117,   118,   239,   207,   288,   289,   317,   274,   103,   104,
     221,   222,   294,     0,   223,   320,   226,   322,   253,    72,
     228,     0,   254,   119,     0,     0,   285,   237,   287,    57,
       0,     0,     0,     0,    72,     0,     0,    57,     0,   105,
     106,     0,     0,     0,   272,     0,   248,   107,     0,     0,
       0,   275,   276,     0,     0,   279,     0,     0,     0,    78,
       0,   258,     0,     0,    79,     0,     0,    78,     0,     0,
       0,     0,    79,   293,     0,   295,     0,   296,   301,   302,
       0,   304,     0,    90,     0,     2,     3,     0,     4,     5,
      80,    81,     6,     7,     0,     0,     0,     0,    80,    81,
      82,   321,     8,     9,   323,    85,    86,   324,     0,   326,
      83,     0,     0,    85,    86,     0,     0,     0,     0,     0,
      10,    11,    12,    13,     0,   132,     0,     0,    14,    15,
      16,    17,    18,     0,     0,    19,    20,    97,    98,    99,
     100,   101,    21,    22,   102,    23,     0,    24,     0,     0,
      25,    26,     0,    27,     0,     0,   -14,   180,   -14,     4,
       5,     0,     0,     6,     7,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   181,     0,   182,   183,   184,
     -69,   -69,   185,   186,   187,   188,   189,   190,   191,   192,
     193,     0,     0,     0,    13,   194,     0,     0,     0,    14,
      15,    16,    17,     0,     0,     0,   -69,    20,     0,     0,
       0,     0,     0,    21,    22,     0,    23,     0,    24,     0,
       0,    25,    26,     0,    55,     0,     0,    68,   -69,    69,
     180,     0,     4,     5,     0,     0,     6,     7,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   181,     0,
     182,   183,   184,   -68,   -68,   185,   186,   187,   188,   189,
     190,   191,   192,   193,     0,     0,     0,    13,   194,     0,
       0,     0,    14,    15,    16,    17,     0,     0,     0,   -68,
      20,     0,     0,     0,     0,     0,    21,    22,     0,    23,
       0,    24,     0,     0,    25,    26,     0,    55,     0,     0,
      68,   -68,    69,   180,     0,     4,     5,     0,     0,     6,
       7,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   181,     0,   182,   183,   184,     0,     0,   185,   186,
     187,   188,   189,   190,   191,   192,   193,     0,     0,     0,
      13,   194,     0,     0,     0,    14,    15,    16,    17,    63,
       0,     4,     5,    20,     0,     6,     7,     0,   -99,    21,
      22,     0,    23,     0,    24,     0,     0,    25,    26,     0,
      55,     0,     0,    68,   195,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,     0,     0,     0,
       0,    14,    15,    16,    17,     0,     0,     0,   -99,    20,
       0,     0,     0,     0,     0,    21,    22,     0,    23,     0,
      24,     0,     0,    25,   249,   -99,    55,     0,     4,     5,
       0,   -99,     6,     7,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   181,     0,   182,   183,   184,     0,
       0,   185,   186,   187,   188,   189,   190,   191,   192,   193,
       0,     0,     0,    13,   194,     0,     0,     0,    14,    15,
      16,    17,     0,     4,     5,     0,    20,     6,     7,     0,
       0,     0,    21,    22,     0,    23,     0,    24,     0,     0,
      25,    26,     0,    55,     0,     0,    68,    63,    69,     4,
       5,     0,     0,     6,     7,     0,     0,     0,    13,     0,
       0,     0,     0,    14,    15,    16,    17,     0,     0,     0,
       0,    20,     0,     0,     0,     0,     0,    21,    22,     0,
      23,     0,    24,     0,    13,    25,    26,     0,    55,    14,
      15,    16,    17,    69,     0,     0,     0,    20,     0,     0,
       0,     0,     0,    21,    22,     0,    23,     0,    24,     0,
       0,    25,    26,   -99,    55,    63,     0,     4,     5,     0,
       0,     6,     7,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   176,     0,     4,     5,     0,     0,     6,     7,     0,
       0,     0,    13,     0,     0,     0,     0,    14,    15,    16,
      17,     0,     0,     0,     0,    20,     0,     0,     0,     0,
       0,    21,    22,     0,    23,     0,    24,     0,    13,    25,
      26,     0,    55,    14,    15,    16,    17,     0,     4,   245,
       0,    20,     6,     7,     0,     0,     0,    21,    22,     0,
      23,     0,    24,     0,     0,    25,    26,   183,    55,     0,
       0,     0,     0,     0,     0,     0,   190,   191,     0,     0,
       4,     5,     0,    13,     6,     7,     0,     0,    14,    15,
      16,    17,     0,     0,     0,     0,    20,     0,     0,   183,
       0,     0,    21,    22,     0,    23,     0,    24,   190,   191,
      25,    26,     0,    55,     0,    13,     0,     0,     0,     0,
      14,    15,    16,    17,     0,     4,     5,     0,    20,     6,
       7,     0,     0,    89,    21,    22,     0,    23,     0,    24,
       0,     0,    25,    26,     0,    55,     0,     0,     0,     0,
       0,     4,     5,     0,     0,     6,     7,     0,     0,     0,
      13,     0,     0,     0,     0,    14,    15,    16,    17,     0,
       0,     0,     0,    20,     0,     0,     0,     0,     0,    21,
      22,     0,    23,     0,    24,     0,    13,    25,    26,     0,
      55,    14,    15,    16,    17,     0,     4,     5,     0,    20,
       6,     7,     0,     0,     0,    21,    22,     0,    23,     0,
      24,     0,     0,    25,    26,     0,    55,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    78,    14,    15,    16,    17,
      79,    78,     0,     0,    20,     0,    79,     0,     0,     0,
      21,    22,     0,    23,     0,    24,     0,     0,    25,    60,
      78,    55,     0,     0,     0,    79,    80,    81,    82,     0,
       0,     0,    80,    81,    82,     0,     0,     0,    83,     0,
      78,    85,    86,     0,    83,    79,    84,    85,    86,     0,
       0,    80,    81,    82,     0,     0,     0,     0,     0,    69,
       0,     0,     0,    83,   205,     0,    85,    86,     0,     0,
       0,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     0,     0,    85,    86
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-269))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-101))

static const yytype_int16 yycheck[] =
{
       1,    29,    30,    26,    29,    53,    54,    46,    80,   202,
       4,    13,    14,    15,     1,    64,     4,   230,    16,    21,
      22,    44,    24,    25,     3,    26,    40,   295,    27,     1,
      16,   220,     4,    35,    50,    66,   304,    57,    58,    59,
      60,    61,    43,    44,    64,    69,     1,    66,   219,     1,
      71,     1,    53,    54,    66,    69,    55,    73,    50,    60,
     109,     1,     9,    50,     1,    21,    22,     4,    24,    48,
      55,    69,    66,     3,    29,   124,   125,    65,    79,    35,
      81,    82,    83,    69,    24,    25,    73,    88,   301,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   272,    54,   175,    54,   276,   295,   108,    55,    56,
      50,    74,     9,   138,    50,   304,    64,    67,    70,     1,
     122,    84,   293,    43,    44,   296,     7,     8,   321,   130,
      67,   324,    72,   134,     5,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,     4,   149,   151,
      38,     9,   323,     1,    13,   326,    14,    67,    55,    56,
      49,   210,    10,   188,   189,    50,   122,   192,   131,   194,
      51,    66,    54,   198,    40,     1,    57,    58,   203,    60,
       6,   229,    40,   138,     4,    67,    71,    66,    73,   238,
      43,    44,   193,   242,   243,   267,    66,    55,    56,   224,
      73,   240,    50,    66,   205,    66,    54,    67,     3,     4,
     138,    10,   213,   214,   239,    26,   217,   218,    20,    67,
     269,   260,   247,   186,    66,    73,   249,    72,   229,   278,
      53,     7,   195,   188,   189,     7,   285,   192,    30,   194,
     298,   204,    25,   198,    59,    60,    61,   275,   249,    64,
      45,    46,   214,   151,   279,   281,   305,   260,    12,    13,
     188,   189,   287,    -1,   192,   312,   194,   316,   231,   224,
     198,    -1,   235,    68,    -1,    -1,   277,   211,   279,   281,
      -1,    -1,    -1,    -1,   239,    -1,    -1,   289,    -1,    43,
      44,    -1,    -1,    -1,   257,    -1,   224,    51,    -1,    -1,
      -1,   264,   265,    -1,    -1,   268,    -1,    -1,    -1,     9,
      -1,   239,    -1,    -1,    14,    -1,    -1,     9,    -1,    -1,
      -1,    -1,    14,   286,    -1,   288,    -1,   290,   291,   292,
      -1,   294,    -1,   289,    -1,     0,     1,    -1,     3,     4,
      40,    41,     7,     8,    -1,    -1,    -1,    -1,    40,    41,
      42,   314,    17,    18,   317,    55,    56,   320,    -1,   322,
      52,    -1,    -1,    55,    56,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    -1,    67,    -1,    -1,    43,    44,
      45,    46,    47,    -1,    -1,    50,    51,    57,    58,    59,
      60,    61,    57,    58,    64,    60,    -1,    62,    -1,    -1,
      65,    66,    -1,    68,    -1,    -1,    71,     1,    73,     3,
       4,    -1,    -1,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    -1,    -1,    -1,    38,    39,    -1,    -1,    -1,    43,
      44,    45,    46,    -1,    -1,    -1,    50,    51,    -1,    -1,
      -1,    -1,    -1,    57,    58,    -1,    60,    -1,    62,    -1,
      -1,    65,    66,    -1,    68,    -1,    -1,    71,    72,    73,
       1,    -1,     3,     4,    -1,    -1,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    -1,    -1,    -1,    38,    39,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,    50,
      51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,
      -1,    62,    -1,    -1,    65,    66,    -1,    68,    -1,    -1,
      71,    72,    73,     1,    -1,     3,     4,    -1,    -1,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    21,    22,    23,    -1,    -1,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    -1,    -1,
      38,    39,    -1,    -1,    -1,    43,    44,    45,    46,     1,
      -1,     3,     4,    51,    -1,     7,     8,    -1,    10,    57,
      58,    -1,    60,    -1,    62,    -1,    -1,    65,    66,    -1,
      68,    -1,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    43,    44,    45,    46,    -1,    -1,    -1,    50,    51,
      -1,    -1,    -1,    -1,    -1,    57,    58,    -1,    60,    -1,
      62,    -1,    -1,    65,    66,    67,    68,    -1,     3,     4,
      -1,    73,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    19,    -1,    21,    22,    23,    -1,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    -1,    -1,    38,    39,    -1,    -1,    -1,    43,    44,
      45,    46,    -1,     3,     4,    -1,    51,     7,     8,    -1,
      -1,    -1,    57,    58,    -1,    60,    -1,    62,    -1,    -1,
      65,    66,    -1,    68,    -1,    -1,    71,     1,    73,     3,
       4,    -1,    -1,     7,     8,    -1,    -1,    -1,    38,    -1,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,
      -1,    51,    -1,    -1,    -1,    -1,    -1,    57,    58,    -1,
      60,    -1,    62,    -1,    38,    65,    66,    -1,    68,    43,
      44,    45,    46,    73,    -1,    -1,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    57,    58,    -1,    60,    -1,    62,    -1,
      -1,    65,    66,    67,    68,     1,    -1,     3,     4,    -1,
      -1,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,     3,     4,    -1,    -1,     7,     8,    -1,
      -1,    -1,    38,    -1,    -1,    -1,    -1,    43,    44,    45,
      46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    57,    58,    -1,    60,    -1,    62,    -1,    38,    65,
      66,    -1,    68,    43,    44,    45,    46,    -1,     3,     4,
      -1,    51,     7,     8,    -1,    -1,    -1,    57,    58,    -1,
      60,    -1,    62,    -1,    -1,    65,    66,    22,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,
       3,     4,    -1,    38,     7,     8,    -1,    -1,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    22,
      -1,    -1,    57,    58,    -1,    60,    -1,    62,    31,    32,
      65,    66,    -1,    68,    -1,    38,    -1,    -1,    -1,    -1,
      43,    44,    45,    46,    -1,     3,     4,    -1,    51,     7,
       8,    -1,    -1,    11,    57,    58,    -1,    60,    -1,    62,
      -1,    -1,    65,    66,    -1,    68,    -1,    -1,    -1,    -1,
      -1,     3,     4,    -1,    -1,     7,     8,    -1,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    43,    44,    45,    46,    -1,
      -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    57,
      58,    -1,    60,    -1,    62,    -1,    38,    65,    66,    -1,
      68,    43,    44,    45,    46,    -1,     3,     4,    -1,    51,
       7,     8,    -1,    -1,    -1,    57,    58,    -1,    60,    -1,
      62,    -1,    -1,    65,    66,    -1,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    43,    44,    45,    46,
      14,     9,    -1,    -1,    51,    -1,    14,    -1,    -1,    -1,
      57,    58,    -1,    60,    -1,    62,    -1,    -1,    65,    66,
       9,    68,    -1,    -1,    -1,    14,    40,    41,    42,    -1,
      -1,    -1,    40,    41,    42,    -1,    -1,    -1,    52,    -1,
       9,    55,    56,    -1,    52,    14,    54,    55,    56,    -1,
      -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    52,    53,    -1,    55,    56,    -1,    -1,
      -1,    40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    55,    56
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    75,     0,     1,     3,     4,     7,     8,    17,    18,
      35,    36,    37,    38,    43,    44,    45,    46,    47,    50,
      51,    57,    58,    60,    62,    65,    66,    68,    76,    78,
      82,    84,    86,   104,   112,   116,   117,   118,   119,   120,
     121,   129,   130,    66,    69,   126,   127,   128,    83,   122,
     130,   130,   130,    66,    66,    68,   117,   130,   117,   117,
      66,   119,   130,     1,   111,   112,    48,   121,    71,    73,
      79,    88,   104,   132,   136,    79,    85,    50,     9,    14,
      40,    41,    42,    52,    54,    55,    56,   114,   115,    11,
     117,    57,    58,    59,    60,    61,    64,    57,    58,    59,
      60,    61,    64,    12,    13,    43,    44,    51,   113,   110,
     111,   112,   111,    16,   126,     3,     4,    45,    46,    68,
      80,    81,    55,   106,   110,   110,   112,    43,    44,   131,
       1,    54,    67,   134,   138,   134,     1,     6,    77,   104,
     105,    87,   105,     5,   112,   129,   112,   112,   112,   105,
     112,    38,   117,   117,   117,   117,   117,   117,   117,   117,
     117,   117,   117,   117,    13,   112,   134,    70,    49,    66,
     117,   134,   134,   112,   105,    40,     1,   112,     1,    88,
       1,    19,    21,    22,    23,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    39,    72,    89,    90,    92,    99,
     103,   112,   132,   133,   136,    53,   112,   122,     1,     4,
     107,   108,   129,    66,    91,     4,    66,    66,    66,   105,
      66,    88,    88,    88,   109,   112,    88,   105,    88,    93,
      87,   135,   136,   105,   112,   134,     1,   138,   112,   109,
      94,     4,   112,   112,    89,     4,    92,    95,    88,    66,
     100,   110,   133,   105,   105,     1,     4,   134,    88,   123,
     124,   125,   126,    67,   134,   134,    26,    40,   136,   111,
      10,   101,   105,    16,   125,   105,   105,    66,   129,   105,
     134,   102,    89,   132,    89,   112,   134,   112,   136,   116,
      20,    96,   134,   105,   136,   105,   105,     1,    24,    25,
      97,   105,   105,    89,   105,    95,    89,     7,     8,    57,
      58,    84,    98,    53,   137,   133,    95,   134,     7,     7,
     137,   105,   134,   105,   105,    87,   105,    89,    87,    89
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


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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


/*----------.
| yyparse.  |
`----------*/

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
  if (yypact_value_is_default (yyn))
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
      if (yytable_value_is_error (yyn))
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

/* Line 1806 of yacc.c  */
#line 221 "awkgram.y"
    {
		rule = 0;
		yyerrok;
	  }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 227 "awkgram.y"
    {
		next_sourcefile();
	  }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 231 "awkgram.y"
    {
		rule = 0;
		/*
		 * If errors, give up, don't produce an infinite
		 * stream of syntax error messages.
		 */
  		/* yyerrok; */
	  }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 243 "awkgram.y"
    {
		(void) append_rule((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
	  }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 247 "awkgram.y"
    {
		if (rule != Rule) {
			msg(_("%s blocks must have an action part"), ruletab[rule]);
			errcount++;
		} else if ((yyvsp[(1) - (2)]) == NULL) {
			msg(_("each rule must have a pattern or an action part"));
			errcount++;
		} else		/* pattern rule with non-empty pattern */
			(void) append_rule((yyvsp[(1) - (2)]), NULL);
	  }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 258 "awkgram.y"
    {
		can_return = FALSE;
		if ((yyvsp[(1) - (2)]) && func_install((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])) < 0)
			YYABORT;
		func_params = NULL;
		yyerrok;
	  }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 266 "awkgram.y"
    {
		want_source = FALSE;
		yyerrok;
	  }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 274 "awkgram.y"
    {
		if (include_source((yyvsp[(1) - (1)])) < 0)
			YYABORT;
		efree((yyvsp[(1) - (1)])->lextok);
		bcfree((yyvsp[(1) - (1)]));
		(yyval) = NULL;
	  }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 282 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 284 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 289 "awkgram.y"
    {	(yyval) = NULL; rule = Rule; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 291 "awkgram.y"
    {	(yyval) = (yyvsp[(1) - (1)]); rule = Rule; }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 293 "awkgram.y"
    {
		INSTRUCTION *tp;

		add_lint((yyvsp[(1) - (4)]), LINT_assign_in_cond);
		add_lint((yyvsp[(4) - (4)]), LINT_assign_in_cond);

		tp = instruction(Op_no_op);
		list_prepend((yyvsp[(1) - (4)]), bcalloc(Op_line_range, !!do_profiling + 1, 0));
		(yyvsp[(1) - (4)])->nexti->triggered = FALSE;
		(yyvsp[(1) - (4)])->nexti->target_jmp = (yyvsp[(4) - (4)])->nexti;

		list_append((yyvsp[(1) - (4)]), instruction(Op_cond_pair));
		(yyvsp[(1) - (4)])->lasti->line_range = (yyvsp[(1) - (4)])->nexti;
		(yyvsp[(1) - (4)])->lasti->target_jmp = tp;

		list_append((yyvsp[(4) - (4)]), instruction(Op_cond_pair));
		(yyvsp[(4) - (4)])->lasti->line_range = (yyvsp[(1) - (4)])->nexti;
		(yyvsp[(4) - (4)])->lasti->target_jmp = tp;
		if (do_profiling) {
			((yyvsp[(1) - (4)])->nexti + 1)->condpair_left = (yyvsp[(1) - (4)])->lasti;
			((yyvsp[(1) - (4)])->nexti + 1)->condpair_right = (yyvsp[(4) - (4)])->lasti;
		}
		(yyval) = list_append(list_merge((yyvsp[(1) - (4)]), (yyvsp[(4) - (4)])), tp);
		rule = Rule;
	  }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 319 "awkgram.y"
    {
		static int begin_seen = 0;
		if (do_lint_old && ++begin_seen == 2)
			warning_ln((yyvsp[(1) - (1)])->source_line,
				_("old awk does not support multiple `BEGIN' or `END' rules"));

		(yyvsp[(1) - (1)])->in_rule = rule = BEGIN;
		(yyvsp[(1) - (1)])->source_file = source;
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 330 "awkgram.y"
    {
		static int end_seen = 0;
		if (do_lint_old && ++end_seen == 2)
			warning_ln((yyvsp[(1) - (1)])->source_line,
				_("old awk does not support multiple `BEGIN' or `END' rules"));

		(yyvsp[(1) - (1)])->in_rule = rule = END;
		(yyvsp[(1) - (1)])->source_file = source;
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 341 "awkgram.y"
    {
		(yyvsp[(1) - (1)])->in_rule = rule = BEGINFILE;
		(yyvsp[(1) - (1)])->source_file = source;
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 347 "awkgram.y"
    {
		(yyvsp[(1) - (1)])->in_rule = rule = ENDFILE;
		(yyvsp[(1) - (1)])->source_file = source;
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 356 "awkgram.y"
    {
		if ((yyvsp[(2) - (5)]) == NULL)
			(yyval) = list_create(instruction(Op_no_op));
		else
			(yyval) = (yyvsp[(2) - (5)]);
	  }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 366 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 368 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 370 "awkgram.y"
    {
		yyerror(_("`%s' is a built-in function, it cannot be redefined"),
			tokstart);
		(yyvsp[(1) - (1)])->opcode = Op_symbol;	/* Op_symbol instead of Op_token so that
		                         * free_bc_internal does not try to free it
		                         */
		(yyvsp[(1) - (1)])->lextok = builtin_func;
		(yyval) = (yyvsp[(1) - (1)]);
		/* yyerrok; */
	  }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 381 "awkgram.y"
    { (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 391 "awkgram.y"
    {
		param_counter = 0;
		func_params = NULL;
	  }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 396 "awkgram.y"
    {
			NODE *t;

			(yyvsp[(1) - (7)])->source_file = source;
			t = make_param((yyvsp[(3) - (7)])->lextok);
			(yyvsp[(3) - (7)])->lextok = NULL;
			bcfree((yyvsp[(3) - (7)]));
			t->flags |= FUNC;
			t->rnode = func_params;
			func_params = t;
			(yyval) = (yyvsp[(1) - (7)]);
			can_return = TRUE;
			/* check for duplicate parameter names */
			if (dup_parms((yyvsp[(1) - (7)]), t))
				errcount++;
		}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 420 "awkgram.y"
    { ++want_regexp; }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 422 "awkgram.y"
    {
		  NODE *n, *exp;
		  char *re;
		  size_t len;

		  re = (yyvsp[(3) - (3)])->lextok;
		  len = strlen(re);
		  if (do_lint) {
			if (len == 0)
				lintwarn_ln((yyvsp[(3) - (3)])->source_line,
					_("regexp constant `//' looks like a C++ comment, but is not"));
			else if ((re)[0] == '*' && (re)[len-1] == '*')
				/* possible C comment */
				lintwarn_ln((yyvsp[(3) - (3)])->source_line,
					_("regexp constant `/%s/' looks like a C comment, but is not"), re);
		  }

		  exp = make_str_node(re, len, ALREADY_MALLOCED);
		  n = make_regnode(Node_regex, exp);
		  if (n == NULL) {
			unref(exp);
			YYABORT;
		  }
		  (yyval) = (yyvsp[(3) - (3)]);
		  (yyval)->opcode = Op_match_rec;
		  (yyval)->memory = n;
		}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 453 "awkgram.y"
    { bcfree((yyvsp[(1) - (1)])); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 459 "awkgram.y"
    {	(yyval) = NULL; }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 461 "awkgram.y"
    {
		if ((yyvsp[(2) - (2)]) == NULL)
			(yyval) = (yyvsp[(1) - (2)]);
		else {
			add_lint((yyvsp[(2) - (2)]), LINT_no_effect);
			if ((yyvsp[(1) - (2)]) == NULL)
				(yyval) = (yyvsp[(2) - (2)]);
			else
				(yyval) = list_merge((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
		}
	    yyerrok;
	  }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 474 "awkgram.y"
    {	(yyval) = NULL; }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 484 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 486 "awkgram.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 488 "awkgram.y"
    {
		if (do_profiling)
			(yyval) = list_prepend((yyvsp[(1) - (1)]), instruction(Op_exec_count));
		else
			(yyval) = (yyvsp[(1) - (1)]);
 	  }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 495 "awkgram.y"
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

		if ((yyvsp[(7) - (9)]) != NULL) {
			curr = (yyvsp[(7) - (9)])->nexti;
			bcfree((yyvsp[(7) - (9)]));	/* Op_list */
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

		ip = (yyvsp[(3) - (9)]);
		if (do_profiling) {
			(void) list_prepend(ip, (yyvsp[(1) - (9)]));
			(void) list_prepend(ip, instruction(Op_exec_count));
			(yyvsp[(1) - (9)])->target_break = tbreak;
			((yyvsp[(1) - (9)]) + 1)->switch_start = cexp->nexti;
			((yyvsp[(1) - (9)]) + 1)->switch_end = cexp->lasti;
		}/* else
				$1 is NULL */

		(void) list_append(cexp, dflt);
		(void) list_merge(ip, cexp);
		(yyval) = list_merge(ip, cstmt);

		break_allowed--;			
		fix_break_continue(ip, tbreak, NULL);
	  }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 585 "awkgram.y"
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
		add_lint((yyvsp[(3) - (6)]), LINT_assign_in_cond);
		tcont = (yyvsp[(3) - (6)])->nexti;
		ip = list_append((yyvsp[(3) - (6)]), instruction(Op_jmp_false));
		ip->lasti->target_jmp = tbreak;

		if (do_profiling) {
			(void) list_append(ip, instruction(Op_exec_count));
			(yyvsp[(1) - (6)])->target_break = tbreak;
			(yyvsp[(1) - (6)])->target_continue = tcont;
			((yyvsp[(1) - (6)]) + 1)->while_body = ip->lasti;
			(void) list_prepend(ip, (yyvsp[(1) - (6)]));
		}/* else
				$1 is NULL */

		if ((yyvsp[(6) - (6)]) != NULL)
			(void) list_merge(ip, (yyvsp[(6) - (6)]));
		(void) list_append(ip, instruction(Op_jmp));
		ip->lasti->target_jmp = tcont;
		(yyval) = list_append(ip, tbreak);

		break_allowed--;
		continue_allowed--;
		fix_break_continue(ip, tbreak, tcont);
	  }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 627 "awkgram.y"
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
		tcont = (yyvsp[(6) - (8)])->nexti;
		add_lint((yyvsp[(6) - (8)]), LINT_assign_in_cond);
		if ((yyvsp[(3) - (8)]) != NULL)
			ip = list_merge((yyvsp[(3) - (8)]), (yyvsp[(6) - (8)]));
		else
			ip = list_prepend((yyvsp[(6) - (8)]), instruction(Op_no_op));
		if (do_profiling)
			(void) list_prepend(ip, instruction(Op_exec_count));
		(void) list_append(ip, instruction(Op_jmp_true));
		ip->lasti->target_jmp = ip->nexti;
		(yyval) = list_append(ip, tbreak);

		break_allowed--;
		continue_allowed--;
		fix_break_continue(ip, tbreak, tcont);

		if (do_profiling) {
			(yyvsp[(1) - (8)])->target_break = tbreak;
			(yyvsp[(1) - (8)])->target_continue = tcont;
			((yyvsp[(1) - (8)]) + 1)->doloop_cond = tcont;
			(yyval) = list_prepend(ip, (yyvsp[(1) - (8)]));
			bcfree((yyvsp[(4) - (8)]));
		} /* else
				$1 and $4 are NULLs */
	  }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 669 "awkgram.y"
    {
		INSTRUCTION *ip;
		char *var_name = (yyvsp[(3) - (8)])->lextok;

		if ((yyvsp[(8) - (8)]) != NULL
				&& (yyvsp[(8) - (8)])->lasti->opcode == Op_K_delete
				&& (yyvsp[(8) - (8)])->lasti->expr_count == 1
				&& (yyvsp[(8) - (8)])->nexti->opcode == Op_push
				&& ((yyvsp[(8) - (8)])->nexti->memory->type != Node_var || !((yyvsp[(8) - (8)])->nexti->memory->var_update))
				&& strcmp((yyvsp[(8) - (8)])->nexti->memory->vname, var_name) == 0
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

			ip = (yyvsp[(8) - (8)])->nexti->nexti; 
			if ((yyvsp[(5) - (8)])->nexti->opcode == Op_push && (yyvsp[(5) - (8)])->lasti == (yyvsp[(5) - (8)])->nexti)
				arr = (yyvsp[(5) - (8)])->nexti->memory;
			if (arr != NULL
					&& ip->opcode == Op_no_op
					&& ip->nexti->opcode == Op_push_array
					&& strcmp(ip->nexti->memory->vname, arr->vname) == 0
					&& ip->nexti->nexti == (yyvsp[(8) - (8)])->lasti
			) {
				(void) make_assignable((yyvsp[(8) - (8)])->nexti);
				(yyvsp[(8) - (8)])->lasti->opcode = Op_K_delete_loop;
				(yyvsp[(8) - (8)])->lasti->expr_count = 0;
				if ((yyvsp[(1) - (8)]) != NULL)
					bcfree((yyvsp[(1) - (8)]));
				efree(var_name);
				bcfree((yyvsp[(3) - (8)]));
				bcfree((yyvsp[(4) - (8)]));
				bcfree((yyvsp[(5) - (8)]));
				(yyval) = (yyvsp[(8) - (8)]);
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
			ip = (yyvsp[(5) - (8)]);
			ip->nexti->opcode = Op_push_array;

			tbreak = instruction(Op_arrayfor_final);
			(yyvsp[(4) - (8)])->opcode = Op_arrayfor_incr;
			(yyvsp[(4) - (8)])->array_var = variable(var_name, Node_var);
			(yyvsp[(4) - (8)])->target_jmp = tbreak;
			tcont = (yyvsp[(4) - (8)]);
			(yyvsp[(3) - (8)])->opcode = Op_arrayfor_init;
			(yyvsp[(3) - (8)])->target_jmp = tbreak;
			(void) list_append(ip, (yyvsp[(3) - (8)]));

			if (do_profiling) {
				(yyvsp[(1) - (8)])->opcode = Op_K_arrayfor;
				(yyvsp[(1) - (8)])->target_continue = tcont;
				(yyvsp[(1) - (8)])->target_break = tbreak;
				(void) list_append(ip, (yyvsp[(1) - (8)]));
			} /* else
					$1 is NULL */

			/* add update_FOO instruction if necessary */ 
			if ((yyvsp[(4) - (8)])->array_var->type == Node_var && (yyvsp[(4) - (8)])->array_var->var_update) {
				(void) list_append(ip, instruction(Op_var_update));
				ip->lasti->memory = (yyvsp[(4) - (8)])->array_var;
				ip->lasti->update_var = (yyvsp[(4) - (8)])->array_var->var_update;
			}
			(void) list_append(ip, (yyvsp[(4) - (8)]));

			/* add set_FOO instruction if necessary */
			if ((yyvsp[(4) - (8)])->array_var->type == Node_var && (yyvsp[(4) - (8)])->array_var->var_assign) {
				(void) list_append(ip, instruction(Op_var_assign));
				ip->lasti->memory = (yyvsp[(4) - (8)])->array_var;
				ip->lasti->assign_var = (yyvsp[(4) - (8)])->array_var->var_assign;
			}

			if (do_profiling) {
				(void) list_append(ip, instruction(Op_exec_count));
				((yyvsp[(1) - (8)]) + 1)->forloop_cond = (yyvsp[(4) - (8)]);
				((yyvsp[(1) - (8)]) + 1)->forloop_body = ip->lasti; 
			}

			if ((yyvsp[(8) - (8)]) != NULL)
				(void) list_merge(ip, (yyvsp[(8) - (8)]));

			(void) list_append(ip, instruction(Op_jmp));
			ip->lasti->target_jmp = (yyvsp[(4) - (8)]);
			(yyval) = list_append(ip, tbreak);
			fix_break_continue(ip, tbreak, tcont);
		} 

		break_allowed--;
		continue_allowed--;
	  }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 784 "awkgram.y"
    {
		(yyval) = mk_for_loop((yyvsp[(1) - (12)]), (yyvsp[(3) - (12)]), (yyvsp[(6) - (12)]), (yyvsp[(9) - (12)]), (yyvsp[(12) - (12)]));

		break_allowed--;
		continue_allowed--;
	  }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 791 "awkgram.y"
    {
		(yyval) = mk_for_loop((yyvsp[(1) - (11)]), (yyvsp[(3) - (11)]), (INSTRUCTION *) NULL, (yyvsp[(8) - (11)]), (yyvsp[(11) - (11)]));

		break_allowed--;
		continue_allowed--;
	  }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 798 "awkgram.y"
    {
		if (do_profiling)
			(yyval) = list_prepend((yyvsp[(1) - (1)]), instruction(Op_exec_count));
		else
			(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 808 "awkgram.y"
    { 
		if (! break_allowed)
			error_ln((yyvsp[(1) - (2)])->source_line,
				_("`break' is not allowed outside a loop or switch"));
		(yyvsp[(1) - (2)])->target_jmp = NULL;
		(yyval) = list_create((yyvsp[(1) - (2)]));

	  }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 817 "awkgram.y"
    {
		if (! continue_allowed)
			error_ln((yyvsp[(1) - (2)])->source_line,
				_("`continue' is not allowed outside a loop"));
		(yyvsp[(1) - (2)])->target_jmp = NULL;
		(yyval) = list_create((yyvsp[(1) - (2)]));

	  }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 826 "awkgram.y"
    {
		/* if inside function (rule = 0), resolve context at run-time */
		if (rule && rule != Rule)
			error_ln((yyvsp[(1) - (2)])->source_line,
				_("`next' used in %s action"), ruletab[rule]);
		(yyvsp[(1) - (2)])->target_jmp = ip_rec;
		(yyval) = list_create((yyvsp[(1) - (2)]));
	  }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 835 "awkgram.y"
    {
		if (do_traditional)
			error_ln((yyvsp[(1) - (2)])->source_line,
				_("`nextfile' is a gawk extension"));

		/* if inside function (rule = 0), resolve context at run-time */
		if (rule == BEGIN || rule == END || rule == ENDFILE)
			error_ln((yyvsp[(1) - (2)])->source_line,
				_("`nextfile' used in %s action"), ruletab[rule]);

		(yyvsp[(1) - (2)])->target_jmp = ip_newfile;
		(yyvsp[(1) - (2)])->target_endfile = ip_endfile;
		(yyval) = list_create((yyvsp[(1) - (2)]));
	  }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 850 "awkgram.y"
    {
		/* Initialize the two possible jump targets, the actual target
		 * is resolved at run-time. 
		 */
		(yyvsp[(1) - (3)])->target_end = ip_end;	/* first instruction in end_block */
		(yyvsp[(1) - (3)])->target_atexit = ip_atexit;	/* cleanup and go home */

		if ((yyvsp[(2) - (3)]) == NULL) {
			(yyval) = list_create((yyvsp[(1) - (3)]));
			(void) list_prepend((yyval), instruction(Op_push_i));
			(yyval)->nexti->memory = Nnull_string;
		} else
			(yyval) = list_append((yyvsp[(2) - (3)]), (yyvsp[(1) - (3)]));
	  }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 865 "awkgram.y"
    {
		if (! can_return)
			yyerror(_("`return' used outside function context"));
	  }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 868 "awkgram.y"
    {
		if ((yyvsp[(3) - (4)]) == NULL) {
			(yyval) = list_create((yyvsp[(1) - (4)]));
			(void) list_prepend((yyval), instruction(Op_push_i));
			(yyval)->nexti->memory = Nnull_string;
		} else
			(yyval) = list_append((yyvsp[(3) - (4)]), (yyvsp[(1) - (4)]));
	  }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 888 "awkgram.y"
    { in_print = TRUE; in_parens = 0; }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 889 "awkgram.y"
    {
		/*
		 * Optimization: plain `print' has no expression list, so $3 is null.
		 * If $3 is NULL or is a bytecode list for $0 use Op_K_print_rec,
		 * which is faster for these two cases.
		 */

		if ((yyvsp[(1) - (4)])->opcode == Op_K_print &&
				((yyvsp[(3) - (4)]) == NULL
					|| ((yyvsp[(3) - (4)])->lasti->opcode == Op_field_spec
						&& (yyvsp[(3) - (4)])->nexti->nexti->nexti == (yyvsp[(3) - (4)])->lasti
						&& (yyvsp[(3) - (4)])->nexti->nexti->opcode == Op_push_i
						&& (yyvsp[(3) - (4)])->nexti->nexti->memory->type == Node_val
						&& (yyvsp[(3) - (4)])->nexti->nexti->memory->numbr == 0.0)
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

			if ((yyvsp[(3) - (4)]) != NULL) {
				bcfree((yyvsp[(3) - (4)])->lasti);				/* Op_field_spec */
				(yyvsp[(3) - (4)])->nexti->nexti->memory->flags &= ~PERM;
				(yyvsp[(3) - (4)])->nexti->nexti->memory->flags |= MALLOC;			
				unref((yyvsp[(3) - (4)])->nexti->nexti->memory);	/* Node_val */
				bcfree((yyvsp[(3) - (4)])->nexti->nexti);		/* Op_push_i */
				bcfree((yyvsp[(3) - (4)])->nexti);				/* Op_list */
				bcfree((yyvsp[(3) - (4)]));						/* Op_list */
			} else {
				if (do_lint && (rule == BEGIN || rule == END) && ! warned) {
					warned = TRUE;
					lintwarn_ln((yyvsp[(1) - (4)])->source_line,
		_("plain `print' in BEGIN or END rule should probably be `print \"\"'"));
				}
			}

			(yyvsp[(1) - (4)])->expr_count = 0;
			(yyvsp[(1) - (4)])->opcode = Op_K_print_rec;
			if ((yyvsp[(4) - (4)]) == NULL) {    /* no redircetion */
				(yyvsp[(1) - (4)])->redir_type = 0;
				(yyval) = list_create((yyvsp[(1) - (4)]));
			} else {
				INSTRUCTION *ip;
				ip = (yyvsp[(4) - (4)])->nexti;
				(yyvsp[(1) - (4)])->redir_type = ip->redir_type;
				(yyvsp[(4) - (4)])->nexti = ip->nexti;
				bcfree(ip);
				(yyval) = list_append((yyvsp[(4) - (4)]), (yyvsp[(1) - (4)]));
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
			 
			if ((yyvsp[(4) - (4)]) == NULL) {		/* no redirection */
				if ((yyvsp[(3) - (4)]) == NULL)	{	/* printf without arg */
					(yyvsp[(1) - (4)])->expr_count = 0;
					(yyvsp[(1) - (4)])->redir_type = 0;
					(yyval) = list_create((yyvsp[(1) - (4)]));
				} else {
					INSTRUCTION *t = (yyvsp[(3) - (4)]);
					(yyvsp[(1) - (4)])->expr_count = count_expressions(&t, FALSE);
					(yyvsp[(1) - (4)])->redir_type = 0;
					(yyval) = list_append(t, (yyvsp[(1) - (4)]));
				}
			} else {
				INSTRUCTION *ip;
				ip = (yyvsp[(4) - (4)])->nexti;
				(yyvsp[(1) - (4)])->redir_type = ip->redir_type;
				(yyvsp[(4) - (4)])->nexti = ip->nexti;
				bcfree(ip);
				if ((yyvsp[(3) - (4)]) == NULL) {
					(yyvsp[(1) - (4)])->expr_count = 0;
					(yyval) = list_append((yyvsp[(4) - (4)]), (yyvsp[(1) - (4)]));
				} else {
					INSTRUCTION *t = (yyvsp[(3) - (4)]);
					(yyvsp[(1) - (4)])->expr_count = count_expressions(&t, FALSE);
					(yyval) = list_append(list_merge((yyvsp[(4) - (4)]), t), (yyvsp[(1) - (4)]));
				}
			}
		}
	  }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 984 "awkgram.y"
    { sub_counter = 0; }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 985 "awkgram.y"
    {
		char *arr = (yyvsp[(2) - (4)])->lextok;

		(yyvsp[(2) - (4)])->opcode = Op_push_array;
		(yyvsp[(2) - (4)])->memory = variable(arr, Node_var_new);

		if ((yyvsp[(4) - (4)]) == NULL) {
			static short warned = FALSE;

			if (do_lint && ! warned) {
				warned = TRUE;
				lintwarn_ln((yyvsp[(1) - (4)])->source_line,
					_("`delete array' is a gawk extension"));
			}
			if (do_traditional)
				error_ln((yyvsp[(1) - (4)])->source_line,
					_("`delete array' is a gawk extension"));
			(yyvsp[(1) - (4)])->expr_count = 0;
			(yyval) = list_append(list_create((yyvsp[(2) - (4)])), (yyvsp[(1) - (4)]));
		} else {
			(yyvsp[(1) - (4)])->expr_count = sub_counter;
			(yyval) = list_append(list_append((yyvsp[(4) - (4)]), (yyvsp[(2) - (4)])), (yyvsp[(1) - (4)]));
		}
	  }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 1014 "awkgram.y"
    {
		static short warned = FALSE;
		char *arr = (yyvsp[(3) - (4)])->lextok;

		if (do_lint && ! warned) {
			warned = TRUE;
			lintwarn_ln((yyvsp[(1) - (4)])->source_line,
				_("`delete(array)' is a non-portable tawk extension"));
		}
		if (do_traditional) {
			error_ln((yyvsp[(1) - (4)])->source_line,
				_("`delete array' is a gawk extension"));
		}
		(yyvsp[(3) - (4)])->memory = variable(arr, Node_var_new);
		(yyvsp[(3) - (4)])->opcode = Op_push_array;
		(yyvsp[(1) - (4)])->expr_count = 0;
		(yyval) = list_append(list_create((yyvsp[(3) - (4)])), (yyvsp[(1) - (4)]));
	  }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 1033 "awkgram.y"
    {	(yyval) = optimize_assignment((yyvsp[(1) - (1)])); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 1038 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 1040 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 1045 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 1047 "awkgram.y"
    {
		if ((yyvsp[(1) - (2)]) == NULL)
			(yyval) = list_create((yyvsp[(2) - (2)]));
		else
			(yyval) = list_prepend((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
	  }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 1054 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 1059 "awkgram.y"
    {
		INSTRUCTION *casestmt = (yyvsp[(5) - (5)]);
		if ((yyvsp[(5) - (5)]) == NULL)
			casestmt = list_create(instruction(Op_no_op));	
		if (do_profiling)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		(yyvsp[(1) - (5)])->case_exp = (yyvsp[(2) - (5)]);
		(yyvsp[(1) - (5)])->case_stmt = casestmt;
		bcfree((yyvsp[(3) - (5)]));
		(yyval) = (yyvsp[(1) - (5)]);
	  }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 1071 "awkgram.y"
    {
		INSTRUCTION *casestmt = (yyvsp[(4) - (4)]);
		if ((yyvsp[(4) - (4)]) == NULL)
			casestmt = list_create(instruction(Op_no_op));
		if (do_profiling)
			(void) list_prepend(casestmt, instruction(Op_exec_count));
		bcfree((yyvsp[(2) - (4)]));
		(yyvsp[(1) - (4)])->case_stmt = casestmt;
		(yyval) = (yyvsp[(1) - (4)]);
	  }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 1085 "awkgram.y"
    {	(yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 1087 "awkgram.y"
    { 
		(yyvsp[(2) - (2)])->memory->numbr = -(force_number((yyvsp[(2) - (2)])->memory));
		bcfree((yyvsp[(1) - (2)]));
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 1093 "awkgram.y"
    {
		bcfree((yyvsp[(1) - (2)]));
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 1098 "awkgram.y"
    {	(yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 1100 "awkgram.y"
    {
		(yyvsp[(1) - (1)])->opcode = Op_push_re;
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1108 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 1110 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 1120 "awkgram.y"
    {
		(yyval) = (yyvsp[(2) - (3)]);
	  }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 1127 "awkgram.y"
    {
		in_print = FALSE;
		in_parens = 0;
		(yyval) = NULL;
	  }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 1132 "awkgram.y"
    { in_print = FALSE; in_parens = 0; }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 1133 "awkgram.y"
    {
		if ((yyvsp[(1) - (3)])->redir_type == redirect_twoway
		    	&& (yyvsp[(3) - (3)])->lasti->opcode == Op_K_getline_redir
		   	 	&& (yyvsp[(3) - (3)])->lasti->redir_type == redirect_twoway)
			yyerror(_("multistage two-way pipelines don't work"));
		(yyval) = list_prepend((yyvsp[(3) - (3)]), (yyvsp[(1) - (3)]));
	  }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 1144 "awkgram.y"
    {
		(yyval) = mk_condition((yyvsp[(3) - (6)]), (yyvsp[(1) - (6)]), (yyvsp[(6) - (6)]), NULL, NULL);
	  }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 1149 "awkgram.y"
    {
		(yyval) = mk_condition((yyvsp[(3) - (9)]), (yyvsp[(1) - (9)]), (yyvsp[(6) - (9)]), (yyvsp[(7) - (9)]), (yyvsp[(9) - (9)]));
	  }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 1166 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 1168 "awkgram.y"
    {
		bcfree((yyvsp[(1) - (2)]));
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 1181 "awkgram.y"
    {
		append_param((yyvsp[(1) - (1)])->lextok);
		(yyvsp[(1) - (1)])->lextok = NULL;
		bcfree((yyvsp[(1) - (1)]));
	  }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1187 "awkgram.y"
    {
		append_param((yyvsp[(3) - (3)])->lextok);
		(yyvsp[(3) - (3)])->lextok = NULL;
		bcfree((yyvsp[(3) - (3)]));
		yyerrok;
	  }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1194 "awkgram.y"
    { /* func_params = NULL; */ }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1196 "awkgram.y"
    { /* func_params = NULL; */ }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1198 "awkgram.y"
    { /* func_params = NULL; */ }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1204 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 1206 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 1211 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 1213 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1218 "awkgram.y"
    {	(yyval) = mk_expression_list(NULL, (yyvsp[(1) - (1)])); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1220 "awkgram.y"
    {
		(yyval) = mk_expression_list((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
		yyerrok;
	  }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1225 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1227 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1229 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1231 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1237 "awkgram.y"
    {
		if (do_lint && (yyvsp[(3) - (3)])->lasti->opcode == Op_match_rec)
			lintwarn_ln((yyvsp[(2) - (3)])->source_line,
				_("regular expression on right of assignment"));
		(yyval) = mk_assignment((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)]));
	  }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1244 "awkgram.y"
    {	(yyval) = mk_boolean((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1246 "awkgram.y"
    {	(yyval) = mk_boolean((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 1248 "awkgram.y"
    {
		if ((yyvsp[(1) - (3)])->lasti->opcode == Op_match_rec)
			warning_ln((yyvsp[(2) - (3)])->source_line,
				_("regular expression on left of `~' or `!~' operator"));

		if ((yyvsp[(3) - (3)])->lasti == (yyvsp[(3) - (3)])->nexti && (yyvsp[(3) - (3)])->nexti->opcode == Op_match_rec) {
			(yyvsp[(2) - (3)])->memory = (yyvsp[(3) - (3)])->nexti->memory;
			bcfree((yyvsp[(3) - (3)])->nexti);	/* Op_match_rec */
			bcfree((yyvsp[(3) - (3)]));			/* Op_list */
			(yyval) = list_append((yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]));
		} else {
			(yyvsp[(2) - (3)])->memory = make_regnode(Node_dynregex, NULL);
			(yyval) = list_append(list_merge((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])), (yyvsp[(2) - (3)]));
		}
	  }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 1264 "awkgram.y"
    {
		if (do_lint_old)
		  warning_ln((yyvsp[(2) - (3)])->source_line,
				_("old awk does not support the keyword `in' except after `for'"));
		(yyvsp[(3) - (3)])->nexti->opcode = Op_push_array;
		(yyvsp[(2) - (3)])->opcode = Op_in_array;
		(yyvsp[(2) - (3)])->expr_count = 1;
		(yyval) = list_append(list_merge((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])), (yyvsp[(2) - (3)]));
	  }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1274 "awkgram.y"
    {
		if (do_lint && (yyvsp[(3) - (3)])->lasti->opcode == Op_match_rec)
			lintwarn_ln((yyvsp[(2) - (3)])->source_line,
				_("regular expression on right of comparison"));
		(yyval) = list_append(list_merge((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])), (yyvsp[(2) - (3)]));
	  }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1281 "awkgram.y"
    { (yyval) = mk_condition((yyvsp[(1) - (5)]), (yyvsp[(2) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(4) - (5)]), (yyvsp[(5) - (5)])); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1283 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1288 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1290 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1292 "awkgram.y"
    {	
		(yyvsp[(2) - (2)])->opcode = Op_assign_quotient;
		(yyval) = (yyvsp[(2) - (2)]);
	  }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1300 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1302 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1307 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1309 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1314 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1316 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1318 "awkgram.y"
    {
		int count = 2;
		int is_simple_var = FALSE;

		if ((yyvsp[(1) - (2)])->lasti->opcode == Op_concat) {
			/* multiple (> 2) adjacent strings optimization */
			is_simple_var = ((yyvsp[(1) - (2)])->lasti->concat_flag & CSVAR);
			count = (yyvsp[(1) - (2)])->lasti->expr_count + 1;
			(yyvsp[(1) - (2)])->lasti->opcode = Op_no_op;
		} else {
			is_simple_var = ((yyvsp[(1) - (2)])->nexti->opcode == Op_push
						&& (yyvsp[(1) - (2)])->lasti == (yyvsp[(1) - (2)])->nexti); /* first exp. is a simple
						                             * variable?; kludge for use
						                             * in Op_assign_concat.
			 			                             */
		}

		if (do_optimize > 1
				&& (yyvsp[(1) - (2)])->nexti == (yyvsp[(1) - (2)])->lasti && (yyvsp[(1) - (2)])->nexti->opcode == Op_push_i
				&& (yyvsp[(2) - (2)])->nexti == (yyvsp[(2) - (2)])->lasti && (yyvsp[(2) - (2)])->nexti->opcode == Op_push_i
		) {
			NODE *n1 = (yyvsp[(1) - (2)])->nexti->memory;
			NODE *n2 = (yyvsp[(2) - (2)])->nexti->memory;
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
			bcfree((yyvsp[(2) - (2)])->nexti);
			bcfree((yyvsp[(2) - (2)]));
			(yyval) = (yyvsp[(1) - (2)]);
		} else {
			(yyval) = list_append(list_merge((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])), instruction(Op_concat));
			(yyval)->lasti->concat_flag = (is_simple_var ? CSVAR : 0);
			(yyval)->lasti->expr_count = count;
			if (count > max_args)
				max_args = count;
		}
	  }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1373 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1375 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1377 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1379 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1381 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1383 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1385 "awkgram.y"
    {
		/*
		 * In BEGINFILE/ENDFILE, allow `getline var < file'
		 */

		if (rule == BEGINFILE || rule == ENDFILE) {
			if ((yyvsp[(2) - (3)]) != NULL && (yyvsp[(3) - (3)]) != NULL)
				;	 /* all  ok */
			else {
				if ((yyvsp[(2) - (3)]) != NULL)
					error_ln((yyvsp[(1) - (3)])->source_line,
						_("`getline var' invalid inside `%s' rule"), ruletab[rule]);
				else
					error_ln((yyvsp[(1) - (3)])->source_line,
						_("`getline' invalid inside `%s' rule"), ruletab[rule]);
			}
		}
		if (do_lint && rule == END && (yyvsp[(3) - (3)]) == NULL)
			lintwarn_ln((yyvsp[(1) - (3)])->source_line,
				_("non-redirected `getline' undefined inside END action"));
		(yyval) = mk_getline((yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]), redirect_input);
	  }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1408 "awkgram.y"
    {
		(yyvsp[(2) - (2)])->opcode = Op_postincrement;
		(yyval) = mk_assignment((yyvsp[(1) - (2)]), NULL, (yyvsp[(2) - (2)]));
	  }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1413 "awkgram.y"
    {
		(yyvsp[(2) - (2)])->opcode = Op_postdecrement;
		(yyval) = mk_assignment((yyvsp[(1) - (2)]), NULL, (yyvsp[(2) - (2)]));
	  }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1418 "awkgram.y"
    {
		if (do_lint_old) {
		    warning_ln((yyvsp[(4) - (5)])->source_line,
				_("old awk does not support the keyword `in' except after `for'"));
		    warning_ln((yyvsp[(4) - (5)])->source_line,
				_("old awk does not support multidimensional arrays"));
		}
		(yyvsp[(5) - (5)])->nexti->opcode = Op_push_array;
		(yyvsp[(4) - (5)])->opcode = Op_in_array;
		if ((yyvsp[(2) - (5)]) == NULL) {	/* error */
			errcount++;
			(yyvsp[(4) - (5)])->expr_count = 0;
			(yyval) = list_merge((yyvsp[(5) - (5)]), (yyvsp[(4) - (5)]));
		} else {
			INSTRUCTION *t = (yyvsp[(2) - (5)]);
			(yyvsp[(4) - (5)])->expr_count = count_expressions(&t, FALSE);
			(yyval) = list_append(list_merge(t, (yyvsp[(5) - (5)])), (yyvsp[(4) - (5)]));
		}
	  }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1443 "awkgram.y"
    {
		  (yyval) = mk_getline((yyvsp[(3) - (4)]), (yyvsp[(4) - (4)]), (yyvsp[(1) - (4)]), (yyvsp[(2) - (4)])->redir_type);
		  bcfree((yyvsp[(2) - (4)]));
		}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1449 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1451 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1453 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1455 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1457 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1459 "awkgram.y"
    { (yyval) = mk_binary((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)])); }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1464 "awkgram.y"
    {
		(yyval) = list_create((yyvsp[(1) - (1)]));
	  }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1468 "awkgram.y"
    {
		if ((yyvsp[(2) - (2)])->opcode == Op_match_rec) {
			(yyvsp[(2) - (2)])->opcode = Op_nomatch;
			(yyvsp[(1) - (2)])->opcode = Op_push_i;
			(yyvsp[(1) - (2)])->memory = mk_number(0.0, (PERM|NUMCUR|NUMBER));	
			(yyval) = list_append(list_append(list_create((yyvsp[(1) - (2)])),
								instruction(Op_field_spec)), (yyvsp[(2) - (2)]));
		} else {
			if (do_optimize > 1 && (yyvsp[(2) - (2)])->nexti == (yyvsp[(2) - (2)])->lasti
							&& (yyvsp[(2) - (2)])->nexti->opcode == Op_push_i
			) {
				NODE *n = (yyvsp[(2) - (2)])->nexti->memory;
				if ((n->flags & (STRCUR|STRING)) != 0) {
					n->numbr = (AWKNUM) (n->stlen == 0);
					n->flags &= ~(STRCUR|STRING);
					n->flags |= (NUMCUR|NUMBER);
					efree(n->stptr);
					n->stptr = NULL;
					n->stlen = 0;
				} else
					n->numbr = (AWKNUM) (n->numbr == 0.0);
				bcfree((yyvsp[(1) - (2)]));
				(yyval) = (yyvsp[(2) - (2)]);
			} else {
				(yyvsp[(1) - (2)])->opcode = Op_not;
				add_lint((yyvsp[(2) - (2)]), LINT_assign_in_cond);
				(yyval) = list_append((yyvsp[(2) - (2)]), (yyvsp[(1) - (2)]));
			}
		}
	   }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1499 "awkgram.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1501 "awkgram.y"
    {
		(yyval) = snode((yyvsp[(3) - (4)]), (yyvsp[(1) - (4)]));
		if ((yyval) == NULL)
			YYABORT;
	  }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1507 "awkgram.y"
    {
		(yyval) = snode((yyvsp[(3) - (4)]), (yyvsp[(1) - (4)]));
		if ((yyval) == NULL)
			YYABORT;
	  }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1513 "awkgram.y"
    {
		static short warned1 = FALSE;

		if (do_lint && ! warned1) {
			warned1 = TRUE;
			lintwarn_ln((yyvsp[(1) - (1)])->source_line,
				_("call of `length' without parentheses is not portable"));
		}
		(yyval) = snode(NULL, (yyvsp[(1) - (1)]));
		if ((yyval) == NULL)
			YYABORT;
	  }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1528 "awkgram.y"
    {
		(yyvsp[(1) - (2)])->opcode = Op_preincrement;
		(yyval) = mk_assignment((yyvsp[(2) - (2)]), NULL, (yyvsp[(1) - (2)]));
	  }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1533 "awkgram.y"
    {
		(yyvsp[(1) - (2)])->opcode = Op_predecrement;
		(yyval) = mk_assignment((yyvsp[(2) - (2)]), NULL, (yyvsp[(1) - (2)]));
	  }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1538 "awkgram.y"
    {
		(yyval) = list_create((yyvsp[(1) - (1)]));
	  }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1542 "awkgram.y"
    {
		(yyval) = list_create((yyvsp[(1) - (1)]));
	  }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1546 "awkgram.y"
    {
		if ((yyvsp[(2) - (2)])->lasti->opcode == Op_push_i
				&& ((yyvsp[(2) - (2)])->lasti->memory->flags & (STRCUR|STRING)) == 0) {
			(yyvsp[(2) - (2)])->lasti->memory->numbr = -(force_number((yyvsp[(2) - (2)])->lasti->memory));
			(yyval) = (yyvsp[(2) - (2)]);
			bcfree((yyvsp[(1) - (2)]));
		} else {
			(yyvsp[(1) - (2)])->opcode = Op_unary_minus;
			(yyval) = list_append((yyvsp[(2) - (2)]), (yyvsp[(1) - (2)]));
		}
	  }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1558 "awkgram.y"
    {
	    /*
	     * was: $$ = $2
	     * POSIX semantics: force a conversion to numeric type
	     */
		(yyvsp[(1) - (2)])->opcode = Op_plus_i;
		(yyvsp[(1) - (2)])->memory = mk_number((AWKNUM) 0.0, (PERM|NUMCUR|NUMBER));
		(yyval) = list_append((yyvsp[(2) - (2)]), (yyvsp[(1) - (2)]));
	  }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1571 "awkgram.y"
    {
		func_use((yyvsp[(1) - (1)])->lasti->func_name, FUNC_USE);
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1576 "awkgram.y"
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
		
		f = (yyvsp[(2) - (2)])->lasti;
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

		(yyval) = list_prepend((yyvsp[(2) - (2)]), t);
	  }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1612 "awkgram.y"
    {
		param_sanity((yyvsp[(3) - (4)]));
		(yyvsp[(1) - (4)])->opcode = Op_func_call;
		(yyvsp[(1) - (4)])->func_body = NULL;
		if ((yyvsp[(3) - (4)]) == NULL) {	/* no argument or error */
			((yyvsp[(1) - (4)]) + 1)->expr_count = 0;
			(yyval) = list_create((yyvsp[(1) - (4)]));
		} else {
			INSTRUCTION *t = (yyvsp[(3) - (4)]);
			((yyvsp[(1) - (4)]) + 1)->expr_count = count_expressions(&t, TRUE); 
			(yyval) = list_append(t, (yyvsp[(1) - (4)]));
		}
	  }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1629 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1631 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1636 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1638 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 1643 "awkgram.y"
    {	(yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1645 "awkgram.y"
    {
		(yyval) = list_merge((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
	  }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1652 "awkgram.y"
    {
		INSTRUCTION *ip = (yyvsp[(1) - (1)])->lasti; 
		int count = ip->sub_count;	/* # of SUBSEP-seperated expressions */
		if (count > 1) {
			/* change Op_subscript or Op_sub_array to Op_concat */
			ip->opcode = Op_concat;
			ip->concat_flag = CSUBSEP;
			ip->expr_count = count;
		} else
			ip->opcode = Op_no_op;
		sub_counter++;	/* count # of dimensions */
		(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1669 "awkgram.y"
    {
		INSTRUCTION *t = (yyvsp[(2) - (3)]);
		if ((yyvsp[(2) - (3)]) == NULL) {
			error_ln((yyvsp[(3) - (3)])->source_line,
				_("invalid subscript expression"));
			/* install Null string as subscript. */
			t = list_create(instruction(Op_push_i));
			t->nexti->memory = Nnull_string;
			(yyvsp[(3) - (3)])->sub_count = 1;			
		} else
			(yyvsp[(3) - (3)])->sub_count = count_expressions(&t, FALSE);
		(yyval) = list_append(t, (yyvsp[(3) - (3)]));
	  }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1686 "awkgram.y"
    {	(yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1688 "awkgram.y"
    {
		(yyval) = list_merge((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
	  }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1695 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1700 "awkgram.y"
    {
		char *var_name = (yyvsp[(1) - (1)])->lextok;

		(yyvsp[(1) - (1)])->opcode = Op_push;
		(yyvsp[(1) - (1)])->memory = variable(var_name, Node_var_new);
		(yyval) = list_create((yyvsp[(1) - (1)]));
	  }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1708 "awkgram.y"
    {
		NODE *n;

		char *arr = (yyvsp[(1) - (2)])->lextok;
		if ((n = lookup(arr)) != NULL && ! isarray(n))
			yyerror(_("use of non-array as array"));
		(yyvsp[(1) - (2)])->memory = variable(arr, Node_var_new);
		(yyvsp[(1) - (2)])->opcode = Op_push_array;
		(yyval) = list_prepend((yyvsp[(2) - (2)]), (yyvsp[(1) - (2)]));
	  }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1722 "awkgram.y"
    {
		INSTRUCTION *ip = (yyvsp[(1) - (1)])->nexti;
		if (ip->opcode == Op_push
				&& ip->memory->type == Node_var
				&& ip->memory->var_update
		) {
			(yyval) = list_prepend((yyvsp[(1) - (1)]), instruction(Op_var_update));
			(yyval)->nexti->memory = ip->memory;
			(yyval)->nexti->update_var = ip->memory->var_update;
		} else
			(yyval) = (yyvsp[(1) - (1)]);
	  }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1735 "awkgram.y"
    {
		(yyval) = list_append((yyvsp[(2) - (3)]), (yyvsp[(1) - (3)]));
		if ((yyvsp[(3) - (3)]) != NULL)
		  mk_assignment((yyvsp[(2) - (3)]), NULL, (yyvsp[(3) - (3)]));
	  }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1744 "awkgram.y"
    {
		(yyvsp[(1) - (1)])->opcode = Op_postincrement;
	  }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1748 "awkgram.y"
    {
		(yyvsp[(1) - (1)])->opcode = Op_postdecrement;
	  }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1751 "awkgram.y"
    { (yyval) = NULL; }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1759 "awkgram.y"
    { yyerrok; }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1763 "awkgram.y"
    { yyerrok; }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1772 "awkgram.y"
    { yyerrok; }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1776 "awkgram.y"
    { (yyval) = (yyvsp[(1) - (1)]); yyerrok; }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1780 "awkgram.y"
    { yyerrok; }
    break;



/* Line 1806 of yacc.c  */
#line 4253 "awkgram.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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
      if (!yypact_value_is_default (yyn))
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
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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



/* Line 2067 of yacc.c  */
#line 1782 "awkgram.y"


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
			 * due to syntax error.
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


