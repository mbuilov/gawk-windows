/*
 * awk.h -- Definitions for gawk. 
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2011 the Free Software Foundation, Inc.
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE	1	/* enable GNU extensions */
#endif /* _GNU_SOURCE */

#if defined(_TANDEM_SOURCE) && ! defined(_SCO_DS)
#define _XOPEN_SOURCE_EXTENDED 1
#endif

#include <stdio.h>
#include <assert.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */
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

#if !defined(__STDC__) || __STDC__ < 1
#error "gawk no longer supports non-C89 environments (no __STDC__ or __STDC__ < 1)"
#endif

#if defined(HAVE_STDARG_H)
#include <stdarg.h>
#else
#error "gawk no loner supports <varargs.h>. Please update your compiler and runtime"
#endif
#include <signal.h>
#include <time.h>
#include <errno.h>
#if ! defined(errno)
extern int errno;
#endif

#ifndef NO_MBSUPPORT
#include "mbsupport.h" /* defines MBS_SUPPORT */
#endif

#if defined(MBS_SUPPORT)
/* We can handle multibyte strings.  */
#include <wchar.h>
#include <wctype.h>
#endif

#ifdef STDC_HEADERS
#include <float.h>
#endif

#undef CHARBITS
#undef INTBITS

#if !defined(ZOS_USS)
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#endif /* !ZOS_USS */

/* ----------------- System dependencies (with more includes) -----------*/

/* This section is the messiest one in the file, not a lot that can be done */

#define MALLOC_ARG_T size_t

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

#if ! defined(S_ISREG) && defined(S_IFREG)
#define	S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#endif	/* not STDC_HEADERS */

#include "protos.h"

#ifdef HAVE_STRING_H
#include <string.h>
#ifdef NEED_MEMORY_H
#include <memory.h>
#endif	/* NEED_MEMORY_H */
#else	/* not HAVE_STRING_H */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif	/* HAVE_STRINGS_H */
#endif	/* not HAVE_STRING_H */

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

#ifndef HAVE_VPRINTF
#error "you lose: you need a system with vfprintf"
#endif	/* HAVE_VPRINTF */

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

#ifdef HAVE_LIBSIGSEGV
#include <sigsegv.h>
#else
typedef void *stackoverflow_context_t;
#define sigsegv_install_handler(catchsegv) signal(SIGSEGV, catchsig)
/* define as 0 rather than empty so that (void) cast on it works */
#define stackoverflow_install_handler(catchstackoverflow, extra_stack, STACK_SIZE) 0
#endif

/* use this as lintwarn("...")
   this is a hack but it gives us the right semantics */
#define lintwarn (*(set_loc(__FILE__, __LINE__),lintfunc))

#include "regex.h"
#include "dfa.h"
typedef struct Regexp {
	struct re_pattern_buffer pat;
	struct re_registers regs;
	struct dfa *dfareg;
	short dfa;
	short has_anchor;	/* speed up of avoid_dfa kludge, temporary */
} Regexp;
#define	RESTART(rp,s)	(rp)->regs.start[0]
#define	REEND(rp,s)	(rp)->regs.end[0]
#define	SUBPATSTART(rp,s,n)	(rp)->regs.start[n]
#define	SUBPATEND(rp,s,n)	(rp)->regs.end[n]
#define	NUMSUBPATS(rp,s)	(rp)->regs.num_regs

/* regexp matching flags: */
#define RE_NEED_START	1	/* need to know start/end of match */
#define RE_NO_BOL	2	/* not allowed to match ^ in regexp */

/* Stuff for losing systems. */
#if !defined(HAVE_STRTOD)
extern double gawk_strtod();
#define strtod gawk_strtod
#endif

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
# define __attribute__(x)
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

/* We use __extension__ in some places to suppress -pedantic warnings
   about GCC extensions.  This feature didn't work properly before
   gcc 2.8.  */
#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
#define __extension__
#endif

/* ------------------ Constants, Structures, Typedefs  ------------------ */

#define AWKNUM	double

#ifndef TRUE
/* a bit hackneyed, but what the heck */
#define TRUE	1
#define FALSE	0
#endif

#define LINT_INVALID	1	/* only warn about invalid */
#define LINT_ALL	2	/* warn about all things */

enum defrule {BEGIN = 1, Rule, END, BEGINFILE, ENDFILE,
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

	Node_hashnode,		/* an identifier in the symbol table */
	Node_ahash,		/* an array element */
	Node_array_ref,		/* array passed by ref as parameter */

	/* program execution -- stack item types */
	Node_arrayfor,
	Node_frame,
	Node_instruction,

	Node_final		/* sentry value, not legal */
} NODETYPE;


struct exp_instruction;

/*
 * NOTE - this struct is a rather kludgey -- it is packed to minimize
 * space usage, at the expense of cleanliness.  Alter at own risk.
 */
typedef struct exp_node {
	union {
		struct {
			union {
				struct exp_node *lptr;
				long ll;
			} l;
			union {
				struct exp_node *rptr;
				Regexp *preg;
				struct exp_node **av;
				void (*uptr)(void);
				struct exp_instruction *iptr;
			} r;
			union {
				struct exp_node *extra;
				void (*aptr)(void);
				long xl;
				char **param_list;
			} x;
			char *name;
			struct exp_node *rn;
			unsigned long reflags;
#				define	CASE		1
#				define	CONSTANT	2
#				define	FS_DFLT		4
		} nodep;
		struct {
			AWKNUM fltnum;	/* this is here for optimal packing of
					 * the structure on many machines
					 */
			char *sp;
			size_t slen;
			long sref;
			int idx;
#ifdef MBS_SUPPORT
			wchar_t *wsp;
			size_t wslen;
#endif
		} val;
		struct {
			AWKNUM num;
			struct exp_node *next;
			char *name;
			size_t length;
			struct exp_node *value;
			long ref;
			size_t code;
		} hash;
#define	hnext	sub.hash.next
#define	hname	sub.hash.name
#define	hlength	sub.hash.length
#define	hvalue	sub.hash.value

#define	ahnext		sub.hash.next
#define	ahname_str	sub.hash.name
#define ahname_len	sub.hash.length
#define ahname_num	sub.hash.num
#define	ahvalue	sub.hash.value
#define ahname_ref	sub.hash.ref
#define	ahcode	sub.hash.code
	} sub;
	NODETYPE type;
	unsigned short flags;
#		define	MALLOC	1	/* can be free'd */
#		define	PERM	2	/* can't be free'd */
#		define	STRING	4	/* assigned as string */
#		define	STRCUR	8	/* string value is current */
#		define	NUMCUR	16	/* numeric value is current */
#		define	NUMBER	32	/* assigned as number */
#		define	MAYBE_NUM 64	/* user input: if NUMERIC then
					 * a NUMBER */
#		define	ARRAYMAXED 128	/* array is at max size */
#		define	FUNC	256	/* this parameter is really a
					 * function name; see awkgram.y */
#		define	FIELD	512	/* this is a field */
#		define	INTLSTR	1024	/* use localized version */
#		define	NUMIND	2048	/* numeric val of index is current */
#		define	WSTRCUR	4096	/* wide str value is current */
} NODE;


#define vname sub.nodep.name

#define lnode	sub.nodep.l.lptr
#define nextp	sub.nodep.l.lptr
#define rnode	sub.nodep.r.rptr

#define	param_cnt	sub.nodep.l.ll
#define param		vname

#define parmlist    sub.nodep.x.param_list
#define code_ptr    sub.nodep.r.iptr

#define re_reg	sub.nodep.r.preg
#define re_flags sub.nodep.reflags
#define re_text lnode
#define re_exp	sub.nodep.x.extra
#define	re_cnt	flags

#define stptr	sub.val.sp
#define stlen	sub.val.slen
#define valref	sub.val.sref
#define	stfmt	sub.val.idx

#define wstptr	sub.val.wsp
#define wstlen	sub.val.wslen

#define numbr	sub.val.fltnum

/* Node_frame: */
#define stack        sub.nodep.r.av
#define func_node    sub.nodep.x.extra
#define reti         sub.nodep.reflags

/* Node_var: */
#define var_value lnode
#define var_update   sub.nodep.r.uptr
#define var_assign	 sub.nodep.x.aptr

/* Node_var_array: */
#define var_array    sub.nodep.r.av
#define array_size   sub.nodep.l.ll
#define table_size   sub.nodep.x.xl
#define parent_array sub.nodep.rn

/* Node_array_ref: */
#define orig_array lnode
#define prev_array rnode

/* --------------------------------lint warning types----------------------------*/
typedef enum lintvals {
	LINT_illegal,
	LINT_assign_in_cond,
	LINT_no_effect
} LINTTYPE;

/* --------------------------------Instruction ---------------------------------- */

typedef enum opcodeval {
	/* illegal entry == 0 */
	Op_illegal,

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
	Op_K_delete,
	Op_K_delete_loop,
	Op_K_getline_redir,
	Op_K_getline,
	Op_K_nextfile,

	Op_builtin,
	Op_in_array,		/* boolean test of membership in array */

	/* function call instruction */
	Op_func_call,
	Op_indirect_func_call,

	Op_push,		/* scalar variable */
	Op_push_arg,	/* variable type (scalar or array) argument to built-in */
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
	Op_after_beginfile,
	Op_after_endfile,

	Op_ext_func,
	Op_func,

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
	Op_final			/* sentry value, not legal */
} OPCODE;

enum redirval {
	/* I/O redirections */
	redirect_output = 1,
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
		long dl;
		char *name;
	} d;

	union {
		long  xl;
		NODE *xn;
		void (*aptr)(void);
		struct exp_instruction *xi;
		struct break_point *bpt;
	} x;

	short source_line;
	OPCODE opcode;
} INSTRUCTION;

#define func_name       d.name

#define memory          d.dn
#define builtin         d.fptr
#define builtin_idx     d.dl

#define expr_count      x.xl

#define target_continue d.di
#define target_jmp      d.di
#define target_break    x.xi

/* Op_K_exit */
#define target_end      d.di
#define target_atexit   x.xi	

/* Op_newfile, Op_K_getline */
#define target_endfile	x.xi

/* Op_K_getline */
#define target_beginfile	d.di

/* Op_token */
#define lextok          d.name

/* Op_rule */
#define in_rule         x.xl
#define source_file     d.name

 /* Op_K_case, Op_K_default */
#define case_stmt       x.xi
#define case_exp        d.di
#define stmt_start		case_exp
#define stmt_end		case_stmt
#define match_exp		x.xl

#define target_stmt       x.xi

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

/* Op_func_call */
#define inrule	        d.dl

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

/* Op_concat */
#define concat_flag	    d.dl
#define CSUBSEP			1
#define CSVAR			2

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

typedef struct iobuf {
	const char *name;       /* filename */
	int fd;                 /* file descriptor */
	struct stat sbuf;       /* stat buf */
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

	void *opaque;		/* private data for open hooks */
	int (*get_record)(char **out, struct iobuf *, int *errcode);
	void (*close_func)(struct iobuf *);		/* open and close hooks */
	
	int errcode;

	int flag;
#		define	IOP_IS_TTY	1
#		define	IOP_NOFREE_OBJ	2
#		define  IOP_AT_EOF      4
#		define  IOP_CLOSED      8
#		define  IOP_AT_START    16			
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
	FILE *fp;
	FILE *ifp;	/* input fp, needed for PIPES_SIMULATED */
	IOBUF *iop;
	int pid;
	int status;
	struct redirect *prev;
	struct redirect *next;
	const char *mode;
};

/*
 * structure for our source, either a command line string or a source file.
 */

typedef struct srcfile {
	struct srcfile *next;
	struct srcfile *prev;

	enum srctype { SRC_CMDLINE = 1, SRC_STDIN, SRC_FILE, SRC_INC } stype;
	char *src;	/* name on command line or inclde statement */
	char *fullpath;	/* full path after AWKPATH search */
	time_t mtime;
	struct stat sbuf;
	int srclines;	/* no of lines in source */
	size_t bufsize;
	char *buf;
	int *line_offset;	/* offset to the beginning of each line */
	int fd;
	int maxlen;	/* size of the longest line */

	char *lexptr;
	char *lexend;
	char *lexeme;
	char *lexptr_begin;
	int lasttok;
} SRCFILE;

/* structure for execution context */
typedef struct context {
	INSTRUCTION pools;
	NODE symbols;
	INSTRUCTION rule_list;
	SRCFILE srcfiles;
	int sourceline;
	char *source;
	void (*install_func)(char *);
	struct context *prev;
} AWK_CONTEXT;

/* for debugging purposes */
struct flagtab {
	int val;
	const char *name;
};

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
extern int IGNORECASE;
extern int RS_is_null;
extern char *OFS;
extern int OFSlen;
extern char *ORS;
extern int ORSlen;
extern char *OFMT;
extern char *CONVFMT;
extern int CONVFMTidx;
extern int OFMTidx;
extern char *TEXTDOMAIN;
extern NODE *BINMODE_node, *CONVFMT_node, *FIELDWIDTHS_node, *FILENAME_node;
extern NODE *FNR_node, *FS_node, *IGNORECASE_node, *NF_node;
extern NODE *NR_node, *OFMT_node, *OFS_node, *ORS_node, *RLENGTH_node;
extern NODE *RSTART_node, *RS_node, *RT_node, *SUBSEP_node, *PROCINFO_node;
extern NODE *LINT_node, *ERRNO_node, *TEXTDOMAIN_node, *FPAT_node;
extern NODE *Nnull_string;
extern NODE *Null_field;
extern NODE **fields_arr;
extern int sourceline;
extern char *source;

#if __GNUC__ < 2
extern NODE *_t;	/* used as temporary in tree_eval */
#endif
extern NODE *_r;	/* used as temporary in stack macros */

extern NODE *nextfree;
extern int field0_valid;
extern int do_traditional;
extern int do_posix;
extern int do_intervals;
extern int do_intl;
extern int do_non_decimal_data;
extern int do_profiling;
extern int do_dump_vars;
extern int do_tidy_mem;
extern int do_sandbox;
extern int do_optimize;
extern int use_lc_numeric;
extern int exit_val;

#ifdef NO_LINT
#define do_lint 0
#define do_lint_old 0
#else
extern int do_lint;
extern int do_lint_old;
#endif
#ifdef MBS_SUPPORT
extern int gawk_mb_cur_max;
#else
extern const int gawk_mb_cur_max;
#endif

#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
extern GETGROUPS_T *groupset;
extern int ngroups;
#endif

#ifdef HAVE_LOCALE_H
extern struct lconv loc;
#endif /* HAVE_LOCALE_H */

extern const char *myname;
extern const char def_strftime_format[];

extern char quote;
extern char *defpath;
extern char envsep;

extern char casetable[];	/* for case-independent regexp matching */

/*
 * Provide a way for code to know which program is executing:
 * gawk vs dgawk vs pgawk.
 */
enum exe_mode { exe_normal = 1, exe_debugging, exe_profiling };
extern enum exe_mode which_gawk;	/* (defined in eval.c) */

/* ------------------------- Runtime stack -------------------------------- */

typedef union stack_item {
	NODE *rptr;		/* variable etc. */
	NODE **lptr;		/* address of a variable etc. */
} STACK_ITEM;

extern STACK_ITEM *stack_ptr;
extern NODE *frame_ptr;
extern STACK_ITEM *stack_bottom;
extern STACK_ITEM *stack_top;

#define decr_sp()			(stack_ptr--)
#define incr_sp()		((stack_ptr < stack_top) ? ++stack_ptr : grow_stack())
#define stack_adj(n)			(stack_ptr += (n))
#define stack_empty()			(stack_ptr < stack_bottom)

#define POP()				decr_sp()->rptr
#define POP_ADDRESS()			decr_sp()->lptr
#define PEEK(n)				(stack_ptr - (n))->rptr
#define TOP()				stack_ptr->rptr		/* same as PEEK(0) */
#define TOP_ADDRESS()			stack_ptr->lptr 
#define PUSH(r)				(void) (incr_sp()->rptr = (r))
#define PUSH_ADDRESS(l)			(void) (incr_sp()->lptr = (l))
#define REPLACE(r)			(void) (stack_ptr->rptr = (r))
#define REPLACE_ADDRESS(l)		(void) (stack_ptr->lptr = (l))


/* function param */
#define GET_PARAM(n)			frame_ptr->stack[n]

/*
 * UPREF and DEREF --- simplified versions of dupnode and unref
 * UPREF does not handle FIELD node. Most appropriate use is
 * for elements on the runtime stack. When in doubt, use dupnode.
 */   

#define DEREF(r)	( _r = (r), (!(_r->flags & PERM) && (--_r->valref == 0)) ?  unref(_r) : (void)0 )

#if __GNUC__ >= 2
#define UPREF(r)	({ NODE *_t = (r); !(_t->flags & PERM) && _t->valref++;})

#define POP_ARRAY()	({ NODE *_t = POP(); \
			_t->type == Node_var_array ? \
				_t : get_array(_t, TRUE); })

#define POP_PARAM()	({ NODE *_t = POP(); \
			_t->type == Node_var_array ? \
				_t : get_array(_t, FALSE); })

#define POP_NUMBER(x) ({ NODE *_t = POP_SCALAR(); \
				x = force_number(_t); DEREF(_t); })
#define TOP_NUMBER(x) ({ NODE *_t = TOP_SCALAR(); \
				x = force_number(_t); DEREF(_t); })

#define POP_SCALAR()			({ NODE *_t = POP(); _t->type != Node_var_array ? _t \
			: (fatal(_("attempt to use array `%s' in a scalar context"), array_vname(_t)), _t);})
#define TOP_SCALAR()			({ NODE *_t = TOP(); _t->type != Node_var_array ? _t \
			: (fatal(_("attempt to use array `%s' in a scalar context"), array_vname(_t)), _t);})

#else	/* not __GNUC__ */

#define UPREF(r)	(_t = (r), !(_t->flags & PERM) && _t->valref++)

#define POP_ARRAY()	(_t = POP(), \
			_t->type == Node_var_array ? \
				_t : get_array(_t, TRUE))

#define POP_PARAM()	(_t = POP(), \
			_t->type == Node_var_array ? \
				_t : get_array(_t, FALSE))

#define POP_NUMBER(x) (_t = POP_SCALAR(), \
				x = force_number(_t), DEREF(_t))
#define TOP_NUMBER(x) (_t = TOP_SCALAR(), \
				x = force_number(_t), DEREF(_t))

#define POP_SCALAR()	(_t = POP(), _t->type != Node_var_array ? _t \
			: (fatal(_("attempt to use array `%s' in a scalar context"), array_vname(_t)), _t))
#define TOP_SCALAR()	(_t = TOP(), _t->type != Node_var_array ? _t \
			: (fatal(_("attempt to use array `%s' in a scalar context"), array_vname(_t)), _t))

#endif	/* __GNUC__ */


#define POP_STRING() force_string(POP_SCALAR())
#define TOP_STRING() force_string(TOP_SCALAR())

/* ------------------------- Pseudo-functions ------------------------- */

#define is_identchar(c)		(isalnum(c) || (c) == '_')

#define var_uninitialized(n)	((n)->var_value == Nnull_string)

#define get_lhs(n, r)	 (n)->type == Node_var && ! var_uninitialized(n) ? \
				&((n)->var_value) : r_get_lhs((n), (r))

#ifdef MPROF
#define	getnode(n)	emalloc((n), NODE *, sizeof(NODE), "getnode"), \
                          (n)->flags = 0
#define	freenode(n)	efree(n)
#else	/* not MPROF */
#define getnode(n)  (void) (nextfree ? \
                              (n = nextfree, nextfree = nextfree->nextp) \
                              : (n = more_nodes()))
#define	freenode(n)	((n)->flags = 0, (n)->nextp = nextfree, nextfree = (n))
#endif	/* not MPROF */

#define make_number(x)  mk_number((x), (unsigned int)(MALLOC|NUMCUR|NUMBER))

#define	make_string(s, l)		r_make_str_node((s), (size_t) (l), 0)
#define make_str_node(s, l, f)	r_make_str_node((s), (size_t) (l), (f))

#define		SCAN			1
#define		ALREADY_MALLOCED	2

#define	cant_happen()	r_fatal("internal error line %d, file: %s", \
				__LINE__, __FILE__)

#define	emalloc(var,ty,x,str)	(void)((var=(ty)malloc((MALLOC_ARG_T)(x))) ||\
				 (fatal(_("%s: %s: can't allocate %ld bytes of memory (%s)"),\
					(str), #var, (long) (x), strerror(errno)),0))
#define	erealloc(var,ty,x,str)	(void)((var = (ty)realloc((char *)var, (MALLOC_ARG_T)(x))) \
				||\
				 (fatal(_("%s: %s: can't allocate %ld bytes of memory (%s)"),\
					(str), #var, (long) (x), strerror(errno)),0))

#define efree(p)	free(p)

#ifdef GAWKDEBUG
#define	force_number	r_force_number
#define	force_string	r_force_string
#else /* not GAWKDEBUG */
#if __GNUC__ >= 2
#define	force_number(n)	__extension__ ({NODE *_tn = (n);\
			(_tn->flags & NUMCUR) ? _tn->numbr : r_force_number(_tn);})

#define	force_string(s)	__extension__ ({NODE *_ts = (s);\
			   ((_ts->flags & STRCUR) && \
			   (_ts->stfmt == -1 || _ts->stfmt == CONVFMTidx)) ?\
			  _ts : format_val(CONVFMT, CONVFMTidx, _ts);})
#else /* not __GNUC__ */
#define	force_number	r_force_number
#define	force_string	r_force_string
#endif /* __GNUC__ */
#endif /* GAWKDEBUG */

#define	STREQ(a,b)	(*(a) == *(b) && strcmp((a), (b)) == 0)
#define	STREQN(a,b,n)	((n) && *(a)== *(b) && \
			 strncmp((a), (b), (size_t) (n)) == 0)

#define fatal		set_loc(__FILE__, __LINE__), r_fatal


extern jmp_buf fatal_tag;
extern int fatal_tag_valid;

#define PUSH_BINDING(stack, tag, val)	\
if (val++) \
	memcpy((char *) (stack), (const char *) tag, sizeof(jmp_buf))
#define POP_BINDING(stack, tag, val)	\
if (--val)	\
	memcpy((char *) tag, (const char *) (stack), sizeof(jmp_buf))

/* ------------- Function prototypes or defs (as appropriate) ------------- */
typedef int (*Func_print)(FILE *, const char *, ...);

/* array.c */
typedef enum sort_context { SORTED_IN = 1, ASORT, ASORTI } SORT_CTXT;
extern NODE **assoc_list(NODE *array, const char *sort_str, SORT_CTXT sort_ctxt);
extern NODE *get_array(NODE *symbol, int canfatal);
extern char *array_vname(const NODE *symbol);
extern void array_init(void);
extern void set_SUBSEP(void);
extern NODE *concat_exp(int nargs, int do_subsep);
extern void ahash_unref(NODE *tmp);
extern void assoc_clear(NODE *symbol);
extern NODE *in_array(NODE *symbol, NODE *subs);
extern NODE **assoc_lookup(NODE *symbol, NODE *subs, int reference);
extern void do_delete(NODE *symbol, int nsubs);
extern void do_delete_loop(NODE *symbol, NODE **lhs);
extern NODE *assoc_dump(NODE *symbol, int indent_level);
extern NODE *do_adump(int nargs);
extern NODE *do_asort(int nargs);
extern NODE *do_asorti(int nargs);
extern unsigned long (*hash)(const char *s, size_t len, unsigned long hsize, size_t *code);
/* awkgram.c */
extern NODE *mk_symbol(NODETYPE type, NODE *value);
extern NODE *install_symbol(char *name, NODE *value);
extern NODE *remove_symbol(char *name);
extern NODE *lookup(const char *name);
extern NODE *variable(char *name, NODETYPE type);
extern int parse_program(INSTRUCTION **pcode);
extern void dump_funcs(void);
extern void dump_vars(const char *fname);
extern void release_all_vars(void);
extern const char *getfname(NODE *(*)(int));
extern NODE *stopme(int nargs);
extern void shadow_funcs(void);
extern int check_special(const char *name);
extern int foreach_func(int (*)(INSTRUCTION *, void *), int, void *);
extern INSTRUCTION *bcalloc(OPCODE op, int size, int srcline);
extern void bcfree(INSTRUCTION *);
extern SRCFILE *add_srcfile(int stype, char *src, SRCFILE *curr, int *already_included, int *errcode);
extern void register_deferred_variable(const char *name, NODE *(*load_func)(void));
extern int files_are_same(char *path, SRCFILE *src);
extern void valinfo(NODE *n, Func_print print_func, FILE *fp);
extern void print_vars(Func_print print_func, FILE *fp);
extern AWK_CONTEXT *new_context(void);
extern void push_context(AWK_CONTEXT *ctxt);
extern void pop_context();
extern int in_main_context();
extern void free_context(AWK_CONTEXT *ctxt, int );
extern void append_symbol(char *name);

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
extern NODE *do_gsub(int nargs);
extern NODE *do_sub(int nargs);
extern NODE *do_gensub(int nargs);
extern NODE *format_tree(const char *, size_t, NODE **, long);
extern NODE *do_lshift(int nargs);
extern NODE *do_rshift(int nargs);
extern NODE *do_and(int nargs);
extern NODE *do_or(int nargs);
extern NODE *do_xor(int nargs);
extern NODE *do_compl(int nargs);
extern NODE *do_strtonum(int nargs);
extern AWKNUM nondec2awknum(char *str, size_t len);
extern NODE *do_dcgettext(int nargs);
extern NODE *do_dcngettext(int nargs);
extern NODE *do_bindtextdomain(int nargs);
#ifdef MBS_SUPPORT
extern int strncasecmpmbs(const unsigned char *,
			  const unsigned char *, size_t);
#endif
/* eval.c */
extern void PUSH_CODE(INSTRUCTION *cp);
extern INSTRUCTION *POP_CODE(void);
extern int interpret(INSTRUCTION *);
extern int cmp_nodes(NODE *, NODE *);
extern void set_IGNORECASE(void);
extern void set_OFS(void);
extern void set_ORS(void);
extern void set_OFMT(void);
extern void set_CONVFMT(void);
extern void set_BINMODE(void);
extern void set_LINT(void);
extern void set_TEXTDOMAIN(void);
extern void update_ERRNO(void);
extern void update_ERRNO_saved(int);
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
extern NODE **r_get_lhs(NODE *n, int reference);
extern STACK_ITEM *grow_stack(void);
#ifdef PROFILING
extern void dump_fcall_stack(FILE *fp);
#endif
/* ext.c */
NODE *do_ext(int nargs);
#ifdef DYNAMIC
void make_builtin(const char *, NODE *(*)(int), int);
size_t get_curfunc_arg_count(void);
NODE *get_argument(int);
NODE *get_actual_argument(int, int, int);
#define get_scalar_argument(i, opt)  get_actual_argument((i), (opt), FALSE)
#define get_array_argument(i, opt)   get_actual_argument((i), (opt), TRUE)
#endif
/* field.c */
extern void init_fields(void);
extern void set_record(const char *buf, int cnt);
extern void reset_record(void);
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
	Using_FPAT
} field_sep_type;
extern field_sep_type current_field_sep(void);

/* gawkmisc.c */
extern char *gawk_name(const char *filespec);
extern void os_arg_fixup(int *argcp, char ***argvp);
extern int os_devopen(const char *name, int flag);
extern void os_close_on_exec(int fd, const char *name, const char *what, const char *dir);
extern int os_isatty(int fd);
extern int os_isdir(int fd);
extern int os_is_setuid(void);
extern int os_setbinmode(int fd, int mode);
extern void os_restore_mode(int fd);
extern size_t optimal_bufsize(int fd, struct stat *sbuf);
extern int ispath(const char *file);
extern int isdirpunct(int c);

/* io.c */
extern void register_open_hook(void *(*open_func)(IOBUF *));
extern void set_FNR(void);
extern void set_NR(void);

extern struct redirect *redirect(NODE *redir_exp, int redirtype, int *errflg);
extern NODE *do_close(int nargs);
extern int flush_io(void);
extern int close_io(int *stdio_problem);
extern int devopen(const char *name, const char *mode);
extern int srcopen(SRCFILE *s);
extern char *find_source(const char *src, struct stat *stb, int *errcode);
extern NODE *do_getline_redir(int intovar, int redirtype);
extern NODE *do_getline(int intovar, IOBUF *iop);
extern struct redirect *getredirect(const char *str, int len);
extern int inrec(IOBUF *iop);
extern int nextfile(IOBUF **curfile, int skipping);
/* main.c */
extern int arg_assign(char *arg, int initing);
extern int is_std_var(const char *var);
extern char *estrdup(const char *str, size_t len);
extern void update_global_values();
/* msg.c */
extern void gawk_exit(int status);
extern void err(const char *s, const char *emsg, va_list argp) ATTRIBUTE_PRINTF(2, 0);
extern void msg (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void error (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void warning (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
extern void set_loc (const char *file, int line);
extern void r_fatal (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2)
extern void (*lintfunc) (const char *mesg, ...) ATTRIBUTE_PRINTF_1;
#else
extern void (*lintfunc) (const char *mesg, ...);
#endif
/* profile.c */
extern void init_profiling(int *flag, const char *def_file);
extern void init_profiling_signals(void);
extern void set_prof_file(const char *filename);
extern void dump_prog(INSTRUCTION *code);
extern char *pp_number(AWKNUM d);
extern char *pp_string(const char *in_str, size_t len, int delim);
extern char *pp_node(NODE *n);
extern int pp_func(INSTRUCTION *pc, void *);
extern void pp_string_fp(Func_print print_func, FILE *fp, const char *str,
		size_t namelen, int delim, int breaklines);
/* node.c */
extern AWKNUM r_force_number(NODE *n);
extern NODE *format_val(const char *format, int index, NODE *s);
extern NODE *r_force_string(NODE *s);
extern NODE *dupnode(NODE *n);
extern NODE *mk_number(AWKNUM x, unsigned int flags);
extern NODE *r_make_str_node(const char *s, unsigned long len, int scan);
extern NODE *more_nodes(void);
extern void unref(NODE *tmp);
extern int parse_escape(const char **string_ptr);
#ifdef MBS_SUPPORT
extern NODE *str2wstr(NODE *n, size_t **ptr);
extern NODE *wstr2str(NODE *n);
#define force_wstring(n)	str2wstr(n, NULL)
extern const wchar_t *wstrstr(const wchar_t *haystack, size_t hs_len,
		const wchar_t *needle, size_t needle_len);
extern const wchar_t *wcasestrstr(const wchar_t *haystack, size_t hs_len,
		const wchar_t *needle, size_t needle_len);
extern void free_wstr(NODE *n);
extern wint_t btowc_cache[];
#define btowc_cache(x) btowc_cache[(x)&0xFF]
extern void init_btowc_cache();
#define is_valid_character(b)	(btowc_cache[(b)&0xFF] != WEOF)
#else
#define free_wstr(NODE)	/* empty */
#endif
/* re.c */
extern Regexp *make_regexp(const char *s, size_t len, int ignorecase, int dfa, int canfatal);
extern int research(Regexp *rp, char *str, int start, size_t len, int flags);
extern void refree(Regexp *rp);
extern void reg_error(const char *s);
extern Regexp *re_update(NODE *t);
extern void resyntax(int syntax);
extern void resetup(void);
extern int avoid_dfa(NODE *re, char *str, size_t len);
extern int reisstring(const char *text, size_t len, Regexp *re, const char *buf);
extern int remaybelong(const char *text, size_t len);
extern int isnondecimal(const char *str, int use_locale);

/* floatcomp.c */
#ifdef VMS	/* VMS linker weirdness? */
#define Ceil	gawk_ceil
#define Floor	gawk_floor
#endif

extern AWKNUM Floor(AWKNUM n);
extern AWKNUM Ceil(AWKNUM n);
#ifdef HAVE_UINTMAX_T
extern uintmax_t adjust_uint(uintmax_t n);
#else
#define adjust_uint(n) (n)
#endif

#define INVALID_HANDLE (-1)

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

/* For z/OS, from Dave Pitts. EXIT_FAILURE is normally 8, make it 1. */
#ifdef ZOS_USS
#undef DYNAMIC

#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#endif

#define EXIT_FAILURE 1
#endif
