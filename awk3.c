/*
 * awk3 -- Builtin functions and various utility procedures 
 *
 * $Log:	awk3.c,v $
 * Revision 1.40  89/03/31  13:17:20  david
 * GNU license; be careful about types in sprintf calls
 * 
 * Revision 1.39  89/03/29  14:20:07  david
 * delinting and code movement
 * change redirect() to return struct redirect *
 * 
 * Revision 1.38  89/03/22  22:10:20  david
 * a cleaner way to handle assignment to $n where n > 0
 * 
 * Revision 1.37  89/03/21  10:54:21  david
 * cleanup and fix of handling of precision in format string of printf call
 * 
 * Revision 1.36  89/03/15  22:01:51  david
 * ENVIRON fix from hack
 * relegated -Ft to strict compatibility
 * getline error return fix
 * printf %c fix (only print 1 char of a string)
 * tolower & toupper added
 * /dev/fd/N etc special files added
 * 
 * Revision 1.35  89/03/15  21:34:05  david
 * try to free more memory
 * 
 * Revision 1.34  88/12/13  22:28:10  david
 * temporarily #ifdef out flush_io in redirect(); adjust atan2() for 
 * force_number as a macro
 * 
 * Revision 1.32  88/12/01  15:03:21  david
 * renamed hack_print_node to do_print (at last!)
 * moved force_string() up out of print_simple for simplicity
 * 
 * Revision 1.31  88/11/30  15:17:27  david
 * free previous value in set_fs
 * 
 * Revision 1.30  88/11/29  16:24:47  david
 * fix bug in previous change
 * 
 * Revision 1.29  88/11/29  15:14:52  david
 * dynamically manage open files/pipes to allow an arbitrary number of open files
 * (i.e. when out of file descriptors, close the least recently used file,
 * saving the current offset; if it is reused, reopen and seek to saved offset)
 * 
 * Revision 1.28  88/11/28  20:12:53  david
 * correct previous error in cleanup of do_substr
 * 
 * Revision 1.27  88/11/23  21:42:13  david
 * Arnold: change ENV to ENVIRON nad a further bug fix for -Ft
 * ..
 * 
 * Revision 1.26  88/11/22  13:50:33  david
 * Arnold: added ENV array and bug fix to -Ft
 * 
 * Revision 1.25  88/11/15  10:24:08  david
 * Arnold: cleanup of comments, #include's and obsolete code
 * 
 * Revision 1.24  88/11/14  21:57:03  david
 * Arnold:  init. FILENAME to "-" and cleanup in do_substr()
 * 
 * Revision 1.23  88/11/01  12:17:45  david
 * cleanu and code movement; changes to reflect change to parse_fields()
 * 
 * Revision 1.22  88/10/19  21:58:43  david
 * replace malloc and realloc with error checking versions
 * 
 * Revision 1.21  88/10/17  20:55:31  david
 * SYSV --> USG
 * 
 * Revision 1.20  88/10/13  21:59:55  david
 * purge FAST and cleanup error messages
 * 
 * Revision 1.19  88/10/06  21:54:28  david
 * cleaned up I/O handling
 * 
 * Revision 1.18  88/10/06  15:49:01  david
 * changes from Arnold: be careful about flushing I/O; warn about error on close;
 * return seed from srand
 * 
 * Revision 1.17  88/09/19  20:39:11  david
 * minor cleanup
 * 
 * Revision 1.16  88/08/09  14:55:16  david
 * getline now gets next file properly
 * stupid bug in do_split() fixed
 * substr() now works if second arg. is negative (truncated to 0)
 * 
 * Revision 1.15  88/06/13  18:07:12  david
 * delete -R option
 * cleanup of redirection code [from Arnold]
 * 
 * Revision 1.14  88/06/07  23:41:00  david
 * some paranoid typecasting plus one bug fix:
 * in do_getline(), use stdin if input_file is NULL and ther is no redirection 
 * 
 * Revision 1.13  88/06/06  21:40:49  david
 * oops! got a little overenthusiastic on that last merge
 * 
 * Revision 1.12  88/06/06  11:27:57  david
 * get rid of some obsolete code
 * merge parsing of fields for record input and split()
 * 
 * Revision 1.11  88/06/05  21:00:35  david
 * flush I/O buffers before calling system (fix from Arnold)
 * 
 * Revision 1.10  88/06/05  20:59:26  david
 * local vars. now come off a stack
 * 
 * Revision 1.9  88/06/01  22:08:24  david
 * in split(), ensure that if second arg. is a local var. that the value is 
 * looked up
 * 
 * Revision 1.8  88/05/31  09:30:16  david
 * Arnold's portability fixes to last change in random() stuff
 * 
 * Revision 1.7  88/05/30  09:53:49  david
 * clean up some fatal() calls
 * de-lint the random number code
 * 
 * Revision 1.6  88/05/27  11:06:21  david
 * input_file wasn't getting properly reset after getline
 * 
 * Revision 1.5  88/05/26  22:49:55  david
 * fixed error message for redirection
 * 
 * Revision 1.4  88/05/18  18:20:02  david
 * fixed case where RS==""; record was including a trailing newline
 * 
 * Revision 1.3  88/04/13  17:39:26  david
 * fixed bug in handling of NR and FNR
 * 
 * Revision 1.2  88/04/12  16:04:02  david
 * fixed bug: NF at end of record generated one less field than it should have
 * 
 * Revision 1.1  88/04/08  15:15:07  david
 * Initial revision
 *  Revision 1.7  88/04/08  15:08:48  david bug fix for file
 * descriptor handlin 
 *
 * Revision 1.6  88/04/08  14:48:36  david changes from Arnold Robbins 
 *
 * Revision 1.5  88/03/28  14:13:54  david *** empty log message *** 
 *
 * Revision 1.4  88/03/23  22:17:41  david mostly delinting -- a couple of bug
 * fixes 
 *
 * Revision 1.3  88/03/18  21:00:13  david Baseline -- hoefully all the
 * functionality of the new awk added. Just debugging and tuning to do. 
 *
 * Revision 1.2  87/11/19  14:42:31  david expanded functionality for getline
 * broke out get_a_record() from inrec() so that the former can be used from
 * do_getline add system() builtin and skeletons for many other new builtins 
 *
 * Revision 1.1  87/10/27  15:23:33  david Initial revision 
 *
 */

/* 
 * Copyright (C) 1986, 1988, 1989 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

extern int parse_fields();
extern void assoc_clear();
extern NODE *node();

#ifdef USG
extern long lrand48();
extern void srand48();
#else
extern void srandom();
extern char *initstate();
extern char *setstate();
extern long random();
#endif

static void set_element();
static void get_one();
static void get_two();
static int get_three();
static int a_get_three();
static void close_one();
static int close_fp();
static NODE *spc_var();

NODE *do_sprintf();

/* These nodes store all the special variables AWK uses */
NODE *FS_node, *NF_node, *RS_node, *NR_node;
NODE *FILENAME_node, *OFS_node, *ORS_node, *OFMT_node;
NODE *FNR_node, *RLENGTH_node, *RSTART_node, *SUBSEP_node;
NODE *ENVIRON_node, *IGNORECASE_node;
NODE *ARGC_node, *ARGV_node;

/*
 * Set all the special variables to their initial values.
 */
void
init_vars()
{
	extern char **environ;
	char *var, *val;
	NODE **aptr;
	int i;

	FS_node = spc_var("FS", make_string(" ", 1));
	NF_node = spc_var("NF", make_number(-1.0));
	RS_node = spc_var("RS", make_string("\n", 1));
	NR_node = spc_var("NR", make_number(0.0));
	FNR_node = spc_var("FNR", make_number(0.0));
	FILENAME_node = spc_var("FILENAME", make_string("-", 1));
	OFS_node = spc_var("OFS", make_string(" ", 1));
	ORS_node = spc_var("ORS", make_string("\n", 1));
	OFMT_node = spc_var("OFMT", make_string("%.6g", 4));
	RLENGTH_node = spc_var("RLENGTH", make_number(0.0));
	RSTART_node = spc_var("RSTART", make_number(0.0));
	SUBSEP_node = spc_var("SUBSEP", make_string("\034", 1));
	IGNORECASE_node = spc_var("IGNORECASE", make_number(0.0));

	ENVIRON_node = spc_var("ENVIRON", Nnull_string);
	for (i = 0; environ[i]; i++) {
		static char nullstr[] = "";

		var = environ[i];
		val = index(var, '=');
		if (val)
			*val++ = '\0';
		else
			val = nullstr;
		aptr = assoc_lookup(ENVIRON_node, tmp_string(var, strlen (var)));
		*aptr = make_string(val, strlen (val));

		/* restore '=' so that system() gets a valid environment */
		if (val != nullstr)
			*--val = '=';
	}
}

/*
 * OFMT is special because we don't dare use force_string on it for fear of
 * infinite loops.  Thus, if it isn't a string, we return the default "%.6g"
 * This may or may not be the right thing to do, but its the easiest 
 */
/* This routine isn't used!  It should be.  */
#ifdef notdef
char *
get_ofmt()
{
	register NODE *tmp;

	tmp = OFMT_node->var_value;
	if ((tmp->type != Node_string && tmp->type != Node_str_num) || tmp->stlen == 0)
		return "%.6g";
	return tmp->stptr;
}
#endif

char *
get_fs()
{
	register NODE *tmp;

	tmp = force_string(FS_node->var_value);
	if (tmp->stlen == 0)
		return 0;
	return tmp->stptr;
}

void
set_fs(str)
char *str;
{
	register NODE **tmp;

	tmp = get_lhs(FS_node, 0);
	/*
	 * Only if in full compatibility mode check for the stupid special
	 * case so -F\t works as documented in awk even though the shell
	 * hands us -Ft.  Bleah!
	 */
	if (strict && str[0] == 't' && str[1] == '\0')
		str[0] = '\t';
	*tmp = make_string(str, 1);
	do_deref();
}

/* Builtin functions */
NODE *
do_exp(tree)
NODE *tree;
{
	NODE *tmp;
	double exp();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM)exp((double)force_number(tmp)));
}

NODE *
do_index(tree)
NODE *tree;
{
	NODE *s1, *s2;
	register char *p1, *p2;
	register int l1, l2;

	get_two(tree, &s1, &s2);
	p1 = s1->stptr;
	p2 = s2->stptr;
	l1 = s1->stlen;
	l2 = s2->stlen;
	while (l1) {
		if (STREQN(p1, p2, l2))
			return tmp_number((AWKNUM) (1 + s1->stlen - l1));
		l1--;
		p1++;
	}
	return tmp_number((AWKNUM) 0.0);
}

NODE *
do_int(tree)
NODE *tree;
{
	NODE *tmp;
	double floor();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM)floor((double)force_number(tmp)));
}

NODE *
do_length(tree)
NODE *tree;
{
	NODE *tmp;

	get_one(tree, &tmp);
	return tmp_number((AWKNUM) (force_string(tmp)->stlen));
}

NODE *
do_log(tree)
NODE *tree;
{
	NODE *tmp;
	double log();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM)log((double)force_number(tmp)));
}

void
do_printf(tree)
NODE *tree;
{
	register FILE *fp = stdout;
	int errflg = 0;		/* not used, sigh */

	if (tree->rnode)
		fp = redirect(tree->rnode, &errflg)->fp;
	if (fp)
		print_simple(do_sprintf(tree->lnode), fp);
}

static void
set_element(num, s, len, n)
int num;
char *s;
int len;
NODE *n;
{
	*assoc_lookup(n, tmp_number((AWKNUM) (num))) = make_string(s, len);
}

NODE *
do_split(tree)
NODE *tree;
{
	NODE *t1, *t2, *t3;
	register char *splitc;
	char *s;
	NODE *n;

	if (a_get_three(tree, &t1, &t2, &t3) < 3)
		splitc = get_fs();
	else
		splitc = force_string(t3)->stptr;

	n = t2;
	if (t2->type == Node_param_list)
		n = stack_ptr[t2->param_cnt];
	if (n->type != Node_var && n->type != Node_var_array)
		fatal("second argument of split is not a variable");
	assoc_clear(n);

	tree = force_string(t1);

	s = tree->stptr;
	return tmp_number((AWKNUM)
		parse_fields(HUGE, &s, tree->stlen, splitc, set_element, n));
}

/*
 * Note that the output buffer cannot be static because sprintf may get
 * called recursively by force_string.  Hence the wasteful alloca calls 
 */

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

	/*
	 * Get the next arg to be formatted.  If we've run out of args,
	 * return "" (Null string) 
	 */
#define parse_next_arg() {\
  if(!carg) arg= Nnull_string;\
  else {\
  	get_one(carg,&arg);\
	carg=carg->rnode;\
  }\
 }

	char *obuf;
	int osiz, ofre, olen;
	static char chbuf[] = "0123456789abcdef";
	static char sp[] = " ";
	char *s0, *s1;
	int n0;
	NODE *sfmt, *arg;
	register NODE *carg;
	long fw, prec, lj, alt, big;
	long *cur;
	long val;
	unsigned long uval;
	int sgn;
	int base;
	char cpbuf[30];		/* if we have numbers bigger than 30 */
	char *cend = &cpbuf[30];/* chars, we lose, but seems unlikely */
	char *cp;
	char *fill;
	double tmpval;
	char *pr_str;
	extern char *gcvt();


	obuf = (char *) alloca(120);
	osiz = 120;
	ofre = osiz;
	olen = 0;
	get_one(tree, &sfmt);
	sfmt = force_string(sfmt);
	carg = tree->rnode;
	for (s0 = s1 = sfmt->stptr, n0 = sfmt->stlen; n0-- > 0;) {
		if (*s1 != '%') {
			s1++;
			continue;
		}
		bchunk(s0, s1 - s0);
		s0 = s1;
		cur = &fw;
		fw = 0;
		prec = 0;
		lj = alt = big = 0;
		fill = sp;
		cp = cend;
		s1++;

retry:
		--n0;
		switch (*s1++) {
		case '%':
			bchunk("%", 1);
			s0 = s1;
			break;

		case '0':
			if (fill != sp || lj)
				goto lose;
			if (cur == &fw)
				fill = "0";	/* FALL through */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (cur == 0)
				goto lose;
			*cur = s1[-1] - '0';
			while (n0 > 0 && *s1 >= '0' && *s1 <= '9') {
				--n0;
				*cur = *cur * 10 + *s1++ - '0';
			}
			goto retry;
		case '-':
			if (lj || fill != sp)
				goto lose;
			lj++;
			goto retry;
		case '.':
			if (cur != &fw)
				goto lose;
			cur = &prec;
			goto retry;
		case '#':
			if (alt)
				goto lose;
			alt++;
			goto retry;
		case 'l':
			if (big)
				goto lose;
			big++;
			goto retry;
		case 'c':
			parse_next_arg();
			if (arg->flags & NUM) {
				uval = (unsigned long) arg->numbr;
				cpbuf[0] = uval;
				prec = 1;
				pr_str = cpbuf;
				goto dopr_string;
			}
			if (! prec)
				prec = 1;
			else if (prec > arg->stlen)
				prec = arg->stlen;
			pr_str = cpbuf;
			goto dopr_string;
		case 's':
			parse_next_arg();
			arg = force_string(arg);
			if (!prec || prec > arg->stlen)
				prec = arg->stlen;
			pr_str = arg->stptr;

	dopr_string:
			if (fw > prec && !lj) {
				while (fw > prec) {
					bchunk(sp, 1);
					fw--;
				}
			}
			bchunk(pr_str, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(sp, 1);
					fw--;
				}
			}
			s0 = s1;
			break;
		case 'd':
			parse_next_arg();
			val = (long) force_number(arg);
			if (val < 0) {
				sgn = 1;
				val = -val;
			} else
				sgn = 0;
			do {
				*--cp = '0' + val % 10;
				val /= 10;
			} while (val);
			if (sgn)
				*--cp = '-';
			prec = cend - cp;
			if (fw > prec && !lj) {
				if (fill != sp && *cp == '-') {
					bchunk(cp, 1);
					cp++;
					prec--;
					fw--;
				}
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			bchunk(cp, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			s0 = s1;
			break;
		case 'u':
			base = 10;
			goto pr_unsigned;
		case 'o':
			base = 8;
			goto pr_unsigned;
		case 'x':
			base = 16;
			goto pr_unsigned;
	pr_unsigned:
			parse_next_arg();
			uval = (unsigned long) force_number(arg);
			do {
				*--cp = chbuf[uval % base];
				uval /= base;
			} while (uval);
			prec = cend - cp;
			if (fw > prec && !lj) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			bchunk(cp, (int) prec);
			if (fw > prec) {
				while (fw > prec) {
					bchunk(fill, 1);
					fw--;
				}
			}
			s0 = s1;
			break;
		case 'g':
			parse_next_arg();
			tmpval = force_number(arg);
			if (prec == 0)
				prec = 13;
			(void) gcvt(tmpval, (int) prec, cpbuf);
			prec = strlen(cpbuf);
			cp = cpbuf;
			if (fw > prec && !lj) {
				if (fill != sp && *cp == '-') {
					bchunk(cp, 1);
					cp++;
					prec--;
				}	/* Deal with .5 as 0.5 */
				if (fill == sp && *cp == '.') {
					--fw;
					while (--fw >= prec) {
						bchunk(fill, 1);
					}
					bchunk("0", 1);
				} else
					while (fw-- > prec)
						bchunk(fill, 1);
			} else {/* Turn .5 into 0.5 */
				/* FOO */
				if (*cp == '.' && fill == sp) {
					bchunk("0", 1);
					--fw;
				}
			}
			bchunk(cp, (int) prec);
			if (fw > prec)
				while (fw-- > prec)
					bchunk(fill, 1);
			s0 = s1;
			break;
		case 'f':
			parse_next_arg();
			tmpval = force_number(arg);
			chksize(fw + prec + 5);	/* 5==slop */

			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (fill != sp)
				*cp++ = '0';
			if (cur != &fw) {
				(void) strcpy(cp, "*.*f");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (int) prec, (double) tmpval);
			} else {
				(void) strcpy(cp, "*f");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (double) tmpval);
			}
			cp = obuf + olen;
			ofre -= strlen(obuf + olen);
			olen += strlen(obuf + olen);	/* There may be nulls */
			s0 = s1;
			break;
		case 'e':
			parse_next_arg();
			tmpval = force_number(arg);
			chksize(fw + prec + 5);	/* 5==slop */
			cp = cpbuf;
			*cp++ = '%';
			if (lj)
				*cp++ = '-';
			if (fill != sp)
				*cp++ = '0';
			if (cur != &fw) {
				(void) strcpy(cp, "*.*e");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (int) prec, (double) tmpval);
			} else {
				(void) strcpy(cp, "*e");
				(void) sprintf(obuf + olen, cpbuf, (int) fw, (double) tmpval);
			}
			cp = obuf + olen;
			ofre -= strlen(obuf + olen);
			olen += strlen(obuf + olen);	/* There may be nulls */
			s0 = s1;
			break;

		default:
	lose:
			break;
		}
	}
	bchunk(s0, s1 - s0);
	return tmp_string(obuf, olen);
}

NODE *
do_sqrt(tree)
NODE *tree;
{
	NODE *tmp;
	double sqrt();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM)sqrt((double)force_number(tmp)));
}

NODE *
do_substr(tree)
NODE *tree;
{
	NODE *t1, *t2, *t3;
	register int indx, length;

	length = -1;
	if (get_three(tree, &t1, &t2, &t3) == 3)
		length = (int) force_number(t3);
	indx = (int) force_number(t2) - 1;
	tree = force_string(t1);
	if (length == -1)
		length = tree->stlen;
	if (indx < 0)
		indx = 0;
	if (indx >= tree->stlen || length <= 0)
		return Nnull_string;
	if (indx + length > tree->stlen)
		length = tree->stlen - indx;
	return tmp_string(tree->stptr + indx, length);
}

NODE *
do_system(tree)
NODE *tree;
{
	NODE *tmp;
	int ret;

	(void) flush_io ();	/* so output is syncrhonous with gawk's */
	get_one(tree, &tmp);
	ret = system(force_string(tmp)->stptr);
	ret = (ret >> 8) & 0xff;
	return tmp_number((AWKNUM) ret);
}

void 
do_print(tree)
register NODE *tree;
{
	register FILE *fp = stdout;
	int errflg = 0;		/* not used, sigh */

	if (tree->rnode)
		fp = redirect(tree->rnode, &errflg)->fp;
	if (!fp)
		return;
	tree = tree->lnode;
	if (!tree)
		tree = WHOLELINE;
	if (tree->type != Node_expression_list) {
		if (!(tree->flags & STR))
			cant_happen();
		print_simple(tree, fp);
	} else {
		while (tree) {
			print_simple(force_string(tree_eval(tree->lnode)), fp);
			tree = tree->rnode;
			if (tree)
				print_simple(OFS_node->var_value, fp);
		}
	}
	print_simple(ORS_node->var_value, fp);
}

NODE *
do_tolower(tree)
NODE *tree;
{
	NODE *t1, *t2;
	register char *cp, *cp2;

	get_one(tree, &t1);
	t1 = force_string(t1);
	t2 = tmp_string(t1->stptr, t1->stlen);
	for (cp = t2->stptr, cp2 = t2->stptr + t2->stlen; cp < cp2; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	return t2;
}

NODE *
do_toupper(tree)
NODE *tree;
{
	NODE *t1, *t2;
	register char *cp;

	get_one(tree, &t1);
	t1 = force_string(t1);
	t2 = tmp_string(t1->stptr, t1->stlen);
	for (cp = t2->stptr; cp < t2->stptr + t2->stlen; cp++)
		if (islower(*cp))
			*cp = toupper(*cp);
	return t2;
}

/*
 * Get the arguments to functions.  No function cares if you give it too many
 * args (they're ignored).  Only a few fuctions complain about being given
 * too few args.  The rest have defaults.
 */

static void
get_one(tree, res)
NODE *tree, **res;
{
	if (!tree) {
		*res = WHOLELINE;
		return;
	}
	*res = tree_eval(tree->lnode);
}

static void
get_two(tree, res1, res2)
NODE *tree, **res1, **res2;
{
	if (!tree) {
		*res1 = WHOLELINE;
		return;
	}
	*res1 = tree_eval(tree->lnode);
	if (!tree->rnode)
		return;
	tree = tree->rnode;
	*res2 = tree_eval(tree->lnode);
}

static int
get_three(tree, res1, res2, res3)
NODE *tree, **res1, **res2, **res3;
{
	if (!tree) {
		*res1 = WHOLELINE;
		return 0;
	}
	*res1 = tree_eval(tree->lnode);
	if (!tree->rnode)
		return 1;
	tree = tree->rnode;
	*res2 = tree_eval(tree->lnode);
	if (!tree->rnode)
		return 2;
	tree = tree->rnode;
	*res3 = tree_eval(tree->lnode);
	return 3;
}

static int
a_get_three(tree, res1, res2, res3)
NODE *tree, **res1, **res2, **res3;
{
	if (!tree) {
		*res1 = WHOLELINE;
		return 0;
	}
	*res1 = tree_eval(tree->lnode);
	if (!tree->rnode)
		return 1;
	tree = tree->rnode;
	*res2 = tree->lnode;
	if (!tree->rnode)
		return 2;
	tree = tree->rnode;
	*res3 = tree_eval(tree->lnode);
	return 3;
}

void
print_simple(tree, fp)
NODE *tree;
FILE *fp;
{
	if (fwrite(tree->stptr, sizeof(char), tree->stlen, fp) != tree->stlen)
		warning("fwrite: %s", sys_errlist[errno]);
	free_temp(tree);
}

NODE *
do_atan2(tree)
NODE *tree;
{
	NODE *t1, *t2;
	extern double atan2();

	get_two(tree, &t1, &t2);
	(void) force_number(t1);
	return tmp_number((AWKNUM) atan2((double) t1->numbr,
		(double) force_number(t2)));
}

NODE *
do_sin(tree)
NODE *tree;
{
	NODE *tmp;
	extern double sin();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM) sin((double)force_number(tmp)));
}

NODE *
do_cos(tree)
NODE *tree;
{
	NODE *tmp;
	extern double cos();

	get_one(tree, &tmp);
	return tmp_number((AWKNUM) cos((double)force_number(tmp)));
}

static int firstrand = 1;

#ifndef USG
static char state[256];
#endif

#define	MAXLONG	2147483647	/* maximum value for long int */

/* ARGSUSED */
NODE *
do_rand(tree)
NODE *tree;
{
#ifdef USG
	return tmp_number((AWKNUM) lrand48() / MAXLONG);
#else
	if (firstrand) {
		(void) initstate((unsigned) 1, state, sizeof state);
		srandom(1);
		firstrand = 0;
	}
	return tmp_number((AWKNUM) random() / MAXLONG);
#endif
}

NODE *
do_srand(tree)
NODE *tree;
{
	NODE *tmp;
	static long save_seed = 1;
	long ret = save_seed;	/* SVR4 awk srand returns previous seed */
	extern long time();

#ifdef USG
	if (tree == NULL)
		srand48(save_seed = time((long *) 0));
	else {
		get_one(tree, &tmp);
		srand48(save_seed = (long) force_number(tmp));
	}
#else
	if (firstrand)
		(void) initstate((unsigned) 1, state, sizeof state);
	else
		(void) setstate(state);

	if (!tree)
		srandom((int) (save_seed = time((long *) 0)));
	else {
		get_one(tree, &tmp);
		srandom((int) (save_seed = (long) force_number(tmp)));
	}
#endif
	firstrand = 0;
	return tmp_number((AWKNUM) ret);
}

/* Create a special variable */
static NODE *
spc_var(name, value)
char *name;
NODE *value;
{
	register NODE *r;

	if ((r = lookup(variables, name)) == NULL)
		r = install(variables, name, node(value, Node_var, (NODE *) NULL));
	return r;
}

void
init_args(argc0, argc, argv0, argv)
int argc0, argc;
char *argv0;
char **argv;
{
	int i, j;
	NODE **aptr;

	ARGV_node = spc_var("ARGV", Nnull_string);
	aptr = assoc_lookup(ARGV_node, tmp_number(0.0));
	*aptr = make_string(argv0, strlen(argv0));
	for (i = argc0, j = 1; i < argc; i++) {
		aptr = assoc_lookup(ARGV_node, tmp_number((AWKNUM) j));
		*aptr = make_string(argv[i], strlen(argv[i]));
		j++;
	}
	ARGC_node = spc_var("ARGC", make_number((AWKNUM) j));
}
