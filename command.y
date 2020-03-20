/*
 * command.y - yacc/bison parser for debugger commands.
 */

/*
 * Copyright (C) 2004, 2010, 2011, 2014, 2016, 2017, 2019, 2020,
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335,
 * USA 
 */

%{
#include "awk.h"
#include "cmd.h"

#if 0
#define YYDEBUG 12
int yydebug = 2;
#endif

static int yylex(void);
static void yyerror(const char *mesg, ...);

static int find_command(const char *token, size_t toklen);

static bool want_nodeval = false;

static int cmd_idx = -1;		/* index of current command in cmd table */
static int repeat_idx = -1;		/* index of last repeatable command in command table */
static CMDARG *arg_list = NULL;		/* list of arguments */
static long dbg_errcount = 0;
static char *lexptr_begin = NULL;
static bool in_commands = false;
static int num_dim;

static bool in_eval = false;
static const char start_EVAL[] = "function @eval(){";
static const char end_EVAL[] = "}";
static CMDARG *append_statement(CMDARG *stmt_list, char *stmt);
static NODE *concat_args(CMDARG *a, int count);

#ifdef HAVE_LIBREADLINE
static char *next_word(char *p, int len, char **endp);
static void history_expand_line(char **line);
static char *command_generator(const char *text, int state);
static char *srcfile_generator(const char *text, int state);
static char *argument_generator(const char *text, int state);
static char *variable_generator(const char *text, int state);
extern char *option_generator(const char *text, int state);
static int this_cmd = D_illegal;
#else
#define history_expand_line(p)	/* nothing */
static int rl_inhibit_completion;	/* dummy variable */
#endif

struct argtoken {
	const char *name;
	enum argtype cmd;
	enum nametypeval value;
};

/*
 * These two should be static, but there are some compilers that
 * don't like the static keyword with an empty size. Therefore give
 * them names that are less likely to conflict with the rest of gawk.
 */
#define argtab zz_debug_argtab
#define cmdtab zz_debug_cmdtab

extern struct argtoken argtab[];
extern struct cmdtoken cmdtab[];

static CMDARG *mk_cmdarg(enum argtype type);
static void append_cmdarg(CMDARG *arg);
static int find_argument(CMDARG *arg);
#define YYSTYPE CMDARG *
%}

%token D_BACKTRACE D_BREAK D_CLEAR D_CONTINUE D_DELETE D_DISABLE D_DOWN
%token D_ENABLE D_FINISH D_FRAME D_HELP D_IGNORE D_INFO D_LIST
%token D_NEXT D_NEXTI D_PRINT D_PRINTF D_QUIT D_RETURN D_RUN D_SET
%token D_STEP D_STEPI D_TBREAK D_UP D_UNTIL
%token D_DISPLAY D_UNDISPLAY D_WATCH D_UNWATCH
%token D_DUMP D_TRACE
%token D_INT D_STRING D_NODE D_VARIABLE
%token D_OPTION D_COMMANDS D_END D_SILENT D_SOURCE
%token D_SAVE D_EVAL D_CONDITION
%token D_STATEMENT

%%

input
	: /* empty */
	| input line
	  {
		cmd_idx = -1;
		want_nodeval = false;
		if (lexptr_begin != NULL) {
			if (input_from_tty && lexptr_begin[0] != '\0')
				add_history(lexptr_begin);
			efree(lexptr_begin);
			lexptr_begin = NULL;
		}
		if (arg_list != NULL) {
			free_cmdarg(arg_list);
			arg_list = NULL;
		}
	  }
	;

line
	: nls
	| command nls
	  {
		if (dbg_errcount == 0 && cmd_idx >= 0) {
			Func_cmd cmdfunc;
			bool terminate = false;
			CMDARG *args;
			int ctype = 0;

			ctype = cmdtab[cmd_idx].type;

			/* a blank line repeats previous command
			 * (list, next, nexti, step, stepi and continue without arguments).
			 * save the index in the command table; used in yylex
			 */
			if ((ctype == D_list
					|| ctype == D_next
					|| ctype == D_step
					|| ctype == D_nexti
					|| ctype == D_stepi
					|| ctype == D_continue)
				&& arg_list == NULL
				&& ! in_commands
				&& input_from_tty
			)
				repeat_idx = cmd_idx;
			else
				repeat_idx = -1;

			/* call the command handler; reset the globals arg_list, cmd_idx,
			 * since this handler could invoke yyparse again.
			 * call do_commands for the list of commands in `commands';
			 * arg_list isn't freed on return.
			 */

			cmdfunc = cmdtab[cmd_idx].cf_ptr;
			if (in_commands)
				cmdfunc = do_commands;
			cmd_idx = -1;
			want_nodeval = false;

			args = arg_list;
			arg_list = NULL;

			terminate = (*cmdfunc)(args, ctype);
			if (! in_commands || ctype == D_commands)
				free_cmdarg(args);
			if (terminate)
				YYACCEPT;
		}
	  }
	| error nls
	  {
		yyerrok;
	  }
	;

control_cmd
	: D_CONTINUE
	| D_NEXT
	| D_NEXTI
	| D_STEP
	| D_STEPI
	;

d_cmd
	: D_UNDISPLAY
	| D_UNWATCH
	| D_DISABLE
	| D_DELETE
	;

frame_cmd
	: D_UP
	| D_DOWN
	| D_BACKTRACE
	| D_FRAME
	;

break_cmd
	: D_BREAK
	| D_TBREAK
	;

/* mid-rule action buried in non-terminal to avoid conflict */
set_want_nodeval
	: { want_nodeval = true; }
	;

eval_prologue
	: D_EVAL set_want_nodeval opt_param_list nls
	  {
		if (dbg_errcount == 0) {
			/* don't free arg_list;	passed on to statement_list
			 * non-terminal (empty rule action). See below.
			 */
			if (input_from_tty) {
				dbg_prompt = eval_prompt;
				fprintf(out_fp,
		_("Type (g)awk statement(s). End with the command `end'\n"));
				rl_inhibit_completion = 1;
			}
			cmd_idx = -1;
			in_eval = true;
		}
	  }
	;

statement_list
	: /* empty */
	  {
		$$ = append_statement(arg_list, (char *) start_EVAL);
		if (read_a_line == read_commands_string)	/* unserializing 'eval' in 'commands' */
			$$->a_string[0] = '\0';
		free_cmdarg(arg_list);
		arg_list = NULL;
	  }
	| statement_list D_STATEMENT { $$ = append_statement($1, lexptr_begin); } nls
	  {
		$$ = $3;
	  }
	;

eval_cmd
	: eval_prologue statement_list D_END
	  {
		arg_list = append_statement($2, (char *) end_EVAL);
		if (read_a_line == read_commands_string) {	/* unserializing 'eval' in 'commands' */
			char *str = arg_list->a_string;
			size_t len = strlen(str);
			assert(len > 2 && str[len - 2] == '}');
			str[len - 2] = '\0';
		}
		if (input_from_tty) {
			dbg_prompt = in_commands ? commands_prompt : dgawk_prompt;
			rl_inhibit_completion = 0;
		}
		cmd_idx = find_command("eval", 4);
		in_eval = false;
	  }
	| D_EVAL set_want_nodeval string_node
	  {
		NODE *n;
		CMDARG *arg;
		n = $3->a_node;
		arg = append_statement(NULL, (char *) start_EVAL);
		(void) append_statement(arg, n->stptr);
		(void) append_statement(arg, (char *) end_EVAL);
		free_cmdarg(arg_list);
		arg_list = arg;
	  }
	;

command
	: D_HELP help_args
	| D_QUIT
	| D_RUN
	| D_FINISH
	| control_cmd opt_plus_integer
	| frame_cmd opt_integer
	  {
		if (cmdtab[cmd_idx].class == D_FRAME
				&& $2 != NULL && $2->a_int < 0)
			yyerror(_("invalid frame number: %d"), $2->a_int);
	  }
	| D_INFO D_STRING
	  {
		int idx = find_argument($2);
		if (idx < 0)
			yyerror(_("info: invalid option - `%s'"), $2->a_string);
		else {
			efree($2->a_string);
			$2->a_string = NULL;
			$2->type = D_argument;
			$2->a_argument = argtab[idx].value;
		}
	  }
	| D_IGNORE plus_integer D_INT
	| D_ENABLE enable_args
	| D_PRINT { want_nodeval = true; } print_args
	| D_PRINTF { want_nodeval = true; } printf_args
	| D_LIST list_args
	| D_UNTIL location
	| D_CLEAR location
	| break_cmd break_args
	| D_SET { want_nodeval = true; } variable '=' node
	| D_OPTION option_args
	| D_RETURN { want_nodeval = true; } opt_node
	| D_DISPLAY { want_nodeval = true; } opt_variable
	| D_WATCH { want_nodeval = true; } variable condition_exp
	| d_cmd opt_integer_list
	| D_DUMP opt_string
	| D_SOURCE D_STRING
	  {
		if (in_cmd_src($2->a_string))
			yyerror(_("source: `%s': already sourced."), $2->a_string);
	  }
	| D_SAVE D_STRING
	  {
		if (! input_from_tty)
			yyerror(_("save: `%s': command not permitted."), $2->a_string);
	  }
	| D_COMMANDS commands_arg
	  {
		int type = 0;
		int num;

		if ($2 != NULL)
			num = $2->a_int;

		if (dbg_errcount != 0)
			;
		else if (in_commands)
			yyerror(_("cannot use command `commands' for breakpoint/watchpoint commands"));
		else if ($2 == NULL &&  ! (type = has_break_or_watch_point(&num, true)))
			yyerror(_("no breakpoint/watchpoint has been set yet"));
		else if ($2 != NULL && ! (type = has_break_or_watch_point(&num, false)))
			yyerror(_("invalid breakpoint/watchpoint number"));
		if (type) {
			in_commands = true;
			if (input_from_tty) {
				dbg_prompt = commands_prompt;
				fprintf(out_fp, _("Type commands for when %s %d is hit, one per line.\n"),
								(type == D_break) ? "breakpoint" : "watchpoint", num);
				fprintf(out_fp, _("End with the command `end'\n"));
			}
		}
	  }
	| D_END
	  {
		if (! in_commands)
			yyerror(_("`end' valid only in command `commands' or `eval'"));
		else {
			if (input_from_tty)
				dbg_prompt = dgawk_prompt;
			in_commands = false;
		}
	  }
	| D_SILENT
	  {
		if (! in_commands)
			yyerror(_("`silent' valid only in command `commands'"));
	  }
	| D_TRACE D_STRING
	  {
		int idx = find_argument($2);
		if (idx < 0)
			yyerror(_("trace: invalid option - `%s'"), $2->a_string);
		else {
			efree($2->a_string);
			$2->a_string = NULL;
			$2->type = D_argument;
			$2->a_argument = argtab[idx].value;
		}
	  }
	| D_CONDITION plus_integer { want_nodeval = true; } condition_exp
	  {
		int type;
		int num = $2->a_int;
		type = has_break_or_watch_point(&num, false);
		if (! type)
			yyerror(_("condition: invalid breakpoint/watchpoint number"));
	  }
	| eval_cmd
	  {
		if (in_commands) {
			/* Prepend command 'eval' to argument list */
			CMDARG *arg;
			arg = mk_cmdarg(D_string);
			arg->a_string = estrdup("eval", 4);
			arg->next = arg_list;
			arg_list = arg;
		}
	  }
	;

condition_exp
	: opt_string_node
	  {
		if ($1 != NULL) {
			NODE *n = $1->a_node;
			$1->type = D_string;
			$1->a_string = n->stptr;
			freenode(n);
		}
		$$ = $1;
	  }
	;

commands_arg
	: opt_plus_integer
	| error
	  {	$$ = NULL; }
	;

opt_param_list
	: /* empty */
	  { $$ = NULL; }
	| param_list
	;

param_list
	: D_VARIABLE
	| param_list D_VARIABLE
	| param_list ',' D_VARIABLE
	| error
	  { $$ = NULL; }
	;

opt_string_node
	: /* empty */
	  { $$ = NULL; }
	| string_node
	| error
	  { $$ = NULL; }
	;

string_node
	: D_NODE
	  {
		NODE *n;
		n = $1->a_node;
		if ((n->flags & STRING) == 0)
			yyerror(_("argument not a string"));
	  }
	;

option_args
	: /* empty */
	  { $$ = NULL; }
	| D_STRING
	  {
		if (find_option($1->a_string) < 0)
			yyerror(_("option: invalid parameter - `%s'"), $1->a_string);
 	  }
	| D_STRING '=' D_STRING
	  {
		if (find_option($1->a_string) < 0)
			yyerror(_("option: invalid parameter - `%s'"), $1->a_string);
 	  }
	;

func_name
	: D_STRING
	  {
		NODE *n;
		n = lookup($1->a_string);
		if (n == NULL || n->type != Node_func)
			yyerror(_("no such function - `%s'"), $1->a_string);
		else {
			$1->type = D_func;
			efree($1->a_string);
			$1->a_string = NULL;
			$1->a_node = n;
		}
	  }
	;

location
	: /* empty */
	  { $$ = NULL; }
	| plus_integer
	| func_name
	| D_STRING ':' plus_integer
	| D_STRING ':' func_name
	;

break_args
	: /* empty */
	  { $$ = NULL; }
	| plus_integer { want_nodeval = true; } condition_exp
	| func_name
	| D_STRING ':' plus_integer { want_nodeval = true; } condition_exp
	| D_STRING ':' func_name
	;

opt_variable
	: /* empty */
	  { $$ = NULL; }
	| variable
	;

opt_string
	: /* empty */
	  { $$ = NULL; }
	| D_STRING
	;

opt_node
	: /* empty */
	  { $$ = NULL; }
	| node
	;

help_args
	: /* empty */
	| D_STRING
	;

enable_args
	: opt_integer_list
	| D_STRING opt_integer_list
	  {
		int idx = find_argument($1);
		if (idx < 0)
			yyerror(_("enable: invalid option - `%s'"), $1->a_string);
		else {
			efree($1->a_string);
			$1->a_string = NULL;
			$1->type = D_argument;
			$1->a_argument = argtab[idx].value;
		}
	  }
	;

print_exp
	: variable
	| '@' D_VARIABLE
	  {
		$2->type = D_array;	/* dump all items */
		$2->a_count = 0;
	  }
	| '@' D_VARIABLE subscript_list /* dump sub-array items*/
	  {
		$2->type = D_array;
		$2->a_count = num_dim;
	  }
	;

print_args
	: print_exp
	| print_args print_exp
	| print_args ',' print_exp
	| error
	;

printf_exp
	: D_NODE
	| variable
	;

printf_args
	: printf_exp
	| printf_args ',' printf_exp
	| error
	;

list_args
	: /* empty */
	  { $$ = NULL; }
	| '+'
	  { $$ = NULL; }
	| '-'
	  {
		CMDARG *a;
		a = mk_cmdarg(D_int);
		a->a_int = -1;
		append_cmdarg(a);
	  }
	| plus_integer
	| func_name
	| integer_range
	| D_STRING ':' plus_integer
	| D_STRING ':' func_name
	| D_STRING ':' integer_range
	;

integer_range
	: plus_integer '-' plus_integer
	  {
		if ($1->a_int > $3->a_int)
			yyerror(_("invalid range specification: %d - %d"),
				$1->a_int, $3->a_int);
		else
			$1->type = D_range;
		$$ = $1;
	  }
	;

opt_integer_list
	: /* empty */
	  { $$ = NULL; }
	| integer_list
	| error
	;

integer_list
	: plus_integer
	| integer_range
	| integer_list plus_integer
	| integer_list integer_range
	;

exp_list
	: node
	  { $$ = $1; }
	| exp_list ',' node
	  { $$ = $1; }
	| error
	;

subscript
	: '[' exp_list ']'
	  {
		CMDARG *a;
		NODE *subs;
		int count = 0;

		for (a = $2; a != NULL; a = a->next)
			count++;
		subs = concat_args($2, count);
		free_cmdarg($2->next);
		$2->next = NULL;
		$2->type = D_node;
		$2->a_node = subs;
		$$ = $2;
	  }
	| '[' exp_list error
	;

subscript_list
	: subscript
	  { $$ = $1; num_dim = 1; }
	| subscript_list subscript
	  {	$$ = $1; num_dim++; }
	;

variable
	: D_VARIABLE
	| '$' D_NODE
	  {
		NODE *n = $2->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value for field number"));
		else
			$2->type = D_field;
		$$ = $2;
	  }
	| D_VARIABLE subscript_list
	  {
		/* a_string is array name, a_count is dimension count */
		$1->type = D_subscript;
		$1->a_count = num_dim;
		$$ = $1;
	  }
	;

node
	: D_NODE
	  { $$ = $1; }
	| '+' D_NODE
	  {
		NODE *n = $2->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		$$ = $2;
	  }
	| '-' D_NODE
	  {
		NODE *n = $2->a_node;
		if ((n->flags & NUMBER) == 0)
			yyerror(_("non-numeric value found, numeric expected"));
		else
			negate_num(n);
		$$ = $2;
	  }
	;

opt_plus_integer
	: /* empty */
	  { $$ = NULL; }
	| plus_integer
	  { $$ = $1; }
	;

opt_integer
	: /* empty */
	  { $$ = NULL; }
	| integer
	  { $$ = $1; }
	;

plus_integer
	: D_INT
	  {
		if ($1->a_int == 0)
			yyerror(_("non-zero integer value"));
		$$ = $1;
	  }
	| '+' D_INT
	  {
		if ($2->a_int == 0)
			yyerror(_("non-zero integer value"));
		$$ = $2;
	  }
	;

integer
	: D_INT
	  { $$ = $1; }
	| '+' D_INT
	  { $$ = $2; }
	| '-' D_INT
	  {
		$2->a_int = - $2->a_int;
		$$ = $2;
	  }
	;

nls
	: '\n'
	  {
		if (lexptr_begin != NULL) {
			if (input_from_tty && lexptr_begin[0] != '\0')
				add_history(lexptr_begin);
			efree(lexptr_begin);
			lexptr_begin = NULL;
		}
	  }
	;

%%


/* append_statement --- append 'stmt' to the list of eval awk statements */

static CMDARG *
append_statement(CMDARG *stmt_list, char *stmt)
{
	CMDARG *a, *arg;
	char *s;
	int len, slen, ssize;

#define EVALSIZE	512

	if (stmt == start_EVAL) {
		len = sizeof(start_EVAL);
		for (a = stmt_list; a != NULL; a = a->next)
			len += strlen(a->a_string) + 1;	/* 1 for ',' */
		len += EVALSIZE;

		emalloc(s, char *, (len + 1) * sizeof(char), "append_statement");
		arg = mk_cmdarg(D_string);
		arg->a_string = s;
		arg->a_count = len;	/* kludge */

		slen = sizeof("function @eval(") - 1;
		memcpy(s, start_EVAL, slen);

		for (a = stmt_list; a != NULL; a = a->next) {
			len = strlen(a->a_string);
			memcpy(s + slen, a->a_string, len);
			slen += len;
			if (a->next != NULL)
				s[slen++] = ',';
		}
		s[slen++] = ')';
		s[slen++] = '{';
		s[slen] = '\0';
		return arg;
	}

	len = strlen(stmt) + 1;	/* 1 for newline */
	s = stmt_list->a_string;
	slen = strlen(s);
	ssize = stmt_list->a_count;
	if (len > ssize - slen) {
		ssize = slen + len + EVALSIZE;
		erealloc(s, char *, (ssize + 1) * sizeof(char), "append_statement");
		stmt_list->a_string = s;
		stmt_list->a_count = ssize;
	}
	memcpy(s + slen, stmt, len);
	slen += len;
	if (slen >= 2 && s[slen - 2] != '\n') {
		s[slen - 1] = '\n';
		s[slen] = '\0';
	}

	if (stmt == end_EVAL)
		erealloc(stmt_list->a_string, char *, slen + 1, "append_statement");
	return stmt_list;

#undef EVALSIZE
}


/* command names sorted in ascending order */

struct cmdtoken cmdtab[] = {
{ "backtrace", "bt", D_backtrace, D_BACKTRACE, do_backtrace,
	gettext_noop("backtrace [N] - print trace of all or N innermost (outermost if N < 0) frames.") },
{ "break", "b", D_break, D_BREAK, do_breakpoint,
	gettext_noop("break [[filename:]N|function] - set breakpoint at the specified location.") },
{ "clear", "", D_clear, D_CLEAR, do_clear,
	gettext_noop("clear [[filename:]N|function] - delete breakpoints previously set.") },
{ "commands", "", D_commands, D_COMMANDS, do_commands,
	gettext_noop("commands [num] - starts a list of commands to be executed at a breakpoint(watchpoint) hit.") },
{ "condition", "", D_condition, D_CONDITION, do_condition,
	gettext_noop("condition num [expr] - set or clear breakpoint or watchpoint condition.") },
{ "continue", "c", D_continue, D_CONTINUE, do_continue,
	gettext_noop("continue [COUNT] - continue program being debugged.") },
{ "delete", "d", D_delete, D_DELETE, do_delete_breakpoint,
	gettext_noop("delete [breakpoints] [range] - delete specified breakpoints.") },
{ "disable", "", D_disable, D_DISABLE, do_disable_breakpoint,
	gettext_noop("disable [breakpoints] [range] - disable specified breakpoints.") },
{ "display", "", D_display, D_DISPLAY, do_display,
	gettext_noop("display [var] - print value of variable each time the program stops.") },
{ "down", "", D_down, D_DOWN, do_down,
	gettext_noop("down [N] - move N frames down the stack.") },
{ "dump", "", D_dump, D_DUMP, do_dump_instructions,
	gettext_noop("dump [filename] - dump instructions to file or stdout.") },
{ "enable", "e", D_enable, D_ENABLE, do_enable_breakpoint,
	gettext_noop("enable [once|del] [breakpoints] [range] - enable specified breakpoints.") },
{ "end", "", D_end, D_END, do_commands,
	gettext_noop("end - end a list of commands or awk statements.") },
{ "eval", "", D_eval, D_EVAL, do_eval,
	gettext_noop("eval stmt|[p1, p2, ...] - evaluate awk statement(s).") },
{ "exit", "", D_quit, D_QUIT, do_quit,
	gettext_noop("exit - (same as quit) exit debugger.") },
{ "finish", "", D_finish, D_FINISH, do_finish,
	gettext_noop("finish - execute until selected stack frame returns.") },
{ "frame", "f", D_frame, D_FRAME, do_frame,
	gettext_noop("frame [N] - select and print stack frame number N.") },
{ "help", "h", D_help, D_HELP, do_help,
	gettext_noop("help [command] - print list of commands or explanation of command.") },
{ "ignore", "", D_ignore, D_IGNORE, do_ignore_breakpoint,
	gettext_noop("ignore N COUNT - set ignore-count of breakpoint number N to COUNT.") },
{ "info", "i", D_info, D_INFO, do_info,
	gettext_noop("info topic - source|sources|variables|functions|break|frame|args|locals|display|watch.") },
{ "list", "l", D_list, D_LIST, do_list,
	gettext_noop("list [-|+|[filename:]lineno|function|range] - list specified line(s).") },
{ "next", "n", D_next, D_NEXT, do_next,
	gettext_noop("next [COUNT] - step program, proceeding through subroutine calls.") },
{ "nexti", "ni", D_nexti, D_NEXTI, do_nexti,
	gettext_noop("nexti [COUNT] - step one instruction, but proceed through subroutine calls.") },
{ "option", "o", D_option, D_OPTION, do_option,
	gettext_noop("option [name[=value]] - set or display debugger option(s).") },
{ "print", "p", D_print, D_PRINT, do_print_var,
	gettext_noop("print var [var] - print value of a variable or array.") },
{ "printf", "", D_printf, D_PRINTF, do_print_f,
	gettext_noop("printf format, [arg], ... - formatted output.") },
{ "quit", "q", D_quit, D_QUIT, do_quit,
	gettext_noop("quit - exit debugger.") },
{ "return", "", D_return, D_RETURN, do_return,
	gettext_noop("return [value] - make selected stack frame return to its caller.") },
{ "run", "r", D_run, D_RUN, do_run,
	gettext_noop("run - start or restart executing program.") },
#ifdef HAVE_LIBREADLINE
{ "save", "", D_save, D_SAVE, do_save,
	gettext_noop("save filename - save commands from the session to file.") },
#endif
{ "set", "", D_set, D_SET, do_set_var,
	gettext_noop("set var = value - assign value to a scalar variable.") },
{ "silent", "", D_silent, D_SILENT, do_commands,
	gettext_noop("silent - suspends usual message when stopped at a breakpoint/watchpoint.") },
{ "source", "", D_source, D_SOURCE, do_source,
	gettext_noop("source file - execute commands from file.") },
{ "step", "s", D_step, D_STEP, do_step,
	gettext_noop("step [COUNT] - step program until it reaches a different source line.") },
{ "stepi", "si", D_stepi, D_STEPI, do_stepi,
	gettext_noop("stepi [COUNT] - step one instruction exactly.") },
{ "tbreak", "t", D_tbreak, D_TBREAK, do_tmp_breakpoint,
	gettext_noop("tbreak [[filename:]N|function] - set a temporary breakpoint.") },
{ "trace", "", D_trace, D_TRACE, do_trace_instruction,
	gettext_noop("trace on|off - print instruction before executing.") },
{ "undisplay",	"", D_undisplay, D_UNDISPLAY, do_undisplay,
	gettext_noop("undisplay [N] - remove variable(s) from automatic display list.") },
{ "until", "u", D_until, D_UNTIL, do_until,
	gettext_noop("until [[filename:]N|function] - execute until program reaches a different line or line N within current frame.") },
{ "unwatch", "", D_unwatch, D_UNWATCH, do_unwatch,
	gettext_noop("unwatch [N] - remove variable(s) from watch list.") },
{ "up",	"", D_up, D_UP, do_up,
	gettext_noop("up [N] - move N frames up the stack.") },
{ "watch", "w", D_watch, D_WATCH, do_watch,
	gettext_noop("watch var - set a watchpoint for a variable.") },
{ "where", "", D_backtrace, D_BACKTRACE, do_backtrace,
	gettext_noop("where [N] - (same as backtrace) print trace of all or N innermost (outermost if N < 0) frames.") },
{ NULL, NULL, D_illegal, 0, (Func_cmd) 0,
	 NULL },
};

struct argtoken argtab[] = {
	{ "args", D_info, A_ARGS },
	{ "break", D_info, A_BREAK },
	{ "del", D_enable, A_DEL },
	{ "display", D_info, A_DISPLAY },
	{ "frame", D_info, A_FRAME },
	{ "functions", D_info, A_FUNCTIONS },
	{ "locals", D_info, A_LOCALS },
	{ "off", D_trace, A_TRACE_OFF },
	{ "on", D_trace, A_TRACE_ON },
	{ "once", D_enable, A_ONCE },
	{ "source", D_info, A_SOURCE },
	{ "sources", D_info, A_SOURCES },
	{ "variables", D_info, A_VARIABLES },
	{ "watch", D_info, A_WATCH },
	{ NULL, D_illegal, A_NONE },
};


/* get_command --- return command handler function */

Func_cmd
get_command(int ctype)
{
	int i;
	for (i = 0; cmdtab[i].name != NULL; i++) {
		if (cmdtab[i].type == ctype)
			return cmdtab[i].cf_ptr;
	}
	return (Func_cmd) 0;
}

/* get_command_name --- return command name given it's type */

const char *
get_command_name(int ctype)
{
	int i;
	for (i = 0; cmdtab[i].name != NULL; i++) {
		if (cmdtab[i].type == ctype)
			return cmdtab[i].name;
	}
	return NULL;
}

/* mk_cmdarg --- make an argument for command */

static CMDARG *
mk_cmdarg(enum argtype type)
{
	CMDARG *arg;
	ezalloc(arg, CMDARG *, sizeof(CMDARG), "mk_cmdarg");
	arg->type = type;
	return arg;
}

/* append_cmdarg --- append ARG to the list of arguments for the current command */

static void
append_cmdarg(CMDARG *arg)
{
	static CMDARG *savetail;

	if (arg_list == NULL)
		arg_list = arg;
	else
		savetail->next = arg;
	savetail = arg;
}

/* free_cmdarg --- free all arguments in LIST */

void
free_cmdarg(CMDARG *list)
{
	CMDARG *arg, *nexta;

	for (arg = list; arg != NULL; arg = nexta) {
		nexta = arg->next;

		switch (arg->type) {
		case D_variable:
		case D_subscript:
		case D_array:
		case D_string:
			if (arg->a_string != NULL)
				efree(arg->a_string);
			break;
		case D_node:
		case D_field:
			unref(arg->a_node);
			break;
		default:
			break;
		}
		efree(arg);
	}
}

/* yyerror --- print a syntax error message */

static void
yyerror(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	fprintf(out_fp, _("error: "));
	vfprintf(out_fp, mesg, args);
	fprintf(out_fp, "\n");
	va_end(args);
	dbg_errcount++;
	repeat_idx = -1;
}


/* yylex --- read a command and turn it into tokens */

static int
#ifdef USE_EBCDIC
yylex_ebcdic(void)
#else
yylex(void)
#endif
{
	static char *lexptr = NULL;
	static char *lexend;
	int c;
	char *tokstart;
	size_t toklen;

	yylval = (CMDARG *) NULL;

	if (dbg_errcount > 0 && lexptr_begin == NULL) {
		/* fake a new line */
		dbg_errcount = 0;
		return '\n';
	}

	if (lexptr_begin == NULL) {
again:
		lexptr_begin = read_a_line(dbg_prompt);
		if (lexptr_begin == NULL) {	/* EOF or error */
			if (get_eof_status() == EXIT_FATAL)
				exit(EXIT_FATAL);
			if (get_eof_status() == EXIT_FAILURE) {
				static int seen_eof = 0;

				/* force a quit, and let do_quit (in debug.c) exit */
				if (! seen_eof) {
					if (errno != 0)	{
						fprintf(stderr, _("cannot read command: %s\n"), strerror(errno));
						exit_val = EXIT_FAILURE;
					} /* else
						exit_val = EXIT_SUCCESS; */

					seen_eof = 1;
					return '\n';	/* end current command if any */
				} else if (seen_eof++ == 1) {
					cmd_idx = find_command("quit", 4);
					return D_QUIT;	/* 'quit' token */
				} else
					return '\n';	/* end command 'quit' */
			}
			if (errno != 0)
				d_error(_("cannot read command: %s"), strerror(errno));
			if (pop_cmd_src() == 0)
				goto again;
			exit(EXIT_FATAL);	/* shouldn't happen */
		}

		if (! in_commands && ! in_eval	/* history expansion off in 'commands' and 'eval' */
				&& input_from_tty
		)
			history_expand_line(&lexptr_begin);

		lexptr = lexptr_begin;
		lexend = lexptr + strlen(lexptr);
		if (*lexptr == '\0'		/* blank line */
				&& repeat_idx >= 0
				&& input_from_tty
				&& ! in_eval
		) {
#ifdef HAVE_LIBREADLINE
			HIST_ENTRY *h;
			h = previous_history();
			if (h != NULL)
				add_history(h->line);
#endif
			cmd_idx = repeat_idx;
			return cmdtab[cmd_idx].class;	/* repeat last command */
		}
		repeat_idx = -1;
	}

	c = *lexptr;

	while (c == ' ' || c == '\t')
		c = *++lexptr;

	if (! input_from_tty && c == '#')
		return '\n';

	tokstart = lexptr;
	if (lexptr >= lexend)
		return '\n';

	if (cmd_idx < 0) {	/* need a command */
		if (c == '?' && tokstart[1] == '\0'	&& ! in_eval) {
			lexptr++;
			cmd_idx = find_command("help", 4);
			return D_HELP;
		}

		while (c != '\0' && c != ' ' && c != '\t') {
			if (! is_alpha(c) && ! in_eval) {
				yyerror(_("invalid character in command"));
				return '\n';
			}
			c = *++lexptr;
		}

		toklen = lexptr - tokstart;

		if (in_eval) {
			if (toklen == 3
					&& tokstart[3] == '\0'
					&& tokstart[0] == 'e'
					&& tokstart[1] == 'n'
					&& tokstart[2] == 'd'
			) {
				cmd_idx = find_command(tokstart, toklen);
				return D_END;
			}
			lexptr = lexend;
			return D_STATEMENT;
		}

		cmd_idx = find_command(tokstart, toklen);
		if (cmd_idx >= 0) {
			if (in_commands && cmdtab[cmd_idx].type != D_eval) {
				/* add the actual command string (lexptr_begin) to
				 * arg_list; command string for 'eval' prepended to the arg_list
				 * in the grammer above (see eval_cmd non-terminal).
				 */
				CMDARG *arg;
				arg = mk_cmdarg(D_string);
				arg->a_string = estrdup(lexptr_begin, lexend - lexptr_begin);
				append_cmdarg(arg);
			}
			return cmdtab[cmd_idx].class;
		} else {
			yyerror(_("unknown command - `%.*s', try help"), toklen, tokstart);
			return '\n';
		}
	}

	c = *lexptr;

	if (cmdtab[cmd_idx].type == D_option) {
		if (c == '=')
			return *lexptr++;
	} else if (c == '-' || c == '+' || c == ':' || c == '|')
		return *lexptr++;

	if (c == '"') {
		char *str, *p;
		int flags = ALREADY_MALLOCED;
		bool esc_seen = false;

		toklen = lexend - lexptr;
		emalloc(str, char *, toklen + 1, "yylex");
		p = str;

		while ((c = *++lexptr) != '"') {
			if (lexptr == lexend) {
err:
				efree(str);
				yyerror(_("unterminated string"));
				return '\n';
			}
			if (c == '\\') {
				c = *++lexptr;
				esc_seen = true;
				if (want_nodeval || c != '"')
					*p++ = '\\';
			}
			if (lexptr == lexend)
				goto err;
			*p++ = c;
		}
		lexptr++;
		*p = '\0';

		if (! want_nodeval) {
			yylval = mk_cmdarg(D_string);
			yylval->a_string = str;
			append_cmdarg(yylval);
			return D_STRING;
		} else {	/* awk string */
			if (esc_seen)
				flags |= SCAN;
			yylval = mk_cmdarg(D_node);
			yylval->a_node = make_str_node(str, p - str, flags);
			append_cmdarg(yylval);
			return D_NODE;
		}
	}

	if (! want_nodeval) {
		while ((c = *++lexptr) != '\0' && c != ':' && c != '-'
					&& c != ' ' && c != '\t' && c != '=')
			;

		/* Is it an integer? */
		if (isdigit((unsigned char) tokstart[0]) && cmdtab[cmd_idx].type != D_option) {
			char *end;
			long l;

			errno = 0;
			l = strtol(tokstart, &end, 0);
			if (errno != 0) {
				yyerror(_("%s"), strerror(errno));
				errno = 0;
				return '\n';
			}

			if (lexptr == end) {
				yylval = mk_cmdarg(D_int);
				yylval->a_int = l;
				append_cmdarg(yylval);
				return D_INT;
			}
		}

		/* Must be string */
		yylval = mk_cmdarg(D_string);
		yylval->a_string = estrdup(tokstart, lexptr - tokstart);
		append_cmdarg(yylval);
		return D_STRING;
	}

	/* look for awk number */

	if (isdigit((unsigned char) tokstart[0])) {
		NODE *r = NULL;

		errno = 0;
#ifdef HAVE_MPFR
		if (do_mpfr) {
			int tval;
			r = mpg_float();
			tval = mpfr_strtofr(r->mpg_numbr, tokstart, & lexptr, 0, ROUND_MODE);
			IEEE_FMT(r->mpg_numbr, tval);
			if (mpfr_integer_p(r->mpg_numbr)) {
				/* integral value, convert to a GMP type. */
				NODE *tmp = r;
				r = mpg_integer();
				mpfr_get_z(r->mpg_i, tmp->mpg_numbr, MPFR_RNDZ);
				unref(tmp);
			}
		} else
#endif
			r = make_number(strtod(tokstart, & lexptr));

		if (errno != 0) {
			yyerror(strerror(errno));
			unref(r);
			errno = 0;
			return '\n';
		}
		yylval = mk_cmdarg(D_node);
		yylval->a_node = r;
		append_cmdarg(yylval);
		return D_NODE;
	}

	c = *lexptr;
	if (c == '$' || c == '@'
			|| c == '[' || c == ']'
			|| c == ',' || c == '=')
		return *lexptr++;

	if (! is_letter(c)) {
		yyerror(_("invalid character"));
		return '\n';
	}

	while (is_identchar(c))
		c = *++lexptr;
	toklen = lexptr - tokstart;

	/* awk variable */
	yylval = mk_cmdarg(D_variable);
	yylval->a_string = estrdup(tokstart, toklen);
	append_cmdarg(yylval);
	return D_VARIABLE;
}

/* Convert single-character tokens coming out of yylex() from EBCDIC to
   ASCII values on-the-fly so that the parse tables need not be regenerated
   for EBCDIC systems.  */
#ifdef USE_EBCDIC
static int
yylex(void)
{
	static char etoa_xlate[256];
	static int do_etoa_init = 1;
	int tok;

	if (do_etoa_init)
	{
		for (tok = 0; tok < 256; tok++)
			etoa_xlate[tok] = (char) tok;
#ifdef HAVE___ETOA_L
		/* IBM helpfully provides this function.  */
		__etoa_l(etoa_xlate, sizeof(etoa_xlate));
#else
# error "An EBCDIC-to-ASCII translation function is needed for this system"
#endif
		do_etoa_init = 0;
	}

	tok = yylex_ebcdic();

	if (tok >= 0 && tok <= 0xFF)
		tok = etoa_xlate[tok];

	return tok;
}
#endif /* USE_EBCDIC */

/* find_argument --- find index in 'argtab' for a command option */

static int
find_argument(CMDARG *arg)
{
	/* non-number argument */
	int idx;
	char *name, *p;
	size_t len;
	assert(cmd_idx >= 0);
	name = arg->a_string;
	len = strlen(name);
	for (idx = 0; (p = (char *) argtab[idx].name) != NULL; idx++) {
		if (cmdtab[cmd_idx].type == argtab[idx].cmd
				&& *p == *name
				&& strlen(p) == len
				&& strncmp(p, name, len) == 0
		)
			return idx;
	}
	return -1;	/* invalid option */
}

/* concat_args --- concatenate argument strings into a single string NODE */

static NODE *
concat_args(CMDARG *arg, int count)
{
	NODE *n;
	NODE **tmp;
	char *str, *subsep, *p;
	long len, subseplen;
	int i;

	if (count == 1) {
		n = force_string(arg->a_node);
		return dupnode(n);
	}

	emalloc(tmp, NODE **, count * sizeof(NODE *), "concat_args");
	subseplen = SUBSEP_node->var_value->stlen;
	subsep = SUBSEP_node->var_value->stptr;
	len = -subseplen;

	for (i = 0; i < count; i++) {
		n = force_string(arg->a_node);
		len += n->stlen + subseplen;
		tmp[i] = n;
		arg = arg->next;
	}

	emalloc(str, char *, len + 1, "concat_args");
	n = tmp[0];
	memcpy(str, n->stptr, n->stlen);
	p = str + n->stlen;
	for (i = 1; i < count; i++) {
		if (subseplen == 1)
			*p++ = *subsep;
		else if (subseplen > 0) {
			memcpy(p, subsep, subseplen);
			p += subseplen;
		}

		n = tmp[i];
		memcpy(p, n->stptr, n->stlen);
		p += n->stlen;
	}
	str[len] = '\0';
	efree(tmp);
	return make_str_node(str, len, ALREADY_MALLOCED);
}

/* find_command --- find the index in 'cmdtab' using exact,
 *                  abbreviation or unique partial match
 */

static int
find_command(const char *token, size_t toklen)
{
	char *name, *abrv;
	int i, k;
	bool try_exact = true;
	int abrv_match = -1;
	int partial_match = -1;

#ifdef USE_EBCDIC
	/* make sure all lower case characters in token (sorting
	 * isn't the solution in this case)
	 */
	for (i = 0; i < toklen; i++) {
		if (token[i] != tolower(token[i]))
			return -1;
	}
#endif

	k = sizeof(cmdtab)/sizeof(cmdtab[0]) - 1;
	for (i = 0; i < k; i++) {
		name = (char *) cmdtab[i].name;
		if (try_exact && *token == *name
				&& toklen == strlen(name)
				&& strncmp(name, token, toklen) == 0
		)
			return i;

		if (*name > *token || i == (k - 1))
			try_exact = false;

		if (abrv_match < 0) {
			abrv = cmdtab[i].abbrvn;
			if (abrv[0] == token[0]) {
				if (toklen == 1 && ! abrv[1])
					abrv_match = i;
				else if (toklen == 2 && abrv[1] == token[1])
					abrv_match = i;
			}
		}
		if (! try_exact && abrv_match >= 0)
			return abrv_match;
		if (partial_match < 0) {
			if (*token == *name
					&& toklen < strlen(name)
					&& strncmp(name, token, toklen) == 0
			) {
				if ((i == k - 1 || strncmp(cmdtab[i + 1].name, token, toklen) != 0)
					&& (i == 0 || strncmp(cmdtab[i - 1].name, token, toklen) != 0)
				)
					partial_match = i;
			}
		}
	}
	return partial_match;
}

/* do_help -- help command */

int
do_help(CMDARG *arg, int cmd)
{
	int i;
	if (arg == NULL) {
		initialize_pager(out_fp);
		if (setjmp(pager_quit_tag) == 0) {
			for (i = 0; cmdtab[i].name != NULL; i++) {
				gprintf(out_fp, "%s:\n", cmdtab[i].name);
				gprintf(out_fp, "\t%s\n", _(cmdtab[i].help_txt));
			}
		}
	} else if (arg->type == D_string) {
		char *name;
		name = arg->a_string;
		i = find_command(name, strlen(name));
		if (i >= 0) {
			fprintf(out_fp, "%s\n", cmdtab[i].help_txt);
			if (strcmp(cmdtab[i].name, "option") == 0)
				option_help();
		} else
			fprintf(out_fp, _("undefined command: %s\n"), name);
	}

	return false;
}


#ifdef HAVE_LIBREADLINE

/* next_word --- find the next word in a line to complete
 *               (word seperation characters are space and tab).
 */

static char *
next_word(char *p, int len, char **endp)
{
	char *q;
	int i;

	if (p == NULL || len <= 0)
		return NULL;
	for (i = 0; i < len; i++, p++)
		if (*p != ' ' && *p != '\t')
			break;
	if (i == len)
		return NULL;
	if (endp != NULL) {
		for (i++, q = p + 1; i < len; i++, q++)
			if (*q == ' ' || *q == '\t')
				break;
		*endp = q;
	}
	return p;
}

/* command_completion --- attempt to complete based on the word number in line;
 *    try to complete on command names if this is the first word; for the next
 *    word(s), the type of completion depends on the command name (first word).
 */

#ifndef RL_READLINE_VERSION		/* < 4.2a */
#define rl_completion_matches(x, y) completion_matches((char *) (x), (y))
#endif


char **
command_completion(const char *text, int start, int end)
{
	char *cmdtok, *e;
	int idx;
	int len;

	rl_attempted_completion_over = true;	/* no default filename completion please */

	this_cmd = D_illegal;
	len = start;
	if ((cmdtok = next_word(rl_line_buffer, len, &e)) == NULL)	/* no first word yet */
		return  rl_completion_matches(text, command_generator);
	len -= (e - rl_line_buffer);

	idx = find_command(cmdtok, e - cmdtok);
	if (idx < 0)
		return NULL;
	this_cmd = cmdtab[idx].type;

	if (! next_word(e, len, NULL)) {
		switch (this_cmd) {
		case D_break:
		case D_list:
		case D_until:
		case D_tbreak:
		case D_clear:
			return rl_completion_matches(text, srcfile_generator);
		case D_info:
		case D_enable:
		case D_trace:
		case D_help:
			return rl_completion_matches(text, argument_generator);
		case D_option:
			return rl_completion_matches(text, option_generator);
		case D_print:
		case D_printf:
		case D_set:
		case D_display:
		case D_watch:
			return rl_completion_matches(text, variable_generator);
		default:
			return NULL;
		}
	}

	if (this_cmd == D_print || this_cmd == D_printf)
		return rl_completion_matches(text, variable_generator);
	return NULL;
}

/* command_generator --- generator function for command completion */

static char *
command_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx = 0;
	char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}
	while ((name = (char *) cmdtab[idx].name) != NULL) {
		idx++;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}
	return NULL;
}

/* srcfile_generator --- generator function for source file completion */

static char *
srcfile_generator(const char *text, int state)
{
	static size_t textlen;
	static SRCFILE *s;
	char *name;
	extern SRCFILE *srcfiles;

	if (! state) {	/* first time */
		textlen = strlen(text);
		s = srcfiles->next;
	}
	while (s != srcfiles) {
		if (s->stype != SRC_FILE && s->stype != SRC_INC) {
			s = s->next;
			continue;
		}
		name = s->src;
		s = s->next;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}
	return NULL;
}

/* argument_generator --- generator function for non-number argument completion */

static char *
argument_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx;
	const char *name;

	if (! state) {	/* first time */
		textlen = strlen(text);
		idx = 0;
	}

	if (this_cmd == D_help) {
		while ((name = cmdtab[idx++].name) != NULL) {
			if (strncmp(name, text, textlen) == 0)
				return estrdup(name, strlen(name));
		}
	} else {
		while ((name = argtab[idx].name) != NULL) {
			if (this_cmd != argtab[idx++].cmd)
				continue;
			if (strncmp(name, text, textlen) == 0)
				return estrdup(name, strlen(name));
		}
	}
	return NULL;
}

/* variable_generator --- generator function for variable name completion */

static char *
variable_generator(const char *text, int state)
{
	static size_t textlen;
	static int idx = 0;
	static NODE *func = NULL;
	static NODE **vars = NULL;
	const char *name;
	NODE *r;

	if (! state) {	/* first time */
		textlen = strlen(text);
		if (vars != NULL)
			efree(vars);
		vars = variable_list();
		idx = 0;
		func = get_function();  /* function in current context */
	}

	/* function params */
	while (func != NULL) {
		if (idx >= func->param_cnt) {
			func = NULL;	/* don't try to match params again */
			idx = 0;
			break;
		}
		name = func->fparms[idx++].param;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}

	/* globals */
	while ((r = vars[idx++]) != NULL) {
		name = r->vname;
		if (strncmp(name, text, textlen) == 0)
			return estrdup(name, strlen(name));
	}

	return NULL;
}

/* history_expand_line ---  history expand the LINE */

static void
history_expand_line(char **line)
{
	int ret;
	char *expansion;

	if (! *line || input_fd != 0 || ! input_from_tty)
		return;
	using_history();
	ret = history_expand(*line, &expansion);
	if (ret < 0 || ret == 2)
		efree(expansion);
	else {
		efree(*line);
		*line = expansion;
	}
}

#endif
