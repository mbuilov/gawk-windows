/*
 * awk.h -- Definitions for gawk.
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

/* ------------------------------ Includes ------------------------------ */

/*
 * config.h absolutely, positively, *M*U*S*T* be included before
 * any system headers.  Otherwise, extreme death, destruction
 * and loss of life results.
 */
#if defined(_TANDEM_SOURCE)
/*
 * config.h forces this even on non-tandem systems but it
 * causes problems elsewhere if used in the check below.
 * so workaround it. bleah.
 */
#define tandem_for_real	1
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(tandem_for_real) && ! defined(_SCO_DS)
#define _XOPEN_SOURCE_EXTENDED 1
#endif

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <setjmp.h>

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#if ! (defined(HAVE_LIBINTL_H) && defined(ENABLE_NLS) && ENABLE_NLS > 0)
#ifndef LOCALEDIR
#define LOCALEDIR NULL
#endif /* LOCALEDIR */
#endif

#if !defined(__SUNPRO_C) && !defined(_MSC_VER)
#if !defined(__STDC__) || __STDC__ < 1
#error "gawk no longer supports non-C89 environments (no __STDC__ or __STDC__ < 1)"
#endif
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#if ! defined(errno)
extern int errno;
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif


/* We can handle multibyte strings.  */
#include <wchar.h>
#include <wctype.h>

#ifdef STDC_HEADERS
#include <float.h>
#endif

#undef CHARBITS
#undef INTBITS

#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif

/* ----------------- System dependencies (with more includes) -----------*/

/* This section is the messiest one in the file, not a lot that can be done */

#ifndef VMS
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#else	/* VMS */
#include <stddef.h>
#include <stat.h>
#include <file.h>	/* avoid <fcntl.h> in io.c */
/* debug.c needs this; when _DECC_V4_SOURCE is defined (as it is
   in our config.h [vms/vms-conf.h]), off_t won't get declared */
# if !defined(__OFF_T) && !defined(_OFF_T)
#  if defined(____OFF_T) || defined(___OFF_T)
typedef __off_t off_t;	/* __off_t is either int or __int64 */
#  else
typedef int off_t;
#  endif
# endif
#endif	/* VMS */

#include "protos.h"

#ifdef HAVE_STRING_H
#include <string.h>
#ifdef NEED_MEMORY_H
#include <memory.h>
#endif	/* NEED_MEMORY_H */
#endif /* HAVE_STRING_H */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif	/* HAVE_STRINGS_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif	/* HAVE_UNISTD_H */

#ifdef VMS
#include <unixlib.h>
#include "vms/redirect.h"
#endif  /*VMS*/

#ifdef HAVE_MPFR
#include <gmp.h>
#include <mpfr.h>
#ifndef MPFR_RNDN
#if !defined(MPFR_VERSION_MAJOR) || MPFR_VERSION_MAJOR < 3
/* for compatibility with MPFR 2.X */
#define MPFR_RNDN GMP_RNDN
#define MPFR_RNDZ GMP_RNDZ
#define MPFR_RNDU GMP_RNDU
#define MPFR_RNDD GMP_RNDD
#endif
#endif
#endif

#include "regex.h"
#include "dfa.h"
typedef struct Regexp {
	struct re_pattern_buffer pat;
	struct re_registers regs;
	struct dfa *dfareg;
	bool has_meta;		/* re has meta chars so (probably) isn't simple string */
	bool maybe_long;	/* re has meta chars that can match long text */
} Regexp;
#define	RESTART(rp,s)	(rp)->regs.start[0]
#define	REEND(rp,s)	(rp)->regs.end[0]
#define	SUBPATSTART(rp,s,n)	(rp)->regs.start[n]
#define	SUBPATEND(rp,s,n)	(rp)->regs.end[n]
#define	NUMSUBPATS(rp,s)	(rp)->regs.num_regs

/* regexp matching flags: */
#define RE_NO_FLAGS	0	/* empty flags */
#define RE_NEED_START	1	/* need to know start/end of match */
#define RE_NO_BOL	2	/* not allowed to match ^ in regexp */

#include "uni_char.h"

#ifdef WINDOWS_NATIVE
/* Should be included before "gawkapi.h", so xstat_t will be defined.  */
#include "mscrtx/xstat.h"
#endif

#include "gawkapi.h"

#ifdef WINDOWS_NATIVE
/* No system headers should be included after "mscrtx/localerpl.h".
   But if you need this in some source file, define NO_LOCALE_RPL
   before including "awk.h" and include "mscrtx/localerpl.h" later.  */
#ifndef NO_LOCALE_RPL
#include "mscrtx/localerpl.h"
#endif
#endif /* WINDOWS_NATIVE */

#ifndef O_BINARY
# ifdef WINDOWS_NATIVE
   /* no "old" (POSIX) names */
#  define O_BINARY	_O_BINARY
# else
#  define O_BINARY	0
# endif
#endif

#ifndef HAVE_SETLOCALE
#define setlocale(locale, val)	/* nothing */
#endif /* HAVE_SETLOCALE */

#ifdef HAVE_MEMCPY_ULONG
extern char *memcpy_ulong(char *dest, const char *src, unsigned long l);
#define memcpy memcpy_ulong
#endif

#ifdef HAVE_MEMSET_ULONG
extern void *memset_ulong(void *dest, int val, unsigned long l);
#define memset memset_ulong
#endif

#ifdef HAVE_FWRITE_UNLOCKED
#define fwrite	fwrite_unlocked
#endif /* HAVE_FWRITE_UNLOCKED */

#if defined(__DJGPP__) || defined(__EMX__) || defined(WINDOWS_NATIVE)
#include "nonposix.h"
#endif /* defined(__DJGPP__) || defined(__EMX__) || defined(WINDOWS_NATIVE) */

#ifdef WINDOWS_NATIVE
/* time_t is 32-bit under MinGW.org for WindowsXP, but we want 64-bit time_t */
typedef __time64_t gawk_time_t;
#define gawk_time(t)		_time64(t)
#define gawk_ctime(t)		_ctime64(t)
#define gawk_mktime(tm)		_mktime64(tm)
#define gawk_gmtime(t)		_gmtime64(t)
#define gawk_localtime(t)	_localtime64(t)
#else /* !WINDOWS_NATIVE */
typedef time_t gawk_time_t;
#define gawk_time(t)		time(t)
#define gawk_ctime(t)		ctime(t)
#define gawk_mktime(tm)		mktime(tm)
#define gawk_gmtime(t)		gmtime(t)
#define gawk_localtime(t)	localtime(t)
#endif /* !WINDOWS_NATIVE */

/* use this as lintwarn("...")
   this is a hack but it gives us the right semantics */
#define lintwarn (*(set_loc(__FILE__, __LINE__),lintfunc))
#define awkwarn  (*(set_loc(__FILE__, __LINE__),r_warning))
#define fatal    (*(set_loc(__FILE__, __LINE__),r_fatal))

#ifdef _PREFAST_
/* only for analysis */
#undef lintwarn
#undef awkwarn
#undef fatal
#define lintwarn r_warning
#define awkwarn r_warning
#define fatal r_fatal
#endif /* _PREFAST_ */

/* No asserts in optimized release build.  */
#ifdef NDEBUG
# ifdef assert
#  undef assert
# endif
# define assert(x) ASSUME(x)
#endif

/* Stuff for losing systems. */
#ifndef HAVE_STRTOD
extern double gawk_strtod();
#define strtod gawk_strtod
#endif

/* ------------------ Constants, Structures, Typedefs  ------------------ */

#define AWKNUM	double

#include "awklong.h" /* awk_long_t/awk_ulong_t */

/* Printf field width/precision format specifier "%*"/"%.*" requires an integer */
#define TO_PRINTF_WIDTH(w) ((int)(w))

enum defrule { UNKRULE = 0, BEGIN, Rule, END, BEGINFILE, ENDFILE,
	MAXRULE /* sentinel, not legal */ };
extern const char *const ruletab[];

typedef enum nodevals {
	/* illegal entry == 0 */
	Node_illegal,

	Node_val,		/* node is a value - type in flags */
	Node_regex,		/* a regexp, text, compiled, flags, etc */
	Node_dynregex,		/* a dynamic regexp */

	/* symbol table values */
	Node_var,		/* scalar variable, lnode is value */
	Node_var_array,		/* array is ptr to elements, table_size num of eles */
	Node_var_new,		/* newly created variable, may become an array */
	Node_param_list,	/* lnode is a variable, rnode is more list */
	Node_func,		/* lnode is param. list, rnode is body */
	Node_ext_func,		/* extension function, code_ptr is builtin code */
	Node_builtin_func,	/* built-in function, main use is for FUNCTAB */

	Node_array_ref,		/* array passed by ref as parameter */
	Node_array_tree,	/* Hashed array tree (HAT) */
	Node_array_leaf,	/* Linear 1-D array */
	Node_dump_array,	/* array info */

	/* program execution -- stack item types */
	Node_arrayfor,
	Node_frame,
	Node_instruction,

	Node_final		/* sentry value, not legal */
} NODETYPE;

struct exp_node;

typedef union bucket_item {
	struct {
		union bucket_item *next;
		char *str;
		size_t len;
		awk_ulong_t code;	/* hash value */
		struct exp_node *name;
		struct exp_node *val;
	} hs;
	struct {
		union bucket_item *next;
		awk_long_t li[2];
		struct exp_node *val[2];
		size_t cnt;	/* <= 2 */
	} hi;
} BUCKET;

enum commenttype {
	EOL_COMMENT = 1,
	BLOCK_COMMENT,
	FOR_COMMENT	// special case
};

/* string hash table */
#define ahnext		hs.next
#define	ahname		hs.name	/* a string index node */
#define	ahname_str	hs.str	/* shallow copy; = ahname->stptr */
#define	ahname_len	hs.len	/* = ahname->stlen */
#define	ahvalue		hs.val
#define	ahcode		hs.code

/* integer hash table */
#define	ainext		hi.next
#define ainum		hi.li	/* integer indices */
#define aivalue		hi.val
#define aicount		hi.cnt

struct exp_instruction;

ATTRIBUTE_PRINTF_TYPEDEF(format, 2, 3)
typedef int (*Func_print)(FILE *file, const char *format, ...);
typedef struct exp_node **(*afunc_t)(struct exp_node *, struct exp_node *);
typedef struct {
	const char *name;
	afunc_t init;
	afunc_t type_of;	/* avoid reserved word typeof */
	afunc_t lookup;
	afunc_t exists;
	afunc_t clear;
	afunc_t a_remove;	/* remove may be defined as macro */
	afunc_t list;
	afunc_t copy;
	afunc_t dump;
	afunc_t store;
} array_funcs_t;

enum reflagvals {
	CONSTANT = 1,
	FS_DFLT  = 2,
};

/* a combination of enum reflagvals */
typedef int reflags_t;

enum flagvals {
/* type = Node_val */
	/*
	 * STRING and NUMBER are mutually exclusive, except for the special
	 * case of an uninitialized value, represented internally by
	 * Nnull_string. They represent the type of a value as assigned.
	 * Nnull_string has both STRING and NUMBER attributes, but all other
	 * scalar values should have precisely one of these bits set.
	 *
	 * STRCUR and NUMCUR are not mutually exclusive. They represent that
	 * the particular type of value is up to date.  For example,
	 *
	 * 	a = 5		# NUMBER | NUMCUR
	 * 	b = a ""	# Adds STRCUR to a, since a string value
	 * 			# is now available. But the type hasn't changed!
	 *
	 * 	a = "42"	# STRING | STRCUR
	 * 	b = a + 0	# Adds NUMCUR to a, since numeric value
	 * 			# is now available. But the type hasn't changed!
	 *
	 * USER_INPUT is the joker.  When STRING|USER_INPUT is set, it means
	 * "this is string data, but the user may have really wanted it to be a
	 * number. If we have to guess, like in a comparison, turn it into a
	 * number if the string is indeed numeric."
	 * For example,    gawk -v a=42 ....
	 * Here, `a' gets STRING|STRCUR|USER_INPUT and then when used where
	 * a number is needed, it gets turned into a NUMBER and STRING
	 * is cleared. In that case, we leave the USER_INPUT in place, so
	 * the combination NUMBER|USER_INPUT means it is a strnum a.k.a. a
	 * "numeric string".
	 *
	 * WSTRCUR is for efficiency. If in a multibyte locale, and we
	 * need to do something character based (substr, length, etc.)
	 * we create the corresponding wide character string and store it,
	 * and add WSTRCUR to the flags so that we don't have to do the
	 * conversion more than once.
	 *
	 * The NUMINT flag may be used with a value of any type -- NUMBER,
	 * STRING, or STRNUM. It indicates that the string representation
	 * equals the result of sprintf("%ld", <numeric value>). So, for
	 * example, NUMINT should NOT be set if it's a strnum or string value
	 * where the string is " 1" or "01" or "+1" or "1.0" or "0.1E1". This
	 * is a hint to indicate that an integer array optimization may be
	 * used when this value appears as a subscript.
	 *
	 * We hope that the rest of the flags are self-explanatory. :-)
	 */
	ZEROFLAGS	= 0,
	MALLOC	= 0x0001,       /* stptr can be free'd, i.e. not a field node pointing into a shared buffer */
	STRING	= 0x0002,       /* assigned as string */
	STRCUR	= 0x0004,       /* string value is current */
	NUMCUR	= 0x0008,       /* numeric value is current */
	NUMBER	= 0x0010,       /* assigned as number */
	USER_INPUT = 0x0020,    /* user input: if NUMERIC then
				 * a NUMBER */
	INTLSTR	= 0x0040,       /* use localized version */
	NUMINT	= 0x0080,       /* numeric value is an integer */
	INTIND	= 0x0100,	/* integral value is array index;
				 * lazy conversion to string.
				 */
	WSTRCUR	= 0x0200,       /* wide str value is current */
	MPFN	= 0x0400,       /* arbitrary-precision floating-point number */
	MPZN	= 0x0800,       /* arbitrary-precision integer */
	NO_EXT_SET = 0x1000,    /* extension cannot set a value for this variable */
	NULL_FIELD = 0x2000,    /* this is the null field */

/* type = Node_var_array */
	ARRAYMAXED	= 0x4000,       /* array is at max size */
	HALFHAT		= 0x8000,       /* half-capacity Hashed Array Tree;
					 * See cint_array.c */
	XARRAY		= 0x10000,
	NUMCONSTSTR	= 0x20000,	/* have string value for numeric constant */
	REGEX           = 0x40000,	/* this is a typed regex */
};

/* Generally, a combination of enum flagvals,
  but can be other values:
  enum pp_push_flag in profile.c */
typedef int node_flags_t;

/*
 * NOTE - this struct is a rather kludgey -- it is packed to minimize
 * space usage, at the expense of cleanliness.  Alter at own risk.
 */
typedef struct exp_node {
	union node_sub_ {
		struct {
			union {
				struct exp_node *lptr;
				struct exp_instruction *li;
				awk_long_t lx;
				awk_ulong_t ulx;
				size_t lsz;
				const array_funcs_t *lp;
			} l;
			union {
				struct exp_node *rptr;
				Regexp *preg[2];
				struct exp_node **av;
				BUCKET **bv;
				void (*uptr)(void);
				struct exp_instruction *iptr;
			} r;
			union {
				struct exp_node *extra;
				void (*aptr)(void);
				unsigned lvl;
				void *cmnt;	// used by pretty printer
			} x;
			char *name;
			struct exp_node *rn;
			size_t reserved;
			size_t cnt;
			union {
				size_t sz;
				reflags_t reflags;
			} s;
		} nodep;

		struct {
#ifdef HAVE_MPFR
			union {
				AWKNUM fltnum;
				mpfr_t mpnum;
				mpz_t mpi;
			} nm;
			int rndmode;
#else
			AWKNUM fltnum;
#endif
			struct exp_node *typre;
			char *sp;
			uni_char_t *wsp;
			size_t slen;
			size_t wslen;
			size_t sref;
			unsigned idx;
			enum commenttype comtype;
		} val;
#ifdef __cplusplus
		node_sub_() {}
#endif
	} sub;
	NODETYPE type;
	node_flags_t flags; // node: also used as counter via 're_cnt'
#ifdef __cplusplus
	struct exp_node &operator=(const struct exp_node &src)
	{
		memcpy((void*) this, &src, sizeof(src));
		return *this;
	}
#endif
} NODE;

#define vname sub.nodep.name

#define lnode	sub.nodep.l.lptr
#define rnode	sub.nodep.r.rptr

/* Node_param_list */
#define dup_ent    sub.nodep.r.rptr

/* Node_param_list, Node_func */
#define param_cnt  sub.nodep.l.lsz

/* Node_func */
#define fparms		sub.nodep.rn
#define code_ptr    sub.nodep.r.iptr

/* Node_regex, Node_dynregex */
#define re_reg	sub.nodep.r.preg
#define re_flags sub.nodep.s.reflags
#define re_text lnode
#define re_exp	sub.nodep.x.extra
#define re_cnt	flags

/* Node_val */
/*
 * Note that the string in stptr may not be NUL-terminated, but it is
 * guaranteed to have at least one extra byte that may be temporarily set
 * to '\0'. This is helpful when calling functions such as strtod that require
 * a NUL-terminated argument. In particular, field values $n for n > 0 and
 * n < NF will not have a NUL terminator, since they point into the $0 buffer.
 * All other strings are NUL-terminated.
 */
#define stptr	sub.val.sp		/* note: may not be NUL-terminated! (and may contain NULs) */
#define stlen	sub.val.slen
#define valref	sub.val.sref
#define stfmt	sub.val.idx
#define strndmode sub.val.rndmode
#define wstptr	sub.val.wsp		/* note: NUL-terminated! (but may contain NULs) */
#define wstlen	sub.val.wslen
#ifdef HAVE_MPFR
#define mpg_numbr	sub.val.nm.mpnum
#define mpg_i		sub.val.nm.mpi
#define numbr		sub.val.nm.fltnum
#else
#define numbr		sub.val.fltnum
#endif
#define typed_re	sub.val.typre

/*
 * If stfmt is set to STFMT_UNUSED, it means that the string representation
 * stored in stptr is not a function of the value of CONVFMT or OFMT. That
 * indicates that either the string value was explicitly assigned, or it
 * was converted from a NUMBER that has an integer value. When stfmt is not
 * set to STFMT_UNUSED, it is an offset into the fmt_list array of distinct
 * CONVFMT and OFMT node pointers.
 */
#define STFMT_UNUSED	((unsigned)-1)

/* Node_arrayfor */
#define for_list	sub.nodep.r.av
#define for_list_size	sub.nodep.s.sz
#define cur_list_idx	sub.nodep.l.lsz
#define for_array 	sub.nodep.rn

/* Node_frame: */
#define stack        sub.nodep.r.av
#define func_node    sub.nodep.x.extra
#define prev_frame_size	sub.nodep.s.sz
#define reti         sub.nodep.l.li

/* Node_var: */
#define var_value    lnode
#define var_update   sub.nodep.r.uptr
#define var_assign   sub.nodep.x.aptr

/* Node_var_array: */
#define buckets		sub.nodep.r.bv
#define nodes		sub.nodep.r.av
#define array_funcs	sub.nodep.l.lp
#define array_base	sub.nodep.l.ulx
#define table_size	sub.nodep.s.sz
#define array_size	sub.nodep.cnt
#define array_capacity	sub.nodep.reserved
#define xarray		sub.nodep.rn
#define parent_array	sub.nodep.x.extra

#define ainit		array_funcs->init
#define atypeof		array_funcs->type_of
#define alookup 	array_funcs->lookup
#define aexists 	array_funcs->exists
#define aclear		array_funcs->clear
#define aremove		array_funcs->a_remove
#define alist		array_funcs->list
#define acopy		array_funcs->copy
#define adump		array_funcs->dump
#define astore		array_funcs->store

/* Node_array_ref: */
#define orig_array lnode
#define prev_array rnode

/* Node_array_print */
#define adepth     sub.nodep.l.lx
#define alevel     sub.nodep.x.lvl

/* Op_comment	*/
#define comment_type	sub.val.comtype

/* --------------------------------lint warning types----------------------------*/
typedef enum lintvals {
	LINT_illegal,
	LINT_assign_in_cond,
	LINT_no_effect
} LINTTYPE;

/* --------------------------------Instruction ---------------------------------- */

typedef enum opcodeval {
	Op_illegal = 0,		/* illegal entry */

	/* binary operators */
	Op_times,
	Op_times_i,
	Op_quotient,
	Op_quotient_i,
	Op_mod,
	Op_mod_i,
	Op_plus,
	Op_plus_i,
	Op_minus,
	Op_minus_i,
	Op_exp,
	Op_exp_i,
	Op_concat,

	/* line range instruction pair */
	Op_line_range,		/* flags for Op_cond_pair */
	Op_cond_pair,		/* conditional pair */

	Op_subscript,
	Op_sub_array,

	/* unary operators */
	Op_preincrement,
	Op_predecrement,
	Op_postincrement,
	Op_postdecrement,
	Op_unary_minus,
	Op_unary_plus,
	Op_field_spec,

	/* unary relationals */
	Op_not,

	/* assignments */
	Op_assign,
	Op_store_var,		/* simple variable assignment optimization */
	Op_store_sub,		/* array[subscript] assignment optimization */
	Op_store_field,  	/* $n assignment optimization */
	Op_assign_times,
	Op_assign_quotient,
	Op_assign_mod,
	Op_assign_plus,
	Op_assign_minus,
	Op_assign_exp,
	Op_assign_concat,

	/* boolean binaries */
	Op_and,			/* a left subexpression in && */
	Op_and_final,		/* right subexpression of && */
	Op_or,
	Op_or_final,

	/* binary relationals */
	Op_equal,
	Op_notequal,
	Op_less,
	Op_greater,
	Op_leq,
	Op_geq,
	Op_match,
	Op_match_rec,		/* match $0 */
	Op_nomatch,

	Op_rule,

	/* keywords */
	Op_K_case,
	Op_K_default,
	Op_K_break,
	Op_K_continue,
	Op_K_print,
	Op_K_print_rec,
	Op_K_printf,
	Op_K_next,
	Op_K_exit,
	Op_K_return,
	Op_K_return_from_eval,
	Op_K_delete,
	Op_K_delete_loop,
	Op_K_getline_redir,
	Op_K_getline,
	Op_K_nextfile,
	Op_K_namespace,

	Op_builtin,
	Op_sub_builtin,		/* sub, gsub and gensub */
	Op_ext_builtin,
	Op_in_array,		/* boolean test of membership in array */

	/* function call instruction */
	Op_func_call,
	Op_indirect_func_call,

	Op_push,		/* scalar variable */
	Op_push_arg,		/* variable type (scalar or array) argument to built-in */
	Op_push_arg_untyped,	/* like Op_push_arg, but for typeof */
	Op_push_i,		/* number, string */
	Op_push_re,		/* regex */
	Op_push_array,
	Op_push_param,
	Op_push_lhs,
	Op_subscript_lhs,
	Op_field_spec_lhs,
	Op_no_op,		/* jump target */
	Op_pop,			/* pop an item from the runtime stack */
	Op_jmp,
	Op_jmp_true,
	Op_jmp_false,
	Op_get_record,
	Op_newfile,
	Op_arrayfor_init,
	Op_arrayfor_incr,
	Op_arrayfor_final,

	Op_var_update,		/* update value of NR, NF or FNR */
	Op_var_assign,
	Op_field_assign,
	Op_subscript_assign,
	Op_after_beginfile,
	Op_after_endfile,

	Op_func,

	Op_comment,		/* for pretty printing */
	Op_exec_count,
	Op_breakpoint,
	Op_lint,
	Op_lint_plus,
	Op_atexit,
	Op_stop,

	/* parsing (yylex and yyparse), should never appear in valid compiled code */
	Op_token,
	Op_symbol,
	Op_list,

	/* program structures -- for use in the profiler/pretty printer */
	Op_K_do,
	Op_K_for,
	Op_K_arrayfor,
	Op_K_while,
	Op_K_switch,
	Op_K_if,
	Op_K_else,
	Op_K_function,
	Op_cond_exp,
	Op_parens,
	Op_final			/* sentry value, not legal */
} OPCODE;

enum redirval {
	/* I/O redirections */
	redirect_none = 0,
	redirect_output,
	redirect_append,
	redirect_pipe,
	redirect_pipein,
	redirect_input,
	redirect_twoway
};

struct break_point;

typedef size_t nargs_t;

typedef struct exp_instruction {
	struct exp_instruction *nexti;
	union exp_instruction_d_ {
		NODE *dn;
		struct exp_instruction *di;
		NODE *(*fptr)(nargs_t nargs);
		awk_value_t *(*efptr)(int num_actual_args,
					awk_value_t *result,
					struct awk_ext_func *finfo);
		LINTTYPE dlt;
		unsigned long long ldl;	// for exec_count
		size_t dsz;
		unsigned dui;
		OPCODE dc;
		enum redirval dt;
		int df;
		char *name;
		const char *sname;
#ifdef __cplusplus
		exp_instruction_d_() {}
#endif
	} d;

	union exp_instruction_x_ {
		bool xb;
		size_t xsz;
		unsigned ln;
		enum defrule xr;
		NODE *xn;
		void (*aptr)(void);
		struct exp_instruction *xi;
		struct break_point *bpt;
		awk_ext_func_t *exf;
#ifdef __cplusplus
		exp_instruction_x_() {}
#endif
	} x;

	struct exp_instruction *comment;
#define IBAD_SRC_LINE ((unsigned short)-1)
	unsigned short source_line;
	unsigned short pool_size;	// memory management in symbol.c
	OPCODE opcode;

#ifdef __cplusplus
	struct exp_instruction &operator=(const struct exp_instruction &src)
	{
		memcpy((void*) this, &src, sizeof(src));
		return *this;
	}
#endif
} INSTRUCTION;

#define func_name       d.name

#define memory          d.dn
#define builtin         d.fptr
#define extfunc         d.efptr
#define builtin_idx     d.dui

#define expr_count      x.xsz

#define c_function	x.exf

#define target_continue d.di
#define target_jmp      d.di
#define target_break    x.xi

/* Op_sub_builtin */
#define sub_flags       d.df
#define GSUB            0x01	/* builtin is gsub */
#define GENSUB          0x02	/* builtin is gensub */
#define LITERAL         0x04	/* target is a literal string */


/* Op_K_exit */
#define target_end      d.di
#define target_atexit   x.xi

/* Op_newfile, Op_K_getline, Op_nextfile */
#define target_endfile	x.xi

/* Op_newfile */
#define target_get_record	x.xi

/* Op_get_record, Op_K_nextfile */
#define target_newfile	d.di

/* Op_K_getline */
#define target_beginfile	d.di

/* Op_get_record */
#define has_endfile		x.xb

/* Op_token */
#define lextok          d.name
#define param_count     x.xsz

/* Op_rule */
#define in_rule         x.xr
#define source_file     d.sname

 /* Op_K_case, Op_K_default */
#define case_stmt       x.xi
#define case_exp        d.di
#define stmt_start      case_exp
#define stmt_end        case_stmt
#define match_exp       x.xb

#define target_stmt     x.xi

/* Op_K_switch */
#define switch_end      x.xi
#define switch_start    d.di

/* Op_K_getline, Op_K_getline_redir */
#define into_var        x.xb

/* Op_K_getline_redir, Op_K_print, Op_K_print_rec, Op_K_printf */
#define redir_type      d.dt

/* Op_arrayfor_incr	*/
#define array_var       x.xn

/* Op_line_range */
#define triggered       x.xb

/* Op_cond_pair */
#define line_range      x.xi

/* Op_func_call, Op_func */
#define func_body       x.xn

/* Op_subscript */
#define sub_count       d.dsz

/* Op_push_lhs, Op_subscript_lhs, Op_field_spec_lhs */
#define do_reference    x.xb

/* Op_list, Op_rule, Op_func */
#define lasti           d.di
#define firsti          x.xi

/* Op_rule, Op_func */
#define last_line       x.ln
#define first_line      source_line

/* Op_lint */
#define lint_type       d.dlt

/* Op_field_spec_lhs */
#define target_assign	d.di

/* Op_var_assign */
#define assign_var	x.aptr

/* Op_var_update */
#define update_var	x.aptr

/* Op_field_assign */
#define field_assign    x.aptr

/* Op_field_assign, Op_var_assign */
#define assign_ctxt	d.dc

/* Op_concat */
#define concat_flag     d.df
#define CSUBSEP		1
#define CSVAR		2

/* Op_breakpoint */
#define break_pt        x.bpt

/*------------------ pretty printing/profiling --------*/
/* Op_exec_count */
#define exec_count      d.ldl

/* Op_K_while */
#define while_body      d.di

/* Op_K_do */
#define doloop_cond     d.di

/* Op_K_for */
#define forloop_cond    d.di
#define forloop_body    x.xi

/* Op_K_if */
#define branch_if       d.di
#define branch_else     x.xi

/* Op_K_else */
#define branch_end      x.xi

/* Op_line_range */
#define condpair_left   d.di
#define condpair_right  x.xi

/* Op_Rule, Op_Func */
#define ns_name		d.name

/* Op_store_var */
#define initval         x.xn

enum iobuf_flags {
	IOP_IS_TTY	= 1,
	IOP_AT_EOF	= 2,
	IOP_CLOSED	= 4,
	IOP_AT_START	= 8,
};

/* a combination of enum iobuf_flags */
typedef int iobuf_flags_t;

typedef struct iobuf {
	struct awk_input_buf_container pubc;	/* exposed to extensions */
#	define publ pubc.input_buf
	char *buf;              /* start data buffer */
	char *off;              /* start of current record in buffer */
	char *dataend;          /* first byte in buffer to hold new data,
				   NULL if not read yet */
	char *end;              /* end of buffer */
	size_t readsize;        /* set from fstat call */
	size_t size;            /* buffer size */
	ssize_t count;          /* amount read last time */
	size_t scanoff;         /* where we were in the buffer when we had
				   to regrow/refill */
	bool valid;
	int errcode;

	iobuf_flags_t flag;
} IOBUF;

typedef void (*Func_ptr)(void);

enum redirect_flags {
	RED_FILE	= 1,
	RED_PIPE	= 2,
	RED_READ	= 4,
	RED_WRITE	= 8,
	RED_APPEND	= 16,
	RED_NOBUF	= 32,
	RED_USED	= 64,	/* closed temporarily to reuse fd */
	RED_EOF		= 128,
	RED_TWOWAY	= 256,
	RED_PTY		= 512,
	RED_SOCKET	= 1024,
	RED_TCP		= 2048,
};

/* a combination of enum redirect_flags */
typedef int redirect_flags_t;

/* structure used to dynamically maintain a linked-list of open files/pipes */
struct redirect {
	redirect_flags_t flag;
	char *value;
	FILE *ifp;	/* input fp, needed for PIPES_SIMULATED */
	IOBUF *iop;
	pid_t pid;
	int status;
	struct redirect *prev;
	struct redirect *next;
	const char *mode;
	awk_output_buf_t output;
};

/* values for BINMODE, used as bit flags */

enum binmode_values {
	TEXT_TRANSLATE	= 0,	/* usual \r\n ---> \n translation */
	BINMODE_INPUT	= 1,	/* no translation for input files */
	BINMODE_OUTPUT	= 2,	/* no translation for output files */
	BINMODE_BOTH	= 3	/* no translation for either */
};

enum srctype {
	SRC_CMDLINE = 1,
	SRC_STDIN,
	SRC_FILE,
	SRC_INC,
	SRC_EXTLIB
};

/*
 * structure for our source, either a command line string or a source file.
 */

typedef struct srcfile {
	struct srcfile *next;
	struct srcfile *prev;

	enum srctype stype;
	char *src;	/* name on command line or include statement */
	char *fullpath;	/* full path after AWKPATH search */
	gawk_time_t mtime;
	gawk_xstat_t sbuf;
	unsigned srclines;	/* no of lines in source */
	size_t bufsize;
	char *buf;
	unsigned *line_offset;	/* offset to the beginning of each line */
	fd_t fd;
	unsigned maxlen;	/* size of the longest line */

	void (*fini_func)(void);	/* dynamic extension of type SRC_EXTLIB */

	char *lexptr;
	char *lexend;
	char *lexeme;
	char *lexptr_begin;
	int lasttok;
	INSTRUCTION *comment;	/* comment on @load line */
	const char *namespc;
} SRCFILE;

struct instruction_mem_pool {
	struct instruction_block *block_list;
	INSTRUCTION *free_space;	// free location in active block
	INSTRUCTION *free_list;
};

// structure for INSTRUCTION pool, needed mainly for debugger
typedef struct instruction_pool {
#define MAX_INSTRUCTION_ALLOC	4	// we don't call bcalloc with more than this
	struct instruction_mem_pool pool[MAX_INSTRUCTION_ALLOC];
} INSTRUCTION_POOL;

/* structure for execution context */
typedef struct context {
	INSTRUCTION_POOL pools;
	NODE symbols;
	INSTRUCTION rule_list;
	SRCFILE srcfiles;
	unsigned sourceline;
	const char *source;
	void (*install_func)(NODE *);
	struct context *prev;
} AWK_CONTEXT;

/* for debugging purposes */
struct flagtab {
	int val;
	const char *name;
};


struct block_item {
	struct block_item *freep;
};

struct block_header {
#ifndef MEMDEBUG
	struct block_item *freep;
#endif
	const char *name;
	size_t size;
	size_t highwater;
#ifdef MEMDEBUG
	size_t active;
#endif
};

enum block_id {
	BLOCK_NODE = 0,
	BLOCK_BUCKET,
	BLOCK_MPFR,
	BLOCK_MPZ,
	BLOCK_MAX	/* count */
};

typedef bool (*Func_pre_exec)(INSTRUCTION **);
typedef void (*Func_post_exec)(INSTRUCTION *);

typedef size_t field_num_t;

#define BAD_NF		((field_num_t)-1)
#define F_UNLIMITED	((field_num_t)-2)
#define SET_NF		(F_UNLIMITED - 1)		/* set NF */

/* -------------------------- External variables -------------------------- */
/* gawk builtin variables */
extern field_num_t NF;
extern field_num_t NR;
extern field_num_t FNR;
extern int BINMODE;
extern bool IGNORECASE;
extern bool RS_is_null;
extern char *OFS;
extern unsigned OFSlen;
extern char *ORS;
extern unsigned ORSlen;
extern char *OFMT;
extern const char *CONVFMT;
extern unsigned CONVFMTidx;
extern unsigned OFMTidx;
#ifdef HAVE_MPFR
extern int MPFR_round_mode;
#endif
extern NODE **fmt_list;
extern char *TEXTDOMAIN;
extern NODE *ARGC_node, *ARGIND_node, *ARGV_node, *BINMODE_node, *CONVFMT_node;
extern NODE *ENVIRON_node, *ERRNO_node, *FIELDWIDTHS_node, *FILENAME_node;
extern NODE *FNR_node, *FPAT_node, *FS_node, *IGNORECASE_node, *LINT_node;
extern NODE *NF_node, *NR_node, *OFMT_node, *OFS_node, *ORS_node, *PROCINFO_node;
extern NODE *RLENGTH_node, *RSTART_node, *RS_node, *RT_node, *SUBSEP_node;
extern NODE *PREC_node, *ROUNDMODE_node;
extern NODE *TEXTDOMAIN_node;
extern NODE *Nnull_string;
extern NODE *Null_field;
extern NODE **fields_arr;
extern unsigned sourceline;
extern const char *source;
extern unsigned errcount;
extern INSTRUCTION *main_beginfile;
extern int (*interpret)(INSTRUCTION *);	/* interpreter routine */
extern NODE *(*make_number)(double);	/* double instead of AWKNUM on purpose */
extern NODE *(*str2number)(NODE *);
extern NODE *(*format_val)(const char *, unsigned, NODE *);
extern int (*cmp_numbers)(const NODE *, const NODE *);

/* built-in array types */
extern const array_funcs_t str_array_func;
extern const array_funcs_t cint_array_func;
extern const array_funcs_t int_array_func;

/* special node used to indicate success in array routines (not NULL) */
extern NODE *success_node;

extern struct block_header nextfree[];
extern bool field0_valid;

extern bool do_itrace;	/* separate so can poke from a debugger */

extern SRCFILE *srcfiles; /* source files */

enum do_flag_values {
	DO_LINT_INVALID	   = 0x00001,	/* only warn about invalid */
	DO_LINT_EXTENSIONS = 0x00002,	/* warn about gawk extensions */
	DO_LINT_ALL	   = 0x00004,	/* warn about all things */
	DO_LINT_OLD	   = 0x00008,	/* warn about stuff not in V7 awk */
	DO_TRADITIONAL	   = 0x00010,	/* no gnu extensions, add traditional weirdnesses */
	DO_POSIX	   = 0x00020,	/* turn off gnu and unix extensions */
	DO_INTL		   = 0x00040,	/* dump locale-izable strings to stdout */
	DO_NON_DEC_DATA	   = 0x00080,	/* allow octal/hex C style DATA. Use with caution! */
	DO_INTERVALS	   = 0x00100,	/* allow {...,...} in regexps, see resetup() */
	DO_PRETTY_PRINT	   = 0x00200,	/* pretty print the program */
	DO_DUMP_VARS	   = 0x00400,	/* dump all global variables at end */
	DO_TIDY_MEM	   = 0x00800,	/* release vars when done */
	DO_SANDBOX	   = 0x01000,	/* sandbox mode - disable 'system' function & redirections */
	DO_PROFILE	   = 0x02000,	/* profile the program */
	DO_DEBUG	   = 0x04000,	/* debug the program */
	DO_MPFR		   = 0x08000,	/* arbitrary-precision floating-point math */
};

/* a combination of enum do_flag_values */
typedef int do_flags_t;

extern do_flags_t do_flags;

#define do_traditional      (do_flags & DO_TRADITIONAL)
#define do_posix            (do_flags & DO_POSIX)
#define do_intl             (do_flags & DO_INTL)
#define do_non_decimal_data (do_flags & DO_NON_DEC_DATA)
#define do_intervals        (do_flags & DO_INTERVALS)
#define do_pretty_print     (do_flags & DO_PRETTY_PRINT)
#define do_profile          (do_flags & DO_PROFILE)
#define do_dump_vars        (do_flags & DO_DUMP_VARS)
#define do_tidy_mem         (do_flags & DO_TIDY_MEM)
#define do_sandbox          (do_flags & DO_SANDBOX)
#define do_debug            (do_flags & DO_DEBUG)
#define do_mpfr             (do_flags & DO_MPFR)

extern bool do_optimize;
extern int use_lc_numeric;
extern int exit_val;

#ifdef NO_LINT
#define do_lint 0
#define do_lint_old 0
#define do_lint_extensions 0
#else
#define do_lint             (do_flags & (DO_LINT_INVALID|DO_LINT_ALL))
#define do_lint_old         (do_flags & DO_LINT_OLD)
#define do_lint_extensions  (do_flags & DO_LINT_EXTENSIONS)
#endif
extern unsigned gawk_mb_cur_max;

#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
extern GETGROUPS_T *groupset;
extern int ngroups;
#endif

#ifdef HAVE_LOCALE_H
extern struct lconv loc;
#endif /* HAVE_LOCALE_H */

#ifdef HAVE_MPFR
extern mpfr_prec_t PRECISION;
extern mpfr_rnd_t ROUND_MODE;
extern mpz_t MNR;
extern mpz_t MFNR;
extern mpz_t mpzval;
extern bool do_ieee_fmt;	/* emulate IEEE 754 floating-point format */
#endif

#ifdef WINDOWS_NATIVE
extern const wchar_t *mywpath;
#endif
extern const char *myname;
extern const char def_strftime_format[];

extern char quote;
extern const char *defpath;
extern const char *deflibpath;
extern char envsep;

extern char casetable[];	/* for case-independent regexp matching */

/* awkgram.y */
extern const char awk_namespace[];	/* "awk" */
extern const char *current_namespace;
extern bool namespace_changed;

/* for debugging */
extern INSTRUCTION *code_block;
extern INSTRUCTION *rule_list;
extern char **d_argv;	/* copy of argv array */
extern FILE *output_fp;
extern const char *command_file;	/* debugger commands */
extern bool output_is_tty;
extern NODE **fcall_list;
extern size_t fcall_count;
extern IOBUF *curfile;
extern bool exiting;

/* for do_print() */
extern NODE **args_array;
extern size_t max_args;

/* for calling interpret() in gawkapi.c */
extern enum defrule currule;

/* ------------------------- Runtime stack -------------------------------- */

typedef union stack_item {
	NODE *rptr;	/* variable etc. */
	NODE **lptr;	/* address of a variable etc. */
} STACK_ITEM;

extern STACK_ITEM *stack_ptr;
extern NODE *frame_ptr;
extern STACK_ITEM *stack_bottom;
extern STACK_ITEM *stack_top;

#define decr_sp()		(stack_ptr--)
#define incr_sp()		((stack_ptr < stack_top) ? ++stack_ptr : grow_stack())
#define stack_adj(n)		(stack_ptr += (n))
#define stack_dec(n)		(stack_ptr -= (n))
#define stack_empty()		(stack_ptr < stack_bottom)

#define POP()			(decr_sp()->rptr)
#define POP_ADDRESS()		(decr_sp()->lptr)
#define PEEK(n)			((stack_ptr - (n))->rptr)
#define TOP()			(stack_ptr->rptr)		/* same as PEEK(0) */
#define TOP_ADDRESS()		(stack_ptr->lptr)
#define PUSH(r)			(void) (incr_sp()->rptr = (r))
#define PUSH_ADDRESS(l)		(void) (incr_sp()->lptr = (l))
#define REPLACE(r)		(void) (stack_ptr->rptr = (r))
#define REPLACE_ADDRESS(l)	(void) (stack_ptr->lptr = (l))

/* function param */
#define GET_PARAM(n)	frame_ptr->stack[n]

/*
 * UPREF --- simplified versions of dupnode, does not handle FIELD node.
 * Most appropriate use is for elements on the runtime stack.
 * When in doubt, use dupnode.
 */

#define UPREF(r)	(void) ((r)->valref++)

extern void r_unref(NODE *tmp);

static inline void
DEREF(NODE *r)
{
	assert(r->valref);
#ifndef GAWKDEBUG
	if (--r->valref)
		return;
#endif
	r_unref(r);
}

#define POP_NUMBER() force_number(POP_SCALAR())
#define TOP_NUMBER() force_number(TOP_SCALAR())

/* ------------------------- Pseudo-functions ------------------------- */
#ifdef HAVE_MPFR

#if 0

/*
 * In principle, there is no need to have both the MPFN and MPZN flags,
 * since we are using 2 bits to encode 1 bit of information. But
 * there may be some minor performance advantages from testing only the
 * node flag bits without needing also to access the global do_mpfr flag bit.
 */
#define numtype_choose(n, mpfrval, mpzval, dblval)	\
 (!do_mpfr ? (dblval) : (((n)->flags & MPFN) ? (mpfrval) : (mpzval)))

#endif

/* N.B. This implementation seems to give the fastest results. */
#define numtype_choose(t, n, mpfrval, mpzval, dblval)	\
 (!((n)->flags & (MPFN|MPZN)) ? (t) (dblval) : ((n)->flags & MPFN) ? (t) (mpfrval) : (t) (mpzval))

static inline awk_ulong_t awk_double_to_ui(double d)
{
	return (awk_ulong_t) d;
}
static inline awk_long_t awk_double_to_si(double d)
{
	return (awk_long_t) d;
}
static inline uintmax_t awk_double_to_uj(double d)
{
	return (uintmax_t) d;
}

/* conversion to C types */
static inline awk_ulong_t get_number_ui(const NODE *n)
{
	awk_ulong_t r = numtype_choose(awk_ulong_t, n,
		mpfr_get_ui((n)->mpg_numbr, ROUND_MODE), mpz_get_ui((n)->mpg_i), awk_double_to_ui((n)->numbr));
	return r;
}
static inline awk_long_t get_number_si(const NODE *n)
{
	awk_long_t r = numtype_choose(awk_long_t, n,
		mpfr_get_si((n)->mpg_numbr, ROUND_MODE), mpz_get_si((n)->mpg_i), awk_double_to_si((n)->numbr));
	return r;
}
static inline double get_number_d(const NODE *n)
{
	double r = numtype_choose(double, n,
		mpfr_get_d((n)->mpg_numbr, ROUND_MODE), mpz_get_d((n)->mpg_i), (n)->numbr);
	return r;
}
static inline uintmax_t get_number_uj(const NODE *n)
{
	uintmax_t r = numtype_choose(uintmax_t, n,
		mpfr_get_uj((n)->mpg_numbr, ROUND_MODE), awk_double_to_uj(mpz_get_d((n)->mpg_i)), awk_double_to_uj((n)->numbr));
	return r;
}

static bool iszero(NODE *n)
{
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
	bool r = numtype_choose(bool, n,
		mpfr_zero_p((n)->mpg_numbr), (mpz_sgn((n)->mpg_i) == 0), ((n)->numbr == 0.0));
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
	return r;
}

#define IEEE_FMT(r, t)		(void) (do_ieee_fmt && format_ieee(r, t))

#define mpg_float()		mpg_node(MPFN)
#define mpg_integer()		mpg_node(MPZN)
#define is_mpg_float(n)		(((n)->flags & MPFN) != 0)
#define is_mpg_integer(n)	(((n)->flags & MPZN) != 0)
#define is_mpg_number(n)	(((n)->flags & (MPZN|MPFN)) != 0)
#else /* !HAVE_MPFR */
#define get_number_ui(n)	((awk_ulong_t) (n)->numbr)
#define get_number_si(n)	((awk_long_t) (n)->numbr)
#define get_number_d(n)		((double) (n)->numbr)
#define get_number_uj(n)	((uintmax_t) (n)->numbr)

#define is_mpg_number(n)	0
#define is_mpg_float(n)		0
#define is_mpg_integer(n)	0

static inline bool iszero(const NODE *n)
{
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
	return n->numbr == 0.0 ? true : false;
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
}
#endif /* !HAVE_MPFR */

static inline field_num_t
get_number_fn(const NODE *n)
{
#ifdef HAVE_MPFR
	return (field_num_t) get_number_ui(n);
#else
	return (field_num_t) n->numbr;
#endif
}

static inline AWKNUM
field_num_as_awknum(field_num_t n)
{
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
	return (AWKNUM) n;
#if defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif
}

#define var_uninitialized(n)	((n)->var_value == Nnull_string)

#define get_lhs(n, r)	 (n)->type == Node_var && ! var_uninitialized(n) ? \
				&((n)->var_value) : r_get_lhs((n), (r))

#define getblock_at(p, id, ty, file, line)	(void) (p = (ty) r_getblock(id, file, line))
#define freeblock(p, id)			(void) (r_freeblock(p, id))

#define getblock(p, id, ty)		getblock_at(p, id, ty, __FILE__, __LINE__)

#define getnode_at(n,file,line)		getblock_at(n, BLOCK_NODE, NODE *, file, line)
#define freenode(n)			freeblock(n, BLOCK_NODE)
#define getnode(n)			getnode_at(n, __FILE__, __LINE__)

static inline NODE *
clearnode(NODE *n)
{
	memset((void*) n, '\0', sizeof(*n));
	return n;
}

#define getbucket_at(b,file,line) 	getblock_at(b, BLOCK_BUCKET, BUCKET *, file, line)
#define freebucket(b)			freeblock(b, BLOCK_BUCKET)
#define getbucket(b)	 		getbucket_at(b, __FILE__, __LINE__)


#define	make_string(s, l)	make_str_node((s), (l), 0)

// Flags for making string nodes
#define		SCAN			1
#define		ALREADY_MALLOCED	2
#define		ELIDE_BACK_NL		4

#define	cant_happen()	r_fatal("internal error line %d, file: %s", \
				__LINE__, __FILE__)

#define	emalloc_at(var,ty,x,str,file,line)	(void) (var = (ty) emalloc_real(x, str, #var, file, line))
#define	ezalloc_at(var,ty,x,str,file,line)	(void) (var = (ty) ezalloc_real(x, str, #var, file, line))
#define	erealloc_at(var,ty,x,str,file,line)	(void) (var = (ty) erealloc_real((void *) var, x, str, #var, file, line))

#define efree(p)	free(p)

#define	emalloc(var,ty,x,str)	emalloc_at(var, ty, x, str, __FILE__, __LINE__)
#define	ezalloc(var,ty,x,str)	ezalloc_at(var, ty, x, str, __FILE__, __LINE__)
#define	erealloc(var,ty,x,str)	erealloc_at(var, ty, x, str, __FILE__, __LINE__)

extern jmp_buf fatal_tag;
extern unsigned fatal_tag_valid;

#define assoc_length(a)	((a)->table_size)
#define assoc_empty(a)	(!assoc_length(a))

static inline NODE **
assoc_lookup(NODE *arr, NODE *sub)
{
	return arr->alookup(arr, sub);
}

/* assoc_clear --- flush all the values in symbol[] */
static inline void
assoc_clear(NODE *arr)
{
	(void) arr->aclear(arr, NULL);
}

/* assoc_remove --- remove an index from symbol[] */
static inline bool
assoc_remove(NODE *arr, NODE *sub)
{
	return arr->aremove(arr, sub) != NULL;
}


/* ------------- Function prototypes or defs (as appropriate) ------------- */

/* array.c */
typedef enum { SORTED_IN = 1, ASORT, ASORTI } sort_context_t;
enum assoc_kind_vals {
	ANONE   = 0x00,		/* "unused" value */
	AINDEX	= 0x001,	/* list of indices */
	AVALUE	= 0x002,	/* list of values */
	AINUM	= 0x004,	/* numeric index */
	AISTR	= 0x008,	/* string index */
	AVNUM	= 0x010,	/* numeric scalar value */
	AVSTR	= 0x020,	/* string scalar value */
	AASC	= 0x040,	/* ascending order */
	ADESC	= 0x080,	/* descending order */
	ADELETE = 0x100		/* need a single index; for use in do_delete_loop */
};

/* a combination of enum assoc_kind_vals */
typedef int assoc_kind_t;

extern NODE *make_array(void);
extern void null_array(NODE *symbol);
extern NODE *force_array(NODE *symbol, bool canfatal);
extern const char *make_aname(const NODE *symbol);
extern const char *array_vname(const NODE *symbol);
extern void array_init(void);
extern NODE **null_afunc(NODE *symbol, NODE *subs);
extern void set_SUBSEP(void);
extern NODE *concat_exp(nargs_t nargs, bool do_subsep);
extern NODE *assoc_copy(NODE *symbol, NODE *newsymb);
extern void assoc_dump(NODE *symbol, NODE *p);
extern NODE **assoc_list(NODE *symbol, const char *sort_str, sort_context_t sort_ctxt);
extern void assoc_info(NODE *subs, NODE *val, NODE *p, const char *aname);
extern void do_delete(NODE *symbol, size_t nsubs);
extern void do_delete_loop(NODE *symbol, NODE **lhs);
extern NODE *do_adump(nargs_t nargs);
extern NODE *do_aoption(nargs_t nargs);
extern NODE *do_asort(nargs_t nargs);
extern NODE *do_asorti(nargs_t nargs);
extern size_t (*hash)(const char *s, size_t len, size_t hsize, awk_ulong_t *code);
extern void init_env_array(NODE *env_node);
extern void init_argv_array(NODE *argv_node, NODE *shadow_node);
extern void indent(unsigned indent_level);

/* awkgram.c */
extern NODE *variable(unsigned location, char *name, NODETYPE type);
extern bool parse_program(INSTRUCTION **pcode, bool from_eval);
extern void track_ext_func(const char *name);
extern void dump_funcs(void);
extern void dump_vars(const char *fname);
extern const char *getfname(NODE *(*)(nargs_t), bool prepend_awk);
extern NODE *stopme(nargs_t nargs);
extern void shadow_funcs(void);
extern int check_special(const char *name);
extern SRCFILE *add_srcfile(enum srctype stype, const char *src, SRCFILE *curr, bool *already_included, int *errcode);
extern void free_srcfile(SRCFILE *thisfile);
extern bool files_are_same(const char *path, SRCFILE *src);
extern void valinfo(NODE *n, Func_print print_func, FILE *fp);
extern void negate_num(NODE *n);

/* function implementing a built-in */
#ifdef __cplusplus
typedef union {
	NODE *(*nargs_fn)(nargs_t nargs);
	NODE *(*nargs_x_fn)(nargs_t nargs, int i);
} builtin_func_t;
#define builtin_func_is_null(f)             ((f).nargs_fn == NULL)
#define builtin_func_is_nargs_fn(f, fn)     ((f).nargs_fn == (fn))
#define builtin_func_is_nargs_x_fn(f, x_fn) ((f).nargs_x_fn == (x_fn))
#define builtin_func_to_nargs_fn(f)         ((f).nargs_fn)
#define builtin_func_to_nargs_x_fn(f)       ((f).nargs_x_fn)
static inline builtin_func_t builtin_func_from_nargs_fn(NODE *(*nargs_fn)(nargs_t))
{
	builtin_func_t f;
	f.nargs_fn = nargs_fn;
	return f;
}
static inline builtin_func_t builtin_func_from_nargs_x_fn(NODE *(*nargs_x_fn)(nargs_t, int))
{
	builtin_func_t f;
	f.nargs_x_fn = nargs_x_fn;
	return f;
}
static inline builtin_func_t builtin_func_null_(void)
{
	builtin_func_t f = { NULL };
	return f;
}
#define BUILTIN_FUNC_NULL builtin_func_null_()
#else /* !__cplusplus */
typedef void *(*builtin_func_t)(nargs_t nargs);
static inline NODE *(*builtin_func_to_nargs_fn(builtin_func_t f))(nargs_t)
{
	return (NODE*(*)(nargs_t))f;
}
static inline builtin_func_t builtin_func_from_nargs_fn(NODE *(*nargs_fn)(nargs_t))
{
	return (void*(*)(nargs_t))nargs_fn;
}
#if defined __GNUC__ && __GNUC__ > 8 - (__GNUC_MINOR__ >= 1)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static inline NODE *(*builtin_func_to_nargs_x_fn(builtin_func_t f))(nargs_t, int)
{
	return (NODE*(*)(nargs_t, int))f;
}
static inline builtin_func_t builtin_func_from_nargs_x_fn(NODE *(*nargs_x_fn)(nargs_t, int))
{
	return (void*(*)(nargs_t))nargs_x_fn;
}
#if defined __GNUC__ && __GNUC__ > 8 - (__GNUC_MINOR__ >= 1)
#pragma GCC diagnostic pop
#endif
static inline bool builtin_func_is_nargs_fn(builtin_func_t f, NODE *(*nargs_fn)(nargs_t))
{
	return builtin_func_to_nargs_fn(f) == nargs_fn;
}
static inline bool builtin_func_is_nargs_x_fn(builtin_func_t f, NODE *(*nargs_x_fn)(nargs_t, int))
{
	return builtin_func_to_nargs_x_fn(f) == nargs_x_fn;
}
#define BUILTIN_FUNC_NULL	((builtin_func_t)NULL)
#define builtin_func_is_null(f)	((f) == BUILTIN_FUNC_NULL)
#endif /* !__cplusplus */

extern builtin_func_t lookup_builtin(const char *name);
extern void install_builtins(void);
extern bool is_alpha(int c);
extern bool is_alnum(int c);
extern bool is_letter(int c);
extern bool is_identchar(int c);
extern NODE *make_regnode(NODETYPE type, NODE *exp);
extern bool validate_qualified_name(const char *token);

/* builtin.c */
extern double double_to_int(double d);
extern NODE *do_exp(nargs_t nargs);
extern NODE *do_fflush(nargs_t nargs);
extern NODE *do_index(nargs_t nargs);
extern NODE *do_int(nargs_t nargs);
extern NODE *do_isarray(nargs_t nargs);
extern NODE *do_length(nargs_t nargs);
extern NODE *do_log(nargs_t nargs);
extern NODE *do_mktime(nargs_t nargs);
extern NODE *do_sprintf(nargs_t nargs);
extern void do_printf(nargs_t nargs, enum redirval redirtype);
extern void print_simple(NODE *tree, FILE *fp);
extern NODE *do_sqrt(nargs_t nargs);
extern NODE *do_substr(nargs_t nargs);
extern NODE *do_strftime(nargs_t nargs);
extern NODE *do_systime(nargs_t nargs);
extern NODE *do_system(nargs_t nargs);
extern void do_print(nargs_t nargs, enum redirval redirtype);
extern void do_print_rec(nargs_t nargs, enum redirval redirtype);
extern NODE *do_tolower(nargs_t nargs);
extern NODE *do_toupper(nargs_t nargs);
extern NODE *do_atan2(nargs_t nargs);
extern NODE *do_sin(nargs_t nargs);
extern NODE *do_cos(nargs_t nargs);
extern NODE *do_rand(nargs_t nargs);
extern NODE *do_srand(nargs_t nargs);
extern NODE *do_match(nargs_t nargs);
extern NODE *do_sub(nargs_t nargs, int flags);
extern NODE *call_sub(const char *name, nargs_t nargs);
extern NODE *call_match(nargs_t nargs);
extern NODE *call_split_func(const char *name, nargs_t nargs);
extern NODE *format_tree(const char *, size_t, NODE **, size_t);
extern NODE *do_lshift(nargs_t nargs);
extern NODE *do_rshift(nargs_t nargs);
extern NODE *do_and(nargs_t nargs);
extern NODE *do_or(nargs_t nargs);
extern NODE *do_xor(nargs_t nargs);
extern NODE *do_compl(nargs_t nargs);
extern NODE *do_strtonum(nargs_t nargs);
extern AWKNUM nondec2awknum(char *str, size_t len, char **endptr);
extern NODE *do_dcgettext(nargs_t nargs);
extern NODE *do_dcngettext(nargs_t nargs);
extern NODE *do_bindtextdomain(nargs_t nargs);
extern NODE *do_intdiv(nargs_t nargs);
extern NODE *do_typeof(nargs_t nargs);
extern int strncasecmpmbs(const unsigned char *,
			  const unsigned char *, size_t);
extern int sanitize_exit_status(int status);
ATTRIBUTE_PRINTF(msg, 3, 0)
extern void check_symtab_functab(NODE *dest, const char *fname, const char *msg);

/* debug.c */
extern void init_debug(void);
extern int debug_prog(INSTRUCTION *pc);
extern void frame_popped(void);

/* eval.c */
extern void PUSH_CODE(INSTRUCTION *cp);
extern INSTRUCTION *POP_CODE(void);
extern void init_interpret(void);
extern int cmp_nodes(NODE *t1, NODE *t2, bool use_strcmp);
extern int cmp_awknums(const NODE *t1, const NODE *t2);
extern void set_IGNORECASE(void);
extern void set_OFS(void);
extern void set_ORS(void);
extern void set_OFMT(void);
extern void set_CONVFMT(void);
extern void set_BINMODE(void);
extern void set_LINT(void);
extern void set_TEXTDOMAIN(void);
extern void update_ERRNO_int(int);
extern void update_ERRNO_string(const char *string);
extern void unset_ERRNO(void);
extern void update_NR(void);
extern void update_NF(void);
extern void update_FNR(void);
extern const char *redflags2str(redirect_flags_t);
extern const char *flags2str(node_flags_t);
extern const char *genflags2str(int flagval, const struct flagtab *tab);
extern const char *nodetype2str(NODETYPE type);
extern void load_casetable(void);
extern AWKNUM calc_exp(AWKNUM x1, AWKNUM x2);
extern const char *opcode2str(OPCODE type);
extern const char *op2str(OPCODE type);
extern NODE **r_get_lhs(NODE *n, bool reference);
extern STACK_ITEM *grow_stack(void);
extern void dump_fcall_stack(FILE *fp);
extern bool register_exec_hook(Func_pre_exec preh, Func_post_exec posth);
extern NODE **r_get_field(NODE *n, Func_ptr *assign, bool reference);
extern INSTRUCTION *unwind_stack(size_t n);

/* ext.c */
extern NODE *do_ext(nargs_t nargs);
void load_ext(const char *lib_name);	/* temporary */
extern void close_extensions(void);
extern bool is_valid_identifier(const char *name);
#ifdef DYNAMIC
extern awk_bool_t make_builtin(const char *name_space, awk_ext_func_t *);
extern NODE *get_argument(size_t);
extern NODE *get_actual_argument(NODE *, size_t, bool);
#define get_scalar_argument(n, i)  get_actual_argument((n), (i), false)
#define get_array_argument(n, i)   get_actual_argument((n), (i), true)
#endif

/* field.c */
extern void init_fields(void);
extern void set_record(const char *buf, size_t cnt, const awk_fieldwidth_info_t *);
extern void reset_record(void);
extern void rebuild_record(void);
extern void set_NF(void);
extern NODE **get_field(field_num_t num, Func_ptr *assign);
extern NODE *do_split(nargs_t nargs);
extern NODE *do_patsplit(nargs_t nargs);
extern void set_FS(void);
extern void set_RS(void);
extern void set_FIELDWIDTHS(void);
extern void set_FPAT(void);
extern void update_PROCINFO_str(const char *subscript, const char *str);
extern void update_PROCINFO_num(const char *subscript, AWKNUM val);

typedef enum {
	Using_FS,
	Using_FIELDWIDTHS,
	Using_FPAT,
	Using_API
} field_sep_type;
extern field_sep_type current_field_sep(void);
extern const char *current_field_sep_str(void);

/* gawkapi.c */
extern gawk_api_t api_impl;
extern void init_ext_api(void);
extern void update_ext_api(void);
extern NODE *awk_value_to_node(const awk_value_t *);
extern void run_ext_exit_handlers(int exitval);
extern void print_ext_versions(void);
extern void free_api_string_copies(void);

/* gawkmisc.c */
extern const char *gawk_name(const char *filespec);
extern void os_arg_fixup(int *argcp, char ***argvp);
extern fd_t os_devopen(const char *name, int flag);
extern void os_close_on_exec(fd_t fd, const char *name, const char *what, const char *dir);
extern bool os_isatty(fd_t fd);
extern bool os_isdir(fd_t fd);
extern bool os_isreadable(const awk_input_buf_t *iobuf, bool *isdir);
extern bool os_is_setuid(void);
extern int os_setbinmode(fd_t fd, int mode);
extern void os_restore_mode(fd_t fd);
#ifdef WINDOWS_NATIVE
extern int os_fstat(int fd, gawk_stat_t *buf);
#define os_stat(path, buf)	_stat64(path, buf)
extern int os_xfstat(int fd, gawk_xstat_t *buf);
#define os_xstat(path, buf)	xstat(path, buf)
#else
#define os_fstat(fd, buf)	fstat(fd, buf)
#define os_stat(path, buf)	stat(path, buf)
#define os_xfstat(fd, buf)	fstat(fd, buf)
#define os_xstat(path, buf)	stat(path, buf)
#endif
extern int os_sleep(unsigned mills);
extern size_t optimal_bufsize(fd_t fd, gawk_xstat_t *sbuf);
extern bool ispath(const char *file);
extern bool isdirpunct(int c);
#ifdef WINDOWS_NATIVE
struct wide_arg;
extern char **convert_wargv(int argc, struct wide_arg *wargs);
extern void gawk_cleanup(void);
#else
#define gawk_cleanup() ((void)0)
#endif
#ifdef WINDOWS_NATIVE
/* same as timegm, but tm values should be in the default ranges */
extern __time64_t gawk_timegm(struct tm *);
#else
#define gawk_timegm	timegm
#endif

/* io.c */
extern void init_sockets(void);
extern void init_io(void);
extern void register_input_parser(awk_input_parser_t *input_parser);
extern void register_output_wrapper(awk_output_wrapper_t *wrapper);
extern void register_two_way_processor(awk_two_way_processor_t *processor);
extern void set_FNR(void);
extern void set_NR(void);
extern size_t gawk_fwrite(const void *buf, size_t size, size_t nmemb, awk_output_buf_t *outbuf);
extern int gawk_fflush(awk_output_buf_t *outbuf);
extern int gawk_ferror(awk_output_buf_t *outbuf);

#if defined __cplusplus && defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif

extern struct redirect *redirect(NODE *redir_exp, enum redirval redirtype, int *errflg, bool failure_fatal);

#if defined __cplusplus && defined __GNUC__ && __GNUC__ > 4 - (__GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

extern struct redirect *redirect_string(const char *redir_exp_str,
		size_t redir_exp_len, bool not_string_flag, enum redirval redirtype,
		int *errflg, fd_t extfd, bool failure_fatal);
extern NODE *do_close(nargs_t nargs);
extern int flush_io(void);
extern int close_io(bool *stdio_problem, bool *got_EPIPE);
typedef enum { CLOSE_ALL, CLOSE_TO, CLOSE_FROM } two_way_close_type;
extern int close_rp(struct redirect *rp, two_way_close_type how);
extern int devopen_simple(const char *name, const char *mode, bool try_real_open);
extern fd_t devopen(const char *name, const char *mode);
extern fd_t srcopen(SRCFILE *s);
extern char *find_source(const char *src, gawk_xstat_t *stb, int *errcode, bool is_extlib);
extern NODE *do_getline_redir(bool intovar, enum redirval redirtype);
extern NODE *do_getline(bool intovar, IOBUF *iop);
extern struct redirect *getredirect(const char *str, size_t len);
extern bool inrec(IOBUF *iop, int *errcode);
extern awk_long_t nextfile(IOBUF **cur, bool skipping);
extern bool is_non_fatal_std(FILE *fp);
extern bool is_non_fatal_redirect(const char *str, size_t len);
extern void ignore_sigpipe(void);
extern void set_sigpipe_to_default(void);
extern bool non_fatal_flush_std_file(FILE *fp);
#ifdef WINDOWS_NATIVE
extern ssize_t read_wrap(fd_t fd, void *buf, size_t size);
#else
#define read_wrap	((ssize_t(*)(fd_t, void*, size_t)) read)
#endif

/* main.c */
extern const char *get_spec_varname(Func_ptr fptr);
extern bool arg_assign(char *arg, bool initing);
extern bool is_std_var(const char *var);
extern bool is_off_limits_var(const char *var);
extern char *estrdup(const char *str, size_t len);
extern void update_global_values(void);
extern awk_long_t getenv_long(const char *name);
extern void after_beginfile(IOBUF **cur);
extern void set_current_namespace(const char *new_namespace);

/* mpfr.c */
extern void set_PREC(void);
extern void set_ROUNDMODE(void);
extern void mpfr_unset(NODE *n);
#ifdef HAVE_MPFR
extern int mpg_cmp(const NODE *, const NODE *);
extern int format_ieee(mpfr_ptr, int);
extern NODE *mpg_update_var(NODE *);
extern field_num_t mpg_set_var(NODE *);
extern NODE *do_mpfr_and(nargs_t nargs);
extern NODE *do_mpfr_atan2(nargs_t nargs);
extern NODE *do_mpfr_compl(nargs_t nargs);
extern NODE *do_mpfr_cos(nargs_t nargs);
extern NODE *do_mpfr_exp(nargs_t nargs);
extern NODE *do_mpfr_int(nargs_t nargs);
extern NODE *do_mpfr_intdiv(nargs_t nargs);
extern NODE *do_mpfr_log(nargs_t nargs);
extern NODE *do_mpfr_lshift(nargs_t nargs);
extern NODE *do_mpfr_or(nargs_t nargs);
extern NODE *do_mpfr_rand(nargs_t nargs);
extern NODE *do_mpfr_rshift(nargs_t nargs);
extern NODE *do_mpfr_sin(nargs_t nargs);
extern NODE *do_mpfr_sqrt(nargs_t nargs);
extern NODE *do_mpfr_srand(nargs_t nargs);
extern NODE *do_mpfr_strtonum(nargs_t nargs);
extern NODE *do_mpfr_xor(nargs_t nargs);
extern void init_mpfr(mpfr_prec_t, const char *);
extern void cleanup_mpfr(void);
extern NODE *mpg_node(enum flagvals);
extern const char *mpg_fmt(const char *, ...);
extern int mpg_strtoui(mpz_ptr, char *, size_t, char **, int);
extern void mpg_zero(NODE *n);
#endif

/* msg.c */
ATTRIBUTE_NORETURN extern void gawk_exit(int status);
ATTRIBUTE_NORETURN extern void final_exit(int status);
ATTRIBUTE_PRINTF(emsg, 3, 0) extern void err(bool isfatal, const char *s, const char *emsg, va_list argp);
ATTRIBUTE_PRINTF(mesg, 1, 2) extern void msg(const char *mesg, ...);
ATTRIBUTE_PRINTF(mesg, 1, 2) extern void error(const char *mesg, ...);
ATTRIBUTE_PRINTF(mesg, 1, 2) extern void r_warning(const char *mesg, ...);
extern void set_loc(const char *file, unsigned line);
ATTRIBUTE_NORETURN
ATTRIBUTE_PRINTF(mesg, 1, 2) extern void r_fatal(const char *mesg, ...);
ATTRIBUTE_PRINTF_PTR(mesg, 1, 2) extern void (*lintfunc)(const char *mesg, ...);
/* profile.c */
extern void init_profiling_signals(void);
extern void set_prof_file(const char *filename);
extern void dump_prog(INSTRUCTION *code);
extern char *pp_number(NODE *n);
extern char *pp_string(const char *in_str, size_t len, int delim);
extern char *pp_node(NODE *n);
extern int pp_func(INSTRUCTION *pc, void *);
extern void pp_string_fp(Func_print print_func, FILE *fp, const char *str,
		size_t namelen, int delim, bool breaklines);
extern const char *redir2str(enum redirval redirtype);

/* node.c */
extern NODE *r_force_number(NODE *n);
extern NODE *r_format_val(const char *format, unsigned index, NODE *s);
extern NODE *r_dupnode(NODE *n);
extern NODE *make_str_node(const char *s, size_t len, int flags);
extern NODE *make_typed_regex(const char *re, size_t len);
extern struct block_item *more_blocks(enum block_id id);
extern int parse_escape(const char **string_ptr);
extern NODE *str2wstr(NODE *n, size_t **ptr);
extern NODE *wstr2str(NODE *n);
#define force_wstring(n)	str2wstr(n, NULL)
extern const uni_char_t *wstrstr(const uni_char_t *haystack, size_t hs_len,
		const uni_char_t *needle, size_t needle_len);
extern const uni_char_t *wcasestrstr(const uni_char_t *haystack, size_t hs_len,
		const uni_char_t *needle, size_t needle_len);
extern void r_free_wstr(NODE *n);
#define free_wstr(n)	do { if ((n)->flags & WSTRCUR) r_free_wstr(n); } while(0)
extern wint_t btowc_cache[];
#define btowc_cache(x) btowc_cache[(x)&0xFF]
extern void init_btowc_cache(void);
#define is_valid_character(b)	(btowc_cache[(b)&0xFF] != WEOF)
extern bool out_of_range(NODE *n);
extern char *format_nan_inf(NODE *n, char format);
#ifdef MEMDEBUG
extern void *r_getblock(enum block_id id, const char *file, unsigned line);
extern void r_freeblock(void *, enum block_id id);
#else /* !MEMDEBUG */
static inline void *
r_getblock(enum block_id id, const char *file, unsigned line)
{
	struct block_item *p;
	(void) file, (void) line;
	p = nextfree[id].freep;
	if (p != NULL)
		nextfree[id].freep = p->freep;
	else
		p = more_blocks(id);
	return p;
}
static inline void
r_freeblock(void *ptr, enum block_id id)
{
	struct block_item *p = (struct block_item*) ptr;
	p->freep = nextfree[id].freep;
	nextfree[id].freep = p;
}
#endif /* !MEMDEBUG */

/* re.c */
extern Regexp *make_regexp(const char *s, size_t len, bool ignorecase, bool dfa, bool canfatal);
extern regoff_t research(Regexp *rp, char *str, int start, size_t len, int flags);
extern void refree(Regexp *rp);
extern void reg_error(const char *s);
extern Regexp *re_update(NODE *t);
extern void resyntax(int syntax);
extern void resetup(void);
extern bool reisstring(const char *text, size_t len, Regexp *re, const char *buf);
extern unsigned get_numbase(const char *str, size_t len, bool use_locale);
extern bool using_utf8(void);

/* symbol.c */
extern void load_symbols(void);
extern void init_symbol_table(void);
extern NODE *symbol_table;
extern NODE *func_table;
extern NODE *install_symbol(const char *name, NODETYPE type);
extern NODE *remove_symbol(NODE *r);
extern void destroy_symbol(NODE *r);
extern void release_symbols(NODE *symlist, int keep_globals);
extern void append_symbol(NODE *r);
extern NODE *lookup(const char *name);
extern NODE *make_params(char **pnames, size_t pcount);
extern void install_params(NODE *func);
extern void remove_params(NODE *func);
extern void release_all_vars(void);
extern int foreach_func(NODE **table, int (*)(INSTRUCTION *, void *), void *);
extern INSTRUCTION *bcalloc(OPCODE op, unsigned size, unsigned srcline);
extern void bcfree(INSTRUCTION *);
extern AWK_CONTEXT *new_context(void);
extern void push_context(AWK_CONTEXT *ctxt);
extern void pop_context(void);
extern bool in_main_context(void);
extern void free_context(AWK_CONTEXT *ctxt, bool keep_globals);
extern NODE **variable_list(void);
extern NODE **function_list(bool sort);
extern void print_vars(NODE **table, Func_print print_func, FILE *fp);
extern bool check_param_names(void);
extern bool is_all_upper(const char *name);
extern bool cmp_keyword(const char name[], const char keyword[], unsigned len);

/* floatcomp.c */
#ifdef HAVE_UINTMAX_T
extern uintmax_t adjust_uint(uintmax_t n);
#else
#define adjust_uint(n) (n)
#endif

/* int_array.c */
extern AWKNUM int_kilobytes(NODE *symbol);

/* str_array.c */
extern NODE **is_integer(NODE *symbol, NODE *subs);
extern AWKNUM str_kilobytes(NODE *symbol);

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
#if defined(VMS)
#define WEXITSTATUS(stat_val) (stat_val)
#else /* ! defined(VMS) */
#define WEXITSTATUS(stat_val) ((((unsigned) (stat_val)) >> 8) & 0xFF)
#endif /* ! defined(VMS)) */
#endif /* WEXITSTATUS */

/* For z/OS, from Dave Pitts. EXIT_FAILURE is normally 8, make it 1. */
#if defined(EXIT_FAILURE) && EXIT_FAILURE == 8
# undef EXIT_FAILURE
#endif

/* EXIT_SUCCESS and EXIT_FAILURE normally come from <stdlib.h> */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif
/* EXIT_FATAL is specific to gawk, not part of Standard C */
#ifndef EXIT_FATAL
# define EXIT_FATAL   2
#endif

/* ------------------ Inline Functions ------------------ */

/*
 * These must come last to get all the function declarations and
 * macro definitions before their bodies.
 *
 * This is wasteful if the compiler doesn't support inline. We won't
 * worry about it until someone complains.
 */

/* POP_ARRAY --- get the array at the top of the stack */

static inline NODE *
POP_ARRAY(bool check_for_untyped)
{
	NODE *t = POP();
	static bool warned = false;

	if (do_lint && ! warned && check_for_untyped && t->type == Node_var_new) {
		warned = true;
		lintwarn(_("behavior of `for' loop on untyped variable is not defined by POSIX"));
	}

	return (t->type == Node_var_array) ? t : force_array(t, true);
}

/* POP_PARAM --- get the top parameter, array or scalar */

static inline NODE *
POP_PARAM(void)
{
	NODE *t = POP();

	return (t->type == Node_var_array) ? t : force_array(t, false);
}

/* POP_SCALAR --- pop the scalar at the top of the stack */

static inline NODE *
POP_SCALAR(void)
{
	NODE *t = POP();

	if (t->type == Node_var_array)
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(t));

	return t;
}

/* TOP_SCALAR --- get the scalar at the top of the stack */

static inline NODE *
TOP_SCALAR(void)
{
	NODE *t = TOP();

	if (t->type == Node_var_array)
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(t));

	return t;
}

/* POP_STRING --- pop the string at the top of the stack */
#define POP_STRING()	force_string(POP_SCALAR())

/* TOP_STRING --- get the string at the top of the stack */
#define TOP_STRING()	force_string(TOP_SCALAR())

/* in_array --- return pointer to element in array if there */

static inline NODE *
in_array(NODE *a, NODE *s)
{
	NODE **ret;

	ret = a->aexists(a, s);

	return ret ? *ret : NULL;
}

#ifdef GAWKDEBUG
#define dupnode	r_dupnode
#else
/* dupnode --- up the reference on a node */

static inline NODE *
dupnode(NODE *n)
{
	if ((n->flags & MALLOC) != 0) {
		n->valref++;
		return n;
	}
	return r_dupnode(n);
}
#endif

/*
 * force_string_fmt --- force a node to have a string value in a given format.
 * The string representation of a number may change due to whether it was most
 * recently rendered with CONVFMT or OFMT, or due to changes in the CONVFMT
 * and OFMT values. But if the value entered gawk as a string or strnum, then
 * stfmt should be set to STFMT_UNUSED, and the string representation should
 * not change.
 *
 * Additional twist: If ROUNDMODE changed at some point we have to
 * recompute also.
 */

static inline NODE *
force_string_fmt(NODE *s, const char *fmtstr, unsigned fmtidx)
{
	if ((s->flags & STRCUR) != 0
		&& (s->stfmt == STFMT_UNUSED || (s->stfmt == fmtidx
#ifdef HAVE_MPFR
						&& s->strndmode == MPFR_round_mode
#endif
				)))
		return s;
	return format_val(fmtstr, fmtidx, s);
}

/* conceptually should be force_string_convfmt, but this is the typical case */
#define force_string(s)		force_string_fmt((s), CONVFMT, CONVFMTidx)

#define force_string_ofmt(s)	force_string_fmt((s), OFMT, OFMTidx)

#ifdef GAWKDEBUG
#define unref	r_unref
#define	force_number	str2number
#else /* not GAWKDEBUG */

/* unref --- decrease the reference count and/or free a node */

static inline void
unref(NODE *r)
{
	assert(r == NULL || r->valref);
	if (r != NULL && !--r->valref)
		r_unref(r);
}

/* force_number --- force a  node to have a numeric value */

static inline NODE *
force_number(NODE *n)
{
	return (n->flags & NUMCUR) != 0 ? n : str2number(n);
}

#endif /* GAWKDEBUG */


/* fixtype --- make a node decide if it's a number or a string */

/*
 * In certain contexts, the true type of a scalar value matters, and we
 * must ascertain whether it is a NUMBER or a STRING. In such situations,
 * please use this function to resolve the type.
 *
 * It is safe to assume that the return value will be the same NODE,
 * since force_number on a USER_INPUT should always return the same NODE,
 * and force_string on an INTIND should as well.
 */

static inline NODE *
fixtype(NODE *n)
{
	assert(n && n->type == Node_val);
	if ((n->flags & (NUMCUR|USER_INPUT)) == USER_INPUT)
		return force_number(n);
	if ((n->flags & INTIND) != 0)
		return force_string(n);
	return n;
}

/* boolval --- return true/false based on awk's criteria */

/*
 * In awk, a value is considered to be true if it is nonzero _or_
 * non-null. Otherwise, the value is false.
 */

static inline bool
boolval(NODE *t)
{
	(void) fixtype(t);
	if ((t->flags & NUMBER) != 0)
		return ! iszero(t);
	return (t->stlen > 0);
}

/* emalloc_real --- malloc with error checking */

static inline void *
emalloc_real(size_t count, const char *where, const char *var, const char *file, unsigned line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%u: emalloc called with zero bytes", file, line);

	ret = (void *) malloc(count);
	if (ret == NULL)
		fatal(_("%s:%u:%s: %s: cannot allocate %" ZUFMT " bytes of memory: %s"),
			file, line, where, var, count, strerror(errno));

	return ret;
}

/* ezalloc_real --- malloc zero-filled bytes with error checking */

static inline void *
ezalloc_real(size_t count, const char *where, const char *var, const char *file, unsigned line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%u: ezalloc called with zero bytes", file, line);

	ret = (void *) calloc(1, count);
	if (ret == NULL)
		fatal(_("%s:%u:%s: %s: cannot allocate %" ZUFMT " bytes of memory: %s"),
			file, line, where, var, count, strerror(errno));

	return ret;
}

/* erealloc_real --- realloc with error checking */

static inline void *
erealloc_real(void *ptr, size_t count, const char *where, const char *var, const char *file, unsigned line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%u: erealloc called with zero bytes", file, line);

	ret = (void *) realloc(ptr, count);
	if (ret == NULL)
		fatal(_("%s:%u:%s: %s: cannot reallocate %" ZUFMT " bytes of memory: %s"),
			file, line, where, var, count, strerror(errno));

	return ret;
}

/* make_number_node --- make node with the given flags */

#define make_number_node(flags) make_number_node_real(flags, __FILE__, __LINE__)

static inline NODE *
make_number_node_real(enum flagvals flags, const char *file, unsigned line)
{
	NODE *r;
	getnode_at(r, file, line);
	clearnode(r);
	r->type = Node_val;
	r->valref = 1u;
	r->flags = ((node_flags_t) flags|MALLOC|NUMBER|NUMCUR);
	return r;
}

/* assoc_set -- set an element in an array. Does unref(sub)! */

static inline void
assoc_set(NODE *array, NODE *sub, NODE *value)
{

	NODE **lhs = assoc_lookup(array, sub);
	unref(*lhs);
	*lhs = value;
	if (array->astore != NULL)
		(*array->astore)(array, sub);
	unref(sub);
}

/*
 * str_terminate_f, str_terminate, str_restore: function and macros to
 * reduce chances of typos when terminating and restoring strings.
 * This also helps to enforce that the NODE must be in scope when we restore.
 */

static inline void
str_terminate_f(NODE *n, char *savep)
{
	*savep = n->stptr[n->stlen];
	n->stptr[n->stlen] = '\0';
}

#define str_terminate(n, save) str_terminate_f((n), &save)
#define str_restore(n, save) (n)->stptr[(n)->stlen] = save

/* charp_const_cast --- cast (const char *) to (char *) */

static inline char *
charp_const_cast(const char *str)
{
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_CAST_QUAL
	return (char*) str;
PRAGMA_WARNING_POP
}

#ifdef SIGPIPE
#define ignore_sigpipe() signal(SIGPIPE, SIG_IGN)
#define set_sigpipe_to_default() signal(SIGPIPE, SIG_DFL)
#define die_via_sigpipe() (signal(SIGPIPE, SIG_DFL), kill(getpid(), SIGPIPE))
#else	/* !SIGPIPE */
#define ignore_sigpipe()
#define set_sigpipe_to_default()
#ifdef WINDOWS_NATIVE
/* 0xC0000008 is EXCEPTION_INVALID_HANDLE, somewhat appropriate for EPIPE */
#define die_via_sigpipe() exit((int)0xC0000008)
#else	/* !WINDOWS_NATIVE */
#define die_via_sigpipe() exit(EXIT_FATAL)
#endif	/* !WINDOWS_NATIVE */
#endif	/* !SIGPIPE */


/* char_digit_value --- return [0..9] if char looks like a decimal digit,
   otherwise return -1.  */

static inline int
char_digit_value(int c)
{
#if \
	'0' + 1 == '1' && '1' + 1 == '2' && '2' + 1 == '3' && \
	'3' + 1 == '4' && '4' + 1 == '5' && '5' + 1 == '6' && \
	'6' + 1 == '7' && '7' + 1 == '8' && '8' + 1 == '9'
	return '0' <= c && c <= '9' ? c - '0' : -1;
#else
	switch (c) {
	case '0': return 0; case '1': return 1; case '2': return 2;
	case '3': return 3; case '4': return 4; case '5': return 5;
	case '6': return 6; case '7': return 7; case '8': return 8;
	case '9': return 9;
	default: return -1;
	}
#endif
}

/* char_xdigit_value --- return [0..15] if char looks like a hexadecimal digit,
   otherwise return -1.  */

static inline int
char_xdigit_value(int c)
{
#if \
	'0' + 1 == '1' && '1' + 1 == '2' && '2' + 1 == '3' && \
	'3' + 1 == '4' && '4' + 1 == '5' && '5' + 1 == '6' && \
	'6' + 1 == '7' && '7' + 1 == '8' && '8' + 1 == '9' && \
	'a' + 1 == 'b' && 'b' + 1 == 'c' && 'c' + 1 == 'd' && \
	'd' + 1 == 'e' && 'e' + 1 == 'f' && \
	'A' + 1 == 'B' && 'B' + 1 == 'C' && 'C' + 1 == 'D' && \
	'D' + 1 == 'E' && 'E' + 1 == 'F'
	return
		'0' <= c && c <= '9' ? c - '0' :
		'a' <= c && c <= 'f' ? c - 'a' + 10 :
		'A' <= c && c <= 'F' ? c - 'A' + 10 : -1;
#else
	switch (c) {
	case '0': return 0; case '1': return 1; case '2': return 2;
	case '3': return 3; case '4': return 4; case '5': return 5;
	case '6': return 6; case '7': return 7; case '8': return 8;
	case '9': return 9;
	case 'a': case 'A': return 10;
	case 'b': case 'B': return 11;
	case 'c': case 'C': return 12;
	case 'd': case 'D': return 13;
	case 'e': case 'E': return 14;
	case 'f': case 'F': return 15;
	default: return -1;
	}
#endif
}

/* locale-independent replacement of isdigit() */
#define char_is_digit(c)	(char_digit_value(c) != -1)

/* locale-independent replacement of isxdigit() */
#define char_is_xdigit(c)	(char_xdigit_value(c) != -1)
