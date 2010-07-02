/*
 * gawk - routines for dealing with record input and fields
 *
 * Copyright (C) 1988 Free Software Foundation
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

#include "awk.h"

static int getline_redirect = 0;/* "getline <file" being executed */
static char *line_buf = NULL;	/* holds current input line */
static int line_alloc = 0;	/* current allocation for line_buf */

int field_num;			/* save number of field in get_lhs */
char *field_begin;
NODE **fields_arr;		/* array of pointers to the field nodes */
NODE node0;			/* node for $0 which never gets free'd */
int node0_valid = 1;		/* $(>0) has not been changed yet */
char f_empty[] = "";
int parse_high_water = 0;	/* field number that we have parsed so far */
char *parse_extent;		/* marks where to restart parse of record */
char *save_fs = " ";		/* save current value of FS when line is read,
				 * to be used in deferred parsing
				 */
static get_a_record();

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
set_field(num, str, len, dummy)
int num;
char *str;
int len;
NODE *dummy;	/* not used -- just to make interface same as set_element */
{
	NODE *n;
	int t;

	erealloc(fields_arr, NODE **, (num + 1) * sizeof(NODE *), "set_field");
	/* fill in fields that don't exist */
	for (t = parse_high_water + 1; t < num; t++)
		fields_arr[t] = Nnull_string;
	n = make_string(str, len);
	fields_arr[num] = n;
	parse_high_water = num;
}

/* Someone assigned a value to $(something).  Fix up $0 to be right */
rebuild_record()
{
	register int tlen;
	register NODE *tmp;
	NODE *ofs;
	char *ops;
	register char *cops;
	register NODE **ptr, **maxp;
	extern NODE *OFS_node;

	maxp = 0;
	tlen = 0;
	ofs = force_string(*get_lhs(OFS_node));
	deref = 0;
	ptr = &fields_arr[parse_high_water];
	while (ptr > &fields_arr[0]) {
		tmp = force_string(*ptr);
		tlen += tmp->stlen;
		if (tmp->stlen && !maxp)
			maxp = ptr;
		ptr--;
	}
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
	tmp = make_string(ops, tlen);
	deref = fields_arr[0];
	do_deref();
	fields_arr[0] = tmp;
}


/*
 * This reads in a record from the input file
 */
int
inrec()
{
	int cnt;
	int retval = 0;

	cnt = get_a_record(&line_buf, &line_alloc);
	if (cnt == EOF) {
		cnt = 0;
		retval = 1;
	} else {
		if (!getline_redirect) {
			assign_number(&(NR_node->var_value),
			    NR_node->var_value->numbr + 1.0);
			assign_number(&(FNR_node->var_value),
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
set_record(buf, cnt)
char *buf;
int cnt;
{
	char *get_fs();

	assign_number(&(NF_node->var_value), (AWKNUM) -1);
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
get_field(num)
int num;
{
	int n;

	/*
	 * if requesting whole line but some other field has been altered,
	 * then the whole line must be rebuilt
	 */
	if (num == 0 && node0_valid == 0) {
		/* first, parse remainder of input record */
		(void) parse_fields(HUGE-1, &parse_extent,
		    fields_arr[0]->stlen - (parse_extent-fields_arr[0]->stptr),
		    save_fs, set_field, (NODE *)NULL);
		rebuild_record();
		parse_high_water = 0;
		return &fields_arr[0];
	}
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
	if (n < num)	/* requested field number beyond end of record;
			 * set_field will just extend the number of fields,
			 * with empty fields
			 */
		set_field(num, f_empty, 0, (NODE *) NULL);
	/*
	 * if we reached the end of the record, set NF to the number of fields
	 * actually parsed.  Note that num might actually refer to a field that
	 * is beyond the end of the record, but we won't set NF to that value at
	 * this point, since this may only be a reference to the field and NF
	 * only gets set if the field is assigned to
	 */
	if (*parse_extent == '\0')
		assign_number(&(NF_node->var_value), (AWKNUM) n);

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
char *fs;
int (*set) ();	/* routine to set the value of the parsed field */
NODE *n;
{
	char *s = *buf;
	char *field;
	int field_len;
	char *scan;
	char *end = s + len;
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
		field_len = 0;
		field = scan;
		if (*fs == ' ')
			while (*scan != ' ' && *scan != '\t' && scan < end) {
				scan++;
				field_len++;
			}
		else {
			while (*scan != *fs && scan < end) {
				scan++;
				field_len++;
			}
			if (scan == end-1 && *scan == *fs) {
				(*set)(++NF, field, field_len, n);
				field = scan;
				field_len = 0;
			}
		}
		(*set)(++NF, field, field_len, n);
		if (scan == end)
			break;
	}
	*buf = scan;
	return NF;
}

int
re_split(buf, len, fs, reregs)
char *buf, *fs;
int len;
struct re_registers *reregs;
{
	typedef struct re_pattern_buffer RPAT;
	static RPAT *rp;
	static char *last_fs = NULL;

	if (last_fs != NULL && strcmp(fs, last_fs) != 0) {	/* fs has changed */
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
		if (re_compile_pattern(fs, strlen(fs), rp) != NULL)
			fatal("illegal regular expression for FS: `%s'", fs);
	}
	return re_search(rp, buf, len, 0, len, reregs);
}

static int				/* count of chars read or EOF */
get_a_record(bp, sizep)
char **bp;			/* *bp points to beginning of line on return */
int *sizep;			/* *sizep is current allocation of *bp */
{
	register char *buf;	/* buffer; realloced if necessary */
	int bsz;		/* current buffer size */
	register char *cur;	/* current position in buffer */
	register char *buf_end;	/* end of buffer */
	register int rs;	/* rs is the current record separator */
	register int c;
	extern FILE *input_file;

	bsz = *sizep;
	buf = *bp;
	if (!buf) {
		emalloc(buf, char *, 128, "get_a_record");
		bsz = 128;
	}
	rs = get_rs();
	buf_end = buf + bsz;
	cur = buf;
	while ((c = getc(input_file)) != EOF) {
		if (rs == 0 && c == '\n' && cur != buf && cur[-1] == '\n') {
			cur--;
			break;
		}
		else if (c == rs)
			break;
		*cur++ = c;
		if (cur == buf_end) {
			erealloc(buf, char *, bsz * 2, "get_a_record");
			cur = buf + bsz;
			bsz *= 2;
			buf_end = buf + bsz;
		}
	}
	if (rs == 0 && c == EOF && cur != buf && cur[-1] == '\n')
		cur--;
	*cur = '\0';
	*bp = buf;
	*sizep = bsz;
	if (c == EOF && cur == buf)
		return EOF;
	return cur - buf;
}

NODE *
do_getline(tree)
NODE *tree;
{
	FILE *save_fp;
	FILE *redirect();
	int cnt;
	NODE **lhs;
	extern NODE **get_lhs();
	extern FILE *input_file;
	extern FILE *nextfile();

	if (tree->rnode == NULL && (input_file == NULL || feof(input_file))) {
		input_file = nextfile();
		if (input_file == NULL)
			return tmp_number((AWKNUM) 0.0);
	}
	save_fp = input_file;
	if (tree->rnode != NULL) {	/* with redirection */
		input_file = redirect(tree->rnode);
		getline_redirect++;
	}
	if (tree->lnode == NULL) {	/* read in $0 */
		if (inrec() != 0) {
			input_file = save_fp;
			getline_redirect = 0;
			return tmp_number((AWKNUM) 0.0);
		}
	} else {			/* read in a named variable */
		char *s = NULL;
		int n = 0;

		lhs = get_lhs(tree->lnode);
		cnt = get_a_record(&s, &n);
		if (!getline_redirect) {
			assign_number(&(NR_node->var_value),
			    NR_node->var_value->numbr + 1.0);
			assign_number(&(FNR_node->var_value),
			    FNR_node->var_value->numbr + 1.0);
		}
		if (cnt == EOF) {
			input_file = save_fp;
			getline_redirect = 0;
			free(s);
			return tmp_number((AWKNUM) 0.0);
		}
		*lhs = make_string(s, strlen(s));
		free(s);
		/* we may have to regenerate $0 here! */
		if (field_num == 0)
			set_record(fields_arr[0]->stptr, fields_arr[0]->stlen);
		else if (field_num > 0) {
			node0_valid = 0;
			if (NF_node->var_value->numbr == -1 &&
			    field_num > NF_node->var_value->numbr)
				assign_number(&(NF_node->var_value),
					(AWKNUM) field_num);
		}
		field_num = -1;
		do_deref();
	}
	getline_redirect = 0;
	input_file = save_fp;
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
get_lhs(ptr)
NODE *ptr;
{
	register NODE **aptr;
	NODE *n;
	NODE **assoc_lookup();
	extern NODE *concat_exp();

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
			(void) get_field(HUGE-1); /* parse entire record */
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
		aptr = get_field(field_num);
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
				deref = 0;
				return;
			}
			free((char *)(deref->stptr));
		}
		deref->stptr = NULL;
		deref->numbr = -1111111.0;
		deref->flags = 0;
		deref->type = Node_illegal;
		free((char *)deref);
	}
	deref = 0;
}
