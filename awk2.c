/*
 * awk2 --- gawk parse tree interpreter
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

#include <setjmp.h>
#include <stdio.h>

#ifdef SYSV
/* nasty nasty berkelixm */
#define _setjmp setjmp
#define _longjmp longjmp
#endif

#include "awk.h"

NODE **get_lhs();

extern NODE dumb[],*OFMT_node;
/* BEGIN and END blocks need special handling, because we are handed them
 * as raw Node_statement_lists, not as Node_rule_lists (jfw)
 */
extern NODE *begin_block, *end_block;
NODE *do_sprintf();
extern struct obstack other_stack;


#define min(a,b) ((a) < (b) ? (a) : (b))

/* More of that debugging stuff */
#ifdef FAST
#define DEBUG(X)
#else
#define DEBUG(X) print_debug X
#endif

/* longjmp return codes, must be nonzero */
/* Continue means either for loop/while continue, or next input record */
#define TAG_CONTINUE 1
/* Break means either for/while break, or stop reading input */
#define TAG_BREAK 2

/* the loop_tag_valid variable allows continue/break-out-of-context
 * to be caught and diagnosed (jfw) */
#define PUSH_BINDING(stack, x) (bcopy ((char *)(x), (char *)(stack), sizeof (jmp_buf)), loop_tag_valid++)
#define RESTORE_BINDING(stack, x) (bcopy ((char *)(stack), (char *)(x), sizeof (jmp_buf)), loop_tag_valid--)

/* for "for(iggy in foo) {" */
struct search {
	int	numleft;
	AHASH	**arr_ptr;
	AHASH	*bucket;
	NODE	*symbol;
	NODE	*retval;
};

struct search *assoc_scan(),*assoc_next();
/* Tree is a bunch of rules to run.
   Returns zero if it hit an exit() statement */
interpret (tree)
     NODE *tree;
{
  register NODE *t;			/* temporary */

  auto jmp_buf loop_tag_stack;	/* shallow binding stack for loop_tag */
  static jmp_buf loop_tag;	/* always the current binding */
  static int loop_tag_valid = 0;/* nonzero when loop_tag valid (jfw) */

  static jmp_buf rule_tag;	/* tag the rule currently being run,
				   for NEXT and EXIT statements.  It is
				   static because there are no nested rules */

  register NODE **lhs;	/* lhs == Left Hand Side for assigns, etc */
  register struct search *l;	/* For array_for */


  extern struct obstack temp_strings;
  extern char *ob_dummy;
  NODE *do_printf();

  /* clean up temporary strings created by evaluating expressions in
     previous recursive calls */
  obstack_free (&temp_strings, ob_dummy);

  if(tree == NULL)
    return 1;
  switch (tree->type) {
#ifndef FAST
    /* Can't run these! */
  case Node_illegal:
  case Node_rule_node:
  case Node_if_branches:
  case Node_expression_list:
  case Node_K_BEGIN:
  case Node_K_END:
  case Node_redirect_output:
  case Node_redirect_append:
  case Node_redirect_pipe:
  case Node_var_array:
    abort();
#endif

  case Node_rule_list:
    for (t = tree; t != NULL; t = t->rnode) {
      switch (_setjmp(rule_tag)) {
      case 0:			/* normal non-jump */
	if (eval_condition (t->lnode->lnode)) {
	  DEBUG(("Found a rule",t->lnode->rnode));
	  if (t->lnode->rnode == NULL) {
	    /* special case: pattern with no action is equivalent to 
	     * an action of {print} (jfw) */
	    NODE printnode;
	    printnode.type = Node_K_print;
	    printnode.lnode = NULL;
	    printnode.rnode = NULL;
	    hack_print_node(&printnode);
	  } else
	    (void)interpret (t->lnode->rnode);
	}
	break;
      case TAG_CONTINUE:	/* NEXT statement */
	return 1;
      case TAG_BREAK:
        return 0;
      }
    }
    break;

  case Node_statement_list:
    /* print_a_node(tree); */
    /* because BEGIN and END do not have Node_rule_list nature, yet can
     * have exits and nexts, we special-case a setjmp of rule_tag here.
     * (jfw)
     */
    if (tree == begin_block || tree == end_block) {
	switch (_setjmp(rule_tag)) {
	case TAG_CONTINUE:	/* next */
		panic("unexpected next");
		return 1;
	case TAG_BREAK:		return 0;
	}
    }
    for (t = tree; t != NULL; t = t->rnode) {
      DEBUG(("Statements",t->lnode));
      (void)interpret (t->lnode);
    }
    break;

  case Node_K_if:
    DEBUG(("IF",tree->lnode));
    if (eval_condition(tree->lnode)) {
      DEBUG(("True",tree->rnode->lnode));
      (void)interpret (tree->rnode->lnode);
    } else {
      DEBUG(("False",tree->rnode->rnode));
      (void)interpret (tree->rnode->rnode);
    }
    break;

  case Node_K_while:
    PUSH_BINDING (loop_tag_stack, loop_tag);

    DEBUG(("WHILE",tree->lnode));
    while (eval_condition (tree->lnode)) {
      switch (_setjmp (loop_tag)) {
      case 0:			/* normal non-jump */
        DEBUG(("DO",tree->rnode));
	(void)interpret (tree->rnode);
	break;
      case TAG_CONTINUE:	/* continue statement */
	break;
      case TAG_BREAK:		/* break statement */
	RESTORE_BINDING (loop_tag_stack, loop_tag);
	return 1;
#ifndef FAST
      default:
	abort ();		/* never happens */
#endif
      }
    }
    RESTORE_BINDING (loop_tag_stack, loop_tag);
    break;

  case Node_K_for:
    PUSH_BINDING (loop_tag_stack, loop_tag);

    DEBUG(("FOR",tree->forloop->init));
    (void)interpret (tree->forloop->init);

    DEBUG(("FOR.WHILE",tree->forloop->cond));
    while (eval_condition (tree->forloop->cond)) {
      switch (_setjmp (loop_tag)) {
      case 0:			/* normal non-jump */
        DEBUG(("FOR.DO",tree->lnode));
	(void)interpret (tree->lnode);
	/* fall through */
      case TAG_CONTINUE:	/* continue statement */
        DEBUG(("FOR.INCR",tree->forloop->incr));
	(void)interpret (tree->forloop->incr);
	break;
      case TAG_BREAK:		/* break statement */
	RESTORE_BINDING (loop_tag_stack, loop_tag);
	return 1;
#ifndef FAST
      default:
	abort ();		/* never happens */
#endif
      }
    }
    RESTORE_BINDING (loop_tag_stack, loop_tag);
    break;

  case Node_K_arrayfor:
#define hakvar forloop->init
#define arrvar forloop->incr
	PUSH_BINDING(loop_tag_stack, loop_tag);
	DEBUG(("AFOR.VAR",tree->hakvar));
	lhs=get_lhs(tree->hakvar);
	do_deref();
	for(l=assoc_scan(tree->arrvar);l;l=assoc_next(l)) {
		*lhs=dupnode(l->retval);
		DEBUG(("AFOR.NEXTIS",*lhs));
		switch(_setjmp(loop_tag)) {
		case 0:
			DEBUG(("AFOR.DO",tree->lnode));
			(void)interpret(tree->lnode);
		case TAG_CONTINUE:
			break;

		case TAG_BREAK:
			RESTORE_BINDING(loop_tag_stack, loop_tag);
			return 1;
#ifndef FAST
		default:
			abort();
#endif
		}
	}
	RESTORE_BINDING(loop_tag_stack, loop_tag);
	break;

  case Node_K_break:
    DEBUG(("BREAK",NULL));
    if (loop_tag_valid == 0)	/* jfw */
	panic("unexpected break or continue");
    _longjmp (loop_tag, TAG_BREAK);
    break;

  case Node_K_continue:
    DEBUG(("CONTINUE",NULL));
    if (loop_tag_valid == 0)	/* jfw */
	panic("unexpected break or continue");
    _longjmp (loop_tag, TAG_CONTINUE);
    break;

  case Node_K_print:
    DEBUG(("PRINT",tree));
    (void)hack_print_node (tree);
    break;

  case Node_K_printf:
    DEBUG(("PRINTF",tree));
    (void)do_printf(tree);
    break;

  case Node_K_next:
    DEBUG(("NEXT",NULL));
    _longjmp (rule_tag, TAG_CONTINUE);
    break;

  case Node_K_exit:
    /* The unix awk doc says to skip the rest of the input.  Does that
       mean after performing all the rules on the current line?
       Unix awk quits immediately, so this does too. */
    /* The UN*X exit can also take an optional arg return code.  We don't */
    /* Well, we parse it, but never *DO* it */
    DEBUG(("EXIT",NULL));
    _longjmp (rule_tag, TAG_BREAK);
    break;

  default:
    /* Appears to be an expression statement.  Throw away the value. */
    DEBUG(("E",NULL));
    (void)tree_eval (tree);
    break;
  }
  return 1;
}

/* evaluate a subtree, allocating strings on a temporary stack. */
/* This used to return a whole NODE, instead of a ptr to one, but that
   led to lots of obnoxious copying.  I got rid of it (JF) */
NODE *
tree_eval (tree)
     NODE *tree;
{
  register NODE *r, *t1, *t2;		/* return value and temporary subtrees */
  register NODE **lhs;
  static AWKNUM x;		/* Why are these static? */
  extern struct obstack temp_strings;

  if(tree == NULL) {
    DEBUG(("NULL",NULL));
    return Nnull_string;
  }
  switch (tree->type) {
    /* trivial data */
  case Node_string:
  case Node_number:
    DEBUG(("DATA",tree));
    return tree;

    /* Builtins */
  case Node_builtin:
    DEBUG(("builtin",tree));
    return ((*tree->proc)(tree->subnode));

    /* unary operations */

  case Node_var:
  case Node_subscript:
  case Node_field_spec:
    DEBUG(("var_type ref",tree));
    lhs=get_lhs(tree);
    return *lhs;

  case Node_preincrement:
  case Node_predecrement:
    DEBUG(("+-X",tree));
    lhs=get_lhs(tree->subnode);
    assign_number(lhs,force_number(*lhs) + (tree->type==Node_preincrement ? 1.0 : -1.0));
    return *lhs;

  case Node_postincrement:
  case Node_postdecrement:
    DEBUG(("X+-",tree));
    lhs=get_lhs(tree->subnode);
    x = force_number(*lhs);
    assign_number (lhs, x + (tree->type==Node_postincrement ? 1.0 : -1.0));
    return tmp_number(x);

  case Node_unary_minus:
    DEBUG(("UMINUS",tree));
    return tmp_number(-force_number(tree_eval(tree->subnode)));

    /* assignments */
  case Node_assign:
    DEBUG(("ASSIGN",tree));
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    *lhs= dupnode(r);
    do_deref();
    /* FOO we have to regenerate $0 here! */
    if(tree->lnode->type==Node_field_spec)
      fix_fields();
    return r;
    /* other assignment types are easier because they are numeric */
  case Node_assign_times:
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    assign_number(lhs, force_number(*lhs) * force_number(r));
    do_deref();
    return r;

  case Node_assign_quotient:
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    assign_number(lhs, force_number(*lhs) / force_number(r));
    do_deref();
    return r;

  case Node_assign_mod:
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    assign_number(lhs, (AWKNUM)(((int) force_number(*lhs)) % ((int) force_number(r))));
    do_deref();
    return r;

  case Node_assign_plus:
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    assign_number(lhs, force_number(*lhs) + force_number(r));
    do_deref();
    return r;

  case Node_assign_minus:
    r = tree_eval (tree->rnode);
    lhs=get_lhs(tree->lnode);
    assign_number(lhs, force_number(*lhs) - force_number(r));
    do_deref();
    return r;
  }
  /* Note that if TREE is invalid, gAWK will probably bomb in one of these
     tree_evals here.  */
  /* evaluate subtrees in order to do binary operation, then keep going */
  t1 = tree_eval (tree->lnode);
  t2 = tree_eval (tree->rnode);

  switch (tree->type) {

  case Node_concat:
    t1=force_string(t1);
    t2=force_string(t2);

    r=(NODE *)obstack_alloc(&temp_strings,sizeof(NODE));
    r->type=Node_temp_string;
    r->stlen=t1->stlen+t2->stlen;
    r->stref=1;
    r->stptr=(char *)obstack_alloc(&temp_strings,r->stlen+1);
    bcopy(t1->stptr,r->stptr,t1->stlen);
    bcopy(t2->stptr,r->stptr+t1->stlen,t2->stlen);
    r->stptr[r->stlen]='\0';
    return r;

  case Node_times:
    return tmp_number(force_number(t1) * force_number(t2));

  case Node_quotient:
    x=force_number(t2);
    if(x==(AWKNUM)0) return tmp_number((AWKNUM)0);
    else return tmp_number(force_number(t1) / x);

  case Node_mod:
    x=force_number(t2);
    if(x==(AWKNUM)0) return tmp_number((AWKNUM)0);
    return tmp_number((AWKNUM)	/* uggh... */
      (((int) force_number(t1)) % ((int) x)));

  case Node_plus:
    return tmp_number(force_number(t1) + force_number(t2));

  case Node_minus:
    return tmp_number(force_number(t1) - force_number(t2));

#ifndef FAST
  default:
    fprintf (stderr, "internal error: illegal numeric operation\n");
    abort ();
#endif
  }
  return 0;
}

/* We can't dereference a variable until after we've given it its new value.
   This variable points to the value we have to free up */
NODE *deref;

/* This returns a POINTER to a node pointer.
   *get_lhs(ptr) is the current value of the var, or where to store the
   var's new value */

NODE **
get_lhs(ptr)
NODE *ptr;
{
  register NODE *subexp;
  register NODE	**aptr;
  register int	num;
  extern NODE **fields_arr;
  extern f_arr_siz;
  NODE **assoc_lookup();
  extern char f_empty[];	/* jfw */

#ifndef FAST
  if(ptr == NULL)
    abort();
#endif
  deref = NULL;
  switch(ptr->type) {
  case Node_var:
    deref=ptr->var_value;
    return &(ptr->var_value);

  case Node_field_spec:
    num=(int)force_number(tree_eval(ptr->lnode));
    if(num<0) num=0;		/* JF what should I do? */
    if(num>f_arr_siz)
      set_field(num,f_empty,0);	/* jfw: so blank_strings can be simpler */
    deref = NULL;
    return &fields_arr[num];

  case Node_subscript:
    subexp = tree_eval(ptr->rnode);
    aptr=assoc_lookup(ptr->lnode,subexp);
    deref= *aptr;
    return aptr;
  }
#ifndef FAST
  abort();
  return 0;
#endif
}

do_deref()
{
  if(deref) {
    switch(deref->type) {
    case Node_string:
      if(deref!=Nnull_string)
        FREE_ONE_REFERENCE(deref);
      break;
    case Node_number:
      free((char *)deref);
      break;
#ifndef FAST
    default:
      abort();
#endif
    }
    deref = 0;
  }
}

/* This makes numeric operations slightly more efficient.
   Just change the value of a numeric node, if possible */
assign_number (ptr, value)
NODE **ptr;
AWKNUM value;
{
  switch ((*ptr)->type) {
  case Node_string:
    if(*ptr!=Nnull_string)
      FREE_ONE_REFERENCE (*ptr);
  case Node_temp_string:	/* jfw: dont crash if we say $2 += 4 */
    *ptr=make_number(value);
    return;
  case Node_number:
    (*ptr)->numbr = value;
    deref=0;
    break;
#ifndef FAST
  default:
    printf("assign_number nodetype %d\n", (*ptr)->type); /* jfw: add mesg. */
    abort ();
#endif
  }
}


/* Routines to deal with fields */
#define ORIG_F	30

NODE	**fields_arr;
NODE	*fields_nodes;
int	f_arr_siz;
char	f_empty [] = "";

init_fields()
{
	register NODE **tmp;
	register NODE *xtmp;

	f_arr_siz=ORIG_F;
	fields_arr=(NODE **)malloc(ORIG_F * sizeof(NODE *));
	fields_nodes=(NODE *)malloc(ORIG_F * sizeof(NODE));
	tmp= &fields_arr[f_arr_siz];
	xtmp= &fields_nodes[f_arr_siz];
	while(--tmp>= &fields_arr[0]) {
		--xtmp;
		*tmp=xtmp;
		xtmp->type=Node_temp_string;
		xtmp->stlen=0;
		xtmp->stref=1;
		xtmp->stptr=f_empty;
	}
}

blank_fields()
{
	register NODE **tmp;
	extern char *parse_end;

	tmp= &fields_arr[f_arr_siz];
	while(--tmp>= &fields_arr[0]) {
		switch(tmp[0]->type) {
		case Node_number:
			free((char *)*tmp);
			*tmp= &fields_nodes[tmp-fields_arr];
			break;
		case Node_string:
			if(*tmp!=Nnull_string)
				FREE_ONE_REFERENCE(*tmp);
			*tmp= &fields_nodes[tmp-fields_arr];
			break;
		case Node_temp_string:
			break;
#ifndef FAST
		default:
			abort();
#endif
		}
		if ((*tmp)->stptr != f_empty) {	/* jfw */
			/*Then it was assigned a string with set_field */
			/*out of a private buffer to inrec, so don't free it*/
			(*tmp)->stptr = f_empty;
			(*tmp)->stlen = 0;
			(*tmp)->stref = 1;
		}
		/* *tmp=Nnull_string; */
	}
	/* Free the strings */
	obstack_free(&other_stack,parse_end);
}

/* Danger!  Must only be called for fields we know have just been blanked,
   or fields we know don't exist yet.  */
set_field(n,str,len)
char *str;
{
	NODE *field_string();

	if(n>f_arr_siz) {
		int t;

		fields_arr=(NODE **)realloc((char *)fields_arr,(n+1)*sizeof(NODE *));
		fields_nodes=(NODE *)realloc((char *)fields_nodes,(n+1)*sizeof(NODE));
		for(t=f_arr_siz;t<=n;t++) {
			fields_arr[t]= &fields_nodes[t];
			fields_nodes[t].type=Node_temp_string;
			fields_nodes[t].stlen=0;
			fields_nodes[t].stref=1;
			fields_nodes[t].stptr=f_empty;
		}
		f_arr_siz=n+1;
	}
	fields_nodes[n].stlen=len;
	if(n==0) {
		fields_nodes[n].stptr=(char*)obstack_alloc(&other_stack,len+1);
		bcopy(str,fields_nodes[n].stptr,len);
		fields_nodes[n].stptr[len]='\0';
	} else {
		fields_nodes[n].stptr=str;
		str[len]='\0';
	}
}

#ifdef DONTDEF
/* Nodes created with this will go away when the next input line is read */
NODE *
field_string(s,len)
char *s;
{
	register NODE *r;

	r=(NODE *)obstack_alloc(&other_stack,sizeof(NODE));
	r->type=Node_temp_string;
	r->stref=1;
	r->stlen=len;
	r->stptr=(char*)obstack_alloc(&other_stack,len+1);
	bcopy(s,r->stptr,len);
	/* r->stptr=s;
	r->stptr[len]='\0'; */

	return r;
}
#endif

/* Someone assigned a value to $(something).  Fix up $0 to be right */
fix_fields()
{
	register int tlen;
	register NODE	*tmp;
	NODE	*ofs;
	char	*ops;
	register char	*cops;
	register NODE	**ptr,**maxp;
	extern NODE *OFS_node;

	maxp=0;
	tlen=0;
	ofs=force_string(*get_lhs(OFS_node));
	ptr= &fields_arr[f_arr_siz];
	while(--ptr> &fields_arr[0]) {
		tmp=force_string(*ptr);
		tlen+=tmp->stlen;
		if(tmp->stlen && !maxp)
			maxp=ptr;
	}
	if(!maxp) {
		if (fields_arr[0] != fields_nodes)
			FREE_ONE_REFERENCE(fields_arr[0]);
		fields_arr[0]=Nnull_string;
		return;
	}
	
	tlen+=((maxp-fields_arr)-1)*ofs->stlen;
	ops=(char *)malloc(tlen+1);
	cops=ops;
	for(ptr= &fields_arr[1];ptr<=maxp;ptr++) {
		tmp=force_string(*ptr);
		bcopy(tmp->stptr,cops,tmp->stlen);
		cops+=tmp->stlen;
		if(ptr!=maxp) {
			bcopy(ofs->stptr,cops,ofs->stlen);
			cops+=ofs->stlen;
		}
	}
	tmp=newnode(Node_string);
	tmp->stptr=ops;
	tmp->stlen=tlen;
	tmp->stref=1;
	tmp->stptr[tlen]='\0';
	/* don't free unless it's new */
	if (fields_arr[0] != fields_nodes)
		FREE_ONE_REFERENCE(fields_arr[0]);
	fields_arr[0]=tmp;
}


/* Is TREE true or false?  Returns 0==false, non-zero==true */
int
eval_condition (tree)
NODE *tree;
{
  register int	di;
  register NODE	*t1,*t2;

  if(tree==NULL)	/* Null trees are the easiest kinds */
    return 1;
  switch (tree->type) {
    /* Maybe it's easy; check and see. */
    /* BEGIN and END are always false */
  case Node_K_BEGIN:
    return 0;
    break;

  case Node_K_END:
    return 0;
    break;

  case Node_and:
    return eval_condition (tree->lnode)
      && eval_condition (tree->rnode);

  case Node_or:
    return eval_condition (tree->lnode)
      || eval_condition (tree->rnode);
    
  case Node_not:
    return !eval_condition (tree->lnode);

  /* Node_line_range is kind of like Node_match, EXCEPT:
   * the lnode field (more properly, the condpair field) is a node of
   * a Node_cond_pair; whether we evaluate the lnode of that node or the
   * rnode depends on the triggered word.  More precisely:  if we are not
   * yet triggered, we tree_eval the lnode; if that returns true, we set
   * the triggered word.  If we are triggered (not ELSE IF, note), we
   * tree_eval the rnode, clear triggered if it succeeds, and perform our
   * action (regardless of success or failure).  We want to be able to
   * begin and end on a single input record, so this isn't an ELSE IF, as
   * noted above.
   * This feature was implemented by John Woods, jfw@eddie.mit.edu, during
   * a rainy weekend.
   */
  case Node_line_range:
	if (!tree->triggered)
	    if (!eval_condition(tree->condpair->lnode))
		return 0;
	    else
		tree->triggered = 1;
	/* Else we are triggered */
	if (eval_condition(tree->condpair->rnode))
	    tree->triggered = 0;
	return 1;
  }

  /* Could just be J.random expression.
     in which case, null and 0 are false,
     anything else is true */

  switch(tree->type) {
  case Node_match:
  case Node_nomatch:
  case Node_equal:
  case Node_notequal:
  case Node_less:
  case Node_greater:
  case Node_leq:
  case Node_geq:
  	break;

  default:	/* This is so 'if(iggy)', etc, will work */
    /* Non-zero and non-empty are true */
    t1=tree_eval(tree);
    switch(t1->type) {
    case Node_number:
      return t1->numbr!=0.0;
    case Node_string:
    case Node_temp_string:
      return t1->stlen!=0;
#ifndef FAST
    default:
      abort();
#endif
    }
  }
  /* couldn't fob it off recursively, eval left subtree and
     see if it's a pattern match operation */

  t1 = tree_eval (tree->lnode);

  if (tree->type == Node_match || tree->type == Node_nomatch) {
    t1=force_string(t1);
    return (re_search (tree->rereg, t1->stptr,
		      t1->stlen, 0, t1->stlen,
		      NULL) == -1)
      ^ (tree->type == Node_match);
  }

  /* still no luck--- eval the right subtree and try binary ops */

  t2 = tree_eval (tree->rnode);

  di=cmp_nodes(t1,t2);

  switch (tree->type) {
  case Node_equal:
    return di == 0;
  case Node_notequal:
    return di != 0;
  case Node_less:
    return di < 0;
  case Node_greater:
    return di > 0;
  case Node_leq:
    return di <= 0;
  case Node_geq:
    return di >= 0;
#ifndef FAST
  default:
    fprintf(stderr,"Panic: unknown conditonal\n");
    abort ();
#endif
  }
  return 0;
}

/* FOO this doesn't properly compare "12.0" and 12.0 etc */
/* or "1E1" and 10 etc */
/* Perhaps someone should fix it.  */
/* Consider it fixed (jfw) */

/* strtod() would have been better, except (1) real awk is needlessly
 * restrictive in what strings it will consider to be numbers, and
 * (2) I couldn't find the public domain version anywhere handy.
 */
is_a_number(str)	/* does the string str have pure-numeric syntax? */
char *str;		/* don't convert it, assume that atof is better */
{
	if (*str == 0) return 1; /* null string has numeric value of0 */
		/* This is still a bug: in real awk, an explicit "" string
		 * is not treated as a number.  Perhaps it is only variables
		 * that, when empty, are also 0s.  This bug-lette here at
		 * least lets uninitialized variables to compare equal to
		 * zero like they should.
		 */
	if (*str == '-') str++;
	if (*str == 0) return 0;
	/* must be either . or digits (.4 is legal) */
	if (*str != '.' && !isdigit(*str)) return 0;
	while (isdigit(*str)) str++;
	if (*str == '.') {
		str++;
		while (isdigit(*str)) str++;
	}
	/* curiously, real awk DOESN'T consider "1E1" to be equal to 10!
	 * Or even equal to 1E1 for that matter!  For a laugh, try:
	 * awk 'BEGIN {if ("1E1" == 1E1) print "eq"; else print "neq";exit}'
	 * Since this behavior is QUITE curious, I include the code for the
	 * adventurous.  One might also feel like skipping leading whitespace
	 * (awk doesn't) and allowing a leading + (awk doesn't).
#ifdef Allow_Exponents
	if (*str == 'e' || *str == 'E') {
		str++;
		if (*str == '+' || *str == '-') str++;
		if (!isdigit(*str)) return 0;
		while (isdigit(*str)) str++;
	}
#endif
	/* if we have digested the whole string, we are successful */
	return (*str == 0);
}

cmp_nodes(t1,t2)
NODE *t1,*t2;
{
  register int	di;
  register AWKNUM d;


  if(t1==t2) {
    return 0;
  }
#ifndef FAST
  if(!t1 || !t2) {
    abort();
    return t1 ? 1 : -1;
  }

#endif
  if (t1->type == Node_number && t2->type == Node_number) {
    d = t1->numbr - t2->numbr;
    if (d < 0.0)
      return -1;
    if (d > 0.0)
      return 1;
    return 0;
  }
  t1=force_string(t1);
  t2=force_string(t2);
  /* "real" awk treats things as numbers if they both "look" like numbers. */
  if (*t1->stptr && *t2->stptr	/* don't allow both to be empty strings(jfw)*/
  &&  is_a_number(t1->stptr) && is_a_number(t2->stptr)) {
	double atof();
	d = atof(t1->stptr) - atof(t2->stptr);
	if (d < 0.0) return -1;
	if (d > 0.0) return 1;
	return 0;
  }
  di = strncmp (t1->stptr, t2->stptr, min (t1->stlen, t2->stlen));
  if (di == 0)
    di = t1->stlen - t2->stlen;
  if(di>0) return 1;
  if(di<0) return -1;
  return 0;
}


#ifdef DONTDEF
int primes[] = {31,61,127,257,509,1021,2053,4099,8191,16381};
#endif

/* routines for associative arrays.  SYMBOL is the address of the node
   (or other pointer) being dereferenced.  SUBS is a number or string
   used as the subscript. */

/* #define ASSOC_HASHSIZE 1009	/* prime */
#define ASSOC_HASHSIZE 29
#define STIR_BITS(n) ((n) << 5 | (((n) >> 27) & 0x1f))
#define HASHSTEP(old, c) ((old << 1) + c)
#define MAKE_POS(v) (v & ~0x80000000) /* make number positive */

/* static AHASH *assoc_table[ASSOC_HASHSIZE]; */


/* Flush all the values in symbol[] before doing a split() */
assoc_clear(symbol)
NODE	*symbol;
{
	int	i;
	AHASH	*bucket,*next;

	if(symbol->var_array==0)
		return;
	for(i=0;i<ASSOC_HASHSIZE;i++) {
		for(bucket=symbol->var_array[i];bucket;bucket=next) {
			next=bucket->next;
			deref=bucket->name;
			do_deref();
			deref=bucket->value;
			do_deref();
			free((void *)bucket);
		}
		symbol->var_array[i]=0;
	}
}

/* Find SYMBOL[SUBS] in the assoc array.  Install it with value "" if it
   isn't there.  */
/* Returns a pointer ala get_lhs to where its value is stored */
NODE **
assoc_lookup (symbol, subs)
NODE	*symbol,
	*subs;
{
  int hash1 = 0, hashf(), i;
  AHASH *bucket;
  NODETYPE ty;

  if(subs->type==Node_number) {
  	hash1=(int)subs->numbr;
	ty=Node_number;
  } else {
    ty=Node_string;
    subs=force_string(subs);
    for(i=0;i<subs->stlen;i++)
      hash1=HASHSTEP(hash1,subs->stptr[i]);

    /* hash1 ^= (int) STIR_BITS((int)symbol); */
  }
  hash1 = MAKE_POS(STIR_BITS((int)hash1)) % ASSOC_HASHSIZE;

				/* this table really should grow dynamically */
  if(symbol->var_array==0) {
    symbol->var_array=(AHASH **)malloc(sizeof(AHASH *)*ASSOC_HASHSIZE);
    for(i=0;i<ASSOC_HASHSIZE;i++) {
      symbol->var_array[i]=0;
    }
  } else {
    for (bucket = symbol->var_array[hash1]; bucket; bucket = bucket->next) {
      if (bucket->name->type!= ty || cmp_nodes(bucket->name,subs))
        continue;
      return &(bucket->value);
    }
      /* Didn't find it on first pass.  Try again. */
    for (bucket = symbol->var_array[hash1]; bucket; bucket = bucket->next) {
      if (cmp_nodes(bucket->name,subs))
        continue;
      return &(bucket->value);
    }
  }
  bucket = (AHASH *) malloc(sizeof (AHASH));
  bucket->symbol = symbol;
  bucket->name = dupnode(subs);
  bucket->value = Nnull_string;
  bucket->next = symbol->var_array[hash1];
  symbol->var_array[hash1]=bucket;
  return &(bucket->value);
}

struct search *
assoc_scan(symbol)
NODE *symbol;
{
	struct search *lookat;

	if(!symbol->var_array)
		return 0;
	lookat=(struct search *)obstack_alloc(&other_stack,sizeof(struct search));
	/* lookat->symbol=symbol; */
	lookat->numleft=ASSOC_HASHSIZE;
	lookat->arr_ptr=symbol->var_array;
	lookat->bucket=symbol->var_array[0];
	return assoc_next(lookat);
}

struct search *
assoc_next(lookat)
struct search *lookat;
{
	for(;lookat->numleft;lookat->numleft--) {
		while(lookat->bucket!=0) {
			lookat->retval=lookat->bucket->name;
			lookat->bucket=lookat->bucket->next;
			return lookat;
		}
		lookat->bucket= *++(lookat->arr_ptr);
	}
	return 0;
}


#ifdef FAST
NODE *
strforce(n)
NODE *n;
{
	extern NODE dumb[],*OFMT_node;
	NODE *do_sprintf();

	dumb[1].lnode=n;
	if(OFMT_node->var_value->type!=Node_string)
	  panic("Insane value for OFMT detected.");
	return do_sprintf(&dumb[0]);
}

#else
AWKNUM
force_number (n)
NODE *n;
{
  double atof();	/* Forgetting this is bad */

  if(n==NULL)
    abort();
  switch (n->type) {
  case Node_number:
    return n->numbr;
  case Node_string:
  case Node_temp_string:
    return atof(n->stptr);
  default:
    abort ();
  }
  return 0.0;
}

NODE *
force_string(s)
NODE *s;
{
  if(s==NULL)
    abort();
  switch(s->type) {
  case Node_string:
  case Node_temp_string:
    return s;
  case Node_number:
    if((*get_lhs(OFMT_node))->type!=Node_string)
      panic("Insane value for OFMT!",0);
    dumb[1].lnode=s;
    return do_sprintf(&dumb[0]);
  default:
    abort();
  }
  return NULL;
}
#endif
