/*
 * debug.c - gawk debugger 
 */

/* 
 * Copyright (C) 2004, 2010, 2011 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "awk.h"
#include "cmd.h"

#ifndef O_RDONLY
#include <fcntl.h>	/* open() */
#endif

extern int exiting;
extern SRCFILE *srcfiles;
extern INSTRUCTION *rule_list;
extern INSTRUCTION *code_block;
extern NODE **fcall_list;
extern long fcall_count;
extern FILE *output_fp;
extern IOBUF *curfile;
extern const char *command_file;
extern int r_interpret(INSTRUCTION *);
extern int zzparse(void);
#define read_command()		(void) zzparse()

extern int free_instruction(INSTRUCTION *, int *);
extern void destroy_symbol(char *name);
extern const char *redir2str(int redirtype);

static char *linebuf = NULL;	/* used to print a single line of source */
static size_t linebuf_len;

FILE *out_fp;
char *dPrompt;
char *commands_Prompt = "> ";	/* breakpoint or watchpoint commands list */
char *eval_Prompt = "@> ";		/* awk statement(s) */

int input_from_tty = FALSE;
int input_fd;

static SRCFILE *cur_srcfile;
static long cur_frame = 0;
static INSTRUCTION *cur_pc;
int cur_rule = 0;

static int prog_running = FALSE;

struct condition {
	INSTRUCTION *code;
	AWK_CONTEXT *ctxt;
	char *expr;
};

struct commands_item {
	struct commands_item *next;
	struct commands_item *prev;
	int cmd;
	char *cmd_string;
	CMDARG *arg;
};

/* breakpoint structure */		
typedef struct break_point {
	struct break_point *next;
	struct break_point *prev;
	int number;

	long ignore_count;
	long hit_count;
	char *src;
	INSTRUCTION *bpi;	/* Op_breakpoint */

	struct commands_item commands;  /* list of commands to run */
	int silent;

	struct condition cndn;

	short flags;
#define BP_ENABLE       1
#define BP_ENABLE_ONCE  2		/* enable once */
#define BP_TEMP         4
#define BP_IGNORE       8

} BREAKPOINT; 

static BREAKPOINT breakpoints = { &breakpoints, &breakpoints, 0 };

#ifdef HAVE_LIBREADLINE
/* do_save -- save command */
static int sess_history_base = 0;
#endif

/* 'list' command */
static int last_printed_line = 0;
static int last_print_count;	/* # of lines printed */

/* watch or display item */
struct list_item {
	struct list_item *next;
	struct list_item *prev;
	int number;     /* item number */

	NODE *symbol;   /* variable or function param */
	NODE **subs;    /* subscripts */
	int num_subs;	/* subscript(dimension) count */ 
	char *sname;	/* symbol or param name */

	long fcall_count;

	struct commands_item commands;		
	int silent;
	struct condition cndn;

	/* This is for the value of the watched item */
	union {
		NODE *n;
		long l;
	} value[2];
#define cur_value value[0].n
#define cur_size  value[0].l
#define old_value value[1].n
#define old_size  value[1].l

	int flags;
#define PARAM           1
#define SUBSCRIPT       2
#define FIELD_NUM       4
#define OLD_IS_ARRAY    8    /* old item is array */ 
#define CUR_IS_ARRAY    16   /* current item is array */ 
};

#define IS_PARAM(d)	(((d)->flags & PARAM) != 0)
#define IS_SUBSCRIPT(d)	(((d)->flags & SUBSCRIPT) != 0)
#define IS_FIELD(d)	(((d)->flags & FIELD_NUM) != 0)
#define WATCHING_ARRAY(d)	(((d)->flags & CUR_IS_ARRAY) != 0)

static struct list_item display_list = { &display_list, &display_list, 0 };
static struct list_item watch_list = { &watch_list, &watch_list, 0 };


/* Structure to maintain data for processing debugger commands */

static struct {
	long fcall_count;    /* 'finish', 'until', 'next', 'step', 'nexti' commands */
	int sourceline;      /* source line number last
	                      * time we stopped execution,
	                      * used by next, until and step commands
	                      */
	char *source;        /* next, until and step */

	INSTRUCTION *pc;     /* 'until' and 'return' commands */
	int repeat_count;    /* 'step', 'next', 'stepi', 'nexti' commands */
	int print_frame;     /* print frame info,  'finish' and 'until' */
	int print_ret;       /* print returned value, 'finish' */ 
	int break_point;     /* non-zero (breakpoint number) if stopped at break point */
	int watch_point;     /* non-zero (watchpoint number) if stopped at watch point */

	int (*check_func)(INSTRUCTION **);  /* function to decide when to suspend
	                                         * awk interpreter and return control
	                                         * to debugger command interpreter.
	                                         */
 
	enum argtype command;		 /* command type */
} stop;


/* restart related stuff */
extern char **d_argv;	/* copy of argv array */
static int need_restart = FALSE;
enum { BREAK=1, WATCH, DISPLAY, HISTORY, OPTION };
static const char *const env_variable[] = {
"",
"DGAWK_BREAK",
"DGAWK_WATCH",
"DGAWK_DISPLAY",
"DGAWK_HISTORY",
"DGAWK_OPTION",
};
static void serialize(int );
static void unserialize(int );
static const char *commands_string = NULL;
static int commands_string_len = 0;
static char line_sep;
#define FSEP	(char)'\037'
#define RSEP	(char)'\036'
#define CSEP	(char)'\035'


/* debugger option */
struct dbg_option {
	const char *name;
	int *num_val;
	char **str_val;
	void (*assign)(const char *);
	const char *help_txt; 
};

#define DEFAULT_HISTFILE	"./.dgawk_history"
#define DEFAULT_OPTFILE		"./.dgawkrc"
#define DEFAULT_PROMPT		"dgawk> "
#define DEFAULT_LISTSIZE	15
#define DEFAULT_HISTSIZE	100

static void set_gawk_output(const char *file);
static void set_prompt(const char *value);
static void set_listsize(const char *value);
static void set_trace(const char *value);
static void set_save_history(const char *value);
static void set_save_options(const char *value);
static void set_history_size(const char *value);
static const char *options_file = DEFAULT_OPTFILE;
#ifdef HAVE_LIBREADLINE
static const char *history_file = DEFAULT_HISTFILE;
#endif

/* keep all option variables in one place */

static char *output_file = "/dev/stdout";  /* gawk output redirection */
char *dgawk_Prompt = NULL;          /* initialized in interpret */
static int list_size = DEFAULT_LISTSIZE;   /* # of lines that 'list' prints */
static int do_trace = FALSE;
static int do_save_history = TRUE;
static int do_save_options = TRUE;
static int history_size = DEFAULT_HISTSIZE;  /* max # of lines in history file */

static const struct dbg_option option_list[] = {
{"history_size", &history_size, NULL, &set_history_size,
	gettext_noop("set or show the number of lines to keep in history file.") },
{"listsize", &list_size, NULL, &set_listsize,
	gettext_noop("set or show the list command window size.") },
{"outfile", NULL, &output_file, &set_gawk_output,
	gettext_noop("set or show gawk output file.") },
{"prompt", NULL, &dgawk_Prompt, &set_prompt,
	gettext_noop("set or show debugger prompt."), },
{"save_history", &do_save_history, NULL, &set_save_history,
	gettext_noop("(un)set or show saving of command history (value=on|off).") },
{"save_options", &do_save_options, NULL, &set_save_options,
	gettext_noop("(un)set or show saving of options (value=on|off).") },
{"trace", &do_trace, NULL, &set_trace,
	gettext_noop("(un)set or show instruction tracing (value=on|off).") },
{0, NULL, NULL, NULL, 0},
};

static void save_options(const char *file);


/* pager */
jmp_buf pager_quit_tag;
int pager_quit_tag_valid;
static int screen_width = INT_MAX;	/* no of columns */
static int screen_height = INT_MAX;	/* no of rows */
static int pager_lines_printed = 0;	/* no of lines printed so far */ 

static void restart(int run) ATTRIBUTE_NORETURN;
static void close_all(void);
static int open_readfd(const char *file);
static int find_lines(SRCFILE *s);
static SRCFILE *source_find(char *src);
static int print_lines(char *src, int start_line, int nlines);
static void print_symbol(NODE *r, int isparam);
static NODE *find_frame(long num);
static NODE *find_param(const char *name, long num, char **pname);
static NODE *find_symbol(const char *name, char **pname);
static NODE *find_array(const char *name);
static void print_field(long field_num);
static int print_function(INSTRUCTION *pc, void *);
static void print_frame(NODE *func, char *src, int srcline);
static void print_numbered_frame(long num);
static void print_cur_frame_and_sourceline(void);
static INSTRUCTION *find_rule(char *src, long lineno);
static INSTRUCTION *mk_breakpoint(char *src, int srcline);
static int execute_commands(struct commands_item *commands);
static void delete_commands_item(struct commands_item *c);
static NODE *execute_code(volatile INSTRUCTION *code);
static int pre_execute_code(INSTRUCTION **pi);
static int parse_condition(int type, int num, char *expr);
static BREAKPOINT *add_breakpoint(INSTRUCTION *, INSTRUCTION *, char *, int);
static BREAKPOINT *set_breakpoint_next(INSTRUCTION *rp, INSTRUCTION *ip);
static BREAKPOINT *set_breakpoint_at(INSTRUCTION *, int, int);
static int set_breakpoint(CMDARG *arg, int temporary);
static void delete_breakpoint(BREAKPOINT *b);
static BREAKPOINT *find_breakpoint(long num);
static void display(struct list_item *d);
static struct list_item *find_item(struct list_item *list, long num);
static struct list_item *add_item(struct list_item *list, int type, NODE *symbol, char *pname);
static void delete_item(struct list_item *d);
static int breakpoint_triggered(BREAKPOINT *b);
static int watchpoint_triggered(struct list_item *w);

static void print_instruction(INSTRUCTION *pc, Func_print print_func, FILE *fp, int in_dump);
static void next_command();
static char *g_readline(const char *prompt);
static int prompt_yes_no(const char *, char , int , FILE *);

static struct pf_data {
	Func_print print_func;
	int defn;
	FILE *fp;
} pf_data;

char * (*read_a_line)(const char *) = 0;	/* reads a line of input */

struct command_source
{
	int fd;
	int is_tty;
	char * (*read_func)(const char *);
	int (*close_func)(int);
	int eof_status;		/* see push_cmd_src */
	int cmd;			/* D_source or 0 */
	char *str;			/* sourced file */
	struct command_source *next;
};

static struct command_source *cmd_src = NULL;

#define get_param_count(f)   (f)->lnode->param_cnt
#define get_params(f)        (f)->parmlist


#define CHECK_PROG_RUNNING() \
	do { \
		if (! prog_running) { \
			d_error(_("program not running.")); \
			return FALSE; \
		} \
	} while (FALSE)


/* g_readline --  read a line of text; the interface is like 'readline' but
 *		without	any command-line editing; used when not compiled with
 *		readline support and/or input is not from terminal (prompt set to NULL).
 */

static char *
g_readline(const char *prompt)
{
	char *line;
	size_t line_size = 100;
	static char buf[2];
	char *p, *end;
	int n;

	if (input_from_tty && prompt && *prompt)
		fprintf(out_fp, "%s", prompt);

	emalloc(line, char *, line_size + 1, "g_readline");
	p = line;
	end = line + line_size;
	while ((n = read(input_fd, buf, 1)) > 0) {
		if (buf[0] == '\n') {
			if (p > line && p[-1] == '\r')
				p--;
			break;
		}
		if (p == end) {
			erealloc(line, char *, 2 * line_size + 1, "g_readline");
			p = line + line_size;
			line_size *= 2;
			end = line + line_size;
		}
		*p++ = buf[0];
	}
	if (n == -1 || (n == 0 && p == line)) {
		efree(line);
		return NULL;
	}
	*p = '\0';
	return line;
}


/* d_error --- print an error message */

void
d_error(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	fprintf(out_fp, _("error: "));
	vfprintf(out_fp, mesg, args);
	fprintf(out_fp, "\n");
	va_end(args);
}

/* find_lines --- find the positions of the lines in the source file. */

static int
find_lines(SRCFILE *s)
{
	char *buf, *p, *end;
	int n;
	int ofs = 0;
	int *pos;
	int pos_size;
	int maxlen = 0;
	int numlines = 0;
	char lastchar = '\0';

	emalloc(buf, char *, s->bufsize, "find_lines");
	pos_size = s->srclines;
	emalloc(s->line_offset, int *, (pos_size + 2) * sizeof(int), "find_lines");
	pos = s->line_offset;
	pos[0] = 0;

	while ((n = read(s->fd, buf, s->bufsize)) > 0) {
		end = buf + n;
		lastchar = buf[n - 1];
		p = buf;
		while (p < end) {
			if (*p++ == '\n') {
				if (++numlines > pos_size) {
					erealloc(s->line_offset, int *, (2 * pos_size + 2) * sizeof(int), "find_lines");
					pos = s->line_offset + pos_size;
					pos_size *= 2;
				}
				*++pos = ofs + (p - buf);
				if ((pos[0] - pos[-1]) > maxlen)
					maxlen = pos[0] - pos[-1];	/* length including NEWLINE */
			}
		}
		ofs += n;
	}
	efree(buf);

	if (n == -1) {
		d_error(_("can't read source file `%s' (%s)"),
					s->src, strerror(errno));
		return -1;
	}
	if (ofs <= 0) {
		fprintf(out_fp, _("source file `%s' is empty.\n"), s->src);
		return -1;
	}

	if (lastchar != '\n') {
		/* fake a NEWLINE at end */
		*++pos = ofs + 1;
		numlines++;
		if ((pos[0] - pos[-1]) > maxlen)
			maxlen = pos[0] - pos[-1];
	}
	s->maxlen = maxlen;
	s->srclines = numlines;
	return 0;
}

/* source_find --- return the SRCFILE struct for the source 'src' */

static SRCFILE *
source_find(char *src)
{
	SRCFILE *s;
	struct stat sbuf;
	char *path;
	int errno_val = 0;

	if (src == NULL || *src == '\0') {
		d_error(_("no current source file."));
		return NULL;
	}

	if (cur_srcfile->src == src)  /* strcmp(cur_srcfile->src, src) == 0 */
		return cur_srcfile;

	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if ((s->stype == SRC_FILE || s->stype == SRC_INC)
				&& strcmp(s->src, src) == 0)
			return s;
	}

	path = find_source(src, &sbuf, &errno_val);
	if (path != NULL) {
		for (s = srcfiles->next; s != srcfiles; s = s->next) {
			if ((s->stype == SRC_FILE || s->stype == SRC_INC)
			    && files_are_same(path, s)) {
				efree(path);
				return s;
			}
		efree(path);
		}
	}

	d_error(_("cannot find source file named `%s' (%s)"), src, strerror(errno_val));
	return NULL;
}

/* print_lines --- print source lines, and update 'cur_srcfile' */

static int
print_lines(char *src, int start_line, int nlines)
{
	SRCFILE *s;
	int *pos;
	int i;
	struct stat sbuf;

	s = source_find(src);
	if (s == NULL)
		return -1;
	if (s->fd <= INVALID_HANDLE && (s->fd = srcopen(s)) <= INVALID_HANDLE) {
		d_error(_("can't open source file `%s' for reading (%s)"),
				src, strerror(errno));
		return -1;
	}

	if (fstat(s->fd, &sbuf) == 0 && s->mtime < sbuf.st_mtime) {
		fprintf(out_fp, _("WARNING: source file `%s' modified since program compilation.\n"),
				src);
		efree(s->line_offset);
		s->line_offset = NULL;
		s->mtime = sbuf.st_mtime;

		/* reopen source file */
		close(s->fd);
		s->fd = INVALID_HANDLE;
		if ((s->fd = srcopen(s)) <= INVALID_HANDLE) {
			d_error(_("can't open source file `%s' for reading (%s)"),
					src, strerror(errno));
			return -1;
		}
	}

	if (s->line_offset == NULL && find_lines(s) != 0)
		return -1;
  	if (start_line < 1 || start_line > s->srclines) {
		d_error(_("line number %d out of range; `%s' has %d lines"),
					start_line, src, s->srclines);
		return -1;
	}

	assert(nlines > 0);
	if ((start_line + nlines - 1) > s->srclines)
		nlines = s->srclines - start_line + 1;

	pos = s->line_offset;
	if (lseek(s->fd, (off_t) pos[start_line - 1], SEEK_SET) < 0) {
		d_error("%s: %s", src, strerror(errno));
		return -1;
	}

	if (linebuf == NULL) {
		emalloc(linebuf, char *, s->maxlen + 20, "print_lines"); /* 19 for line # */
		linebuf_len = s->maxlen;
	} else if (linebuf_len < s->maxlen) {
		erealloc(linebuf, char *, s->maxlen + 20, "print_lines");
		linebuf_len = s->maxlen;
	}

	for (i = start_line; i < start_line + nlines; i++) {
		int supposed_len, len;
		char *p;
		
		sprintf(linebuf, "%-8d", i);

		/* mark the line about to be executed with =>; nlines > 1
	 	 * condition makes sure that we are in list command
		 */
		if (nlines > 1) {
			BREAKPOINT *b;
			int has_bpt = FALSE;		
			for (b = breakpoints.prev; b != &breakpoints; b = b->prev) {
				if (src == b->src && i == b->bpi->source_line) {
					has_bpt = TRUE;
					break;
				}
			}
			if (prog_running && src == source && i == sourceline) {
				if (has_bpt)
					sprintf(linebuf, "%-4d:b=>", i);
				else
					sprintf(linebuf, "%-4d  =>", i);
			} else if (has_bpt)
				sprintf(linebuf, "%-4d:b  ", i);			
		}

		p = linebuf + strlen(linebuf);
		supposed_len = pos[i] - pos[i - 1];
		len = read(s->fd, p, supposed_len);
		switch (len) {
		case -1:
			d_error(_("can't read source file `%s' (%s)"),
						src, strerror(errno));
			return -1;

		case 0:
			d_error(_("unexpected eof while reading file `%s', line %d"),
						src, i);
			return -1;

		default:
			if (i == s->srclines && p[len - 1] != '\n')
				p[len++] = '\n';
#if 0
			if (len != supposed_len || p[len - 1] != '\n') {
				d_error(_("source file `%s' modified since start of program execution"),
						src);
				return -1;
			}
#endif
			len += (p - linebuf);
			if (fwrite(linebuf, sizeof(char), len, out_fp) != len)
				return -1;
		}
	}

	if (cur_srcfile != s) {
		if (cur_srcfile->fd != INVALID_HANDLE) {
			close(cur_srcfile->fd);
			cur_srcfile->fd = INVALID_HANDLE;
		}
		cur_srcfile = s;
	}
	return (i - 1);		/* no of lines printed */
}

/* do_list --- list command */

int
do_list(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	long line_first, line_last;
	long count = list_size;
	INSTRUCTION *rp;
	char *src = cur_srcfile->src;
	
	line_first = last_printed_line + 1;		/* default or no arg */
	if (arg == NULL)	/* list or list + */
		goto list;

	switch (arg->type) {
	case D_int:		/* list n or list - */
		if (arg->a_int < 0) {		/* list - */
			line_first = last_printed_line - last_print_count - list_size + 1;
			if (line_first < 1) {
				if (last_printed_line != last_print_count)
					line_first = 1;
				else
					return FALSE;
			}
		} else {
line:
			line_first = arg->a_int - list_size / 2;
			if (line_first < 1)
				line_first = 1;
		}
		break;

	case D_range:	/* list m-n */
range:
		line_first = arg->a_int;
		arg = arg->next;
		assert(arg != NULL);
		assert(arg->type == D_int);
		count = arg->a_int - line_first + 1;
		break;

	case D_string:		
		src = arg->a_string;
		if (arg->next != NULL) {
			arg = arg->next;		
			if (arg->type == D_int)	/* list file:n */
				goto line;
			else if (arg->type == D_range)	/* list file:m-n */
				goto range;
			else if (arg->type == D_func)	/* list file:function */
				goto func;
			else
				line_first = 1;
		} else
			line_first = 1;
		break;

	case D_func:	/* list function */
func:
		rp = arg->a_node->code_ptr;
		src = rp->source_file;
		line_first = rp->source_line - list_size / 2;
		if (line_first < 1)
			line_first = 1;
		break;

	default:
		break;
 	}

list:
	line_last = print_lines(src, line_first, count);
	if (line_last != -1) {
		last_printed_line = line_last;
		last_print_count = line_last - line_first + 1;
	} 
	return FALSE;
}

/* do_info --- info command */

int
do_info(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	if (arg == NULL || arg->type != D_argument)
		return FALSE;

	switch (arg->a_argument) {
	case A_SOURCE:
		fprintf(out_fp, _("Current source file: %s\n"), cur_srcfile->src);
		fprintf(out_fp, _("Number of lines: %d\n"), cur_srcfile->srclines);
		break;

	case A_SOURCES:
	{
		SRCFILE *s;
		for (s = srcfiles->next; s != srcfiles; s = s->next) {
			fprintf(out_fp, _("Source file (lines): %s (%d)\n"),
					(s->stype == SRC_FILE || s->stype == SRC_INC) ? s->src
			 		                                      : "cmd. line",
					s->srclines);
		}
	}
		break;

	case A_BREAK:
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			BREAKPOINT *b;
			struct commands_item *c;

			gprintf(out_fp, _("Number  Disp  Enabled  Location\n\n"));
			for (b = breakpoints.prev; b != &breakpoints; b = b->prev) {
				char *disp = "keep";
				if (b->flags & BP_ENABLE_ONCE)
					disp = "dis";
				else if(b->flags & BP_TEMP)
					disp = "del";
				gprintf(out_fp, "%-6d  %-4.4s  %-7.7s  file %s, line #%d\n",
						b->number, disp, (b->flags & BP_ENABLE) ? "yes" : "no",
					 	b->src,	b->bpi->source_line);
				if (b->hit_count > 0)
					gprintf(out_fp, _("\tno of hits = %ld\n"), b->hit_count);
				if (b->flags & BP_IGNORE)
					gprintf(out_fp, _("\tignore next %ld hit(s)\n"), b->ignore_count);
				if (b->cndn.code != NULL)
					gprintf(out_fp, _("\tstop condition: %s\n"), b->cndn.expr);
				if (b->commands.next != &b->commands)
					gprintf(out_fp, _("\tcommands:\n"));
				for (c = b->commands.next; c != &b->commands; c = c->next) {
					gprintf(out_fp, "\t%s\n", c->cmd_string);
					if (c->cmd == D_eval) {
						char *start, *end;
						CMDARG *a = c->arg;
						start = strchr(a->a_string, '{');
						end = strrchr(a->a_string, '}');
						if (start == NULL || end == NULL)
							continue;
						start++;
						*end = '\0';
						gprintf(out_fp, "%s", start);	/* FIXME: translate ? */ 
						*end = '}';
					}
				}
			}
		}
		break;

	case A_FRAME:
		CHECK_PROG_RUNNING();
		fprintf(out_fp, _("Current frame: "));
		print_numbered_frame(cur_frame);
		if (cur_frame < fcall_count) {
			fprintf(out_fp, _("Called by frame: "));
			print_numbered_frame(cur_frame + 1);
		}
		if (cur_frame > 0) {
			fprintf(out_fp, _("Caller of frame: "));
			print_numbered_frame(cur_frame - 1);
		}
		break;

	case A_ARGS:
	case A_LOCALS:
	{
		NODE *f, *func;
		INSTRUCTION *pc;
		int arg_count, pcount;
		int i, from, to;
		char **pnames;

		CHECK_PROG_RUNNING();
		f = find_frame(cur_frame);
		func = f->func_node;
		if (func == NULL) {
			/* print ARGV ? */
			fprintf(out_fp, _("None in main().\n"));
			return FALSE;
		}

		pcount = get_param_count(func);        /* # of defined params */
		pnames = get_params(func);             /* param names */

		pc = (INSTRUCTION *) f->reti;          /* Op_func_call instruction */
		arg_count = (pc + 1)->expr_count;      /* # of arguments supplied */

		if (arg_count > pcount)                /* extra args */
			arg_count = pcount;
		if (arg->a_argument == A_ARGS) {
			from = 0;
			to = arg_count - 1;
		} else {
			from = arg_count;
			to = pcount - 1;
		}

		for (i = from; i <= to; i++) {
			NODE *r;
			r = f->stack[i];
			if (r->type == Node_array_ref)
				r = r->orig_array;
			fprintf(out_fp, "%s = ", pnames[i]);
			print_symbol(r, TRUE);
		}
		if (to < from)
			fprintf(out_fp, "%s",
				arg->a_argument == A_ARGS ?
					_("No arguments.\n") :
					_("No locals.\n"));
	}
		break;

	case A_VARIABLES:
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			gprintf(out_fp, _("All defined variables:\n\n"));
			print_vars(gprintf, out_fp);
		}
		break;

	case A_FUNCTIONS:
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			gprintf(out_fp, _("All defined functions:\n\n"));
			pf_data.print_func = gprintf;
			pf_data.fp = out_fp;
			pf_data.defn = TRUE;
			(void) foreach_func((int (*)(INSTRUCTION *, void *)) print_function,
			                    FALSE, /* sort */
			                    &pf_data /* data */
			                   );
		}
		break;

	case A_DISPLAY:
	case A_WATCH:
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			struct list_item *d, *list;

			if (arg->a_argument == A_DISPLAY) {
				list = &display_list;
				gprintf(out_fp, _("Auto-display variables:\n\n"));
			} else {
				list = &watch_list;
				gprintf(out_fp, _("Watch variables:\n\n"));
			}
			for (d = list->prev; d != list; d = d->prev) {
				int i;
				struct commands_item *c;
				NODE *symbol = d->symbol;

				if (IS_SUBSCRIPT(d)) {
					gprintf(out_fp, "%d:\t%s",  d->number, d->sname);
					for (i = 0; i < d->num_subs; i++) {
						NODE *sub;
						sub = d->subs[i];
						gprintf(out_fp, "[\"%s\"]", sub->stptr);
					}
					gprintf(out_fp, "\n");  
				} else if (IS_FIELD(d))
					gprintf(out_fp, "%d:\t$%ld\n", d->number, (long) symbol->numbr);
				else
					gprintf(out_fp, "%d:\t%s\n", d->number, d->sname);
				if (d->cndn.code != NULL)
					gprintf(out_fp, _("\tstop condition: %s\n"), d->cndn.expr);
				if (d->commands.next != &d->commands)
					gprintf(out_fp, _("\tcommands:\n"));
				for (c = d->commands.next; c != &d->commands; c = c->next) {
					gprintf(out_fp, "\t%s\n", c->cmd_string);
					if (c->cmd == D_eval) {
						char *start, *end;
						CMDARG *a = c->arg;
						start = strchr(a->a_string, '{');
						end = strrchr(a->a_string, '}');
						if (start == NULL || end == NULL)
							continue;
						start++;
						*end = '\0';
						gprintf(out_fp, "%s", start);	/* FIXME: translate ? */  
						*end = '}';
					}
				}

			}
		}
		break;

	default:
		break;
	}

	return FALSE;
}

/* print_symbol --- print a symbol table entry */

static void
print_symbol(NODE *r, int isparam)
{
	switch (r->type) {
	case Node_var_new:
		fprintf(out_fp, "untyped variable\n");
		break;
	case Node_var:
		if (! isparam && r->var_update)
			r->var_update();
		valinfo(r->var_value, fprintf, out_fp);
		break;
	case Node_var_array:
		fprintf(out_fp, "array, %ld elements\n", r->table_size);
		break;
	case Node_func:
		fprintf(out_fp, "`function'\n");
		break;
	default:
		break;
	}
}

/* find_frame --- find frame given a frame number */

static NODE *
find_frame(long num)
{
	assert(num >= 0);
	if (num == 0)
		return frame_ptr;

	assert(prog_running == TRUE);
	assert(num <= fcall_count);
	assert(fcall_list[num] != NULL);
	return fcall_list[num];
}

/* find_param --- find a function parameter in a given frame number */

static NODE *
find_param(const char *name, long num, char **pname)
{
	NODE *r = NULL;
	NODE *f;

	if (pname)
		*pname = NULL;

	if (num < 0 || num > fcall_count || name == NULL)
		return NULL;
	f = find_frame(num);
	if (f->func_node != NULL) {		/* in function */
		NODE *func;
		char **pnames;
		int i, pcount;

		func = f->func_node;
		pnames = get_params(func);
		pcount = get_param_count(func);

		for (i = 0; i < pcount; i++) {
			if (STREQ(name, pnames[i])) {
				r = f->stack[i];
				if (r->type == Node_array_ref)
					r = r->orig_array;
				if (pname)
					*pname = pnames[i];
				break;
			}
		}
	}
	return r;
}

/* find_symbol --- find a symbol in current context */

static
NODE *find_symbol(const char *name, char **pname)
{
	NODE *r = NULL;

	if (pname)
		*pname = NULL;
	if (prog_running)
		r = find_param(name, cur_frame, pname);
	if (r == NULL)
		r = lookup(name);
	if (r == NULL)
		fprintf(out_fp, _("no symbol `%s' in current context\n"), name);
	return r;
}

/* find_array -- find an array in current context */

static NODE *
find_array(const char *name)
{
	NODE *r;
	r = find_symbol(name, NULL);
	if (r != NULL && r->type != Node_var_array) {
		fprintf(out_fp, _("`%s' is not an array\n"), name);
		return NULL;
	}
	return r;
}

/* print_field --- print the value of $n */

static void
print_field(long field_num)
{
	NODE **lhs;
	lhs = get_field(field_num, NULL);
	if (*lhs == Null_field || *lhs == Nnull_string)
		fprintf(out_fp, _("$%ld = uninitialized field\n"), field_num);
	else {
		fprintf(out_fp, "$%ld = ", field_num);
		valinfo(*lhs, fprintf, out_fp);
	}
}

/* print_array --- print the contents of an array */

static int
print_array(volatile NODE *arr, char *arr_name)
{
	NODE *bucket;
	NODE **list;
	int i;
	size_t num_elems = 0;
	volatile NODE *r;
	volatile int ret = 0;
	volatile jmp_buf pager_quit_tag_stack;

	if (arr->var_array == NULL || arr->table_size == 0) {
		gprintf(out_fp, _("array `%s' is empty\n"), arr_name);
		return 0;
	}

	num_elems = arr->table_size;

	/* sort indices, sub_arrays are also sorted! */
	list = assoc_list((NODE *) arr, "@ind_str_asc", SORTED_IN);

	PUSH_BINDING(pager_quit_tag_stack, pager_quit_tag, pager_quit_tag_valid);
	if (setjmp(pager_quit_tag) == 0) {
		for (i = 0; ret == 0 && i < num_elems; i++) {
			bucket = list[i];
			r = bucket->ahvalue;
			if (r->type == Node_var_array)
				ret = print_array(r, r->vname);
			else {
				gprintf(out_fp, "%s[\"%s\"] = ", arr_name, bucket->ahname_str);
				valinfo((NODE *) r, gprintf, out_fp);
			}
		}
	} else
		ret = 1;

	POP_BINDING(pager_quit_tag_stack, pager_quit_tag, pager_quit_tag_valid);

	for (i = 0; i < num_elems; i++)
		unref(list[i]);
	efree(list);

	return ret;
}

/* print_subscript --- print an array element */

static void
print_subscript(NODE *arr, char *arr_name, CMDARG *a, int count)
{
	NODE *r, *subs;

	subs = a->a_node;
	r = in_array(arr, subs);
	if (r == NULL)
		fprintf(out_fp, _("[\"%s\"] not in array `%s'\n"), subs->stptr, arr_name);
	else if (r->type == Node_var_array) {
		if (count > 1)
			print_subscript(r, r->vname, a->next, count - 1);
		else {
			/* print # of elements in array */
			fprintf(out_fp, "%s = ", r->vname);
			print_symbol(r, FALSE);	
		}
	} else {
		fprintf(out_fp, "%s[\"%s\"] = ", arr_name, subs->stptr);
		valinfo(r, fprintf, out_fp);
	}
}

/* do_print_var --- print command */

int
do_print_var(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	NODE *r;
	CMDARG *a;
	char *name, *pname;

	for (a = arg; a != NULL; a = a->next) {
		switch (a->type) {
		case D_variable:
			name = a->a_string;
			if ((r = find_symbol(name, &pname)) != NULL) {
				fprintf(out_fp, "%s = ", name);
				print_symbol(r, (pname != NULL));
			}
			break;

		case D_subscript:
			assert(a->a_count > 0);
			name = a->a_string;
			r = find_array(name);
			if (r != NULL)
				print_subscript(r, name, a->next, a->a_count);
			break;

		case D_array:
			name = a->a_string;
			if ((r = find_array(name)) != NULL) {
				int count = a->a_count;
				for (; count > 0; count--) {
					NODE *value, *subs; 
					a = a->next;
					subs = a->a_node;
					value = in_array(r, subs);
					if (value == NULL) {
						fprintf(out_fp, _("[\"%s\"] not in array `%s'\n"),
									subs->stptr, name);
						break;
					} else if (value->type != Node_var_array) {
						fprintf(out_fp, _("`%s[\"%s\"]' is not an array\n"),
									name, subs->stptr);
						break;
					} else {
						r = value;
						name = r->vname;
					}
				}
				if (count == 0) {
					initialize_pager(out_fp);
					print_array((volatile NODE *) r, name);
				}
			}
			break;

		case D_field:
			print_field(a->a_node->numbr);
			break;

		default:
			/* notably D_node, subscript for invalid array name; skip */
			break;
		}
	}	
	return FALSE;
}		

/* do_set_var --- set command */

int
do_set_var(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	NODE *r, *val;
	NODE **lhs;
	char *name, *pname;

	switch (arg->type) {
	case D_variable:
		name = arg->a_string;
		arg = arg->next;
		val = arg->a_node;

		if ((r = find_symbol(name, &pname)) == NULL)
			break;

		switch (r->type) {
		case Node_var_new:
			r->type = Node_var;
			r->var_value = Nnull_string;
			/* fall through */
		case Node_var:
			lhs = &r->var_value;
			unref(*lhs);
			*lhs = dupnode(val);
			if (pname == NULL && r->var_assign != NULL)
				r->var_assign();
			fprintf(out_fp, "%s = ", name);
			print_symbol(r, (pname != NULL));
			break;

		default:
			d_error(_("`%s' is not a scalar variable"), name);
			break;
		}
		break;

	case D_subscript:
	{
		NODE *subs, *value;
		int count = arg->a_count;

		assert(count > 0);
		name = arg->a_string;
		r = find_array(name);
		if (r == NULL)
			break;
		for (; count > 0; count--) {
			arg = arg->next;
			subs = arg->a_node;
			value = in_array(r, subs);

			if (count == 1) {
				if (value != NULL && value->type == Node_var_array)
					d_error(_("attempt to use array `%s[\"%s\"]' in a scalar context"),
								name, subs->stptr);
				else {
					arg = arg->next;
					val = arg->a_node;
					lhs = assoc_lookup(r, subs, FALSE);
					unref(*lhs);
					*lhs = dupnode(val);
					fprintf(out_fp, "%s[\"%s\"] = ", name, subs->stptr);
					valinfo(*lhs, fprintf, out_fp);
				}
			} else {
				if (value == NULL) {
					NODE *array;

					getnode(array);
					array->type = Node_var_array;
					array->var_array = NULL;
					array->vname = estrdup(subs->stptr, subs->stlen);
					*assoc_lookup(r, subs, FALSE) = array;
					r = array;
				} else if (value->type != Node_var_array) {
					d_error(_("attempt to use scalar `%s[\"%s\"]' as array"),
							name, subs->stptr);
					break;
				} else {
					r = value;
					name = r->vname;
				}
			}
		}
	}
		break;

	case D_field:
	{
		long field_num;
		Func_ptr assign = NULL;

		field_num = (long) arg->a_node->numbr;
		assert(field_num >= 0);
		arg = arg->next;
		val = arg->a_node;
		lhs = get_field(field_num, &assign);
		unref(*lhs);
		*lhs = dupnode(val);
		if (assign)
			assign();
		print_field(field_num);
	}
		break;

	default:
		break;
	}
	return FALSE;
}		

/* find_item --- find an item in the watch/display list */

static struct list_item *
find_item(struct list_item *list, long num)
{
	struct list_item *d;

	if (num <= 0)
		return NULL;
	for (d = list->next; d != list; d = d->next) {
		if (d->number == num)
			return d;
	}
	return NULL;
}

/* delete_item --- delete an item from the watch/display list */

static void
delete_item(struct list_item *d)
{
	struct commands_item *c;
	int i;

	if (IS_SUBSCRIPT(d)) {
		for (i = 0; i < d->num_subs; i++)
			unref(d->subs[i]);
		efree(d->subs);
	} else if (IS_FIELD(d))
		unref(d->symbol);

	if ((d->flags & CUR_IS_ARRAY) == 0)
		unref(d->cur_value);
	if ((d->flags & OLD_IS_ARRAY) == 0)
		unref(d->old_value);

	/* delete commands */
	for (c = d->commands.next; c != &d->commands; c = c->next) {
		c = c->prev;
		delete_commands_item(c->next);
	}

	free_context(d->cndn.ctxt, FALSE);	
	if (d->cndn.expr != NULL)
		efree(d->cndn.expr);
	
	d->next->prev = d->prev;
	d->prev->next = d->next;
	efree(d);
}

/* add_item --- craete a watch/display item and add it to the list */

static struct list_item *
add_item(struct list_item *list, int type, NODE *symbol, char *pname)
{
	struct list_item *d;

	emalloc(d, struct list_item *, sizeof(struct list_item), "add_item");
	memset(d, 0, sizeof(struct list_item));
	d->commands.next = d->commands.prev = &d->commands;

	d->number = ++list->number;
	d->sname = symbol->vname;
	if (pname != NULL) {	/* function param */
		d->sname = pname;
		d->flags |= PARAM;
		d->fcall_count = fcall_count - cur_frame;
	}

	if (type == D_field) { /* field number */
		d->symbol = symbol;
		d->flags |= FIELD_NUM;
	} else if (type == D_subscript) {	/* subscript */
		d->symbol = symbol;
		d->flags |= SUBSCRIPT;
	} else /* array or variable */
		d->symbol = symbol;

	/* add to list */
	d->next = list->next;
	d->prev = list;
	list->next = d;
	d->next->prev = d;
	return d;
}

/* do_add_item --- add an item to the watch/display list */

static struct list_item *
do_add_item(struct list_item *list, CMDARG *arg)
{
	NODE *symbol = NULL;
	char *name, *pname = NULL;
	struct list_item *item = NULL;

	switch (arg->type) {
	case D_subscript:
	case D_variable:
		name = arg->a_string;
		if ((symbol = find_symbol(name, &pname)) == NULL)
			return NULL;
		if (symbol->type == Node_func) {
			d_error(_("`%s' is a function"), name);
			return NULL;
		}
		if (arg->type == D_subscript && symbol->type != Node_var_array) {
			d_error(_("`%s' is not an array\n"), name);
			return NULL;
		}

		item = add_item(list, arg->type, symbol, pname);
		if (item != NULL && arg->type == D_subscript) {
			NODE **subs;
			int count = arg->a_count;
			int i;

			assert(count > 0);
			emalloc(subs, NODE **, count * sizeof(NODE *), "do_add_item");
			for (i = 0; i < count; i++) {
				arg = arg->next;
				subs[i] = dupnode(arg->a_node);
				(void) force_string(subs[i]);
			}
			item->subs = subs;
			item->num_subs = count;
		}
		break;

	case D_field:
		symbol = dupnode(arg->a_node);
		item = add_item(list, D_field, symbol, NULL);
		break;

	default:
		break;
	}

	/* watch condition if any */
	if (list == &watch_list) {
		arg = arg->next;
		if (item != NULL && arg != NULL) {
			if (parse_condition(D_watch, item->number, arg->a_string) == 0)
				arg->a_string = NULL;	/* don't let free_cmdarg free it */
			else
				fprintf(out_fp, _("watchpoint %d is unconditional\n"), item->number);
		}
	}
	return item;
}

/* do_delete_item --- delete a watch/display item from list. */

static void
do_delete_item(struct list_item *list, CMDARG *arg)
{
	if (arg == NULL) {
		while (list->next != list)
			delete_item(list->next);
	}

	for (; arg != NULL; arg = arg->next) {
		struct list_item *d;
		if (arg->type == D_range) {
			long i, j;

			i = arg->a_int;
			arg = arg->next;
			j = arg->a_int;
			if (j > list->number)
				j = list->number;
			for (; i <= j; i++) {
				if ((d = find_item(list, i)) != NULL)
					delete_item(d);
			}
		} else {
			if ((d = find_item(list, arg->a_int)) == NULL) {
				/* split into two for easier message translation */
				if (list == &display_list)
					d_error(_("No display item numbered %ld"),
						arg->a_int);
				else
					d_error(_("No watch item numbered %ld"),
						arg->a_int);
			} else
				delete_item(d);
		}
	}
}

/* display --- print an item from the auto-display list */

static void
display(struct list_item *d)
{
	NODE *symbol;

	symbol = d->symbol;
	if (IS_PARAM(d)	&& (d->fcall_count != (fcall_count - cur_frame)))
		return;

	if (IS_SUBSCRIPT(d)) {
		NODE *sub, *r;
		int i = 0, count = d->num_subs;
		for (i = 0; i < count; i++) {
			sub = d->subs[i];
			r = in_array(symbol, sub);
			if (r == NULL) {
				fprintf(out_fp, _("%d: [\"%s\"] not in array `%s'\n"),
							d->number, sub->stptr, d->sname);
				break;
			}
			if (r->type == Node_var_array) {
				symbol = r;
				if (i == count - 1)	/* it's a sub-array */
					goto print_sym;		/* print # of elements in sub-array */
			} else {							
				if (i != count - 1)
					return;		/* FIXME msg and delete item ? */
				fprintf(out_fp, "%d: %s[\"%s\"] = ", d->number,
							d->sname, sub->stptr);
				valinfo(r, fprintf, out_fp);
			}
		}
	} else if (IS_FIELD(d)) {
		NODE *r = d->symbol;
		fprintf(out_fp, "%d: ", d->number);
		print_field(r->numbr);
	} else {
print_sym:
		fprintf(out_fp, "%d: %s = ", d->number, d->sname);
		print_symbol(symbol, IS_PARAM(d));
	}
}


/* do_display --- display command */

int
do_display(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	struct list_item *d;

	if (arg == NULL) {
		/* display all items */
		for (d = display_list.prev; d != &display_list; d = d->prev)
			display(d);
		return FALSE;
	}

	if ((d = do_add_item(&display_list, arg)) != NULL)
		display(d);

	return FALSE;
}

/* do_undisplay --- undisplay command */
	
int
do_undisplay(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	do_delete_item(&display_list, arg);
	return FALSE; 
}

/* condition_triggered --- test if a condition expression is true */ 

static int
condition_triggered(struct condition *cndn)
{
	NODE *r;
	int di;

	assert(cndn != NULL);
	if (cndn->code == NULL)
		return TRUE;

	push_context(cndn->ctxt);
	r = execute_code((volatile INSTRUCTION *) cndn->code);
	pop_context();  /* switch to prev context */
	if (r == NULL)      /* fatal error */
		return FALSE;   /* not triggered */

	force_number(r);
	di = (r->numbr != 0.0);
	DEREF(r);
	return di;
}



static int
find_subscript(struct list_item *item, NODE **ptr)
{
	NODE *symbol = item->symbol;
	NODE *sub, *r;
	int i = 0, count = item->num_subs;

	r = *ptr = NULL;
	for (i = 0; i < count; i++) {
		sub = item->subs[i];
		r = in_array(symbol, sub);
		if (r == NULL)
			return 0;
		if (r->type == Node_var_array)
			symbol = r;
		else if (i < count - 1)
			return -1;
	}
	*ptr = r;
	return 0;
}

/* cmp_val --- compare values of watched item, returns TRUE if different; */
 
static int
cmp_val(struct list_item *w, NODE *old, NODE *new) 
{
		/*
		 *	case    old     new     result
		 *	------------------------------
		 *	1:      NULL    ARRAY   TRUE	
		 *	2:      NULL    SCALAR  TRUE
		 *	3:      NULL    NULL    FALSE
		 *	4:      SCALAR  SCALAR  cmp_node
		 *	5:      SCALAR  ARRAY   TRUE
		 *	6:      SCALAR  NULL    TRUE
		 *	7:      ARRAY   SCALAR  TRUE
		 *	8:      ARRAY   ARRAY   compare size	
		 *	9:      ARRAY   NULL    TRUE
		 */

	if (WATCHING_ARRAY(w)) {
		long size = 0;
		if (! new)		/* 9 */
			return TRUE;
		if (new->type == Node_val)	/* 7 */
			return TRUE;
		/* new->type == Node_var_array */	/* 8 */
		if (new->var_array != NULL)
			size = new->table_size;
		if (w->cur_size == size)
			return FALSE;
		return TRUE;
	}

	if (! old && ! new)	/* 3 */
		return FALSE;
	if ((! old && new)	/* 1, 2 */
			|| (old && ! new))	/* 6 */
		return TRUE;

	if (new->type == Node_var_array)	/* 5 */
		return TRUE;
	return cmp_nodes(old, new);			/* 4 */
}

/* watchpoint_triggered --- check if we should stop at this watchpoint;
 *                          update old and current values accordingly.
 */

static int
watchpoint_triggered(struct list_item *w)
{
	NODE *symbol;
	NODE *t1, *t2;

	symbol = w->symbol;
	if (IS_PARAM(w) && (w->fcall_count != (fcall_count - cur_frame)))
		return 0; /* parameter with same name in a different function */
	if (! condition_triggered(&w->cndn))
		return 0;

	t1 = w->cur_value;
	t2 = (NODE *) 0;
	if (IS_SUBSCRIPT(w))
		(void) find_subscript(w, &t2);
	else if (IS_FIELD(w)) {
		long field_num;
		field_num = (long) w->symbol->numbr;
		t2 = *get_field(field_num, NULL);
	} else {
		switch (symbol->type) {
		case Node_var:
			t2 = symbol->var_value;
			break;
		case Node_var_array:
			t2 = symbol;
			break;
		case Node_var_new:
			break;
		default:
			cant_happen();
		}
	}

	if (! cmp_val(w, t1, t2))
		return 0;

	/* update old and current values */

	if ((w->flags & OLD_IS_ARRAY) == 0)
		unref(w->old_value);
	w->flags &= ~OLD_IS_ARRAY;
	if (WATCHING_ARRAY(w)) {	/* 7, 8, 9 */
		w->old_size = w->cur_size;
		w->flags |= OLD_IS_ARRAY;
		if (! t2) {
			w->flags &= ~CUR_IS_ARRAY;
			w->cur_value = 0;
		} else if (t2->type == Node_val) {
			w->flags &= ~CUR_IS_ARRAY;
			w->cur_value = dupnode(t2);
		} else
			w->cur_size = (t2->var_array != NULL) ? t2->table_size : 0;
	} else if (! t1) { /* 1, 2 */
		w->old_value = 0;
		/* new != NULL */
		if (t2->type == Node_val)
			w->cur_value = dupnode(t2);
		else {
			w->flags |= CUR_IS_ARRAY;
			w->cur_size = (t2->var_array != NULL) ? t2->table_size : 0;
		}
	} else /* if (t1->type == Node_val) */ {	/* 4, 5, 6 */
		w->old_value = w->cur_value;
		if (! t2)
			w->cur_value = 0;
		else if (t2->type == Node_var_array) {
			w->flags |= CUR_IS_ARRAY;
			w->cur_size = (t2->var_array != NULL) ? t2->table_size : 0;
		} else
			w->cur_value = dupnode(t2);
	}

	return w->number;
}

/* initialize_watch_item --- initialize current value of a watched item */

static int
initialize_watch_item(struct list_item *w)
{
	NODE *t, *r;
	NODE *symbol = w->symbol;

	if (IS_SUBSCRIPT(w)) {
		if (find_subscript(w, &r) == -1) {
			d_error(_("attempt to use scalar value as array"));
			return -1;
		}

		if (r == NULL)
			w->cur_value = (NODE *) 0;
		else if (r->type == Node_var_array) { /* it's a sub-array */
			w->flags |= CUR_IS_ARRAY;
			w->cur_size = (r->var_array != NULL) ? r->table_size : 0;
		} else
			w->cur_value = dupnode(r);
	} else if (IS_FIELD(w)) {
		long field_num;
		t = w->symbol;
		field_num = (long) t->numbr;
		r = *get_field(field_num, NULL);
		w->cur_value = dupnode(r);
	} else {
		if (symbol->type == Node_var_new)
			w->cur_value = (NODE *) 0;
		else if (symbol->type == Node_var) {
			r = symbol->var_value;
			w->cur_value = dupnode(r);
		} else if (symbol->type == Node_var_array) {
			w->flags |= CUR_IS_ARRAY;
			w->cur_size = (symbol->var_array != NULL) ? symbol->table_size : 0;
		} /* else
			can't happen */
	}
	return 0;
}

/* do_watch --- watch command */

int
do_watch(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	struct list_item *w;
	NODE *symbol, *sub;
	int i;

	w = do_add_item(&watch_list, arg);
	if (w == NULL)
		return FALSE;

	if (initialize_watch_item(w) == -1) {
		delete_item(w);
		return FALSE;
	}

	fprintf(out_fp, "Watchpoint %d: ", w->number);
	symbol = w->symbol;

/* FIXME: common code also in print_watch_item */
	if (IS_SUBSCRIPT(w)) {
		fprintf(out_fp, "%s", w->sname);
		for (i = 0; i < w->num_subs; i++) {
			sub = w->subs[i];
			fprintf(out_fp, "[\"%s\"]", sub->stptr);
		}
		fprintf(out_fp, "\n");
	} else if (IS_FIELD(w))
		fprintf(out_fp, "$%ld\n", (long) symbol->numbr);
	else
		fprintf(out_fp, "%s\n", w->sname);

	return FALSE;
}

/* do_unwatch --- unwatch command */
	
int
do_unwatch(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	do_delete_item(&watch_list, arg);
	return FALSE;
}

/* callback from pop_frame in eval.c */

void
frame_popped()
{
	struct list_item *item;

	/* delete all out of scope watchpoints */
	for (item = watch_list.next; item != &watch_list; item = item->next) {
		if (IS_PARAM(item) && (item->fcall_count > fcall_count)) {
			fprintf(out_fp,
				_("Watchpoint %d deleted because parameter is out of scope.\n"),
				item->number);
			item = item->prev;
			delete_item(item->next);
		}
	}

	/* delete all out of scope display items */
	for (item = display_list.next; item != &display_list; item = item->next) {
		if (IS_PARAM(item) && (item->fcall_count > fcall_count)) {
			fprintf(out_fp,
				_("Display %d deleted because parameter is out of scope.\n"),
				item->number);
			item = item->prev;
			delete_item(item->next);
		}
	}
}

/* print_function --- print function name, parameters, and optionally
 *                file and line number.
 */

static int
print_function(INSTRUCTION *pc, void *x)
{
	NODE *func;
	int i, pcount;
	char **pnames;
	struct pf_data *data = (struct pf_data *) x;  
	int defn = data->defn;
	Func_print print_func = data->print_func;	
	FILE *fp = data->fp;

	func = pc->func_body;
	pcount = get_param_count(func);
	pnames = get_params(func);

	print_func(fp, "%s(", func->lnode->param);
	for (i = 0; i < pcount; i++) {
		print_func(fp, "%s", pnames[i]);
		if (i < pcount - 1)               
			print_func(fp, ", ");
	}
	print_func(fp, ")");
	if (defn)
		print_func(fp, _(" in file `%s', line %d\n"),
				pc->source_file, pc->source_line);
	return 0;
}

/* print_frame --- print function name, parameters,
 *                 source and line number of where it is
 *                 executing.
 */

static void
print_frame(NODE *func, char *src, int srcline)
{
	if (func == NULL)
		fprintf(out_fp, "main()");
	else {
		pf_data.print_func = fprintf;
		pf_data.fp = out_fp;
		pf_data.defn = FALSE;
		(void) print_function(func->code_ptr, &pf_data);
	}
	fprintf(out_fp, _(" at `%s':%d"), src, srcline);
}

/* print_numbered_frame --- print a frame given its number */

static void
print_numbered_frame(long num)
{
	NODE *f;

	assert(prog_running == TRUE);
	f = find_frame(num);
	if (num == 0) {
		fprintf(out_fp, "#%ld\t ", num);
		print_frame(f->func_node, source, sourceline);
	} else {
		fprintf(out_fp, _("#%ld\tin "), num);
		print_frame(f->func_node, f->vname,
			((INSTRUCTION *) find_frame(num - 1)->reti)->source_line); 
	}
	fprintf(out_fp, "\n");
}

/* do_backtrace --- backtrace command */

int
do_backtrace(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	long cur = 0;
	long last = fcall_count;

	CHECK_PROG_RUNNING();
	if (arg != NULL && arg->type == D_int) {
		long count = arg->a_int;

		/* frame_ptr (frame #0),  fcall_list[1, 2, ... fcall_count] => total count */
		if (count >= 0) {
			/* toward outermost frame #fcall_count */
			last = count - 1;
			if (last > fcall_count)
				 last = fcall_count;
		} else {
			/* toward innermost frame #0 */
			cur = 1 + fcall_count + count;
			if (cur < 0)
				cur = 0;
		}
	} 

	for (; cur <= last; cur++) {
		print_numbered_frame(cur);
	}
	if (cur <= fcall_count)
		fprintf(out_fp, _("More stack frames follow ...\n"));
	return FALSE;
}  	

/* print_cur_frame_and_sourceline --- print current frame, and
 *                                    current source line.
 */
 
static void
print_cur_frame_and_sourceline()
{
	NODE *f;
	int srcline;
	char *src;

	assert(prog_running == TRUE);
	f = find_frame(cur_frame);
	if (cur_frame == 0) {
		src = source;
		srcline = sourceline;
	} else {
		f = find_frame(cur_frame);
		src = f->vname;
		srcline = ((INSTRUCTION *) find_frame(cur_frame - 1)->reti)->source_line;
	}

	fprintf(out_fp, (cur_frame > 0 ? _("#%ld\tin ") : "#%ld\t "), cur_frame);
	print_frame(f->func_node, src, srcline);
	fprintf(out_fp, "\n");
	print_lines(src, srcline, 1);
	last_printed_line = srcline - list_size / 2;
	if (last_printed_line < 0)
		last_printed_line = 0;
}

/* do_frame --- frame command */

int
do_frame(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	CHECK_PROG_RUNNING();
	if (arg && arg->type == D_int) {
		if (arg->a_int < 0 || arg->a_int > fcall_count) {
			d_error(_("invalid frame number"));
			return FALSE;
		}
		cur_frame = arg->a_int;
	}
	print_cur_frame_and_sourceline();
	return FALSE;
}

/* do_up --- up command */

int
do_up(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	CHECK_PROG_RUNNING();
	if (arg != NULL && arg->type == D_int)
		cur_frame += arg->a_int;
	else
		cur_frame++;
	if (cur_frame < 0)
		cur_frame = 0;
	else if (cur_frame > fcall_count)
		cur_frame = fcall_count;
	print_cur_frame_and_sourceline();
	return FALSE;
}

/* do_down --- down command */

int
do_down(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	CHECK_PROG_RUNNING();
	if (arg != NULL && arg->type == D_int)
		cur_frame -= arg->a_int;
	else
		cur_frame--;
	if (cur_frame < 0)
		cur_frame = 0;
	else if (cur_frame > fcall_count)
		cur_frame = fcall_count;
	print_cur_frame_and_sourceline();
	return FALSE;
}

/* find_rule --- find a rule or function in file 'src' containing
 *               source line 'lineno'
 */

static INSTRUCTION *
find_rule(char *src, long lineno)
{
	INSTRUCTION *rp;

	assert(lineno > 0);
	for (rp = rule_list->nexti; rp != NULL; rp = rp->nexti) {
		if ((rp - 1)->source_file == src
				&& lineno >= (rp + 1)->first_line
				&& lineno <= (rp + 1)->last_line)
			return (rp - 1);
	}
	return NULL;
}

/* mk_breakpoint --- create a breakpoint instruction and the corresponding
 *                   breakpoint structure.
 */
 
static INSTRUCTION *
mk_breakpoint(char *src, int srcline)
{
	INSTRUCTION *bp;
	BREAKPOINT *b;

	bp = bcalloc(Op_breakpoint, 1, srcline);
	emalloc(b, BREAKPOINT *, sizeof(BREAKPOINT), "mk_breakpoint");
	memset(&b->cndn, 0, sizeof(struct condition));
	b->commands.next = b->commands.prev = &b->commands;
	b->silent = FALSE;


	b->number = ++watch_list.number;	/* breakpoints and watchpoints use same counter */
	b->ignore_count = 0;
	b->hit_count = 0;
	b->flags = BP_ENABLE;
	b->src = src;
	bp->break_pt = b;
	b->bpi = bp;
			
	/* prepend to list */
	b->next = breakpoints.next;
	b->prev = &breakpoints;
	breakpoints.next = b;
	b->next->prev = b;
	return bp;
}

/* delete_breakpoint --- delete a breakpoint structure and
 *                       disable the breakpoint instruction.
 */

static void
delete_breakpoint(BREAKPOINT *b)
{
	INSTRUCTION *pc = b->bpi;
	struct commands_item *c;

	/* N.B.: easiest thing to do is to turn Op_breakpoint into a no-op;
	 * deleteing the instruction is not that simple,
	 * since could have reference to it somewhere else (e.g. cur_pc).
	 */
  
	pc->opcode = Op_no_op;
	pc->source_line = 0;
	pc->break_pt = NULL;

	/* delete commands */
	for (c = b->commands.next; c != &b->commands; c = c->next) {
		c = c->prev;
		delete_commands_item(c->next);
	}

	free_context(b->cndn.ctxt, FALSE);	
	if (b->cndn.expr != NULL)
		efree(b->cndn.expr);

	/* remove from list */
	b->next->prev = b->prev;
	b->prev->next = b->next;
	efree(b);
}

/* find_breakpoint --- find the breakpoint structure from a breakpoint number */

static BREAKPOINT *
find_breakpoint(long num)
{
	BREAKPOINT *b;

	if (num <= 0)
		return NULL; 

	for (b = breakpoints.next; b != &breakpoints; b = b->next) {
		if (b->number == num)
			return b;
	}
	return NULL;
}

/* add_breakpoint --- add a breakpoint instruction between PREVP and IP */
 
static BREAKPOINT *
add_breakpoint(INSTRUCTION *prevp, INSTRUCTION *ip, char *src, int silent)
{
	BREAKPOINT *b;
	INSTRUCTION *bp;
	int lineno = ip->source_line;

	/* add new breakpoint instruction at the end of
	 * already set breakpoints at this line number.
	 */

	while (ip->opcode == Op_breakpoint && ip->source_line == lineno) {
		if (! silent) {
			b = ip->break_pt;
			/*
			 * This is more verbose that it might otherwise be,
			 * in order to provide easily translatable strings.
			 */
			if (b->flags & BP_ENABLE) {
				if (b->flags & BP_IGNORE)
					fprintf(out_fp,
			_("Note: breakpoint %d (enabled, ignore next %ld hits), also set at %s:%d"),
						b->number,
						b->ignore_count,
						b->src,
						lineno);
				else
					fprintf(out_fp,
			_("Note: breakpoint %d (enabled), also set at %s:%d"),
						b->number,
						b->src,
						lineno);
			} else {
				if (b->flags & BP_IGNORE)
					fprintf(out_fp,
			_("Note: breakpoint %d (disabled, ignore next %ld hits), also set at %s:%d"),
						b->number,
						b->ignore_count,
						b->src,
						lineno);
				else
					fprintf(out_fp,
			_("Note: breakpoint %d (disabled), also set at %s:%d"),
						b->number,
						b->src,
						lineno);
			}
		}
		prevp = ip;
		ip = ip->nexti;
	}

	assert(ip->source_line == lineno);

	bp = mk_breakpoint(src, lineno);
	prevp->nexti = bp;
	bp->nexti = ip;
	b = bp->break_pt;
	if (! silent)
		fprintf(out_fp, _("Breakpoint %d set at file `%s', line %d\n"),
						b->number, src, lineno);
	return b;
}

/* set_breakpoint_at --- set a breakpoint at given line number*/

static BREAKPOINT *
set_breakpoint_at(INSTRUCTION *rp, int lineno, int silent)
{
	INSTRUCTION *ip, *prevp;

	for (prevp = rp, ip = rp->nexti; ip; prevp = ip, ip = ip->nexti) {
		if (ip->source_line >= lineno)
			return add_breakpoint(prevp, ip, rp->source_file, silent);
		if (ip == (rp + 1)->lasti)
			break;
	}
	return NULL;
}

/* set_breakpoint_next --- set a breakpoint at the next instruction */

static BREAKPOINT *
set_breakpoint_next(INSTRUCTION *rp, INSTRUCTION *ip)
{
	INSTRUCTION *prevp;

	if (ip == (rp + 1)->lasti)
		return NULL;
	prevp = ip;
	if (ip->opcode != Op_breakpoint)
		ip = ip->nexti;
	for (; ip; prevp = ip, ip = ip->nexti) {
		if (ip->source_line > 0)
			return add_breakpoint(prevp, ip, rp->source_file, FALSE);
		if (ip == (rp + 1)->lasti)
			break;
	}
	return NULL;
}
		
/* set_breakpoint --- set a breakpoint */

static int
set_breakpoint(CMDARG *arg, int temporary)
{
	int lineno;
	BREAKPOINT *b = NULL;
	INSTRUCTION *rp, *ip;
	NODE *func;
	SRCFILE *s = cur_srcfile;
	char *src = cur_srcfile->src;

	if (arg == NULL) {
/*
* (From GDB Documentation):
* 
* When called without any arguments, break sets a breakpoint at the next instruction
* to be executed in the selected stack frame (see section Examining the Stack).
* In any selected frame but the innermost, this makes your program stop as soon
* as control returns to that frame. This is similar to the effect of a finish command
* in the frame inside the selected frame--except that finish does not leave an
* active breakpoint. If you use break without an argument in the innermost frame,
* GDB stops the next time it reaches the current location; this may be useful
* inside loops.
* GDB normally ignores breakpoints when it resumes execution, until at least
* one instruction has been executed. If it did not do this,
* you would be unable to proceed past a breakpoint without first disabling the
* breakpoint. This rule applies whether or not the breakpoint already existed
* when your program stopped. 
*/
		CHECK_PROG_RUNNING();
		if (cur_frame == 0) {
			src = source;
			ip = cur_pc;
		} else {
			NODE *f;
			f = find_frame(cur_frame);
			src = f->vname;
			ip = (INSTRUCTION *) find_frame(cur_frame - 1)->reti;  /* Op_func_call */
		}
		rp = find_rule(src, ip->source_line);
		assert(rp != NULL);
		if ((b = set_breakpoint_next(rp, ip)) == NULL)
			fprintf(out_fp, _("Can't set breakpoint in file `%s'\n"), src);
		else {
			if (cur_frame == 0) {	/* stop next time */
				b->flags |= BP_IGNORE;
				b->ignore_count = 1;
			}
			if (temporary)
				b->flags |= BP_TEMP;
		}
		return FALSE;
	}

	/* arg != NULL */

	switch (arg->type) {
	case D_string:	/* break filename:lineno|function */
		s = source_find(arg->a_string);
		arg = arg->next;
		if (s == NULL || arg == NULL
				|| (arg->type != D_int && arg->type != D_func))
			return FALSE;
		src = s->src;
		if (arg->type == D_func) /* break filename:function */
			goto func;
		else
			/* fall through */
	case D_int:		/* break lineno */
		lineno = (int) arg->a_int;
		if (lineno <= 0 || lineno > s->srclines)
			d_error(_("line number %d in file `%s' out of range"), lineno, src);
		else {
			rp = find_rule(src, lineno);
			if (rp == NULL)
				fprintf(out_fp, _("Can't find rule!!!\n"));
			if (rp == NULL || (b = set_breakpoint_at(rp, lineno, FALSE)) == NULL)
				fprintf(out_fp, _("Can't set breakpoint at `%s':%d\n"),
						src, lineno);
			if (b != NULL && temporary)
				b->flags |= BP_TEMP;
		}
		break;

	case D_func:	/* break function */
func:
		func = arg->a_node;
		rp = func->code_ptr;
		if ((b = set_breakpoint_at(rp, rp->source_line, FALSE)) == NULL)
			fprintf(out_fp, _("Can't set breakpoint in function `%s'\n"),
						func->lnode->param);
		else if (temporary)
			b->flags |= BP_TEMP;
		lineno = b->bpi->source_line;
		break;

	default:
		return FALSE;
	}
	/* condition if any */
	arg = arg->next;
	if (b != NULL && arg != NULL) {
		if (parse_condition(D_break, b->number, arg->a_string) == 0)
			arg->a_string = NULL;	/* don't let free_cmdarg free it */
		else
			fprintf(out_fp, _("breakpoint %d set at file `%s', line %d is unconditional\n"),
							b->number, src, lineno);
	}
	return FALSE;
}


/* breakpoint_triggered --- check if we should stop at this breakpoint */

static int
breakpoint_triggered(BREAKPOINT *b)
{
	if ((b->flags & BP_ENABLE) == 0)
		return 0;
	if ((b->flags & BP_IGNORE) != 0) {
		if (--b->ignore_count <= 0)
			b->flags &= ~BP_IGNORE;
		return 0;
	}

	if (! condition_triggered(&b->cndn))
		return 0;

	b->hit_count++;
	if (b->flags & BP_ENABLE_ONCE) {
		b->flags &= ~BP_ENABLE_ONCE;
		b->flags &= ~BP_ENABLE;
	} 
	return b->number;
}

/* do_breakpoint --- break command */

int
do_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	return set_breakpoint(arg, FALSE);
}	

/* do_tmp_breakpoint --- tbreak command */

int
do_tmp_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	return set_breakpoint(arg, TRUE);
}

/* do_clear --- clear command */

int
do_clear(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	int lineno;
	BREAKPOINT *b;
	INSTRUCTION *rp, *ip;
	NODE *func;
	SRCFILE *s = cur_srcfile;
	char *src = cur_srcfile->src;
	int bp_found = FALSE;

	if (arg == NULL) {	/* clear */
		CHECK_PROG_RUNNING();
		if (cur_frame == 0) {
			lineno = sourceline;
			src = source;
		} else {
			NODE *f;
			f = find_frame(cur_frame);
			src = f->vname;
			lineno = ((INSTRUCTION *) find_frame(cur_frame - 1)->reti)->source_line;
		}
		goto delete_bp;
	}

	switch (arg->type) {
	case D_string:	/* clear filename:lineno|function */
		s = source_find(arg->a_string);
		arg = arg->next;
		if (s == NULL || arg == NULL ||
				(arg->type != D_int && arg->type != D_func))
			return FALSE;
		src = s->src;
		if (arg->type == D_func)
			goto func;
		/* else
			fall through */
	case D_int:	/* clear lineno */
		lineno = (int) arg->a_int;
		if (lineno <= 0 || lineno > s->srclines) {
			d_error(_("line number %d in file `%s' out of range"), lineno, src);
			return FALSE;
		}
		break;

	case D_func:	/* clear function */
func:
		func = arg->a_node;
		rp = func->code_ptr;
		for (ip = rp->nexti; ip; ip = ip->nexti) {
			if (ip->source_line <= 0)
				continue;
			if (ip->opcode != Op_breakpoint)
				break;
			b = ip->break_pt;
			if (++bp_found == 1)
				fprintf(out_fp, _("Deleted breakpoint %d"), b->number);
			else
				fprintf(out_fp, ", %d", b->number);
			delete_breakpoint(b);
		}
		if (! bp_found)
			fprintf(out_fp, _("No breakpoint(s) at entry to function `%s'\n"),
					func->lnode->param);
		else
			fprintf(out_fp, "\n");
		/* fall through */
	default:
		return FALSE;
	}

delete_bp:
	rp = find_rule(src, lineno);
	if (rp != NULL) {
		for (ip = rp->nexti; ip; ip = ip->nexti) {
			if (ip->opcode == Op_breakpoint	&& ip->source_line == lineno) {
				b = ip->break_pt;
				if (++bp_found == 1)
					fprintf(out_fp, _("Deleted breakpoint %d"), b->number);
				else
					fprintf(out_fp, ", %d", b->number);
				delete_breakpoint(b);
			}
			if (ip == (rp + 1)->lasti)
				break;
		}
	}

	if (! bp_found)
		fprintf(out_fp, _("No breakpoint at file `%s', line #%d\n"),
					src, (int) lineno);
	else
		fprintf(out_fp, "\n");
	return FALSE;
}

/* enable_breakpoint --- enable a breakpoint and set its disposition */

static inline void
enable_breakpoint(BREAKPOINT *b, short disp)
{
	b->flags &= ~(BP_ENABLE_ONCE|BP_TEMP);
	b->flags |= BP_ENABLE;
	if (disp)
		b->flags |= disp;
}

/* do_enable_breakpoint --- enable command */

int
do_enable_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	BREAKPOINT *b;
	short disp = 0;

	if (arg != NULL && arg->type == D_argument) {
		if (arg->a_argument == A_DEL)	/* del */
			disp = BP_TEMP;
		else						/* once */
			disp = BP_ENABLE_ONCE;
		arg = arg->next;
	}

	if (arg == NULL) {	/* enable [once|del] */
		for (b = breakpoints.next; b != &breakpoints; b = b->next)
			enable_breakpoint(b, disp);
	}

	for (; arg != NULL; arg = arg->next) {
		if (arg->type == D_range) {
			long i, j;

			i = arg->a_int;
			arg = arg->next;
			j = arg->a_int;
			if (j > breakpoints.number)
				j = breakpoints.number;
			for (; i <= j; i++) {
				if ((b = find_breakpoint(i)) != NULL)
					enable_breakpoint(b, disp);
			}
		} else {
			assert(arg->type == D_int);
			if ((b = find_breakpoint(arg->a_int)) == NULL)
				d_error(_("invalid breakpoint number"));
			else
				enable_breakpoint(b, disp);
		}
	}
	return FALSE;
}

/* do_delete_breakpoint --- delete command */

int
do_delete_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	if (arg == NULL) {
		int delete_all = TRUE;
		delete_all = prompt_yes_no(
					_("Delete all breakpoints? (y or n) "),
					_("y")[0], TRUE, out_fp);
		
		if (delete_all) {
			while (breakpoints.next != &breakpoints)
				delete_breakpoint(breakpoints.next);
		}
	}

	for (; arg != NULL; arg = arg->next) {
		BREAKPOINT *b;
		if (arg->type == D_range) {
			long i, j;

			i = arg->a_int;
			arg = arg->next;
			j = arg->a_int;
			if (j > breakpoints.number)
				j = breakpoints.number;
			for (; i <= j; i++) {
				if ((b = find_breakpoint(i)) != NULL)
					delete_breakpoint(b);
			}
		} else {
			if ((b = find_breakpoint(arg->a_int)) == NULL)
				d_error(_("invalid breakpoint number"));
			else
				delete_breakpoint(b);
		}
	}
	return FALSE;
}

/* do_ignore_breakpoint --- ignore command */

int
do_ignore_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	BREAKPOINT *b;

	if (arg == NULL || arg->type != D_int
			|| arg->next == NULL || arg->next->type != D_int)
		return FALSE;

	if ((b = find_breakpoint(arg->a_int)) == NULL)
		d_error(_("invalid breakpoint number"));
	else {
		b->ignore_count = arg->next->a_int;
		if (b->ignore_count > 0) {
			b->flags |= BP_IGNORE;
			fprintf(out_fp, _("Will ignore next %ld crossing(s) of breakpoint %d.\n"),
					b->ignore_count, b->number);
		} else {
			b->flags &= ~BP_IGNORE;
 			fprintf(out_fp, _("Will stop next time breakpoint %d is reached.\n"),
					b->number);
		}
	}
	return FALSE;
}

/* do_disable_breakpoint --- disable command */

int
do_disable_breakpoint(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	BREAKPOINT *b;

	if (arg == NULL) {
		/* disable all */
		for (b = breakpoints.next; b != &breakpoints; b = b->next)
			b->flags &= ~BP_ENABLE;
	}

	for (; arg != NULL; arg = arg->next) {
		if (arg->type == D_range) {
			long i, j;

			i = arg->a_int;
			arg = arg->next;
			j = arg->a_int;
			if (j > breakpoints.number)
				j = breakpoints.number;
			for (; i <= j; i++)
				if ((b = find_breakpoint(i)) != NULL)
					b->flags &= ~BP_ENABLE;
		} else {
			if ((b = find_breakpoint(arg->a_int)) == NULL)
				d_error(_("invalid breakpoint number"));
			else
				b->flags &= ~BP_ENABLE;
		}
	}
	return FALSE;
}

#ifdef HAVE_LIBREADLINE

/* get_parmlist --- list of function params in current context */

char **
get_parmlist()
{
	NODE *func;

	if (! prog_running)
		return NULL;
	func = find_frame(cur_frame)->func_node;
	if (func == NULL)	/* in main */
		return NULL;
	return func->parmlist;
}

/* initialize_readline --- initialize readline */

static void
initialize_readline()
{
	/* tell readline which stream to use for output,
	 * default input stream is stdin.
	 */
	rl_outstream = out_fp;

	/* allow conditional parsing of the ~/.inputrc file. */
	rl_readline_name = "gawk";

	/* our completion function. */
	rl_attempted_completion_function = command_completion;

	read_a_line = readline;  
}
#else
#define initialize_readline()	/* nothing */
#endif


/* interpret --- debugger entry point */

int
interpret(INSTRUCTION *pc)
{
	char *run;

	input_fd = fileno(stdin);
	out_fp = stdout;
	if (os_isatty(input_fd))
		input_from_tty = TRUE;
	if (input_fd == 0 && input_from_tty)
		initialize_readline();

	if (! read_a_line)
		read_a_line = g_readline;

	push_cmd_src(input_fd, input_from_tty, read_a_line, 0, 0, EXIT_FATAL); 

	setbuf(out_fp, (char *) NULL);                                                           
	for (cur_srcfile = srcfiles->prev; cur_srcfile != srcfiles;
			cur_srcfile = cur_srcfile->prev) {
		if (cur_srcfile->stype == SRC_FILE
			|| cur_srcfile->stype == SRC_INC)
			break;
	}

	if (cur_srcfile == srcfiles) {
		fprintf(out_fp, _("Can only debug programs provided with the `-f' option.\n"));
		exit(EXIT_FAILURE);
	}

	dgawk_Prompt = estrdup(DEFAULT_PROMPT, strlen(DEFAULT_PROMPT));
	dPrompt = dgawk_Prompt;

	memset(&stop, 0, sizeof(stop));
	stop.command = D_illegal;

	if ((run = getenv("DGAWK_RESTART")) != NULL) {
		/* We are restarting; restore state (breakpoints, history etc.)
		 * passed as environment variables and optionally execute the run command.
		 */
		unserialize(BREAK);
		unserialize(WATCH);
		unserialize(DISPLAY);
		unserialize(HISTORY);
		unserialize(OPTION);
		unsetenv("DGAWK_RESTART");
		fprintf(out_fp, "Restarting ...\n");	
		if (run[0] == 'T') 
			(void) do_run(NULL, 0);

	} else if (command_file != NULL) {
		/* run commands from a file (--command=file  or -R file) */
		int fd;
		fd = open_readfd(command_file);
		if (fd == INVALID_HANDLE) {
			fprintf(stderr, _("can't open source file `%s' for reading (%s)"),
						command_file, strerror(errno));
			exit(EXIT_FAILURE);
		}
		push_cmd_src(fd, FALSE, g_readline, close, 0, EXIT_FAILURE);
		cmd_src->str = estrdup(command_file, strlen(command_file));

	} else {
		int fd;

#ifdef HAVE_LIBREADLINE
		(void) read_history(history_file);
		sess_history_base = history_length;
#endif

		/* read saved options */
		fd = open_readfd(options_file);
		if (fd > INVALID_HANDLE)
			push_cmd_src(fd, FALSE, g_readline, close, 0, EXIT_SUCCESS);
	}

	/* start the command interpreter */
	read_command();	/* yyparse */
	return EXIT_SUCCESS;
}


/* N.B.: ignore breakpoints and watchpoints for return command */

/* check_watchpoint --- check if any watchpoint triggered */

static int
check_watchpoint()
{
	struct list_item *w;

	if (stop.command == D_return)
		return FALSE;
	for (w = watch_list.prev; w != &watch_list; w = w->prev) {
		int wnum = watchpoint_triggered(w);
		if (wnum > 0) {
			stop.watch_point = wnum;
			stop.print_frame = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

/* check_breakpoint --- check if breakpoint triggered */

static int
check_breakpoint(INSTRUCTION **pi)
{
	INSTRUCTION *pc;

	pc = *pi;
	if (stop.command == D_return)
		return FALSE; 
	if (pc->opcode == Op_breakpoint) {
		int bnum;
		*pi = pc->nexti;    /* skip past the breakpoint instruction;
		                     * interpreter doesn't process Op_breakpoint.
		                     */
		bnum = breakpoint_triggered(pc->break_pt);
		if (bnum > 0) {
			stop.break_point = bnum;
			stop.print_frame = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

/* restart --- restart the debugger */

static void
restart(int run)
{
	/* save state in the environment after serialization */
	serialize(BREAK);
	serialize(WATCH);
	serialize(DISPLAY);
	serialize(HISTORY);
	serialize(OPTION);

	/* tell the new process to restore state from the environment */
	setenv("DGAWK_RESTART", (run ? "TRUE" : "FALSE"), 1);

	/* close all open files */
	close_all();

	/* start a new process replacing the current process */
	execvp(d_argv[0], d_argv);

	/* execvp failed !!! */
	fprintf(out_fp, _("Failed to restart debugger"));
	exit(EXIT_FAILURE);
}

/* do_run --- run command */

int
do_run(CMDARG *arg ATTRIBUTE_UNUSED, int cmd ATTRIBUTE_UNUSED)
{
	if (prog_running) {
		if (! input_from_tty)
			need_restart = TRUE;	/* handled later */
		else {
			need_restart = prompt_yes_no(
			         _("Program already running. Restart from beginning (y/n)? "),
			         _("y")[0], FALSE, out_fp);

			if (! need_restart) {
				fprintf(out_fp, _("Program not restarted\n"));
				return FALSE;
			}
		}
	}

	if (need_restart) {
		/* avoid endless cycles of restarting */
		if (command_file != NULL) {
			/* input_from_tty = FALSE */
			fprintf(stderr, _("error: cannot restart, operation not allowed\n"));
			exit(EXIT_FAILURE);
		}

		if (cmd_src->cmd == D_source) {
			/* input_from_tty = FALSE */
			fprintf(out_fp, _("error (%s): cannot restart, ignoring rest of the commands\n"), cmd_src->str);
			pop_cmd_src();
			return FALSE;
		}

		restart(TRUE);	/* does not return */
	}

	fprintf(out_fp, _("Starting program: \n"));

	prog_running = TRUE;
	fatal_tag_valid = TRUE;
	if (setjmp(fatal_tag) == 0)
		(void) r_interpret(code_block);

	fatal_tag_valid = FALSE;
	prog_running = FALSE;
	fprintf(out_fp, _("Program exited %s with exit value: %d\n"),
					(! exiting && exit_val != EXIT_SUCCESS) ? "abnormally"
					                                        : "normally",
					exit_val);
	need_restart = TRUE;
	return FALSE;
}

/* do_quit --- quit command */

int
do_quit(CMDARG *arg ATTRIBUTE_UNUSED, int cmd ATTRIBUTE_UNUSED)
{
	int terminate = TRUE;
	if (prog_running)
		terminate = prompt_yes_no(
		            _("The program is running. Exit anyway (y/n)? "),
		            _("y")[0], TRUE, out_fp);
	if (terminate) {
		close_all();
		do_trace = FALSE; 	/* don't save 'trace on' */

#ifdef HAVE_LIBREADLINE
		if (do_save_history && input_from_tty) {
			int ret;
			ret = write_history(history_file);
			if (ret == 0 && history_length > history_size)
				(void) history_truncate_file(history_file, history_size);
		}
#endif
		if (do_save_options && input_from_tty)
			save_options(options_file);

		exit(exit_val);
	}
	return FALSE;
}

/* do_continue --- continue command */

int
do_continue(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	BREAKPOINT *b;

	CHECK_PROG_RUNNING();
	if (! arg || arg->type != D_int)
		return TRUE;

	/* arg is breakpoint ignore count if stopped at a breakpoint */
	if (! stop.break_point) {
		fprintf(out_fp, _("Not stopped at any breakpoint; argument ignored.\n"));
		return TRUE;
	}
	b = find_breakpoint(stop.break_point);
	if (b == NULL) {
		d_error(_("invalid breakpoint number %d."), stop.break_point);
		return FALSE;
	}
	b->flags |= BP_IGNORE;
	b->ignore_count = arg->a_int;
	fprintf(out_fp, _("Will ignore next %ld crossings of breakpoint %d.\n"),
				b->ignore_count, stop.break_point);
	return TRUE;
}

/* next_step --- common code for next and step commands */

static int
next_step(CMDARG *arg, int cmd)
{
	CHECK_PROG_RUNNING();
	if (arg != NULL && arg->type == D_int)
		stop.repeat_count = arg->a_int;
	else
		stop.repeat_count = 1;
	stop.command = cmd;
	return TRUE;
}

/* check_step --- process step command, return TRUE if stopping */

static int
check_step(INSTRUCTION **pi)
{
	if (fcall_count != stop.fcall_count) {
		stop.fcall_count = fcall_count;
		stop.sourceline = sourceline;
		stop.source = source;
		stop.print_frame = TRUE;
		return (--stop.repeat_count == 0);
	}

	if (source != stop.source) {
		stop.source = source;
		stop.sourceline = sourceline;
		return (--stop.repeat_count == 0);
	}

	if (sourceline != stop.sourceline) {
		stop.sourceline = sourceline;
		return (--stop.repeat_count == 0);
	}
	return FALSE;
}

/* do_step -- process step command, return TRUE if stopping */

int
do_step(CMDARG *arg, int cmd)
{
	int ret;
	ret = next_step(arg, cmd);
	if (ret) {
		stop.fcall_count = fcall_count;
		stop.source = source;
		stop.sourceline = sourceline;
		stop.check_func = check_step;
	}
	return ret;
}

/* do_stepi -- process stepi command, return TRUE if stopping */

static int
check_stepi(INSTRUCTION **pi)
{
	return (--stop.repeat_count == 0);
}

/* do_stepi -- stepi command */

int
do_stepi(CMDARG *arg, int cmd)
{
	int ret;
	ret = next_step(arg, cmd);
	if (ret)
		stop.check_func = check_stepi;
	return ret;
}


/* check_next -- process next command returning TRUE if stopping */

static int
check_next(INSTRUCTION **pi)
{
	/* make sure not to step inside function calls */

	if (fcall_count < stop.fcall_count) {
		stop.fcall_count = fcall_count;
		stop.sourceline = sourceline;
		stop.source = source;
		stop.print_frame = TRUE;
		return (--stop.repeat_count == 0);
	}

	if (fcall_count == stop.fcall_count) {
		if (source != stop.source) {
			stop.source = source;
			stop.sourceline = sourceline;
			return (--stop.repeat_count == 0);
		}
		if (sourceline != stop.sourceline) {
			stop.sourceline = sourceline;
			return (--stop.repeat_count == 0);
		}
	}

#if 0
	/* redundant ? */
	if (fcall_count > stop.fcall_count) {
		stop.source = source;
		stop.sourceline = sourceline;
	}
#endif

	return FALSE;
}

/* do_next -- next command */

int
do_next(CMDARG *arg, int cmd)
{
	int ret;

	ret = next_step(arg, cmd);
	if (ret) {
		stop.source = source;
		stop.sourceline = sourceline;
		stop.fcall_count = fcall_count;
		stop.check_func = check_next;
	}
	return ret;
}

/* check_nexti --- process nexti command, returns TRUE if stopping */

static int
check_nexti(INSTRUCTION **pi)
{

	/* make sure not to step inside function calls */

	if (fcall_count < stop.fcall_count) {
		stop.print_frame = TRUE;
		stop.fcall_count = fcall_count;
	}
	return (fcall_count == stop.fcall_count
			&& --stop.repeat_count == 0);
}

/* do_nexti -- nexti command */

int
do_nexti(CMDARG *arg, int cmd)
{
	int ret;

	ret = next_step(arg, cmd);
	if (ret) {
		stop.fcall_count = fcall_count;
		stop.check_func = check_nexti;
	}
	return ret;
}

/* check_finish --- process finish command, returns TRUE if stopping */

static int
check_finish(INSTRUCTION **pi)
{
	if (fcall_count == stop.fcall_count) {
		stop.print_frame = TRUE;
		return TRUE;
	}
	return FALSE;
}

/* do_finish --- finish command */

int
do_finish(CMDARG *arg ATTRIBUTE_UNUSED, int cmd)
{
	CHECK_PROG_RUNNING();
	if (cur_frame == fcall_count) {
		fprintf(out_fp,
			_("'finish' not meaningful in the outermost frame main()\n"));
		return FALSE;
	}
	stop.fcall_count = fcall_count - cur_frame - 1;
	assert(stop.fcall_count >= 0);
	fprintf(out_fp, _("Run till return from "));
	print_numbered_frame(cur_frame);
	stop.check_func = check_finish;
	stop.command = cmd;
	stop.print_ret = TRUE;
	return TRUE;
}

/* check_return --- process return, returns TRUE if stopping */

static int
check_return(INSTRUCTION **pi)
{
	assert(fcall_count >= stop.fcall_count);

	if (fcall_count == stop.fcall_count) {
		stop.print_frame = TRUE;
		return TRUE;
	}

	if (fcall_count > stop.fcall_count) {	/* innermost frame just returned */
		/* force this one to return too */
		NODE *func;

		func = find_frame(cur_frame)->func_node;
		assert(func != NULL);
		*pi = (func->code_ptr + 1)->lasti;		
		/* assert((*pi)->opcode == Op_K_return); */
	}

	return FALSE;
}

/* do_return --- return command */

int
do_return(CMDARG *arg, int cmd)
{
	NODE *func;

	CHECK_PROG_RUNNING();
	func = find_frame(cur_frame)->func_node;
	if (func == NULL) {
		fprintf(out_fp, _("'return' not meaningful in the outermost frame main()\n"));
		return FALSE;
	}

	stop.fcall_count = fcall_count - cur_frame - 1;
	assert(stop.fcall_count >= 0);
	stop.pc = (func->code_ptr + 1)->lasti;
	assert(stop.pc->opcode == Op_K_return);
	stop.command = cmd;

	stop.check_func = check_return;

	if (arg != NULL && arg->type == D_node) {	/* optional return value */
		NODE *n;
		n = dupnode(arg->a_node);
		PUSH(n);
	} else
		PUSH(Nnull_string);

	return TRUE;
}

/* check_until --- process until, returns TRUE if stopping */

int
check_until(INSTRUCTION **pi)
{
	if (fcall_count < stop.fcall_count) { /* current stack frame returned */
		stop.print_frame = TRUE;
		return TRUE;
	} else if (fcall_count == stop.fcall_count) {
		if (stop.pc && *pi == stop.pc)		/* until location */
			return TRUE;
		if (stop.sourceline > 0		/* until */
				&& source == stop.source
				&& sourceline > stop.sourceline)
			return TRUE;
	}
	return FALSE;
}

/* do_until --- until command */

int
do_until(CMDARG *arg, int cmd)
{
	SRCFILE *s = cur_srcfile;
	char *src = cur_srcfile->src;
	int lineno;
	INSTRUCTION *rp, *ip;
	NODE *func;
	
	CHECK_PROG_RUNNING();
	stop.pc = NULL;
	stop.sourceline = 0;

	if (arg == NULL) {	/* until without argument */

    /* GDB doc.: continue running until a source line past the current line,
     * in the current stack frame, is reached. Is used to avoid single
     * stepping through a loop more than once. ...
     * This means that when you reach the end of a loop after single
     * stepping though it, until makes your program continue execution
     * until it exits the loop. In contrast, a next command at the end
     * of a loop simply steps back to the beginning of the loop, which
     * forces you to step through the next iteration.
     */

		stop.source = source;
		stop.sourceline = sourceline;
		stop.fcall_count = fcall_count - cur_frame;
		stop.check_func = check_until;
		stop.command = cmd;
		return TRUE;
	}

    /* GDB: until location - continue running program until
     * either the specified location is reached, or the
     * current stack frame returns.
     */

	switch (arg->type) {
	case D_string:	/* until filename : lineno|function */
		s = source_find(arg->a_string);
		arg = arg->next;
		if (s == NULL || arg == NULL
			|| (arg->type != D_int && arg->type != D_func))
			return FALSE;
		src = s->src;
		if (arg->type == D_func)
			goto func;
		/* else
			fall through */
	case D_int:	/* until lineno */
		lineno = arg->a_int;
		if (lineno <= 0 || lineno > s->srclines) {
			d_error(_("line number %d in file `%s' out of range"),
						lineno, src);
			return FALSE;
		}
		break;

	case D_func:	/* until function */
func:
		func = arg->a_node;
		rp = func->code_ptr;
		for (ip = rp->nexti; ip; ip = ip->nexti) {
			if (ip->opcode != Op_breakpoint && ip->source_line > 0) {
				stop.pc = ip;
				stop.fcall_count = fcall_count - cur_frame;
				stop.check_func = check_until;
				stop.command = cmd;
				return TRUE;
			}
		}
		fprintf(out_fp, _("Can't find specified location in function `%s'\n"),
				func->lnode->param);
		/* fall through */
	default:
		return FALSE;
	}

	if ((rp = find_rule(src, lineno)) == NULL) {
		d_error(_("invalid source line %d in file `%s'"), lineno, src);
		return FALSE;
	}

	for (ip = rp->nexti; ip; ip = ip->nexti) {
		if (ip->opcode != Op_breakpoint && ip->source_line >= lineno) {
			stop.pc = ip;
			stop.fcall_count = fcall_count - cur_frame;
			stop.check_func = check_until;
			stop.command = cmd;
			return TRUE;
		}
		if (ip == (rp + 1)->lasti)
			break;
	}
	fprintf(out_fp, _("Can't find specified location %d in file `%s'\n"),
				lineno, src);
	return FALSE;
}

/* print_watch_item --- print watched item name, old and current values */

static void
print_watch_item(struct list_item *w)
{
	NODE *symbol, *sub;
	int i;

	symbol = w->symbol;
	if (IS_SUBSCRIPT(w)) {
		fprintf(out_fp, "%s", w->sname);
		for (i = 0; i < w->num_subs; i++) {
			sub = w->subs[i];
			fprintf(out_fp, "[\"%s\"]", sub->stptr);
		}
		fprintf(out_fp, "\n");  
	} else if (IS_FIELD(w))
		fprintf(out_fp, "$%ld\n", (long) symbol->numbr);
	else
		fprintf(out_fp, "%s\n", w->sname);


#define print_value(X, S, V)                                        \
if (X)                                                              \
	fprintf(out_fp, "array, %ld elements\n", w->S);                 \
else if (! w->V)                                                    \
	fprintf(out_fp, IS_SUBSCRIPT(w) ?                               \
			_("element not in array\n") : _("untyped variable\n")); \
else                                                                \
	valinfo(w->V, fprintf, out_fp);

	fprintf(out_fp, "  Old value: ");
	print_value((w->flags & OLD_IS_ARRAY), old_size, old_value);
	fprintf(out_fp, "  New value: ");
	print_value((w->flags & CUR_IS_ARRAY), cur_size, cur_value);

#undef print_value
}

/* next_command --- (optionally) print stoppage location and reason;
 *                  also fetch next debug command from the user.
 */

static void
next_command()
{
	static int last_rule = 0;
	struct list_item *d = NULL, *w = NULL;
	BREAKPOINT *b = NULL;
	SRCFILE *s;

	if (source == NULL) {
		stop.command = D_illegal;
		stop.check_func = NULL;
		return;
	}

	if (stop.break_point) {
		b = find_breakpoint(stop.break_point);
		assert(b != NULL);
		if (b->silent)
			goto no_output;
	} else if (stop.watch_point) {
		w = find_item(&watch_list, stop.watch_point);
		if (w->silent)
			goto no_output;
	}

	if (cur_rule != last_rule) {
		fprintf(out_fp, _("Stopping in %s ...\n"), ruletab[cur_rule]);
		last_rule = cur_rule;
	}

	if (b != NULL)
		fprintf(out_fp, "Breakpoint %d, ", b->number);
	else if (w != NULL) {
		fprintf(out_fp, "Watchpoint %d: ", w->number);
		print_watch_item(w);
	}

	/* frame info */	
	if (stop.print_frame) {
		print_frame(frame_ptr->func_node, source, sourceline);
		fprintf(out_fp, "\n");
		stop.print_frame = FALSE;
	} 

	(void) print_lines(source, sourceline, 1);

	/* automatic display of variables */
	for (d = display_list.prev; d != &display_list; d = d->prev)
		display(d);

no_output:
	/* update last_printed_line, so that  output of 'list' is
	 * centered around current sourceline
	 */
 
	last_printed_line = sourceline - list_size / 2;
	if (last_printed_line < 0)
		last_printed_line = 0;

	/* update current source file */	
	s = source_find(source);
	if (cur_srcfile != s) {
		if (cur_srcfile->fd != INVALID_HANDLE) {
			close(cur_srcfile->fd);
			cur_srcfile->fd = INVALID_HANDLE;
		}
		cur_srcfile = s;
	}

	stop.command = D_illegal;
	stop.check_func = NULL;

	if (b != NULL) {
		int ret;
		ret = execute_commands(&b->commands);
		if ((b->flags & BP_TEMP) != 0)
			delete_breakpoint(b);
		if (ret)	/* resume execution */
			return;
	} else if (w != NULL && execute_commands(&w->commands))
		return;

	read_command();		/* zzparse */
}

/* post_execute --- post_hook in the interpreter */ 

void
post_execute(INSTRUCTION *pc)
{
	if (! in_main_context())
		return;

	switch (pc->opcode) {
	case Op_K_next:
	case Op_K_nextfile:
	case Op_K_exit:
		if (stop.command == D_finish) {
			/* cancel finish command */
			stop.print_ret = FALSE;
			stop.print_frame = FALSE;
			stop.command = D_illegal;
			stop.check_func = NULL;
			fprintf(out_fp, _("'finish' not meaningful with non-local jump '%s'\n"),
							op2str(pc->opcode)); 
		} else if (stop.command == D_until) {
			/* cancel until command */
			stop.print_frame = FALSE;
			stop.command = D_illegal;
			stop.check_func = NULL;
			fprintf(out_fp, _("'until' not meaningful with non-local jump '%s'\n"),
							op2str(pc->opcode)); 
		}
		break;

	case Op_K_return:
		if (stop.command == D_finish
				&& fcall_count == stop.fcall_count
				&& stop.print_ret
		) {
			NODE *r;
			/* print the returned value before it disappears. */
			r = TOP();
			fprintf(out_fp, "Returned value = ");
			valinfo(r, fprintf, out_fp);
			stop.print_ret = FALSE;
		}
		break;

	case Op_newfile:
	case Op_get_record:
		return;

	default:
		break;
	}
}

/* pre_execute --- pre_hook, called by the interpreter before execution; 
 *                 checks if execution needs to be suspended and control
 *                 transferred to the debugger.
 */

int
pre_execute(INSTRUCTION **pi)
{
	static int cant_stop = FALSE;
	NODE *m;

	if (! in_main_context())
		return pre_execute_code(pi);

	cur_pc = *pi;
	stop.break_point = 0;
	stop.watch_point = 0;
	cur_frame = 0;

	if (do_trace
		&& cur_pc->opcode != Op_breakpoint
		&& stop.command != D_return
	)
		print_instruction(cur_pc, fprintf, out_fp, FALSE);

/* N.B.: For Op_field_spec_lhs must execute instructions upto Op_field_assign
 * as a group before stopping. Otherwise, watch/print of field variables
 * yield surprising results. Ditto for Op_push_lhs for special variables
 * (upto Op_var_assign, the set_FOO routine).
 */
 
	switch (cur_pc->opcode) {
	case Op_field_spec_lhs:
		cant_stop = TRUE;
		break;

	case Op_field_assign:
		cant_stop = FALSE;
		return TRUE; /* may stop at next instruction */ 

	case Op_push_lhs:
		m = cur_pc->memory;
		if (m->type == Node_var && m->var_assign)
			cant_stop = TRUE;
		break;

	case Op_arrayfor_incr:	/* can have special var as array variable !!! */
		m = cur_pc->array_var;
		if (m->type == Node_var && m->var_assign)
			cant_stop = TRUE;
		break;

	case Op_var_assign:
		cant_stop = FALSE;
		return TRUE; /* may stop at next instruction */

	case Op_rule:
		cur_rule = cur_pc->in_rule;
		return TRUE;

	case Op_func:
	case Op_ext_func:
	case Op_var_update:
		return TRUE;

	case Op_breakpoint:
		break;	/* processed later in check_breakpoint() */

	default:
		if (cur_pc->source_line <= 0) 
			return TRUE;
		break;
	}

	if (cant_stop)
		return TRUE;

	assert(sourceline > 0);

	if (check_breakpoint(pi)
			|| check_watchpoint()
			|| (stop.check_func && stop.check_func(pi))) {
		next_command();	/* return to debugger interface */
		if (stop.command == D_return)
			*pi = stop.pc;	/* jump to this instruction */
	}

	/* if cur_pc == *pi, interpreter executes cur_pc;
	 * Otherwise, jumps to instruction *pi.
	 */
	return (cur_pc == *pi);
}

/* print_memory --- print a scalar value */

static void 
print_memory(NODE *m, char **fparms, Func_print print_func, FILE *fp)
{
	switch (m->type) {
		case Node_val:
			if (m == Nnull_string)
				print_func(fp, "Nnull_string");
			else if ((m->flags & NUMBER) != 0)
				print_func(fp, "%g", m->numbr);
			else if ((m->flags & STRING) != 0)
				pp_string_fp(print_func, fp, m->stptr, m->stlen, '"', FALSE);
			else if ((m->flags & NUMCUR) != 0)
				print_func(fp, "%g", m->numbr);
			else if ((m->flags & STRCUR) != 0)
				pp_string_fp(print_func, fp, m->stptr, m->stlen, '"', FALSE);
			else
				print_func(fp, "-?-");
			print_func(fp, " [%s]", flags2str(m->flags));
			break;

		case Node_regex:
			pp_string_fp(print_func, fp, m->re_exp->stptr, m->re_exp->stlen, '/', FALSE);
			break;

		case Node_dynregex:
			break;
			
		case Node_param_list:
			assert(fparms != NULL);
			print_func(fp, "%s", fparms[m->param_cnt]);
			break;

		case Node_var:
		case Node_var_new:
		case Node_var_array:
			print_func(fp, "%s", m->vname);
			break;

		default:
			print_func(fp, "?");  /* can't happen */
	}
}

/* print_instruction --- print a bytecode */

static void
print_instruction(INSTRUCTION *pc, Func_print print_func, FILE *fp, int in_dump)
{
	static char **fparms = NULL;
	int pcount = 0;
	NODE *func = NULL;
	static int noffset = 0;

	if (noffset == 0) {
		static char buf[50];
		/* offset for 2nd to last lines in a multi-line output */
		noffset = sprintf(buf, "[      :%p] %-20.20s: ", pc, opcode2str(pc->opcode));
	}

	if (pc->opcode == Op_func) {
		func = pc->func_body;
		fparms = get_params(func);
		pcount = get_param_count(func);
		if (in_dump) {
			int j;
			print_func(fp, "\n\t# Function: %s (", func->lnode->param);
			for (j = 0; j < pcount; j++) {
				print_func(fp, "%s", fparms[j]);
				if (j < pcount - 1)
					print_func(fp, ", ");
			}
			print_func(fp, ")\n\n");
		}
	} else if (pc->opcode == Op_ext_func) {
		func = pc->func_body;
		fparms = get_params(func);
		pcount = get_param_count(func);
		if (in_dump)
			print_func(fp, "\n\t# Extension function: %s (... %d params ...)\n\n",
						func->lnode->param, pcount);
	} else if (pc->opcode == Op_rule) {
		if (in_dump)
			print_func(fp, "\n\t# %s\n\n", ruletab[pc->in_rule]);
	}

	if (pc->opcode == Op_newfile)
		print_func(fp, "\n");

	if (pc->source_line <= 0)
		print_func(fp, "[      :%p] %-20.20s: ", pc, opcode2str(pc->opcode));
	else
		print_func(fp, "[%6d:%p] %-20.20s: ",
		                pc->source_line, pc, opcode2str(pc->opcode));

	if (prog_running && ! in_dump) {
		/* find params in the current frame */
		func = find_frame(0)->func_node;
		if (func != NULL)
			fparms = get_params(func);
		/* else
			fparms = NULL; */
	}

			
	switch (pc->opcode) {
	case Op_var_update:
		print_func(fp, "[update_%s]\n", pc->memory->vname);
		break;

	case Op_var_assign:
		print_func(fp, "[set_%s]\n", pc->memory->vname);
		break;

	case Op_field_spec_lhs:
		print_func(fp, "[target_assign = %p] [do_reference = %s]\n",
				pc->target_assign, pc->do_reference ? "TRUE" : "FALSE");
		break;

	case Op_ext_func:
		print_func(fp, "[param_cnt = %d]\n", pcount);
		break; 

	case Op_func:
		print_func(fp, "[param_cnt = %d] [source_file = %s]\n", pcount,
				pc->source_file ? pc->source_file : "cmd. line");
		break;

	case Op_K_getline_redir:
		print_func(fp, "[into_var = %s] [redir_type = \"%s\"]\n",
		                pc->into_var ? "TRUE" : "FALSE",
		                redir2str(pc->redir_type));
		break;

	case Op_K_getline:
		print_func(fp, "[into_var = %s]\n", pc->into_var ? "TRUE" : "FALSE");
		print_func(fp, "%*s[target_beginfile = %p] [target_endfile = %p]\n",
		                noffset, "",
		                (pc + 1)->target_beginfile, (pc + 1)->target_endfile);
		break;

	case Op_K_print_rec:
		print_func(fp, "[redir_type = \"%s\"]\n", redir2str(pc->redir_type));
		break;

	case Op_K_print:
	case Op_K_printf:
		print_func(fp, "[expr_count = %ld] [redir_type = \"%s\"]\n",
		                pc->expr_count, redir2str(pc->redir_type));
		break;

	case Op_indirect_func_call:
	case Op_func_call:
		print_func(fp, "[func_name = %s] [arg_count = %ld]\n",
		                pc->func_name, (pc + 1)->expr_count);
		break;

	case Op_K_nextfile:
	case Op_newfile:
		print_func(fp, "[target_jmp = %p] [target_endfile = %p]\n",
		                pc->target_jmp, pc->target_endfile);
		break;

	case Op_jmp:
	case Op_jmp_false:
	case Op_jmp_true:
	case Op_and:
	case Op_or:
	case Op_K_next:
	case Op_arrayfor_init:
	case Op_K_break:
	case Op_K_continue:
		print_func(fp, "[target_jmp = %p]\n", pc->target_jmp);
		break;
	case Op_K_exit:
		print_func(fp, "[target_end = %p] [target_atexit = %p]\n",
						pc->target_end, pc->target_atexit);
		break;

	case Op_K_case:
		print_func(fp, "[target_jmp = %p] [match_exp = %s]\n",
						pc->target_jmp,	(pc + 1)->match_exp ? "TRUE" : "FALSE");
		break;

	case Op_arrayfor_incr:
		print_func(fp, "[array_var = %s] [target_jmp = %p]\n",
		                pc->array_var->type == Node_param_list ?
		                   fparms[pc->array_var->param_cnt] : pc->array_var->vname,
		                pc->target_jmp);
		break;

	case Op_line_range:
		print_func(fp, "[triggered = %ld] [target_jmp = %p]\n",
		                pc->triggered, pc->target_jmp);
		break;

	case Op_cond_pair:
		print_func(fp, "[line_range = %p] [target_jmp = %p]\n",
		                pc->line_range, pc->target_jmp);
		break;

	case Op_builtin:
	{
		const char *fname = getfname(pc->builtin);
		if (fname == NULL)
			print_func(fp, "(extension func) [arg_count = %ld]\n", pc->expr_count);
		else
			print_func(fp, "%s [arg_count = %ld]\n", fname, pc->expr_count);
	}
		break;

	case Op_subscript:
	case Op_sub_array:
		print_func(fp, "[sub_count = %ld]\n", pc->sub_count);
		break;

	case Op_store_sub:
		print_memory(pc->memory, fparms, print_func, fp);
		print_func(fp, " [sub_count = %ld]\n", pc->expr_count);
		break;

	case Op_subscript_lhs:
		print_func(fp, "[sub_count = %ld] [do_reference = %s]\n",
		                pc->sub_count,
		                pc->do_reference ? "TRUE" : "FALSE");
		break;

	case Op_K_delete:
	case Op_in_array:
		print_func(fp, "[expr_count = %ld]\n", pc->expr_count);
		break;

	case Op_concat:
		/* NB: concat_flag CSVAR only used in grammar, don't display it */ 
		print_func(fp, "[expr_count = %ld] [concat_flag = %s]\n",
						pc->expr_count,
						(pc->concat_flag & CSUBSEP) ? "CSUBSEP" : "0");
		break;

	case Op_rule:
		print_func(fp, "[in_rule = %s] [source_file = %s]\n",
		                ruletab[pc->in_rule],
		                pc->source_file ? pc->source_file : "cmd. line");
		break;

	case Op_lint:
	{
		static const char *const linttypetab[] = {
			"LINT_illegal",
			"LINT_assign_in_cond",
			"LINT_no_effect"
		};
		print_func(fp, "[lint_type = %s]\n", linttypetab[pc->lint_type]);
	}
		break;

	case Op_exec_count:
		print_func(fp, "[exec_count = %ld]\n", pc->exec_count);
		break;

	case Op_store_var:
	case Op_push_lhs:
		print_memory(pc->memory, fparms, print_func, fp);
		print_func(fp, " [do_reference = %s]\n",
		                pc->do_reference ? "TRUE" : "FALSE");
		break;
				
	case Op_push_i:
	case Op_push:
	case Op_push_arg:
	case Op_push_param:
	case Op_push_array:
	case Op_push_re:
	case Op_match_rec:
	case Op_match:
	case Op_nomatch:
	case Op_plus_i:
	case Op_minus_i:
	case Op_times_i:
	case Op_exp_i:
	case Op_quotient_i:
	case Op_mod_i:
	case Op_assign_concat:
		print_memory(pc->memory, fparms, print_func, fp);
		/* fall through */
	default:
		print_func(fp, "\n");
		break;
	}
}

/* do_trace_instruction --- trace command */

int
do_trace_instruction(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	if (arg != NULL && arg->type == D_argument
			&& arg->a_argument == A_TRACE_ON)
		do_trace = TRUE;
	else
		do_trace = FALSE;
	return FALSE;
} 

/* print_code --- print a list of instructions */

static int
print_code(INSTRUCTION *pc, void *x)
{
	struct pf_data *data = (struct pf_data *) x;
	for (; pc != NULL; pc = pc->nexti)
		print_instruction(pc, data->print_func, data->fp, data->defn /* in_dump */);
	return 0;
}

/* do_dump_instructions --- dump command */

int
do_dump_instructions(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	FILE *fp;
  
	if (arg != NULL && arg->type == D_string) {
		/* dump to a file */
		if ((fp = fopen(arg->a_string, "w")) == NULL) {
			d_error(_("could not open `%s' for writing (%s)"),
					arg->a_string, strerror(errno));
			return FALSE;
		}
		pf_data.print_func = fprintf;
		pf_data.fp = fp;
		pf_data.defn = TRUE;	/* in_dump = TRUE */
		(void) print_code(code_block, &pf_data);
		(void) foreach_func((int (*)(INSTRUCTION *, void *)) print_code,
		                     FALSE, /* sort */
		                     &pf_data /* data */
		                   );
		fclose(fp);
		return FALSE;
	}

	initialize_pager(out_fp);
	if (setjmp(pager_quit_tag) == 0) {
		pf_data.print_func = gprintf;
		pf_data.fp = out_fp;
		pf_data.defn = TRUE;	/* in_dump = TRUE */
		(void) print_code(code_block, &pf_data);
		(void) foreach_func((int (*)(INSTRUCTION *, void *)) print_code,
		                     FALSE,	/* sort */
		                     &pf_data	/* data */
		                   );
	}
	return FALSE;
}

/* do_save --- save command */

int
do_save(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
#ifdef HAVE_LIBREADLINE
	FILE *fp;
	HIST_ENTRY **hist_list;
	int i;

	if ((fp = fopen(arg->a_string, "w")) == NULL) {
		d_error(_("could not open `%s' for writing (%s)"),
				arg->a_string, strerror(errno));
		return FALSE;
	}

	hist_list = history_list();
	if (hist_list && history_length > sess_history_base) {
		for (i = sess_history_base; hist_list[i] != NULL; i++) {
			char *line;
			line = hist_list[i]->line;

			/* exclude save commands;
			 * N.B.: this test may fail if there is another
			 * command with the same first 2 letters.
			 */

			if (strlen(line) > 1
					&& STREQN(line, "sa", 2))	
				continue;

			fprintf(fp, "%s\n", line);
		}
	}
	fclose(fp);
#endif
	return FALSE;
}

/* do_option --- option command */

int
do_option(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	const struct dbg_option *opt;	
	char *name, *value;
	
	if (arg == NULL) {	/* display all available options and corresponding values */
		for (opt = option_list; opt->name; opt++) {
			if (opt->str_val != NULL)
				fprintf(out_fp, "%s = \"%s\"\n", opt->name, *(opt->str_val));
			else
				fprintf(out_fp, "%s = %d\n", opt->name, *(opt->num_val));
		}
		return FALSE;
	}

	name = arg->a_string;
	arg = arg->next;
	value = arg ? arg->a_string : NULL;

	for (opt = option_list; opt->name; opt++) {	/* linear search */
		if (STREQ(name, opt->name))
			break;
	}
	if (! opt->name)
		return FALSE;

	if (value == NULL) {	/* display current setting */
		if (opt->str_val != NULL)
			fprintf(out_fp, "%s = \"%s\"\n", opt->name, *(opt->str_val));
		else
			fprintf(out_fp, "%s = %d\n", opt->name, *(opt->num_val));
	} else
		(*(opt->assign))(value);
	return FALSE;
}


#ifdef HAVE_LIBREADLINE

/* initialize_pager --- initialize our idea of the terminal size */ 

void
initialize_pager(FILE *fp)
{
	if (! os_isatty(fileno(fp)) || ! input_from_tty || input_fd != 0) {
		screen_width = INT_MAX;
		screen_height = INT_MAX;
	} else {
		/* get the terminal size from readline. */

		rl_reset_terminal(NULL); /* N.B.: NULL argument means
		                          * "use TERM env variable for terminal name".
		                          */
		rl_get_screen_size(&screen_height, &screen_width);
		if (screen_height <= 1)
			screen_height = INT_MAX;
		if (screen_width <= 1)
			screen_width = INT_MAX;
	}
	pager_lines_printed = 0;
}
#endif

static void
prompt_continue(FILE *fp)
{
	int quit_pager = FALSE;

	if (os_isatty(fileno(fp)) && input_fd == 0)
		quit_pager = prompt_yes_no(
	                _("\t------[Enter] to continue or q [Enter] to quit------"),
	                _("q")[0], FALSE, fp);
	if (quit_pager)
		longjmp(pager_quit_tag, 1);
	pager_lines_printed = 0;
}    

/* gprintf --- like fprintf but allows paging */ 

int
gprintf(FILE *fp, const char *format, ...)
{
	va_list args;
	static char *buf = NULL;
	static size_t buflen = 0;
	static int bl = 0;
	char *p, *q;
	int nchar;

#define GPRINTF_BUFSIZ 512
	if (buf == NULL) {
		buflen = GPRINTF_BUFSIZ;
		emalloc(buf, char *, (buflen + 2) * sizeof(char), "gprintf");
	} else if (buflen - bl < GPRINTF_BUFSIZ/2) {
		buflen += GPRINTF_BUFSIZ;
		erealloc(buf, char *, (buflen + 2) * sizeof(char), "gprintf");
	}	 
#undef GPRINTF_BUFSIZ
	
	while (TRUE) {
		va_start(args, format);
		nchar = vsnprintf(buf + bl, buflen - bl, format, args);
		va_end(args);
		if (nchar == 0)
			return 0;
		if (nchar > 0 && nchar < buflen - bl) {
			bl += nchar;
			if (buf[bl-1] != '\n') /* buffer output until see a newline at end */
				return nchar;
			break;
		}

		/* enlarge buffer, and try again */ 
		buflen *= 2;
		erealloc(buf, char *, (buflen + 2) * sizeof(char), "gprintf");
	}

	bl = 0;
	for (p = buf; (q = strchr(p, '\n')) != NULL; p = q + 1) {
		int sz = (int) (q - p); 

		while (sz > 0) {
			int cnt;
			cnt = sz > screen_width ? screen_width : sz;

			/* do not print partial line before scrolling */
			if (cnt < sz && (pager_lines_printed == (screen_height - 2)))
				prompt_continue(fp);

			if (fwrite(p, sizeof(char), cnt, fp) != cnt)
				return -1;
			if (cnt == sz)
				break;
			else {
				if (++pager_lines_printed == (screen_height - 1))
					prompt_continue(fp);
				sz -= screen_width;
				assert(sz > 0);
				p += cnt;
			}			
		}

		fprintf(fp, "\n");
		if (++pager_lines_printed == (screen_height - 1))
			prompt_continue(fp);
		p++;
	}
	return nchar;
}


static int
serialize_subscript(char *buf, int buflen, struct list_item *item)
{
	int bl = 0, nchar, i;
	NODE *sub;

	nchar = snprintf(buf, buflen, "%d%c%d%c%s%c%d%c",
				item->number, FSEP, D_subscript, FSEP, item->sname, FSEP,
				item->num_subs, FSEP);
	if (nchar <= 0)
		return 0;
	else if (nchar >= buflen)	/* need larger buffer */
		return nchar;
 	bl += nchar;
	for (i = 0; i < item->num_subs; i++) {
		sub = item->subs[i];
		nchar = snprintf(buf + bl, buflen - bl, "%lu%c%s%c",
					(unsigned long) sub->stlen, FSEP, sub->stptr, FSEP);
		if (nchar <= 0)
			return 0;
		bl += nchar;
		if (bl >= buflen)	/* need larger buffer */
			return bl;
	}
	return bl;
}



/* serialize --- convert a list structure to a byte stream and
 *               save in environment.
 */

static void
serialize(int type)
{
#ifndef HAVE_LIBREADLINE
#define HIST_ENTRY void
#define history_list()	NULL
#endif

	static char *buf = NULL;
	static int buflen = 0;
	int bl;
	BREAKPOINT *b = NULL;
	struct list_item *wd = NULL;
	HIST_ENTRY **hist_list = NULL;
	int hist_index = 0;
	struct dbg_option *opt = NULL;
	struct commands_item *commands = NULL, *c;
	int cnum = 0;
	struct condition *cndn = NULL;
	void *ptr, *end_ptr;
#ifdef HAVE_LIBREADLINE
	HIST_ENTRY *h = NULL;
#endif

	switch (type) {
	case BREAK:
		end_ptr = (void *) &breakpoints;
		ptr = (void *) breakpoints.prev;
		break;
	case WATCH:
		end_ptr = (void *) &watch_list;
		ptr = (void *) watch_list.prev;
		break;
	case DISPLAY:
		end_ptr = (void *) &display_list;
		ptr = (void *) display_list.prev;
		break;
	case HISTORY:
		hist_list = history_list();
		if (hist_list == NULL) /* empty history list */
			return;
		end_ptr = NULL;
		ptr = (void *) hist_list[0];
		break;
	case OPTION:
	{
		int n;
		n = sizeof(option_list)/sizeof(option_list[0]);
		end_ptr = (void *) &option_list[n - 1];
		ptr = (void *) option_list;
	}
		break;

	default:
		return;
	}

	if (type != HISTORY && ptr == end_ptr)		/* empty list */
		return;

#define SERIALIZE_BUFSIZ 512

	if (buf == NULL) {	/* first time */
		buflen = SERIALIZE_BUFSIZ;
		emalloc(buf, char *, buflen + 2, "serialize");
	}
	bl = 0;

	while (ptr != end_ptr) {
		int nchar = 0;
		if (buflen - bl < SERIALIZE_BUFSIZ/2) {
enlarge_buffer:
			buflen *= 2;
			erealloc(buf, char *, buflen + 2, "serialize");
		}

#undef SERIALIZE_BUFSIZ

		/* field seperator is FSEP ('\037'), and the record separator is RSEP ('\036') */

		switch (type) {
		case BREAK:
			b = (BREAKPOINT *) ptr;

			/* src source_line flags ignore_count hit_count number;
			 * commands and condition processed later in the end switch
			 */

			nchar = snprintf(buf + bl, buflen - bl,
			                 "%s%c%d%c%d%c%d%c%d%c%d%c",
			                 b->src, FSEP, b->bpi->source_line, FSEP, b->flags, FSEP,
			                 (int) b->ignore_count, FSEP,
			                 (int) b->hit_count, FSEP, b->number, FSEP);
			cnum = b->number;
			commands = &b->commands;
			cndn = &b->cndn;
			break;
		case DISPLAY:
		case WATCH:
			wd = (struct list_item *) ptr;

			/* subscript    -- number type sname num_subs subs(stlen + stptr) [commands [condition]]
			 * variable     -- number type sname [commands [condition]]
			 * field        -- number type symbol(numbr) [commands [condition]]
			 */

			if (IS_PARAM(wd))	/* exclude parameters */
				nchar = 0;
			else if (IS_SUBSCRIPT(wd))
				nchar = serialize_subscript(buf + bl, buflen - bl, wd);
			else if (IS_FIELD(wd))
				nchar = snprintf(buf + bl, buflen - bl, "%d%c%d%c%d%c",
				            wd->number, FSEP, D_field, FSEP, (int) wd->symbol->numbr, FSEP);
			else
				nchar = snprintf(buf + bl, buflen - bl, "%d%c%d%c%s%c",
				            wd->number, FSEP, D_variable, FSEP, wd->sname, FSEP);
			cnum = wd->number;	
			commands = &wd->commands;
			cndn = &wd->cndn;
			break;
		case HISTORY:
#ifdef HAVE_LIBREADLINE
			h = (HIST_ENTRY *) ptr;
			nchar = strlen(h->line);
			if (nchar >= buflen - bl)
				goto enlarge_buffer;
			strcpy(buf + bl, h->line);
#endif
			break;
		case OPTION:
			opt = (struct dbg_option *) ptr;
			if (opt->num_val != NULL)
				nchar = snprintf(buf + bl, buflen - bl,
								"%s%c%d%c", opt->name, FSEP, *(opt->num_val), FSEP);
			else
				nchar = snprintf(buf + bl, buflen - bl,
								"%s%c%s%c", opt->name, FSEP, *(opt->str_val), FSEP);
			break;
		default:
			break;
		}

		if (nchar == 0)	/* skip empty history lines etc.*/
			;
		else if (nchar > 0 && nchar  < buflen - bl) {
			bl += nchar;
			buf[bl] = RSEP;	/* record */
			buf[++bl] = '\0';
		} else
			goto enlarge_buffer;

		switch (type) {
		case BREAK:
		case WATCH:
			/* recreate the `commands' command strings including the `commands'
			 * and `end' commands; command seperator is '\034'.
			 * re-parsed in unserialize to recover the commands list.
			 * Alternatively, one could encode(serialize) each command and it's arguments.
			 */

			bl--;	/* undo RSEP from above */

			/* compute required room in buffer */
			nchar = 0;
			for (c = commands->next; c != commands; c = c->next) {
				nchar += (strlen(c->cmd_string) + 1);
				if (c->cmd == D_eval) {
					CMDARG *a = c->arg;
					nchar += (strlen(a->a_string) + 1);	/* awk statements */
					nchar += (strlen("end") + 1);
				}
			}

			if (nchar > 0) {	/* non-empty commands list */
				nchar += (strlen("commands ") + 20 + strlen("end") + 2); /* 20 for cnum (an int) */
				if (nchar > buflen - bl) {
					buflen = bl + nchar;
					erealloc(buf, char *, buflen + 3, "serialize");
				}
				nchar = sprintf(buf + bl, "commands %d", cnum);
				bl += nchar;
				buf[bl++] = CSEP;
				for (c = commands->next; c != commands; c = c->next) {
					nchar = strlen(c->cmd_string);
					memcpy(buf + bl, c->cmd_string, nchar);
					bl += nchar;
					buf[bl++] = CSEP;

					if (c->cmd == D_eval) {
						CMDARG *a = c->arg;
						nchar = strlen(a->a_string);	/* statements */
						memcpy(buf + bl, a->a_string, nchar);
						bl += nchar;
						buf[bl++] = CSEP;
						nchar = strlen("end");	/* end of 'eval' */
						memcpy(buf + bl, "end", nchar);
						bl += nchar;
						buf[bl++] = CSEP;
					}
				}
				nchar = strlen("end");	/* end of 'commands' */
				memcpy(buf + bl, "end", nchar);
				bl += nchar;
			}
			buf[bl++] = FSEP;		/* field */
			buf[bl++] = RSEP;		/* record */
			buf[bl] = '\0';
	
			/* condition expression */	
			if (cndn->expr) {
				bl--;	/* undo RSEP from above */
				nchar = strlen(cndn->expr);
				if (nchar > buflen - bl) {
					buflen = bl + nchar;
					erealloc(buf, char *, buflen + 3, "serialize");
				}
				memcpy(buf + bl, cndn->expr, nchar);
				bl += nchar;
				buf[bl++] = FSEP;		/* field */
				buf[bl++] = RSEP;		/* record */
				buf[bl] = '\0';
			}

			ptr = (type == BREAK) ? (void *) b->prev : (void *) wd->prev;
			break;
		case DISPLAY:
			ptr = (void *) wd->prev;
			break;
		case HISTORY:
			ptr = (void *) hist_list[++hist_index];
			break;
		case OPTION:
			ptr = (void *) (++opt);
			break;
		default:
			break;
		}  
	}

	if (bl > 0)	/* non-empty list */
		setenv(env_variable[type], buf, 1);
}


static void
unserialize_commands(char *str, int str_len)
{
	if (str_len <= 0 || str == NULL)
		return;
	commands_string = str;
	commands_string_len = str_len;
	push_cmd_src(INVALID_HANDLE, FALSE, read_commands_string, 0, 0, EXIT_FATAL);
	line_sep = CSEP;
	read_command();		/* forced to return in do_commands */
	pop_cmd_src();
}


/* unserialize_list_item --- create a list_item structure from unserialized data */
 
static struct list_item *
unserialize_list_item(struct list_item *list, char **pstr, int *pstr_len, int field_cnt)
{
	int num, type, i;
	struct list_item *l;
	NODE *symbol = NULL;
	int sub_cnt = 0, cnt;
	NODE **subs = NULL;

	/* subscript	-- number type sname num_subs subs [commands [condition]]
	 * variable		-- number type sname [commands [condition]]
	 * field		-- number type symbol(numbr) commands [commands [condition]]
	 */

	num = strtol(pstr[0], NULL, 0);
	type = strtol(pstr[1], NULL, 0);

	if (type == D_field) {
		int field_num;
		field_num = strtol(pstr[2], NULL, 0);
		symbol = make_number((AWKNUM) field_num);
		cnt = 3;
	} else {
		char *name;
		name = estrdup(pstr[2], pstr_len[2]);
		symbol = find_symbol(name, NULL);
		efree(name);
		if (symbol == NULL)
			return NULL;
		cnt = 3;
		if (type == D_subscript) {
			int sub_len;
			sub_cnt = strtol(pstr[3], NULL, 0);
			emalloc(subs, NODE **, sub_cnt * sizeof(NODE *), "unserialize_list_item");
			cnt++;
			for (i = 0; i < sub_cnt; i++) {
				sub_len = strtol(pstr[cnt], NULL, 0);
				subs[i] = make_string(pstr[cnt + 1], sub_len);
				cnt += 2;
			}
		}
	}

	l = add_item(list, type, symbol, NULL);
	if (type == D_subscript) {
		l->num_subs = sub_cnt;
		l->subs = subs;
	}
	l->number = num;	/* keep same item number across executions */
	
	if (list == &watch_list) {
		initialize_watch_item(l);
		/* unserialize watchpoint `commands' */
		unserialize_commands(pstr[cnt], pstr_len[cnt]);
		cnt++;
		if (field_cnt > cnt) {
			char *expr;
			expr = estrdup(pstr[cnt], pstr_len[cnt]);
			if (parse_condition(D_watch, l->number, expr) != 0)
				efree(expr);
		}
		if (num > list->number)   /* update list number counter */
			list->number = num;
	} else
		list->number = num;

	return l;	
}

/* unserialize_breakpoint --- create a breakpoint structure from unserialized data */
 
static BREAKPOINT *
unserialize_breakpoint(char **pstr, int *pstr_len, int field_cnt)
{
	char *src;
	int lineno;
	BREAKPOINT *b = NULL;
	INSTRUCTION *rp;
	SRCFILE *s;

	/* src source_line flags ignore_count hit_count number commands [condition] */

	src = estrdup(pstr[0], pstr_len[0]);
	s = source_find(src);
	efree(src);
	if (s == NULL)
		return NULL;
	src = s->src;
	lineno = strtol(pstr[1], NULL, 0);
	if (lineno <= 0 || lineno > s->srclines)
		return NULL;
	rp = find_rule(src, lineno);
	if (rp == NULL
			||  (b = set_breakpoint_at(rp, lineno, TRUE)) == NULL
	)
		return NULL;

	b->flags =  strtol(pstr[2], NULL, 0);
	b->ignore_count = strtol(pstr[3], NULL, 0);
	b->hit_count = strtol(pstr[4], NULL, 0);
	b->number =  strtol(pstr[5], NULL, 0);	/* same number as previous run */

	if (field_cnt > 6)	/* unserialize breakpoint `commands' */
		unserialize_commands(pstr[6], pstr_len[6]);

	if (field_cnt > 7) {	/* condition expression */ 
		char *expr;
		expr = estrdup(pstr[7], pstr_len[7]);
		if (parse_condition(D_break, b->number, expr) != 0)
			efree(expr);
	}

	if (b->number > watch_list.number)  /* watch and break has same number counter */
		watch_list.number = b->number;  /* update counter */
	return b;
}

/* unserialize_option --- set a debugger option from unserialized data. */

static struct dbg_option *
unserialize_option(char **pstr, int *pstr_len, int field_cnt ATTRIBUTE_UNUSED)
{
	const struct dbg_option *opt;

	for (opt = option_list; opt->name; opt++) {
		if (STREQN(pstr[0], opt->name, pstr_len[0])) {
			char *value;
			value = estrdup(pstr[1], pstr_len[1]);
			(*(opt->assign))(value);
			efree(value);
			return ((struct dbg_option *) opt);
		}
	}
	return NULL;
}

/* unserialize -- reconstruct list from serialized data stored in
 *                environment variable.
 */

static void
unserialize(int type)
{
	char *val;
	char *p, *q, *r, *s;
#define MAX_FIELD 30
	static char *pstr[MAX_FIELD];
	static int pstr_len[MAX_FIELD];

	val = getenv(env_variable[type]);
	if (val == NULL)
		return;

	for (p = val; (q = strchr(p, RSEP)) != NULL; p = q + 1) {
		int field_cnt = 0;
		if (type == HISTORY) {
			*q = '\0';
			add_history(p);
			*q = RSEP;
			continue;
		}

		r = p;
		while ((s = strchr(r, FSEP)) != NULL && s < q) {
			pstr[field_cnt] = r;
			pstr_len[field_cnt] = (int) (s - r);
			r = s + 1;
			field_cnt++;
			if (field_cnt == MAX_FIELD)
#ifdef GAWKDEBUG
				fatal("Increase MAX_FIELD and recompile.\n");
#else
				return;	
#endif
		}

		switch (type) {
		case BREAK:
			(void) unserialize_breakpoint(pstr, pstr_len, field_cnt);
			break;
		case DISPLAY:
			(void) unserialize_list_item(&display_list, pstr, pstr_len, field_cnt);
			break;
		case WATCH:
			(void) unserialize_list_item(&watch_list, pstr, pstr_len, field_cnt);
			break;
		case OPTION:
			(void) unserialize_option(pstr, pstr_len, field_cnt);
			break;
		case HISTORY:
			/* processed at the beginning of for loop */
			break;
		default:
			break;
		}
	}

#ifdef HAVE_LIBREADLINE
	if (type == HISTORY)
		sess_history_base = history_length;
#endif

	unsetenv(env_variable[type]);
#undef MAX_FIELD
}

static int
prompt_yes_no(const char *mesg, char res_true, int res_default, FILE *fp)
{
	char *in_str;
	int ret = res_default; /* default */

	if (input_from_tty) {
		fprintf(fp, "%s", _(mesg));
		in_str = read_a_line(NULL);
		if (in_str == NULL)	/* EOF */
			exit(EXIT_FAILURE);
		ret = (*in_str == res_true);
		efree(in_str);
	}
	return ret;
}

/* has_break_or_watch_point --- check if given breakpoint or watchpoint
 *                              number exists. When flag any is TRUE,
 *                              check if any breakpoint/watchpoint
 *                              has been set (ignores num). Returns
 *                              type (breakpoint or watchpoint) or 0.
 */

int
has_break_or_watch_point(int *pnum, int any)
{
	BREAKPOINT *b = NULL;
	struct list_item *w = NULL;

	if (any) {
		if (breakpoints.next != &breakpoints)
			b = breakpoints.next;
		if (watch_list.next != &watch_list)
			w = watch_list.next;

		if (! b && ! w)
			return 0;
		if (b && ! w) {
			*pnum = b->number;
			return D_break;
		}
		if (w && ! b) {
			*pnum = w->number;
			return D_watch;
		}
		if (w->number > b->number) {
			*pnum = w->number;
			return D_watch;
		}
		*pnum = b->number;
		return D_break;
	}

	/* N.B: breakpoints and watchpoints get numbers from a single
	 * counter/sequencer watch_list.number.
	 */ 

	for (b = breakpoints.next; b != &breakpoints; b = b->next) {
		if (b->number == *pnum)
			return D_break;
	}
	for (w = watch_list.next; w != &watch_list; w = w->next) {
		if (w->number == *pnum)
			return D_watch;
	}

	return 0;
}

/* delete_commands_item --- delete item(command) from `commands' list. */

static void
delete_commands_item(struct commands_item *c)
{
	efree(c->cmd_string);
	free_cmdarg(c->arg);
	c->next->prev = c->prev;
	c->prev->next = c->next;
	efree(c);
}

/* do_commands --- commands command */

int
do_commands(CMDARG *arg, int cmd)
{
	static BREAKPOINT *b;
	static struct list_item *w;
	static struct commands_item *commands;
	struct commands_item *c;
	
	if (cmd == D_commands) {
		int num, type;
		if (arg == NULL)
			type = has_break_or_watch_point(&num, TRUE);
		else {
			num = arg->a_int;
			type = has_break_or_watch_point(&num, FALSE);
		}
		b = NULL;
		w = NULL;
		if (type == D_break)
			b = find_breakpoint(num);
		else if (type == D_watch)
			w = find_item(&watch_list, num);
		assert((b != NULL) || (w != NULL));
		commands = (b != NULL) ? &b->commands : &w->commands;

		/* delete current commands */
		for (c = commands->next; c != commands; c = c->next) {
			c = c->prev;
			delete_commands_item(c->next);
		}
		return FALSE;

	} else if (cmd == D_end) {
		commands = NULL;
		if (read_a_line == read_commands_string) /* unserializig commands */
			return TRUE;	/* done unserializing, terminate zzparse() */
		return FALSE;

	} else if (cmd == D_silent) {
		if (b != NULL)
			b->silent = TRUE;
		else if (w != NULL)
			w->silent = TRUE;
		/* we also append silent command to the list for use
		 * in `info break(watch)', and to simplify
		 * serialization/unserialization of commands.
		 */
	}

	assert(commands != NULL);

	emalloc(c, struct commands_item *, sizeof(struct commands_item), "do_commands");
	c->next = NULL;
	c->cmd = cmd;

	/* N.B.: first arg is the command string, see command.y */ 
	c->cmd_string = arg->a_string;
	c->arg = arg->next; /* actual arguments to the command */
	efree(arg);

	/* append to the list */
	c->prev = commands->prev;
	c->next = commands;
	commands->prev = c;
	c->prev->next = c;
	return FALSE;
}

/* execute_commands --- execute breakpoint/watchpoint commands, the first
 *                      command that resumes execution terminates
 *                      commands processing.
 */

static int
execute_commands(struct commands_item *commands)
{
	struct commands_item *c;
	Func_cmd cmd_ptr;
	int ret = FALSE;

	for (c = commands->next; c != commands; c = c->next) {
		if (c->cmd == D_silent)
			continue;
		cmd_ptr = get_command(c->cmd);		/* command handler */
		ret = (*cmd_ptr)(c->arg, c->cmd);
		if (ret)	/* resume execution (continue, next etc.) */
			break;
	}
	return ret;
}

/* do_print_f --- printf command */

int
do_print_f(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	int count = 0;
	int i;
	CMDARG *a;
	NODE **tmp;
	char *name;
	NODE *r;
	volatile jmp_buf fatal_tag_stack;

	/* count maximum required size for tmp */
	for (a = arg; a != NULL ; a = a->next)
		count++;
	emalloc(tmp, NODE **, count * sizeof(NODE *), "do_print_f");

	for (i = 0, a = arg; a != NULL ; i++, a = a->next) {
		switch (a->type) {
		case D_variable:
			name = a->a_string;
			r = find_symbol(name, NULL);
			if (r == NULL)
				goto done;
			if (r->type == Node_var_new)
				tmp[i] = Nnull_string;
			else if (r->type != Node_var) {
				d_error(_("`%s' is not a scalar variable"), name);
				goto done;
			} else
				tmp[i] = r->var_value;
			break;
		case D_field:
		{
			long field_num;
			r = a->a_node;
			field_num = (long) r->numbr;
			tmp[i] = *get_field(field_num, NULL);
		}
			break;
		case D_subscript:
		{
			int cnt = a->a_count;
			name = a->a_string;
			r = find_array(name);
			if (r == NULL)
				goto done;

			for (; cnt > 0; cnt--) {
				NODE *value, *subs; 
				a = a->next;
				subs = a->a_node;
				value = in_array(r, subs);
				if (cnt == 1) {
					if (value == NULL)
						tmp[i] = Nnull_string;		/* FIXME: goto done ? */
					else if (value->type == Node_var_array) {
						d_error(_("attempt to use array `%s[\"%s\"]' in a scalar context"),
									name, subs->stptr);
						goto done;
					} else 
						tmp[i] = value;
				} else {
					if (value == NULL) {
						d_error(_("[\"%s\"] not in array `%s'"),
									subs->stptr, name);
						goto done;
					} else if (value->type != Node_var_array) {
						d_error(_("attempt to use scalar `%s[\"%s\"]' as array"),
									name, subs->stptr);
						goto done;
					} else {
						r = value;
						name = r->vname;
					}
				}
			}
		}
			break;			 
		case D_node:
			tmp[i] = a->a_node;
			break;
		default:
			break;
		}
	}

	force_string(tmp[0]);

	PUSH_BINDING(fatal_tag_stack, fatal_tag, fatal_tag_valid);
	if (setjmp(fatal_tag) == 0)
		r = format_tree(tmp[0]->stptr, tmp[0]->stlen, tmp, i);
	else {
		/* fatal error, restore exit_val of program */
		exit_val = EXIT_SUCCESS;
		r = NULL;
	}
	POP_BINDING(fatal_tag_stack, fatal_tag, fatal_tag_valid);

	if (r != NULL) {
		(void) fwrite(r->stptr, sizeof(char), r->stlen, out_fp);
		unref(r);
	}
done:
	efree(tmp);
	return FALSE;
}

/* do_source --- source command */

int
do_source(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	int fd;
	char *file = arg->a_string;

	fd = open_readfd(file);
	if (fd <= INVALID_HANDLE) {
		d_error(_("can't open source file `%s' for reading (%s)"),
					file, strerror(errno));
		return FALSE;
	}

	push_cmd_src(fd, FALSE, g_readline, close, D_source, EXIT_SUCCESS);
	cmd_src->str = estrdup(file, strlen(file));
	return FALSE;
}

/* open_readfd --- open a file for reading */

static int
open_readfd(const char *file)
{
	int fd;
	
	fd = open(file, O_RDONLY);
	if (fd <= INVALID_HANDLE)
		return INVALID_HANDLE;
	else if (os_isdir(fd)) {
		(void) close(fd);
		errno = EISDIR;
		return INVALID_HANDLE;
	}
	return fd;
}

/* find_option --- check if option name is valid */

int
find_option(char *name)
{
	const char *p;
	int idx;

	for (idx = 0; (p = option_list[idx].name); idx++) {
		if (STREQ(p, name))
			return idx;
	}
	return -1;
}

/* option_help --- display help text for debugger options */

void
option_help()
{
	const struct dbg_option *opt;	

	for (opt = option_list; opt->name; opt++)
		fprintf(out_fp, "\t%-15.15s - %s\n", opt->name, _(opt->help_txt));
}

#ifdef HAVE_LIBREADLINE
	
/* option_generator --- generator function for option name completion */

char *
option_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx;
	const char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}

	while ((name = option_list[idx++].name)) {
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}
	return NULL;
}

#endif

/* set_gawk_output --- redirect gawk (normal) output */

static void
set_gawk_output(const char *file)
{
	int fd = INVALID_HANDLE;
	FILE *fp = NULL;

	if (output_fp != stdout) {
		if (output_fp != stderr) {
			fclose(output_fp);
			efree(output_file);
		}
		output_fp = stdout;
		output_is_tty = os_isatty(fileno(stdout));
		output_file = "/dev/stdout";
	}

	if (file == NULL || file[0] == '\0')
		return;

	errno = 0;
	if ((fd = os_devopen(file, O_WRONLY)) != INVALID_HANDLE) {
		fp = fdopen(fd, "w");
		if (fp == NULL)
			close(fd);

	} else if (STREQN(file, "/dev/", 5)) {
		char *cp = (char *) file + 5;

		if (STREQ(cp, "stdout"))
			return;
		if (STREQ(cp, "stderr")) {
			output_fp = stderr;
			output_file = "/dev/stderr";
			output_is_tty = os_isatty(fileno(stderr));
			return;
		}
		
		if (STREQN(cp, "fd/", 3)) {
			cp += 3;
			fd = (int) strtoul(cp, NULL, 10);
			if (errno == 0 && fd > INVALID_HANDLE) {
				fp = fdopen(fd, "w");
				if (fp == NULL)
					fd = INVALID_HANDLE;
			} else
				fd = INVALID_HANDLE;
		} else {
			/* /dev/ttyN, /dev/pts/N, /dev/null etc. */
			fd = open(file, O_WRONLY);
		}

		if (fd > INVALID_HANDLE && fp == NULL) {
			fp = fdopen(fd, "w");
			if (fp == NULL)
				close(fd); 
		}

	} else {
		/* regular file */
		fp = fopen(file, "w");
	}

	if (fp != NULL) {
		output_fp = fp;
		output_file = estrdup(file, strlen(file));
		setbuf(fp, (char *) NULL);
		output_is_tty = os_isatty(fileno(fp));
	} else {
		d_error(_("could not open `%s' for writing (%s)"),
					file,
					errno != 0 ? strerror(errno) : _("reason unknown"));
		fprintf(out_fp, _("sending output to stdout\n"));
	}
}

/* set_prompt --- set debugger prompt */

static void
set_prompt(const char *value)
{
	efree(dgawk_Prompt);
	dgawk_Prompt = estrdup(value, strlen(value));
	dPrompt = dgawk_Prompt;
}

/* set_option_flag --- convert option string to flag value */ 

static int
set_option_flag(const char *value)
{
	long n;
	if (STREQ(value, "on"))
		return TRUE;
	if (STREQ(value, "off"))
		return FALSE;
	errno = 0;
	n = strtol(value, NULL, 0);
	return (errno == 0 && n != 0);
}

/* set_option_num --- set integer option value from string */

static void
set_option_num(int *pnum, const char *value)
{
	long n;
	errno = 0;
	n = strtol(value, NULL, 0);
	if (errno == 0 && n > 0)
		*pnum = n;
	else
		d_error(_("invalid number"));
}

/* set_listsize --- set list output window size */

static void
set_listsize(const char *value)
{
	set_option_num(&list_size, value);
}

/* set_trace --- set instruction tracing on or off */

static void
set_trace(const char *value)
{
	do_trace = set_option_flag(value);
}

/* set_save_history --- save history on exit */

static void
set_save_history(const char *value)
{
	do_save_history = set_option_flag(value);
}

/* set_save_options --- save options on exit */

static void
set_save_options(const char *value)
{
	do_save_options = set_option_flag(value);
}

/* set_history_size --- maximum entries in history file */

static void
set_history_size(const char *value)
{
	set_option_num(&history_size, value);
}


/* read_commands_string --- one of the many ways zzlex fetches a line to parse;
 *                          this one is used to parse `commands' string during
 *                          unserialization.
 */

char *
read_commands_string(const char *prompt ATTRIBUTE_UNUSED)
{
	char *p, *end, *line;

	if (commands_string == NULL)
		return NULL;

	p = (char *) commands_string;
	end = (char *) commands_string + commands_string_len; 
	for (; p < end; p++) {
		if (*p == line_sep) {
			line = estrdup(commands_string, p - commands_string);
			commands_string = p + 1;
			commands_string_len = end - commands_string;
			return line;
		}
	}

	line = estrdup(commands_string, commands_string_len);
	commands_string = NULL;
	commands_string_len = 0;
	return line;
}

/* save_options --- save current options to file */

static void
save_options(const char *file)
{
	FILE *fp;
	const struct dbg_option *opt;	

	fp = fopen(file, "w");
	if (fp == NULL)
		return;

	for (opt = option_list; opt->name; opt++) {
		if (opt->str_val != NULL)
			fprintf(fp, "option %s = \"%s\"\n", opt->name, *(opt->str_val));
		else
			fprintf(fp, "option %s = %d\n", opt->name, *(opt->num_val));
	}
	fclose(fp);
	chmod(file, 0600);
}

/* close_all --- close all open files */

static void
close_all()
{
	int stdio_problem;
	struct command_source *cs;

	(void) nextfile(&curfile, TRUE);	/* close input data file */
	(void) close_io(&stdio_problem);
	if (cur_srcfile->fd != INVALID_HANDLE) {
		close(cur_srcfile->fd);
		cur_srcfile->fd = INVALID_HANDLE;
	}
	for (cs = cmd_src; cs != NULL; cs = cs->next) {
		if (cs->close_func && cs->fd != INVALID_HANDLE) {
			cs->close_func(cs->fd);
			cs->fd = INVALID_HANDLE;
		}
	}

	set_gawk_output(NULL);	/* closes output_fp if not stdout */ 
}

/* install_params --- install function parameters into the symbol table */

static void
install_params(NODE *func)
{
	NODE *np;

	if (func == NULL)
		return;
	/* function parameters of type Node_param_list */
	np = func->lnode;				
	for (np = np->rnode; np != NULL; np = np->rnode) 
		install_symbol(np->param, np);		
}

/* remove_params --- remove function parameters out of the symbol table */

static void
remove_params(NODE *func)
{
	NODE *np;

	if (func == NULL)
		return;
	np = func->lnode;				
	for (np = np->rnode; np != NULL; np = np->rnode)
		remove_symbol(np->param);
}

/* pre_execute_code --- pre_hook for execute_code, called by pre_execute */

static int
pre_execute_code(INSTRUCTION **pi)
{
	INSTRUCTION *ei = *pi;

	switch (ei->opcode) {
	case Op_K_exit:
	case Op_K_next:
	case Op_K_nextfile:
	case Op_K_getline:	/* getline without redirection */
		d_error(_("`%s' not allowed in current context;"
				" statement ignored"),
				op2str(ei->opcode));
		*pi = ei->nexti;
		break;
	case Op_K_return:
		if (ei->nexti != NULL) {	/* not an implicit return */
			NODE *r;
			d_error(_("`return' not allowed in current context;"
					" statement ignored"));
			/* throw away return value already pushed onto stack */
			r = POP_SCALAR();
			DEREF(r);
			*pi = ei->nexti;
		}
		break;
	default:
		break;
	}
	return (ei == *pi);
}

extern void unwind_stack(STACK_ITEM *sp_bottom);

static NODE *
execute_code(volatile INSTRUCTION *code)
{
	volatile NODE *r = NULL;
	volatile jmp_buf fatal_tag_stack;
	STACK_ITEM *ctxt_stack_bottom;

	/* We use one global stack for all contexts.
	 * Remember stack bottom for current context; in case of
	 * a fatal error, unwind stack until stack_ptr is below that 'bottom'.
	 */ 
	ctxt_stack_bottom = stack_ptr + 1;

	PUSH_BINDING(fatal_tag_stack, fatal_tag, fatal_tag_valid);
	if (setjmp(fatal_tag) == 0) {
		(void) r_interpret((INSTRUCTION *) code);
		assert(stack_ptr == ctxt_stack_bottom);
		r = POP_SCALAR();
	} else	/* fatal error */
		unwind_stack(ctxt_stack_bottom);

	POP_BINDING(fatal_tag_stack, fatal_tag, fatal_tag_valid);

	if (exit_val != EXIT_SUCCESS) {	/* must be EXIT_FATAL? */
		exit_val = EXIT_SUCCESS;
		return NULL;
	}
	return (NODE *) r;
}

/* do_eval --- eval command */

int
do_eval(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	NODE *r, *ret_val;
	NODE *f = NULL;
	NODE *this_frame = NULL, *this_func = NULL;
	NODE **sp;
	INSTRUCTION *eval, *code = NULL;
	AWK_CONTEXT *ctxt;
	char **save_parmlist = NULL;
	int ecount = 0, pcount = 0;
	int ret;
	
	if (prog_running) {
		this_frame = find_frame(0);
		this_func = this_frame->func_node;
	}

	install_params(this_func);	/* expose current function parameters to eval */ 
	ctxt = new_context();
	ctxt->install_func = append_symbol;	/* keep track of newly installed globals */
	push_context(ctxt);
	(void) add_srcfile(SRC_CMDLINE, arg->a_string, srcfiles, NULL, NULL);
	ret = parse_program(&code);
	remove_params(this_func);
	if (ret != 0) {
		pop_context();	/* switch to prev context */
		free_context(ctxt, FALSE /* keep_globals */);
		return FALSE;
	}

	f = lookup("@eval");
	assert(f != NULL);
	if (this_func == NULL) {	/* in main */
		/* do a function call */
		eval = bcalloc(Op_func_call, 2, 0);
		eval->source_file = cur_srcfile->src;
		eval->func_body = f;
		eval->func_name = NULL;	/* not needed, func_body already assigned */
		(eval + 1)->expr_count = 0;
		eval->nexti = bcalloc(Op_stop, 1, 0);
		
	} else {
		/* execute as a part of the current function */ 
		int i;
		char **varnames;
		INSTRUCTION *t;
		NODE *np;

		eval = f->code_ptr;	/* Op_func */
		eval->source_file = cur_srcfile->src;
		/* turn implicit Op_K_return into Op_stop */
		t = (eval + 1)->lasti;	/* Op_K_return */
		t->opcode = Op_stop;

		/* add or append eval locals to the current frame stack */
		ecount = f->lnode->param_cnt;	/* eval local count */
		pcount = this_func->lnode->param_cnt;
		save_parmlist = this_func->parmlist;
		
		if (ecount > 0) {
			if (pcount == 0)
				emalloc(this_frame->stack, NODE **, ecount * sizeof(NODE *), "do_eval");
			else
				erealloc(this_frame->stack, NODE **, (pcount + ecount) * sizeof(NODE *), "do_eval");

			emalloc(varnames, char **, (pcount + ecount + 1) * sizeof(char *), "do_eval");
			if (pcount > 0)	
				memcpy(varnames, save_parmlist, pcount * sizeof(char *));
			for (np = f->lnode->rnode, i = 0; np != NULL; np = np->rnode, i++) {
				varnames[pcount + i] = np->param;
				np->param_cnt += pcount;	/* appending eval locals: fixup param_cnt */
			}
			varnames[pcount + ecount] = NULL;
			sp = this_frame->stack + pcount;
			for (i = 0; i < ecount; i++) {
				getnode(r);
				memset(r, 0, sizeof(NODE));
				*sp++ = r;
				/* local variable */
				r->type = Node_var_new;
				r->vname = varnames[pcount + i];
			}

			this_func->parmlist = varnames;
			this_func->lnode->param_cnt += ecount;
		}
	}

#if 0
	pf_data.print_func = fprintf;
	pf_data.fp = out_fp;
	pf_data.defn = FALSE;	/* in_dump = FALSE */
	(void) print_code(f->code_ptr, &pf_data);
#endif

	ret_val = execute_code((volatile INSTRUCTION *) eval);

	if (ret_val != NULL)
		DEREF(ret_val);	/* throw away return value */
	/* else
		fatal error */

	if (this_func != NULL && ecount > 0) { 
		int i;

		/* undo frame manipulation from above */

		/* free eval locals */
		sp = this_frame->stack + pcount;
		for (i = ecount; i > 0; i--) {
			r = *sp;
			if (r->type == Node_var)     /* eval local variable */
				DEREF(r->var_value);
			else if (r->type == Node_var_array)     /* eval local array */
				assoc_clear(r);
			freenode(r);
			*sp++ = (NODE *) 0;
		}
		if (pcount == 0) {
			efree(this_frame->stack);
			this_frame->stack = NULL;
		} /* else
				restore_frame() will free it */

		efree(this_func->parmlist);
		this_func->parmlist = save_parmlist;
		this_func->lnode->param_cnt -= ecount;
	}

	/* always destroy symbol "@eval", however destroy all newly installed
	 * globals only if fatal error in r_interpret (r == NULL).
	 */

	pop_context();	/* switch to prev context */
	free_context(ctxt, (ret_val != NULL));   /* free all instructions and optionally symbols */
	if (ret_val != NULL)
		destroy_symbol("@eval");	/* destroy "@eval" */
	return FALSE;
}

/*
GDB Documentation:
	... When you use condition, GDB checks expression
immediately for syntactic correctness, and to determine whether symbols
in it have referents in the context of your breakpoint. If expression
uses symbols not referenced in the context of the breakpoint, GDB prints
an error message:  	

    No symbol "foo" in current context.
*/

static int invalid_symbol = 0;

void
check_symbol(char *name)
{
	invalid_symbol++;
	d_error(_("No symbol `%s' in current context"), name);
	/* install anyway, but keep track of it */
	append_symbol(name);
}

/* parse_condition --- compile a condition expression */

static int
parse_condition(int type, int num, char *expr)
{
	INSTRUCTION *code = NULL;
	AWK_CONTEXT *ctxt = NULL;
	int ret;
	BREAKPOINT *b;
	struct list_item *w;
	NODE *this_func = NULL;
	INSTRUCTION *it, *stop, *rule;
	struct condition *cndn = NULL;

	if (type == D_break && (b = find_breakpoint(num)) != NULL) {
		INSTRUCTION *rp;
		cndn = &b->cndn;
		rp = find_rule(b->src, b->bpi->source_line);
		if (rp != NULL && rp->opcode == Op_func)
			this_func = rp->func_body; 
	} else if (type == D_watch && (w = find_item(&watch_list, num)) != NULL) {
		cndn = &w->cndn;
		this_func = find_frame(cur_frame)->func_node;
	}

	if (cndn == NULL)
		return -1;
	if (expr == NULL)
		goto out;	/* delete condition */

	install_params(this_func);
	ctxt = new_context();
	invalid_symbol = 0;
	ctxt->install_func = check_symbol;
	push_context(ctxt);
	(void) add_srcfile(SRC_CMDLINE, expr, srcfiles, NULL, NULL);
	ret = parse_program(&code);
	remove_params(this_func); 
	pop_context();

	if (ret != 0 || invalid_symbol) {
		free_context(ctxt, FALSE /* keep_globals */);
		return -1;
	}

	/* condition expression is parsed as awk pattern without
	 * any action. The code is then modified to end up with 
	 * a `1.0' on stack when the expression is true, `0.0' otherwise.
	 */

	assert(code != NULL);
	rule = ctxt->rule_list.nexti;
	stop = bcalloc(Op_stop, 1, 0);

	it = rule->firsti;	/* Op_K_print_rec */
	assert(it->opcode == Op_K_print_rec);
	it->opcode = Op_push_i; 
	it->memory = mk_number((AWKNUM) 1.0, PERM|NUMBER|NUMCUR);
	it->nexti = bcalloc(Op_jmp, 1, 0);
	it->nexti->target_jmp = stop;
	it->nexti->nexti = rule->lasti;
 
	it = rule->lasti;		/* Op_no_op, target for Op_jmp_false */
	assert(it->opcode == Op_no_op);
	it->opcode = Op_push_i;
	it->memory = mk_number((AWKNUM) 0.0, PERM|NUMBER|NUMCUR);
	it->nexti = stop;

out:
	if (cndn->expr != NULL)
		efree(cndn->expr);
	free_context(cndn->ctxt, FALSE);
	cndn->code = code;
	cndn->expr = expr;
	cndn->ctxt = ctxt;

	return 0;
}

/* do_condition --- condition command */

int
do_condition(CMDARG *arg, int cmd ATTRIBUTE_UNUSED)
{
	int type, num;
	char *expr = NULL;

	num = arg->a_int;
	type = has_break_or_watch_point(&num, FALSE);
	if (! type)
		return FALSE;
	arg = arg->next;	/* condition expression */
	if (arg != NULL)
		expr = arg->a_string;
	if (parse_condition(type, num, expr) == 0 && arg != NULL)
		arg->a_string = NULL;	/* don't let free_cmdarg free it */
	return FALSE;
}

/* in_cmd_src --- check if filename already in cmd_src */

int
in_cmd_src(const char *filename)
{
	struct command_source *cs;
	for (cs = cmd_src; cs != NULL; cs = cs->next) {
		if (cs->str != NULL && STREQ(cs->str, filename))
			return TRUE;
	}
	return FALSE;
}

int
get_eof_status()
{
	if (cmd_src == NULL)
		return EXIT_FATAL;
	return cmd_src->eof_status;
}

void
push_cmd_src(
	int fd,
	int istty,
	char * (*readfunc)(const char *),
	int (*closefunc)(int),
	int ctype,
	int eofstatus)
{
	struct command_source *cs;
	emalloc(cs, struct command_source *, sizeof(struct command_source), "push_cmd_src");
	cs->fd = fd;
	cs->is_tty = istty;
	cs->read_func = readfunc;
	cs->close_func = closefunc;
	cs->cmd = ctype;

	/* eof_status = EXIT_FATAL     - exit with status EXIT_FATAL on EOF or error.
	 *            = EXIT_FAILURE   - exit status EXIT_FAILURE on error.
	 *            = EXIT_SUCCESS   - don't exit on EOF or error. 
	 */
	cs->eof_status = eofstatus;
	cs->str = NULL;
	cs->next = cmd_src;
	cmd_src = cs;

	input_fd = fd;
	input_from_tty = istty;
	read_a_line = readfunc;
}

int
pop_cmd_src()
{
	struct command_source *cs;

	if (cmd_src->next == NULL)
		return -1;

	cs = cmd_src;
	cmd_src = cs->next;
	if (cs->close_func && cs->fd != INVALID_HANDLE)
		cs->close_func(cs->fd);
	if (cs->str != NULL)
		efree(cs->str);
	efree(cs);

	input_fd = cmd_src->fd;
	input_from_tty = cmd_src->is_tty;
	read_a_line = cmd_src->read_func;
	return 0;
}
