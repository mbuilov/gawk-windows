/*
 * awk.h -- Definitions for gawk.
 *
 * Copyright (C) 1986 Free Software Foundation
 *   Written by Paul Rubin, August 1986
 *
 */

/*
GAWK is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GAWK General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GAWK,
but only under the conditions described in the GAWK General Public
License.  A copy of this license is supposed to have been given to you
along with GAWK so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GAWK, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

#define AWKNUM	float

#include <ctype.h>
#define is_identchar(c) (isalnum(c) || (c) == '_')

#include "obstack.h"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
char *malloc(),*realloc();
void free();

typedef enum {
  /* illegal entry == 0 */ 
  Node_illegal,		/* 0 */

  /* binary operators  lnode and rnode are the expressions to work on */
  Node_times,		/* 1 */
  Node_quotient,	/* 2 */
  Node_mod,		/* 3 */
  Node_plus,		/* 4 */
  Node_minus,		/* 5 */
  Node_cond_pair,	/* 6: conditional pair (see Node_line_range) jfw */
  Node_subscript,	/* 7 */
  Node_concat,		/* 8 */

  /* unary operators   subnode is the expression to work on */
  Node_preincrement,	/* 9 */
  Node_predecrement,	/* 10 */
  Node_postincrement, 	/* 11 */
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
  Node_notequal,	/* 24 */
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
  Node_string,		/* 32 has stlen, stptr, and stref */
  Node_temp_string,	/* 33 has stlen, stptr, and stref */
  Node_number,		/* 34 has numbr */

  /* program structures */ 
  Node_rule_list,	/* 35 lnode is a rule, rnode is rest of list */
  Node_rule_node,	/* 36 lnode is an conditional, rnode is statement */
  Node_statement_list,	/* 37 lnode is a statement, rnode is more list */
  Node_if_branches,	/* 38 lnode is to run on true, rnode on false */
  Node_expression_list,	/* 39 lnode is an exp, rnode is more list */

  /* keywords */ 
  Node_K_BEGIN,		/* 40 no stuff */
  Node_K_END,		/* 41 ditto */
  Node_K_if,		/* 42 lnode is conditonal, rnode is if_branches */
  Node_K_while,		/* 43 lnode is condtional, rnode is stuff to run */
  Node_K_for,		/* 44 lnode is for_struct, rnode is stuff to run */
  Node_K_arrayfor,	/* 45 lnode is for_struct, rnode is stuff to run */
  Node_K_break,		/* 46 no subs */
  Node_K_continue,	/* 47 no stuff */
  Node_K_print,		/* 48 lnode is exp_list, rnode is redirect */
  Node_K_printf,	/* 49 lnode is exp_list, rnode is redirect */
  Node_K_next,		/* 59 no subs */
  Node_K_exit,		/* 51 subnode is return value, or NULL */

  /* I/O redirection for print statements */
  Node_redirect_output,	/* 52 subnode is where to redirect */
  Node_redirect_append,	/* 53 subnode is where to redirect */
  Node_redirect_pipe,	/* 54 subnode is where to redirect */

  /* Variables */
  Node_var,		/* 55 rnode is value, lnode is array stuff */
  Node_var_array,	/* 56 array is ptr to elements, asize num of eles */

  /* Builtins   subnode is explist to work on, proc is func to call */
  Node_builtin,		/* 57 */

  /* pattern: conditional ',' conditional ;  lnode of Node_line_range is
   * the two conditionals (Node_cond_pair), other word (rnode place) is
   * a flag indicating whether or not this range has been entered.
   * (jfw@eddie.mit.edu)
   */
  Node_line_range,	/* 58 */
} NODETYPE;

typedef struct exp_node {
  NODETYPE type;
  union {
  	struct {
	  	struct exp_node *lptr;
		union {
			struct exp_node *rptr;
			struct exp_node *(* pptr)();
			struct re_pattern_buffer *preg;
			struct for_loop_header *hd;
			struct ahash **av;
			int r_ent;	/* range entered (jfw) */
		} r;
	} nodep;
	struct {
		struct exp_node **ap;
		int as;
	} ar;
	struct {
		char *sp;
		short slen,sref;
	} str;
	AWKNUM fltnum;
  } sub;
} NODE;

#define lnode	sub.nodep.lptr
#define rnode	sub.nodep.r.rptr

#define subnode	lnode
#define proc	sub.nodep.r.pptr

#define reexp	lnode
#define rereg	sub.nodep.r.preg

#define forsub	lnode
#define forloop	sub.nodep.r.hd

#define array	sub.ar.ap
#define arrsiz	sub.ar.as

#define stptr	sub.str.sp
#define stlen	sub.str.slen
#define stref	sub.str.sref

#define numbr	sub.fltnum

#define var_value lnode
#define var_array sub.nodep.r.av

#define condpair lnode
#define triggered sub.nodep.r.r_ent

NODE *newnode(), *dupnode();
NODE *node(), *snode(), *make_number(), *make_string();
NODE *mkrangenode();	/* to remove the temptation to use sub.nodep.r.rptr
			 * as a boolean flag, or to call node() with a 0 and
			 * hope that it will store correctly as an int. (jfw)
			 */
NODE *tmp_string(),*tmp_number();
NODE *variable(), *append_right();

NODE *tree_eval();

struct re_pattern_buffer *make_regexp();

extern NODE *Nnull_string;

#ifdef FAST
double atof();
NODE *strforce();
#define force_number(x)		((x)->type==Node_number ? (x)->numbr : atof((x)->stptr))
#define force_string(x)		((x)->type==Node_number ? (strforce(x)) : (x))
#define tmp_node(ty)		(global_tmp=(NODE *)obstack_alloc(&temp_strings,sizeof(NODE)),global_tmp->type=ty)
#define tmp_number(n)		(tmp_node(Node_number),global_tmp->numbr=(n),global_tmp)
/* #define tmp_string(s,len)	(tmp_node(Node_temp_string),global_tmp->stref=1,global_tmp->stlen=len,global_tmp->stptr=(char *)obstack_alloc(&temp_strings,len+1),bcopy(s,global_tmp->stptr,len),global_tmp->stptr[len]='\0',global_tmp) */
NODE *global_tmp;
#else
AWKNUM	force_number();
NODE	*force_string();
#endif

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
	NODE	*name,
		*symbol,
		*value;
};


typedef struct for_loop_header {
  NODE *init;
  NODE *cond;
  NODE *incr;
} FOR_LOOP_HEADER;

FOR_LOOP_HEADER *make_for_loop();

#define ADD_ONE_REFERENCE(s) ++(s)->stref
#define FREE_ONE_REFERENCE(s) {\
  if(s==Nnull_string) {\
    fprintf(stderr,"Free_Nnull_string %d",(s)->stref);\
  }\
  if (--(s)->stref == 0) {\
    free((char *)((s)->stptr));\
    free((char *)s);\
  }\
}
/* #define FREE_ONE_REFERENCE(s) {if (--(s)->stref == 0) {printf("FREE %x\n",s);free((s)->stptr);free(s);}} */
