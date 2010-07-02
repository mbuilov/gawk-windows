/*
   Debug.c -- Various debugging routines

   Copyright (C) 1986 Free Software Foundation
     Written by Jay Fenlason, December 1986

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
#include "awk.h"
#include <stdio.h>

#ifndef FAST

extern NODE **fields_arr;
extern f_arr_siz;


/* This is all debugging stuff.  Ignore it and maybe it'll go away. */

/* Some of it could be turned into a really cute trace command, if anyone
   wants to.  */
char *nnames[] = {
	"Illegal Node",
	"Times",	"Divide",	"Mod",		"Plus",		"Minus",
	"Cond-pair" /* jfw */,	"Subscript",	"Concat",
	"++Pre",	"--Pre",	"Post++",
	"Post--",	"Uminus",	"Field",
	"Assign",	"*=",		"/=",		"%=",
	"+=",		"-=",
	"And",		"Or",
	"Equal",	"!=",		"Less",		"Greater",	"<=",		">=",
	"Not",
	"Match",	"Nomatch",
	"String",	"TmpString",	"Number",
	"Rule_list",	"Rule_node",	"State_list",	"If_branches",	"Exp_list",
	"BEGIN",	"END",		"IF",		"WHILE",	"FOR",
	"arrayfor",	"BREAK",	"CONTINUE",	"PRINT",	"PRINTF",
	"next",		"exit",		"redirect",	"Append",
	"Pipe",		"variable",	"Varray",	"builtin",
	"Line-range" /*jfw*/,
};

ptree(n)
{
	print_parse_tree((NODE *)n);
}

pt()
{
	int x;
	scanf("%x",&x);
	printf("0x%x\n",x);
	print_parse_tree((NODE *)x);
	fflush(stdout);
}

static depth = 0;
print_parse_tree(ptr)
NODE *ptr;
{
	register int n;

	if(!ptr) {
		printf("NULL\n");
		return;
	}
	if((int)(ptr->type)<0 || (int)(ptr->type)>sizeof(nnames)/sizeof(nnames[0])) {
		printf("(0x%x Type %d??)\n",ptr,ptr->type);
		return;
	}
	printf("(%d)%*s",depth,depth,"");
	switch((int)ptr->type) {
	case (int)Node_string:
	case (int)Node_temp_string:
		printf("(0x%x String \"%.*s\")\n",ptr,ptr->stlen,ptr->stptr);
		return;
	case (int)Node_number:
		printf("(0x%x Number %g)\n",ptr,ptr->numbr);
		return;
	case (int)Node_var_array:
		printf("(0x%x Array of %d)\n",ptr,ptr->arrsiz);
		for(n=0;n<ptr->arrsiz;n++) {
			printf("'");
			print_simple((ptr->array)[n*2],stdout);
			printf("' is '");
			print_simple((ptr->array)[n*2+1],stdout);
			printf("'\n");
		}
		return;
	}
	if(ptr->lnode) printf("0x%x = left<--",ptr->lnode);
	printf("(0x%x %s.%d)",ptr,nnames[(int)(ptr->type)],ptr->type);
	if(ptr->rnode) printf("-->right = 0x%x",ptr->rnode);
	printf("\n");
	depth++;
	if(ptr->lnode)
		print_parse_tree(ptr->lnode);
	switch((int)ptr->type) {
	case (int)Node_line_range:	/* jfw */
	case (int)Node_match:
	case (int)Node_nomatch:
		break;
	case (int)Node_builtin:
		printf("Builtin: %d\n",ptr->proc);	/* jfw: was \N */
		break;
	case (int)Node_K_for:
	case (int)Node_K_arrayfor:
		printf("(%s:)\n",nnames[(int)(ptr->type)]);
		print_parse_tree(ptr->forloop->init);
		printf("looping:\n");
		print_parse_tree(ptr->forloop->cond);
		printf("doing:\n");
		print_parse_tree(ptr->forloop->incr);
		break;
	default:
		if(ptr->rnode)
			print_parse_tree(ptr->rnode);
		break;
	}
	--depth;
}
#endif

#ifndef FAST
/*
 * print out all the variables in the world
 */

dump_vars()
{
  register int n;
  register HASHNODE *buc;

  printf("Fields:");
  dump_fields();
  printf("Vars:\n");
  for(n=0;n<HASHSIZE;n++) {
    for(buc=variables[n];buc;buc=buc->next) {
      printf("'%.*s': ",buc->length,buc->name);
      print_simple(buc->value->var_value,stdout);
      printf(":");
      print_parse_tree(buc->value->lnode);
      /* print_parse_tree(buc->value); */
    }
  }
  printf("End\n");
}
#endif

#ifndef FAST
dump_fields()
{
	register NODE	**p;
	register int	n;

	printf("%d fields\n",f_arr_siz);
	for(n=0,p= &fields_arr[0];n<f_arr_siz;n++,p++) {
		printf("$%d is '",n);
		print_simple(*p,stdout);
		printf("'\n");
	}
}
#endif

#ifndef FAST
/*VARARGS1*/
print_debug(str,n)
char *str;
{
	extern int debugging;

	if(debugging)
		printf("%s:%d\n",str,n);
}

int indent = 0;

print_a_node(ptr)
NODE *ptr;
{
	NODE *p1;
	char *str,*str2;
	int n;
	HASHNODE *buc;

	if(!ptr) return;	/* don't print null ptrs */
	switch(ptr->type) {
	case Node_number:
		printf("%g",ptr->numbr);
		return;
	case Node_string:
		printf("\"%.*s\"",ptr->stlen,ptr->stptr);
		return;
	case Node_times:
		str="*";
		goto pr_twoop;
	case Node_quotient:
		str="/";
		goto pr_twoop;
	case Node_mod:
		str="%";
		goto pr_twoop;
	case Node_plus:
		str="+";
		goto pr_twoop;
	case Node_minus:
		str="-";
		goto pr_twoop;
	case Node_concat:
		str=" ";
		goto pr_twoop;
	case Node_assign:
		str="=";
		goto pr_twoop;
	case Node_assign_times:
		str="*=";
		goto pr_twoop;
	case Node_assign_quotient:
		str="/=";
		goto pr_twoop;
	case Node_assign_mod:
		str="%=";
		goto pr_twoop;
	case Node_assign_plus:
		str="+=";
		goto pr_twoop;
	case Node_assign_minus:
		str="-=";
		goto pr_twoop;
	case Node_and:
		str="&&";
		goto pr_twoop;
	case Node_or:
		str="||";
		goto pr_twoop;
	case Node_equal:
		str="==";
		goto pr_twoop;
	case Node_notequal:
		str="!=";
		goto pr_twoop;
	case Node_less:
		str="<";
		goto pr_twoop;
	case Node_greater:
		str=">";
		goto pr_twoop;
	case Node_leq:
		str="<=";
		goto pr_twoop;
	case Node_geq:
		str=">=";
		goto pr_twoop;

 pr_twoop:
 		print_a_node(ptr->lnode);
		printf("%s",str);
		print_a_node(ptr->rnode);
		return;

	case Node_not:
		str="!";
		str2="";
		goto pr_oneop;
	case Node_field_spec:
		str="$(";
		str2=")";
		goto pr_oneop;
	case Node_postincrement:
		str="";
		str2="++";
		goto pr_oneop;
	case Node_postdecrement:
		str="";
		str2="--";
		goto pr_oneop;
	case Node_preincrement:
		str="++";
		str2="";
		goto pr_oneop;
	case Node_predecrement:
		str="--";
		str2="";
		goto pr_oneop;
 pr_oneop:
		printf(str);
		print_a_node(ptr->subnode);
		printf(str2);
		return;

	case Node_expression_list:
		print_a_node(ptr->lnode);
		if(ptr->rnode) {
			printf(",");
			print_a_node(ptr->rnode);
		}
		return;

	case Node_var:
		for(n=0;n<HASHSIZE;n++) {
			for(buc=variables[n];buc;buc=buc->next) {
				if(buc->value==ptr) {
					printf("%.*s",buc->length,buc->name);
					n=HASHSIZE;
					break;
				}
			}
		}
		return;
	case Node_subscript:
		print_a_node(ptr->lnode);
		printf("[");
		print_a_node(ptr->rnode);
		printf("]");
		return;
	case Node_builtin:
		printf("some_builtin(");
		print_a_node(ptr->subnode);
		printf(")");
		return;

	case Node_statement_list:
		printf("{\n");
		indent++;
		for(n=indent;n;--n)
			printf("  ");
		while(ptr) {
			print_maybe_semi(ptr->lnode);
			if(ptr->rnode)
				for(n=indent;n;--n)
					printf("  ");
			ptr=ptr->rnode;
		}
		--indent;
		for(n=indent;n;--n)
			printf("  ");
		printf("}\n");
		for(n=indent;n;--n)
			printf("  ");
		return;

	case Node_K_if:
		printf("if(");
		print_a_node(ptr->lnode);
		printf(") ");
		ptr=ptr->rnode;
		if(ptr->lnode->type==Node_statement_list) {
			printf("{\n");
			indent++;
			for(p1=ptr->lnode;p1;p1=p1->rnode) {
				for(n=indent;n;--n)
					printf("  ");
				print_maybe_semi(p1->lnode);
			}
			--indent;
			for(n=indent;n;--n)
				printf("  ");
			if(ptr->rnode) {
				printf("} else ");
			} else {
				printf("}\n");
				return;
			}
		} else {
			print_maybe_semi(ptr->lnode);
			if(ptr->rnode) {
				for(n=indent;n;--n)
					printf("  ");
				printf("else ");
			} else return;
		}
		if(!ptr->rnode) return;
		deal_with_curls(ptr->rnode);
		return;

	case Node_K_for:
		printf("for(");
		print_a_node(ptr->forloop->init);
		printf(";");
		print_a_node(ptr->forloop->cond);
		printf(";");
		print_a_node(ptr->forloop->incr);
		printf(") ");
		deal_with_curls(ptr->forsub);
		return;
	case Node_K_arrayfor:
		printf("for(");
		print_a_node(ptr->forloop->init);
		printf(" in ");
		print_a_node(ptr->forloop->incr);
		printf(") ");
		deal_with_curls(ptr->forsub);
		return;

	case Node_K_printf:
		printf("printf(");
		print_a_node(ptr->lnode);
		printf(")");
		return;
	case Node_K_print:
		printf("print(");
		print_a_node(ptr->lnode);
		printf(")");
		return;
	case Node_K_next:
		printf("next");
		return;
	case Node_K_break:
		printf("break");
		return;
	default:
		print_parse_tree(ptr);
		return;
	}
}

print_maybe_semi(ptr)
NODE *ptr;
{
	print_a_node(ptr);
	switch(ptr->type) {
	case Node_K_if:
	case Node_K_for:
	case Node_K_arrayfor:
	case Node_statement_list:
		break;
	default:
		printf(";\n");
		break;
	}
}
deal_with_curls(ptr)
NODE *ptr;
{
	int n;

	if(ptr->type==Node_statement_list) {
		printf("{\n");
		indent++;
		while(ptr) {
			for(n=indent;n;--n)
				printf("  ");
			print_maybe_semi(ptr->lnode);
			ptr=ptr->rnode;
		}
		--indent;
		for(n=indent;n;--n)
			printf("  ");
		printf("}\n");
	} else {
		print_maybe_semi(ptr);
	}
}

NODE *
do_prvars()
{
	dump_vars();
	return Nnull_string;
}

NODE *
do_bp()
{
	return Nnull_string;
}

#endif
