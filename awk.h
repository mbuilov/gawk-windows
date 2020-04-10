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

#if !defined(__SUNPRO_C)
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
#endif	/* not STDC_HEADERS */


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

#if HAVE_UNISTD_H
#include <unistd.h>
#endif	/* HAVE_UNISTD_H */

#ifdef VMS
#include <unixlib.h>
#include "vms/redirect.h"
#endif  /*VMS*/

#ifndef O_BINARY
#define O_BINARY	0
#endif

#ifndef HAVE_SETLOCALE
#define setlocale(locale, val)	/* nothing */
#endif /* HAVE_SETLOCALE */

#if HAVE_MEMCPY_ULONG
extern char *memcpy_ulong(char *dest, const char *src, unsigned long l);
#define memcpy memcpy_ulong
#endif
#if HAVE_MEMSET_ULONG
extern void *memset_ulong(void *dest, int val, unsigned long l);
#define memset memset_ulong
#endif

#ifdef HAVE_FWRITE_UNLOCKED
#define fwrite	fwrite_unlocked
#endif /* HAVE_FWRITE_UNLOCKED */

#if defined(__DJGPP__) || defined(__EMX__) || defined(__MINGW32__)
#include "nonposix.h"
#endif /* defined(__DJGPP__) || defined(__EMX__) || defined(__MINGW32__) */

/* use this as lintwarn("...")
   this is a hack but it gives us the right semantics */
#define lintwarn (*(set_loc(__FILE__, __LINE__),lintfunc))
/* same thing for warning */
#define warning (*(set_loc(__FILE__, __LINE__),r_warning))

#ifdef HAVE_MPFR
#include <gmp.h>
#include <mpfr.h>
#ifndef MPFR_RNDN
/* for compatibility with MPFR 2.X */
#define MPFR_RNDN GMP_RNDN
#define MPFR_RNDZ GMP_RNDZ
#define MPFR_RNDU GMP_RNDU
#define MPFR_RNDD GMP_RNDD
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

#include "gawkapi.h"

/* Stuff for losing systems. */
#if !defined(HAVE_STRTOD)
extern double gawk_strtod();
#define strtod gawk_strtod
#endif

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
# define __attribute__(arg)
#endif

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif /* ATTRIBUTE_UNUSED */

#ifndef ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
#endif /* ATTRIBUTE_NORETURN */

#ifndef ATTRIBUTE_PRINTF
#define ATTRIBUTE_PRINTF(m, n) __attribute__ ((__format__ (__printf__, m, n)))
#define ATTRIBUTE_PRINTF_1 ATTRIBUTE_PRINTF(1, 2)
#define ATTRIBUTE_PRINTF_2 ATTRIBUTE_PRINTF(2, 3)
#endif /* ATTRIBUTE_PRINTF */

/* ------------------ Constants, Structures, Typedefs  ------------------ */

#define AWKNUM	double

enum defrule { BEGIN = 1, Rule, END, BEGINFILE, ENDFILE,
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
		size_t code;
		struct exp_node *name;
		struct exp_node *val;
	} hs;
	struct {
		union bucket_item *next;
		long li[2];
		struct exp_node *val[2];
		size_t cnt;
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

typedef int (*Func_print)(FILE *, const char *, ...);
typedef struct exp_node **(*afunc_t)(struct exp_node *, struct exp_node *);
typedef struct {
	const char *name;
	afunc_t init;
	afunc_t type_of;	/* avoid reserved word typeof */
	afunc_t lookup;
	afunc_t exists;
	afunc_t clear;
	afunc_t remove;
	afunc_t list;
	afunc_t copy;
	afunc_t dump;
	afunc_t store;
} array_funcs_t;

/*
 * NOTE - this struct is a rather kludgey -- it is packed to minimize
 * space usage, at the expense of cleanliness.  Alter at own risk.
 */
typedef struct exp_node {
	union {
		struct {
			union {
				struct exp_node *lptr;
				struct exp_instruction *li;
				long ll;
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
				long xl;
				void *cmnt;	// used by pretty printer
			} x;
			char *name;
			size_t reserved;
			struct exp_node *rn;
			unsigned long cnt;
			unsigned long reflags;
#				define	CONSTANT	1
#				define	FS_DFLT		2
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
			char *sp;
			size_t slen;
			long sref;
			int idx;
			wchar_t *wsp;
			size_t wslen;
			struct exp_node *typre;
			enum commenttype comtype;
		} val;
	} sub;
	NODETYPE type;
	unsigned int flags;

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
#		define	MALLOC	0x0001       /* stptr can be free'd, i.e. not a field node pointing into a shared buffer */
#		define	STRING	0x0002       /* assigned as string */
#		define	STRCUR	0x0004       /* string value is current */
#		define	NUMCUR	0x0008       /* numeric value is current */
#		define	NUMBER	0x0010       /* assigned as number */
#		define	USER_INPUT 0x0020    /* user input: if NUMERIC then
		                              * a NUMBER */
#		define	INTLSTR	0x0040       /* use localized version */
#		define	NUMINT	0x0080       /* numeric value is an integer */
#		define	INTIND	0x0100	     /* integral value is array index;
		                              * lazy conversion to string.
		                              */
#		define	WSTRCUR	0x0200       /* wide str value is current */
#		define	MPFN	0x0400       /* arbitrary-precision floating-point number */
#		define	MPZN	0x0800       /* arbitrary-precision integer */
#		define	NO_EXT_SET 0x1000    /* extension cannot set a value for this variable */
#		define	NULL_FIELD 0x2000    /* this is the null field */

/* type = Node_var_array */
#		define	ARRAYMAXED	0x4000       /* array is at max size */
#		define	HALFHAT		0x8000       /* half-capacity Hashed Array Tree;
		                                      * See cint_array.c */
#		define	XARRAY		0x10000
#		define	NUMCONSTSTR	0x20000	/* have string value for numeric constant */
#		define  REGEX           0x40000 /* this is a typed regex */
} NODE;

#define vname sub.nodep.name

#define lnode	sub.nodep.l.lptr
#define rnode	sub.nodep.r.rptr

/* Node_param_list */
#define param      vname
#define dup_ent    sub.nodep.r.rptr

/* Node_param_list, Node_func */
#define param_cnt  sub.nodep.l.ll

/* Node_func */
#define fparms		sub.nodep.rn
#define code_ptr    sub.nodep.r.iptr

/* Node_regex, Node_dynregex */
#define re_reg	sub.nodep.r.preg
#define re_flags sub.nodep.reflags
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
#define stptr	sub.val.sp
#define stlen	sub.val.slen
#define valref	sub.val.sref
#define stfmt	sub.val.idx
#define strndmode sub.val.rndmode
#define wstptr	sub.val.wsp
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
#define STFMT_UNUSED	-1

/* Node_arrayfor */
#define for_list	sub.nodep.r.av
#define for_list_size	sub.nodep.reflags
#define cur_idx		sub.nodep.l.ll
#define for_array 	sub.nodep.rn

/* Node_frame: */
#define stack        sub.nodep.r.av
#define func_node    sub.nodep.x.extra
#define prev_frame_size	sub.nodep.reflags
#define reti         sub.nodep.l.li

/* Node_var: */
#define var_value    lnode
#define var_update   sub.nodep.r.uptr
#define var_assign   sub.nodep.x.aptr

/* Node_var_array: */
#define buckets		sub.nodep.r.bv
#define nodes		sub.nodep.r.av
#define array_funcs	sub.nodep.l.lp
#define array_base	sub.nodep.l.ll
#define table_size	sub.nodep.reflags
#define array_size	sub.nodep.cnt
#define array_capacity	sub.nodep.reserved
#define xarray		sub.nodep.rn
#define parent_array	sub.nodep.x.extra

#define ainit		array_funcs->init
#define atypeof		array_funcs->type_of
#define alookup 	array_funcs->lookup
#define aexists 	array_funcs->exists
#define aclear		array_funcs->clear
#define aremove		array_funcs->remove
#define alist		array_funcs->list
#define acopy		array_funcs->copy
#define adump		array_funcs->dump
#define astore		array_funcs->store

/* Node_array_ref: */
#define orig_array lnode
#define prev_array rnode

/* Node_array_print */
#define adepth     sub.nodep.l.ll
#define alevel     sub.nodep.x.xl

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

typedef struct exp_instruction {
	struct exp_instruction *nexti;
	union {
		NODE *dn;
		struct exp_instruction *di;
		NODE *(*fptr)(int);
		awk_value_t *(*efptr)(int num_actual_args,
					awk_value_t *result,
					struct awk_ext_func *finfo);
		long dl;
		char *name;
	} d;

	union {
		long  xl;
		NODE *xn;
		void (*aptr)(void);
		struct exp_instruction *xi;
		struct break_point *bpt;
		awk_ext_func_t *exf;
	} x;

	struct exp_instruction *comment;
	short source_line;
	short pool_size;	// memory management in symbol.c
	OPCODE opcode;
} INSTRUCTION;

#define func_name       d.name

#define memory          d.dn
#define builtin         d.fptr
#define extfunc         d.efptr
#define builtin_idx     d.dl

#define expr_count      x.xl

#define c_function	x.exf

#define target_continue d.di
#define target_jmp      d.di
#define target_break    x.xi

/* Op_sub_builtin */
#define sub_flags       d.dl
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
#define has_endfile		x.xl

/* Op_token */
#define lextok          d.name
#define param_count     x.xl

/* Op_rule */
#define in_rule         x.xl
#define source_file     d.name

 /* Op_K_case, Op_K_default */
#define case_stmt       x.xi
#define case_exp        d.di
#define stmt_start      case_exp
#define stmt_end        case_stmt
#define match_exp       x.xl

#define target_stmt     x.xi

/* Op_K_switch */
#define switch_end      x.xi
#define switch_start    d.di

/* Op_K_getline, Op_K_getline_redir */
#define into_var        x.xl

/* Op_K_getline_redir, Op_K_print, Op_K_print_rec, Op_K_printf */
#define redir_type      d.dl

/* Op_arrayfor_incr	*/
#define array_var       x.xn

/* Op_line_range */
#define triggered       x.xl

/* Op_cond_pair */
#define line_range      x.xi

/* Op_func_call, Op_func */
#define func_body       x.xn

/* Op_subscript */
#define sub_count       d.dl

/* Op_push_lhs, Op_subscript_lhs, Op_field_spec_lhs */
#define do_reference    x.xl

/* Op_list, Op_rule, Op_func */
#define lasti           d.di
#define firsti          x.xi

/* Op_rule, Op_func */
#define last_line       x.xl
#define first_line      source_line

/* Op_lint */
#define lint_type       d.dl

/* Op_field_spec_lhs */
#define target_assign	d.di

/* Op_var_assign */
#define assign_var	x.aptr

/* Op_var_update */
#define update_var	x.aptr

/* Op_field_assign */
#define field_assign    x.aptr

/* Op_field_assign, Op_var_assign */
#define assign_ctxt	d.dl

/* Op_concat */
#define concat_flag     d.dl
#define CSUBSEP		1
#define CSVAR		2

/* Op_breakpoint */
#define break_pt        x.bpt

/*------------------ pretty printing/profiling --------*/
/* Op_exec_count */
#define exec_count      d.dl

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

typedef struct iobuf {
	awk_input_buf_t public;	/* exposed to extensions */
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

	int flag;
#		define	IOP_IS_TTY	1
#		define  IOP_AT_EOF      2
#		define  IOP_CLOSED      4
#		define  IOP_AT_START    8
} IOBUF;

typedef void (*Func_ptr)(void);

/* structure used to dynamically maintain a linked-list of open files/pipes */
struct redirect {
	unsigned int flag;
#		define	RED_FILE	1
#		define	RED_PIPE	2
#		define	RED_READ	4
#		define	RED_WRITE	8
#		define	RED_APPEND	16
#		define	RED_NOBUF	32
#		define	RED_USED	64	/* closed temporarily to reuse fd */
#		define	RED_EOF		128
#		define	RED_TWOWAY	256
#		define	RED_PTY		512
#		define	RED_SOCKET	1024
#		define	RED_TCP		2048
	char *value;
	FILE *ifp;	/* input fp, needed for PIPES_SIMULATED */
	IOBUF *iop;
	int pid;
	int status;
	struct redirect *prev;
	struct redirect *next;
	const char *mode;
	awk_output_buf_t output;
};

/* values for BINMODE, used as bit flags */

enum binmode_values {
	TEXT_TRANSLATE = 0,	/* usual \r\n ---> \n translation */
	BINMODE_INPUT = 1,	/* no translation for input files */
	BINMODE_OUTPUT = 2,	/* no translation for output files */
	BINMODE_BOTH = 3	/* no translation for either */
};

/*
 * structure for our source, either a command line string or a source file.
 */

typedef struct srcfile {
	struct srcfile *next;
	struct srcfile *prev;

	enum srctype {
		SRC_CMDLINE = 1,
		SRC_STDIN,
		SRC_FILE,
		SRC_INC,
		SRC_EXTLIB
	} stype;
	char *src;	/* name on command line or include statement */
	char *fullpath;	/* full path after AWKPATH search */
	time_t mtime;
	struct stat sbuf;
	int srclines;	/* no of lines in source */
	size_t bufsize;
	char *buf;
	int *line_offset;	/* offset to the beginning of each line */
	int fd;
	int maxlen;	/* size of the longest line */

	void (*fini_func)();	/* dynamic extension of type SRC_EXTLIB */

	char *lexptr;
	char *lexend;
	char *lexeme;
	char *lexptr_begin;
	int lasttok;
	INSTRUCTION *comment;	/* comment on @load line */
	const char *namespace;
} SRCFILE;

// structure for INSTRUCTION pool, needed mainly for debugger
typedef struct instruction_pool {
#define MAX_INSTRUCTION_ALLOC	4	// we don't call bcalloc with more than this
	struct instruction_mem_pool {
		struct instruction_block *block_list;
		INSTRUCTION *free_space;	// free location in active block
		INSTRUCTION *free_list;
	} pool[MAX_INSTRUCTION_ALLOC];
} INSTRUCTION_POOL;

/* structure for execution context */
typedef struct context {
	INSTRUCTION_POOL pools;
	NODE symbols;
	INSTRUCTION rule_list;
	SRCFILE srcfiles;
	int sourceline;
	char *source;
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
	struct block_item *freep;
	size_t size;
	const char *name;
	long highwater;
#ifdef MEMDEBUG
	long active;
#endif
};

enum block_id {
	BLOCK_NODE = 0,
	BLOCK_BUCKET,
	BLOCK_MPFR,
	BLOCK_MPZ,
	BLOCK_MAX	/* count */
};

typedef int (*Func_pre_exec)(INSTRUCTION **);
typedef void (*Func_post_exec)(INSTRUCTION *);

#ifndef LONG_MAX
#define LONG_MAX ((long)(~(1L << (sizeof (long) * 8 - 1))))
#endif
#ifndef ULONG_MAX
#define ULONG_MAX (~(unsigned long)0)
#endif
#ifndef LONG_MIN
#define LONG_MIN ((long)(-LONG_MAX - 1L))
#endif
#define UNLIMITED    LONG_MAX

/* -------------------------- External variables -------------------------- */
/* gawk builtin variables */
extern long NF;
extern long NR;
extern long FNR;
extern int BINMODE;
extern bool IGNORECASE;
extern bool RS_is_null;
extern char *OFS;
extern int OFSlen;
extern char *ORS;
extern int ORSlen;
extern char *OFMT;
extern char *CONVFMT;
extern int CONVFMTidx;
extern int OFMTidx;
#ifdef HAVE_MPFR
extern int MPFR_round_mode;
#endif
extern char *TEXTDOMAIN;
extern NODE *BINMODE_node, *CONVFMT_node, *FIELDWIDTHS_node, *FILENAME_node;
extern NODE *FNR_node, *FS_node, *IGNORECASE_node, *NF_node;
extern NODE *NR_node, *OFMT_node, *OFS_node, *ORS_node, *RLENGTH_node;
extern NODE *RSTART_node, *RS_node, *RT_node, *SUBSEP_node, *PROCINFO_node;
extern NODE *LINT_node, *ERRNO_node, *TEXTDOMAIN_node, *FPAT_node;
extern NODE *PREC_node, *ROUNDMODE_node;
extern NODE *Nnull_string;
extern NODE *Null_field;
extern NODE **fields_arr;
extern int sourceline;
extern char *source;
extern int errcount;
extern int (*interpret)(INSTRUCTION *);	/* interpreter routine */
extern NODE *(*make_number)(double);	/* double instead of AWKNUM on purpose */
extern NODE *(*str2number)(NODE *);
extern NODE *(*format_val)(const char *, int, NODE *);
extern int (*cmp_numbers)(const NODE *, const NODE *);

/* built-in array types */
extern const array_funcs_t str_array_func;
extern const array_funcs_t cint_array_func;
extern const array_funcs_t int_array_func;

/* special node used to indicate success in array routines (not NULL) */
extern NODE *success_node;

extern struct block_header nextfree[];
extern bool field0_valid;

extern int do_flags;

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
	DO_MPFR		   = 0x08000	/* arbitrary-precision floating-point math */
};

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
extern int gawk_mb_cur_max;

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


extern const char *myname;
extern const char def_strftime_format[];

extern char quote;
extern char *defpath;
extern char *deflibpath;
extern char envsep;

extern char casetable[];	/* for case-independent regexp matching */

extern const char awk_namespace[];	/* "awk" */
extern const char *current_namespace;
extern bool namespace_changed;

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
	assert(r->valref > 0);
	if (--r->valref == 0)
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
#define numtype_choose(n, mpfrval, mpzval, dblval)	\
 (!((n)->flags & (MPFN|MPZN)) ? (dblval) : (((n)->flags & MPFN) ? (mpfrval) : (mpzval)))

/* conversion to C types */
#define get_number_ui(n)	numtype_choose((n), mpfr_get_ui((n)->mpg_numbr, ROUND_MODE), mpz_get_ui((n)->mpg_i), (unsigned long) (n)->numbr)

#define get_number_si(n)	numtype_choose((n), mpfr_get_si((n)->mpg_numbr, ROUND_MODE), mpz_get_si((n)->mpg_i), (long) (n)->numbr)

#define get_number_d(n)		numtype_choose((n), mpfr_get_d((n)->mpg_numbr, ROUND_MODE), mpz_get_d((n)->mpg_i), (double) (n)->numbr)

#define get_number_uj(n)	numtype_choose((n), mpfr_get_uj((n)->mpg_numbr, ROUND_MODE), (uintmax_t) mpz_get_d((n)->mpg_i), (uintmax_t) (n)->numbr)

#define iszero(n)		numtype_choose((n), mpfr_zero_p((n)->mpg_numbr), (mpz_sgn((n)->mpg_i) == 0), ((n)->numbr == 0.0))

#define IEEE_FMT(r, t)		(void) (do_ieee_fmt && format_ieee(r, t))

#define mpg_float()		mpg_node(MPFN)
#define mpg_integer()		mpg_node(MPZN)
#define is_mpg_float(n)		(((n)->flags & MPFN) != 0)
#define is_mpg_integer(n)	(((n)->flags & MPZN) != 0)
#define is_mpg_number(n)	(((n)->flags & (MPZN|MPFN)) != 0)
#else
#define get_number_ui(n)	(unsigned long) (n)->numbr
#define get_number_si(n)	(long) (n)->numbr
#define get_number_d(n)		(double) (n)->numbr
#define get_number_uj(n)	(uintmax_t) (n)->numbr

#define is_mpg_number(n)	0
#define is_mpg_float(n)		0
#define is_mpg_integer(n)	0
#define iszero(n)		((n)->numbr == 0.0)
#endif

#define var_uninitialized(n)	((n)->var_value == Nnull_string)

#define get_lhs(n, r)	 (n)->type == Node_var && ! var_uninitialized(n) ? \
				&((n)->var_value) : r_get_lhs((n), (r))

#ifdef MEMDEBUG

extern void *r_getblock(int id);
extern void r_freeblock(void *, int id);
#define getblock(p, id, ty)	(void) (p = r_getblock(id))
#define freeblock(p, id)	(void) (r_freeblock(p, id))

#else /* MEMDEBUG */

#define getblock(p, id, ty)  (void) ((p = (ty) nextfree[id].freep) ? \
			(ty) (nextfree[id].freep = ((struct block_item *) p)->freep) \
			: (p = (ty) more_blocks(id)))
#define freeblock(p, id)	 (void) (((struct block_item *) p)->freep = nextfree[id].freep, \
					nextfree[id].freep = (struct block_item *) p)

#endif /* MEMDEBUG */

#define getnode(n)	getblock(n, BLOCK_NODE, NODE *)
#define freenode(n)	freeblock(n, BLOCK_NODE)

#define getbucket(b) 	getblock(b, BLOCK_BUCKET, BUCKET *)
#define freebucket(b)	freeblock(b, BLOCK_BUCKET)

#define	make_string(s, l)	make_str_node((s), (l), 0)

// Flags for making string nodes
#define		SCAN			1
#define		ALREADY_MALLOCED	2
#define		ELIDE_BACK_NL		4

#define	cant_happen()	r_fatal("internal error line %d, file: %s", \
				__LINE__, __FILE__)

#define	emalloc(var,ty,x,str)	(void) (var = (ty) emalloc_real((size_t)(x), str, #var, __FILE__, __LINE__))
#define	ezalloc(var,ty,x,str)	(void) (var = (ty) ezalloc_real((size_t)(x), str, #var, __FILE__, __LINE__))
#define	erealloc(var,ty,x,str)	(void) (var = (ty) erealloc_real((void *) var, (size_t)(x), str, #var, __FILE__, __LINE__))

#define efree(p)	free(p)

#define fatal		(*(set_loc(__FILE__, __LINE__), r_fatal))

extern jmp_buf fatal_tag;
extern int fatal_tag_valid;

#define assoc_length(a)	((a)->table_size)
#define assoc_empty(a)	(assoc_length(a) == 0)
#define assoc_lookup(a, s)	((a)->alookup(a, s))

/* assoc_clear --- flush all the values in symbol[] */
#define assoc_clear(a)	(void) ((a)->aclear(a, NULL))

/* assoc_remove --- remove an index from symbol[] */
#define assoc_remove(a, s) ((a)->aremove(a, s) != NULL)


/* ------------- Function prototypes or defs (as appropriate) ------------- */
/* array.c */
typedef enum { SORTED_IN = 1, ASORT, ASORTI } sort_context_t;
typedef enum {
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
} assoc_kind_t;

extern NODE *make_array(void);
extern void null_array(NODE *symbol);
extern NODE *force_array(NODE *symbol, bool canfatal);
extern const char *make_aname(const NODE *symbol);
extern const char *array_vname(const NODE *symbol);
extern void array_init(void);
extern NODE **null_afunc(NODE *symbol, NODE *subs);
extern void set_SUBSEP(void);
extern NODE *concat_exp(int nargs, bool do_subsep);
extern NODE *assoc_copy(NODE *symbol, NODE *newsymb);
extern void assoc_dump(NODE *symbol, NODE *p);
extern NODE **assoc_list(NODE *symbol, const char *sort_str, sort_context_t sort_ctxt);
extern void assoc_info(NODE *subs, NODE *val, NODE *p, const char *aname);
extern void do_delete(NODE *symbol, int nsubs);
extern void do_delete_loop(NODE *symbol, NODE **lhs);
extern NODE *do_adump(int nargs);
extern NODE *do_aoption(int nargs);
extern NODE *do_asort(int nargs);
extern NODE *do_asorti(int nargs);
extern unsigned long (*hash)(const char *s, size_t len, unsigned long hsize, size_t *code);
extern void init_env_array(NODE *env_node);
extern void init_argv_array(NODE *argv_node, NODE *shadow_node);
/* awkgram.c */
extern NODE *variable(int location, char *name, NODETYPE type);
extern int parse_program(INSTRUCTION **pcode, bool from_eval);
extern void track_ext_func(const char *name);
extern void dump_funcs(void);
extern void dump_vars(const char *fname);
extern const char *getfname(NODE *(*)(int), bool prepend_awk);
extern NODE *stopme(int nargs);
extern void shadow_funcs(void);
extern int check_special(const char *name);
extern SRCFILE *add_srcfile(enum srctype stype, char *src, SRCFILE *curr, bool *already_included, int *errcode);
extern void free_srcfile(SRCFILE *thisfile);
extern int files_are_same(char *path, SRCFILE *src);
extern void valinfo(NODE *n, Func_print print_func, FILE *fp);
extern void negate_num(NODE *n);
typedef NODE *(*builtin_func_t)(int);	/* function that implements a built-in */
extern builtin_func_t lookup_builtin(const char *name);
extern void install_builtins(void);
extern bool is_alpha(int c);
extern bool is_alnum(int c);
extern bool is_letter(int c);
extern bool is_identchar(int c);
extern NODE *make_regnode(int type, NODE *exp);
extern bool validate_qualified_name(char *token);
/* builtin.c */
extern double double_to_int(double d);
extern NODE *do_exp(int nargs);
extern NODE *do_fflush(int nargs);
extern NODE *do_index(int nargs);
extern NODE *do_int(int nargs);
extern NODE *do_isarray(int nargs);
extern NODE *do_length(int nargs);
extern NODE *do_log(int nargs);
extern NODE *do_mktime(int nargs);
extern NODE *do_sprintf(int nargs);
extern void do_printf(int nargs, int redirtype);
extern void print_simple(NODE *tree, FILE *fp);
extern NODE *do_sqrt(int nargs);
extern NODE *do_substr(int nargs);
extern NODE *do_strftime(int nargs);
extern NODE *do_systime(int nargs);
extern NODE *do_system(int nargs);
extern void do_print(int nargs, int redirtype);
extern void do_print_rec(int args, int redirtype);
extern NODE *do_tolower(int nargs);
extern NODE *do_toupper(int nargs);
extern NODE *do_atan2(int nargs);
extern NODE *do_sin(int nargs);
extern NODE *do_cos(int nargs);
extern NODE *do_rand(int nargs);
extern NODE *do_srand(int nargs);
extern NODE *do_match(int nargs);
extern NODE *do_sub(int nargs, unsigned int flags);
extern NODE *call_sub(const char *name, int nargs);
extern NODE *call_match(int nargs);
extern NODE *call_split_func(const char *name, int nargs);
extern NODE *format_tree(const char *, size_t, NODE **, long);
extern NODE *do_lshift(int nargs);
extern NODE *do_rshift(int nargs);
extern NODE *do_and(int nargs);
extern NODE *do_or(int nargs);
extern NODE *do_xor(int nargs);
extern NODE *do_compl(int nargs);
extern NODE *do_strtonum(int nargs);
extern AWKNUM nondec2awknum(char *str, size_t len, char **endptr);
extern NODE *do_dcgettext(int nargs);
extern NODE *do_dcngettext(int nargs);
extern NODE *do_bindtextdomain(int nargs);
extern NODE *do_intdiv(int nargs);
extern NODE *do_typeof(int nargs);
extern int strncasecmpmbs(const unsigned char *,
			  const unsigned char *, size_t);
extern int sanitize_exit_status(int status);
/* debug.c */
extern void init_debug(void);
extern int debug_prog(INSTRUCTION *pc);
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
extern const char *redflags2str(int);
extern const char *flags2str(int);
extern const char *genflags2str(int flagval, const struct flagtab *tab);
extern const char *nodetype2str(NODETYPE type);
extern void load_casetable(void);
extern AWKNUM calc_exp(AWKNUM x1, AWKNUM x2);
extern const char *opcode2str(OPCODE type);
extern const char *op2str(OPCODE type);
extern NODE **r_get_lhs(NODE *n, bool reference);
extern STACK_ITEM *grow_stack(void);
extern void dump_fcall_stack(FILE *fp);
extern int register_exec_hook(Func_pre_exec preh, Func_post_exec posth);
extern NODE **r_get_field(NODE *n, Func_ptr *assign, bool reference);
/* ext.c */
extern NODE *do_ext(int nargs);
void load_ext(const char *lib_name);	/* temporary */
extern void close_extensions(void);
extern bool is_valid_identifier(const char *name);
#ifdef DYNAMIC
extern awk_bool_t make_builtin(const char *name_space, const awk_ext_func_t *);
extern NODE *get_argument(int);
extern NODE *get_actual_argument(NODE *, int, bool);
#define get_scalar_argument(n, i)  get_actual_argument((n), (i), false)
#define get_array_argument(n, i)   get_actual_argument((n), (i), true)
#endif
/* field.c */
extern void init_fields(void);
extern void set_record(const char *buf, int cnt, const awk_fieldwidth_info_t *);
extern void reset_record(void);
extern void rebuild_record(void);
extern void set_NF(void);
extern NODE **get_field(long num, Func_ptr *assign);
extern NODE *do_split(int nargs);
extern NODE *do_patsplit(int nargs);
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

/* gawkapi.c: */
extern gawk_api_t api_impl;
extern void init_ext_api(void);
extern void update_ext_api(void);
extern NODE *awk_value_to_node(const awk_value_t *);
extern void run_ext_exit_handlers(int exitval);
extern void print_ext_versions(void);
extern void free_api_string_copies(void);

/* gawkmisc.c */
extern char *gawk_name(const char *filespec);
extern void os_arg_fixup(int *argcp, char ***argvp);
extern int os_devopen(const char *name, int flag);
extern void os_close_on_exec(int fd, const char *name, const char *what, const char *dir);
extern int os_isatty(int fd);
extern int os_isdir(int fd);
extern int os_isreadable(const awk_input_buf_t *iobuf, bool *isdir);
extern int os_is_setuid(void);
extern int os_setbinmode(int fd, int mode);
extern void os_restore_mode(int fd);
extern size_t optimal_bufsize(int fd, struct stat *sbuf);
extern int ispath(const char *file);
extern int isdirpunct(int c);

/* io.c */
extern void init_sockets(void);
extern void init_io(void);
extern void register_input_parser(awk_input_parser_t *input_parser);
extern void register_output_wrapper(awk_output_wrapper_t *wrapper);
extern void register_two_way_processor(awk_two_way_processor_t *processor);
extern void set_FNR(void);
extern void set_NR(void);

extern struct redirect *redirect(NODE *redir_exp, int redirtype, int *errflg, bool failure_fatal);
extern struct redirect *redirect_string(const char *redir_exp_str,
		size_t redir_exp_len, bool not_string_flag, int redirtype,
		int *errflg, int extfd, bool failure_fatal);
extern NODE *do_close(int nargs);
extern int flush_io(void);
extern int close_io(bool *stdio_problem, bool *got_EPIPE);
typedef enum { CLOSE_ALL, CLOSE_TO, CLOSE_FROM } two_way_close_type;
extern int close_rp(struct redirect *rp, two_way_close_type how);
extern int devopen_simple(const char *name, const char *mode, bool try_real_open);
extern int devopen(const char *name, const char *mode);
extern int srcopen(SRCFILE *s);
extern char *find_source(const char *src, struct stat *stb, int *errcode, int is_extlib);
extern NODE *do_getline_redir(int intovar, enum redirval redirtype);
extern NODE *do_getline(int intovar, IOBUF *iop);
extern struct redirect *getredirect(const char *str, int len);
extern bool inrec(IOBUF *iop, int *errcode);
extern int nextfile(IOBUF **curfile, bool skipping);
extern bool is_non_fatal_std(FILE *fp);
extern bool is_non_fatal_redirect(const char *str, size_t len);
extern void ignore_sigpipe(void);
extern void set_sigpipe_to_default(void);
extern bool non_fatal_flush_std_file(FILE *fp);

/* main.c */
extern int arg_assign(char *arg, bool initing);
extern int is_std_var(const char *var);
extern int is_off_limits_var(const char *var);
extern char *estrdup(const char *str, size_t len);
extern void update_global_values();
extern long getenv_long(const char *name);
extern void after_beginfile(IOBUF **curfile);
extern void set_current_namespace(const char *new_namespace);

/* mpfr.c */
extern void set_PREC(void);
extern void set_ROUNDMODE(void);
extern void mpfr_unset(NODE *n);
#ifdef HAVE_MPFR
extern int mpg_cmp(const NODE *, const NODE *);
extern int format_ieee(mpfr_ptr, int);
extern NODE *mpg_update_var(NODE *);
extern long mpg_set_var(NODE *);
extern NODE *do_mpfr_and(int);
extern NODE *do_mpfr_atan2(int);
extern NODE *do_mpfr_compl(int);
extern NODE *do_mpfr_cos(int);
extern NODE *do_mpfr_exp(int);
extern NODE *do_mpfr_int(int);
extern NODE *do_mpfr_intdiv(int);
extern NODE *do_mpfr_log(int);
extern NODE *do_mpfr_lshift(int);
extern NODE *do_mpfr_or(int);
extern NODE *do_mpfr_rand(int);
extern NODE *do_mpfr_rshift(int);
extern NODE *do_mpfr_sin(int);
extern NODE *do_mpfr_sqrt(int);
extern NODE *do_mpfr_srand(int);
extern NODE *do_mpfr_strtonum(int);
extern NODE *do_mpfr_xor(int);
extern void init_mpfr(mpfr_prec_t, const char *);
extern void cleanup_mpfr(void);
extern NODE *mpg_node(unsigned int);
extern const char *mpg_fmt(const char *, ...);
extern int mpg_strtoui(mpz_ptr, char *, size_t, char **, int);
#endif
/* msg.c */
extern void gawk_exit(int status);
extern void final_exit(int status) ATTRIBUTE_NORETURN;
extern void err(bool isfatal, const char *s, const char *emsg, va_list argp) ATTRIBUTE_PRINTF(3, 0);
extern void msg (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void error (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void r_warning (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void set_loc (const char *file, int line);
extern void r_fatal (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2)
extern void (*lintfunc)(const char *mesg, ...) ATTRIBUTE_PRINTF_1;
#else
extern void (*lintfunc)(const char *mesg, ...);
#endif
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
/* node.c */
extern NODE *r_force_number(NODE *n);
extern NODE *r_format_val(const char *format, int index, NODE *s);
extern NODE *r_dupnode(NODE *n);
extern NODE *make_str_node(const char *s, size_t len, int flags);
extern NODE *make_typed_regex(const char *re, size_t len);
extern void *more_blocks(int id);
extern int parse_escape(const char **string_ptr);
extern NODE *str2wstr(NODE *n, size_t **ptr);
extern NODE *wstr2str(NODE *n);
#define force_wstring(n)	str2wstr(n, NULL)
extern const wchar_t *wstrstr(const wchar_t *haystack, size_t hs_len,
		const wchar_t *needle, size_t needle_len);
extern const wchar_t *wcasestrstr(const wchar_t *haystack, size_t hs_len,
		const wchar_t *needle, size_t needle_len);
extern void r_free_wstr(NODE *n);
#define free_wstr(n)	do { if ((n)->flags & WSTRCUR) r_free_wstr(n); } while(0)
extern wint_t btowc_cache[];
#define btowc_cache(x) btowc_cache[(x)&0xFF]
extern void init_btowc_cache();
#define is_valid_character(b)	(btowc_cache[(b)&0xFF] != WEOF)
extern bool out_of_range(NODE *n);
extern char *format_nan_inf(NODE *n, char format);
/* re.c */
extern Regexp *make_regexp(const char *s, size_t len, bool ignorecase, bool dfa, bool canfatal);
extern int research(Regexp *rp, char *str, int start, size_t len, int flags);
extern void refree(Regexp *rp);
extern void reg_error(const char *s);
extern Regexp *re_update(NODE *t);
extern void resyntax(int syntax);
extern void resetup(void);
extern int reisstring(const char *text, size_t len, Regexp *re, const char *buf);
extern int get_numbase(const char *str, size_t len, bool use_locale);
extern bool using_utf8(void);

/* symbol.c */
extern void load_symbols();
extern void init_symbol_table();
extern NODE *symbol_table;
extern NODE *func_table;
extern NODE *install_symbol(const char *name, NODETYPE type);
extern NODE *remove_symbol(NODE *r);
extern void destroy_symbol(NODE *r);
extern void release_symbols(NODE *symlist, int keep_globals);
extern void append_symbol(NODE *r);
extern NODE *lookup(const char *name);
extern NODE *make_params(char **pnames, int pcount);
extern void install_params(NODE *func);
extern void remove_params(NODE *func);
extern void release_all_vars(void);
extern int foreach_func(NODE **table, int (*)(INSTRUCTION *, void *), void *);
extern INSTRUCTION *bcalloc(OPCODE op, int size, int srcline);
extern void bcfree(INSTRUCTION *);
extern AWK_CONTEXT *new_context(void);
extern void push_context(AWK_CONTEXT *ctxt);
extern void pop_context();
extern int in_main_context();
extern void free_context(AWK_CONTEXT *ctxt, bool keep_globals);
extern NODE **variable_list();
extern NODE **function_list(bool sort);
extern void print_vars(NODE **table, Func_print print_func, FILE *fp);
extern bool check_param_names(void);
extern bool is_all_upper(const char *name);

/* floatcomp.c */
#ifdef HAVE_UINTMAX_T
extern uintmax_t adjust_uint(uintmax_t n);
#else
#define adjust_uint(n) (n)
#endif

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
POP_PARAM()
{
	NODE *t = POP();

	return (t->type == Node_var_array) ? t : force_array(t, false);
}

/* POP_SCALAR --- pop the scalar at the top of the stack */

static inline NODE *
POP_SCALAR()
{
	NODE *t = POP();

	if (t->type == Node_var_array)
		fatal(_("attempt to use array `%s' in a scalar context"), array_vname(t));

	return t;
}

/* TOP_SCALAR --- get the scalar at the top of the stack */

static inline NODE *
TOP_SCALAR()
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
force_string_fmt(NODE *s, const char *fmtstr, int fmtidx)
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
	if (r != NULL && --r->valref <= 0)
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
	assert(n->type == Node_val);
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
emalloc_real(size_t count, const char *where, const char *var, const char *file, int line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%d: emalloc called with zero bytes", file, line);

	ret = (void *) malloc(count);
	if (ret == NULL)
		fatal(_("%s:%d:%s: %s: cannot allocate %ld bytes of memory: %s"),
			file, line, where, var, (long) count, strerror(errno));

	return ret;
}

/* ezalloc_real --- malloc zero-filled bytes with error checking */

static inline void *
ezalloc_real(size_t count, const char *where, const char *var, const char *file, int line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%d: ezalloc called with zero bytes", file, line);

	ret = (void *) calloc(1, count);
	if (ret == NULL)
		fatal(_("%s:%d:%s: %s: cannot allocate %ld bytes of memory: %s"),
			file, line, where, var, (long) count, strerror(errno));

	return ret;
}

/* erealloc_real --- realloc with error checking */

static inline void *
erealloc_real(void *ptr, size_t count, const char *where, const char *var, const char *file, int line)
{
	void *ret;

	if (count == 0)
		fatal("%s:%d: erealloc called with zero bytes", file, line);

	ret = (void *) realloc(ptr, count);
	if (ret == NULL)
		fatal(_("%s:%d:%s: %s: cannot reallocate %ld bytes of memory: %s"),
			file, line, where, var, (long) count, strerror(errno));

	return ret;
}

/* make_number_node --- make node with the given flags */

static inline NODE *
make_number_node(unsigned int flags)
{
	NODE *r;
	getnode(r);
	memset(r, 0, sizeof(*r));
	r->type = Node_val;
	r->valref = 1;
	r->flags = (flags|MALLOC|NUMBER|NUMCUR);
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

#ifdef SIGPIPE
#define ignore_sigpipe() signal(SIGPIPE, SIG_IGN)
#define set_sigpipe_to_default() signal(SIGPIPE, SIG_DFL)
#define die_via_sigpipe() (signal(SIGPIPE, SIG_DFL), kill(getpid(), SIGPIPE))
#else
#define ignore_sigpipe()
#define set_sigpipe_to_default()
#ifdef __MINGW32__
/* 0xC0000008 is EXCEPTION_INVALID_HANDLE, somewhat appropriate for EPIPE */
#define die_via_sigpipe() exit(0xC0000008)
#else  /* !__MINGW32__ */
#define die_via_sigpipe() exit(EXIT_FATAL)
#endif	/* !__MINGW32__ */
#endif
