/*
 * awk.h -- Definitions for gawk. 
 *
 * Copyright (C) 1986 Free Software Foundation Written by Paul Rubin, August
 * 1986 
 *
 * $Log:	awk.h,v $
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

#define AWKNUM	double

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <varargs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "regex.h"
#define is_identchar(c) (isalnum(c) || (c) == '_')

#ifdef notdef
#define	free	do_free		/* for debugging */
#define	malloc	do_malloc	/* for debugging */
#endif

#include "obstack.h"
#define obstack_chunk_alloc safe_malloc
#define obstack_chunk_free free
char *malloc(), *realloc();
char *safe_malloc();
void free();

typedef enum {
	/* illegal entry == 0 */
	Node_illegal,		/* 0 */

	/* binary operators  lnode and rnode are the expressions to work on */
	Node_times,		/* 1 */
	Node_quotient,		/* 2 */
	Node_mod,		/* 3 */
	Node_plus,		/* 4 */
	Node_minus,		/* 5 */
	Node_cond_pair,		/* 6: conditional pair (see Node_line_range)
				 * jfw */
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
	 * (jfw@eddie.mit.edu) 
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
	Node_case_match,	/* 79 case independant regexp match */
	Node_case_nomatch,	/* 80 case independant regexp no match */
} NODETYPE;

typedef struct exp_node {
	NODETYPE type;
	union {
		struct {
			union {
				struct exp_node *lptr;
				char *param_name;
			} l;
			union {
				struct exp_node *rptr;
				struct exp_node *(*pptr) ();
				struct re_pattern_buffer *preg;
				struct for_loop_header *hd;
				struct ahash **av;
				int r_ent;	/* range entered (jfw) */
			} r;
			int number;
			char *name;
		} nodep;
		struct {
			struct exp_node **ap;
			int as;
		} ar;
		struct {
			char *sp;
			AWKNUM fltnum;	/* this is here for optimal packing of
					 * the structure on many machines
					 */
			short slen;
			unsigned char sref;
		} val;
	} sub;
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

#define forsub	lnode
#define forloop	rnode->sub.nodep.r.hd

#define array	sub.ar.ap
#define arrsiz	sub.ar.as

#define stptr	sub.val.sp
#define stlen	sub.val.slen
#define stref	sub.val.sref
#define	valstat	flags

#define numbr	sub.val.fltnum

#define var_value lnode
#define var_array sub.nodep.r.av

#define condpair lnode
#define triggered sub.nodep.r.r_ent

NODE *newnode(), *dupnode();
NODE *node(), *snode(), *make_number(), *make_string(), *make_name();
NODE *make_param();
NODE *mkrangenode();		/* to remove the temptation to use
				 * sub.nodep.r.rptr as a boolean flag, or to
				 * call node() with a 0 and hope that it will
				 * store correctly as an int. (jfw) */
NODE *tmp_string(), *tmp_number();
NODE *variable(), *append_right();

NODE *r_tree_eval();
NODE **get_lhs();

struct re_pattern_buffer *make_regexp();

extern NODE **stack_ptr;
extern NODE *Nnull_string;
extern NODE *FS_node, *NF_node, *RS_node, *NR_node;
extern NODE *FILENAME_node, *OFS_node, *ORS_node, *OFMT_node;
extern NODE *FNR_node, *RLENGTH_node, *RSTART_node, *SUBSEP_node;

extern struct obstack other_stack;
extern NODE *deref;
extern NODE **fields_arr;
extern int sourceline;
extern char *source;

#ifdef USG
int sprintf();
#else
char *sprintf();
#endif
char *strcpy(), *strcat();

double atof();
AWKNUM r_force_number();
NODE *r_force_string();


NODE *expression_value;

#define HASHSIZE 101

typedef struct hashnode HASHNODE;
struct hashnode {
	HASHNODE *next;
	char *name;
	int length;
	NODE *value;
} *variables[HASHSIZE];


typedef struct ahash AHASH;
struct ahash {
	AHASH *next;
	NODE *name, *symbol, *value;
};


typedef struct for_loop_header {
	NODE *init;
	NODE *cond;
	NODE *incr;
} FOR_LOOP_HEADER;

NODE *make_for_loop();

/* for "for(iggy in foo) {" */
struct search {
	int numleft;
	AHASH **arr_ptr;
	AHASH *bucket;
	NODE *symbol;
	NODE *retval;
};

struct search *assoc_scan(), *assoc_next();

extern NODE *_t;	/* used as temporary in following macro */
extern NODE *_result;
#define	tree_eval(t)	(_result = (_t = (t),(_t) == NULL ? Nnull_string : \
			((_t)->type == Node_val ? (_t) : r_tree_eval((_t)))))

#define	free_temp(n)	if ((n)->flags&TEMP) { deref = (n); do_deref(); } else
#define	free_result()	if (_result) free_temp(_result); else

#ifdef USG
#define index	strchr
#define rindex	strrchr
#define bcmp	memcmp
/* nasty nasty berkelixm */
#define _setjmp	setjmp
#define _longjmp	longjmp
#endif

char *index();

/* longjmp return codes, must be nonzero */
/* Continue means either for loop/while continue, or next input record */
#define TAG_CONTINUE 1
/* Break means either for/while break, or stop reading input */
#define TAG_BREAK 2
/* Return means return from a function call; leave value in ret_node */
#define	TAG_RETURN 3

/*
 * the loop_tag_valid variable allows continue/break-out-of-context to be
 * caught and diagnosed (jfw) 
 */
#define PUSH_BINDING(stack, x, val) (bcopy ((char *)(x), (char *)(stack), sizeof (jmp_buf)), val++)
#define RESTORE_BINDING(stack, x, val) (bcopy ((char *)(stack), (char *)(x), sizeof (jmp_buf)), val--)

/* nasty nasty SunOS-ism */
#ifdef sparc
#include <alloca.h>
#endif

extern char *myname;
void msg();
void warning();
void illegal_type();
void fatal();

#define	cant_happen()	fatal("line %d, file: %s; bailing out", \
				__LINE__, __FILE__);

/*
 * if you don't have vprintf, but you are BSD, the version defined in
 * awk5.c should do the trick.  Otherwise, use this and cross your fingers.
 */
#if !defined(VPRINTF) && !defined(BSD)
#define vfprintf(fp,fmt,arg)	_doprnt((fmt), (arg), (fp))
#endif

extern int errno;
extern char *sys_errlist[];

#define	emalloc(var,ty,x,str)	if ((var = (ty) malloc((unsigned)(x))) == NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", sys_errlist[errno]); else
#define	erealloc(var,ty,x,str)	if((var=(ty)realloc(var,(unsigned)(x)))==NULL)\
				    fatal("%s: %s: can't allocate memory (%s)",\
					(str), "var", sys_errlist[errno]); else
#ifdef DEBUG
#define	force_number	r_force_number
#define	force_string	r_force_string
#else
#define	force_number(n)	(_t = (n),(_t->flags & NUM) ? _t->numbr : r_force_number(_t))
#define	force_string(s)	(_t = (s),(_t->flags & STR) ? _t : r_force_string(_t))
#endif

#define	STREQ(a,b)	(*(a) == *(b) && strcmp((a), (b)) == 0)
#define HUGE	0x7fffffff

extern int node0_valid;
extern int field_num;
extern NODE **get_field();
#define	WHOLELINE	(node0_valid ? fields_arr[0] : *get_field(0))

extern int strict;
