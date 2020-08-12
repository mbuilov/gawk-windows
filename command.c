/* A Bison parser, made by GNU Bison 3.7.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         ZZSTYPE
/* Substitute the variable and function names.  */
#define yyparse         zzparse
#define yylex           zzlex
#define yyerror         zzerror
#define yydebug         zzdebug
#define yynerrs         zznerrs
#define yylval          zzlval
#define yychar          zzchar

/* First part of user prologue.  */
#line 30 "command.y"

#include "awk.h"
#include "cmd.h"

#if 0
#define YYDEBUG 12
int yydebug = 2;
#endif

static cmdtok_t yylex(void);
ATTRIBUTE_PRINTF(mesg, 1, 2)
static void yyerror(const char *mesg, ...);

static int find_command(const char *token, size_t toklen);

static bool want_nodeval = false;

static int cmd_idx = -1;		/* index of current command in cmd table */
static int repeat_idx = -1;		/* index of last repeatable command in command table */
static CMDARG *arg_list = NULL;		/* list of arguments */
static unsigned long dbg_errcount = 0;
static char *lexptr_begin = NULL;
static bool in_commands = false;
static unsigned num_dim;

static bool in_eval = false;
static const char start_EVAL[] = "function @eval(){";
static const char end_EVAL[] = "}";
static CMDARG *append_statement(CMDARG *stmt_list, const char *stmt);
static NODE *concat_args(CMDARG *a, unsigned count);

#ifdef HAVE_LIBREADLINE
static const char *next_word(const char *p, size_t len, const char **endp);
static void history_expand_line(char **line);
static char *command_generator(const char *text, int state);
static char *srcfile_generator(const char *text, int state);
static char *argument_generator(const char *text, int state);
static char *variable_generator(const char *text, int state);
extern char *option_generator(const char *text, int state);
static enum argtype this_cmd = D_illegal;
#else
#define history_expand_line(p)	((void)0)	/* nothing */
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

extern const struct argtoken argtab[];
extern const struct cmdtoken cmdtab[];

static CMDARG *mk_cmdarg(enum argtype type);
static void append_cmdarg(CMDARG *arg);
static int find_argument(CMDARG *arg);
#define ZZSTYPE CMDARG *
#define yysymbol_kind_t zzsymbol_kind_t

#line 148 "command.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif


/* Debug traces.  */
#ifndef ZZDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ZZDEBUG 1
#  else
#   define ZZDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ZZDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ZZDEBUG */
#if ZZDEBUG
extern int zzdebug;
#endif

/* Token kinds.  */
#ifndef ZZTOKENTYPE
# define ZZTOKENTYPE
  enum zztokentype
  {
    ZZEMPTY = -2,
    ZZEOF = 0,                     /* "end of file"  */
    ZZerror = 256,                 /* error  */
    ZZUNDEF = 257,                 /* "invalid token"  */
    D_BACKTRACE = 258,             /* D_BACKTRACE  */
    D_BREAK = 259,                 /* D_BREAK  */
    D_CLEAR = 260,                 /* D_CLEAR  */
    D_CONTINUE = 261,              /* D_CONTINUE  */
    D_DELETE = 262,                /* D_DELETE  */
    D_DISABLE = 263,               /* D_DISABLE  */
    D_DOWN = 264,                  /* D_DOWN  */
    D_ENABLE = 265,                /* D_ENABLE  */
    D_FINISH = 266,                /* D_FINISH  */
    D_FRAME = 267,                 /* D_FRAME  */
    D_HELP = 268,                  /* D_HELP  */
    D_IGNORE = 269,                /* D_IGNORE  */
    D_INFO = 270,                  /* D_INFO  */
    D_LIST = 271,                  /* D_LIST  */
    D_NEXT = 272,                  /* D_NEXT  */
    D_NEXTI = 273,                 /* D_NEXTI  */
    D_PRINT = 274,                 /* D_PRINT  */
    D_PRINTF = 275,                /* D_PRINTF  */
    D_QUIT = 276,                  /* D_QUIT  */
    D_RETURN = 277,                /* D_RETURN  */
    D_RUN = 278,                   /* D_RUN  */
    D_SET = 279,                   /* D_SET  */
    D_STEP = 280,                  /* D_STEP  */
    D_STEPI = 281,                 /* D_STEPI  */
    D_TBREAK = 282,                /* D_TBREAK  */
    D_UP = 283,                    /* D_UP  */
    D_UNTIL = 284,                 /* D_UNTIL  */
    D_DISPLAY = 285,               /* D_DISPLAY  */
    D_UNDISPLAY = 286,             /* D_UNDISPLAY  */
    D_WATCH = 287,                 /* D_WATCH  */
    D_UNWATCH = 288,               /* D_UNWATCH  */
    D_DUMP = 289,                  /* D_DUMP  */
    D_TRACE = 290,                 /* D_TRACE  */
    D_INT = 291,                   /* D_INT  */
    D_STRING = 292,                /* D_STRING  */
    D_NODE = 293,                  /* D_NODE  */
    D_VARIABLE = 294,              /* D_VARIABLE  */
    D_OPTION = 295,                /* D_OPTION  */
    D_COMMANDS = 296,              /* D_COMMANDS  */
    D_END = 297,                   /* D_END  */
    D_SILENT = 298,                /* D_SILENT  */
    D_SOURCE = 299,                /* D_SOURCE  */
    D_SAVE = 300,                  /* D_SAVE  */
    D_EVAL = 301,                  /* D_EVAL  */
    D_CONDITION = 302,             /* D_CONDITION  */
    D_STATEMENT = 303              /* D_STATEMENT  */
  };
  typedef enum zztokentype zztoken_kind_t;
#endif

/* Value type.  */
#if ! defined ZZSTYPE && ! defined ZZSTYPE_IS_DECLARED
typedef int ZZSTYPE;
# define ZZSTYPE_IS_TRIVIAL 1
# define ZZSTYPE_IS_DECLARED 1
#endif


extern ZZSTYPE zzlval;

int zzparse (void);


/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_D_BACKTRACE = 3,                /* D_BACKTRACE  */
  YYSYMBOL_D_BREAK = 4,                    /* D_BREAK  */
  YYSYMBOL_D_CLEAR = 5,                    /* D_CLEAR  */
  YYSYMBOL_D_CONTINUE = 6,                 /* D_CONTINUE  */
  YYSYMBOL_D_DELETE = 7,                   /* D_DELETE  */
  YYSYMBOL_D_DISABLE = 8,                  /* D_DISABLE  */
  YYSYMBOL_D_DOWN = 9,                     /* D_DOWN  */
  YYSYMBOL_D_ENABLE = 10,                  /* D_ENABLE  */
  YYSYMBOL_D_FINISH = 11,                  /* D_FINISH  */
  YYSYMBOL_D_FRAME = 12,                   /* D_FRAME  */
  YYSYMBOL_D_HELP = 13,                    /* D_HELP  */
  YYSYMBOL_D_IGNORE = 14,                  /* D_IGNORE  */
  YYSYMBOL_D_INFO = 15,                    /* D_INFO  */
  YYSYMBOL_D_LIST = 16,                    /* D_LIST  */
  YYSYMBOL_D_NEXT = 17,                    /* D_NEXT  */
  YYSYMBOL_D_NEXTI = 18,                   /* D_NEXTI  */
  YYSYMBOL_D_PRINT = 19,                   /* D_PRINT  */
  YYSYMBOL_D_PRINTF = 20,                  /* D_PRINTF  */
  YYSYMBOL_D_QUIT = 21,                    /* D_QUIT  */
  YYSYMBOL_D_RETURN = 22,                  /* D_RETURN  */
  YYSYMBOL_D_RUN = 23,                     /* D_RUN  */
  YYSYMBOL_D_SET = 24,                     /* D_SET  */
  YYSYMBOL_D_STEP = 25,                    /* D_STEP  */
  YYSYMBOL_D_STEPI = 26,                   /* D_STEPI  */
  YYSYMBOL_D_TBREAK = 27,                  /* D_TBREAK  */
  YYSYMBOL_D_UP = 28,                      /* D_UP  */
  YYSYMBOL_D_UNTIL = 29,                   /* D_UNTIL  */
  YYSYMBOL_D_DISPLAY = 30,                 /* D_DISPLAY  */
  YYSYMBOL_D_UNDISPLAY = 31,               /* D_UNDISPLAY  */
  YYSYMBOL_D_WATCH = 32,                   /* D_WATCH  */
  YYSYMBOL_D_UNWATCH = 33,                 /* D_UNWATCH  */
  YYSYMBOL_D_DUMP = 34,                    /* D_DUMP  */
  YYSYMBOL_D_TRACE = 35,                   /* D_TRACE  */
  YYSYMBOL_D_INT = 36,                     /* D_INT  */
  YYSYMBOL_D_STRING = 37,                  /* D_STRING  */
  YYSYMBOL_D_NODE = 38,                    /* D_NODE  */
  YYSYMBOL_D_VARIABLE = 39,                /* D_VARIABLE  */
  YYSYMBOL_D_OPTION = 40,                  /* D_OPTION  */
  YYSYMBOL_D_COMMANDS = 41,                /* D_COMMANDS  */
  YYSYMBOL_D_END = 42,                     /* D_END  */
  YYSYMBOL_D_SILENT = 43,                  /* D_SILENT  */
  YYSYMBOL_D_SOURCE = 44,                  /* D_SOURCE  */
  YYSYMBOL_D_SAVE = 45,                    /* D_SAVE  */
  YYSYMBOL_D_EVAL = 46,                    /* D_EVAL  */
  YYSYMBOL_D_CONDITION = 47,               /* D_CONDITION  */
  YYSYMBOL_D_STATEMENT = 48,               /* D_STATEMENT  */
  YYSYMBOL_49_ = 49,                       /* '='  */
  YYSYMBOL_50_ = 50,                       /* ','  */
  YYSYMBOL_51_ = 51,                       /* ':'  */
  YYSYMBOL_52_ = 52,                       /* '@'  */
  YYSYMBOL_53_ = 53,                       /* '+'  */
  YYSYMBOL_54_ = 54,                       /* '-'  */
  YYSYMBOL_55_ = 55,                       /* '['  */
  YYSYMBOL_56_ = 56,                       /* ']'  */
  YYSYMBOL_57_ = 57,                       /* '$'  */
  YYSYMBOL_58_n_ = 58,                     /* '\n'  */
  YYSYMBOL_YYACCEPT = 59,                  /* $accept  */
  YYSYMBOL_input = 60,                     /* input  */
  YYSYMBOL_line = 61,                      /* line  */
  YYSYMBOL_control_cmd = 62,               /* control_cmd  */
  YYSYMBOL_d_cmd = 63,                     /* d_cmd  */
  YYSYMBOL_frame_cmd = 64,                 /* frame_cmd  */
  YYSYMBOL_break_cmd = 65,                 /* break_cmd  */
  YYSYMBOL_set_want_nodeval = 66,          /* set_want_nodeval  */
  YYSYMBOL_eval_prologue = 67,             /* eval_prologue  */
  YYSYMBOL_statement_list = 68,            /* statement_list  */
  YYSYMBOL_69_1 = 69,                      /* @1  */
  YYSYMBOL_eval_cmd = 70,                  /* eval_cmd  */
  YYSYMBOL_command = 71,                   /* command  */
  YYSYMBOL_72_2 = 72,                      /* $@2  */
  YYSYMBOL_73_3 = 73,                      /* $@3  */
  YYSYMBOL_74_4 = 74,                      /* $@4  */
  YYSYMBOL_75_5 = 75,                      /* $@5  */
  YYSYMBOL_76_6 = 76,                      /* $@6  */
  YYSYMBOL_77_7 = 77,                      /* $@7  */
  YYSYMBOL_78_8 = 78,                      /* $@8  */
  YYSYMBOL_condition_exp = 79,             /* condition_exp  */
  YYSYMBOL_commands_arg = 80,              /* commands_arg  */
  YYSYMBOL_opt_param_list = 81,            /* opt_param_list  */
  YYSYMBOL_param_list = 82,                /* param_list  */
  YYSYMBOL_opt_string_node = 83,           /* opt_string_node  */
  YYSYMBOL_string_node = 84,               /* string_node  */
  YYSYMBOL_option_args = 85,               /* option_args  */
  YYSYMBOL_func_name = 86,                 /* func_name  */
  YYSYMBOL_location = 87,                  /* location  */
  YYSYMBOL_break_args = 88,                /* break_args  */
  YYSYMBOL_89_9 = 89,                      /* $@9  */
  YYSYMBOL_90_10 = 90,                     /* $@10  */
  YYSYMBOL_opt_variable = 91,              /* opt_variable  */
  YYSYMBOL_opt_string = 92,                /* opt_string  */
  YYSYMBOL_opt_node = 93,                  /* opt_node  */
  YYSYMBOL_help_args = 94,                 /* help_args  */
  YYSYMBOL_enable_args = 95,               /* enable_args  */
  YYSYMBOL_print_exp = 96,                 /* print_exp  */
  YYSYMBOL_print_args = 97,                /* print_args  */
  YYSYMBOL_printf_exp = 98,                /* printf_exp  */
  YYSYMBOL_printf_args = 99,               /* printf_args  */
  YYSYMBOL_list_args = 100,                /* list_args  */
  YYSYMBOL_integer_range = 101,            /* integer_range  */
  YYSYMBOL_opt_integer_list = 102,         /* opt_integer_list  */
  YYSYMBOL_integer_list = 103,             /* integer_list  */
  YYSYMBOL_exp_list = 104,                 /* exp_list  */
  YYSYMBOL_subscript = 105,                /* subscript  */
  YYSYMBOL_subscript_list = 106,           /* subscript_list  */
  YYSYMBOL_variable = 107,                 /* variable  */
  YYSYMBOL_node = 108,                     /* node  */
  YYSYMBOL_opt_plus_integer = 109,         /* opt_plus_integer  */
  YYSYMBOL_opt_integer = 110,              /* opt_integer  */
  YYSYMBOL_plus_integer = 111,             /* plus_integer  */
  YYSYMBOL_integer = 112,                  /* integer  */
  YYSYMBOL_nls = 113                       /* nls  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined ZZSTYPE_IS_TRIVIAL && ZZSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

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
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  203

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   303


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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

#if ZZDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   113,   113,   114,   132,   133,   183,   190,   191,   192,
     193,   194,   198,   199,   200,   201,   205,   206,   207,   208,
     212,   213,   218,   222,   242,   249,   249,   256,   272,   286,
     287,   288,   289,   290,   291,   297,   309,   310,   311,   311,
     312,   312,   313,   314,   315,   316,   317,   317,   318,   319,
     319,   320,   320,   321,   321,   322,   323,   324,   329,   334,
     360,   370,   375,   387,   387,   395,   409,   422,   423,   429,
     430,   434,   435,   436,   437,   443,   444,   445,   450,   461,
     462,   467,   475,   492,   493,   494,   495,   496,   501,   502,
     502,   503,   504,   504,   505,   510,   511,   516,   517,   522,
     523,   527,   528,   532,   533,   548,   549,   554,   562,   563,
     564,   565,   569,   570,   574,   575,   576,   581,   582,   584,
     591,   592,   593,   594,   595,   596,   600,   613,   614,   615,
     619,   620,   621,   622,   626,   628,   630,   634,   649,   653,
     655,   660,   661,   670,   680,   682,   689,   702,   703,   709,
     710,   715,   721,   730,   732,   734,   742
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if ZZDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "D_BACKTRACE",
  "D_BREAK", "D_CLEAR", "D_CONTINUE", "D_DELETE", "D_DISABLE", "D_DOWN",
  "D_ENABLE", "D_FINISH", "D_FRAME", "D_HELP", "D_IGNORE", "D_INFO",
  "D_LIST", "D_NEXT", "D_NEXTI", "D_PRINT", "D_PRINTF", "D_QUIT",
  "D_RETURN", "D_RUN", "D_SET", "D_STEP", "D_STEPI", "D_TBREAK", "D_UP",
  "D_UNTIL", "D_DISPLAY", "D_UNDISPLAY", "D_WATCH", "D_UNWATCH", "D_DUMP",
  "D_TRACE", "D_INT", "D_STRING", "D_NODE", "D_VARIABLE", "D_OPTION",
  "D_COMMANDS", "D_END", "D_SILENT", "D_SOURCE", "D_SAVE", "D_EVAL",
  "D_CONDITION", "D_STATEMENT", "'='", "','", "':'", "'@'", "'+'", "'-'",
  "'['", "']'", "'$'", "'\\n'", "$accept", "input", "line", "control_cmd",
  "d_cmd", "frame_cmd", "break_cmd", "set_want_nodeval", "eval_prologue",
  "statement_list", "@1", "eval_cmd", "command", "$@2", "$@3", "$@4",
  "$@5", "$@6", "$@7", "$@8", "condition_exp", "commands_arg",
  "opt_param_list", "param_list", "opt_string_node", "string_node",
  "option_args", "func_name", "location", "break_args", "$@9", "$@10",
  "opt_variable", "opt_string", "opt_node", "help_args", "enable_args",
  "print_exp", "print_args", "printf_exp", "printf_args", "list_args",
  "integer_range", "opt_integer_list", "integer_list", "exp_list",
  "subscript", "subscript_list", "variable", "node", "opt_plus_integer",
  "opt_integer", "plus_integer", "integer", "nls", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,    61,
      44,    58,    64,    43,    45,    91,    93,    36,    10
};
#endif

#define YYPACT_NINF (-151)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-148)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
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

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
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

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
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
static const yytype_int8 yystos[] =
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

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
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

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ZZEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ZZEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ZZerror or ZZUNDEF. */
#define YYERRCODE ZZUNDEF


/* Enable debugging if requested.  */
#if ZZDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ZZDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ZZDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = ZZEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == ZZEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= ZZEOF)
    {
      yychar = ZZEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ZZerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ZZUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = ZZEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* input: input line  */
#line 115 "command.y"
          {
		cmd_idx = -1;
		want_nodeval = false;
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
#line 1513 "command.c"
    break;

  case 5: /* line: command nls  */
#line 134 "command.y"
          {
		if (dbg_errcount == 0 && cmd_idx >= 0) {
			Func_cmd cmdfunc;
			bool terminate = false;
			CMDARG *args;
			enum argtype ctype;

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
			want_nodeval = false;

			args = arg_list;
			arg_list = NULL;

			terminate = (*cmdfunc)(args, ctype);
			if (! in_commands || ctype == D_commands)
				free_cmdarg(args);
			if (terminate)
				YYACCEPT;
		}
	  }
#line 1567 "command.c"
    break;

  case 6: /* line: error nls  */
#line 184 "command.y"
          {
		yyerrok;
	  }
#line 1575 "command.c"
    break;

  case 22: /* set_want_nodeval: %empty  */
#line 218 "command.y"
          { want_nodeval = true; }
#line 1581 "command.c"
    break;

  case 23: /* eval_prologue: D_EVAL set_want_nodeval opt_param_list nls  */
#line 223 "command.y"
          {
		if (dbg_errcount == 0) {
			/* don't free arg_list;	passed on to statement_list
			 * non-terminal (empty rule action). See below.
			 */
			if (input_from_tty) {
				dbg_prompt = eval_prompt;
				fprintf(out_fp,
		_("Type (g)awk statement(s). End with the command `end'\n"));
				rl_inhibit_completion = 1;
			}
			cmd_idx = -1;
			in_eval = true;
		}
	  }
#line 1601 "command.c"
    break;

  case 24: /* statement_list: %empty  */
#line 242 "command.y"
          {
		yyval = append_statement(arg_list, start_EVAL);
		if (read_a_line == read_commands_string)	/* unserializing 'eval' in 'commands' */
			yyval->a_string[0] = '\0';
		free_cmdarg(arg_list);
		arg_list = NULL;
	  }
#line 1613 "command.c"
    break;

  case 25: /* @1: %empty  */
#line 249 "command.y"
                                     { yyval = append_statement(yyvsp[-1], lexptr_begin); }
#line 1619 "command.c"
    break;

  case 26: /* statement_list: statement_list D_STATEMENT @1 nls  */
#line 250 "command.y"
          {
		yyval = yyvsp[-1];
	  }
#line 1627 "command.c"
    break;

  case 27: /* eval_cmd: eval_prologue statement_list D_END  */
#line 257 "command.y"
          {
		arg_list = append_statement(yyvsp[-1], end_EVAL);
		if (read_a_line == read_commands_string) {	/* unserializing 'eval' in 'commands' */
			char *str = arg_list->a_string;
			size_t len = strlen(str);
			assert(len > 2 && str[len - 2] == '}');
			str[len - 2] = '\0';
		}
		if (input_from_tty) {
			dbg_prompt = in_commands ? commands_prompt : dgawk_prompt;
			rl_inhibit_completion = 0;
		}
		cmd_idx = find_command("eval", 4);
		in_eval = false;
	  }
#line 1647 "command.c"
    break;

  case 28: /* eval_cmd: D_EVAL set_want_nodeval string_node  */
#line 273 "command.y"
          {
		NODE *n;
		CMDARG *arg;
		n = yyvsp[0]->a_node;
		arg = append_statement(NULL, start_EVAL);
		(void) append_statement(arg, n->stptr);
		(void) append_statement(arg, end_EVAL);
		free_cmdarg(arg_list);
		arg_list = arg;
	  }
#line 1662 "command.c"
    break;

  case 34: /* command: frame_cmd opt_integer  */
#line 292 "command.y"
          {
		if (cmdtab[cmd_idx].cls == D_FRAME
				&& yyvsp[0] != NULL && yyvsp[0]->a_int < 0)
			yyerror(_("invalid frame number: %" AWKLONGFMT ""), TO_AWK_LONG(yyvsp[0]->a_int));
	  }
#line 1672 "command.c"
    break;

  case 35: /* command: D_INFO D_STRING  */
#line 298 "command.y"
          {
		int idx = find_argument(yyvsp[0]);
		if (idx < 0)
			yyerror(_("info: invalid option - `%s'"), yyvsp[0]->a_string);
		else {
			efree(yyvsp[0]->a_string);
			yyvsp[0]->a_string = NULL;
			yyvsp[0]->type = D_argument;
			yyvsp[0]->a_argument = argtab[idx].value;
		}
	  }
#line 1688 "command.c"
    break;

  case 38: /* $@2: %empty  */
#line 311 "command.y"
                  { want_nodeval = true; }
#line 1694 "command.c"
    break;

  case 40: /* $@3: %empty  */
#line 312 "command.y"
                   { want_nodeval = true; }
#line 1700 "command.c"
    break;

  case 46: /* $@4: %empty  */
#line 317 "command.y"
                { want_nodeval = true; }
#line 1706 "command.c"
    break;

  case 49: /* $@5: %empty  */
#line 319 "command.y"
                   { want_nodeval = true; }
#line 1712 "command.c"
    break;

  case 51: /* $@6: %empty  */
#line 320 "command.y"
                    { want_nodeval = true; }
#line 1718 "command.c"
    break;

  case 53: /* $@7: %empty  */
#line 321 "command.y"
                  { want_nodeval = true; }
#line 1724 "command.c"
    break;

  case 57: /* command: D_SOURCE D_STRING  */
#line 325 "command.y"
          {
		if (in_cmd_src(yyvsp[0]->a_string))
			yyerror(_("source: `%s': already sourced."), yyvsp[0]->a_string);
	  }
#line 1733 "command.c"
    break;

  case 58: /* command: D_SAVE D_STRING  */
#line 330 "command.y"
          {
		if (! input_from_tty)
			yyerror(_("save: `%s': command not permitted."), yyvsp[0]->a_string);
	  }
#line 1742 "command.c"
    break;

  case 59: /* command: D_COMMANDS commands_arg  */
#line 335 "command.y"
          {
		enum argtype type = D_illegal;
		unsigned num = 0; /* Silence compiler.  */

		if (yyvsp[0] != NULL)
			num = (unsigned) yyvsp[0]->a_int;

		if (dbg_errcount != 0)
			;
		else if (in_commands)
			yyerror(_("cannot use command `commands' for breakpoint/watchpoint commands"));
		else if (yyvsp[0] == NULL && D_illegal == (type = has_break_or_watch_point(&num, true)))
			yyerror(_("no breakpoint/watchpoint has been set yet"));
		else if (yyvsp[0] != NULL && D_illegal == (type = has_break_or_watch_point(&num, false)))
			yyerror(_("invalid breakpoint/watchpoint number"));
		if (type != D_illegal) {
			in_commands = true;
			if (input_from_tty) {
				dbg_prompt = commands_prompt;
				fprintf(out_fp, _("Type commands for when %s %u is hit, one per line.\n"),
								(type == D_break) ? "breakpoint" : "watchpoint", num);
				fprintf(out_fp, _("End with the command `end'\n"));
			}
		}
	  }
#line 1772 "command.c"
    break;

  case 60: /* command: D_END  */
#line 361 "command.y"
          {
		if (! in_commands)
			yyerror(_("`end' valid only in command `commands' or `eval'"));
		else {
			if (input_from_tty)
				dbg_prompt = dgawk_prompt;
			in_commands = false;
		}
	  }
#line 1786 "command.c"
    break;

  case 61: /* command: D_SILENT  */
#line 371 "command.y"
          {
		if (! in_commands)
			yyerror(_("`silent' valid only in command `commands'"));
	  }
#line 1795 "command.c"
    break;

  case 62: /* command: D_TRACE D_STRING  */
#line 376 "command.y"
          {
		int idx = find_argument(yyvsp[0]);
		if (idx < 0)
			yyerror(_("trace: invalid option - `%s'"), yyvsp[0]->a_string);
		else {
			efree(yyvsp[0]->a_string);
			yyvsp[0]->a_string = NULL;
			yyvsp[0]->type = D_argument;
			yyvsp[0]->a_argument = argtab[idx].value;
		}
	  }
#line 1811 "command.c"
    break;

  case 63: /* $@8: %empty  */
#line 387 "command.y"
                                   { want_nodeval = true; }
#line 1817 "command.c"
    break;

  case 64: /* command: D_CONDITION plus_integer $@8 condition_exp  */
#line 388 "command.y"
          {
		enum argtype type;
		unsigned num = (unsigned) yyvsp[-2]->a_int;
		type = has_break_or_watch_point(&num, false);
		if (type == D_illegal)
			yyerror(_("condition: invalid breakpoint/watchpoint number"));
	  }
#line 1829 "command.c"
    break;

  case 65: /* command: eval_cmd  */
#line 396 "command.y"
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
#line 1844 "command.c"
    break;

  case 66: /* condition_exp: opt_string_node  */
#line 410 "command.y"
          {
		if (yyvsp[0] != NULL) {
			NODE *n = yyvsp[0]->a_node;
			yyvsp[0]->type = D_string;
			yyvsp[0]->a_string = n->stptr;
			freenode(n);
		}
		yyval = yyvsp[0];
	  }
#line 1858 "command.c"
    break;

  case 68: /* commands_arg: error  */
#line 424 "command.y"
          {	yyval = NULL; }
#line 1864 "command.c"
    break;

  case 69: /* opt_param_list: %empty  */
#line 429 "command.y"
          { yyval = NULL; }
#line 1870 "command.c"
    break;

  case 74: /* param_list: error  */
#line 438 "command.y"
          { yyval = NULL; }
#line 1876 "command.c"
    break;

  case 75: /* opt_string_node: %empty  */
#line 443 "command.y"
          { yyval = NULL; }
#line 1882 "command.c"
    break;

  case 77: /* opt_string_node: error  */
#line 446 "command.y"
          { yyval = NULL; }
#line 1888 "command.c"
    break;

  case 78: /* string_node: D_NODE  */
#line 451 "command.y"
          {
		NODE *n;
		n = yyvsp[0]->a_node;
		if ((n->flags & STRING) == 0)
			yyerror(_("argument not a string"));
	  }
#line 1899 "command.c"
    break;

  case 79: /* option_args: %empty  */
#line 461 "command.y"
          { yyval = NULL; }
#line 1905 "command.c"
    break;

  case 80: /* option_args: D_STRING  */
#line 463 "command.y"
          {
		if (find_option(yyvsp[0]->a_string) < 0)
			yyerror(_("option: invalid parameter - `%s'"), yyvsp[0]->a_string);
 	  }
#line 1914 "command.c"
    break;

  case 81: /* option_args: D_STRING '=' D_STRING  */
#line 468 "command.y"
          {
		if (find_option(yyvsp[-2]->a_string) < 0)
			yyerror(_("option: invalid parameter - `%s'"), yyvsp[-2]->a_string);
 	  }
#line 1923 "command.c"
    break;

  case 82: /* func_name: D_STRING  */
#line 476 "command.y"
          {
		NODE *n;
		n = lookup(yyvsp[0]->a_string);
		if (n == NULL || n->type != Node_func)
			yyerror(_("no such function - `%s'"), yyvsp[0]->a_string);
		else {
			yyvsp[0]->type = D_func;
			efree(yyvsp[0]->a_string);
			yyvsp[0]->a_string = NULL;
			yyvsp[0]->a_node = n;
		}
	  }
#line 1940 "command.c"
    break;

  case 83: /* location: %empty  */
#line 492 "command.y"
          { yyval = NULL; }
#line 1946 "command.c"
    break;

  case 88: /* break_args: %empty  */
#line 501 "command.y"
          { yyval = NULL; }
#line 1952 "command.c"
    break;

  case 89: /* $@9: %empty  */
#line 502 "command.y"
                       { want_nodeval = true; }
#line 1958 "command.c"
    break;

  case 92: /* $@10: %empty  */
#line 504 "command.y"
                                    { want_nodeval = true; }
#line 1964 "command.c"
    break;

  case 95: /* opt_variable: %empty  */
#line 510 "command.y"
          { yyval = NULL; }
#line 1970 "command.c"
    break;

  case 97: /* opt_string: %empty  */
#line 516 "command.y"
          { yyval = NULL; }
#line 1976 "command.c"
    break;

  case 99: /* opt_node: %empty  */
#line 522 "command.y"
          { yyval = NULL; }
#line 1982 "command.c"
    break;

  case 104: /* enable_args: D_STRING opt_integer_list  */
#line 534 "command.y"
          {
		int idx = find_argument(yyvsp[-1]);
		if (idx < 0)
			yyerror(_("enable: invalid option - `%s'"), yyvsp[-1]->a_string);
		else {
			efree(yyvsp[-1]->a_string);
			yyvsp[-1]->a_string = NULL;
			yyvsp[-1]->type = D_argument;
			yyvsp[-1]->a_argument = argtab[idx].value;
		}
	  }
#line 1998 "command.c"
    break;

  case 106: /* print_exp: '@' D_VARIABLE  */
#line 550 "command.y"
          {
		yyvsp[0]->type = D_array;	/* dump all items */
		yyvsp[0]->a_count = 0;
	  }
#line 2007 "command.c"
    break;

  case 107: /* print_exp: '@' D_VARIABLE subscript_list  */
#line 555 "command.y"
          {
		yyvsp[-1]->type = D_array;
		yyvsp[-1]->a_count = num_dim;
	  }
#line 2016 "command.c"
    break;

  case 117: /* list_args: %empty  */
#line 581 "command.y"
          { yyval = NULL; }
#line 2022 "command.c"
    break;

  case 118: /* list_args: '+'  */
#line 583 "command.y"
          { yyval = NULL; }
#line 2028 "command.c"
    break;

  case 119: /* list_args: '-'  */
#line 585 "command.y"
          {
		CMDARG *a;
		a = mk_cmdarg(D_int);
		a->a_int = -1;
		append_cmdarg(a);
	  }
#line 2039 "command.c"
    break;

  case 126: /* integer_range: plus_integer '-' plus_integer  */
#line 601 "command.y"
          {
		if (yyvsp[-2]->a_int > yyvsp[0]->a_int)
			yyerror(_("invalid range specification: %" AWKLONGFMT " - %" AWKLONGFMT ""),
				TO_AWK_LONG(yyvsp[-2]->a_int), TO_AWK_LONG(yyvsp[0]->a_int));
		else
			yyvsp[-2]->type = D_range;
		yyval = yyvsp[-2];
	  }
#line 2052 "command.c"
    break;

  case 127: /* opt_integer_list: %empty  */
#line 613 "command.y"
          { yyval = NULL; }
#line 2058 "command.c"
    break;

  case 134: /* exp_list: node  */
#line 627 "command.y"
          { yyval = yyvsp[0]; }
#line 2064 "command.c"
    break;

  case 135: /* exp_list: exp_list ',' node  */
#line 629 "command.y"
          { yyval = yyvsp[-2]; }
#line 2070 "command.c"
    break;

  case 137: /* subscript: '[' exp_list ']'  */
#line 635 "command.y"
          {
		CMDARG *a;
		NODE *subs;
		unsigned count = 0;

		for (a = yyvsp[-1]; a != NULL; a = a->next)
			count++;
		subs = concat_args(yyvsp[-1], count);
		free_cmdarg(yyvsp[-1]->next);
		yyvsp[-1]->next = NULL;
		yyvsp[-1]->type = D_node;
		yyvsp[-1]->a_node = subs;
		yyval = yyvsp[-1];
	  }
#line 2089 "command.c"
    break;

  case 139: /* subscript_list: subscript  */
#line 654 "command.y"
          { yyval = yyvsp[0]; num_dim = 1; }
#line 2095 "command.c"
    break;

  case 140: /* subscript_list: subscript_list subscript  */
#line 656 "command.y"
          {	yyval = yyvsp[-1]; num_dim++; }
#line 2101 "command.c"
    break;

  case 142: /* variable: '$' D_NODE  */
#line 662 "command.y"
          {
		NODE *n = yyvsp[0]->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value for field number"));
		else
			yyvsp[0]->type = D_field;
		yyval = yyvsp[0];
	  }
#line 2114 "command.c"
    break;

  case 143: /* variable: D_VARIABLE subscript_list  */
#line 671 "command.y"
          {
		/* a_string is array name, a_count is dimension count */
		yyvsp[-1]->type = D_subscript;
		yyvsp[-1]->a_count = num_dim;
		yyval = yyvsp[-1];
	  }
#line 2125 "command.c"
    break;

  case 144: /* node: D_NODE  */
#line 681 "command.y"
          { yyval = yyvsp[0]; }
#line 2131 "command.c"
    break;

  case 145: /* node: '+' D_NODE  */
#line 683 "command.y"
          {
		NODE *n = yyvsp[0]->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		yyval = yyvsp[0];
	  }
#line 2142 "command.c"
    break;

  case 146: /* node: '-' D_NODE  */
#line 690 "command.y"
          {
		NODE *n = yyvsp[0]->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		else
			negate_num(n);
		yyval = yyvsp[0];
	  }
#line 2155 "command.c"
    break;

  case 147: /* opt_plus_integer: %empty  */
#line 702 "command.y"
          { yyval = NULL; }
#line 2161 "command.c"
    break;

  case 148: /* opt_plus_integer: plus_integer  */
#line 704 "command.y"
          { yyval = yyvsp[0]; }
#line 2167 "command.c"
    break;

  case 149: /* opt_integer: %empty  */
#line 709 "command.y"
          { yyval = NULL; }
#line 2173 "command.c"
    break;

  case 150: /* opt_integer: integer  */
#line 711 "command.y"
          { yyval = yyvsp[0]; }
#line 2179 "command.c"
    break;

  case 151: /* plus_integer: D_INT  */
#line 716 "command.y"
          {
		if (yyvsp[0]->a_int == 0)
			yyerror(_("non-zero integer value"));
		yyval = yyvsp[0];
	  }
#line 2189 "command.c"
    break;

  case 152: /* plus_integer: '+' D_INT  */
#line 722 "command.y"
          {
		if (yyvsp[0]->a_int == 0)
			yyerror(_("non-zero integer value"));
		yyval = yyvsp[0];
	  }
#line 2199 "command.c"
    break;

  case 153: /* integer: D_INT  */
#line 731 "command.y"
          { yyval = yyvsp[0]; }
#line 2205 "command.c"
    break;

  case 154: /* integer: '+' D_INT  */
#line 733 "command.y"
          { yyval = yyvsp[0]; }
#line 2211 "command.c"
    break;

  case 155: /* integer: '-' D_INT  */
#line 735 "command.y"
          {
		yyvsp[0]->a_int = - yyvsp[0]->a_int;
		yyval = yyvsp[0];
	  }
#line 2220 "command.c"
    break;

  case 156: /* nls: '\n'  */
#line 743 "command.y"
          {
		if (lexptr_begin != NULL) {
			if (input_from_tty && lexptr_begin[0] != '\0')
				add_history(lexptr_begin);
			efree(lexptr_begin);
			lexptr_begin = NULL;
		}
	  }
#line 2233 "command.c"
    break;


#line 2237 "command.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == ZZEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= ZZEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == ZZEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = ZZEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != ZZEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 753 "command.y"



/* append_statement --- append 'stmt' to the list of eval awk statements */

static CMDARG *
append_statement(CMDARG *stmt_list, const char *stmt)
{
	CMDARG *a, *arg;
	char *s;
	size_t len, slen, ssize;

#define EVALSIZE	512

	if (stmt == start_EVAL) {
		len = sizeof(start_EVAL);
		for (a = stmt_list; a != NULL; a = a->next)
			len += strlen(a->a_string) + 1;	/* 1 for ',' */
		len += EVALSIZE;

		emalloc(s, char *, (len + 1) * sizeof(char), "append_statement");
		arg = mk_cmdarg(D_string);
		arg->a_string = s;
		arg->a_count = (unsigned) len;	/* kludge */

		slen = sizeof("function @eval(") - 1;
		memcpy(s, start_EVAL, slen);

		for (a = stmt_list; a != NULL; a = a->next) {
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
	s = stmt_list->a_string;
	slen = strlen(s);
	ssize = stmt_list->a_count;
	if (len > ssize - slen) {
		ssize = slen + len + EVALSIZE;
		erealloc(s, char *, (ssize + 1) * sizeof(char), "append_statement");
		stmt_list->a_string = s;
		stmt_list->a_count = (unsigned) ssize;
	}
	memcpy(s + slen, stmt, len);
	slen += len;
	if (slen >= 2 && s[slen - 2] != '\n') {
		s[slen - 1] = '\n';
		s[slen] = '\0';
	}

	if (stmt == end_EVAL)
		erealloc(stmt_list->a_string, char *, slen + 1, "append_statement");
	return stmt_list;

#undef EVALSIZE
}


/* command names sorted in ascending order */

const struct cmdtoken cmdtab[] = {
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
{ "exit", "", D_quit, D_QUIT, do_quit,
	gettext_noop("exit - (same as quit) exit debugger.") },
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
{ "where", "", D_backtrace, D_BACKTRACE, do_backtrace,
	gettext_noop("where [N] - (same as backtrace) print trace of all or N innermost (outermost if N < 0) frames.") },
{ NULL, NULL, D_illegal, 0, (Func_cmd) 0,
	 NULL },
};

const struct argtoken argtab[] = {
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
	{ NULL, D_illegal, A_NONE },
};


/* get_command --- return command handler function */

Func_cmd
get_command(enum argtype ctype)
{
	const struct cmdtoken *cmd;
	for (cmd = cmdtab; cmd->name != NULL; cmd++) {
		if (cmd->type == ctype)
			return cmd->cf_ptr;
	}
	return (Func_cmd) 0;
}

/* get_command_name --- return command name given it's type */

const char *
get_command_name(enum argtype ctype)
{
	const struct cmdtoken *cmd;
	for (cmd = cmdtab; cmd->name != NULL; cmd++) {
		if (cmd->type == ctype)
			return cmd->name;
	}
	return NULL;
}

/* mk_cmdarg --- make an argument for command */

static CMDARG *
mk_cmdarg(enum argtype type)
{
	CMDARG *arg;
	ezalloc(arg, CMDARG *, sizeof(CMDARG), "mk_cmdarg");
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

ATTRIBUTE_PRINTF(mesg, 1, 2)
static void
yyerror(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	fprintf(out_fp, _("error: "));
	vfprintf(out_fp, mesg, args);
	fprintf(out_fp, "\n");
	va_end(args);
	dbg_errcount++;
	repeat_idx = -1;
}


/* yylex --- read a command and turn it into tokens */

static cmdtok_t
#ifdef USE_EBCDIC
yylex_ebcdic(void)
#else
yylex(void)
#endif
{
	static char *lexptr = NULL;
	static char *lexend;
	int c;
	char *tokstart;
	size_t toklen;

	yylval = (CMDARG *) NULL;

	if (dbg_errcount > 0 && lexptr_begin == NULL) {
		/* fake a new line */
		dbg_errcount = 0;
		return '\n';
	}

	if (lexptr_begin == NULL) {
again:
		lexptr_begin = read_a_line(dbg_prompt);
		if (lexptr_begin == NULL) {	/* EOF or error */
			if (get_eof_status() == EXIT_FATAL)
				exit(EXIT_FATAL);
			if (get_eof_status() == EXIT_FAILURE) {
				static int seen_eof = 0;

				/* force a quit, and let do_quit (in debug.c) exit */
				if (! seen_eof) {
					if (errno != 0)	{
						fprintf(stderr, _("cannot read command: %s\n"), strerror(errno));
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
				d_error(_("cannot read command: %s"), strerror(errno));
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
			return cmdtab[cmd_idx].cls;	/* repeat last command */
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
			if (! is_alpha(c) && ! in_eval) {
				yyerror(_("invalid character in command"));
				return '\n';
			}
			c = *++lexptr;
		}

		toklen = (size_t) (lexptr - tokstart);

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
				arg->a_string = estrdup(lexptr_begin, (size_t) (lexend - lexptr_begin));
				append_cmdarg(arg);
			}
			return cmdtab[cmd_idx].cls;
		} else {
			yyerror(_("unknown command - `%.*s', try help"),
				TO_PRINTF_WIDTH(toklen), tokstart);
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
		bool esc_seen = false;

		toklen = (size_t) (lexend - lexptr);
		emalloc(str, char *, toklen + 1, "yylex");
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
				esc_seen = true;
				if (want_nodeval || c != '"')
					*p++ = '\\';
			}
			if (lexptr == lexend)
				goto err;
			*p++ = (char) c;
		}
		lexptr++;
		*p = '\0';

		if (! want_nodeval) {
			yylval = mk_cmdarg(D_string);
			yylval->a_string = str;
			append_cmdarg(yylval);
			return D_STRING;
		} else {	/* awk string */
			if (esc_seen)
				flags |= SCAN;
			yylval = mk_cmdarg(D_node);
			yylval->a_node = make_str_node(str, (size_t) (p - str), flags);
			append_cmdarg(yylval);
			return D_NODE;
		}
	}

	if (! want_nodeval) {
		while ((c = *++lexptr) != '\0' && c != ':' && c != '-'
					&& c != ' ' && c != '\t' && c != '=')
			;

		/* Is it an integer? */
		if (char_is_digit((unsigned char) tokstart[0]) && cmdtab[cmd_idx].type != D_option) {
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
		yylval->a_string = estrdup(tokstart, (size_t) (lexptr - tokstart));
		append_cmdarg(yylval);
		return D_STRING;
	}

	/* look for awk number */

	if (char_is_digit((unsigned char) tokstart[0])) {
		NODE *r = NULL;

		errno = 0;
#ifdef HAVE_MPFR
		if (do_mpfr) {
			int tval;
			r = mpg_float();
			tval = mpfr_strtofr(r->mpg_numbr, tokstart, & lexptr, 0, ROUND_MODE);
			IEEE_FMT(r->mpg_numbr, tval);
			if (mpfr_integer_p(r->mpg_numbr)) {
				/* integral value, convert to a GMP type. */
				NODE *tmp = r;
				r = mpg_integer();
				mpfr_get_z(r->mpg_i, tmp->mpg_numbr, MPFR_RNDZ);
				unref(tmp);
			}
		} else
#endif
			r = make_number(strtod(tokstart, & lexptr));

		if (errno != 0) {
			yyerror("%s", strerror(errno));
			unref(r);
			errno = 0;
			return '\n';
		}
		yylval = mk_cmdarg(D_node);
		yylval->a_node = r;
		append_cmdarg(yylval);
		return D_NODE;
	}

	c = *lexptr;
	if (c == '$' || c == '@'
			|| c == '[' || c == ']'
			|| c == ',' || c == '=')
		return *lexptr++;

	if (! is_letter(c)) {
		yyerror(_("invalid character"));
		return '\n';
	}

	while (is_identchar(c))
		c = *++lexptr;
	toklen = (size_t) (lexptr - tokstart);

	/* awk variable */
	yylval = mk_cmdarg(D_variable);
	yylval->a_string = estrdup(tokstart, toklen);
	append_cmdarg(yylval);
	return D_VARIABLE;
}

/* Convert single-character tokens coming out of yylex() from EBCDIC to
   ASCII values on-the-fly so that the parse tables need not be regenerated
   for EBCDIC systems.  */
#ifdef USE_EBCDIC
static cmdtok_t
yylex(void)
{
	static char etoa_xlate[256];
	static int do_etoa_init = 1;
	cmdtok_t tok;

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
		do_etoa_init = 0;
	}

	tok = yylex_ebcdic();

	if (tok >= 0 && tok <= 0xFF)
		tok = etoa_xlate[tok];

	return tok;
}
#endif /* USE_EBCDIC */

/* find_argument --- find index in 'argtab' for a command option */

static int
find_argument(CMDARG *arg)
{
	/* non-number argument */
	int idx;
	const char *name, *p;
	size_t len;
	assert(cmd_idx >= 0);
	name = arg->a_string;
	len = strlen(name);
	for (idx = 0; (p = argtab[idx].name) != NULL; idx++) {
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
concat_args(CMDARG *arg, unsigned count)
{
	NODE *n;
	NODE **tmp;
	char *str, *subsep, *p;
	size_t len, subseplen;
	unsigned i;

	if (count == 1) {
		n = force_string(arg->a_node);
		return dupnode(n);
	}

	emalloc(tmp, NODE **, count * sizeof(NODE *), "concat_args");
	subseplen = SUBSEP_node->var_value->stlen;
	subsep = SUBSEP_node->var_value->stptr;
	len = (size_t)0 - subseplen;

	for (i = 0; i < count; i++) {
		n = force_string(arg->a_node);
		len += n->stlen + subseplen;
		tmp[i] = n;
		arg = arg->next;
	}

	emalloc(str, char *, len + 1, "concat_args");
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
	const char *name, *abrv;
	int i, k;
	bool try_exact = true;
	int abrv_match = -1;
	int partial_match = -1;

#ifdef USE_EBCDIC
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
		name = cmdtab[i].name;
		if (try_exact && *token == *name
				&& toklen == strlen(name)
				&& strncmp(name, token, toklen) == 0
		)
			return i;

		if (*name > *token || i == (k - 1))
			try_exact = false;

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

bool
do_help(CMDARG *arg, enum argtype cmd)
{
	int i;
	(void) cmd;

	if (arg == NULL) {
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			for (i = 0; cmdtab[i].name != NULL; i++) {
				gprintf(out_fp, "%s:\n", cmdtab[i].name);
				gprintf(out_fp, "\t%s\n", _(cmdtab[i].help_txt));
			}
		}
	} else if (arg->type == D_string) {
		const char *name;
		name = arg->a_string;
		i = find_command(name, strlen(name));
		if (i >= 0) {
			fprintf(out_fp, "%s\n", cmdtab[i].help_txt);
			if (strcmp(cmdtab[i].name, "option") == 0)
				option_help();
		} else
			fprintf(out_fp, _("undefined command: %s\n"), name);
	}

	return false;
}


#ifdef HAVE_LIBREADLINE

/* next_word --- find the next word in a line to complete
 *               (word seperation characters are space and tab).
 */

static const char *
next_word(const char *p, size_t len, const char **endp)
{
	const char *q;
	size_t i;

	if (p == NULL || len == 0)
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
	const char *cmdtok, *e;
	int idx;
	size_t len;

	(void) end;

	rl_attempted_completion_over = true;	/* no default filename completion please */

	this_cmd = D_illegal;
	len = (size_t) start;
	if ((cmdtok = next_word(rl_line_buffer, len, &e)) == NULL)	/* no first word yet */
		return  rl_completion_matches(text, command_generator);
	len -= (size_t) (e - rl_line_buffer);

	idx = find_command(cmdtok, (size_t) (e - cmdtok));
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
	static unsigned idx = 0;
	const char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}
	while ((name = cmdtab[idx].name) != NULL) {
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
	const char *name;
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
	static unsigned idx;
	const char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}

	if (this_cmd == D_help) {
		while ((name = cmdtab[idx++].name) != NULL) {
			if (strncmp(name, text, textlen) == 0)
				return estrdup(name, strlen(name));
		}
	} else {
		while ((name = argtab[idx].name) != NULL) {
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
	static unsigned idx = 0;
	static NODE *func = NULL;
	static NODE **vars = NULL;
	const char *name;
	NODE *r;

	if (! state) {	/* first time */
		textlen = strlen(text);
		if (vars != NULL)
			efree(vars);
		vars = variable_list();
		idx = 0;
		func = get_function();  /* function in current context */
	}

	/* function params */
	while (func != NULL) {
		if (idx >= func->param_cnt) {
			func = NULL;	/* don't try to match params again */
			idx = 0;
			break;
		}
		name = func->fparms[idx++].vname;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}

	/* globals */
	while ((r = vars[idx++]) != NULL) {
		name = r->vname;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
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

#endif /* HAVE_LIBREADLINE */
