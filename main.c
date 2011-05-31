/*
 * main.c -- Code generator and main program for gawk. 
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2011 the Free Software Foundation, Inc.
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

/* FIX THIS BEFORE EVERY RELEASE: */
#define UPDATE_YEAR	2011

#include "awk.h"
#include "getopt.h"

#ifdef HAVE_MCHECK_H
#include <mcheck.h>
#endif

#define DEFAULT_PROFILE		"awkprof.out"	/* where to put profile */
#define DEFAULT_VARFILE		"awkvars.out"	/* where to put vars */

static const char *varfile = DEFAULT_VARFILE;
const char *command_file = NULL;	/* debugger commands */

static void usage(int exitval, FILE *fp) ATTRIBUTE_NORETURN;
static void copyleft(void) ATTRIBUTE_NORETURN;
static void cmdline_fs(char *str);
static void init_args(int argc0, int argc, const char *argv0, char **argv);
static void init_vars(void);
static NODE *load_environ(void);
static NODE *load_procinfo(void);
static RETSIGTYPE catchsig(int sig);
#ifdef HAVE_LIBSIGSEGV
static int catchsegv(void *fault_address, int serious);
static void catchstackoverflow(int emergency, stackoverflow_context_t scp);
#endif
static void nostalgia(void) ATTRIBUTE_NORETURN;
static void version(void) ATTRIBUTE_NORETURN;
static void init_fds(void);
static void init_groupset(void);

static void save_argv(int, char **);

/* These nodes store all the special variables AWK uses */
NODE *ARGC_node, *ARGIND_node, *ARGV_node, *BINMODE_node, *CONVFMT_node;
NODE *ENVIRON_node, *ERRNO_node, *FIELDWIDTHS_node, *FILENAME_node;
NODE *FNR_node, *FPAT_node, *FS_node, *IGNORECASE_node, *LINT_node;
NODE *NF_node, *NR_node, *OFMT_node, *OFS_node, *ORS_node, *PROCINFO_node;
NODE *RLENGTH_node, *RSTART_node, *RS_node, *RT_node, *SUBSEP_node;
NODE *TEXTDOMAIN_node;

NODE *_r;	/* used as temporary in stack macros */

long NF;
long NR;
long FNR;
int BINMODE;
int IGNORECASE;
char *OFS;
char *ORS;
char *OFMT;
char *TEXTDOMAIN;

/*
 * CONVFMT is a convenience pointer for the current number to string format.
 * We must supply an initial value to avoid recursion problems of
 *	set_CONVFMT -> fmt_index -> r_force_string: gets NULL CONVFMT
 * Fun, fun, fun, fun.
 */
char *CONVFMT = "%.6g";

NODE *Nnull_string;		/* The global null string */

#if defined(HAVE_LOCALE_H)
struct lconv loc;		/* current locale */
static void init_locale(struct lconv *l);
#endif /* defined(HAVE_LOCALE_H) */

/* The name the program was invoked under, for error messages */
const char *myname;

/* A block of AWK code to be run */
INSTRUCTION *code_block = NULL;

char **d_argv;			/* saved argv for debugger restarting */
/*
 * List of rules and functions with first and last instruction (source_line)
 * information; used for profiling and debugging.
 */
INSTRUCTION *rule_list;

int exit_val = EXIT_SUCCESS;		/* exit value */

#if defined(YYDEBUG) || defined(GAWKDEBUG)
extern int yydebug;
#endif

SRCFILE *srcfiles; /* source files */

/*
 * structure to remember variable pre-assignments
 */
struct pre_assign {
	enum assign_type { PRE_ASSIGN = 1, PRE_ASSIGN_FS } type;
	char *val;
};

static struct pre_assign *preassigns = NULL;	/* requested via -v or -F */
static long numassigns = -1;			/* how many of them */

static int disallow_var_assigns = FALSE;	/* true for --exec */

static void add_preassign(enum assign_type type, char *val);

#undef do_lint
#undef do_lint_old

int do_traditional = FALSE;	/* no gnu extensions, add traditional weirdnesses */
int do_posix = FALSE;		/* turn off gnu and unix extensions */
int do_lint = FALSE;		/* provide warnings about questionable stuff */
int do_lint_old = FALSE;	/* warn about stuff not in V7 awk */
int do_intl = FALSE;		/* dump locale-izable strings to stdout */
int do_non_decimal_data = FALSE;	/* allow octal/hex C style DATA. Use with caution! */
int do_nostalgia = FALSE;	/* provide a blast from the past */
int do_intervals = FALSE;	/* allow {...,...} in regexps, see resetup() */
int do_profiling = FALSE;	/* profile and pretty print the program */
int do_dump_vars = FALSE;	/* dump all global variables at end */
int do_tidy_mem = FALSE;	/* release vars when done */
int do_optimize = TRUE;		/* apply default optimizations */
int do_binary = FALSE;		/* hands off my data! */
int do_sandbox = FALSE; 	/* sandbox mode - disable 'system' function & redirections */
int use_lc_numeric = FALSE;	/* obey locale for decimal point */

#ifdef MBS_SUPPORT
int gawk_mb_cur_max;		/* MB_CUR_MAX value, see comment in main() */
#else
const int gawk_mb_cur_max = 1;
#endif

FILE *output_fp;		/* default output for debugger */
int output_is_tty = FALSE;	/* control flushing of output */

/* default format for strftime(), available via PROCINFO */
const char def_strftime_format[] = "%a %b %e %H:%M:%S %Z %Y";

extern const char *version_string;

#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
GETGROUPS_T *groupset;		/* current group set */
int ngroups;			/* size of said set */
#endif

void (*lintfunc)(const char *mesg, ...) = warning;

/*
 * Note: reserve -D for future use, to merge dgawk into gawk.
 * Note: reserve -l for future use, for xgawk's -l option.
 */
static const struct option optab[] = {
	{ "traditional",	no_argument,		& do_traditional,	1 },
	{ "lint",		optional_argument,	NULL,		'L' },
	{ "lint-old",		no_argument,		& do_lint_old,	1 },
	{ "optimize",		no_argument,		& do_optimize,	'O' },
	{ "posix",		no_argument,		& do_posix,	1 },
	{ "command",		required_argument,	NULL,		'R' },
	{ "nostalgia",		no_argument,		& do_nostalgia,	1 },
	{ "gen-pot",		no_argument,		& do_intl,	1 },
	{ "non-decimal-data",	no_argument,		& do_non_decimal_data, 1 },
	{ "profile",		optional_argument,	NULL,		'p' },
	{ "copyright",		no_argument,		NULL,		'C' },
	{ "field-separator",	required_argument,	NULL,		'F' },
	{ "file",		required_argument,	NULL,		'f' },
	{ "re-interval",	no_argument,		& do_intervals,	1 },
	{ "source",		required_argument,	NULL,		'e' },
	{ "dump-variables",	optional_argument,	NULL,		'd' },
	{ "assign",		required_argument,	NULL,		'v' },
	{ "version",		no_argument,		NULL,		'V' },
	{ "help",		no_argument,		NULL,		'h' },
	{ "exec",		required_argument,	NULL,		'E' },
	{ "use-lc-numeric",	no_argument,		& use_lc_numeric, 1 },
	{ "characters-as-bytes", no_argument,		& do_binary,	 'b' },
	{ "sandbox",		no_argument,		& do_sandbox, 	1 },
#if defined(YYDEBUG) || defined(GAWKDEBUG)
	{ "parsedebug",		no_argument,		NULL,		'Y' },
#endif
	{ NULL, 0, NULL, '\0' }
};

#ifdef NO_LINT
#define do_lint 0
#define do_lint_old 0
#endif

/* main --- process args, parse program, run it, clean up */

int
main(int argc, char **argv)
{
	/*
	 * The + on the front tells GNU getopt not to rearrange argv.
	 * Note: reserve -D for future use, to merge dgawk into gawk.
	 * Note: reserve -l for future use, for xgawk's -l option.
	 */
	const char *optlist = "+F:f:v:W;m:bcCd::e:E:gh:L:nNOp::PrR:StVY";
	int stopped_early = FALSE;
	int old_optind;
	int i;
	int c;
	char *scan, *src;
	char *extra_stack;

	/* do these checks early */
	do_tidy_mem = (getenv("TIDYMEM") != NULL);

#ifdef HAVE_MCHECK_H
#ifdef HAVE_MTRACE
	if (do_tidy_mem)
		mtrace();
#endif /* HAVE_MTRACE */
#endif /* HAVE_MCHECK_H */

#if defined(LC_CTYPE)
	setlocale(LC_CTYPE, "");
#endif
#if defined(LC_COLLATE)
	setlocale(LC_COLLATE, "");
#endif
#if defined(LC_MESSAGES)
	setlocale(LC_MESSAGES, "");
#endif
#if defined(LC_NUMERIC) && defined(HAVE_LOCALE_H)
	/*
	 * Force the issue here.  According to POSIX 2001, decimal
	 * point is used for parsing source code and for command-line
	 * assignments and the locale value for processing input,
	 * number to string conversion, and printing output.
	 *
	 * 10/2005 --- see below also; we now only use the locale's
	 * decimal point if do_posix in effect.
	 *
	 * 9/2007:
	 * This is a mess. We need to get the locale's numeric info for
	 * the thousands separator for the %'d flag.
	 */
	setlocale(LC_NUMERIC, "");
	init_locale(& loc);
	setlocale(LC_NUMERIC, "C");
#endif
#if defined(LC_TIME)
	setlocale(LC_TIME, "");
#endif

#ifdef MBS_SUPPORT
	/*
	 * In glibc, MB_CUR_MAX is actually a function.  This value is
	 * tested *a lot* in many speed-critical places in gawk. Caching
	 * this value once makes a speed difference.
	 */
	gawk_mb_cur_max = MB_CUR_MAX;
	/* Without MBS_SUPPORT, gawk_mb_cur_max is 1. */

	/* init the cache for checking bytes if they're characters */
	init_btowc_cache();
#endif

	(void) bindtextdomain(PACKAGE, LOCALEDIR);
	(void) textdomain(PACKAGE);

	(void) signal(SIGFPE, catchsig);
#ifdef SIGBUS
	(void) signal(SIGBUS, catchsig);
#endif

	(void) sigsegv_install_handler(catchsegv);
#define STACK_SIZE (16*1024)
	emalloc(extra_stack, char *, STACK_SIZE, "main");
	(void) stackoverflow_install_handler(catchstackoverflow, extra_stack, STACK_SIZE);
#undef STACK_SIZE

	myname = gawk_name(argv[0]);
	os_arg_fixup(&argc, &argv); /* emulate redirection, expand wildcards */

	if (argc < 2)
		usage(EXIT_FAILURE, stderr);

	/* Robustness: check that file descriptors 0, 1, 2 are open */
	init_fds();

	/* init array handling. */
	array_init();

	/* we do error messages ourselves on invalid options */
	opterr = FALSE;

	/* copy argv before getopt gets to it; used to restart the debugger */  
	save_argv(argc, argv);

	/* initialize global (main) execution context */
	push_context(new_context());

	/* option processing. ready, set, go! */
	for (optopt = 0, old_optind = 1;
	     (c = getopt_long(argc, argv, optlist, optab, NULL)) != EOF;
	     optopt = 0, old_optind = optind) {
		if (do_posix)
			opterr = TRUE;

		switch (c) {
		case 'F':
			add_preassign(PRE_ASSIGN_FS, optarg);
			break;

		case 'E':
			disallow_var_assigns = TRUE;
			/* fall through */
		case 'f':
			/*
			 * Allow multiple -f options.
			 * This makes function libraries real easy.
			 * Most of the magic is in the scanner.
			 *
			 * The following is to allow for whitespace at the end
			 * of a #! /bin/gawk line in an executable file
			 */
			scan = optarg;
			if (argv[optind-1] != optarg)
				while (isspace((unsigned char) *scan))
					scan++;
			src = (*scan == '\0' ? argv[optind++] : optarg);
			(void) add_srcfile((src && src[0] == '-' && src[1] == '\0') ?
					SRC_STDIN : SRC_FILE,
					src, srcfiles, NULL, NULL);

			break;

		case 'v':
			add_preassign(PRE_ASSIGN, optarg);
			break;

		case 'm':
			/*
			 * BWK awk extension.
			 *	-mf nnn		set # fields, gawk ignores
			 *	-mr nnn		set record length, ditto
			 *
			 * As of at least 10/2007, BWK awk also ignores it.
			 */
			if (do_lint)
				lintwarn(_("`-m[fr]' option irrelevant in gawk"));
			if (optarg[0] != 'r' && optarg[0] != 'f')
				warning(_("-m option usage: `-m[fr] nnn'"));
			break;

		case 'b':
			do_binary = TRUE;
			break;

		case 'c':
			do_traditional = TRUE;
			break;

		case 'C':
			copyleft();
			break;

		case 'd':
			do_dump_vars = TRUE;
			if (optarg != NULL && optarg[0] != '\0')
				varfile = optarg;
			break;

		case 'e':
			if (optarg[0] == '\0')
				warning(_("empty argument to `-e/--source' ignored"));
			else
				(void) add_srcfile(SRC_CMDLINE, optarg, srcfiles, NULL, NULL);
			break;

		case 'g':
			do_intl = TRUE;
			break;

		case 'h':
			/* write usage to stdout, per GNU coding stds */
			usage(EXIT_SUCCESS, stdout);
			break;

#ifndef NO_LINT
		case 'L':
			do_lint = LINT_ALL;
			if (optarg != NULL) {
				if (strcmp(optarg, "fatal") == 0)
					lintfunc = r_fatal;
				else if (strcmp(optarg, "invalid") == 0)
					do_lint = LINT_INVALID;
			}
			break;

		case 't':
			do_lint_old = TRUE;
			break;
#else
		case 'L':
		case 't':
			break;
#endif

		case 'n':
			do_non_decimal_data = TRUE;
			break;

		case 'N':
			use_lc_numeric = TRUE;
			break;

		case 'O':
			do_optimize++;
			break;

		case 'p':
			do_profiling = TRUE;
			if (optarg != NULL)
				set_prof_file(optarg);
			else
				set_prof_file(DEFAULT_PROFILE);
			break;

		case 'P':
			do_posix = TRUE;
			break;

		case 'r':
			do_intervals = TRUE;
 			break;
 
		case 'S':
			do_sandbox = TRUE;
  			break;

		case 'V':
			version();
			break;

		case 'W':       /* gawk specific options - now in getopt_long */
			fprintf(stderr, _("%s: option `-W %s' unrecognized, ignored\n"),
				argv[0], optarg);
			break;

		case 0:
			/*
			 * getopt_long found an option that sets a variable
			 * instead of returning a letter. Do nothing, just
			 * cycle around for the next one.
			 */
			break;

		case 'Y':
		case 'R':
#if defined(YYDEBUG) || defined(GAWKDEBUG)
			if (c == 'Y') {
				yydebug = 2;
				break;
			}
#endif
			if (c == 'R' &&  which_gawk == exe_debugging) {
				if (optarg[0] != '\0')
					command_file = optarg;
				break;
			}
			/* if not debugging or dgawk, fall through */

		case '?':
		default:
			/*
			 * If not posix, an unrecognized option stops argument
			 * processing so that it can go into ARGV for the awk
			 * program to see. This makes use of ``#! /bin/gawk -f''
			 * easier.
			 *
			 * However, it's never simple. If optopt is set,
			 * an option that requires an argument didn't get the
			 * argument. We care because if opterr is 0, then
			 * getopt_long won't print the error message for us.
			 */
			if (! do_posix
			    && (optopt == '\0' || strchr(optlist, optopt) == NULL)) {
				/*
				 * can't just do optind--. In case of an
				 * option with >= 2 letters, getopt_long
				 * won't have incremented optind.
				 */
				optind = old_optind;
				stopped_early = TRUE;
				goto out;
			} else if (optopt != '\0') {
				/* Use POSIX required message format */
				fprintf(stderr,
					_("%s: option requires an argument -- %c\n"),
					myname, optopt);
				usage(EXIT_FAILURE, stderr);
			}
			/* else
				let getopt print error message for us */
			break;
		}
		if (c == 'E')	/* --exec ends option processing */
			break;
	}
out:

	if (do_nostalgia)
		nostalgia();

	/* check for POSIXLY_CORRECT environment variable */
	if (! do_posix && getenv("POSIXLY_CORRECT") != NULL) {
		do_posix = TRUE;
		if (do_lint)
			lintwarn(
	_("environment variable `POSIXLY_CORRECT' set: turning on `--posix'"));
	}

	if (do_posix) {
		use_lc_numeric = TRUE;
		if (do_traditional)	/* both on command line */
			warning(_("`--posix' overrides `--traditional'"));
		else
			do_traditional = TRUE;
			/*
			 * POSIX compliance also implies
			 * no GNU extensions either.
			 */
	}

	if (do_traditional && do_non_decimal_data) {
		do_non_decimal_data = FALSE;
		warning(_("`--posix'/`--traditional' overrides `--non-decimal-data'"));
	}

	if (do_lint && os_is_setuid())
		warning(_("running %s setuid root may be a security problem"), myname);

#ifdef MBS_SUPPORT
	if (do_binary) {
		if (do_posix)
			warning(_("`--posix' overrides `--binary'"));
		else
			gawk_mb_cur_max = 1;	/* hands off my data! */
	}
#endif

	/*
	 * Force profiling if this is pgawk.
	 * Don't bother if the command line already set profiling up.
	 */
	if (! do_profiling)
		init_profiling(& do_profiling, DEFAULT_PROFILE);

	/* load group set */
	init_groupset();

	/* initialize the null string */
	Nnull_string = make_string("", 0);
	Nnull_string->numbr = 0.0;
	Nnull_string->type = Node_val;
	Nnull_string->flags = (PERM|STRCUR|STRING|NUMCUR|NUMBER);

	/*
	 * Tell the regex routines how they should work.
	 * Do this before initializing variables, since
	 * they could want to do a regexp compile.
	 */
	resetup();

	(void) grow_stack();

	/* Set up the special variables */
	init_vars();

	/* Set up the field variables */
	init_fields();

	/* Now process the pre-assignments */
	for (i = 0; i <= numassigns; i++) {
		if (preassigns[i].type == PRE_ASSIGN)
			(void) arg_assign(preassigns[i].val, TRUE);
		else	/* PRE_ASSIGN_FS */
			cmdline_fs(preassigns[i].val);
		efree(preassigns[i].val);
	}

	if (preassigns != NULL)
		efree(preassigns);

	if ((BINMODE & 1) != 0)
		if (os_setbinmode(fileno(stdin), O_BINARY) == -1)
			fatal(_("can't set binary mode on stdin (%s)"), strerror(errno));
	if ((BINMODE & 2) != 0) {
		if (os_setbinmode(fileno(stdout), O_BINARY) == -1)
			fatal(_("can't set binary mode on stdout (%s)"), strerror(errno));
		if (os_setbinmode(fileno(stderr), O_BINARY) == -1)
			fatal(_("can't set binary mode on stderr (%s)"), strerror(errno));
	}

#ifdef GAWKDEBUG
	setbuf(stdout, (char *) NULL);	/* make debugging easier */
#endif
	if (os_isatty(fileno(stdout)))
		output_is_tty = TRUE;
	/* No -f or --source options, use next arg */
	if (srcfiles->next == srcfiles) {
		if (optind > argc - 1 || stopped_early) /* no args left or no program */
			usage(EXIT_FAILURE, stderr);
		(void) add_srcfile(SRC_CMDLINE, argv[optind], srcfiles, NULL, NULL);
		optind++;
	}

	init_args(optind, argc,
			do_posix ? argv[0] : myname,
			argv);

#if defined(LC_NUMERIC)
	/*
	 * FRAGILE!  CAREFUL!
	 * Pre-initing the variables with arg_assign() can change the
	 * locale.  Force it to C before parsing the program.
	 */
	setlocale(LC_NUMERIC, "C");
#endif
	/* Read in the program */
	if (parse_program(&code_block) != 0)
		exit(EXIT_FAILURE);
	
	if (do_intl)
		exit(EXIT_SUCCESS);

	if (do_lint)
		shadow_funcs();

	if (do_lint && code_block->nexti->opcode == Op_atexit)
		lintwarn(_("no program text at all!"));

	init_profiling_signals();

#if defined(LC_NUMERIC)
	/*
	 * See comment above about using locale's decimal point.
	 *
	 * 10/2005:
	 * Bitter experience teaches us that most people the world over
	 * use period as the decimal point, not whatever their locale
	 * uses.  Thus, only use the locale's decimal point if being
	 * posixly anal-retentive.
	 *
	 * 7/2007:
	 * Be a little bit kinder. Allow the --use-lc-numeric option
	 * to also use the local decimal point. This avoids the draconian
	 * strictness of POSIX mode if someone just wants to parse their
	 * data using the local decimal point.
	 */
	if (use_lc_numeric)
		setlocale(LC_NUMERIC, "");
#endif

	output_fp = stdout;
	interpret(code_block);

	if (do_profiling) {
		dump_prog(code_block);
		dump_funcs();
	}

	if (do_dump_vars)
		dump_vars(varfile);

	if (do_tidy_mem)
		release_all_vars();
	
	/* keep valgrind happier */
	if (extra_stack)
		efree(extra_stack);

	exit(exit_val);		/* more portable */
	return exit_val;	/* to suppress warnings */
}

/* add_preassign --- add one element to preassigns */

static void
add_preassign(enum assign_type type, char *val)
{
	static long alloc_assigns;		/* for how many are allocated */

#define INIT_SRC 4

	++numassigns;

	if (preassigns == NULL) {
		emalloc(preassigns, struct pre_assign *,
			INIT_SRC * sizeof(struct pre_assign), "add_preassign");
		alloc_assigns = INIT_SRC;
	} else if (numassigns >= alloc_assigns) {
		alloc_assigns *= 2;
		erealloc(preassigns, struct pre_assign *,
			alloc_assigns * sizeof(struct pre_assign), "add_preassigns");
	}
	preassigns[numassigns].type = type;
	preassigns[numassigns].val = estrdup(val, strlen(val));

#undef INIT_SRC
}

/* usage --- print usage information and exit */

static void
usage(int exitval, FILE *fp)
{
	/* Not factoring out common stuff makes it easier to translate. */
	fprintf(fp, _("Usage: %s [POSIX or GNU style options] -f progfile [--] file ...\n"),
		myname);
	fprintf(fp, _("Usage: %s [POSIX or GNU style options] [--] %cprogram%c file ...\n"),
		myname, quote, quote);

	/* GNU long options info. This is too many options. */

	fputs(_("POSIX options:\t\tGNU long options: (standard)\n"), fp);
	fputs(_("\t-f progfile\t\t--file=progfile\n"), fp);
	fputs(_("\t-F fs\t\t\t--field-separator=fs\n"), fp);
	fputs(_("\t-v var=val\t\t--assign=var=val\n"), fp);
	fputs(_("Short options:\t\tGNU long options: (extensions)\n"), fp);
	fputs(_("\t-b\t\t\t--characters-as-bytes\n"), fp);
	fputs(_("\t-c\t\t\t--traditional\n"), fp);
	fputs(_("\t-C\t\t\t--copyright\n"), fp);
	fputs(_("\t-d[file]\t\t--dump-variables[=file]\n"), fp);
	fputs(_("\t-e 'program-text'\t--source='program-text'\n"), fp);
	fputs(_("\t-E file\t\t\t--exec=file\n"), fp);
	fputs(_("\t-g\t\t\t--gen-pot\n"), fp);
	fputs(_("\t-h\t\t\t--help\n"), fp);
	fputs(_("\t-L [fatal]\t\t--lint[=fatal]\n"), fp);
	fputs(_("\t-n\t\t\t--non-decimal-data\n"), fp);
	fputs(_("\t-N\t\t\t--use-lc-numeric\n"), fp);
	fputs(_("\t-O\t\t\t--optimize\n"), fp);
	fputs(_("\t-p[file]\t\t--profile[=file]\n"), fp);
	fputs(_("\t-P\t\t\t--posix\n"), fp);
	fputs(_("\t-r\t\t\t--re-interval\n"), fp);
	if (which_gawk == exe_debugging)
		fputs(_("\t-R file\t\t\t--command=file\n"), fp);
	fputs(_("\t-S\t\t\t--sandbox\n"), fp);
	fputs(_("\t-t\t\t\t--lint-old\n"), fp);
	fputs(_("\t-V\t\t\t--version\n"), fp);
#ifdef NOSTALGIA
	fputs(_("\t-W nostalgia\t\t--nostalgia\n"), fp);
#endif
#ifdef GAWKDEBUG
	fputs(_("\t-Y\t\t--parsedebug\n"), fp);
#endif

	/* This is one string to make things easier on translators. */
	/* TRANSLATORS: --help output 5 (end)
	   TRANSLATORS: the placeholder indicates the bug-reporting address
	   for this application.  Please add _another line_ with the
	   address for translation bugs.
	   no-wrap */
	fputs(_("\nTo report bugs, see node `Bugs' in `gawk.info', which is\n\
section `Reporting Problems and Bugs' in the printed version.\n\n"), fp);

	/* ditto */
	fputs(_("gawk is a pattern scanning and processing language.\n\
By default it reads standard input and writes standard output.\n\n"), fp);

	/* ditto */
	fputs(_("Examples:\n\tgawk '{ sum += $1 }; END { print sum }' file\n\
\tgawk -F: '{ print $1 }' /etc/passwd\n"), fp);

	fflush(fp);

	if (ferror(fp)) {
		if (fp == stdout)
			warning(_("error writing standard output (%s)"), strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(exitval);
}

/* copyleft --- print out the short GNU copyright information */

static void
copyleft()
{
	static const char blurb_part1[] =
	  N_("Copyright (C) 1989, 1991-%d Free Software Foundation.\n\
\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 3 of the License, or\n\
(at your option) any later version.\n\
\n");
	static const char blurb_part2[] =
	  N_("This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n");
	static const char blurb_part3[] =
	  N_("You should have received a copy of the GNU General Public License\n\
along with this program. If not, see http://www.gnu.org/licenses/.\n");
 
	/* multiple blurbs are needed for some brain dead compilers. */
	printf(_(blurb_part1), UPDATE_YEAR);	/* Last update year */
	fputs(_(blurb_part2), stdout);
	fputs(_(blurb_part3), stdout);
	fflush(stdout);

	if (ferror(stdout)) {
		warning(_("error writing standard output (%s)"), strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/* cmdline_fs --- set FS from the command line */

static void
cmdline_fs(char *str)
{
	NODE **tmp;

	tmp = &FS_node->var_value;
	unref(*tmp);
	/*
	 * Only if in full compatibility mode check for the stupid special
	 * case so -F\t works as documented in awk book even though the shell
	 * hands us -Ft.  Bleah!
	 *
	 * Thankfully, POSIX didn't propagate this "feature".
	 */
	if (str[0] == 't' && str[1] == '\0') {
		if (do_lint)
			lintwarn(_("-Ft does not set FS to tab in POSIX awk"));
		if (do_traditional && ! do_posix)
			str[0] = '\t';
	}
	*tmp = make_str_node(str, strlen(str), SCAN); /* do process escapes */
	set_FS();
}

/* init_args --- set up ARGV from stuff on the command line */

static void
init_args(int argc0, int argc, const char *argv0, char **argv)
{
	int i, j;
	NODE **aptr;
	NODE *tmp;

	ARGV_node = install_symbol(estrdup("ARGV", 4), mk_symbol(Node_var_array, (NODE *) NULL));
	tmp =  make_number(0.0);
	aptr = assoc_lookup(ARGV_node, tmp, FALSE);
	unref(tmp);
	unref(*aptr);
	*aptr = make_string(argv0, strlen(argv0));
	(*aptr)->flags |= MAYBE_NUM;
	for (i = argc0, j = 1; i < argc; i++, j++) {
		tmp = make_number((AWKNUM) j);
		aptr = assoc_lookup(ARGV_node, tmp, FALSE);
		unref(tmp);
		unref(*aptr);
		*aptr = make_string(argv[i], strlen(argv[i]));
		(*aptr)->flags |= MAYBE_NUM;
	}

	ARGC_node = install_symbol(estrdup("ARGC", 4),
					mk_symbol(Node_var, make_number((AWKNUM) j)));
}

/*
 * Set all the special variables to their initial values.
 * Note that some of the variables that have set_FOO routines should
 * *N*O*T* have those routines called upon initialization, and thus
 * they have NULL entries in that field. This is notably true of FS
 * and IGNORECASE.
 */

struct varinit {
	NODE **spec;
	const char *name;
	const char *strval;
	AWKNUM numval;
	Func_ptr update;
	Func_ptr assign;
	int do_assign;
	int flags;
#define NO_INSTALL	0x01
#define NON_STANDARD	0x02
};

static const struct varinit varinit[] = {
{NULL,		"ARGC",		NULL,	0,  NULL, NULL,	FALSE, NO_INSTALL },
{&ARGIND_node,	"ARGIND",	NULL,	0,  NULL, NULL,	FALSE, NON_STANDARD },
{NULL,		"ARGV",		NULL,	0,  NULL, NULL,	FALSE, NO_INSTALL },
{&BINMODE_node,	"BINMODE",	NULL,	0,  NULL, set_BINMODE,	FALSE, NON_STANDARD },
{&CONVFMT_node,	"CONVFMT",	"%.6g",	0,  NULL, set_CONVFMT,TRUE, 	0 },
{NULL,		"ENVIRON",	NULL,	0,  NULL, NULL,	FALSE, NO_INSTALL },
{&ERRNO_node,	"ERRNO",	"",	0,  NULL, NULL,	FALSE, NON_STANDARD },
{&FIELDWIDTHS_node, "FIELDWIDTHS", "",	0,  NULL, set_FIELDWIDTHS,	FALSE, NON_STANDARD },
{&FILENAME_node, "FILENAME",	"",	0,  NULL, NULL,	FALSE, 0 },
{&FNR_node,	"FNR",		NULL,	0,  update_FNR, set_FNR,	TRUE, 0 },
{&FS_node,	"FS",		" ",	0,  NULL, set_FS,	FALSE, 0 },
{&FPAT_node,	"FPAT",		"[^[:space:]]+", 0,  NULL, set_FPAT,	FALSE, 0 },
{&IGNORECASE_node, "IGNORECASE", NULL,	0,  NULL, set_IGNORECASE,	FALSE, NON_STANDARD },
{&LINT_node,	"LINT",		NULL,	0,  NULL, set_LINT,	FALSE, NON_STANDARD },
{&NF_node,	"NF",		NULL,	-1, update_NF, set_NF,	FALSE, 0 },
{&NR_node,	"NR",		NULL,	0,  update_NR, set_NR,	TRUE, 0 },
{&OFMT_node,	"OFMT",		"%.6g",	0,  NULL, set_OFMT,	TRUE, 0 },
{&OFS_node,	"OFS",		" ",	0,  NULL, set_OFS,	TRUE, 0 },
{&ORS_node,	"ORS",		"\n",	0,  NULL, set_ORS,	TRUE, 0 },
{NULL,		"PROCINFO",	NULL,	0,  NULL, NULL,	FALSE, NO_INSTALL | NON_STANDARD },
{&RLENGTH_node, "RLENGTH",	NULL,	0,  NULL, NULL,	FALSE, 0 },
{&RS_node,	"RS",		"\n",	0,  NULL, set_RS,	TRUE, 0 },
{&RSTART_node,	"RSTART",	NULL,	0,  NULL, NULL,	FALSE, 0 },
{&RT_node,	"RT",		"",	0,  NULL, NULL,	FALSE, NON_STANDARD },
{&SUBSEP_node,	"SUBSEP",	"\034",	0,  NULL, set_SUBSEP,	TRUE, 0 },
{&TEXTDOMAIN_node,	"TEXTDOMAIN",	"messages",	0,  NULL, set_TEXTDOMAIN,	TRUE, NON_STANDARD },
{0,		NULL,		NULL,	0,  NULL, NULL,	FALSE, 0 },
};

/* init_vars --- actually initialize everything in the symbol table */

static void
init_vars()
{
	const struct varinit *vp;
	NODE *n;

	for (vp = varinit; vp->name != NULL; vp++) {
		if ((vp->flags & NO_INSTALL) != 0)
			continue;
		n = mk_symbol(Node_var, vp->strval == NULL
				? make_number(vp->numval)
				: make_string(vp->strval, strlen(vp->strval)));
		n->var_assign = (Func_ptr) vp->assign;
		n->var_update = (Func_ptr) vp->update;

		*(vp->spec) = install_symbol(estrdup(vp->name, strlen(vp->name)), n);
		if (vp->do_assign)
			(*(vp->assign))();
	}

	/* Set up deferred variables (loaded only when accessed). */
	if (! do_traditional)
		register_deferred_variable("PROCINFO", load_procinfo);
	register_deferred_variable("ENVIRON", load_environ);
}

/* load_environ --- populate the ENVIRON array */

static NODE *
load_environ()
{
#if ! (defined(VMS) && defined(__DECC))
	extern char **environ;
#endif
	char *var, *val;
	NODE **aptr;
	int i;
	NODE *tmp;

	ENVIRON_node = install_symbol(estrdup("ENVIRON", 7), 
				mk_symbol(Node_var_array, (NODE *) NULL));

	for (i = 0; environ[i] != NULL; i++) {
		static char nullstr[] = "";

		var = environ[i];
		val = strchr(var, '=');
		if (val != NULL)
			*val++ = '\0';
		else
			val = nullstr;
		tmp = make_string(var, strlen(var));
		aptr = assoc_lookup(ENVIRON_node, tmp, FALSE);
		unref(tmp);
		unref(*aptr);
		*aptr = make_string(val, strlen(val));
		(*aptr)->flags |= MAYBE_NUM;

		/* restore '=' so that system() gets a valid environment */
		if (val != nullstr)
			*--val = '=';
	}
	/*
	 * Put AWKPATH into ENVIRON if it's not there.
	 * This allows querying it from within awk programs.
	 */
	tmp = make_string("AWKPATH", 7);
	if (! in_array(ENVIRON_node, tmp)) {
		/*
		 * On VMS, environ[] only holds a subset of what getenv() can
		 * find, so look AWKPATH up before resorting to default path.
		 */
		val = getenv("AWKPATH");
		if (val == NULL)
			val = defpath;
		aptr = assoc_lookup(ENVIRON_node, tmp, FALSE);
		unref(*aptr);
		*aptr = make_string(val, strlen(val));
	}
	unref(tmp);
	return ENVIRON_node;
}

/* load_procinfo --- populate the PROCINFO array */

static NODE *
load_procinfo()
{
#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
	int i;
	char name[100];
#endif
	AWKNUM value;

	PROCINFO_node = install_symbol(estrdup("PROCINFO", 8),
				mk_symbol(Node_var_array, (NODE *) NULL));

	update_PROCINFO_str("version", VERSION);
	update_PROCINFO_str("strftime", def_strftime_format);

#ifdef GETPGRP_VOID
#define getpgrp_arg() /* nothing */
#else
#define getpgrp_arg() getpid()
#endif

	value = getpgrp(getpgrp_arg());
	update_PROCINFO_num("pgrpid", value);

	/*
	 * Could put a lot of this into a table, but then there's
	 * portability problems declaring all the functions. So just
	 * do it the slow and stupid way. Sigh.
	 */

	value = getpid();
	update_PROCINFO_num("pid", value);

	value = getppid();
	update_PROCINFO_num("ppid", value);

	value = getuid();
	update_PROCINFO_num("uid", value);

	value = geteuid();
	update_PROCINFO_num("euid", value);

	value = getgid();
	update_PROCINFO_num("gid", value);

	value = getegid();
	update_PROCINFO_num("egid", value);

	switch (current_field_sep()) {
	case Using_FIELDWIDTHS:
		update_PROCINFO_str("FS", "FIELDWIDTHS");
		break;
	case Using_FPAT:
		update_PROCINFO_str("FS", "FPAT");
		break;
	case Using_FS:
		update_PROCINFO_str("FS", "FS");
		break;
	default:
		fatal(_("unknown value for field spec: %d\n"),
				current_field_sep());
		break;
	}


#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
	for (i = 0; i < ngroups; i++) {
		sprintf(name, "group%d", i + 1);
		value = groupset[i];
		update_PROCINFO_num(name, value);
	}
	if (groupset) {
		efree(groupset);
		groupset = NULL;
	}
#endif
	return PROCINFO_node;
}

/* is_std_var --- return true if a variable is a standard variable */

int
is_std_var(const char *var)
{
	const struct varinit *vp;

	for (vp = varinit; vp->name != NULL; vp++) {
		if (strcmp(vp->name, var) == 0) {
			if ((do_traditional || do_posix) && (vp->flags & NON_STANDARD) != 0)
				return FALSE;

			return TRUE;
		}
	}

	return FALSE;
}

/* arg_assign --- process a command-line assignment */

int
arg_assign(char *arg, int initing)
{
	char *cp, *cp2;
	int badvar;
	NODE *var;
	NODE *it;
	NODE **lhs;
	long save_FNR;

	if (! initing && disallow_var_assigns)
		return FALSE;	/* --exec */

	cp = strchr(arg, '=');

	if (cp == NULL) {
		if (! initing)
			return FALSE;	/* This is file name, not assignment. */

		fprintf(stderr,
			_("%s: `%s' argument to `-v' not in `var=value' form\n\n"),
			myname, arg);
		usage(EXIT_FAILURE, stderr);
	}

	*cp++ = '\0';

	/* avoid false source indications in a fatal message */
	source = NULL;
	sourceline = 0;
	save_FNR = FNR;
	FNR = 0;

	/* first check that the variable name has valid syntax */
	badvar = FALSE;
	if (! isalpha((unsigned char) arg[0]) && arg[0] != '_')
		badvar = TRUE;
	else
		for (cp2 = arg+1; *cp2; cp2++)
			if (! isalnum((unsigned char) *cp2) && *cp2 != '_') {
				badvar = TRUE;
				break;
			}

	if (badvar) {
		if (initing)
			fatal(_("`%s' is not a legal variable name"), arg);

		if (do_lint)
			lintwarn(_("`%s' is not a variable name, looking for file `%s=%s'"),
				arg, arg, cp);
	} else {
		if (check_special(arg) >= 0)
			fatal(_("cannot use gawk builtin `%s' as variable name"), arg);

		if (! initing) {
			var = lookup(arg);
			if (var != NULL && var->type == Node_func)
				fatal(_("cannot use function `%s' as variable name"), arg); 
		}

		/*
		 * BWK awk expands escapes inside assignments.
		 * This makes sense, so we do it too.
		 */
		it = make_str_node(cp, strlen(cp), SCAN);
		it->flags |= MAYBE_NUM;
#ifdef LC_NUMERIC
		/*
		 * See comment above about locale decimal point.
		 */
		if (do_posix)
			setlocale(LC_NUMERIC, "C");
		(void) force_number(it);
		if (do_posix)
			setlocale(LC_NUMERIC, "");
#endif /* LC_NUMERIC */

		/*
		 * since we are restoring the original text of ARGV later,
		 * need to copy the variable name part if we don't want
		 * name like v=abc instead of just v in var->vname
		 */

		cp2 = estrdup(arg, cp - arg);	/* var name */

		var = variable(cp2, Node_var);
		if (var == NULL)	/* error */
			exit(EXIT_FATAL);
		if (var->type == Node_var && var->var_update)
			var->var_update();
		lhs = get_lhs(var, FALSE);
		unref(*lhs);
		*lhs = it;
		/* check for set_FOO() routine */
		if (var->type == Node_var && var->var_assign)
			var->var_assign();
	}

	if (! initing)
		*--cp = '=';	/* restore original text of ARGV */
	FNR = save_FNR;
	return ! badvar;
}

/* catchsig --- catch signals */

static RETSIGTYPE
catchsig(int sig)
{
	if (sig == SIGFPE) {
		fatal(_("floating point exception"));
	} else if (sig == SIGSEGV
#ifdef SIGBUS
	        || sig == SIGBUS
#endif
	) {
		set_loc(__FILE__, __LINE__);
		msg(_("fatal error: internal error"));
		/* fatal won't abort() if not compiled for debugging */
		abort();
	} else
		cant_happen();
	/* NOTREACHED */
}

#ifdef HAVE_LIBSIGSEGV
/* catchsegv --- for use with libsigsegv */

static int
catchsegv(void *fault_address, int serious)
{
	set_loc(__FILE__, __LINE__);
	msg(_("fatal error: internal error: segfault"));
	abort();
	/*NOTREACHED*/
	return 0;
}

/* catchstackoverflow --- for use with libsigsegv */

static void
catchstackoverflow(int emergency, stackoverflow_context_t scp)
{
	set_loc(__FILE__, __LINE__);
	msg(_("fatal error: internal error: stack overflow"));
	abort();
	/*NOTREACHED*/
	return;
}
#endif /* HAVE_LIBSIGSEGV */

/* nostalgia --- print the famous error message and die */

static void
nostalgia()
{
	/*
	 * N.B.: This string is not gettextized, on purpose.
	 * So there.
	 */
	fprintf(stderr, "awk: bailing out near line 1\n");
	fflush(stderr);
	abort();
}

/* version --- print version message */

static void
version()
{
	printf("%s\n", version_string);
	/*
	 * Per GNU coding standards, print copyright info,
	 * then exit successfully, do nothing else.
	 */
	copyleft();
	exit(EXIT_SUCCESS);
}

/* init_fds --- check for 0, 1, 2, open on /dev/null if possible */

static void
init_fds()
{
	struct stat sbuf;
	int fd;
	int newfd;
	char const *const opposite_mode[] = {"w", "r", "r"};

	/* maybe no stderr, don't bother with error mesg */
	for (fd = 0; fd <= 2; fd++) {
		if (fstat(fd, &sbuf) < 0) {
#if MAKE_A_HEROIC_EFFORT
			if (do_lint)
				lintwarn(_("no pre-opened fd %d"), fd);
#endif
			newfd = devopen("/dev/null", opposite_mode[fd]);
			/* turn off some compiler warnings "set but not used" */
			newfd += 0;
#ifdef MAKE_A_HEROIC_EFFORT
			if (do_lint && newfd < 0)
				lintwarn(_("could not pre-open /dev/null for fd %d"), fd);
#endif
		}
	}
}

/* init_groupset --- initialize groupset */

static void
init_groupset()
{
#if defined(HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
#ifdef GETGROUPS_NOT_STANDARD
	/* For systems that aren't standards conformant, use old way. */
	ngroups = NGROUPS_MAX;
#else
	/*
	 * If called with 0 for both args, return value is
	 * total number of groups.
	 */
	ngroups = getgroups(0, NULL);
#endif
	if (ngroups == -1)
		fatal(_("could not find groups: %s"), strerror(errno));
	else if (ngroups == 0)
		return;

	/* fill in groups */
	emalloc(groupset, GETGROUPS_T *, ngroups * sizeof(GETGROUPS_T), "init_groupset");

	ngroups = getgroups(ngroups, groupset);
	if (ngroups == -1)
		fatal(_("could not find groups: %s"), strerror(errno));
#endif
}

/* estrdup --- duplicate a string */

char *
estrdup(const char *str, size_t len)
{
	char *s;
	emalloc(s, char *, len + 1, "estrdup");
	memcpy(s, str, len);
	s[len] = '\0';
	return s;
}
             
#if defined(HAVE_LOCALE_H)

/* init_locale --- initialize locale info. */

/*
 * On some operating systems, the pointers in the struct returned
 * by localeconv() can become dangling pointers after a call to
 * setlocale().  So we do a deep copy.
 *
 * Thanks to KIMURA Koichi <kimura.koichi@canon.co.jp>.
 */

static void
init_locale(struct lconv *l)
{
	struct lconv *t;

	t = localeconv();
	*l = *t;
	l->thousands_sep = estrdup(t->thousands_sep, strlen(t->thousands_sep));
	l->decimal_point = estrdup(t->decimal_point, strlen(t->decimal_point));
	l->grouping = estrdup(t->grouping, strlen(t->grouping));
	l->int_curr_symbol = estrdup(t->int_curr_symbol, strlen(t->int_curr_symbol));
	l->currency_symbol = estrdup(t->currency_symbol, strlen(t->currency_symbol));
	l->mon_decimal_point = estrdup(t->mon_decimal_point, strlen(t->mon_decimal_point));
	l->mon_thousands_sep = estrdup(t->mon_thousands_sep, strlen(t->mon_thousands_sep));
	l->mon_grouping = estrdup(t->mon_grouping, strlen(t->mon_grouping));
	l->positive_sign = estrdup(t->positive_sign, strlen(t->positive_sign));
	l->negative_sign = estrdup(t->negative_sign, strlen(t->negative_sign));
}
#endif /* LOCALE_H */

/* save_argv --- save argv array */

static void
save_argv(int argc, char **argv)
{
	int i;

	emalloc(d_argv, char **, (argc + 1) * sizeof(char *), "save_argv");
	for (i = 0; i < argc; i++)
		d_argv[i] = estrdup(argv[i], strlen(argv[i]));
	d_argv[argc] = NULL;
}

/*
 * update_global_values --- make sure the symbol table has correct values.
 * Called from the grammar before dumping values.
 */

void
update_global_values()
{
	const struct varinit *vp;

	for (vp = varinit; vp->name; vp++) {
		if (vp->update != NULL)
			vp->update();
	}
}
