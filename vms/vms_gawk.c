/* vms_gawk.c -- parse GAWK command line using DCL syntax

   Copyright (C) 1991-1993, 1996, 2003, 2005, 2011, 2014
   the Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */


/*
 * vms_gawk.c - routines to parse the command line as a native DCL command
 *	       rather than as a foreign command string.
 *							Pat Rankin, Nov'89
 *						[ revised for 2.12, May'91 ]
 *						[ revised for 4.0.0, Feb'11 ]
 */

#include "awk.h"
#include "vms.h"

#define USAGE_PROG_RQRD 1
#define USAGE_FILE_RQRD 2
#define USAGE_BAD_COMBO 3
#define USAGE_RUN_CMD	4
#define STS$M_INHIB_MSG 0x10000000

#define Present(arg)		vmswork(Cli_Present(arg))
#define Get_Value(arg,buf,siz)	vmswork(Cli_Get_Value(arg,buf,siz))

#ifndef __DECC
extern void   GAWK_CMD();	/* created with $ SET COMMAND/OBJECT */
#define gawk_cmd ((const void *)GAWK_CMD) */
#else	/* Use ANSI definitions for DEC C */
#pragma extern_model save
#pragma extern_model strict_refdef
/* (could use globalvalue rather than _refdef if we omit GAWK_CMD's `&') */
extern void  *GAWK_CMD;
#pragma extern_model restore
#define gawk_cmd ((const void *)&GAWK_CMD)
#endif
extern void   _exit(int);
static int    vms_usage(int);

static const char *CmdName = "GAWK";

#define ARG_SIZ 250
union arg_w_prefix {	/* structure used to simplify prepending of "-" */
    char     value[2+ARG_SIZ+1];
    struct {
	char prefix[2];		/* for "-?" */
	char buf[ARG_SIZ];
	char suffix[1];		/* room for '\0' */
    } arg;
};

#define chk_option(qualifier,optname)	\
    if (Present(qualifier))	\
	strcat(strcat(buf.arg.buf, W_cnt++ ? "," : ""), optname)


/* vms_gawk() - parse GAWK command line using DCL and convert it into the */
/*	       appropriate "-arg" values for compatability with GNU code  */
int
vms_gawk()
{
    U_Long sts;
    union arg_w_prefix buf;
    char misc_args[10], *misc_argp;
    int  argc, W_cnt;
    int native_dcl = 1,	/* assume true until we know otherwise */
	short_circ;	/* some options make P1, /commands, /input superfluous */

    /* check "GAWK_P1"--it's required; its presence will tip us off */
    sts = Cli_Present("GAWK_P1");
    if (CondVal(sts) == CondVal(CLI$_SYNTAX)) {
	native_dcl = 0;		/* not invoked via a native command verb */
	/* syntax error indicates that we weren't invoked as a native DCL
	   command, so we'll now attempt to generate a command from the
	   foreign command string and parse that.
	*/
	sts = Cli_Parse_Command(gawk_cmd, "GAWK");	/* (*not* CmdName) */
	if (vmswork(sts))
	    sts = Cli_Present("GAWK_P1");
    }
    short_circ = Present("USAGE") || Present("VERSION") || Present("COPYRIGHT");
    if (vmswork(sts))		/* command parsed successfully */
	v_add_arg(argc = 0, CmdName);	/* save "GAWK" as argv[0] */
    else if (CondVal(sts) == CondVal(CLI$_INSFPRM))
	/* vms_usage() will handle /usage, /version, and /copyright */
	return short_circ ? vms_usage(0)
		: native_dcl ? vms_usage(USAGE_FILE_RQRD) : 0; /* insufficient parameters */
    else if (CondVal(sts) == CondVal(CLI$_CONFLICT))
	return vms_usage(USAGE_BAD_COMBO);  /* conflicting qualifiers (/input+/command) */
#if 0	/* 3.1.2: removed since this can't distinguish RUN vs fork+exec */
    else if (CondVal(sts) == CondVal(CLI$_RUNUSED))
	return vms_usage(USAGE_RUN_CMD);    /* RUN GAWK won't work (no command line) */
#endif
    else
	return 0;	/* forced to rely on original parsing */

    if (short_circ)		/* give usage message & quit or have main() */
	return vms_usage(0);	/* give --version or --copyleft mesg & quit */
    else if (! (Present("PROGRAM") || Present("PROGFILE")) )
	return native_dcl ? vms_usage(USAGE_PROG_RQRD) : 0; /* missing required option */

    misc_argp = misc_args;
    *misc_argp++ = '-';		/* now points at &misc_args[1] */
    if (Present("OPTIMIZE"))
	*misc_argp++ = 'O';
    W_cnt = 0,	buf.arg.buf[0] = '\0';
    strncpy(buf.arg.prefix, "-W", 2);
    if (Present("LINT")) {
	if (!Present("LINT.FATAL") && !Present("LINT.INVALID"))
	    chk_option("LINT.WARN", "lint");
	chk_option("LINT.FATAL", "lint=fatal");
	chk_option("LINT.INVALID", "lint=invalid");
	chk_option("LINT.OLD", "lint-old");	/* distinct option */
    }
    chk_option("POSIX", "posix");
    if (CondVal(Cli_Present("TRADITIONAL")) != CondVal(CLI$_NEGATED))
	chk_option("STRICT", "traditional");  /* /strict is synonym for /traditional */
    if (CondVal(Cli_Present("STRICT")) != CondVal(CLI$_NEGATED))
	chk_option("TRADITIONAL", "traditional");
    chk_option("RE_INTERVAL", "re-interval");  /* only used with /traditional */
    chk_option("SANDBOX", "sandbox");
    /* potentially a problem due to leading "NO" */
    chk_option("NON_DECIMAL_DATA", "non-decimal-data");
    /* note: locale and translation stuff is not supported by vms gawk */
    chk_option("CHARACTERS_AS_BYTES", "characters-as-bytes");
    chk_option("USE_LC_NUMERIC", "use-lc-numeric");
    chk_option("GEN_POT", "gen-pot");
# if 0
    /* /copyright and /version don't reach here anymore (short_circ above) */
    chk_option("COPYRIGHT", "copyright");	/* --copyleft */
    chk_option("VERSION", "version");
# endif
    if (W_cnt > 0)			/* got something */
	v_add_arg(++argc, strdup(buf.value));

#ifdef DEBUG /* most debugging functionality moved to separate DGAWK program */
    if (Present("DEBUG"))
	    *misc_argp++ = 'Y';		/* --parsedebug */
#endif
    *misc_argp = '\0';		/* terminate misc_args[] */
    if (misc_argp > &misc_args[1])	/* got something */
	v_add_arg(++argc, misc_args);	/* store it/them */

    if (Present("PROFILE")) {	    /* /profile[=file] */
	strncpy(buf.arg.prefix, "-p", 2);
	if (Get_Value("PROFILE", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }
    if (Present("DUMP_VARIABLES")) { /* /dump_variables[=file] */
	strncpy(buf.arg.prefix, "-d", 2);
	if (Get_Value("DUMP_VARIABLES", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }
    if (Present("FIELD_SEP")) {     /* field separator */
	strncpy(buf.arg.prefix, "-F", 2);
	if (Get_Value("FIELD_SEP", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }
    if (Present("VARIABLES")) {     /* variables to init prior to BEGIN */
	strncpy(buf.arg.prefix, "-v", 2);
	while (Get_Value("VARIABLES", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }
    /* the relative order of -e and -f args matters; unfortunately,
       we're losing that here... */
    if (Present("MOREPROG")) {	    /* /extra_input=text -> -e text */
	strncpy(buf.arg.prefix, "-e", 2);
	if (Get_Value("MOREPROG", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }
    if (Present("PROGFILE")) {	    /* program files, /input=file -> -f file */
	strncpy(buf.arg.prefix, "-f", 2);
	while (Get_Value("PROGFILE", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
	v_add_arg(++argc, "--");
    } else if (Present("PROGRAM")) {	/* program text, /commands -> 'text' */
	v_add_arg(++argc, "--");
	if (Get_Value("PROGRAM", buf.value, sizeof buf.value))
	    v_add_arg(++argc, strdup(buf.value));
    }

    /* we know that "GAWK_P1" is present [data files and/or 'var=value'] */
    while (Get_Value("GAWK_P1", buf.value, sizeof buf.value))
	v_add_arg(++argc, strdup(buf.value));

    if (Present("OUTPUT")) {	/* let other parser treat this as 'stdout' */
	strncpy(buf.arg.prefix, ">$", 2);
	if (Get_Value("OUTPUT", buf.arg.buf, sizeof buf.arg.buf))
	    v_add_arg(++argc, strdup(buf.value));
    }

    return ++argc;		/*(increment to account for arg[0])*/
}

/* vms_usage() - display one or more messages and then terminate */
static int	/* note: usually doesn't return */
vms_usage( int complaint )
{
    static const char
	*usage_txt = "\n\
usage: %s /COMMANDS=\"awk program text\"  data_file[,data_file,...] \n\
   or  %s /INPUT=awk_file  data_file[,\"Var=value\",data_file,...] \n\
   or  %s /INPUT=(awk_file1,awk_file2,...)  data_file[,...] \n\
   or  %s /INPUT=awk_file /EXTRA_COMMANDS=\"program text\" data_file \n\
",
    *options_txt = "\n\
options: /FIELD_SEPARATOR=\"FS_value\" \n\
   and   /VARIABLES=(\"Var1=value1\",\"Var2=value2\",...) \n\
   and   /OPTIMIZE  /PROFILE[=file]  /DUMP_VARIABLES[=file] \n\
   and   /POSIX  /[NO]TRADITIONAL  /[NO]STRICT  /RE_INTERVAL \n\
   and   /SANDBOX  /NON_DECIMAL_DATA \n\
   and   /LINT[=WARN]  or  /LINT=OLD  or  /LINT=FATAL \n\
   and   /VERSION  /COPYRIGHT  /USAGE \n\
   and   /OUTPUT=out_file \n\
",  /* omitted: /LINT=INVALID /CHARACTERS_AS_BYTES /USE_LC_NUMERIC /GEN_POT */
	*no_prog = "missing required element: /COMMANDS or /INPUT",
	*no_file = "missing required element: data_file \n\
       (use \"SYS$INPUT:\" to read data lines from the terminal)",
	*bad_combo = "invalid combination of qualifiers \n\
       (/INPUT=awk_file and /COMMANDS=\"awk program\" are mutually exclusive)",
	*run_used = "\"RUN\" was used; required command components missing";
    int status, argc;

    /* presence of /usage, /version, or /copyright for feedback+quit
       supersedes absence of required program or data file */
    if (Present("USAGE")) {
	complaint = 0;			/* clean exit */
    } else if (Present("VERSION") || Present("COPYRIGHT")) {
	/* construct a truncated Unix-style command line to control main() */
	v_add_arg(argc=0, CmdName);	/* save "GAWK" as argv[0] */
#if 0
	v_add_arg(++argc, Present("VERSION") ? "-V" : "-C");
#else
	v_add_arg(++argc, "-W");
	v_add_arg(++argc, Present("VERSION") ? "version" : "copyright");
#endif
	/* kludge to suppress 'usage' message from main() */
	v_add_arg(++argc, "--");		/* no more arguments */
	v_add_arg(++argc, "{}");		/* dummy program text */
	v_add_arg(++argc, "NL:");		/* dummy input file */
	return ++argc;			/* count argv[0] too */
    }

    fflush(stdout);
    switch (complaint) {
      case USAGE_PROG_RQRD:
	fprintf(stderr, "\n%%%s-W-%s, %s \n", CmdName, "PROG_RQRD", no_prog);
	status = CLI$_VALREQ | STS$M_INHIB_MSG;
	break;
      case USAGE_FILE_RQRD:
	fprintf(stderr, "\n%%%s-W-%s, %s \n", CmdName, "FILE_RQRD", no_file);
	status = CLI$_INSFPRM | STS$M_INHIB_MSG;
	break;
      case USAGE_BAD_COMBO:
	fprintf(stderr, "\n%%%s-W-%s, %s \n", CmdName, "BAD_COMBO", bad_combo);
	status = CLI$_CONFLICT | STS$M_INHIB_MSG;
	break;
      case USAGE_RUN_CMD:
	fprintf(stderr, "\n%%%s-W-%s, %s \n", CmdName, "RUN_CMD", run_used);
	status = CLI$_NOOPTPRS | STS$M_INHIB_MSG;
	break;
      default:
	status = 1;
	break;
    }
    fprintf(stderr, usage_txt, CmdName, CmdName, CmdName, CmdName);
    fprintf(stderr, options_txt);
    fflush(stderr);

    errno = EVMSERR;
    vaxc$errno = status;
    _exit(status);
    /* NOTREACHED */
    return 0;
}
