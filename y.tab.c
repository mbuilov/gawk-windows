
# line 27 "awk.y"
#ifdef DEBUG
#define YYDEBUG 12
#endif

#include "awk.h"

/*
 * This line is necessary since the Bison parser skeleton uses bcopy.
 * Systems without memcpy should use -DMEMCPY_MISSING, per the Makefile.
 * It should not hurt anything if Yacc is being used instead of Bison.
 */
#define bcopy(s,d,n)	memcpy((d),(s),(n))

extern void msg();
extern struct re_pattern_buffer *mk_re_parse();

NODE *node();
NODE *lookup();
NODE *install();

static NODE *snode();
static NODE *mkrangenode();
static FILE *pathopen();
static NODE *make_for_loop();
static NODE *append_right();
static void func_install();
static NODE *make_param();
static int hashf();
static void pop_params();
static void pop_var();
static int yylex ();
static void yyerror();

static int want_regexp;		/* lexical scanning kludge */
static int want_assign;		/* lexical scanning kludge */
static int can_return;		/* lexical scanning kludge */
static int io_allowed = 1;	/* lexical scanning kludge */
static int lineno = 1;		/* for error msgs */
static char *lexptr;		/* pointer to next char during parsing */
static char *lexptr_begin;	/* keep track of where we were for error msgs */
static int curinfile = -1;	/* index into sourcefiles[] */
static int param_counter;

NODE *variables[HASHSIZE];

extern int errcount;
extern NODE *begin_block;
extern NODE *end_block;

# line 77 "awk.y"
typedef union  {
	long lval;
	AWKNUM fval;
	NODE *nodeval;
	NODETYPE nodetypeval;
	char *sval;
	NODE *(*ptrval)();
} YYSTYPE;
# define FUNC_CALL 257
# define NAME 258
# define REGEXP 259
# define ERROR 260
# define NUMBER 261
# define YSTRING 262
# define RELOP 263
# define APPEND_OP 264
# define ASSIGNOP 265
# define MATCHOP 266
# define NEWLINE 267
# define CONCAT_OP 268
# define LEX_BEGIN 269
# define LEX_END 270
# define LEX_IF 271
# define LEX_ELSE 272
# define LEX_RETURN 273
# define LEX_DELETE 274
# define LEX_WHILE 275
# define LEX_DO 276
# define LEX_FOR 277
# define LEX_BREAK 278
# define LEX_CONTINUE 279
# define LEX_PRINT 280
# define LEX_PRINTF 281
# define LEX_NEXT 282
# define LEX_EXIT 283
# define LEX_FUNCTION 284
# define LEX_GETLINE 285
# define LEX_IN 286
# define LEX_AND 287
# define LEX_OR 288
# define INCREMENT 289
# define DECREMENT 290
# define LEX_BUILTIN 291
# define LEX_LENGTH 292
# define UNARY 293
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 671 "awk.y"


struct token {
	char *operator;		/* text to match */
	NODETYPE value;		/* node type */
	int class;		/* lexical class */
	short nostrict;		/* ignore if in strict compatibility mode */
	NODE *(*ptr) ();	/* function that implements this keyword */
};

extern NODE
	*do_exp(),	*do_getline(),	*do_index(),	*do_length(),
	*do_sqrt(),	*do_log(),	*do_sprintf(),	*do_substr(),
	*do_split(),	*do_system(),	*do_int(),	*do_close(),
	*do_atan2(),	*do_sin(),	*do_cos(),	*do_rand(),
	*do_srand(),	*do_match(),	*do_tolower(),	*do_toupper(),
	*do_sub(),	*do_gsub();

/* Special functions for debugging */
#ifdef DEBUG
NODE *do_prvars(), *do_bp();
#endif

/* Tokentab is sorted ascii ascending order, so it can be binary searched. */

static struct token tokentab[] = {
	{ "BEGIN",	Node_illegal,		LEX_BEGIN,	0,	0 },
	{ "END",	Node_illegal,		LEX_END,	0,	0 },
	{ "atan2",	Node_builtin,		LEX_BUILTIN,	0,	do_atan2 },
#ifdef DEBUG
	{ "bp",		Node_builtin,		LEX_BUILTIN,	0,	do_bp },
#endif
	{ "break",	Node_K_break,		LEX_BREAK,	0,	0 },
	{ "close",	Node_builtin,		LEX_BUILTIN,	0,	do_close },
	{ "continue",	Node_K_continue,	LEX_CONTINUE,	0,	0 },
	{ "cos",	Node_builtin,		LEX_BUILTIN,	0,	do_cos },
	{ "delete",	Node_K_delete,		LEX_DELETE,	0,	0 },
	{ "do",		Node_K_do,		LEX_DO,		0,	0 },
	{ "else",	Node_illegal,		LEX_ELSE,	0,	0 },
	{ "exit",	Node_K_exit,		LEX_EXIT,	0,	0 },
	{ "exp",	Node_builtin,		LEX_BUILTIN,	0,	do_exp },
	{ "for",	Node_K_for,		LEX_FOR,	0,	0 },
	{ "func",	Node_K_function,	LEX_FUNCTION,	0,	0 },
	{ "function",	Node_K_function,	LEX_FUNCTION,	0,	0 },
	{ "getline",	Node_K_getline,		LEX_GETLINE,	0,	0 },
	{ "gsub",	Node_builtin,		LEX_BUILTIN,	0,	do_gsub },
	{ "if",		Node_K_if,		LEX_IF,		0,	0 },
	{ "in",		Node_illegal,		LEX_IN,		0,	0 },
	{ "index",	Node_builtin,		LEX_BUILTIN,	0,	do_index },
	{ "int",	Node_builtin,		LEX_BUILTIN,	0,	do_int },
	{ "length",	Node_builtin,		LEX_LENGTH,	0,	do_length },
	{ "log",	Node_builtin,		LEX_BUILTIN,	0,	do_log },
	{ "match",	Node_builtin,		LEX_BUILTIN,	0,	do_match },
	{ "next",	Node_K_next,		LEX_NEXT,	0,	0 },
	{ "print",	Node_K_print,		LEX_PRINT,	0,	0 },
	{ "printf",	Node_K_printf,		LEX_PRINTF,	0,	0 },
#ifdef DEBUG
	{ "prvars",	Node_builtin,		LEX_BUILTIN,	0,	do_prvars },
#endif
	{ "rand",	Node_builtin,		LEX_BUILTIN,	0,	do_rand },
	{ "return",	Node_K_return,		LEX_RETURN,	0,	0 },
	{ "sin",	Node_builtin,		LEX_BUILTIN,	0,	do_sin },
	{ "split",	Node_builtin,		LEX_BUILTIN,	0,	do_split },
	{ "sprintf",	Node_builtin,		LEX_BUILTIN,	0,	do_sprintf },
	{ "sqrt",	Node_builtin,		LEX_BUILTIN,	0,	do_sqrt },
	{ "srand",	Node_builtin,		LEX_BUILTIN,	0,	do_srand },
	{ "sub",	Node_builtin,		LEX_BUILTIN,	0,	do_sub },
	{ "substr",	Node_builtin,		LEX_BUILTIN,	0,	do_substr },
	{ "system",	Node_builtin,		LEX_BUILTIN,	0,	do_system },
	{ "tolower",	Node_builtin,		LEX_BUILTIN,	0,	do_tolower },
	{ "toupper",	Node_builtin,		LEX_BUILTIN,	0,	do_toupper },
	{ "while",	Node_K_while,		LEX_WHILE,	0,	0 },
};

static char *token_start;

/* VARARGS0 */
static void
yyerror(va_alist)
va_dcl
{
	va_list args;
	char *mesg;
	register char *ptr, *beg;
	char *scan;

	errcount++;
	va_start(args);
	mesg = va_arg(args, char *);
	va_end(args);
	/* Find the current line in the input file */
	if (! lexptr) {
		beg = "(END OF FILE)";
		ptr = beg + 13;
	} else {
		if (*lexptr == '\n' && lexptr != lexptr_begin)
			--lexptr;
		for (beg = lexptr; beg != lexptr_begin && *beg != '\n'; --beg)
			;
		/* NL isn't guaranteed */
		for (ptr = lexptr; *ptr && *ptr != '\n'; ptr++)
			;
		if (beg != lexptr_begin)
			beg++;
	}
	msg("syntax error near line %d:\n%.*s", lineno, ptr - beg, beg);
	scan = beg;
	while (scan < token_start)
		if (*scan++ == '\t')
			putc('\t', stderr);
		else
			putc(' ', stderr);
	putc('^', stderr);
	putc(' ', stderr);
	vfprintf(stderr, mesg, args);
	putc('\n', stderr);
	exit(1);
}

/*
 * Parse a C escape sequence.  STRING_PTR points to a variable containing a
 * pointer to the string to parse.  That pointer is updated past the
 * characters we use.  The value of the escape sequence is returned. 
 *
 * A negative value means the sequence \ newline was seen, which is supposed to
 * be equivalent to nothing at all. 
 *
 * If \ is followed by a null character, we return a negative value and leave
 * the string pointer pointing at the null character. 
 *
 * If \ is followed by 000, we return 0 and leave the string pointer after the
 * zeros.  A value of 0 does not mean end of string.  
 */

int
parse_escape(string_ptr)
char **string_ptr;
{
	register int c = *(*string_ptr)++;
	register int i;
	register int count = 0;

	switch (c) {
	case 'a':
		return BELL;
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case '\n':
		return -2;
	case 0:
		(*string_ptr)--;
		return -1;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		i = c - '0';
		count = 0;
		while (++count < 3) {
			if ((c = *(*string_ptr)++) >= '0' && c <= '7') {
				i *= 8;
				i += c - '0';
			} else {
				(*string_ptr)--;
				break;
			}
		}
		return i;
	case 'x':
		i = 0;
		while (1) {
			if (isxdigit((c = *(*string_ptr)++))) {
				if (isdigit(c))
					i += c - '0';
				else if (isupper(c))
					i += c - 'A' + 10;
				else
					i += c - 'a' + 10;
			} else {
				(*string_ptr)--;
				break;
			}
		}
		return i;
	default:
		return c;
	}
}

/*
 * Read the input and turn it into tokens. Input is now read from a file
 * instead of from malloc'ed memory. The main program takes a program
 * passed as a command line argument and writes it to a temp file. Otherwise
 * the file name is made available in an external variable.
 */

static int
yylex()
{
	register int c;
	register int namelen;
	register char *tokstart;
	char *tokkey;
	static did_newline = 0;	/* the grammar insists that actions end
				 * with newlines.  This was easier than
				 * hacking the grammar. */
	int seen_e = 0;		/* These are for numbers */
	int seen_point = 0;
	int esc_seen;
	extern char **sourcefile;
	extern int tempsource, numfiles;
	static int file_opened = 0;
	static FILE *fin;
	static char cbuf[BUFSIZ];
	int low, mid, high;
#ifdef DEBUG
	extern int debugging;
#endif

	if (! file_opened) {
		file_opened = 1;
#ifdef DEBUG
		if (debugging) {
			int i;

			for (i = 0; i <= numfiles; i++)
				fprintf (stderr, "sourcefile[%d] = %s\n", i,
						sourcefile[i]);
		}
#endif
	nextfile:
		if ((fin = pathopen (sourcefile[++curinfile])) == NULL)
			fatal("cannot open `%s' for reading (%s)",
				sourcefile[curinfile],
				strerror(errno));
		*(lexptr = cbuf) = '\0';
		/*
		 * immediately unlink the tempfile so that it will
		 * go away cleanly if we bomb.
		 */
		if (tempsource && curinfile == 0)
			(void) unlink (sourcefile[curinfile]);
	}

retry:
	if (! *lexptr)
		if (fgets (cbuf, sizeof cbuf, fin) == NULL) {
			if (fin != NULL)
				fclose (fin);	/* be neat and clean */
			if (curinfile < numfiles)
				goto nextfile;
			return 0;
		} else
			lexptr = lexptr_begin = cbuf;

	if (want_regexp) {
		int in_brack = 0;

		want_regexp = 0;
		token_start = tokstart = lexptr;
		while (c = *lexptr++) {
			switch (c) {
			case '[':
				in_brack = 1;
				break;
			case ']':
				in_brack = 0;
				break;
			case '\\':
				if (*lexptr++ == '\0') {
					yyerror("unterminated regexp ends with \\");
					return ERROR;
				} else if (lexptr[-1] == '\n')
					goto retry;
				break;
			case '/':	/* end of the regexp */
				if (in_brack)
					break;

				lexptr--;
				yylval.sval = tokstart;
				return REGEXP;
			case '\n':
				lineno++;
			case '\0':
				lexptr--;	/* so error messages work */
				yyerror("unterminated regexp");
				return ERROR;
			}
		}
	}

	if (*lexptr == '\n') {
		lexptr++;
		lineno++;
		return NEWLINE;
	}

	while (*lexptr == ' ' || *lexptr == '\t')
		lexptr++;

	token_start = tokstart = lexptr;

	switch (c = *lexptr++) {
	case 0:
		return 0;

	case '\n':
		lineno++;
		return NEWLINE;

	case '#':		/* it's a comment */
		while (*lexptr != '\n' && *lexptr != '\0')
			lexptr++;
		goto retry;

	case '\\':
		if (*lexptr == '\n') {
			lineno++;
			lexptr++;
			goto retry;
		} else
			break;
	case ')':
	case ']':
	case '(':	
	case '[':
	case '$':
	case ';':
	case ':':
	case '?':

		/*
		 * set node type to ILLEGAL because the action should set it
		 * to the right thing 
		 */
		yylval.nodetypeval = Node_illegal;
		return c;

	case '{':
	case ',':
		yylval.nodetypeval = Node_illegal;
		return c;

	case '*':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_times;
			lexptr++;
			return ASSIGNOP;
		} else if (*lexptr == '*') {	/* make ** and **= aliases
						 * for ^ and ^= */
			if (lexptr[1] == '=') {
				yylval.nodetypeval = Node_assign_exp;
				lexptr += 2;
				return ASSIGNOP;
			} else {
				yylval.nodetypeval = Node_illegal;
				lexptr++;
				return '^';
			}
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '/':
		if (want_assign && *lexptr == '=') {
			yylval.nodetypeval = Node_assign_quotient;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '%':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_mod;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '^':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_exp;
			lexptr++;
			return ASSIGNOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '+':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_plus;
			lexptr++;
			return ASSIGNOP;
		}
		if (*lexptr == '+') {
			yylval.nodetypeval = Node_illegal;
			lexptr++;
			return INCREMENT;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '!':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_notequal;
			lexptr++;
			return RELOP;
		}
		if (*lexptr == '~') {
			yylval.nodetypeval = Node_nomatch;
			lexptr++;
			return MATCHOP;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

	case '<':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_leq;
			lexptr++;
			return RELOP;
		}
		yylval.nodetypeval = Node_less;
		return c;

	case '=':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_equal;
			lexptr++;
			return RELOP;
		}
		yylval.nodetypeval = Node_assign;
		return ASSIGNOP;

	case '>':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_geq;
			lexptr++;
			return RELOP;
		} else if (*lexptr == '>') {
			yylval.nodetypeval = Node_redirect_append;
			lexptr++;
			return APPEND_OP;
		}
		yylval.nodetypeval = Node_greater;
		return c;

	case '~':
		yylval.nodetypeval = Node_match;
		return MATCHOP;

	case '}':
		/*
		 * Added did newline stuff.  Easier than
		 * hacking the grammar
		 */
		if (did_newline) {
			did_newline = 0;
			return c;
		}
		did_newline++;
		--lexptr;
		return NEWLINE;

	case '"':
		esc_seen = 0;
		while (*lexptr != '\0') {
			switch (*lexptr++) {
			case '\\':
				esc_seen = 1;
				if (*lexptr == '\n')
					yyerror("newline in string");
				if (*lexptr++ != '\0')
					break;
				/* fall through */
			case '\n':
				lexptr--;
				yyerror("unterminated string");
				return ERROR;
			case '"':
				yylval.nodeval = make_str_node(tokstart + 1,
						lexptr-tokstart-2, esc_seen);
				yylval.nodeval->flags |= PERM;
				return YSTRING;
			}
		}
		return ERROR;

	case '-':
		if (*lexptr == '=') {
			yylval.nodetypeval = Node_assign_minus;
			lexptr++;
			return ASSIGNOP;
		}
		if (*lexptr == '-') {
			yylval.nodetypeval = Node_illegal;
			lexptr++;
			return DECREMENT;
		}
		yylval.nodetypeval = Node_illegal;
		return c;

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
	case '.':
		/* It's a number */
		for (namelen = 0; (c = tokstart[namelen]) != '\0'; namelen++) {
			switch (c) {
			case '.':
				if (seen_point)
					goto got_number;
				++seen_point;
				break;
			case 'e':
			case 'E':
				if (seen_e)
					goto got_number;
				++seen_e;
				if (tokstart[namelen + 1] == '-' ||
				    tokstart[namelen + 1] == '+')
					namelen++;
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
				break;
			default:
				goto got_number;
			}
		}

got_number:
		lexptr = tokstart + namelen;
		/*
		yylval.nodeval = make_string(tokstart, namelen);
		(void) force_number(yylval.nodeval);
		*/
		yylval.nodeval = make_number(atof(tokstart));
		yylval.nodeval->flags |= PERM;
		return NUMBER;

	case '&':
		if (*lexptr == '&') {
			yylval.nodetypeval = Node_and;
			while (c = *++lexptr) {
				if (c == '#')
					while ((c = *++lexptr) != '\n'
					       && c != '\0')
						;
				if (c == '\n')
					lineno++;
				else if (! isspace(c))
					break;
			}
			return LEX_AND;
		}
		return ERROR;

	case '|':
		if (*lexptr == '|') {
			yylval.nodetypeval = Node_or;
			while (c = *++lexptr) {
				if (c == '#')
					while ((c = *++lexptr) != '\n'
					       && c != '\0')
						;
				if (c == '\n')
					lineno++;
				else if (! isspace(c))
					break;
			}
			return LEX_OR;
		}
		yylval.nodetypeval = Node_illegal;
		return c;
	}

	if (c != '_' && ! isalpha(c)) {
		yyerror("Invalid char '%c' in expression\n", c);
		return ERROR;
	}

	/* it's some type of name-type-thing.  Find its length */
	for (namelen = 0; is_identchar(tokstart[namelen]); namelen++)
		/* null */ ;
	emalloc(tokkey, char *, namelen+1, "yylex");
	memcpy(tokkey, tokstart, namelen);
	tokkey[namelen] = '\0';

	/* See if it is a special token.  */
	low = 0;
	high = (sizeof (tokentab) / sizeof (tokentab[0])) - 1;
	while (low <= high) {
		int i, c;

		mid = (low + high) / 2;
		c = *tokstart - tokentab[mid].operator[0];
		i = c ? c : strcmp (tokkey, tokentab[mid].operator);

		if (i < 0) {		/* token < mid */
			high = mid - 1;
		} else if (i > 0) {	/* token > mid */
			low = mid + 1;
		} else {
			lexptr = tokstart + namelen;
			if (strict && tokentab[mid].nostrict)
				break;
			if (tokentab[mid].class == LEX_BUILTIN
			    || tokentab[mid].class == LEX_LENGTH)
				yylval.ptrval = tokentab[mid].ptr;
			else
				yylval.nodetypeval = tokentab[mid].value;
			return tokentab[mid].class;
		}
	}

	/* It's a name.  See how long it is.  */
	yylval.sval = tokkey;
	lexptr = tokstart + namelen;
	if (*lexptr == '(')
		return FUNC_CALL;
	else
		return NAME;
}

#ifndef DEFPATH
#ifdef MSDOS
#define DEFPATH	"."
#define ENVSEP	';'
#else
#define DEFPATH	".:/usr/lib/awk:/usr/local/lib/awk"
#define ENVSEP	':'
#endif
#endif

static FILE *
pathopen (file)
char *file;
{
	static char *savepath = DEFPATH;
	static int first = 1;
	char *awkpath, *cp;
	char trypath[BUFSIZ];
	FILE *fp;
#ifdef DEBUG
	extern int debugging;
#endif
	int fd;

	if (strcmp (file, "-") == 0)
		return (stdin);

	if (strict)
		return (fopen (file, "r"));

	if (first) {
		first = 0;
		if ((awkpath = getenv ("AWKPATH")) != NULL && *awkpath)
			savepath = awkpath;	/* used for restarting */
	}
	awkpath = savepath;

	/* some kind of path name, no search */
#ifndef MSDOS
	if (strchr (file, '/') != NULL)
#else
	if (strchr (file, '/') != NULL || strchr (file, '\\') != NULL
			|| strchr (file, ':') != NULL)
#endif
		return ( (fd = devopen (file, "r")) >= 0 ?
				fdopen(fd, "r") :
				NULL);

	do {
		trypath[0] = '\0';
		/* this should take into account limits on size of trypath */
		for (cp = trypath; *awkpath && *awkpath != ENVSEP; )
			*cp++ = *awkpath++;

		if (cp != trypath) {	/* nun-null element in path */
			*cp++ = '/';
			strcpy (cp, file);
		} else
			strcpy (trypath, file);
#ifdef DEBUG
		if (debugging)
			fprintf(stderr, "trying: %s\n", trypath);
#endif
		if ((fd = devopen (trypath, "r")) >= 0
		    && (fp = fdopen(fd, "r")) != NULL)
			return (fp);

		/* no luck, keep going */
		if(*awkpath == ENVSEP && awkpath[1] != '\0')
			awkpath++;	/* skip colon */
	} while (*awkpath);
#ifdef MSDOS
	/*
	 * Under DOS (and probably elsewhere) you might have one of the awk
	 * paths defined, WITHOUT the current working directory in it.
	 * Therefore you should try to open the file in the current directory.
	 */
	return ( (fd = devopen(file, "r")) >= 0 ? fdopen(fd, "r") : NULL);
#else
	return (NULL);
#endif
}

static NODE *
node_common(op)
NODETYPE op;
{
	register NODE *r;
	extern int numfiles;
	extern int tempsource;
	extern char **sourcefile;

	r = newnode(op);
	r->source_line = lineno;
	if (numfiles > -1 && ! tempsource)
		r->source_file = sourcefile[curinfile];
	else
		r->source_file = NULL;
	return r;
}

/*
 * This allocates a node with defined lnode and rnode. 
 * This should only be used by yyparse+co while reading in the program 
 */
NODE *
node(left, op, right)
NODE *left, *right;
NODETYPE op;
{
	register NODE *r;

	r = node_common(op);
	r->lnode = left;
	r->rnode = right;
	return r;
}

/*
 * This allocates a node with defined subnode and proc
 * Otherwise like node()
 */
static NODE *
snode(subn, op, procp)
NODETYPE op;
NODE *(*procp) ();
NODE *subn;
{
	register NODE *r;

	r = node_common(op);
	r->subnode = subn;
	r->proc = procp;
	return r;
}

/*
 * This allocates a Node_line_range node with defined condpair and
 * zeroes the trigger word to avoid the temptation of assuming that calling
 * 'node( foo, Node_line_range, 0)' will properly initialize 'triggered'. 
 */
/* Otherwise like node() */
static NODE *
mkrangenode(cpair)
NODE *cpair;
{
	register NODE *r;

	r = newnode(Node_line_range);
	r->condpair = cpair;
	r->triggered = 0;
	return r;
}

/* Build a for loop */
static NODE *
make_for_loop(init, cond, incr)
NODE *init, *cond, *incr;
{
	register FOR_LOOP_HEADER *r;
	NODE *n;

	emalloc(r, FOR_LOOP_HEADER *, sizeof(FOR_LOOP_HEADER), "make_for_loop");
	n = newnode(Node_illegal);
	r->init = init;
	r->cond = cond;
	r->incr = incr;
	n->sub.nodep.r.hd = r;
	return n;
}

/*
 * Install a name in the hash table specified, even if it is already there.
 * Name stops with first non alphanumeric. Caller must check against
 * redefinition if that is desired. 
 */
NODE *
install(table, name, value)
NODE **table;
char *name;
NODE *value;
{
	register NODE *hp;
	register int len, bucket;
	register char *p;

	len = 0;
	p = name;
	while (is_identchar(*p))
		p++;
	len = p - name;

	hp = newnode(Node_hashnode);
	bucket = hashf(name, len, HASHSIZE);
	hp->hnext = table[bucket];
	table[bucket] = hp;
	hp->hlength = len;
	hp->hvalue = value;
	emalloc(hp->hname, char *, len + 1, "install");
	memcpy(hp->hname, name, len);
	hp->hname[len] = '\0';
	return hp->hvalue;
}

/*
 * find the most recent hash node for name name (ending with first
 * non-identifier char) installed by install 
 */
NODE *
lookup(table, name)
NODE **table;
char *name;
{
	register char *bp;
	register NODE *bucket;
	register int len;

	for (bp = name; is_identchar(*bp); bp++)
		;
	len = bp - name;
	bucket = table[hashf(name, len, HASHSIZE)];
	while (bucket) {
		if (bucket->hlength == len && STREQN(bucket->hname, name, len))
			return bucket->hvalue;
		bucket = bucket->hnext;
	}
	return NULL;
}

#define HASHSTEP(old, c) ((old << 1) + c)
#define MAKE_POS(v) (v & ~0x80000000)	/* make number positive */

/*
 * return hash function on name.
 */
static int
hashf(name, len, hashsize)
register char *name;
register int len;
int hashsize;
{
	register int r = 0;

	while (len--)
		r = HASHSTEP(r, *name++);

	r = MAKE_POS(r) % hashsize;
	return r;
}

/*
 * Add new to the rightmost branch of LIST.  This uses n^2 time, so we make
 * a simple attempt at optimizing it.
 */
static NODE *
append_right(list, new)
NODE *list, *new;

{
	register NODE *oldlist;
	static NODE *savefront = NULL, *savetail = NULL;

	oldlist = list;
	if (savefront == oldlist) {
		savetail = savetail->rnode = new;
		return oldlist;
	} else
		savefront = oldlist;
	while (list->rnode != NULL)
		list = list->rnode;
	savetail = list->rnode = new;
	return oldlist;
}

/*
 * check if name is already installed;  if so, it had better have Null value,
 * in which case def is added as the value. Otherwise, install name with def
 * as value. 
 */
static void
func_install(params, def)
NODE *params;
NODE *def;
{
	NODE *r;

	pop_params(params->rnode);
	pop_var(params, 0);
	r = lookup(variables, params->param);
	if (r != NULL) {
		fatal("function name `%s' previously defined", params->param);
	} else
		(void) install(variables, params->param,
			node(params, Node_func, def));
}

static void
pop_var(np, freeit)
NODE *np;
int freeit;
{
	register char *bp;
	register NODE *bucket, **save;
	register int len;
	char *name;

	name = np->param;
	for (bp = name; is_identchar(*bp); bp++)
		;
	len = bp - name;
	save = &(variables[hashf(name, len, HASHSIZE)]);
	for (bucket = *save; bucket; bucket = bucket->hnext) {
		if (len == bucket->hlength && STREQN(bucket->hname, name, len)) {
			*save = bucket->hnext;
			freenode(bucket);
			free(bucket->hname);
			if (freeit)
				free(np->param);
			return;
		}
		save = &(bucket->hnext);
	}
}

static void
pop_params(params)
NODE *params;
{
	register NODE *np;

	for (np = params; np != NULL; np = np->rnode)
		pop_var(np, 1);
}

static NODE *
make_param(name)
char *name;
{
	NODE *r;

	r = newnode(Node_param_list);
	r->param = name;
	r->rnode = NULL;
	r->param_cnt = param_counter++;
	return (install(variables, name, r));
}

/* Name points to a variable name.  Make sure its in the symbol table */
NODE *
variable(name)
char *name;
{
	register NODE *r;

	if ((r = lookup(variables, name)) == NULL)
		r = install(variables, name,
			node(Nnull_string, Node_var, (NODE *) NULL));
	return r;
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 123,
	266, 0,
	-2, 99,
-1, 125,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 103,
-1, 126,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 104,
-1, 127,
	263, 0,
	60, 0,
	62, 0,
	124, 0,
	-2, 105,
-1, 244,
	266, 0,
	-2, 116,
-1, 246,
	263, 0,
	-2, 118,
	};
# define YYNPROD 155
# define YYLAST 1904
short yyact[]={

  60,  86,  87,  13, 222, 120,  13, 231, 105, 287,
   4,  35,  43, 209,  20,  80, 181,  24,  89,  43,
  85,  17, 152,  24,  34,  24,  33, 197,  25,  96,
 260, 282,  34, 281,  33, 257,  25,  86,  87, 245,
 256,  57, 255,  58,  86,  87, 163, 162, 158, 124,
  80, 119,  61, 121, 122, 123, 170, 125, 126, 127,
 128,  22, 130, 196,  80,  98,  91, 168, 215,  80,
  61,  61,  20, 105,  61,  24,  22, 171, 156,  17,
 152, 182,  34,  24,  33, 225,  25,  96, 165, 199,
  34,  43,  33, 168,  25, 248, 259, 172,  43,  61,
 217, 266,  50,  63, 160,  51, 180, 141, 139, 180,
 180, 180, 110,  61, 109, 155, 108,  20, 164,  80,
  24, 157,   6,  95,  17, 154,  24,  34,  37,  33,
  96,  25, 107,  34,  84,  33,  44,  25,  39,  80,
 191,  80, 156,  43, 101,  89, 161,  75, 224,  99,
 102, 253, 103, 118, 100,  69, 145, 146, 156, 254,
  10, 216,  22,   5,  64, 218, 219, 221,  20,   1,
  48,  24, 138,  12,   0,  17,  20,   0,  34,  24,
  33,   0,  25,  17,   0,   0,  34, 187,  33, 229,
  25, 184, 185,   0,  43, 189,   0,   0, 236, 237,
 238,  98,  43, 232,   0,   0,   0,  22,   0,  64,
   0,   0,  20,   0,   0,  24, 180, 210,  95,  17,
   4,  24,  34,   0,  33,  96,  25,   4,  34, 198,
  33, 101,   0,   0, 261, 200,  99,   0,  43,  23,
   0, 100,  31,  32,  56,  23,   0,  23,   0,   0,
  31,  32, 206,   0,   0,   0,   0, 194,  22,  80,
 272,   0,  20, 156, 166,  24,   0, 249,  80,  17,
  29,  30,  34, 214,  33, 156,  25,  20,  29,  30,
  24,   0, 166, 270,  17,  80, 166,  34,  98,  33,
   0,  25,  80,  80,  80, 134,  28,  23, 263,   0,
  31,  32,  22,  43,  57,  23,  58,  59,  31,  32,
  81, 166,  78,  79,  70,  71,  72,  73,  74,  82,
  83,  76,  77,   0,  18, 201,   0,   0,  29,  30,
  26,  27,   0,   0,   0,   0,  29,  30, 183,   0,
  66,  28,  23, 269,   0,  31,  32,  28,  23,   0,
   0,  31,  32,   0,   0,  81,  88,  78,  79,  70,
  71,  72,  73,  74,  82,  83,  76,  77,  51,  18,
   0,   0,   0,  29,  30,  26,  27,   0, 275,  29,
  30,  26,  27,   0,   0,  65,   0,   0,   0,   0,
 285,  66,  28,  23,   0,   0,  31,  32,   0,   0,
  28,  23,   0,  62,  31,  32,  81,   0,  78,  79,
  70,  71,  72,  73,  74,  82,  83,  76,  77,   0,
  18,   0,   0,   0,  29,  30,  26,  27,  18,   0,
   0,   0,  29,  30,  26,  27,  28,  23,   0,   0,
  31,  32,  28,  23, 179,   0,  31,  32,   0, 133,
  81,   0,  78,  79,  70,  71,  72,  73,  74,  82,
  83,  76,  77,   0,  18,   0,   0, 132,  29,  30,
  26,  27, 137,   0,  29,  30,  26,  27,   0,   0,
   0,   0,   0, 195,   0,  20,   0,  23,  24,   0,
  31,  32,  17, 168,   0,  34,   0,  33,  11,  25,
   0,  28,  23,   0, 133,  31,  32,  56,   0,  46,
  54,   4,  57,   0,  58,  59,   0,   0,  29,  30,
   0,   0, 186,   0, 133,   0, 192,   0,   0,  18,
  55,  52,  53,  29,  30,  26,  27,  20, 115,   0,
  24,   0, 190, 117,  17,   0,   0,  34,   0,  33,
   0,  25,   0,   0, 251,   0,   0,   0,  20,   0,
   0,  24,   0,  43,  57,  17,  58,  59,  34,   0,
  33, 208,  25,   0,  20,   0,  51,  24,   0,   0,
   0,  17,   0,   0,  34,   0,  33,  68,  25,   0,
  14,   0,   0,  14,   0,   0,   0,   0,  14, 188,
  49,  57,   0,  58,  59,   0,   0,   0,   0,   0,
  20,   0,   0,  24,   0,   0,   0,  17,   0,   0,
  34,  61,  33,   0,  25,   0,   0,  14,  51,   0,
   0,   0,  14,   0,   0,   0,   0,  57,   0,  58,
  59,   0,   0,   0, 271,   0,   0, 147,   0,   0,
   0,   0,   0, 279,  20,   0,   0,  24,   0,   0,
   0,  17,   0,   2,  34,  51,  33,   0,  25,  36,
 289,   0,   0,   0,   0,   0,   0, 294, 295, 296,
   0,  57,   0,  58,  59,   0, 104,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  51,   0,   0,   0,   0, 116,   0,   0,  28,
  23,   0,   0,  31,  32,  56,   0,   0,  54,   0,
   0,   0,   0,   0,   0, 129,   0,   0, 135,   0,
   0, 136,   0,   0,   0, 140,   0,  18,  55,  52,
  53,  29,  30,  26,  27,  51,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  28,  23,   0,   0,  31,  32,  56,   0,   0,
  54,   0, 239, 241, 242, 243, 244,   0, 246, 247,
   0,  90,  28,  23,   0,   0,  31,  32,   0,  18,
  55,  52,  53,  29,  30,  26,  27,   0,  28,  23,
   0,   0,  31,  32,  56,   0,   0,  54,   0,   0,
  18,   0,   0,   0,  29,  30,  26,  27,   0,   0,
 265,   0,   0,  16,   0,   0,  18,  55,  52,  53,
  29,  30,  26,  27,  28,  23, 277,   0,  31,  32,
  56,   0,  92,  54,  97,   0,   0,   0,  97,   0,
   0,   0,   0, 111, 112,   0,   0,  97,  97,   0,
   0,   0,  18,  55,  52,  53,  29,  30,  26,  27,
   0,   0,   0,   0,   0,   0,   0,   0,  28,  23,
   0,   0,  31,  32,  56,   0,   0,  54,   0,   0,
 152,   0,   0,  24, 258,   0,   0,  96,   0, 149,
  34,   0,  33,   0,  25,   0,  18,  55,  52,  53,
  29,  30,  26,  27,   0, 264, 267,   0,   0,  97,
 207,   0,  97,  97,  97,  97,  97,  97,  20,   0,
   0,  24, 280,   0,   0,  17,   0, 284,  34,   0,
  33,   0,  25,   0,  92,   0,   0, 288,   0,   0,
 291, 292,   0,   0, 293,  57,   0,  58,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 152, 149,   0,  24,  92,   0,  97,  96,   0,   0,
  34,   0,  33,   0,  25,   0,   0,  20,   0,   0,
  24,   0,   0,   0,  17,  97,   0,  34,   0,  33,
 207,  25,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  57,   0,  58,   0,   0,  51,
   0,   0,   0,   0, 149, 149, 149, 149, 149,   0,
 149, 149, 149,   0,  20,   0,   0,  24,   0,   0,
   0,  17,   0,   0,  34,   0,  33,   0,  25,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 149,   0, 149, 149, 149, 149,   0,
 149, 149, 149,  20,   0,   0,  24,   0,  51,   0,
  17,   0,   0,  34,   0,  33,   0,  25, 149, 149,
   0,   0,   0, 152,   0,   0,  24,   0,   0,   0,
  96, 149,   0,  34,   0,  33,   0,  25,   0,   0,
   0,   0,   0,   0,  28,  23,   0,   0,  31,  32,
 206,   0,   0, 204,  22,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  21,   0,   0,   0,   0,   0,
   0,   0, 150, 205, 202, 203,  29,  30,  26,  27,
   0,   0,  28,  23,   0,  94,  31,  32,  56, 106,
   0,  54,   0,  22, 152,   0,   0,  24, 113, 114,
   0,  96,   0,   0,  34,   0,  33,   0,  25,   0,
  18,  55,  52,   0,  29,  30,  26,  27,   0,   0,
   0,   0,   0,   0,  28,  23,   0,   0,  31,  32,
 206,   0,   0, 204,   0,   0,   0,   0,   0,   0,
 153,  28,  23,   0,   0,  31,  32,  56,   0,   0,
  54,   0, 150, 205, 202, 203,  29,  30,  26,  27,
  94,   0,   0, 173, 174, 175, 176, 177, 178,  18,
  55,   0,   0,  29,  30,  26,  27,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  38,  28,  23,
   0,   0,  31,  32,   0,   0, 152,   0,   4,  24,
   8,   9,   0,  96,   0,   0,  34,   0,  33,   0,
  25, 169, 153,   0,   0,  15,  18,  94,   0,   0,
  29,  30,  26,  27,   0,   0,   7,  28,  23,   0,
   0,  31,  32,   0,   0,   0, 223,   0,   0,   8,
   9,   0,   0,   0,   0,   0,   0,  28,  23,   0,
   0,  31,  32, 206,  15,  18, 204,   0,   0,  29,
  30,  26,  27,   0,   0, 153, 153, 153, 153, 153,
   0, 153, 153, 153,   0, 150, 205, 202,   0,  29,
  30,  26,  27,  20,   0,   0,  24,   0,   0,   0,
  17, 152,   0,  34,  24,  33,   0,  25, 144,   0,
 167,  34,   0,  33, 153,  25, 153, 153, 153, 153,
   0, 153, 153, 153,   0,   0,   0,   0,  28,  23,
   0,   0,  31,  32, 206,   0,   0, 204,   0, 153,
 153,  20,   0,   0,  24,   0,   0,   0,  17,   0,
   0,  34, 153,  33,   0,  25, 150, 205,   0,   0,
  29,  30,  26,  27,   0,   0, 152,   0,   0,  24,
   0,   0,   0,  96,   0,   0,  34,   0,  33,  20,
  25,   0,  24,   0,   0,   0,  17,   0,   0,  34,
   0,  33,   0,  25,   0,   0,   0,   0,   0,   3,
   0,   0,   0,   0, 226,   0, 227, 228,  41,  41,
  41,   0,   0, 230,   0,   0,   0, 234,   0,   0,
   0,   0,   0,   0,   0,  19,   0,   0,   0, 240,
  28,  23,   0,   0,  31,  32,   0,   0, 252,   0,
   0,   0,   0,   0,   0,   0,  93,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0, 150,   0,
   0,  40,  29,  30,  26,  27,   0,   0,   0,   0,
   0,  45,  47,  41,  41, 268,   0,   0,   0,   0,
  41,   0, 273,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 283,   0,   0, 286,   0,   0,
   0, 151,   0,   0,   0,   0,   0, 290,   0,   0,
   0,   0,   0,   0,   0,   0, 220,  28,  23,   0,
   0,  31,  32,   0, 148,  28,  23,   0,   0,  31,
  32,   0,   0,   0,   0, 142, 143,   0,   0,   0,
   0,   0, 159,   0,   0,  18,   0,   0,   0,  29,
  30,  26,  27, 150,  41,  41,   0,  29,  30,  26,
  27,   0,   0,   0,   0,  28,  23,   0,   0,  31,
  32,   0,   0, 151,   0,   0,  67,   0, 211,   0,
   0,   0,   0,   0,   0,  42,  42,  42,   0,   0,
  28,  23,   0,  18,  31,  32,  41,  29,  30,  26,
  27,   0,   0,  28, 193,   0,   0,  31,  32,   0,
   0,   0,   0,   0,  41,   0, 212, 213, 150,   0,
   0,   0,  29,  30,  26,  27, 151, 151, 151, 151,
 151,  18, 151, 151, 151,  29,  30,  26,  27, 131,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  42,  42,   0,   0,   0,   0,   0,  42, 235,   0,
   0,   0,   0,  41,   0, 151,  41, 151, 151, 151,
 151,   0, 151, 151, 151,   0, 250,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 151, 151,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 151,   0,   0,   0,   0,   0, 131,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 276,   0,   0, 278,   0,
   0,  42,  42,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 233,   0,  42,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  42,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 262,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0, 274,   0,
  42,   0,   0,  42 };
short yypact[]={

-257,-1000,1040,-256,-1000,1001,-1000,-1000, -40, -40,
 -47,-1000, -62, 577,  84,-1000,-245, 525, -13,-1000,
  90, 107,-257, -18, 185,-1000,  76,  74,  72, -13,
 -13,-1000,-1000, 185, 185,-1000,-1000,-1000,-1000, -62,
-1000,-256,-257,-1000, -62,-1000,-1000,-1000,-1000, 135,
1368,-280,1368,1368,1368,-209,1368,1368,1368,1368,
 229,-257,  32,  39,-257,-1000,-1000,-257,  84,-1000,
  68,-257,  67, -40, -40,1328,-1000,1368,-1000,-210,
 244,  64,-1000,-1000,-211,-1000,-1000,-1000,  26, 452,
-1000,  17,-1000,-1000, -29, 185,1368,-288, 185, 185,
 185, 185, 185, 185,-1000, 525,-1000,-243, 525, 525,
 525,-1000,-1000, -29, -29,-1000,-1000,-1000,  39, 621,
 -13, 954, 895, -19,-1000, 229, 229, 229, 541,-1000,
-1000,-1000,  32,-1000,-1000,-1000,-1000,-1000,  39,1368,
 179,1406,-1000,-1000, 525, -35,  69, 937,-1000,-252,
 -13,-1000,  90, 107, -40, -40, 621,1368, -23,-1000,
1368,  60,-1000,-1000,1368,1320,1368,-282,-1000,-1000,
-1000, 185, 452, -29, -29, -29, -29, 194, 194,  55,
 621,  38,  52,  30,  52,  52,-1000,-1000,1368,-1000,
-1000, 452,-268, -83,  32,  26, -40,1368,1368,1368,
1233,1393,1393,1393,1393,-219,1393,1393,  47,-1000,
  17,-1000,-1000,-1000, -40, 525, 452,-216, 621, 621,
-1000, 621,-223, 107,-1000,-1000,-1000,-1000,-1000, 621,
-257,  56,-228, 143, -35,-1000, 621, 621, 621, 937,
-1000, 937,1131,1060, -11,-1000,  47, 857,1393,-1000,
-1000,   8,-257,  52,  27,-1000,-1000,-1000, 179,1368,
  52, 504,1368, -40,1393, 937, -40, 179,-257,-225,
-1000,-1000, 452,-257,1368,  52,-1000, 937,-1000,-263,
-1000,-1000,-1000,-257, 179,  52,-257,-257,-1000,-1000,
-257, 179, 179, 179,-1000,-1000,-1000 };
short yypgo[]={

   0, 173, 170, 571,   0, 169, 163, 122,1134, 160,
 498, 823, 159, 157, 156, 338,  81, 103, 385, 155,
 151, 115,  66,1485,  56,  63,1281,  88,1459, 663,
 147, 146, 138, 136,1521, 134, 587, 403, 132,  62,
1636, 125, 121, 118,  95 };
short yyr1[]={

   0,   5,   6,   6,   6,   6,  32,   7,  33,   7,
   7,   7,   7,   7,   7,   7,  31,  31,  35,   1,
   2,   9,   9,  38,  23,  10,  10,  17,  17,  17,
  17,  34,  34,  18,  18,  18,  18,  18,  18,  18,
  18,  18,  18,  18,  18,  18,  41,  18,  18,  42,
  18,  18,  18,  30,  30,  19,  19,  28,  28,  29,
  29,  24,  24,  25,  25,  25,  25,  20,  20,  12,
  12,  12,  12,  12,  21,  21,  14,  14,  13,  13,
  13,  13,  13,  13,  16,  16,  15,  15,  15,  15,
  15,  15,  43,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,  44,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   8,   8,   8,   8,   8,   8,   8,   8,
   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
   8,   8,   8,  22,  22,  11,  11,  11,  36,  37,
  26,  39,  39,  40,  27 };
short yyr2[]={

   0,   3,   1,   2,   1,   2,   0,   3,   0,   3,
   2,   2,   2,   1,   2,   2,   1,   1,   0,   7,
   3,   1,   3,   0,   4,   3,   4,   1,   2,   1,
   2,   1,   2,   2,   2,   3,   1,   6,   8,   8,
  10,   9,   2,   2,   6,   4,   0,   3,   3,   0,
   4,   6,   2,   1,   1,   6,   9,   1,   2,   0,
   1,   0,   2,   0,   2,   2,   2,   0,   1,   1,
   3,   1,   2,   3,   0,   1,   0,   1,   1,   3,
   1,   2,   3,   3,   0,   1,   1,   3,   1,   2,
   3,   3,   0,   4,   5,   4,   3,   3,   3,   3,
   1,   2,   3,   3,   3,   3,   5,   1,   2,   0,
   4,   3,   3,   3,   1,   2,   3,   3,   3,   5,
   1,   2,   2,   3,   4,   4,   1,   4,   2,   2,
   2,   2,   1,   1,   1,   3,   3,   3,   3,   3,
   3,   2,   2,   0,   1,   1,   4,   2,   2,   2,
   1,   0,   1,   1,   2 };
short yychk[]={

-1000,  -5, -29, -28, 267,  -6,  -7, 256, 269, 270,
  -9, -10,  -1,  -4, -36, 284, -11,  40, 285, -23,
  33,  -8, 123, 258,  36,  47, 291, 292, 257, 289,
 290, 261, 262,  45,  43, 267, -29,  -7, 256, -32,
 -34, -28, -40,  59, -33, -34, -10, -34,  -2, -36,
 -27, 124, 287, 288, 266, 286, 263,  60,  62,  63,
  -4,  44, -37, -17, 125, -18, 256, -40, -36, -19,
 275, 276, 277, 278, 279, -30, 282, 283, 273, 274,
  -4, 271, 280, 281, -35, 265, 289, 290, -15,  -4,
 256, -22, -11, -23,  -8,  33,  40, -11,  94,  42,
  47,  37,  43,  45, -29,  91,  -8, -38,  40,  40,
  40, -11, -11,  -8,  -8, -10, -29, -10, -17,  -4,
 285,  -4,  -4,  -4, 258,  -4,  -4,  -4,  -4, -29,
 -39, -40, -37, -18, 256, -29, -29, -37, -17,  40,
 -29,  40, -34, -34,  40, -14, -13,  -3, 256, -11,
 285, -23,  33,  -8, -41, -21,  -4, -42, 258, -34,
  40, -31, 258, 257, -43, -27, 256, -26,  41, -26,
 -24,  60,  -4,  -8,  -8,  -8,  -8,  -8,  -8, -15,
  -4, 259, -16, -15, -16, -16, -37, -22,  58, -39,
 -37,  -4, -18, 258, -21, -15, -25,  62, 264, 124,
 -27, 256, 287, 288, 266, 286, 263,  63,  -3, 265,
 -22, -23, -34, -34, -21,  91,  -4,  40,  -4,  -4,
 256,  -4, 286,  -8,  93,  47, -26, -26, -26,  -4,
 -26, 275, 286, -40, -26, -34,  -4,  -4,  -4,  -3,
 256,  -3,  -3,  -3,  -3, 258,  -3,  -3, -44, -24,
 -34, -15, -26, -20, -12, 258, 256, 258, -29,  40,
 258,  -4, -40, -25,  58,  -3,  93, -29, -26, -27,
 256, -18,  -4, -26, -40, -21, -34,  -3, -34, -18,
 -29, 258, 256, -26, -29, -21, -26, 272, -29, -18,
 -26, -29, -29, -29, -18, -18, -18 };
short yydef[]={

  59,  -2,   0,  60,  57,  59,   2,   4,   6,   8,
   0,  13,   0,  21,   0,  18, 132,   0, 143, 100,
   0, 107,  59, 145,   0,  23,   0, 126,   0,   0,
   0, 133, 134,   0,   0,  58,   1,   3,   5,   0,
  10,  31,  59, 153,   0,  11,  12,  14,  15,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 108,  59, 151,   0,  59,  27,  29,  59,   0,  36,
   0,  59,   0,   0,   0,  76,  46,  74,  49,   0,
   0,   0,  53,  54,   0,  92, 130, 131,   0,  86,
  88,  61, 144, 101, 122,   0,   0, 132,   0,   0,
   0,   0,   0,   0, 148,   0, 147,   0,  84,  84,
  84, 128, 129, 141, 142,   7,  32,   9,   0,  22,
 143,  97,  98,  -2, 102,  -2,  -2,  -2,   0, 154,
  25, 152, 151,  28,  30, 149,  33,  34,   0,   0,
   0,  74,  42,  43,   0,  63,  77,  78,  80, 132,
 143, 114,   0, 120,   0,   0,  75,  74,   0,  52,
   0,   0,  16,  17,   0,   0,  89,   0, 150, 123,
  96,   0,   0, 135, 136, 137, 138, 139, 140,   0,
  86,   0,   0,  85,   0,   0,  20,  95,   0,  26,
  35,   0,   0, 145,   0,   0,   0,   0,   0,   0,
   0,  81,   0,   0,   0,   0,   0,   0, 121, 109,
  61, 115,  47,  48,   0,   0,   0,  67,  93,  87,
  91,  90,   0,  62, 146,  24, 124, 125, 127, 106,
  59,   0,   0,   0,  63,  45,  64,  65,  66,  79,
  83,  82, 111, 112,  -2, 117,  -2,   0,   0, 113,
  50,   0,  59,   0,  68,  69,  71,  94,   0,   0,
   0,   0,  74,   0,   0, 110,   0,   0,  59,   0,
  72,  37,   0,  59,  74,   0,  44, 119,  51,  55,
  19,  70,  73,  59,   0,   0,  59,  59,  38,  39,
  59,   0,   0,   0,  41,  56,  40 };
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif not lint

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
static char *yydisplay();
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "[yydebug] push state %d\n", yystate);
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	yyyylex();
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		yyyylex();
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */
#ifdef YYIMPROVE
			if ((yyn = yypact[yystate]) > YYFLAG && yyn < YYLAST) {
				register int x;

				for (x = yyn>0 ? yyn : 0; x < YYLAST; ++x)
					if (yychk[yyact[x]] == x - yyn
					    && x - yyn != YYERRCODE)
						yyerror(0, yydisplay(x - yyn));
			}
			yyerror(0,0);
#else
			yyerror( "syntax error" );
#endif
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "[yydebug] recovery pops %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "[yydebug] recovery discards %s\n", yydisplay(yychar) );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("[yydebug] reduce by (%d), uncover %d\n",yyn, yyr1[yyn]);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 1:
# line 137 "awk.y"
{ expression_value = yypvt[-1].nodeval; } break;
case 2:
# line 142 "awk.y"
{ 
			if (yypvt[-0].nodeval != NULL)
				yyval.nodeval = yypvt[-0].nodeval;
			else
				yyval.nodeval = NULL;
			yyerrok;
		} break;
case 3:
# line 151 "awk.y"
{
			if (yypvt[-0].nodeval == NULL)
				yyval.nodeval = yypvt[-1].nodeval;
			else if (yypvt[-1].nodeval == NULL)
				yyval.nodeval = yypvt[-0].nodeval;
			else {
				if (yypvt[-1].nodeval->type != Node_rule_list)
					yypvt[-1].nodeval = node(yypvt[-1].nodeval, Node_rule_list,
						(NODE*)NULL);
				yyval.nodeval = append_right (yypvt[-1].nodeval,
				   node(yypvt[-0].nodeval, Node_rule_list,(NODE *) NULL));
			}
			yyerrok;
		} break;
case 4:
# line 165 "awk.y"
{ yyval.nodeval = NULL; } break;
case 5:
# line 166 "awk.y"
{ yyval.nodeval = NULL; } break;
case 6:
# line 170 "awk.y"
{ io_allowed = 0; } break;
case 7:
# line 172 "awk.y"
{
		if (begin_block) {
			if (begin_block->type != Node_rule_list)
				begin_block = node(begin_block, Node_rule_list,
					(NODE *)NULL);
			append_right (begin_block, node(
			    node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval),
			    Node_rule_list, (NODE *)NULL) );
		} else
			begin_block = node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval);
		yyval.nodeval = NULL;
		io_allowed = 1;
		yyerrok;
	  } break;
case 8:
# line 186 "awk.y"
{ io_allowed = 0; } break;
case 9:
# line 188 "awk.y"
{
		if (end_block) {
			if (end_block->type != Node_rule_list)
				end_block = node(end_block, Node_rule_list,
					(NODE *)NULL);
			append_right (end_block, node(
			    node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval),
			    Node_rule_list, (NODE *)NULL));
		} else
			end_block = node((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval);
		yyval.nodeval = NULL;
		io_allowed = 1;
		yyerrok;
	  } break;
case 10:
# line 203 "awk.y"
{
		msg ("error near line %d: BEGIN blocks must have an action part", lineno);
		errcount++;
		yyerrok;
	  } break;
case 11:
# line 209 "awk.y"
{
		msg ("error near line %d: END blocks must have an action part", lineno);
		errcount++;
		yyerrok;
	  } break;
case 12:
# line 215 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_rule_node, yypvt[-0].nodeval); yyerrok; } break;
case 13:
# line 217 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_rule_node, yypvt[-0].nodeval); yyerrok; } break;
case 14:
# line 219 "awk.y"
{ if(yypvt[-1].nodeval) yyval.nodeval = node (yypvt[-1].nodeval, Node_rule_node, (NODE *)NULL); yyerrok; } break;
case 15:
# line 221 "awk.y"
{
			func_install(yypvt[-1].nodeval, yypvt[-0].nodeval);
			yyval.nodeval = NULL;
			yyerrok;
		} break;
case 16:
# line 230 "awk.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 17:
# line 232 "awk.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 18:
# line 237 "awk.y"
{
			param_counter = 0;
		} break;
case 19:
# line 241 "awk.y"
{
			yyval.nodeval = append_right(make_param(yypvt[-4].sval), yypvt[-2].nodeval);
			can_return = 1;
		} break;
case 20:
# line 249 "awk.y"
{
		yyval.nodeval = yypvt[-1].nodeval;
		can_return = 0;
	  } break;
case 21:
# line 258 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 22:
# line 260 "awk.y"
{ yyval.nodeval = mkrangenode ( node(yypvt[-2].nodeval, Node_cond_pair, yypvt[-0].nodeval) ); } break;
case 23:
# line 269 "awk.y"
{ ++want_regexp; } break;
case 24:
# line 271 "awk.y"
{
		  want_regexp = 0;
		  yyval.nodeval = node((NODE *)NULL,Node_regex,(NODE *)mk_re_parse(yypvt[-1].sval, 0));
		  yyval.nodeval -> re_case = 0;
		  emalloc (yyval.nodeval -> re_text, char *, strlen(yypvt[-1].sval)+1, "regexp");
		  strcpy (yyval.nodeval -> re_text, yypvt[-1].sval);
		} break;
case 25:
# line 282 "awk.y"
{
			/* empty actions are different from missing actions */
			yyval.nodeval = node ((NODE *) NULL, Node_illegal, (NODE *) NULL);
		} break;
case 26:
# line 287 "awk.y"
{ yyval.nodeval = yypvt[-2].nodeval ; } break;
case 27:
# line 292 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 28:
# line 294 "awk.y"
{
			if (yypvt[-1].nodeval == NULL || yypvt[-1].nodeval->type != Node_statement_list)
				yypvt[-1].nodeval = node(yypvt[-1].nodeval, Node_statement_list,(NODE *)NULL);
	    		yyval.nodeval = append_right(yypvt[-1].nodeval,
				node( yypvt[-0].nodeval, Node_statement_list, (NODE *)NULL));
	    		yyerrok;
		} break;
case 29:
# line 302 "awk.y"
{ yyval.nodeval = NULL; } break;
case 30:
# line 304 "awk.y"
{ yyval.nodeval = NULL; } break;
case 31:
# line 309 "awk.y"
{ yyval.nodetypeval = Node_illegal; } break;
case 32:
# line 311 "awk.y"
{ yyval.nodetypeval = Node_illegal; } break;
case 33:
# line 317 "awk.y"
{ yyval.nodeval = NULL; } break;
case 34:
# line 319 "awk.y"
{ yyval.nodeval = NULL; } break;
case 35:
# line 321 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 36:
# line 323 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 37:
# line 325 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, Node_K_while, yypvt[-0].nodeval); } break;
case 38:
# line 327 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_K_do, yypvt[-5].nodeval); } break;
case 39:
# line 329 "awk.y"
{
		yyval.nodeval = node (yypvt[-0].nodeval, Node_K_arrayfor, make_for_loop(variable(yypvt[-5].sval),
			(NODE *)NULL, variable(yypvt[-3].sval)));
	  } break;
case 40:
# line 334 "awk.y"
{
		yyval.nodeval = node(yypvt[-0].nodeval, Node_K_for, (NODE *)make_for_loop(yypvt[-7].nodeval, yypvt[-5].nodeval, yypvt[-3].nodeval));
	  } break;
case 41:
# line 338 "awk.y"
{
		yyval.nodeval = node (yypvt[-0].nodeval, Node_K_for,
			(NODE *)make_for_loop(yypvt[-6].nodeval, (NODE *)NULL, yypvt[-3].nodeval));
	  } break;
case 42:
# line 344 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_break, (NODE *)NULL); } break;
case 43:
# line 347 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_continue, (NODE *)NULL); } break;
case 44:
# line 349 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-5].nodetypeval, yypvt[-1].nodeval); } break;
case 45:
# line 351 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-3].nodetypeval, yypvt[-1].nodeval); } break;
case 46:
# line 353 "awk.y"
{ if (! io_allowed) yyerror("next used in BEGIN or END action"); } break;
case 47:
# line 355 "awk.y"
{ yyval.nodeval = node ((NODE *)NULL, Node_K_next, (NODE *)NULL); } break;
case 48:
# line 357 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_K_exit, (NODE *)NULL); } break;
case 49:
# line 359 "awk.y"
{ if (! can_return) yyerror("return used outside function context"); } break;
case 50:
# line 361 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_K_return, (NODE *)NULL); } break;
case 51:
# line 363 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-4].sval), Node_K_delete, yypvt[-2].nodeval); } break;
case 52:
# line 365 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 53:
# line 370 "awk.y"
{ yyval.nodetypeval = yypvt[-0].nodetypeval; } break;
case 54:
# line 372 "awk.y"
{ yyval.nodetypeval = yypvt[-0].nodetypeval; } break;
case 55:
# line 377 "awk.y"
{
		yyval.nodeval = node(yypvt[-3].nodeval, Node_K_if, 
			node(yypvt[-0].nodeval, Node_if_branches, (NODE *)NULL));
	  } break;
case 56:
# line 383 "awk.y"
{ yyval.nodeval = node (yypvt[-6].nodeval, Node_K_if,
				node (yypvt[-3].nodeval, Node_if_branches, yypvt[-0].nodeval)); } break;
case 57:
# line 389 "awk.y"
{ yyval.nodetypeval = NULL; } break;
case 58:
# line 391 "awk.y"
{ yyval.nodetypeval = NULL; } break;
case 59:
# line 396 "awk.y"
{ yyval.nodetypeval = NULL; } break;
case 60:
# line 398 "awk.y"
{ yyval.nodetypeval = NULL; } break;
case 61:
# line 403 "awk.y"
{ yyval.nodeval = NULL; } break;
case 62:
# line 405 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_input, (NODE *)NULL); } break;
case 63:
# line 410 "awk.y"
{ yyval.nodeval = NULL; } break;
case 64:
# line 412 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_output, (NODE *)NULL); } break;
case 65:
# line 414 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_append, (NODE *)NULL); } break;
case 66:
# line 416 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_redirect_pipe, (NODE *)NULL); } break;
case 67:
# line 421 "awk.y"
{ yyval.nodeval = NULL; } break;
case 68:
# line 423 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 69:
# line 428 "awk.y"
{ yyval.nodeval = make_param(yypvt[-0].sval); } break;
case 70:
# line 430 "awk.y"
{ yyval.nodeval = append_right(yypvt[-2].nodeval, make_param(yypvt[-0].sval)); yyerrok; } break;
case 71:
# line 432 "awk.y"
{ yyval.nodeval = NULL; } break;
case 72:
# line 434 "awk.y"
{ yyval.nodeval = NULL; } break;
case 73:
# line 436 "awk.y"
{ yyval.nodeval = NULL; } break;
case 74:
# line 442 "awk.y"
{ yyval.nodeval = NULL; } break;
case 75:
# line 444 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 76:
# line 449 "awk.y"
{ yyval.nodeval = NULL; } break;
case 77:
# line 451 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 78:
# line 456 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL); } break;
case 79:
# line 458 "awk.y"
{
		yyval.nodeval = append_right(yypvt[-2].nodeval,
			node( yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL));
		yyerrok;
	  } break;
case 80:
# line 464 "awk.y"
{ yyval.nodeval = NULL; } break;
case 81:
# line 466 "awk.y"
{ yyval.nodeval = NULL; } break;
case 82:
# line 468 "awk.y"
{ yyval.nodeval = NULL; } break;
case 83:
# line 470 "awk.y"
{ yyval.nodeval = NULL; } break;
case 84:
# line 475 "awk.y"
{ yyval.nodeval = NULL; } break;
case 85:
# line 477 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 86:
# line 482 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL); } break;
case 87:
# line 484 "awk.y"
{
			yyval.nodeval = append_right(yypvt[-2].nodeval,
				node( yypvt[-0].nodeval, Node_expression_list, (NODE *)NULL));
			yyerrok;
		} break;
case 88:
# line 490 "awk.y"
{ yyval.nodeval = NULL; } break;
case 89:
# line 492 "awk.y"
{ yyval.nodeval = NULL; } break;
case 90:
# line 494 "awk.y"
{ yyval.nodeval = NULL; } break;
case 91:
# line 496 "awk.y"
{ yyval.nodeval = NULL; } break;
case 92:
# line 501 "awk.y"
{ want_assign = 0; } break;
case 93:
# line 503 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-2].nodetypeval, yypvt[-0].nodeval); } break;
case 94:
# line 505 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval), Node_in_array, yypvt[-3].nodeval); } break;
case 95:
# line 507 "awk.y"
{
		  yyval.nodeval = node (yypvt[-0].nodeval, Node_K_getline,
			 node (yypvt[-3].nodeval, Node_redirect_pipein, (NODE *)NULL));
		} break;
case 96:
# line 512 "awk.y"
{
		  /* "too painful to do right" */
		  /*
		  if (! io_allowed && $3 == NULL)
			yyerror("non-redirected getline illegal inside BEGIN or END action");
		  */
		  yyval.nodeval = node (yypvt[-1].nodeval, Node_K_getline, yypvt[-0].nodeval);
		} break;
case 97:
# line 521 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_and, yypvt[-0].nodeval); } break;
case 98:
# line 523 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_or, yypvt[-0].nodeval); } break;
case 99:
# line 525 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval); } break;
case 100:
# line 527 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 101:
# line 529 "awk.y"
{ yyval.nodeval = node((NODE *) NULL, Node_nomatch, yypvt[-0].nodeval); } break;
case 102:
# line 531 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval), Node_in_array, yypvt[-2].nodeval); } break;
case 103:
# line 533 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval); } break;
case 104:
# line 535 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_less, yypvt[-0].nodeval); } break;
case 105:
# line 537 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_greater, yypvt[-0].nodeval); } break;
case 106:
# line 539 "awk.y"
{ yyval.nodeval = node(yypvt[-4].nodeval, Node_cond_exp, node(yypvt[-2].nodeval, Node_if_branches, yypvt[-0].nodeval));} break;
case 107:
# line 541 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 108:
# line 543 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_concat, yypvt[-0].nodeval); } break;
case 109:
# line 548 "awk.y"
{ want_assign = 0; } break;
case 110:
# line 550 "awk.y"
{ yyval.nodeval = node (yypvt[-3].nodeval, yypvt[-2].nodetypeval, yypvt[-0].nodeval); } break;
case 111:
# line 552 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_and, yypvt[-0].nodeval); } break;
case 112:
# line 554 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_or, yypvt[-0].nodeval); } break;
case 113:
# line 556 "awk.y"
{
		  /* "too painful to do right" */
		  /*
		  if (! io_allowed && $3 == NULL)
			yyerror("non-redirected getline illegal inside BEGIN or END action");
		  */
		  yyval.nodeval = node (yypvt[-1].nodeval, Node_K_getline, yypvt[-0].nodeval);
		} break;
case 114:
# line 565 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 115:
# line 567 "awk.y"
{ yyval.nodeval = node((NODE *) NULL, Node_nomatch, yypvt[-0].nodeval); } break;
case 116:
# line 569 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval); } break;
case 117:
# line 571 "awk.y"
{ yyval.nodeval = node (variable(yypvt[-0].sval), Node_in_array, yypvt[-2].nodeval); } break;
case 118:
# line 573 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, yypvt[-1].nodetypeval, yypvt[-0].nodeval); } break;
case 119:
# line 575 "awk.y"
{ yyval.nodeval = node(yypvt[-4].nodeval, Node_cond_exp, node(yypvt[-2].nodeval, Node_if_branches, yypvt[-0].nodeval));} break;
case 120:
# line 577 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 121:
# line 579 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_concat, yypvt[-0].nodeval); } break;
case 122:
# line 584 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_not,(NODE *) NULL); } break;
case 123:
# line 586 "awk.y"
{ yyval.nodeval = yypvt[-1].nodeval; } break;
case 124:
# line 588 "awk.y"
{ yyval.nodeval = snode (yypvt[-1].nodeval, Node_builtin, yypvt[-3].ptrval); } break;
case 125:
# line 590 "awk.y"
{ yyval.nodeval = snode (yypvt[-1].nodeval, Node_builtin, yypvt[-3].ptrval); } break;
case 126:
# line 592 "awk.y"
{ yyval.nodeval = snode ((NODE *)NULL, Node_builtin, yypvt[-0].ptrval); } break;
case 127:
# line 594 "awk.y"
{
		yyval.nodeval = node (yypvt[-1].nodeval, Node_func_call, make_string(yypvt[-3].sval, strlen(yypvt[-3].sval)));
	  } break;
case 128:
# line 598 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_preincrement, (NODE *)NULL); } break;
case 129:
# line 600 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_predecrement, (NODE *)NULL); } break;
case 130:
# line 602 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_postincrement, (NODE *)NULL); } break;
case 131:
# line 604 "awk.y"
{ yyval.nodeval = node (yypvt[-1].nodeval, Node_postdecrement, (NODE *)NULL); } break;
case 132:
# line 606 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 133:
# line 608 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 134:
# line 610 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 135:
# line 614 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_exp, yypvt[-0].nodeval); } break;
case 136:
# line 616 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_times, yypvt[-0].nodeval); } break;
case 137:
# line 618 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_quotient, yypvt[-0].nodeval); } break;
case 138:
# line 620 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_mod, yypvt[-0].nodeval); } break;
case 139:
# line 622 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_plus, yypvt[-0].nodeval); } break;
case 140:
# line 624 "awk.y"
{ yyval.nodeval = node (yypvt[-2].nodeval, Node_minus, yypvt[-0].nodeval); } break;
case 141:
# line 626 "awk.y"
{ yyval.nodeval = node (yypvt[-0].nodeval, Node_unary_minus, (NODE *)NULL); } break;
case 142:
# line 628 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 143:
# line 633 "awk.y"
{ yyval.nodeval = NULL; } break;
case 144:
# line 635 "awk.y"
{ yyval.nodeval = yypvt[-0].nodeval; } break;
case 145:
# line 640 "awk.y"
{ want_assign = 1; yyval.nodeval = variable (yypvt[-0].sval); } break;
case 146:
# line 642 "awk.y"
{ want_assign = 1; yyval.nodeval = node (variable(yypvt[-3].sval), Node_subscript, yypvt[-1].nodeval); } break;
case 147:
# line 644 "awk.y"
{ want_assign = 1; yyval.nodeval = node (yypvt[-0].nodeval, Node_field_spec, (NODE *)NULL); } break;
case 149:
# line 652 "awk.y"
{ yyerrok; } break;
case 150:
# line 656 "awk.y"
{ yyval.nodetypeval = Node_illegal; yyerrok; } break;
case 153:
# line 665 "awk.y"
{ yyerrok; } break;
case 154:
# line 668 "awk.y"
{ yyval.nodetypeval = Node_illegal; yyerrok; } break;
		}
		goto yystack;  /* stack new state and value */

	}

#include <ctype.h>

#define	DIM(x)	(sizeof(x) / sizeof x[0])

static char *
yydisplay(ch)
register int ch;
{
	static char buf[15];
	static char *token[] = {
#		include "y.tok.h"	/* token names */
		0
	};

	switch (ch) {
	case 0:
		return "[end of file]";
	case YYERRCODE:
		return "[error]";
	case '\b':
		return "'\\b'";
	case '\f':
		return "'\\f'";
	case '\n':
		return "'\\n'";
	case '\r':
		return "'\\r'";
	case '\t':
		return "'\\t'";
	}

	if (ch > 256 && ch < 256 + DIM(token))
		return token[ch - 257];
	if (isascii(ch) && isprint(ch))
		sprintf(buf, "'%c'", ch);
	else if (ch < 256)
		sprintf(buf, "'%04.3o'", ch);
	else
		sprintf(buf, "token %d", ch);
	return buf;
}

static
yyyylex()
{
	if (yychar < 0) {
		if ((yychar = yylex()) < 0)
			yychar = 0;
#ifdef YYDEBUG
		if (yydebug)
			printf("[yydebug] reading %s\n", yydisplay(yychar));
#endif
	}
}
