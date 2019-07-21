/*
 * msg.c - routines for error messages.
 */

/*
 * Copyright (C) 1986, 1988, 1989, 1991-2001, 2003, 2010-2013, 2017-2019,
 * the Free Software Foundation, Inc.
 *
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 *
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
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

#include "awk.h"

extern FILE *output_fp;
int sourceline = 0;
char *source = NULL;
static const char *srcfile = NULL;
static int srcline;

jmp_buf fatal_tag;
int fatal_tag_valid = 0;

/* err --- print an error message with source line and file and record */

/* VARARGS2 */
void
err(bool isfatal, const char *s, const char *emsg, va_list argp)
{
	char *file;
	const char *me;

	static bool first = true;
	static bool add_src_info = false;
	static long lineno_val = 0;	// Easter Egg

	if (first) {
		first = false;
		add_src_info = (getenv("GAWK_MSG_SRC") != NULL);
		if (! do_traditional) {
			NODE *n = lookup("LINENO");

			if (n != NULL && n->type == Node_var)
				lineno_val = get_number_d(n->var_value);
		}
	}

	(void) fflush(output_fp);
	me = myname;
	(void) fprintf(stderr, "%s: ", me);

	if (srcfile != NULL && add_src_info) {
		fprintf(stderr, "%s:%d:", srcfile, srcline);
		srcfile = NULL;
	}

	if (sourceline > 0) {
		if (source != NULL)
			(void) fprintf(stderr, "%s:", source);
		else
			(void) fprintf(stderr, _("cmd. line:"));

		(void) fprintf(stderr, "%ld: ", sourceline + lineno_val);
	}

#ifdef HAVE_MPFR
	if (FNR_node && is_mpg_number(FNR_node->var_value)) {
		NODE *val;
		val = mpg_update_var(FNR_node);
		assert((val->flags & MPZN) != 0);
		if (mpz_sgn(val->mpg_i) > 0) {
			int len = FILENAME_node->var_value->stlen;
			file = FILENAME_node->var_value->stptr;
			(void) putc('(', stderr);
			if (file)
				(void) fprintf(stderr, "FILENAME=%.*s ", len, file);
			(void) mpfr_fprintf(stderr, "FNR=%Zd) ", val->mpg_i);
		}
	} else
#endif
	if (FNR > 0) {
		int len = FILENAME_node->var_value->stlen;
		file = FILENAME_node->var_value->stptr;
		(void) putc('(', stderr);
		if (file)
			(void) fprintf(stderr, "FILENAME=%.*s ", len, file);
		(void) fprintf(stderr, "FNR=%ld) ", FNR);
	}

	(void) fprintf(stderr, "%s", s);
	vfprintf(stderr, emsg, argp);
	(void) fprintf(stderr, "\n");
	(void) fflush(stderr);

	if (isfatal) {
#ifdef GAWKDEBUG
		// GLIBC 2.27 doesn't necessarily flush on abort. Sigh.
		fflush(NULL);
		abort();
#endif
		gawk_exit(EXIT_FATAL);
	}
}

/* msg --- take a varargs error message and print it */

void
msg(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	err(false, "", mesg, args);
	va_end(args);
}

/* r_warning --- print a warning message */

void
r_warning(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	err(false, _("warning: "), mesg, args);
	va_end(args);
}

void
error(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	err(false, _("error: "), mesg, args);
	va_end(args);
}

/* set_loc --- set location where a fatal error happened */

void
set_loc(const char *file, int line)
{
	srcfile = file;
	srcline = line;

	/* This stupid line keeps some compilers happy: */
	file = srcfile; line = srcline;
}

/* r_fatal --- print a fatal error message */

void
r_fatal(const char *mesg, ...)
{
	va_list args;
	va_start(args, mesg);
	err(true, _("fatal: "), mesg, args);
	va_end(args);
}

/* gawk_exit --- longjmp out if necessary */

void
gawk_exit(int status)
{
	if (fatal_tag_valid) {
		exit_val = status;
		longjmp(fatal_tag, 1);
	}

	final_exit(status);
}

/* final_exit --- run extension exit handlers and exit */

void
final_exit(int status)
{
	/* run any extension exit handlers */
	run_ext_exit_handlers(status);

	/* we could close_io() here */
	close_extensions();

	exit(status);
}
