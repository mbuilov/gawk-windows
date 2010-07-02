/*
 * awk1 -- Expression tree constructors and main program for gawk.
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

#include <stdio.h>
#include "regex.h"
#include "awk.h"

/* Temporary nodes are stored here.  ob_dummy is a dummy object used to
   keep the obstack library from free()ing up the entire stack.  */
struct obstack temp_strings;
char *ob_dummy;

/* The parse tree and field nodes are stored here.  Parse_end is a dummy
   item used to free up unneeded fields without freeing the program being run
 */
struct obstack other_stack;
char *parse_end;

/* The global null string */
NODE *Nnull_string;

/* The special variable that contains the name of the current input file */
extern NODE *FILENAME_node;

/* The name the program was invoked under, for error messages */
char *myname;

/* A block of gAWK code to be run before running the program */
NODE	*begin_block = 0;

/* A block of gAWK code to be run after the last input file */
NODE	*end_block = 0;

FILE *input_file;	/* Where to read from */

#ifndef FAST
/* non-zero means in debugging is enabled.  Probably not very useful */
int debugging;
#endif

char *index();


main(argc, argv)
     int argc;
     char **argv;
{
  register int i;
  register NODE *tmp;
  char	**do_vars;
#ifndef FAST
	/* Print out the parse tree.   For debugging */
  register int dotree = 0;
  extern int yydebug;
#endif
  extern char *lexptr;
  extern char *lexptr_begin;
  FILE *fp,*fopen();

  --argc;
  myname= *argv++;
  if(!argc)
    usage();

	/* Tell the regex routines how they should work. . . */
  re_set_syntax(RE_NO_BK_PARENS|RE_NO_BK_VBAR);

	/* Set up the stack for temporary strings */
  obstack_init (&temp_strings);
  ob_dummy=obstack_alloc(&temp_strings,0);

    /* Set up the other stack for other things */
  obstack_init(&other_stack);
	/* initialize the null string */
  Nnull_string = make_string("",0);
  /* This was to keep Nnull_string from ever being free()d  It didn't work */
  /* Nnull_string->stref=32000; */
  	/* Set up the special variables */
	/* Note that this must be done BEFORE arg parsing else -R and -F
	   break horribly */
  init_vars();


  for(;*argv && **argv=='-';argc--,argv++) {
    switch(argv[0][1]) {
#ifndef FAST
    case 'd':
      debugging++;
      dotree++;
      break;

    case 'D':
      debugging++;
      yydebug=2;
      break;
#endif
      /* This feature isn't in un*x awk, but might be useful */
    case 'R':
      set_rs(&argv[0][2]);
      break;

    case 'F':
      set_fs(&argv[0][2]);
      break;


    		/* It would be better to read the input file in as we parse
		   it.  Its done this way for hysterical reasons.  Feel
		   free to fix it. */
    case 'f':
      if(lexptr)
        panic("Can only use one -f option");
      if((fp=fopen(argv[1],"r"))==NULL)
	er_panic(argv[1]);
      else {
        char *curptr;
	int siz,nread;

        curptr=lexptr=malloc(2000);
	if(curptr==NULL)
		panic("Memory exhausted");	/* jfw: instead of abort() */
	siz=2000;
	i=siz-1;
	while((nread=fread(curptr,sizeof(char),i,fp)) > 0) {
	  curptr+=nread;
	  i-=nread;
	  if(i==0) {
	    lexptr=realloc(lexptr,siz*2);
	    if(lexptr==NULL)
	    	panic("Memory exhausted");	/* jfw: instead of abort() */
	    curptr=lexptr+siz-1;
	    i=siz;
	    siz*=2;
	  }
	}
	*curptr='\0';
	fclose(fp);
      }
      argc--;
      argv++;
      break;

    case '\0':		/* A file */
      break;

    default:
      panic("Unknown option %s",argv[0]);
    }
  }
  if (debugging) setbuf(stdout, 0);	/* jfw: make debugging easier */
  /* No -f option, use next arg */
  if(!lexptr) {
    if(!argc) usage();
    lexptr= *argv++;
    --argc;
  }

  /* Read in the program */
  lexptr_begin=lexptr;
  (void)yyparse ();

  /* Anything allocated on the other_stack after here will be freed
     when the next input line is read.
     */
  parse_end=obstack_alloc(&other_stack,0);

#ifndef FAST
  if(dotree)
    print_parse_tree(expression_value);
#endif
  /* Set up the field variables */
  init_fields();

  /* Look for BEGIN and END blocks.  Only one of each allowed */
  for(tmp=expression_value;tmp;tmp=tmp->rnode) {
    if(!tmp->lnode || !tmp->lnode->lnode)
      continue;
    if(tmp->lnode->lnode->type==Node_K_BEGIN)
      begin_block=tmp->lnode->rnode;
    else if(tmp->lnode->lnode->type==Node_K_END)
      end_block=tmp->lnode->rnode;
  }
  if(begin_block && interpret(begin_block) == 0) exit(0);	/* jfw */
  do_vars=argv;
  while(argc>0 && index(*argv,'=')) {
    argv++;
    --argc;
  }
  if(do_vars==argv) do_vars=0;
  if(argc==0) {
    static char *dumb[2]= { "-", 0};

    argc=1;
    argv= &dumb[0];
  }
  while(argc--) {
    if(!strcmp(*argv,"-")) {
      input_file=stdin;
      FILENAME_node->var_value=Nnull_string;
      ADD_ONE_REFERENCE(Nnull_string);
    } else {
      extern NODE *deref;

      input_file=fopen(*argv,"r");
      /* This should print the error message from errno */
      if(!input_file)
        er_panic(*argv);
      /* This is a kludge.  */
      deref=FILENAME_node->var_value;
      do_deref();
      FILENAME_node->var_value=make_string(*argv,strlen(*argv));
    }
    /* This is where it spends all its time.  The infamous MAIN LOOP */
    if(inrec()==0) {
    	if(do_vars) {
		while(do_vars!=argv && *do_vars) {
			char *cp;

			cp=index(*do_vars,'=');
			*cp++='\0';
			variable(*do_vars)->var_value=make_string(cp,strlen(cp));
			do_vars++;
		}
		do_vars=0;
	}
	do 
          obstack_free(&temp_strings, ob_dummy);
	while (interpret(expression_value) && inrec() == 0);
    }
    if(input_file!=stdin) fclose(input_file);
    argv++;
  }
  if(end_block) (void)interpret(end_block);
  exit(0);
}

/* These exit values are arbitrary */
/*VARARGS1*/
panic(str,arg)
char *str;
{
	fprintf(stderr,"%s: ",myname);
	fprintf(stderr,str,arg);
	fprintf(stderr,"\n");
	exit(12);
}

er_panic(str)
char *str;
{
	fprintf(stderr,"%s: ",myname);
	perror(str);
	exit(15);
}

usage()
{
	fprintf(stderr,"%s: usage: %s {-f progfile | program } [-F{c} -R{c}] file . . .\n",myname,myname);
	exit(11);
}


/* This allocates a new node of type ty.  Note that this node will not go
   away unless freed, so don't use it for tmp storage */
NODE *
newnode(ty)
NODETYPE ty;
{
	register NODE *r;

	r=(NODE *)malloc(sizeof(NODE));
	if(r==NULL)
		abort();
	r->type=ty;
	return r;
}


/* Duplicate a node.  (For global strings, "duplicate" means crank up
   the reference count.)  This creates global nodes. . .*/
NODE *
dupnode(n)
NODE *n;
{
	register NODE *r;

	if(n->type==Node_string) {
		n->stref++;
		return n;
	} else if(n->type==Node_temp_string) {
		r=newnode(Node_string);
		r->stlen=n->stlen;
		r->stref=1;
		r->stptr=malloc(n->stlen+1);
		if(r->stptr==NULL)
			abort();
		bcopy (n->stptr, r->stptr, n->stlen);
		r->stptr[r->stlen]='\0';			/* JF for hackval */
		return r;
	} else {
		r=newnode(Node_illegal);
		*r= *n;
		return r;
	}
}

/* This allocates a node with defined lnode and rnode. */
/* This should only be used by yyparse+co while
   reading in the program */
NODE *
node (left, op, right)
     NODE *left, *right;
     NODETYPE op;
{
  register NODE *r;
  
  r = (NODE *)obstack_alloc(&other_stack,sizeof(NODE));
  r->type=op;
  r->lnode = left;
  r->rnode = right;
  return r;
}

/* This allocates a node with defined subnode and proc */
/* Otherwise like node() */
NODE *
snode(subn, op, procp)
NODETYPE op;
NODE *(*procp)();
NODE *subn;
{
	register NODE *r;

	r=(NODE *)obstack_alloc(&other_stack,sizeof(NODE));
	r->type=op;
	r->subnode=subn;
	r->proc=procp;
	return r;
}

/* (jfw) This allocates a Node_line_range node
 * with defined condpair and zeroes the trigger word
 * to avoid the temptation of assuming that calling
 * 'node( foo, Node_line_range, 0)' will properly initialize 'triggered'.
 */
/* Otherwise like node() */
NODE *
mkrangenode(cpair)
NODE *cpair;
{
	register NODE *r;

	r=(NODE *)obstack_alloc(&other_stack,sizeof(NODE));
	r->type=Node_line_range;
	r->condpair=cpair;
	r->triggered = 0;
	return r;
}

/* this allocates a node with defined numbr */
/* This creates global nodes! */
NODE *
make_number (x)
     AWKNUM x;
{
  register NODE *r;

  r=newnode(Node_number);
  r->numbr = x;
  return r;
}

/* This creates temporary nodes.  They go away quite quicly, so
   don't use them for anything important */
#ifndef FAST
NODE *
tmp_number(x)
AWKNUM	x;
{
#ifdef DONTDEF
	return make_number(x);
#endif
	NODE *r;

	r=(NODE *)obstack_alloc(&temp_strings,sizeof(NODE));
	r->type=Node_number;
	r->numbr=x;
	return r;
}
#endif

/* Make a string node.  If len==0, the string passed in S is supposed to end
   with a double quote, but have had the beginning double quote
   already stripped off by yylex.
   If LEN!=0, we don't care what s ends with.  This creates a global node */

NODE *
make_string (s,len)
     char *s;
{
  register NODE *r;
  register char *pf,*pt;
  register int	c;

  /* the aborts are impossible because yylex is supposed to have
     already checked for unterminated strings */
  if(len==-1) {		/* Called from yyparse, find our own len */
#ifndef FAST
    if (s[-1] != '\"')	/* Didn't start with " */
      abort ();
#endif

    for(pf = pt = s; *pf != '\0' && *pf!='\"';) {
      c= *pf++;
      switch(c) {
#ifndef FAST
      case '\0':
	abort();
#endif

      case '\\':
#ifndef FAST
      	if(*pf=='\0')
	  abort();
#endif

	c= *pf++;
	switch(c) {
	case '\\':	/* no massagary needed */
	case '\'':
	case '\"':
	  break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  c-='0';
	  while(*pf && *pf>='0' && *pf<='7') {
	    c=c*8+ *pf++ - '0';
	  }
	  break;
	case 'b':
	  c='\b';
	  break;
	case 'f':
	  c='\f';
	  break;
	case 'n':
	  c='\n';
	  break;
	case 'r':
	  c='\r';
	  break;
	case 't':
	  c='\t';
	  break;
	case 'v':
	  c='\v';
	  break;
	default:
	  *pt++='\\';
	  break;
	}
	/* FALL THROUGH */
      default:
	*pt++=c;
	break;
      }
    }
#ifndef FAST
    if(*pf=='\0')
      abort();	/* JF hit the end of the buf */
#endif
    len = pt - s;		/* JF was p - s - 1 */
  }

  r=newnode(Node_string);
  r->stptr=(char *)malloc(len+1);
  if(r->stptr==0)
  	abort();
  r->type=Node_string;
  r->stlen=len;
  r->stref=1;
  bcopy (s, r->stptr, len);
  r->stptr[len]='\0';		/* JF a hack */

  return r;
}

/* #ifndef FAST */
/* This should be a macro for speed, but the C compiler chokes. */
/* Read the warning under tmp_number */
NODE *
tmp_string(s,len)
char *s;
{
  register NODE *r;

#ifdef DONTDEF
  return make_string(s,len);
#endif
  r=(NODE *)obstack_alloc(&temp_strings,sizeof(NODE));
  r->stptr=(char *)obstack_alloc(&temp_strings,len+1);
  r->type=Node_temp_string;
  r->stlen=len;
  r->stref=1;
  bcopy (s, r->stptr, len);
  r->stptr[len]='\0';		/* JF a hack */

  return r;
}
/* #endif */

/* Generate compiled regular expressions */
struct re_pattern_buffer *
make_regexp (s)
     char *s;
{
  typedef struct re_pattern_buffer RPAT;
  RPAT *rp;
  char *p, *err;

  rp = (RPAT *) obstack_alloc(&other_stack, sizeof (RPAT));
  bzero((char *)rp,sizeof(RPAT));
  rp->buffer = (char *)malloc(8);	/* JF I'd obstack allocate it,
					   except the regex routines
					   try to realloc() it, which fails. */
  /* Note that this means it may never be freed.  Someone fix, please? */

  rp->allocated = 8;
  rp->fastmap = (char *)obstack_alloc(&other_stack, 256);

  for (p = s; *p != '\0'; p++) {
    if (*p == '\\')
      p++;
    else if (*p == '/')
      break;
  }
#ifndef FAST
  if (*p != '/')
    abort ();			/* impossible */
#endif

	/* JF was re_compile_pattern, but that mishandles ( ) and |,
	   so I had to write my own front end.  Sigh. */

  if ((err = re_compile_pattern (s, p - s, rp)) != NULL) {
    fprintf (stderr, "illegal regexp: ");
    yyerror (err);		/* fatal */
  }

  return rp;
}

/* Build a for loop */
FOR_LOOP_HEADER *
make_for_loop (init, cond, incr)
     NODE *init, *cond, *incr;
{
  register FOR_LOOP_HEADER *r;
  
  r = (FOR_LOOP_HEADER *)obstack_alloc(&other_stack,sizeof (FOR_LOOP_HEADER));
  r->init = init;
  r->cond = cond;
  r->incr = incr;
  return r;
}

/* Name points to a variable name.  Make sure its in the symbol table */
NODE *
variable (name)
     char *name;
{
  register NODE *r;
  NODE	*lookup(), *install();

  if ((r = lookup (variables, name)) == NULL) {
    r = install (variables, name, node(Nnull_string, Node_var, (NODE *)NULL));
    					/* JF  make_number (0.0) is WRONG */
  }
  return r;
}

/* Create a special variable */
NODE *
spc_var (name,value)
char *name;
NODE *value;
{
  register NODE *r;
  NODE *lookup(), *install();

  if ((r = lookup(variables, name)) == NULL)
    r = install (variables, name, node(value, Node_var, (NODE *)NULL));
  return r;
}


/*
 * Install a name in the hash table specified, even if it is already there.
 * Name stops with first non alphanumeric.
 * Caller must check against redefinition if that is desired.
 */
NODE *
install (table, name, value)
     HASHNODE **table;
     char *name;
     NODE *value;
{
  register HASHNODE *hp;
  register int i, len, bucket;
  register char *p;

  len = 0;
  p = name;
  while (is_identchar(*p))
    p++;
  len = p - name;

  i = sizeof (HASHNODE) + len + 1;
  hp = (HASHNODE *)obstack_alloc(&other_stack,i);
  bucket = hashf(name, len, HASHSIZE);
  hp->next = table[bucket];
  table[bucket] = hp;
  hp->length = len;
  hp->value = value;
  hp->name = ((char *) hp) + sizeof (HASHNODE);
  hp->length = len;
  bcopy (name, hp->name, len);
  return hp->value;
}

/*
 * find the most recent hash node for name name (ending with first
 * non-identifier char) installed by install
 */
NODE *
lookup (table, name)
     HASHNODE **table;
     char *name;
{
  register char *bp;
  register HASHNODE *bucket;
  register int len;

  for (bp = name; is_identchar(*bp); bp++)
    ;
  len = bp - name;
  bucket = table[hashf(name, len, HASHSIZE)];
  while (bucket) {
    if (bucket->length == len && strncmp(bucket->name, name, len) == 0)
      return bucket->value;
    bucket = bucket->next;
  }
  return NULL;
}

#define HASHSTEP(old, c) ((old << 1) + c)
#define MAKE_POS(v) (v & ~0x80000000) /* make number positive */

/*
 * return hash function on name.  must be compatible with the one
 * computed a step at a time, elsewhere  (JF: Where?  I can't find it!)
 */
int
hashf(name, len, hashsize)
     register char *name;
     register int len;
     int hashsize;
{
  register int r = 0;
  
  while (len--)
    r = HASHSTEP(r, *name++);
  
  return MAKE_POS(r) % hashsize;
}

/* Add new to the rightmost branch of LIST.  This uses n^2 time, but
   doesn't get used enough to make optimizing worth it. . . */
/* You don't believe me?  Profile it yourself! */

NODE *
append_right(list,new)
NODE *list,*new;
{
	register NODE *oldlist;

	oldlist = list;
	while(list->rnode!=NULL)
		list=list->rnode;
	list->rnode = new;
	return oldlist;
}
