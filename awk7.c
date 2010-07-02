/*
 * awk7.c - routines for dealing with record input and fields
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
#include <fcntl.h>

extern NODE *concat_exp();

static void do_file();
static int get_rs();
static IOBUF *nextfile();
static int re_split();
static int get_a_record();
static int iop_close();
static IOBUF *iop_alloc();
static void close_one();
static int close_fp();

static int getline_redirect = 0;/* "getline <file" being executed */
static char *line_buf = NULL;	/* holds current input line */
static int line_alloc = 0;	/* current allocation for line_buf */
static char *parse_extent;	/* marks where to restart parse of record */
static int parse_high_water=0;	/* field number that we have parsed so far */
static char f_empty[] = "";
static char *save_fs = " ";	/* save current value of FS when line is read,
				 * to be used in deferred parsing
				 */

extern NODE *ARGC_node;
extern NODE *ARGV_node;

int field_num;			/* save number of field in get_lhs */
NODE **fields_arr;		/* array of pointers to the field nodes */
NODE node0;			/* node for $0 which never gets free'd */
int node0_valid = 1;		/* $(>0) has not been changed yet */

void
init_fields()
{
	emalloc(fields_arr, NODE **, sizeof(NODE *), "init_fields");
	node0.type = Node_val;
	node0.stref = 0;
	node0.flags = (STR|PERM);	/* never free buf */
	fields_arr[0] = &node0;
}

/*
 * Danger!  Must only be called for fields we know have just been blanked, or
 * fields we know don't exist yet.  
 */

/*ARGSUSED*/
static void
set_field(num, str, len, dummy)
int num;
char *str;
int len;
NODE *dummy;	/* not used -- just to make interface same as set_element */
{
	NODE *n;
	int t;
	static int nf_high_water = 0;

	if (num > nf_high_water) {
		erealloc(fields_arr, NODE **, (num + 1) * sizeof(NODE *), "set_field");
		nf_high_water = num;
	}
	/* fill in fields that don't exist */
	for (t = parse_high_water + 1; t < num; t++)
		fields_arr[t] = Nnull_string;
	n = make_string(str, len);
	fields_arr[num] = n;
	parse_high_water = num;
}

/* Someone assigned a value to $(something).  Fix up $0 to be right */
static void
rebuild_record()
{
	register int tlen;
	register NODE *tmp;
	NODE *ofs;
	char *ops;
	register char *cops;
	register NODE **ptr, **maxp;

	maxp = 0;
	tlen = 0;
	ofs = force_string(OFS_node->var_value);
	ptr = &fields_arr[parse_high_water];
	while (ptr > &fields_arr[0]) {
		tmp = force_string(*ptr);
		tlen += tmp->stlen;
		if (tmp->stlen && !maxp)
			maxp = ptr;
		ptr--;
	}
	if (maxp) {
		tlen += ((maxp - fields_arr) - 1) * ofs->stlen;
		emalloc(ops, char *, tlen + 1, "fix_fields");
		cops = ops;
		for (ptr = &fields_arr[1]; ptr <= maxp; ptr++) {
			tmp = force_string(*ptr);
			bcopy(tmp->stptr, cops, tmp->stlen);
			cops += tmp->stlen;
			if (ptr != maxp) {
				bcopy(ofs->stptr, cops, ofs->stlen);
				cops += ofs->stlen;
			}
		}
	} else
		ops = "";
	tmp = make_string(ops, tlen);
	deref = fields_arr[0];
	do_deref();
	fields_arr[0] = tmp;
}

#ifndef SLOWIO
#define	DO_END_OF_BUF	len = bp - start;\
			while (len > iop->secsiz) {\
				erealloc(iop->secbuf, char *, iop->secsiz *= 2, "get");\
			}\
			bcopy(start, iop->secbuf, len);\
			start = iop->secbuf;\
			last = start + len;\
			iop->cnt = read(iop->fd, iop->buf, iop->size);\
			if (iop->cnt < 0)\
				return iop->cnt;\
			end_data = iop->buf + iop->cnt;\
			iop->off = bp = iop->buf;

#define	DO_END_OF_DATA	iop->cnt = read(iop->fd, end_data, end_buf - end_data);\
			if (iop->cnt < 0)\
				return iop->cnt;\
			end_data += iop->cnt;\
			if (iop->cnt == 0)\
				break;\
			iop->cnt = end_data - iop->buf;

static int
get_a_record(res, iop)
char **res;
IOBUF *iop;
{
	register char *end_data;
	register char *end_buf;
	char *start;
	register char *bp;
	char *last;
	int len;
	register char rs = get_rs();

	end_data = iop->buf + iop->cnt;
	if (iop->off >= end_data) {
		iop->cnt = read(iop->fd, iop->buf, iop->size);
		if (iop->cnt <= 0)
			return iop->cnt = EOF;
		end_data = iop->buf + iop->cnt;
		iop->off = iop->buf;
	}
	start = bp = iop->off;
	end_buf = iop->buf + iop->size;
	if (rs == 0) {
		while (!(*bp == '\n' && bp != iop->buf && bp[-1] == '\n')) {
			if (++bp == end_buf) {
				DO_END_OF_BUF
			}
			if (bp == end_data) {
				DO_END_OF_DATA
			}
		}
		if (*bp == '\n' && bp != start && bp[-1] == '\n')
			bp--;
		else if (bp != iop->buf && bp[-1] != '\n')
			warning("record not terminated");
		else
			bp--;
		iop->off = bp + 2;
	} else {
		while (*bp++ != rs) {
			if (bp == end_buf) {
				DO_END_OF_BUF
			}
			if (bp == end_data) {
				DO_END_OF_DATA
			}
		}
		if (*--bp != rs) {
			warning("record not terminated");
			bp++;
		}
		iop->off = bp + 1;
	}
	if (start == iop->secbuf) {
		len = bp - iop->buf;
		while (len > iop->secsiz) {
			erealloc(iop->secbuf, char *, iop->secsiz *= 2, "get2");
		}
		bcopy(iop->buf, last, len);
		bp = last + len;
	}
	*bp = '\0';
	*res = start;
	return bp - start;
}
#endif

/*
 * This reads in a record from the input file
 */
static int
inrec(iop)
IOBUF *iop;
{
	int cnt;
	int retval = 0;

#ifndef SLOWIO
	cnt = get_a_record(&line_buf, iop);
#else
	cnt = get_a_record(fp, &line_buf, &line_alloc);
#endif
	if (cnt == EOF) {
		cnt = 0;
		retval = 1;
	} else {
		if (!getline_redirect) {
			assign_number(&NR_node->var_value,
			    NR_node->var_value->numbr + 1.0);
			assign_number(&FNR_node->var_value,
			    FNR_node->var_value->numbr + 1.0);
		}
	}
	set_record(line_buf, cnt);

	return retval;
}

/*
 * setup $0, but defer parsing rest of line until reference is made to $(>0)
 * or to NF.  At that point, parse only as much as necessary.
 */
void
set_record(buf, cnt)
char *buf;
int cnt;
{
	register int i;

	assign_number(&NF_node->var_value, (AWKNUM)-1);
	for (i = 1; i <= parse_high_water; i++) {
		deref = fields_arr[i];
		do_deref();
	}
	parse_high_water = 0;
	node0_valid = 1;
	if (buf == line_buf) {
		deref = fields_arr[0];
		do_deref();
		save_fs = get_fs();
		node0.type = Node_val;
		node0.stptr = buf;
		node0.stlen = cnt;
		node0.stref = 1;
		node0.flags = (STR|PERM);	/* never free buf */
		fields_arr[0] = &node0;
	}
}

NODE **
get_field(num, assign)
int num;
int assign;	/* this field is on the LHS of an assign */
{
	int n;

	/*
	 * if requesting whole line but some other field has been altered,
	 * then the whole line must be rebuilt
	 */
	if (num == 0 && node0_valid == 0) {
		/* first, parse remainder of input record */
		n = parse_fields(HUGE-1, &parse_extent,
		    node0.stlen - (parse_extent - node0.stptr),
		    save_fs, set_field, (NODE *)NULL);
		assign_number(&NF_node->var_value, (AWKNUM)n);
		rebuild_record();
		return &fields_arr[0];
	}
	if (num > 0 && assign)
		node0_valid = 0;
	if (num <= parse_high_water)	/* we have already parsed this field */
		return &fields_arr[num];
	if (parse_high_water == 0 && num > 0)	/* starting at the beginning */
		parse_extent = fields_arr[0]->stptr;
	/*
	 * parse up to num fields, calling set_field() for each, and saving
	 * in parse_extent the point where the parse left off
	 */
	n = parse_fields(num, &parse_extent,
		fields_arr[0]->stlen - (parse_extent-fields_arr[0]->stptr),
		save_fs, set_field, (NODE *)NULL);
	if (num == HUGE-1)
		num = n;
	if (n < num) {	/* requested field number beyond end of record;
			 * set_field will just extend the number of fields,
			 * with empty fields
			 */
		set_field(num, f_empty, 0, (NODE *) NULL);
		/*
		 * if this field is onthe LHS of an assignment, then we want to
		 * set NF to this value, below
		 */
		if (assign)
			n = num;
	}
	/*
	 * if we reached the end of the record, set NF to the number of fields
	 * so far.  Note that num might actually refer to a field that
	 * is beyond the end of the record, but we won't set NF to that value at
	 * this point, since this is only a reference to the field and NF
	 * only gets set if the field is assigned to -- in this case n has
	 * been set to num above
	 */
	if (*parse_extent == '\0')
		assign_number(&NF_node->var_value, (AWKNUM)n);

	return &fields_arr[num];
}

/*
 * this is called both from get_field() and from do_split()
 */
int
parse_fields(up_to, buf, len, fs, set, n)
int up_to;	/* parse only up to this field number */
char **buf;	/* on input: string to parse; on output: point to start next */
int len;
register char *fs;
void (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	char *s = *buf;
	register char *field;
	register char *scan;
	register char *end = s + len;
	int NF = parse_high_water;

	if (up_to == HUGE)
		NF = 0;
	if (*fs && *(fs + 1) != '\0') {	/* fs is a regexp */
		struct re_registers reregs;

		scan = s;
		while (re_split(scan, end - scan, fs, &reregs) != -1 &&
		    NF < up_to) {
			(*set)(++NF, scan, reregs.start[0], n);
			scan += reregs.end[0];
		}
		if (NF != up_to && scan <= end) {
			(*set)(++NF, scan, end - scan, n);
			scan = end;
		}
		*buf = scan;
		return (NF);
	}
	for (scan = s; scan < end && NF < up_to; scan++) {
		/*
		 * special case:  fs is single space, strip leading
		 * whitespace 
		 */
		if (*fs == ' ') {
			while ((*scan == ' ' || *scan == '\t') && scan < end)
				scan++;
			if (scan >= end)
				break;
		}
		field = scan;
		if (*fs == ' ')
			while (*scan != ' ' && *scan != '\t' && scan < end)
				scan++;
		else {
			while (*scan != *fs && scan < end)
				scan++;
			if (scan == end-1 && *scan == *fs) {
				(*set)(++NF, field, scan - field, n);
				field = scan;
			}
		}
		(*set)(++NF, field, scan - field, n);
		if (scan == end)
			break;
	}
	*buf = scan;
	return NF;
}

static int
re_split(buf, len, fs, reregs)
char *buf, *fs;
int len;
struct re_registers *reregs;
{
	typedef struct re_pattern_buffer RPAT;
	static RPAT *rp;
	static char *last_fs = NULL;

	if ((last_fs != NULL && !STREQ(fs, last_fs))
	    || (rp && ! strict && ((IGNORECASE_node->var_value->numbr != 0)
			 ^ (rp->translate != NULL))))
	{
		/* fs has changed or IGNORECASE has changed */
		free(rp->buffer);
		free(rp->fastmap);
		free((char *) rp);
		free(last_fs);
		last_fs = NULL;
	}
	if (last_fs == NULL) {	/* first time */
		emalloc(rp, RPAT *, sizeof(RPAT), "re_split");
		bzero((char *) rp, sizeof(RPAT));
		emalloc(rp->buffer, char *, 8, "re_split");
		rp->allocated = 8;
		emalloc(rp->fastmap, char *, 256, "re_split");
		emalloc(last_fs, char *, strlen(fs) + 1, "re_split");
		(void) strcpy(last_fs, fs);
		if (! strict && IGNORECASE_node->var_value->numbr != 0.0)
			rp->translate = casetable;
		else
			rp->translate = NULL;
		if (re_compile_pattern(fs, strlen(fs), rp) != NULL)
			fatal("illegal regular expression for FS: `%s'", fs);
	}
	return re_search(rp, buf, len, 0, len, reregs);
}

#ifdef SLOWIO
static int				/* count of chars read or EOF */
get_a_record(fp, bp, sizep)
FILE *fp;
char **bp;			/* *bp points to beginning of line on return */
int *sizep;			/* *sizep is current allocation of *bp */
{
	register char *buf;	/* buffer; realloced if necessary */
	int bsz;		/* current buffer size */
	register char *cur;	/* current position in buffer */
	register char *buf_end;	/* end of buffer */
	register int rs;	/* rs is the current record separator */
	register int c;

	bsz = *sizep;
	buf = *bp;
	if (!buf) {
		emalloc(buf, char *, 128, "get_a_record");
		bsz = 128;
	}
	rs = get_rs();
	buf_end = buf + bsz;
	cur = buf;
	if (rs == 0) {
		while ((c = getc(fp)) != EOF) {
			if (c == '\n' && cur != buf && cur[-1] == '\n') {
			cur--;
			break;
		}
		*cur++ = c;
		if (cur == buf_end) {
			erealloc(buf, char *, bsz * 2, "get_a_record");
			cur = buf + bsz;
			bsz *= 2;
			buf_end = buf + bsz;
		}
	}
		if (c == EOF && cur != buf && cur[-1] == '\n')
		cur--;
	} else {
		while ((c = getc(fp)) != EOF) {
			if (c == rs)
				break;
			*cur++ = c;
			if (cur == buf_end) {
				erealloc(buf, char *, bsz * 2, "get_a_record");
				cur = buf + bsz;
				bsz *= 2;
				buf_end = buf + bsz;
			}
		}
	}
	*cur = '\0';
	*bp = buf;
	*sizep = bsz;
	if (c == EOF && cur == buf)
		return EOF;
	return cur - buf;
}
#endif

NODE *
do_getline(tree)
NODE *tree;
{
	IOBUF *iop;
	int cnt;
	NODE **lhs;
	int redir_error = 0;

	if (tree->rnode == NULL) {	 /* no redirection */
		iop = nextfile();
		if (iop == NULL)		/* end of input */
			return tmp_number((AWKNUM) 0.0);
	}
	if (tree->rnode != NULL) {	/* with redirection */
		iop = redirect(tree->rnode, &redir_error)->iop;
		if (iop == NULL && redir_error)	/* failed redirect */
			return tmp_number((AWKNUM) -1.0);
		getline_redirect++;
	}
	if (tree->lnode == NULL) {	/* no optional var. -- read in $0 */
		if (inrec(iop) != 0) {
			getline_redirect = 0;
			return tmp_number((AWKNUM) 0.0);
		}
	} else {			/* read in a named variable */
		char *s = NULL;
		int n = 0;

		lhs = get_lhs(tree->lnode, 1);
#ifdef SLOWIO
		cnt = get_a_record(fp, &s, &n);
#else
		cnt = get_a_record(&s, iop);
#endif
		if (!getline_redirect) {
			assign_number(&NR_node->var_value,
			    NR_node->var_value->numbr + 1.0);
			assign_number(&FNR_node->var_value,
			    FNR_node->var_value->numbr + 1.0);
		}
		if (cnt == EOF) {
			getline_redirect = 0;
			free(s);
			return tmp_number((AWKNUM) 0.0);
		}
		*lhs = make_string(s, strlen(s));
		/*free(s);*/
		do_deref();
		/* we may have to regenerate $0 here! */
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		field_num = -1;
	}
	getline_redirect = 0;
	return tmp_number((AWKNUM) 1.0);
}

/*
 * We can't dereference a variable until after we've given it its new value.
 * This variable points to the value we have to free up 
 */
NODE *deref;

/*
 * This returns a POINTER to a node pointer. get_lhs(ptr) is the current
 * value of the var, or where to store the var's new value 
 */

NODE **
get_lhs(ptr, assign)
NODE *ptr;
int assign;		/* this is being called for the LHS of an assign. */
{
	register NODE **aptr;
	NODE *n;

#ifdef DEBUG
	if (ptr == NULL)
		cant_happen();
#endif
	deref = NULL;
	field_num = -1;
	switch (ptr->type) {
	case Node_var:
	case Node_var_array:
		if (ptr == NF_node && (int) NF_node->var_value->numbr == -1)
			(void) get_field(HUGE-1, assign); /* parse record */
		deref = ptr->var_value;
#ifdef DEBUG
		if (deref->type != Node_val)
			cant_happen();
		if (deref->flags == 0)
			cant_happen();
#endif
		return &(ptr->var_value);

	case Node_param_list:
		n = stack_ptr[ptr->param_cnt];
#ifdef DEBUG
		deref = n->var_value;
		if (deref->type != Node_val)
			cant_happen();
		if (deref->flags == 0)
			cant_happen();
		deref = 0;
#endif
		return &(n->var_value);

	case Node_field_spec:
		field_num = (int) force_number(tree_eval(ptr->lnode));
		free_result();
		if (field_num < 0)
			fatal("attempt to access field %d", field_num);
		aptr = get_field(field_num, assign);
		deref = *aptr;
		return aptr;

	case Node_subscript:
		n = ptr->lnode;
		if (n->type == Node_param_list)
			n = stack_ptr[n->param_cnt];
		aptr = assoc_lookup(n, concat_exp(ptr->rnode));
		deref = *aptr;
#ifdef DEBUG
		if (deref->type != Node_val)
			cant_happen();
		if (deref->flags == 0)
			cant_happen();
#endif
		return aptr;
	case Node_func:
		fatal ("`%s' is a function, assignment is not allowed",
			ptr->lnode->param);
	}
	return 0;
}

void
do_deref()
{
	if (deref == NULL)
		return;
	if (deref == Nnull_string) {
		deref = 0;
		return;
	}
#ifdef DEBUG
	if (deref->flags == 0)
		cant_happen();
#endif
	if ((deref->flags & MALLOC) || (deref->flags & TEMP)) {
#ifdef DEBUG
		if (deref->flags & PERM)
			cant_happen();
#endif
		if (deref->flags & STR) {
			if (deref->stref > 0 && deref->stref != 255)
				deref->stref--;
			if (deref->stref > 0) {
				deref->flags &= ~TEMP;
				deref = 0;
				return;
			}
			free(deref->stptr);
		}
		freenode(deref);
	}
	deref = 0;
}

static IOBUF *
nextfile()
{
	static int i = 1;
	static int files = 0;
	static IOBUF *curfile = NULL;
	char *arg;
	char *cp;
	int fd = -1;

	if (curfile != NULL && curfile->cnt != EOF)
		return curfile;
	for (; i < (int) (ARGC_node->lnode->numbr); i++) {
		arg = (*assoc_lookup(ARGV_node, tmp_number((AWKNUM) i)))->stptr;
		if (*arg == '\0')
			continue;
		cp = index(arg, '=');
		if (cp != NULL) {
			*cp++ = '\0';
			variable(arg)->var_value = make_string(cp, strlen(cp));
			*--cp = '=';	/* restore original text of ARGV */
		} else {
			files++;
			if (STREQ(arg, "-"))
				fd = 0;
			else
				fd = open(arg, O_RDONLY);
			if (fd == -1)
				fatal("cannot open file `%s' for reading (%s)",
					arg, sys_errlist[errno]);
				/* NOTREACHED */
			/* This is a kludge.  */
			deref = FILENAME_node->var_value;
			do_deref();
			FILENAME_node->var_value =
				make_string(arg, strlen(arg));
			FNR_node->var_value->numbr = 0.0;
			i++;
			break;
		}
	}
	if (files == 0) {
		files++;
		/* no args. -- use stdin */
		/* FILENAME is init'ed to "-" */
		/* FNR is init'ed to 0 */
		fd = 0;
	}
	if (fd == -1)
		return NULL;
	return curfile = iop_alloc(fd);
}

static IOBUF *
iop_alloc(fd)
int fd;
{
	IOBUF *iop;
	struct stat stb;

	/*
	 * System V doesn't have the file system block size in the
	 * stat structure. So we have to make some sort of reasonable
	 * guess. We use stdio's BUFSIZ, since that what it was
	 * meant for in the first place.
	 */
#if defined(USG) || defined(MSDOS)
#define	DEFBLKSIZE	BUFSIZ
#else
#define DEFBLKSIZE	stb.st_blksize
#endif

	if (fd == -1)
		return NULL;
	emalloc(iop, IOBUF *, sizeof(IOBUF), "nextfile");
	if (isatty(fd))
		iop->size = 128;
	else if (fstat(fd, &stb) == -1)
		fatal("can't stat fd %d", fd);
	else if (lseek(fd, 0L, 0) == -1)
		iop->size = DEFBLKSIZE;
	else
		iop->size = stb.st_size < DEFBLKSIZE ?
				stb.st_size+1 : DEFBLKSIZE;
	errno = 0;
	iop->fd = fd;
	emalloc(iop->buf, char *, iop->size, "nextfile");
	iop->off = iop->buf;
	iop->cnt = 0;
	iop->secsiz = iop->size < 128 ? iop->size : 128;
	emalloc(iop->secbuf, char *, iop->secsiz, "nextfile");
	return iop;
}

void
do_input()
{
	IOBUF *iop;
	extern int exiting;

	while ((iop = nextfile()) != NULL) {
		do_file(iop);
		if (exiting)
			break;
	}
}

static int
iop_close(iop)
IOBUF *iop;
{
	int ret;

	ret = close(iop->fd);
	if (ret == -1)
		warning("close of fd %d failed");
	free(iop->buf);
	free(iop->secbuf);
	free((char *)iop);
	return ret == -1 ? 1 : 0;
}

static void
do_file(iop)
IOBUF *iop;
{
	/* This is where it spends all its time.  The infamous MAIN LOOP */
	if (inrec(iop) == 0)
		while (interpret(expression_value) && inrec(iop) == 0)
			;
	(void) iop_close(iop);
}

static int
get_rs()
{
	register NODE *tmp;

	tmp = force_string(RS_node->var_value);
	if (tmp->stlen == 0)
		return 0;
	return *(tmp->stptr);
}
struct redirect *red_head = NULL;

/* Redirection for printf and print commands */
struct redirect *
redirect(tree, errflg)
NODE *tree;
int *errflg;
{
	register NODE *tmp;
	register struct redirect *rp;
	register char *str;
	register FILE *fp;
	int tflag;
	char *direction = "to";

	tflag = 0;
	switch (tree->type) {
	case Node_redirect_append:
		tflag = RED_APPEND;
	case Node_redirect_output:
		tflag |= (RED_FILE|RED_WRITE);
		break;
#ifndef MSDOS
	case Node_redirect_pipe:
		tflag = (RED_PIPE|RED_WRITE);
		break;
	case Node_redirect_pipein:
		tflag = (RED_PIPE|RED_READ);
		break;
#else
	case Node_redirect_pipe:
	case Node_redirect_pipein:
		fprintf (stderr, "%s: cannot use pipe in PC version.\n",
			myname);
		exit(1);
		break;
#endif
	case Node_redirect_input:
		tflag = (RED_FILE|RED_READ);
		break;
	default:
		fatal ("invalid tree type %d in redirect()\n", tree->type);
		break;
	}
	tmp = force_string(tree_eval(tree->subnode));
	str = tmp->stptr;
	for (rp = red_head; rp != NULL; rp = rp->next)
		if (rp->flag == tflag && STREQ(rp->value, str))
			break;
	if (rp == NULL) {
		emalloc(rp, struct redirect *, sizeof(struct redirect),
			"redirect");
		emalloc(str, char *, strlen(tmp->stptr)+1, "redirect");
		(void) strcpy(str, tmp->stptr);
		rp->value = str;
		rp->flag = tflag;
		rp->offset = 0;
		rp->fp = NULL;
		rp->iop = NULL;
		/* maintain list in most-recently-used first order */
		if (red_head)
			red_head->prev = rp;
		rp->prev = NULL;
		rp->next = red_head;
		red_head = rp;
	}
	while (rp->fp == NULL && rp->iop == NULL) {
		errno = 0;
		switch (tree->type) {
		case Node_redirect_output:
			fp = rp->fp = fdopen(devopen(str, "w"), "w");
			break;
		case Node_redirect_append:
			fp = rp->fp = fdopen(devopen(str, "a"), "a");
			break;
#ifndef MSDOS
		case Node_redirect_pipe:
			fp = rp->fp = popen(str, "w");
			break;
		case Node_redirect_pipein:
			direction = "from";
			/* this should bypass popen() */
			rp->iop = iop_alloc(fileno(popen(str, "r")));
			break;
#endif
		case Node_redirect_input:
			direction = "from";
			rp->iop = iop_alloc(devopen(str, "r"));
			break;
		}
		if (fp == NULL && rp->iop == NULL) {
			/* too many files open -- close one and try again */
			if (errno == ENFILE || errno == EMFILE)
				close_one();
			else {
				/*
				 * Some other reason for failure.
				 *
				 * On redirection of input from a file,
				 * just return an error, so e.g. getline
				 * can return -1.  For output to file,
				 * complain. The shell will complain on
				 * a bad command to a pipe.
				 */
				*errflg = 1;
				if (tree->type == Node_redirect_output
				    || tree->type == Node_redirect_append)
					fatal("can't redirect %s `%s'\n",
						direction, str);
				else
					return NULL;
			}
		}
	}
	if (rp->offset != 0)	/* this file was previously open */
		if (fseek(fp, rp->offset, 0) == -1)
			fatal("can't seek to %ld on `%s'\n", rp->offset, str);
#ifdef notdef
	(void) flush_io();	/* a la SVR4 awk */
#endif
	free_temp(tmp);
	return rp;
}

static void
close_one()
{
	register struct redirect *rp;
	register struct redirect *rplast;

	/* go to end of list first, to pick up least recently used entry */
	for (rp = red_head; rp != NULL; rp = rp->next)
		rplast = rp;
	/* now work back up through the list */
	for (rp = rplast; rp != NULL; rp = rp->prev)
		if (rp->fp && (rp->flag & RED_FILE)) {
			rp->offset = ftell(rp->fp);
			if (fclose(rp->fp))
				warning("close of \"%s\" failed (%s).",
					rp->value, sys_errlist[errno]);
			rp->fp = NULL;
			break;
		}
	if (rp == NULL)
		/* surely this is the only reason ??? */
		fatal("too many pipes or input files open"); 
}

NODE *
do_close(tree)
NODE *tree;
{
	NODE *tmp;
	register struct redirect *rp;

	tmp = force_string(tree_eval(tree->subnode));
	for (rp = red_head; rp != NULL; rp = rp->next) {
		if (STREQ(rp->value, tmp->stptr))
			break;
	}
	free_temp(tmp);
	if (rp == NULL) /* no match */
		return tmp_number((AWKNUM) 0.0);
	return tmp_number((AWKNUM)close_fp(rp));
}

static int
close_fp(rp)
register struct redirect *rp;
{
	int status;

#ifndef MSDOS
	if (rp->flag == (RED_PIPE|RED_WRITE))
		status = pclose(rp->fp);
	else
#endif
	if (rp->fp)
		status = fclose(rp->fp);
	else if (rp->iop)
		status = iop_close(rp->iop);

	/* SVR4 awk checks and warns about status of close */
	if (status)
		warning("%s close of \"%s\" failed (%s).",
#ifndef MSDOS
			(rp->flag & RED_PIPE) ? "pipe" :
#endif
				"file", rp->value,
			sys_errlist[errno]);
	if (rp->prev)
		rp->prev->next = rp->next;
	else
		red_head = rp->next;
	free(rp->value);
	free((char *)rp);
	return status;
}

int
flush_io ()
{
	register struct redirect *rp;
	int status = 0;

	if (fflush(stdout)) {
		warning("error writing standard output.");
		status++;
	}
	if (fflush(stderr)) {
		warning("error writing standard error.");
		status++;
	}
	for (rp = red_head; rp != NULL; rp = rp->next)
		/* flush both files and pipes, what the heck */
		if ((rp->flag & RED_WRITE) && rp->fp != NULL)
			if (fflush(rp->fp)) {
				warning( "%s flush of \"%s\" failed (%s).",
#ifndef MSDOS
				    (rp->flag  & RED_PIPE) ? "pipe" :
#endif
				    "file", rp->value, sys_errlist[errno]);
				status++;
			}
	return status;
}

int
close_io ()
{
	register struct redirect *rp;
	int status = 0;

	for (rp = red_head; rp != NULL; rp = rp->next)
		if ((rp->fp && close_fp(rp)) || (rp->iop && iop_close(rp->iop)))
			status++;
	return status;
}

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, regular files */
int
devopen (name, mode)
char *name, *mode;
{
	int openfd = -1;
	FILE *fdopen ();
	char *cp;
	int flag;

	switch(mode[0]) {
	case 'r':	flag = O_RDONLY;
			break;

	case 'w':	flag = O_WRONLY|O_CREAT|O_TRUNC;
			break;

	case 'a':	flag = O_WRONLY|O_APPEND|O_CREAT;
			break;
	}

#if defined(STRICT) || defined(NO_DEV_FD)
	return (open (name, flag, 0666));
#else
	if (strict)
		return (open (name, flag, 0666));

	if (!STREQN (name, "/dev/", 5))
		return (open (name, flag, 0666));
	else
		cp = name + 5;
		
	/* XXX - first three tests ignore mode */
	if (STREQ(cp, "stdin"))
		return (0);
	else if (STREQ(cp, "stdout"))
		return (1);
	else if (STREQ(cp, "stderr"))
		return (2);
	else if (STREQN(cp, "fd/", 3)) {
		cp += 3;
		if (sscanf (cp, "%d", & openfd) == 1 && openfd >= 0)
			/* got something */
			return openfd;
		else
			return -1;
	} else
		return (open (name, flag, 0666));
#endif
}
