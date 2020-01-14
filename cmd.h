/*
 * cmd.h - definitions for command parser
 */

/*
 * Copyright (C) 2004, 2010, 2011, 2013, 2014, 2017,
 * the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
extern char **command_completion(const char *text, size_t start, size_t end);
extern void initialize_pager(FILE *fp); /* debug.c */
extern NODE *get_function(void);
#else
#define initialize_pager(x)	((void)0)	/* nothing */
#define add_history(x)	((void)0)	/* nothing */
#endif
#define initialize_pager(x)	((void)0)	/* nothing */
#define add_history(x)	((void)0)	/* nothing */

extern int gprintf(FILE *fp, const char *format, ...);
extern jmp_buf pager_quit_tag;
extern int pager_quit_tag_valid;

extern bool output_is_tty;
extern int input_fd;
extern bool input_from_tty;
extern FILE *out_fp;
extern char *dbg_prompt;
extern char *commands_prompt;
extern char *eval_prompt;
extern char *dgawk_prompt;

enum argtype {
	D_illegal,

	/* commands */
	D_backtrace,
	D_break,
	D_clear,
	D_commands,
	D_condition,
	D_continue,
	D_delete,
	D_disable,
	D_display,
	D_down,
	D_dump,
	D_enable,
	D_end,
	D_eval,
	D_finish,
	D_frame,
	D_help,
	D_ignore,
	D_info,
	D_list,
	D_next,
	D_nexti,
	D_option,
	D_print,
	D_printf,
	D_quit,
	D_return,
	D_run,
	D_save,
	D_set,
	D_silent,
	D_source,
	D_step,
	D_stepi,
	D_tbreak,
	D_trace,
	D_undisplay,
	D_until,
	D_unwatch,
	D_up,
	D_watch,

	/* arguments */
	D_argument,
	D_int,
	D_string,
	D_variable,
	D_node,
	D_field,
	D_array,
	D_subscript,
	D_func,
	D_range
};

/* non-number arguments to commands */

enum nametypeval {
	A_NONE = 0,
	A_ARGS,
	A_BREAK,
	A_DEL,
	A_DISPLAY,
	A_FRAME,
	A_FUNCTIONS,
	A_LOCALS,
	A_ONCE,
	A_SOURCE,
	A_SOURCES,
	A_TRACE_ON,
	A_TRACE_OFF,
	A_VARIABLES,
	A_WATCH
};

typedef struct cmd_argument {
	struct cmd_argument *next;
	enum argtype type;
	union {
		long lval;
		char *sval;
		NODE *nodeval;
	} value;

#define a_int       value.lval	/* type = D_int or D_range */
#define a_argument  value.lval	/* type = D_argument */
#define a_string  value.sval	/* type = D_string, D_array, D_subscript or D_variable */
#define a_node    value.nodeval /* type = D_node, D_field or D_func */

	unsigned a_count;		/* subscript count for D_subscript and D_array */
} CMDARG;

typedef int (*Func_cmd)(CMDARG *, enum argtype);

struct cmdtoken {
	const char *name;
	char *abbrvn;	/* abbreviation */
	enum argtype type;
	int cls;
	Func_cmd cf_ptr;
	const char *help_txt;
};

/* command.c */
extern void free_cmdarg(CMDARG *list);
extern Func_cmd get_command(int ctype);
extern const char *get_command_name(int ctype);

/* debug.c */
extern void d_error(const char *mesg, ...);

/* command.c */
extern int find_option(char *name);
extern void option_help(void);
extern char *(*read_a_line)(const char *prompt);
extern char *read_commands_string(const char *prompt);
extern int in_cmd_src(const char *);
extern int get_eof_status(void);
extern void push_cmd_src(int fd, bool istty, char * (*readfunc)(const char *),
		int (*closefunc)(int), enum argtype cmd, int eofstatus);
extern int pop_cmd_src(void);
extern int has_break_or_watch_point(unsigned *pnum, bool any);
extern int do_list(CMDARG *arg, enum argtype cmd);
extern int do_info(CMDARG *arg, enum argtype cmd);
extern int do_print_var(CMDARG *arg, enum argtype cmd);
extern int do_backtrace(CMDARG *arg, enum argtype cmd);
extern int do_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_tmp_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_delete_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_enable_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_disable_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_ignore_breakpoint(CMDARG *arg, enum argtype cmd);
extern int do_run(CMDARG *arg, enum argtype cmd);
extern int do_quit(CMDARG *arg, enum argtype cmd);
extern int do_continue(CMDARG *arg, enum argtype cmd);
extern int do_step(CMDARG *arg, enum argtype cmd);
extern int do_stepi(CMDARG *arg, enum argtype cmd);
extern int do_next(CMDARG *arg, enum argtype cmd);
extern int do_nexti(CMDARG *arg, enum argtype cmd);
extern int do_clear(CMDARG *arg, enum argtype cmd);
extern int do_finish(CMDARG *arg, enum argtype cmd);
extern int do_help(CMDARG *arg, enum argtype cmd);
extern int do_up(CMDARG *arg, enum argtype cmd);
extern int do_down(CMDARG *arg, enum argtype cmd);
extern int do_frame(CMDARG *arg, enum argtype cmd);
extern int do_until(CMDARG *arg, enum argtype cmd);
extern int do_set_var(CMDARG *arg, enum argtype cmd);
extern int do_return(CMDARG *arg, enum argtype cmd);
extern int do_display(CMDARG *arg, enum argtype cmd);
extern int do_undisplay(CMDARG *arg, enum argtype cmd);
extern int do_watch(CMDARG *arg, enum argtype cmd);
extern int do_unwatch(CMDARG *arg, enum argtype cmd);
extern int do_dump_instructions(CMDARG *arg, enum argtype cmd);
extern int do_trace_instruction(CMDARG *arg, enum argtype cmd);
extern int do_option(CMDARG *arg, enum argtype cmd);
extern int do_commands(CMDARG *arg, enum argtype cmd);
extern int do_print_f(CMDARG *arg, enum argtype cmd);
extern int do_source(CMDARG *arg, enum argtype cmd);
extern int do_save(CMDARG *arg, enum argtype cmd);
extern int do_eval(CMDARG *arg, enum argtype cmd);
extern int do_condition(CMDARG *arg, enum argtype cmd);
