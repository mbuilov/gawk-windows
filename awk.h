/*
 * awk.h -- Definitions for gawk. 
 *
 * Copyright (C) 1986 Free Software Foundation Written by Paul Rubin, August
 * 1986 
 *
 * $Log:	awk.h,v $
 * Revision 1.38  89/03/31  13:15:47  david
 * MSDOS support and more function prototypes
 * 
 * Revision 1.37  89/03/30  10:18:22  david
 * fixed up #if around vfprintf define
 * 
 * Revision 1.36  89/03/29  14:18:19  david
 * delinting
 * move struct redirect and IOBUF here
 * fix WHOLELINE
 * 
 * Revision 1.35  89/03/24  15:56:35  david
 * merge HASHNODE and AHASH into NODE
 * 
 * Revision 1.34  89/03/22  21:01:54  david
 * support for new newnode(); delete obsolete member in struct search
 * 
 * Revision 1.33  89/03/21  19:25:06  david
 * bring some prototypes up to date
 * 
 * Revision 1.32  89/03/21  11:10:44  david
 * major cleanup
 * rearrange NODE structure for space efficiency
 * add MEMDEBUG stuff for finding memory leaks
 * add STREQN define
 * 
 * Revision 1.31  89/03/15  21:53:55  david
 * changes from Arnold: case-insensitive matching, BELL, delete obstack, cleanup
 * 
 * Revision 1.30  89/03/15  21:28:32  david
 * add free_result to free return from tree_eval
 * 
 * Revision 1.29  88/12/15  12:52:10  david
 * casetable made static elsewhere
 * 
 * Revision 1.28  88/12/14  10:50:21  david
 * change FREE_TEMP macro to free_temp
 * 
 * Revision 1.27  88/12/13  22:20:09  david
 * macro-front-end tree_eval, force_string and force_number
 * 
 * Revision 1.25  88/12/08  15:57:11  david
 * added some #ifdef'd out debugging code
 * 
 * Revision 1.24  88/12/07  19:58:37  david
 * changes for printing current source file in error messages
 * 
 * Revision 1.23  88/12/01  15:07:10  david
 * changes to accomodate source line numbers in error messages
 * 
 * Revision 1.22  88/11/30  15:14:59  david
 * FREE_ONE_REFERENCE macro merged inot do_deref()
 * 
 * Revision 1.21  88/11/29  15:17:01  david
 * minor movement
 * 
 * Revision 1.20  88/11/23  21:36:00  david
 * Arnold: portability addition
 * 
 * Revision 1.19  88/11/22  15:51:23  david
 * changed order of elements in NODE decl. for better packing on sparc and 
 * similar machines
 * 
 * Revision 1.18  88/11/22  13:45:15  david
 * Arnold: changes for case-insensitive matching
 * 
 * Revision 1.17  88/11/15  10:15:28  david
 * Arnold: move a bunch of #include's here
 * 
 * Revision 1.16  88/11/14  21:50:26  david
 * Arnold: get sprintf() declaration right; correct STREQ macro
 * 
 * Revision 1.15  88/11/14  21:24:50  david
 * added extern decl. for field_num
 * 
 * Revision 1.14  88/11/03  15:21:03  david
 * extended flags defines; made force_number safe; added TEMP_FREE define
 * 
 * Revision 1.13  88/11/01  12:52:18  david
 * allowed for vprintf code in awk5.c
 * 
 * Revision 1.12  88/11/01  12:07:27  david
 * cleanup; additions of external declarations; added variable name to node;
 * moved flags from sub.val to node proper
 * 
 * Revision 1.11  88/10/19  21:54:29  david
 * safe_malloc to be used by obstack_alloc
 * Node_val to replace other value types (to be done)
 * emalloc and erealloc macros
 * 
 * Revision 1.10  88/10/17  19:52:50  david
 * Arnold: fix cant_happen(); improve VPRINTF; purge FAST
 * 
 * Revision 1.9  88/10/13  22:02:47  david
 * added some external declarations to make life easier
 * #define VPRINTF for portable variable arg list handling
 * 
 * Revision 1.8  88/10/11  22:19:05  david
 * added external decl.
 * 
 * Revision 1.7  88/06/05  22:15:40  david
 * deleted level member from hashnode structure
 * 
 * Revision 1.6  88/06/05  22:05:25  david
 * added cnt member to NODE structure (doesn't add to size, since val member 
 * dominates)
 * 
 * Revision 1.5  88/05/31  09:29:14  david
 * expunge Node_local_var
 * 
 * Revision 1.4  88/05/27  11:04:07  david
 * changed AWKNUM to double to correspond to nawk
 * 
 * Revision 1.3  88/05/13  22:07:56  david
 * moved some defines here from elsewhere
 * 
 * Revision 1.2  88/05/04  12:17:04  david
 * make_for_loop() now returns a NODE *
 * 
 * Revision 1.1  88/04/08  15:15:25  david
 * Initial revision
 *  Revision 1.6  88/04/08  14:48:25  david changes from Arnold
 * Robbins 
 *
 * Revision 1.5  88/03/23  22:17:23  david mostly delinting -- a couple of bug
 * fixes 
 *
 * Revision 1.4  88/03/18  21:00:05  david Baseline -- hoefully all the
 * functionality of the new awk added. Just debugging and tuning to do. 
 *
 * Revision 1.3  87/11/19  14:34:12  david added a bunch of new Node types added
 * a new union entry to the expnode structure to accomodate function
 * parameter names added a level variable to the symbol structure to keep
 * track of function nesting level 
 *
 * Revision 1.2  87/10/29  21:48:32  david added Node_in_array NODETYPE 
 *
 * Revision 1.1  87/10/27  15:23:07  david Initial revision 
 *
 */

/*
 * GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY.  No author or distributor accepts responsibility to anyone for
 * the consequences of using it or for whether it serves any particular
 * purpose or works at all, unless he says so in writing. Refer to the GAWK
 * General Public License for full details. 
 *
 * Everyone is granted permission to copy, modify and redistribute GAWK, but
 * only under the conditions described in the GAWK General Public License.  A
 * copy of this license is supposed to have been given to you along with GAWK
 * so you can know your rights and responsibilities.  It should be in a file
 * named COPYING.  Among other things, the copyright notice and this notice
 * must be preserved on all copies. 
 *
 * In other words, go ahead and share GAWK, but don't try to stop anyone else
 * from sharing it farther.  Help stamp out software hoarding! 
 */

/* ------------------------------ Includes ------------------------------ */
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <varargs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "regex.h"

/* ------------------- System Functions, Variables, etc ------------------- */
/* nasty nasty SunOS-ism */
#ifdef sparc
#include <alloca.h>
#ifdef lint
extern char *alloca();
#endif
#else
#ifdef __STDC__
extern void *alloca();
#else
extern char *alloca();
#endif
#endif
#if defined(USG) || defined(MSDOS)
extern int sprintf();
#define index	strchr
#define rindex	strrchr
#define bcmp	memcmp
#define bcopy(s,d,l) memcpy((d),(s),(l))
#define bzero(p,l) memset((p),0,(l))
/* nasty nasty berkelixm */
#define _setjmp	setjmp
#define _longjmp	longjmp
#else	/* not USG */
extern char *sprintf();
#endif
/*
 * if you don't have vprintf, but you are BSD, the version defined in
 * awk5.c should do the trick.  Otherwise, use this and cross your fingers.
 */
#if defined(NOVPRINTF) && defined(HASDOPRNT)
#define vfprintf(fp,fmt,arg)	_doprnt((fmt), (arg), (fp))
#endif

#ifdef __STDC__
extern char *malloc(unsigned), *realloc(char *, unsigned);
extern void free(char *);
extern char *getenv(char *);

extern char *strcpy(char *, char *), *strcat(char *, char *), *strncpy(char *, char *, int);
extern char *index(char *, int);

extern double atof(char *);
#else
extern char *malloc(), *realloc();
extern void free();
extern char *getenv();

extern char *strcpy(), *strcat(), *strncpy();
extern char *index();

extern double atof();
#endif

extern int errno;
extern char *sys_errlist[];

/* ------------------ Constants, Structures, Typedefs  ------------------ */
#define AWKNUM	double

typedef enum {
	/* illegal entry == 0 */
	Node_illegal,		/* 0 */

	/* binary operators  lnode and rnode are the expressions to work on */
	Node_times,		/* 1 */
	Node_quotient,		/* 2 */
	Node_mod,		/* 3 */
	Node_plus,		/* 4 */
	Node_minus,		/* 5 */
	Node_cond_pair,		/* 6: conditional pair (see Node_line_range) */
	Node_subscript,		/* 7 */
	Node_concat,		/* 8 */

	/* unary operators   subnode is the expression to work on */
	Node_preincrement,	/* 9 */
	Node_predecrement,	/* 10 */
	Node_postincrement,	/* 11 */
	Node_postdecrement,	/* 12 */
	Node_unary_minus,	/* 13 */
	Node_field_spec,	/* 14 */

	/* assignments   lnode is the var to assign to, rnode is the exp */
	Node_assign,		/* 15 */
	Node_assign_times,	/* 16 */
	Node_assign_quotient,	/* 17 */
	Node_assign_mod,	/* 18 */
	Node_assign_plus,	/* 19 */
	Node_assign_minus,	/* 20 */

	/* boolean binaries   lnode and rnode are expressions */
	Node_and,		/* 21 */
	Node_or,		/* 22 */

	/* binary relationals   compares lnode and rnode */
	Node_equal,		/* 23 */
	Node_notequal,		/* 24 */
	Node_less,		/* 25 */
	Node_greater,		/* 26 */
	Node_leq,		/* 27 */
	Node_geq,		/* 28 */

	/* unary relationals   works on subnode */
	Node_not,		/* 29 */

	/* match ops (binary)   work on lnode and rnode ??? */
	Node_match,		/* 30 */
	Node_nomatch,		/* 31 */

	/* data items */
	Node_string,		/* deprecated: 32 has stlen, stptr, and stref */
	Node_temp_string,	/* deprecated: 33 has stlen, stptr, and stref */
	Node_number,		/* deprecated: 34 has numbr */

	/* program structures */
	Node_rule_list,		/* 35 lnode is a rule, rnode is rest of list */
	Node_rule_node,		/* 36 lnode is an conditional, rnode is
				 * statement */
	Node_statement_list,	/* 37 lnode is a statement, rnode is more
				 * list */
	Node_if_branches,	/* 38 lnode is to run on true, rnode on false */
	Node_expression_list,	/* 39 lnode is an exp, rnode is more list */
	Node_param_list,	/* 40 lnode is a variable, rnode is more list */

	/* keywords */
	Node_K_BEGIN,		/* 41 no stuff */
	Node_K_END,		/* 42 ditto */
	Node_K_if,		/* 43 lnode is conditonal, rnode is
				 * if_branches */
	Node_K_while,		/* 44 lnode is condtional, rnode is stuff to
				 * run */
	Node_K_for,		/* 45 lnode is for_struct, rnode is stuff to
				 * run */
	Node_K_arrayfor,	/* 46 lnode is for_struct, rnode is stuff to
				 * run */
	Node_K_break,		/* 47 no subs */
	Node_K_continue,	/* 48 no stuff */
	Node_K_print,		/* 49 lnode is exp_list, rnode is redirect */
	Node_K_printf,		/* 50 lnode is exp_list, rnode is redirect */
	Node_K_next,		/* 51 no subs */
	Node_K_exit,		/* 52 subnode is return value, or NULL */
	Node_K_do,		/* 53 lnode is conditional, rnode is stuff to
				 * run */
	Node_K_return,		/* 54 */
	Node_K_delete,		/* 55 */

	/* I/O redirection for print statements */
	Node_redirect_output,	/* 56 subnode is where to redirect */
	Node_redirect_append,	/* 57 subnode is where to redirect */
	Node_redirect_pipe,	/* 58 subnode is where to redirect */
	Node_redirect_pipein,	/* 59 subnode is where to redirect */
	Node_redirect_input,	/* 60 subnode is where to redirect */

	/* Variables */
	Node_var,		/* 61 rnode is value, lnode is array stuff */
	Node_var_array,		/* 62 array is ptr to elements, asize num of
				 * eles */

	/* Builtins   subnode is explist to work on, proc is func to call */
	Node_builtin,		/* 63 */

	/*
	 * pattern: conditional ',' conditional ;  lnode of Node_line_range
	 * is the two conditionals (Node_cond_pair), other word (rnode place)
	 * is a flag indicating whether or not this range has been entered.
	 */
	Node_line_range,	/* 64 */

	/*
	 * boolean test of membership in array lnode is string-valued
	 * expression rnode is array name 
	 */
	Node_in_array,		/* 65 */
	Node_K_function,	/* 66 lnode is statement list, rnode is
				 * func_params */
	Node_func,		/* 67 lnode is param. list, rnode is
				 * statement list */
	Node_func_call,		/* 68 lnode is name, rnode is expression list */
	Node_K_getline,		/* 69 */
	Node_sub,		/* 70 */
	Node_gsub,		/* 71 */
	Node_K_match,		/* 72 */
	Node_cond_exp,		/* 73 lnode is conditonal, rnode is
				 * if_branches */
	Node_exp,		/* 74 */
	Node_assign_exp,	/* 75 */
	Node_regex,		/* 76 */
	Node_str_num,		/* deprecated: 77 both string and numeric
				 * values are valid
				 */
	Node_val,		/* 78 node is a value - type given by bits in
				 * status - to replace Node_string, Node_num,
				 * Node_temp_str and Node_str_num
				 */
	Node_hashnode,
	Node_ahash,
} NODETYPE;

typedef struct exp_node {
	union {
		struct {
			union {
				struct exp_node *lptr;
				char *param_name;
				char *retext;
				struct exp_node *nextnode;
			} l;
			union {
				struct exp_node *rptr;
				struct exp_node *(*pptr) ();
				struct re_pattern_buffer *preg;
				struct for_loop_header *hd;
				struct exp_node **av;
				int r_ent;	/* range entered */
			} r;
			char *name;
			short number;
			unsigned char recase;
		} nodep;
		struct {
			AWKNUM fltnum;	/* this is here for optimal packing of
					 * the structure on many machines
					 */
			char *sp;
			short slen;
			unsigned char sref;
		} val;
		struct {
			struct exp_node *next;
			char *name;
			int length;
			struct exp_node *value;
		} hash;
#define	hnext	sub.hash.next
#define	hname	sub.hash.name
#define	hlength	sub.hash.length
#define	hvalue	sub.hash.value
		struct {
			struct exp_node *next;
			struct exp_node *name;
			struct exp_node *value;
		} ahash;
#define	ahnext	sub.ahash.next
#define	ahname	sub.ahash.name
#define	ahvalue	sub.ahash.value
	} sub;
	NODETYPE type;
	unsigned char flags;
#				define	MEM	0x7
#				define	MALLOC	1	/* can be free'd */
#				define	TEMP	2	/* should be free'd */
#				define	PERM	4	/* can't be free'd */
#				define	VAL	0x18
#				define	NUM	8
#				define	STR	16
} NODE;

#define lnode	sub.nodep.l.lptr
#define nextp	sub.nodep.l.nextnode
#define rnode	sub.nodep.r.rptr
#define varname	sub.nodep.name
#define source_file	sub.nodep.name
#define	source_line	sub.nodep.number
#define	param_cnt	sub.nodep.number
#define param	sub.nodep.l.param_name

#define subnode	lnode
#define proc	sub.nodep.r.pptr

#define reexp	lnode
#define rereg	sub.nodep.r.preg
#define re_case sub.nodep.recase
#define re_text sub.nodep.l.retext

#define forsub	lnode
#define forloop	rnode->sub.nodep.r.hd

#define stptr	sub.val.sp
#define stlen	sub.val.slen
#define stref	sub.val.sref
#define	valstat	flags

#define numbr	sub.val.fltnum

#define var_value lnode
#define var_array sub.nodep.r.av

#define condpair lnode
#define triggered sub.nodep.r.r_ent

#define HASHSIZE 101

typedef struct for_loop_header {
	NODE *init;
	NODE *cond;
	NODE *incr;
} FOR_LOOP_HEADER;

/* for "for(iggy in foo) {" */
struct search {
	int numleft;
	NODE **arr_ptr;
	NODE *bucket;
	NODE *retval;
};

/* for faster input, bypass stdio */
typedef struct iobuf {
	int fd;
	char *buf;
	char *off;
	int size;	/* this will be determined by an fstat() call */
	int cnt;
	char *secbuf;
	int secsiz;
} IOBUF;

/*
 * structure used to dynamically maintain a linked-list of open files/pipes
 */
struct redirect {
	int flag;
#		define		RED_FILE	1
#		define		RED_PIPE	2
#		define		RED_READ	4
#		define		RED_WRITE	8
#		define		RED_APPEND	16
	char *value;
	FILE *fp;
	IOBUF *iop;
	long offset;		/* used for dynamic management of open files */
	struct redirect *prev;
	struct redirect *next;
};

/* longjmp return codes, must be nonzero */
/* Continue means either for loop/while continue, or next input record */
#define TAG_CONTINUE 1
/* Break means either for/while break, or stop reading input */
#define TAG_BREAK 2
/* Return means return from a function call; leave value in ret_node */
#define	TAG_RETURN 3

#ifdef MSDOS
#define HUGE	0x7fff
#else
#define HUGE	0x7fffffff
#endif

/* -------------------------- External variables -------------------------- */
/* gawk builtin variables */
extern NODE *FS_node, *NF_node, *RS_node, *NR_node;
extern NODE *FILENAME_node, *OFS_node, *ORS_node, *OFMT_node;
extern NODE *FNR_node, *RLENGTH_node, *RSTART_node, *SUBSEP_node;
extern NODE *IGNORECASE_node;

extern NODE **stack_ptr;
extern NODE *Nnull_string;
extern NODE *deref;
extern NODE **fields_arr;
extern int sourceline;
extern char *source;
extern NODE *expression_value;

extern NODE *variables[];

extern NODE *_t;	/* used as temporary in tree_eval */
extern NODE *_result;	/* Ditto */

extern NODE *nextfree;
extern NODE *lastfree;

extern char *myname;

extern int node0_valid;
extern int field_num;
extern int strict;

/* ------------------------- Pseudo-functions ------------------------- */
#define is_identchar(c) (isalnum(c) || (c) == '_')

#define	tree_eval(t)	(_result = (_t = (t),(_t) == NULL ? Nnull_string : \
			((_t)->type == Node_val ? (_t) : r_tree_eval((_t)))))

#define	free_temp(n)	if ((n)->flags&TEMP) { deref = (n); do_deref(); } else
#define	free_result()	if (_result) free_temp(_result); else

/*
 * the loop_tag_valid variable allows continue/break-out-of-context to be
 * caught and diagnosed
 */
#define PUSH_BINDING(stack, x, val) (bcopy ((char *)(x), (char *)(stack), sizeof (jmp_buf)), val++)
#define RESTORE_BINDING(stack, x, val) (bcopy ((char *)(stack), (char *)(x), sizeof (jmp_buf)), val--)

#define	cant_happen()	fatal("line %d, file: %s; bailing out", \
				__LINE__, __FILE__);
#ifdef MEMDEBUG
#define memmsg(x,y,z,zz)	fprintf(stderr, "malloc: %s: %s: %d %0x\n", z, x, y, zz)
#define free(s)	fprintf(stderr, "free: s: %0x\n", s), do_free(s)
#else
#define memmsg(x,y,z,zz)
#endif

#define	emalloc(var,ty,x,str)	if ((var = (ty) malloc((unsigned)(x))) == NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", sys_errlist[errno]); else\
				    memmsg("var", x, str, var)
#define	erealloc(var,ty,x,str)	if((var=(ty)realloc((char *)var,\
						(unsigned)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", sys_errlist[errno]); else\
				    memmsg("re: var", x, str, var)
#ifdef DEBUG
#define	force_number	r_force_number
#define	force_string	r_force_string
#else
#ifdef lint
extern AWKNUM force_number();
#endif
#ifdef MSDOS
extern double _msc51bug;
#define	force_number(n)	(_msc51bug=(_t = (n),(_t->flags & NUM) ? _t->numbr : r_force_number(_t)))
#else
#define	force_number(n)	(_t = (n),(_t->flags & NUM) ? _t->numbr : r_force_number(_t))
#endif
#define	force_string(s)	(_t = (s),(_t->flags & STR) ? _t : r_force_string(_t))
#endif

#define	STREQ(a,b)	(*(a) == *(b) && strcmp((a), (b)) == 0)
#define	STREQN(a,b,n)	((n) && *(a) == *(b) && strncmp((a), (b), (n)) == 0)

#define	WHOLELINE	(node0_valid ? fields_arr[0] : *get_field(0,0))

/* ------------- Function prototypes or defs (as appropriate) ------------- */
#ifdef __STDC__
extern	int devopen(char *, char *);
extern	struct re_pattern_buffer *make_regexp(NODE *, int);
extern	struct re_pattern_buffer *mk_re_parse(char *, int);
extern	NODE *variable(char *);
extern	NODE *install(NODE **, char *, NODE *);
extern	NODE *lookup(NODE **, char *);
extern	NODE *make_name(char *, NODETYPE);
extern	int interpret(NODE *);
extern	NODE *r_tree_eval(NODE *);
extern	void assign_number(NODE **, double);
extern	int cmp_nodes(NODE *, NODE *);
extern	char *get_fs(void);
extern	struct redirect *redirect(NODE *, int *);
extern	int flush_io(void);
extern	void print_simple(NODE *, FILE *);
/* extern	void warning(char *,...); */
/* extern	void fatal(char *,...); */
extern	void set_record(char *, int);
extern	NODE **get_field(int, int);
extern	NODE **get_lhs(NODE *, int);
extern	void do_deref(void );
extern	struct search *assoc_scan(NODE *);
extern	struct search *assoc_next(struct search *);
extern	NODE **assoc_lookup(NODE *, NODE *);
extern	double r_force_number(NODE *);
extern	NODE *r_force_string(NODE *);
extern	NODE *newnode(NODETYPE);
extern	NODE *dupnode(NODE *);
extern	NODE *make_number(double);
extern	NODE *tmp_number(double);
extern	NODE *make_string(char *, int);
extern	NODE *tmp_string(char *, int);
extern	char *re_compile_pattern(char *, int, struct re_pattern_buffer *);
extern	int re_search(struct re_pattern_buffer *, char *, int, int, int, struct re_registers *);

#else
extern	int devopen();
extern	struct re_pattern_buffer *make_regexp();
extern	struct re_pattern_buffer *mk_re_parse();
extern	NODE *variable();
extern	NODE *install();
extern	NODE *lookup();
extern	int interpret();
extern	NODE *r_tree_eval();
extern	void assign_number();
extern	int cmp_nodes();
extern	char *get_fs();
extern	struct redirect *redirect();
extern	int flush_io();
extern	void print_simple();
extern	void warning();
extern	void fatal();
extern	void set_record();
extern	NODE **get_field();
extern	NODE **get_lhs();
extern	void do_deref();
extern	struct search *assoc_scan();
extern	struct search *assoc_next();
extern	NODE **assoc_lookup();
extern	double r_force_number();
extern	NODE *r_force_string();
extern	NODE *newnode();
extern	NODE *dupnode();
extern	NODE *make_number();
extern	NODE *tmp_number();
extern	NODE *make_string();
extern	NODE *tmp_string();
extern	char *re_compile_pattern();
extern	int re_search();
#endif

/* Figure out what '\a' really is. */
#ifdef __STDC__
#define BELL	'\a'		/* sure makes life easy, don't it? */
#else
#	if 'z' - 'a' == 25	/* ascii */
#		if 'a' != 97	/* machine is dumb enough to use mark parity */
#			define BELL	'\207'
#		else
#			define BELL	'\07'
#		endif
#	else
#		define BELL	'\057'
#	endif
#endif

extern char casetable[];	/* for case-independent regexp matching */
