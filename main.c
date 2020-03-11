/*
 * main.c -- Code generator and main program for gawk.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2020,
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

/* FIX THIS BEFORE EVERY RELEASE: */
#define UPDATE_YEAR	2020

#include "awk.h"
#include "getopt.h"

#ifdef HAVE_MCHECK_H
#include <mcheck.h>
#endif

#ifdef HAVE_LIBSIGSEGV
#include <sigsegv.h>
#else
typedef void *stackoverflow_context_t;
/* the argument to this macro is purposely not used */
#define sigsegv_install_handler(catchsegv) signal(SIGSEGV, catchsig)
/* define as 0 rather than empty so that (void) cast on it works */
#define stackoverflow_install_handler(catchstackoverflow, extra_stack, STACK_SIZE) 0
#endif

#define DEFAULT_PROFILE		"awkprof.out"	/* where to put profile */
#define DEFAULT_VARFILE		"awkvars.out"	/* where to put vars */
#define DEFAULT_PREC		53
#define DEFAULT_ROUNDMODE	"N"		/* round to nearest */

static const char *varfile = DEFAULT_VARFILE;
const char *command_file = NULL;	/* debugger commands */

static void usage(int exitval, FILE *fp) ATTRIBUTE_NORETURN;
static void copyleft(void) ATTRIBUTE_NORETURN;
static void cmdline_fs(char *str);
static void init_args(int argc0, int argc, const char *argv0, char **argv);
static void init_vars(void);
static NODE *load_environ(void);
static NODE *load_procinfo(void);
static void catchsig(int sig);
#ifdef HAVE_LIBSIGSEGV
static int catchsegv(void *fault_address, int serious);
static void catchstackoverflow(int emergency, stackoverflow_context_t scp);
#endif
static void nostalgia(void) ATTRIBUTE_NORETURN;
static void version(void) ATTRIBUTE_NORETURN;
static void init_fds(void);
static void init_groupset(void);
static void save_argv(int, char **);
static const char *platform_name();

/* These nodes store all the special variables AWK uses */
NODE *ARGC_node, *ARGIND_node, *ARGV_node, *BINMODE_node, *CONVFMT_node;
NODE *ENVIRON_node, *ERRNO_node, *FIELDWIDTHS_node, *FILENAME_node;
NODE *FNR_node, *FPAT_node, *FS_node, *IGNORECASE_node, *LINT_node;
NODE *NF_node, *NR_node, *OFMT_node, *OFS_node, *ORS_node, *PROCINFO_node;
NODE *RLENGTH_node, *RSTART_node, *RS_node, *RT_node, *SUBSEP_node;
NODE *PREC_node, *ROUNDMODE_node;
NODE *TEXTDOMAIN_node;

long NF;
long NR;
long FNR;
int BINMODE;
bool IGNORECASE;
char *OFS;
char *ORS;
char *OFMT;
char *TEXTDOMAIN;

/*
 * CONVFMT is a convenience pointer for the current number to string format.
 * We must supply an initial value to avoid recursion problems of
 *	set_CONVFMT -> fmt_index -> force_string: gets NULL CONVFMT
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

static bool disallow_var_assigns = false;	/* true for --exec */

static void add_preassign(enum assign_type type, char *val);

static void parse_args(int argc, char **argv);
static void set_locale_stuff(void);
static bool stopped_early = false;

int do_flags = false;
bool do_optimize = true;		/* apply default optimizations */
static int do_nostalgia = false;	/* provide a blast from the past */
static int do_binary = false;		/* hands off my data! */
static int do_version = false;		/* print version info */
static const char *locale = "";		/* default value to setlocale */
static char *locale_dir = LOCALEDIR;	/* default locale dir */

int use_lc_numeric = false;	/* obey locale for decimal point */

int gawk_mb_cur_max;		/* MB_CUR_MAX value, see comment in main() */

FILE *output_fp;		/* default gawk output, can be redirected in the debugger */
bool output_is_tty = false;	/* control flushing of output */

/* default format for strftime(), available via PROCINFO */
const char def_strftime_format[] = "%a %b %e %H:%M:%S %Z %Y";

extern const char *version_string;

#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
GETGROUPS_T *groupset;		/* current group set */
int ngroups;			/* size of said set */
#endif

void (*lintfunc)(const char *mesg, ...) = r_warning;

/* Sorted by long option name! */
static const struct option optab[] = {
	{ "assign",		required_argument,	NULL,	'v' },
	{ "bignum",		no_argument,		NULL,	'M' },
	{ "characters-as-bytes", no_argument,		& do_binary,	 'b' },
	{ "copyright",		no_argument,		NULL,	'C' },
	{ "debug",		optional_argument,	NULL,	'D' },
	{ "dump-variables",	optional_argument,	NULL,	'd' },
	{ "exec",		required_argument,	NULL,	'E' },
	{ "field-separator",	required_argument,	NULL,	'F' },
	{ "file",		required_argument,	NULL,	'f' },
	{ "gen-pot",		no_argument,		NULL,	'g' },
	{ "help",		no_argument,		NULL,	'h' },
	{ "include",		required_argument,	NULL,	'i' },
	{ "lint",		optional_argument,	NULL,	'L' },
	{ "lint-old",		no_argument,		NULL,	't' },
	{ "load",		required_argument,	NULL,	'l' },
#if defined(LOCALEDEBUG)
	{ "locale",		required_argument,	NULL,	'Z' },
#endif
	{ "non-decimal-data",	no_argument,		NULL,	'n' },
	{ "no-optimize",	no_argument,		NULL,	's' },
	{ "nostalgia",		no_argument,		& do_nostalgia,	1 },
	{ "optimize",		no_argument,		NULL,	'O' },
#if defined(YYDEBUG) || defined(GAWKDEBUG)
	{ "parsedebug",		no_argument,		NULL,	'Y' },
#endif
	{ "posix",		no_argument,		NULL,	'P' },
	{ "pretty-print",	optional_argument,	NULL,	'o' },
	{ "profile",		optional_argument,	NULL,	'p' },
	{ "re-interval",	no_argument,		NULL,	'r' },
	{ "sandbox",		no_argument,		NULL, 	'S' },
	{ "source",		required_argument,	NULL,	'e' },
	{ "traditional",	no_argument,		NULL,	'c' },
	{ "use-lc-numeric",	no_argument,		& use_lc_numeric, 1 },
	{ "version",		no_argument,		& do_version, 'V' },
	{ NULL, 0, NULL, '\0' }
};

/* main --- process args, parse program, run it, clean up */

int
main(int argc, char **argv)
{
	int i;
	char *extra_stack;
	int have_srcfile = 0;
	SRCFILE *s;
	char *cp;
#if defined(LOCALEDEBUG)
	const char *initial_locale;
#endif

	/* do these checks early */
	if (getenv("TIDYMEM") != NULL)
		do_flags |= DO_TIDY_MEM;

#ifdef HAVE_MCHECK_H
#ifdef HAVE_MTRACE
	if (do_tidy_mem)
		mtrace();
#endif /* HAVE_MTRACE */
#endif /* HAVE_MCHECK_H */

	myname = gawk_name(argv[0]);
	os_arg_fixup(&argc, &argv); /* emulate redirection, expand wildcards */

	if (argc < 2)
		usage(EXIT_FAILURE, stderr);

	if ((cp = getenv("GAWK_LOCALE_DIR")) != NULL)
		locale_dir = cp;

#if defined(F_GETFL) && defined(O_APPEND)
	// 1/2018: This is needed on modern BSD systems so that the
	// inplace tests pass. I think it's a bug in those kernels
	// but let's just work around it anyway.
	int flags = fcntl(fileno(stderr), F_GETFL, NULL);
	if (flags >= 0 && (flags & O_APPEND) == 0) {
		flags |= O_APPEND;
		(void) fcntl(fileno(stderr), F_SETFL, flags);
	}
#endif

#if defined(LOCALEDEBUG)
	initial_locale = locale;
#endif
	set_locale_stuff();

	(void) signal(SIGFPE, catchsig);
#ifdef SIGBUS
	(void) signal(SIGBUS, catchsig);
#endif

	/*
	 * Ignore SIGPIPE so that writes to pipes that fail don't
	 * kill the process but instead return -1 and set errno.
	 * That lets us print a fatal message instead of dieing suddenly.
	 *
	 * Note that this requires ignoring EPIPE when writing and
	 * flushing stdout/stderr in other parts of the program. E.g.,
	 *
	 * 	gawk 'BEGIN { print "hi" }' | exit
	 *
	 * should not give us "broken pipe" messages --- mainly because
	 * it did not do so in the past and people would complain.
	 */
	ignore_sigpipe();

	(void) sigsegv_install_handler(catchsegv);
#define STACK_SIZE (16*1024)
	emalloc(extra_stack, char *, STACK_SIZE, "main");
	(void) stackoverflow_install_handler(catchstackoverflow, extra_stack, STACK_SIZE);
#undef STACK_SIZE

	/* initialize the null string */
	Nnull_string = make_string("", 0);

	/* Robustness: check that file descriptors 0, 1, 2 are open */
	init_fds();

	/* init array handling. */
	array_init();

	/* init the symbol tables */
	init_symbol_table();

	output_fp = stdout;

	/* initialize global (main) execution context */
	push_context(new_context());

	parse_args(argc, argv);

#if defined(LOCALEDEBUG)
	if (locale != initial_locale)
		set_locale_stuff();
#endif

	/*
	 * In glibc, MB_CUR_MAX is actually a function.  This value is
	 * tested *a lot* in many speed-critical places in gawk. Caching
	 * this value once makes a speed difference.
	 */
	gawk_mb_cur_max = MB_CUR_MAX;

	/* init the cache for checking bytes if they're characters */
	init_btowc_cache();

	/* set up the single byte case table */
	if (gawk_mb_cur_max == 1)
		load_casetable();

	if (do_nostalgia)
		nostalgia();

	/* check for POSIXLY_CORRECT environment variable */
	if (! do_posix && getenv("POSIXLY_CORRECT") != NULL) {
		do_flags |= DO_POSIX;
		if (do_lint)
			lintwarn(
	_("environment variable `POSIXLY_CORRECT' set: turning on `--posix'"));
	}

	// Checks for conflicting command-line arguments.
	if (do_posix) {
		use_lc_numeric = true;
		if (do_traditional)	/* both on command line */
			warning(_("`--posix' overrides `--traditional'"));
		else
			do_flags |= DO_TRADITIONAL;
			/*
			 * POSIX compliance also implies
			 * no GNU extensions either.
			 */
	}

	if (do_traditional && do_non_decimal_data) {
		do_flags &= ~DO_NON_DEC_DATA;
		warning(_("`--posix'/`--traditional' overrides `--non-decimal-data'"));
	}

	if (do_binary) {
		if (do_posix)
			warning(_("`--posix' overrides `--characters-as-bytes'"));
		else
			gawk_mb_cur_max = 1;	/* hands off my data! */
#if defined(LC_ALL)
		setlocale(LC_ALL, "C");
#endif
	}

	if (do_lint && os_is_setuid())
		lintwarn(_("running %s setuid root may be a security problem"), myname);

	if (do_debug)	/* Need to register the debugger pre-exec hook before any other */
		init_debug();

#ifdef HAVE_MPFR
	/* Set up MPFR defaults, and register pre-exec hook to process arithmetic opcodes */
	if (do_mpfr)
		init_mpfr(DEFAULT_PREC, DEFAULT_ROUNDMODE);
#endif

	/* load group set */
	init_groupset();

#ifdef HAVE_MPFR
	if (do_mpfr) {
		mpz_init(Nnull_string->mpg_i);
		Nnull_string->flags = (MALLOC|STRCUR|STRING|MPZN|NUMCUR|NUMBER);
	} else
#endif
	{
		Nnull_string->numbr = 0.0;
		Nnull_string->flags = (MALLOC|STRCUR|STRING|NUMCUR|NUMBER);
	}

	/*
	 * Tell the regex routines how they should work.
	 * Do this before initializing variables, since
	 * they could want to do a regexp compile.
	 */
	resetup();

	/* Set up the special variables */
	init_vars();

	/* Set up the field variables */
	init_fields();

	/* Now process the pre-assignments */
	int dash_v_errs = 0;	// bad stuff for -v
	for (i = 0; i <= numassigns; i++) {
		if (preassigns[i].type == PRE_ASSIGN)
			dash_v_errs += (arg_assign(preassigns[i].val, true) == false);
		else	/* PRE_ASSIGN_FS */
			cmdline_fs(preassigns[i].val);
		efree(preassigns[i].val);
	}

	if (preassigns != NULL)
		efree(preassigns);

	if ((BINMODE & BINMODE_INPUT) != 0)
		if (os_setbinmode(fileno(stdin), O_BINARY) == -1)
			fatal(_("cannot set binary mode on stdin: %s"), strerror(errno));
	if ((BINMODE & BINMODE_OUTPUT) != 0) {
		if (os_setbinmode(fileno(stdout), O_BINARY) == -1)
			fatal(_("cannot set binary mode on stdout: %s"), strerror(errno));
		if (os_setbinmode(fileno(stderr), O_BINARY) == -1)
			fatal(_("cannot set binary mode on stderr: %s"), strerror(errno));
	}

#ifdef GAWKDEBUG
	setbuf(stdout, (char *) NULL);	/* make debugging easier */
#endif
	if (os_isatty(fileno(stdout)))
		output_is_tty = true;

	/* initialize API before loading extension libraries */
	init_ext_api();

	/* load extension libs */
	for (s = srcfiles->next; s != srcfiles; s = s->next) {
		if (s->stype == SRC_EXTLIB)
			load_ext(s->fullpath);
		else if (s->stype != SRC_INC)
			have_srcfile++;
	}

	/* do version check after extensions are loaded to get extension info */
	if (do_version)
		version();

	/* No -f or --source options, use next arg */
	if (! have_srcfile) {
		if (optind > argc - 1 || stopped_early) /* no args left or no program */
			usage(EXIT_FAILURE, stderr);
		(void) add_srcfile(SRC_CMDLINE, argv[optind], srcfiles, NULL, NULL);
		optind++;
	}

	/* Select the interpreter routine */
	init_interpret();

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
	if (parse_program(& code_block, false) != 0 || dash_v_errs > 0)
		exit(EXIT_FAILURE);

	if (do_intl)
		exit(EXIT_SUCCESS);

	set_current_namespace(awk_namespace);

	install_builtins();

	if (do_lint)
		shadow_funcs();

	if (do_lint && code_block->nexti->opcode == Op_atexit)
		lintwarn(_("no program text at all!"));

	load_symbols();

	if (do_profile)
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
		setlocale(LC_NUMERIC, locale);
#endif

	init_io();
	output_fp = stdout;

	if (do_debug)
		debug_prog(code_block);
	else if (do_pretty_print && ! do_profile)
		;	/* run pretty printer only. */
	else
		interpret(code_block);

	if (do_pretty_print) {
		set_current_namespace(awk_namespace);
		dump_prog(code_block);
		dump_funcs();
	}

	if (do_dump_vars)
		dump_vars(varfile);

#ifdef HAVE_MPFR
	if (do_mpfr)
		cleanup_mpfr();
#endif

	if (do_tidy_mem)
		release_all_vars();

	/* keep valgrind happier */
	if (extra_stack)
		efree(extra_stack);

	final_exit(exit_val);
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
	fputs(_("\t-D[file]\t\t--debug[=file]\n"), fp);
	fputs(_("\t-e 'program-text'\t--source='program-text'\n"), fp);
	fputs(_("\t-E file\t\t\t--exec=file\n"), fp);
	fputs(_("\t-g\t\t\t--gen-pot\n"), fp);
	fputs(_("\t-h\t\t\t--help\n"), fp);
	fputs(_("\t-i includefile\t\t--include=includefile\n"), fp);
	fputs(_("\t-l library\t\t--load=library\n"), fp);
	/*
	 * TRANSLATORS: the "fatal", "invalid" and "no-ext" here are literal
	 * values, they should not be translated. Thanks.
	 */
	fputs(_("\t-L[fatal|invalid|no-ext]\t--lint[=fatal|invalid|no-ext]\n"), fp);
	fputs(_("\t-M\t\t\t--bignum\n"), fp);
	fputs(_("\t-N\t\t\t--use-lc-numeric\n"), fp);
	fputs(_("\t-n\t\t\t--non-decimal-data\n"), fp);
	fputs(_("\t-o[file]\t\t--pretty-print[=file]\n"), fp);
	fputs(_("\t-O\t\t\t--optimize\n"), fp);
	fputs(_("\t-p[file]\t\t--profile[=file]\n"), fp);
	fputs(_("\t-P\t\t\t--posix\n"), fp);
	fputs(_("\t-r\t\t\t--re-interval\n"), fp);
	fputs(_("\t-s\t\t\t--no-optimize\n"), fp);
	fputs(_("\t-S\t\t\t--sandbox\n"), fp);
	fputs(_("\t-t\t\t\t--lint-old\n"), fp);
	fputs(_("\t-V\t\t\t--version\n"), fp);
#ifdef NOSTALGIA
	fputs(_("\t-W nostalgia\t\t--nostalgia\n"), fp);
#endif
#ifdef GAWKDEBUG
	fputs(_("\t-Y\t\t\t--parsedebug\n"), fp);
#endif
#ifdef GAWKDEBUG
	fputs(_("\t-Z locale-name\t\t--locale=locale-name\n"), fp);
#endif

	/* This is one string to make things easier on translators. */
	/* TRANSLATORS: --help output (end)
	   no-wrap */
	fputs(_("\nTo report bugs, see node `Bugs' in `gawk.info'\n\
which is section `Reporting Problems and Bugs' in the\n\
printed version.  This same information may be found at\n\
https://www.gnu.org/software/gawk/manual/html_node/Bugs.html.\n\
PLEASE do NOT try to report bugs by posting in comp.lang.awk,\n\
or by using a web forum such as Stack Overflow.\n\n"), fp);

	/* ditto */
	fputs(_("gawk is a pattern scanning and processing language.\n\
By default it reads standard input and writes standard output.\n\n"), fp);

	/* ditto */
	fprintf(fp, _("Examples:\n\t%s '{ sum += $1 }; END { print sum }' file\n\
\t%s -F: '{ print $1 }' /etc/passwd\n"), myname, myname);

	fflush(fp);

	if (ferror(fp)) {
#ifdef __MINGW32__
		if (errno == 0 || errno == EINVAL)
			w32_maybe_set_errno();
#endif
		/* don't warn about stdout/stderr if EPIPE, but do error exit */
		if (errno == EPIPE)
			die_via_sigpipe();

		if (fp == stdout)
			warning(_("error writing standard output: %s"), strerror(errno));
		else if (fp == stderr)
			warning(_("error writing standard error: %s"), strerror(errno));

		// some other problem than SIGPIPE
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
#ifdef __MINGW32__
		if (errno == 0 || errno == EINVAL)
			w32_maybe_set_errno();
#endif
		/* don't warn about stdout if EPIPE, but do error exit */
		if (errno != EPIPE)
			warning(_("error writing standard output: %s"), strerror(errno));
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
	NODE *sub, *val;
	NODE *shadow_node = NULL;

	ARGV_node = install_symbol(estrdup("ARGV", 4), Node_var_array);
	sub = make_number(0.0);
	val = make_string(argv0, strlen(argv0));
	val->flags |= USER_INPUT;
	assoc_set(ARGV_node, sub, val);

	if (do_sandbox) {
		shadow_node = make_array();
		sub = make_string(argv0, strlen(argv0));
		val = make_number(0.0);
		assoc_set(shadow_node, sub, val);
	}


	for (i = argc0, j = 1; i < argc; i++, j++) {
		sub = make_number((AWKNUM) j);
		val = make_string(argv[i], strlen(argv[i]));
		val->flags |= USER_INPUT;
		assoc_set(ARGV_node, sub, val);

		if (do_sandbox) {
			sub = make_string(argv[i], strlen(argv[i]));
			val = make_number(0.0);
			assoc_set(shadow_node, sub, val);
		}
	}

	ARGC_node = install_symbol(estrdup("ARGC", 4), Node_var);
	ARGC_node->var_value = make_number((AWKNUM) j);

	if (do_sandbox)
		init_argv_array(ARGV_node, shadow_node);
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
	bool do_assign;
	int flags;
#define NO_INSTALL	0x01
#define NON_STANDARD	0x02
#define NOT_OFF_LIMITS	0x04	/* may be accessed by extension function */
};

static const struct varinit varinit[] = {
{NULL,		"ARGC",		NULL,	0,  NULL, NULL,	false, NO_INSTALL },
{&ARGIND_node,	"ARGIND",	NULL,	0,  NULL, NULL,	false, NON_STANDARD },
{NULL,		"ARGV",		NULL,	0,  NULL, NULL,	false, NO_INSTALL },
{&BINMODE_node,	"BINMODE",	NULL,	0,  NULL, set_BINMODE,	false, NON_STANDARD },
{&CONVFMT_node,	"CONVFMT",	"%.6g",	0,  NULL, set_CONVFMT,true, 	0 },
{NULL,		"ENVIRON",	NULL,	0,  NULL, NULL,	false, NO_INSTALL },
{&ERRNO_node,	"ERRNO",	"",	0,  NULL, NULL,	false, NON_STANDARD },
{&FIELDWIDTHS_node, "FIELDWIDTHS", "",	0,  NULL, set_FIELDWIDTHS,	false, NON_STANDARD },
{&FILENAME_node, "FILENAME",	"",	0,  NULL, NULL,	false, 0 },
{&FNR_node,	"FNR",		NULL,	0,  update_FNR, set_FNR,	true, 0 },
{&FS_node,	"FS",		" ",	0,  NULL, set_FS,	false, 0 },
{&FPAT_node,	"FPAT",		"[^[:space:]]+", 0,  NULL, set_FPAT,	false, NON_STANDARD },
{&IGNORECASE_node, "IGNORECASE", NULL,	0,  NULL, set_IGNORECASE,	false, NON_STANDARD },
{&LINT_node,	"LINT",		NULL,	0,  NULL, set_LINT,	false, NON_STANDARD },
{&PREC_node,	"PREC",		NULL,	DEFAULT_PREC,	NULL,	set_PREC,	false,	NON_STANDARD},
{&NF_node,	"NF",		NULL,	-1, update_NF, set_NF,	false, 0 },
{&NR_node,	"NR",		NULL,	0,  update_NR, set_NR,	true, 0 },
{&OFMT_node,	"OFMT",		"%.6g",	0,  NULL, set_OFMT,	true, 0 },
{&OFS_node,	"OFS",		" ",	0,  NULL, set_OFS,	true, 0 },
{&ORS_node,	"ORS",		"\n",	0,  NULL, set_ORS,	true, 0 },
{NULL,		"PROCINFO",	NULL,	0,  NULL, NULL,	false, NO_INSTALL | NON_STANDARD | NOT_OFF_LIMITS },
{&RLENGTH_node, "RLENGTH",	NULL,	0,  NULL, NULL,	false, 0 },
{&ROUNDMODE_node, "ROUNDMODE",	DEFAULT_ROUNDMODE,	0,  NULL, set_ROUNDMODE,	false, NON_STANDARD },
{&RS_node,	"RS",		"\n",	0,  NULL, set_RS,	true, 0 },
{&RSTART_node,	"RSTART",	NULL,	0,  NULL, NULL,	false, 0 },
{&RT_node,	"RT",		"",	0,  NULL, NULL,	false, NON_STANDARD },
{&SUBSEP_node,	"SUBSEP",	"\034",	0,  NULL, set_SUBSEP,	true, 0 },
{&TEXTDOMAIN_node,	"TEXTDOMAIN",	"messages",	0,  NULL, set_TEXTDOMAIN,	true, NON_STANDARD },
{0,		NULL,		NULL,	0,  NULL, NULL,	false, 0 },
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
		n = *(vp->spec) = install_symbol(estrdup(vp->name, strlen(vp->name)), Node_var);
		if (vp->strval != NULL)
			n->var_value = make_string(vp->strval, strlen(vp->strval));
		else
			n->var_value = make_number(vp->numval);
		n->var_assign = (Func_ptr) vp->assign;
		n->var_update = (Func_ptr) vp->update;
		if (vp->do_assign)
			(*(vp->assign))();
	}

	/* Load PROCINFO and ENVIRON */
	if (! do_traditional)
		load_procinfo();
	load_environ();
}

/* path_environ --- put path variable into environment if not already there */

static void
path_environ(const char *pname, const char *dflt)
{
	const char *val;
	NODE **aptr;
	NODE *tmp;

	tmp = make_string(pname, strlen(pname));
	/*
	 * On VMS, environ[] only holds a subset of what getenv() can
	 * find, so look AWKPATH up before resorting to default path.
	 */
	val = getenv(pname);
	if (val == NULL || *val == '\0')
		val = dflt;
	aptr = assoc_lookup(ENVIRON_node, tmp);
	/*
	 * If original value was the empty string, set it to
	 * the default value.
	 */
	if ((*aptr)->stlen == 0) {
		unref(*aptr);
		*aptr = make_string(val, strlen(val));
	}

	unref(tmp);
}

/* load_environ --- populate the ENVIRON array */

static NODE *
load_environ()
{
#if ! (defined(VMS) && defined(__DECC))
	extern char **environ;
#endif
	char *var, *val;
	int i;
	NODE *sub, *newval;
	static bool been_here = false;

	if (been_here)
		return ENVIRON_node;

	been_here = true;

	ENVIRON_node = install_symbol(estrdup("ENVIRON", 7), Node_var_array);
	for (i = 0; environ[i] != NULL; i++) {
		static char nullstr[] = "";

		var = environ[i];
		val = strchr(var, '=');
		if (val != NULL)
			*val++ = '\0';
		else
			val = nullstr;
		sub = make_string(var, strlen(var));
		newval = make_string(val, strlen(val));
		newval->flags |= USER_INPUT;
		assoc_set(ENVIRON_node, sub, newval);

		/* restore '=' so that system() gets a valid environment */
		if (val != nullstr)
			*--val = '=';
	}
	/*
	 * Put AWKPATH and AWKLIBPATH into ENVIRON if not already there.
	 * This allows querying it from within awk programs.
	 *
	 * October 2014:
	 * If their values are "", override with the default values;
	 * since 2.10 AWKPATH used default value if environment's
	 * value was "".
	 */
	path_environ("AWKPATH", defpath);
	path_environ("AWKLIBPATH", deflibpath);

	/* set up array functions */
	init_env_array(ENVIRON_node);

	return ENVIRON_node;
}

/* load_procinfo_argv --- populate PROCINFO["argv"] */

static void
load_procinfo_argv()
{
	NODE *sub;
	NODE *val;
	NODE *argv_array;
	int i;

	// build the sub-array first
	getnode(argv_array);
 	memset(argv_array, '\0', sizeof(NODE));  /* valgrind wants this */
	null_array(argv_array);
	argv_array->parent_array = PROCINFO_node;
	argv_array->vname = estrdup("argv", 4);
	for (i = 0; d_argv[i] != NULL; i++) {
		sub = make_number(i);
		val = make_string(d_argv[i], strlen(d_argv[i]));
		assoc_set(argv_array, sub, val);
	}

	// hook it into PROCINFO
	sub = make_string("argv", 4);
	assoc_set(PROCINFO_node, sub, argv_array);

}

/* load_procinfo --- populate the PROCINFO array */

static NODE *
load_procinfo()
{
#if defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0
	int i;
#endif
#if (defined (HAVE_GETGROUPS) && defined(NGROUPS_MAX) && NGROUPS_MAX > 0) || defined(HAVE_MPFR)
	char name[100];
#endif
	AWKNUM value;
	static bool been_here = false;

	if (been_here)
		return PROCINFO_node;

	been_here = true;

	PROCINFO_node = install_symbol(estrdup("PROCINFO", 8), Node_var_array);

	update_PROCINFO_str("version", VERSION);
	update_PROCINFO_str("strftime", def_strftime_format);
	update_PROCINFO_str("platform", platform_name());

#ifdef HAVE_MPFR
	sprintf(name, "GNU MPFR %s", mpfr_get_version());
	update_PROCINFO_str("mpfr_version", name);
	sprintf(name, "GNU MP %s", gmp_version);
	update_PROCINFO_str("gmp_version", name);
	update_PROCINFO_num("prec_max", MPFR_PREC_MAX);
	update_PROCINFO_num("prec_min", MPFR_PREC_MIN);
#endif

#ifdef DYNAMIC
	update_PROCINFO_num("api_major", GAWK_API_MAJOR_VERSION);
	update_PROCINFO_num("api_minor", GAWK_API_MINOR_VERSION);
#endif

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

	update_PROCINFO_str("FS", current_field_sep_str());

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
	load_procinfo_argv();
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
				return false;

			return true;
		}
	}

	return false;
}

/*
 * is_off_limits_var --- return true if a variable is off limits
 * 			to extension functions
 */

int
is_off_limits_var(const char *var)
{
	const struct varinit *vp;

	for (vp = varinit; vp->name != NULL; vp++) {
		if (strcmp(vp->name, var) == 0)
			return ((vp->flags & NOT_OFF_LIMITS) == 0);
	}

	return false;
}

/* get_spec_varname --- return the name of a special variable
	with the given assign or update routine.
*/

const char *
get_spec_varname(Func_ptr fptr)
{
	const struct varinit *vp;

	if (! fptr)
		return NULL;
	for (vp = varinit; vp->name != NULL; vp++) {
		if (vp->assign == fptr || vp->update == fptr)
			return vp->name;
	}
	return NULL;
}


/* arg_assign --- process a command-line assignment */

int
arg_assign(char *arg, bool initing)
{
	char *cp, *cp2;
	bool badvar;
	NODE *var;
	NODE *it;
	NODE **lhs;
	long save_FNR;

	if (! initing && disallow_var_assigns)
		return false;	/* --exec */

	cp = strchr(arg, '=');

	if (cp == NULL) {
		if (! initing)
			return false;	/* This is file name, not assignment. */

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
	badvar = false;
	if (! is_letter((unsigned char) arg[0]))
		badvar = true;
	else
		for (cp2 = arg+1; *cp2; cp2++)
			if (! is_identchar((unsigned char) *cp2) && *cp2 != ':') {
				badvar = true;
				break;
			}

	if (badvar) {
		if (initing)
			fatal(_("`%s' is not a legal variable name"), arg);

		if (do_lint)
			lintwarn(_("`%s' is not a variable name, looking for file `%s=%s'"),
				arg, arg, cp);

		goto done;
	}

	// Assigning a string or typed regex

	if (! validate_qualified_name(arg)) {
		badvar = true;
		goto done;
	}

	if (check_special(arg) >= 0)
		fatal(_("cannot use gawk builtin `%s' as variable name"), arg);

	if (! initing) {
		var = lookup(arg);
		if (var != NULL && var->type == Node_func)
			fatal(_("cannot use function `%s' as variable name"), arg);
	}

	cp2 = cp + strlen(cp) - 1;	// end char
	if (! do_traditional
	    && cp[0] == '@' && cp[1] == '/' && *cp2 == '/') {
		// typed regex
		size_t len = strlen(cp) - 3;

		ezalloc(cp2, char *, len + 1, "arg_assign");
		memcpy(cp2, cp + 2, len);

		it = make_typed_regex(cp2, len);
		// fall through to variable setup
	} else {
		// string assignment

		// POSIX disallows any newlines inside strings
		// The scanner handles that for program files.
		// We have to check here for strings passed to -v.
		if (do_posix && strchr(cp, '\n') != NULL)
			fatal(_("POSIX does not allow physical newlines in string values"));

		/*
		 * BWK awk expands escapes inside assignments.
		 * This makes sense, so we do it too.
		 * In addition, remove \-<newline> as in scanning.
		 */
		it = make_str_node(cp, strlen(cp), SCAN | ELIDE_BACK_NL);
		it->flags |= USER_INPUT;
#ifdef LC_NUMERIC
		/*
		 * See comment above about locale decimal point.
		 */
		if (do_posix)
			setlocale(LC_NUMERIC, "C");
#endif /* LC_NUMERIC */
		(void) force_number(it);
#ifdef LC_NUMERIC
		if (do_posix)
			setlocale(LC_NUMERIC, locale);
#endif /* LC_NUMERIC */
	}

	/*
	 * since we are restoring the original text of ARGV later,
	 * need to copy the variable name part if we don't want
	 * name like v=abc instead of just v in var->vname
	 */

	cp2 = estrdup(arg, cp - arg);	/* var name */

	var = variable(0, cp2, Node_var);
	if (var == NULL)	/* error */
		final_exit(EXIT_FATAL);

	if (var->type == Node_var && var->var_update)
		var->var_update();
	lhs = get_lhs(var, false);
	unref(*lhs);
	*lhs = it;
	/* check for set_FOO() routine */
	if (var->type == Node_var && var->var_assign)
		var->var_assign();

done:
	if (! initing)
		*--cp = '=';	/* restore original text of ARGV */
	FNR = save_FNR;
	return ! badvar;
}

/* catchsig --- catch signals */

static void
catchsig(int sig)
{
	if (sig == SIGFPE) {
		fatal(_("floating point exception"));
	} else if (sig == SIGSEGV
#ifdef SIGBUS
	        || sig == SIGBUS
#endif
	) {
		if (errcount > 0)	// assume a syntax error corrupted our data structures
			exit(EXIT_FATAL);

		set_loc(__FILE__, __LINE__);
		msg(_("fatal error: internal error"));
		/* fatal won't abort() if not compiled for debugging */
		// GLIBC 2.27 doesn't necessarily flush on abort. Sigh.
		fflush(NULL);
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
	if (errcount > 0)	// assume a syntax error corrupted our data structures
		exit(EXIT_FATAL);

	set_loc(__FILE__, __LINE__);
	msg(_("fatal error: internal error: segfault"));
	fflush(NULL);
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
	fflush(NULL);
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
	printf("%s", version_string);
#ifdef DYNAMIC
	printf(", API: %d.%d", GAWK_API_MAJOR_VERSION, GAWK_API_MINOR_VERSION);
#endif
#ifdef HAVE_MPFR
	printf(" (GNU MPFR %s, GNU MP %s)", mpfr_get_version(), gmp_version);
#endif
	printf("\n");
	print_ext_versions();

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
	/* If an error or no groups, just give up and get on with life. */
	if (ngroups <= 0)
		return;

	/* fill in groups */
	emalloc(groupset, GETGROUPS_T *, ngroups * sizeof(GETGROUPS_T), "init_groupset");

	ngroups = getgroups(ngroups, groupset);
	/* same thing here, give up but keep going */
	if (ngroups == -1) {
		efree(groupset);
		ngroups = 0;
		groupset = NULL;
	}
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
 *
 * Also called when accessing through SYMTAB, and from api_sym_lookup().
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

/* getenv_long --- read a long value (>= 0) from an environment var. */

long
getenv_long(const char *name)
{
	const char *val;
	long newval;
	if ((val = getenv(name)) != NULL && isdigit((unsigned char) *val)) {
		for (newval = 0; *val && isdigit((unsigned char) *val); val++)
			newval = (newval * 10) + *val - '0';
		return newval;
	}
	return -1;
}

/* parse_args --- do the getopt_long thing */

static void
parse_args(int argc, char **argv)
{
	/*
	 * The + on the front tells GNU getopt not to rearrange argv.
	 */
	const char *optlist = "+F:f:v:W;bcCd::D::e:E:ghi:l:L::nNo::Op::MPrSstVYZ:";
	int old_optind;
	int c;
	char *scan;
	char *src;

	/* we do error messages ourselves on invalid options */
	opterr = false;

	/* copy argv before getopt gets to it; used to restart the debugger */
	save_argv(argc, argv);

	/* option processing. ready, set, go! */
	for (optopt = 0, old_optind = 1;
	     (c = getopt_long(argc, argv, optlist, optab, NULL)) != EOF;
	     optopt = 0, old_optind = optind) {
		if (do_posix)
			opterr = true;

		switch (c) {
		case 'F':
			add_preassign(PRE_ASSIGN_FS, optarg);
			break;

		case 'E':
			disallow_var_assigns = true;
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

		case 'b':
			do_binary = true;
			break;

		case 'c':
			do_flags |= DO_TRADITIONAL;
			break;

		case 'C':
			copyleft();
			break;

		case 'd':
			do_flags |= DO_DUMP_VARS;
			if (optarg != NULL && optarg[0] != '\0')
				varfile = optarg;
			break;

		case 'D':
			do_flags |= DO_DEBUG;
			if (optarg != NULL && optarg[0] != '\0')
				command_file = optarg;
			break;

		case 'e':
			if (optarg[0] == '\0')
				warning(_("empty argument to `-e/--source' ignored"));
			else
				(void) add_srcfile(SRC_CMDLINE, optarg, srcfiles, NULL, NULL);
			break;

		case 'g':
			do_flags |= DO_INTL;
			break;

		case 'h':
			/* write usage to stdout, per GNU coding stds */
			usage(EXIT_SUCCESS, stdout);
			break;

		case 'i':
			(void) add_srcfile(SRC_INC, optarg, srcfiles, NULL, NULL);
			break;

		case 'l':
			(void) add_srcfile(SRC_EXTLIB, optarg, srcfiles, NULL, NULL);
			break;

#ifndef NO_LINT
		case 'L':
			do_flags |= (DO_LINT_ALL|DO_LINT_EXTENSIONS);
			if (optarg != NULL) {
				if (strcmp(optarg, "fatal") == 0)
					lintfunc = r_fatal;
				else if (strcmp(optarg, "invalid") == 0) {
					do_flags &= ~DO_LINT_ALL;
					do_flags |= DO_LINT_INVALID;
				}
				else if (strcmp(optarg, "no-ext") == 0) {
					do_flags &= ~DO_LINT_EXTENSIONS;
				}
			}
			break;

		case 't':
			do_flags |= DO_LINT_OLD;
			break;
#else
		case 'L':
		case 't':
			break;
#endif

		case 'n':
			do_flags |= DO_NON_DEC_DATA;
			break;

		case 'N':
			use_lc_numeric = true;
			break;

		case 'O':
			do_optimize = true;
			break;

		case 'p':
			if (do_pretty_print)
				warning(_("`--profile' overrides `--pretty-print'"));
			do_flags |= DO_PROFILE;
			/* fall through */
		case 'o':
			if (c == 'o' && do_profile)
				warning(_("`--profile' overrides `--pretty-print'"));
			do_flags |= DO_PRETTY_PRINT;
			if (optarg != NULL)
				set_prof_file(optarg);
			else
				set_prof_file(DEFAULT_PROFILE);
			break;

		case 'M':
#ifdef HAVE_MPFR
			do_flags |= DO_MPFR;
#else
			warning(_("-M ignored: MPFR/GMP support not compiled in"));
#endif
			break;

		case 'P':
			do_flags |= DO_POSIX;
			break;

		case 'r':
			do_flags |= DO_INTERVALS;
 			break;

		case 's':
			do_optimize = false;
			break;

		case 'S':
			do_flags |= DO_SANDBOX;
			break;

		case 'V':
			do_version = true;
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
		case 'Z':
#if defined(YYDEBUG) || defined(GAWKDEBUG)
			if (c == 'Y') {
				yydebug = 2;
				break;
			}
#endif
#if defined(LOCALEDEBUG)
			if (c == 'Z') {
				locale = optarg;
				break;
			}
#endif
			/* if not debugging, fall through */
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
				stopped_early = true;
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
	do_optimize = (do_optimize && ! do_pretty_print);

	return;
}

/* set_locale_stuff --- setup the locale stuff */

static void
set_locale_stuff(void)
{
#if defined(LC_CTYPE)
	setlocale(LC_CTYPE, locale);
#endif
#if defined(LC_COLLATE)
	setlocale(LC_COLLATE, locale);
#endif
#if defined(LC_MESSAGES)
	setlocale(LC_MESSAGES, locale);
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
	setlocale(LC_NUMERIC, locale);
	init_locale(& loc);
	setlocale(LC_NUMERIC, "C");
#endif
#if defined(LC_TIME)
	setlocale(LC_TIME, locale);
#endif

	/* These must be done after calling setlocale */
	(void) bindtextdomain(PACKAGE, locale_dir);
	(void) textdomain(PACKAGE);
}

/* platform_name --- return the platform name */

static const char *
platform_name()
{
	// Cygwin and Mac OS X count as POSIX
#if defined(__VMS)
	return "vms";
#elif defined(__MINGW32__)
	return "mingw";
#elif defined(__DJGPP__)
	return "djgpp";
#elif defined(__EMX__)
	return "os2";
#elif defined(USE_EBCDIC)
	return "os390";
#else
	return "posix";
#endif
}

/* set_current_namespace --- set current_namespace and handle memory management */

void
set_current_namespace(const char *new_namespace)
{
	if (current_namespace != awk_namespace)
		efree((void *) current_namespace);

	current_namespace = new_namespace;
}
