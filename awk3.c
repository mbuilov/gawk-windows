/* awk3.c -- Builtin functions and various utility procedures
   Copyright (C) 1986,1987 Free  Software Foundation
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
#include <stdio.h>
#include "awk.h"

#include <obstack.h>

extern struct obstack temp_strings;

/* This node is the cannonical null string, used everywhere */
extern NODE *Nnull_string;

/* These nodes store all the special variables gAWK uses */
NODE	*FS_node,	*NF_node,	*RS_node,	*NR_node;
NODE	*FILENAME_node,	*OFS_node,	*ORS_node,	*OFMT_node;

/* This dumb kludge is used by force_string to turn a floating point
   number into a string */
NODE	dumb[2];

NODE	**get_lhs();
FILE	*deal_redirect();

struct redirect {
	int flag;		/* JF was NODETYPE */
	NODE	*value;
	FILE	*fp;
};
struct redirect reds[20];	/* An arbitrary limit, surely, but there's an
				   arbitrary limit on open files, too.  So it
				   doesn't make much difference, does it? */


long NR;
int NF;

/* The next #define tells how you find $0.  Its a hack */
extern NODE **fields_arr;
#define WHOLELINE	fields_arr[0]

/* Set all the special variables to their initial values.  Also sets up
   the dumb[] array for force_string */
init_vars()
{
	NODE	*spc_var();
	NODE	*do_sprintf();

	FS_node=spc_var("FS",make_string(" ",1));
	NF_node=spc_var("NF",make_number(0.0));
	RS_node=spc_var("RS",make_string("\n",1));
	NR_node=spc_var("NR",make_number(0.0));
	FILENAME_node=spc_var("FILENAME",Nnull_string);
	OFS_node=spc_var("OFS",make_string(" ",1));
	ORS_node=spc_var("ORS",make_string("\n",1));
	OFMT_node=spc_var("OFMT",make_string("%.6g",4));

		/* This ugly hack is used by force_string
		   to fake a call to sprintf */
	dumb[0].type=Node_expression_list;
	dumb[0].lnode=OFMT_node;
	dumb[0].rnode= &dumb[1];
	dumb[1].type=Node_expression_list;
	dumb[1].lnode=(NODE *)0;		/* fill in the var here */
	dumb[1].rnode=(NODE *)0;
	reds[0].flag=0;			/* Don't depend on uninit data being zero, although it should be */
}

/* OFMT is special because we don't dare use force_string on it for fear of
   infinite loops.  Thus, if it isn't a string, we return the default "%.6g"
   This may or may not be the right thing to do, but its the easiest */
/* This routine isn't used!  It should be.  */
char *get_ofmt()
{
	register NODE *tmp;

	tmp= *get_lhs(OFMT_node);
	if(tmp->type!=Node_string || tmp->stlen==0)
		return "%.6g";
	return tmp->stptr;
}

int
get_fs()
{
	register NODE *tmp;

	tmp=force_string(FS_node->var_value);
	if(tmp->stlen==0) return 0;
	return *(tmp->stptr);
}

set_fs(str)
char *str;
{
	register NODE **tmp;

	tmp= get_lhs(FS_node);
	do_deref();
		/* stupid special case so -F\t works as documented in awk */
		/* even though the shell hands us -Ft.  Bleah! (jfw) */
	if (*str == 't') *str == '\t';
	*tmp=make_string(str,1);
}

set_rs(str)
char *str;
{
	register NODE **tmp;

	tmp= get_lhs(RS_node);
	do_deref();
		/* stupid special case to be consistent with -F (jfw) */
	if (*str == 't') *str == '\t';
	*tmp=make_string(str,1);
}


int
get_rs()
{
	register NODE *tmp;

	tmp=force_string(RS_node->var_value);
	if(tmp->stlen==0) return 0;
	return *(tmp->stptr);
}


/* Builtin functions */
NODE *
do_exp(tree)
NODE *tree;
{
	NODE *tmp;
	double exp();

	get_one(tree,&tmp);
	return tmp_number(exp(force_number(tmp)));
}

/* JF: I don't know what this should return. */
/* jfw: 1 if successful or by land, 0 if end of file or by sea */
NODE *
do_getline(tree)
NODE *tree;
{
	if(inrec() == 0)
		return tmp_number(1.0);
	else
		return tmp_number(0.0);
}

NODE *
do_index(tree)
NODE *tree;
{
	NODE *s1,*s2;
	register char *p1,*p2;
	register int l1,l2;

	get_two(tree,&s1,&s2);
	p1=s1->stptr;
	p2=s2->stptr;
	l1=s1->stlen;
	l2=s2->stlen;
	while(l1) {
		if(!strncmp(p1,p2,l2))
			return tmp_number((AWKNUM)(1+s1->stlen-l1));
		l1--;
		p1++;
	}
	return tmp_number(0.0);
}

NODE *
do_int(tree)
NODE *tree;
{
	NODE	*tmp;
	double	floor();

	get_one(tree,&tmp);
	return tmp_number(floor(force_number(tmp)));
}

NODE *
do_length(tree)
NODE *tree;
{
	NODE *tmp;

	get_one(tree,&tmp);
	return tmp_number((AWKNUM)(force_string(tmp)->stlen));
}

NODE *
do_log(tree)
NODE *tree;
{
	NODE	*tmp;
	double log();

	get_one(tree,&tmp);
	return tmp_number(log(force_number(tmp)));
}


NODE	*
do_printf(tree)
NODE *tree;
{
	register FILE	*fp;
	NODE	*do_sprintf();

	fp=deal_redirect(tree->rnode);
	print_simple(do_sprintf(tree->lnode),fp);
	return Nnull_string;
}


NODE *
do_split(tree)
NODE *tree;
{
	NODE	*t1,*t2,*t3;
	register int	splitc;
	register int	num,snum,olds;
	register char	*ptr,*oldp;
	NODE **assoc_lookup();

	if(a_get_three(tree,&t1,&t2,&t3)<3)
		splitc= get_fs();
	else
		splitc= *(force_string(t3)->stptr);
	num=0;
	tree=force_string(t1);
	olds=snum=tree->stlen;
	oldp=ptr=tree->stptr;
	assoc_clear(t2);
	while(snum--) {
		if(*ptr++==splitc) {
			*assoc_lookup(t2,make_number((AWKNUM)(++num)))=make_string(oldp,(olds-snum)-1);
			oldp=ptr;
			olds=snum;
		}
	}
	*assoc_lookup(t2,make_number((AWKNUM)(++num)))=make_string(oldp,(olds-snum)-1);
	return tmp_number((AWKNUM)num);
}

/* Note that the output buffer cannot be static because sprintf may get called
   recursively by force_string.  Hence the wasteful alloca calls */

/* %e and %f formats are not properly implemented.  Someone should fix them */
NODE *
do_sprintf(tree)
NODE *tree;
{
#define bchunk(s,l) if(l) {\
    if((l)>ofre) {\
      char *tmp;\
      tmp=(char *)alloca(osiz*2);\
      bcopy(obuf,tmp,olen);\
      obuf=tmp;\
      ofre+=osiz;\
      osiz*=2;\
    }\
    bcopy(s,obuf+olen,(l));\
    olen+=(l);\
    ofre-=(l);\
  }

/* Is there space for something L big in the buffer? */
#define chksize(l)  if((l)>ofre) {\
    char *tmp;\
    tmp=(char *)alloca(osiz*2);\
    bcopy(obuf,tmp,olen);\
    obuf=tmp;\
    ofre+=osiz;\
    osiz*=2;\
  }
/* Get the next arg to be formatted.  If we've run out of args, return
   "" (Null string) */
#define parse_next_arg() {\
  if(!carg) arg= Nnull_string;\
  else {\
  	get_one(carg,&arg);\
	carg=carg->rnode;\
  }\
 }

	char *obuf;
	int osiz,ofre,olen;
	static char chbuf[] = "0123456789abcdef";
	static char sp[] =" ";
	char	*s0,*s1;
	int	n0;
	NODE	*sfmt,*arg;
	register NODE *carg;
	long	fw,prec,lj,alt,big;
	long	*cur;
	long	val;
	unsigned long uval;
	int	sgn;
	int	base;
	char	cpbuf[30];		/* if we have numbers bigger than 30 */
	char	*cend= &cpbuf[30];	/* chars, we lose, but seems unlikely */
	char	*cp;
	char	*fill;
	double	tmpval;
	char	*pr_str;
	

	obuf=(char *)alloca(120);
	osiz=120;
	ofre=osiz;
	olen=0;
	get_one(tree,&sfmt);
	sfmt=force_string(sfmt);
	carg=tree->rnode;
	for(s0=s1=sfmt->stptr,n0=sfmt->stlen;n0-->0;) {
		if(*s1!='%') {
			s1++;
			continue;
		}

		bchunk(s0,s1-s0);
		s0=s1;
		cur= &fw;
		fw=0;
		prec=0;
		lj=alt=big=0;
		fill= sp;
		cp=cend;
		s1++;

	retry:
		--n0;
		switch(*s1++) {
		case '%':
			bchunk("%",1);
			s0=s1;
			break;

		case '0':
			if(fill!=sp || lj) goto lose;
			fill="0";		/* FALL through */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(cur==0)
				goto lose;
			*cur= s1[-1]-'0';
			while(n0>0 && *s1>='0' && *s1<='9') {
				--n0;
				*cur= *cur * 10 + *s1++ - '0';
			}
			goto retry;
		case '-':
			if(lj || fill!=sp) goto lose;
			lj++;
			goto retry;
		case '.':
			if(cur!=&fw) goto lose;
			cur= &prec;
			goto retry;
		case '#':
			if(alt) goto lose;
			alt++;
			goto retry;
		case 'l':
			if(big) goto lose;
			big++;
			goto retry;
		case '*':
			if(cur==0) goto lose;
			parse_next_arg();
			*cur=(int)arg;
			goto retry;
		case 'c':
			parse_next_arg();
			if(arg->type==Node_number) {
				uval=(unsigned long)arg->numbr;
				cpbuf[0]=uval;
				prec=1;
				pr_str=cpbuf;
				goto dopr_string;
			}
			if(!prec || prec>arg->stlen)
				prec=arg->stlen;
			pr_str=cpbuf;
			goto dopr_string;
		case 's':
			parse_next_arg();
			arg=force_string(arg);
			if(!prec || prec>arg->stlen)
				prec=arg->stlen;
			pr_str=arg->stptr;

		dopr_string:
			if(fw>prec && !lj) {
				while(fw>prec) {
					bchunk(sp,1);
					fw--;
				}
			}
			bchunk(pr_str,(int)prec);
			if(fw>prec) {
				while(fw>prec) {
					bchunk(sp,1);
					fw--;
				}
			}
			s0=s1;
			break;
		case 'd':
			parse_next_arg();
			val=(long)force_number(arg);
			if(val<0) {
				sgn=1;
				val= -val;
			} else sgn=0;
			do {
				*--cp='0'+val%10;
				val/=10;
			} while (val);
			if(sgn) *--cp='-';
			prec=cend-cp;
			if(fw>prec && !lj) {
				if(fill!=sp && *cp=='-') {
					bchunk(cp,1);
					cp++;
					prec--;
					fw--;
				}
				while(fw>prec) {
					bchunk(fill,1);
					fw--;
				}
			}
			bchunk(cp,(int)prec);
			if(fw>prec) {
				while(fw>prec) {
					bchunk(fill,1);
					fw--;
				}
			}
			s0=s1;
			break;
		case 'u':
			base=10;
			goto pr_unsigned;
		case 'o':
			base=8;
			goto pr_unsigned;
		case 'x':
			base=16;
			goto pr_unsigned;
		pr_unsigned:
			parse_next_arg();
			uval=(unsigned long)force_number(arg);
			do {
				*--cp=chbuf[uval%base];
				uval/=base;
			} while(uval);
			prec=cend-cp;
			if(fw>prec && !lj) {
				while(fw>prec) {
					bchunk(fill,1);
					fw--;
				}
			}
			bchunk(cp,(int)prec);
			if(fw>prec) {
				while(fw>prec) {
					bchunk(fill,1);
					fw--;
				}
			}
			s0=s1;
			break;
		case 'g':
			parse_next_arg();
			tmpval=force_number(arg);
			if(prec==0) prec=13;
			gcvt(tmpval,prec,cpbuf);
			prec=strlen(cpbuf);
			cp=cpbuf;
			if(fw>prec && !lj) {
				if(fill!=sp && *cp=='-') {
					bchunk(cp,1);
					cp++;
					prec--;
				}	/* Deal with .5 as 0.5 */
				if(fill==sp && *cp=='.') {
					--fw;
					while(--fw>=prec) {
						bchunk(fill,1);
					}
					bchunk("0",1);
				} else 
					while(fw-->prec) bchunk(fill,1);
			} else {		/* Turn .5 into 0.5 */
						/* FOO */
				if(*cp=='.' && fill==sp) {
					bchunk("0",1);
					--fw;
				}
			}
			bchunk(cp,(int)prec);
			if(fw>prec) while(fw-->prec) bchunk(fill,1);
			s0=s1;
			break;
			/* JF how to handle these!? */
		case 'f':
			parse_next_arg();
			tmpval=force_number(arg);
			chksize(fw+prec+5);	/* 5==slop */
/* cp=fcvt(tmpval,prec,&dec,&sgn);
   prec=strlen(cp);
   if(sgn) prec++; */
			cp=cpbuf;
			*cp++='%';
			if(lj) *cp++='-';
			if(fill!=sp) *cp++='0';
			if(prec!=0) {
				strcpy(cp,"*.*f");
				sprintf(obuf+olen,cpbuf,fw,prec,(double)tmpval);
			} else {
				strcpy(cp,"*f");
				sprintf(obuf+olen,cpbuf,fw,(double)tmpval);
			}
			cp=obuf+olen;
			ofre-=strlen(obuf+olen);
			olen+=strlen(obuf+olen);/* There may be nulls */
			s0=s1;
			break;
		case 'e':
			parse_next_arg();
			tmpval=force_number(arg);
			chksize(fw+prec+5);	/* 5==slop */
			cp=cpbuf;
			*cp++='%';
			if(lj) *cp++='-';
			if(fill!=sp) *cp++='0';
			if(prec!=0) {
				strcpy(cp,"*.*e");
				sprintf(obuf+olen,cpbuf,fw,prec,(double)tmpval);
			} else {
				strcpy(cp,"*e");
				sprintf(obuf+olen,cpbuf,fw,(double)tmpval);
			}
			cp=obuf+olen;
			ofre-=strlen(obuf+olen);
			olen+=strlen(obuf+olen);/* There may be nulls */
			s0=s1;
			break;
			break;
		/* case 'g':
			parse_next_arg();
			tmpval=force_number(arg);
			if(prec!=0) sprintf(obuf+osiz-ofre,"%*.*g",fw,prec,(double)tmpval);
			else sprintf(obuf+osiz-ofre,"%*g",fw,(double)tmpval);
			ofre-=strlen(obuf+osiz-ofre);
			s0=s1;
			break; */
		default:
		lose:
			break;
		}
	}
	bchunk(s0,s1-s0);
	return tmp_string(obuf,olen);
}

NODE *
do_sqrt(tree)
NODE *tree;
{
	NODE	*tmp;
	double	sqrt();

	get_one(tree,&tmp);
	return tmp_number(sqrt(force_number(tmp)));
}

NODE *
do_substr(tree)
NODE *tree;
{
	NODE	*t1,*t2,*t3;
	register int	n1,n2;

	if(get_three(tree,&t1,&t2,&t3)<3)
		n2=32000;
	else
		n2=(int)force_number(t3);
	n1=(int)force_number(t2)-1;
	tree=force_string(t1);
	if(n1<0 || n1>=tree->stlen || n2<=0)
		return Nnull_string;
	if(n1+n2>tree->stlen)
		n2=tree->stlen-n1;
	return tmp_string(tree->stptr+n1,n2);
}

/* The print command.  Its name is historical */
hack_print_node(tree)
NODE	*tree;
{
	register FILE	*fp;

#ifndef FAST
	if(!tree || tree->type != Node_K_print)
		abort();
#endif
	fp=deal_redirect(tree->rnode);
	tree=tree->lnode;
	if(!tree) tree=WHOLELINE;
	if(tree->type!=Node_expression_list) {
		print_simple(tree,fp);
	} else {
		while(tree) {
			print_simple(tree_eval(tree->lnode),fp);
			tree=tree->rnode;
			if(tree) print_simple(OFS_node->var_value,fp);
		}
	}
	print_simple(ORS_node->var_value,fp);
}


/* Get the arguments to functions.  No function cares if you give it
   too many args (they're ignored).  Only a few fuctions complain
   about being given too few args.  The rest have defaults */

get_one(tree,res)
NODE *tree,**res;
{
	if(!tree) {
		*res= WHOLELINE;
		return;
	}
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res=tree_eval(tree->lnode);
}

get_two(tree,res1,res2)
NODE *tree,**res1,**res2;
{
	if(!tree) {
		*res1= WHOLELINE;
		return;
	}
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res1=tree_eval(tree->lnode);
	if(!tree->rnode)
		return;
	tree=tree->rnode;
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res2=tree_eval(tree->lnode);
}

get_three(tree,res1,res2,res3)
NODE *tree,**res1,**res2,**res3;
{
	if(!tree) {
		*res1= WHOLELINE;
		return 0;
	}
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res1=tree_eval(tree->lnode);
	if(!tree->rnode)
		return 1;
	tree=tree->rnode;
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res2=tree_eval(tree->lnode);
	if(!tree->rnode)
		return 2;
	tree=tree->rnode;
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res3=tree_eval(tree->lnode);
	return 3;
}

a_get_three(tree,res1,res2,res3)
NODE *tree,**res1,**res2,**res3;
{
	if(!tree) {
		*res1= WHOLELINE;
		return 0;
	}
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res1=tree_eval(tree->lnode);
	if(!tree->rnode)
		return 1;
	tree=tree->rnode;
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res2=tree->lnode;
	if(!tree->rnode)
		return 2;
	tree=tree->rnode;
#ifndef FAST
	if(tree->type!=Node_expression_list)
		abort();
#endif
	*res3=tree_eval(tree->lnode);
	return 3;
}

/* FOO this should re-allocate the buffer if it isn't big enough.
   Also, it should do RMS style only-parse-enough stuff. */
/* This reads in a line from the input file */
inrec()
{
	static char *buf,*buf_end;
	static bsz;
	register char *cur;
	register char *tmp;
	register char *ttmp;
	int cnt;
	int tcnt;
	register int	c;
	int	rs;
	int	fs;
	extern FILE *input_file;
	NODE **get_lhs();

	rs = get_rs();
	fs = get_fs();
	blank_fields();
	NR++;
	NF=0;
	if(!buf) {
		buf=malloc(128);
		bsz=128;
		buf_end=buf+bsz;
	}
	cur=buf;
	cnt=0;
	while ((c=getc(input_file))!=EOF) {
		if((!rs && c=='\n' && cur[-1]=='\n' && cur!=buf) || (c == rs))
			break;
		*cur++=c;
		cnt++;
		if(cur==buf_end) {
			buf=realloc(buf,bsz*2);
			cur=buf+bsz;
			bsz*=2;
			buf_end=buf+bsz;
		}
	}
	*cur='\0';
	set_field(0,buf,cnt);
	assign_number(&(NF_node->var_value),0.0);
	if(c==EOF && cnt==0)
		return 1;
	assign_number(&(NR_node->var_value),1.0+force_number(NR_node->var_value));
	for(tmp=buf;tmp<cur;tmp++) {
		if(fs==' ') {
			while((*tmp==' ' || *tmp=='\t') && tmp<cur)
				tmp++;
			if(tmp>=cur)
				break;
		}
		tcnt=0;
		ttmp=tmp;
		if(fs==' ') {
			while(*tmp!=' ' && *tmp!='\t' && tmp<cur) {
				tmp++;
				tcnt++;
			}
		} else {
			while(*tmp!=fs && tmp<cur) {
				tmp++;
				tcnt++;
			}
		}
		set_field(++NF,ttmp,tcnt);
	}
	assign_number(&(NF_node->var_value),(AWKNUM)NF);
	return 0;
}

/* Redirection for printf and print commands */
FILE *
deal_redirect(tree)
NODE	*tree;
{
	register NODE	*tmp;
	register struct redirect *rp;
	register char	*str;
	register FILE	*fp;
	FILE	*popen();
	int	tflag;


	if(!tree) return stdout;
	tflag= (tree->type==Node_redirect_pipe) ? 1 : 2;
	tmp=tree_eval(tree->subnode);
	for(rp=reds;rp->flag!=0 && rp<&reds[20];rp++) {	/* That limit again */
		if(rp->flag==tflag && cmp_nodes(rp->value,tmp)==0)
			break;
	}
	if(rp==&reds[20]) {
		panic("too many redirections",0);
		return 0;
	}
	if(rp->flag!=0)
		return rp->fp;
	rp->flag=tflag;
	rp->value=dupnode(tmp);
	str=force_string(tmp)->stptr;
	switch(tree->type) {
	case Node_redirect_output:
		fp=rp->fp=fopen(str,"w");
		break;
	case Node_redirect_append:
		fp=rp->fp=fopen(str,"a");
		break;
	case Node_redirect_pipe:
		fp=rp->fp=popen(str,"w");
		break;
	}
	if(fp==0) panic("can't redirect to '%s'\n",str);
	rp++;
	rp->flag=0;
	return fp;
}

print_simple(tree,fp)
NODE *tree;
FILE *fp;
{
#ifndef FAST
	/* Deal with some obscure bugs */
	if(tree==(NODE *)0x55000000) {
		fprintf(fp,"***HUH***");
		return;
	}
	if((int)tree&01) {
		fprintf(fp,"$that's odd$");
		return;
	}
#endif
	tree=force_string(tree);
	fwrite(tree->stptr,sizeof(char),tree->stlen,fp);
}

