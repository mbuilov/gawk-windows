/*
 * inplace.c - Provide support for in-place editing.
 */

/*
 * Copyright (C) 2013-2015, 2017, 2018, the Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE 1
#endif
#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
#endif

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

#if ! defined(S_ISREG) && defined(S_IFREG)
#define	S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#ifdef __MINGW32__
# define chown(x,y,z)  (0)
# define link(f1,f2)   rename(f1,f2)
int
mkstemp (char *template)
{
  char *tmp_fname = _mktemp (template);

  if (tmp_fname)
    return _open (tmp_fname, O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
  return -1;
}
#endif

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "inplace extension: version 1.0";

int plugin_is_GPL_compatible;

static struct {
	char *tname;
	int default_stdout;
	int posrc;	/* return code from fgetpos */
	fpos_t pos;
} state = { NULL, -1 };

/*
 * XXX Known problems:
 * 1. Should copy ACL.
 * 2. Not reentrant, so will not work if multiple files are open at
 *    the same time.  I'm not sure this is a meaningful problem in practice.
 */

static void
at_exit(void *data, int exit_status)
{
	(void) data;		/* silence warnings */
	(void) exit_status;	/* silence warnings */
	if (state.tname) {
		unlink(state.tname);
		gawk_free(state.tname);
		state.tname = NULL;
	}
}

/*
 * N.B. Almost everything is a fatal error because this feature is typically
 * used for one-liners where the user is not going to be worrying about
 * checking errors.  If anything unexpected occurs, we want to abort
 * immediately!
 */

static int
invalid_filename(const awk_string_t *filename)
{
	return filename->len == 0 ||
		(filename->len == 1 && *filename->str == '-');
}

/* do_inplace_begin --- start in-place editing */

static awk_value_t *
do_inplace_begin(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename;
	struct stat sbuf;
	int fd;

	assert(result != NULL);
	fflush(stdout);

	if (state.tname)
		fatal(ext_id, _("inplace::begin: in-place editing already active"));

	if (nargs != 2)
		fatal(ext_id, _("inplace::begin: expects 2 arguments but called with %d"), nargs);

	if (! get_argument(0, AWK_STRING, &filename))
		fatal(ext_id, _("inplace::begin: cannot retrieve 1st argument as a string filename"));

	/*
	 * N.B. In the current implementation, the 2nd suffix arg is not used
	 * in this function.  It is used only in the inplace_end function.
	 */

	if (invalid_filename(&filename.str_value)) {
		warning(ext_id, _("inplace::begin: disabling in-place editing for invalid FILENAME `%s'"),
			filename.str_value.str);
		unset_ERRNO();
		return make_number(-1, result);
	}

	if (stat(filename.str_value.str, & sbuf) < 0) {
		warning(ext_id, _("inplace::begin: Cannot stat `%s' (%s)"),
			filename.str_value.str, strerror(errno));
		update_ERRNO_int(errno);
		return make_number(-1, result);
	}

	if (! S_ISREG(sbuf.st_mode)) {
		warning(ext_id, _("inplace::begin: `%s' is not a regular file"),
			filename.str_value.str);
		unset_ERRNO();
		return make_number(-1, result);
	}

	/* create a temporary file to which to redirect stdout */
	emalloc(state.tname, char *, filename.str_value.len+14, "do_inplace_begin");
	sprintf(state.tname, "%s.gawk.XXXXXX", filename.str_value.str);

	if ((fd = mkstemp(state.tname)) < 0)
		fatal(ext_id, _("inplace::begin: mkstemp(`%s') failed (%s)"),
			state.tname, strerror(errno));

	/* N.B. chown/chmod should be more portable than fchown/fchmod */
	if (chown(state.tname, sbuf.st_uid, sbuf.st_gid) < 0) {
		/* jumping through hoops to silence gcc and clang. :-( */
		int junk;
		junk = chown(state.tname, -1, sbuf.st_gid);
		++junk;
	}

	if (chmod(state.tname, sbuf.st_mode) < 0)
		fatal(ext_id, _("inplace::begin: chmod failed (%s)"),
			strerror(errno));

	fflush(stdout);
	/* N.B. fgetpos fails when stdout is a tty */
	state.posrc = fgetpos(stdout, &state.pos);
	if ((state.default_stdout = dup(STDOUT_FILENO)) < 0)
		fatal(ext_id, _("inplace::begin: dup(stdout) failed (%s)"),
			strerror(errno));
	if (dup2(fd, STDOUT_FILENO) < 0)
		fatal(ext_id, _("inplace::begin: dup2(%d, stdout) failed (%s)"),
			fd, strerror(errno));
	if (close(fd) < 0)
		fatal(ext_id, _("inplace::begin: close(%d) failed (%s)"),
			fd, strerror(errno));
	rewind(stdout);
	return make_number(0, result);
}

/* do_inplace_end --- finish in-place editing */

static awk_value_t *
do_inplace_end(int nargs, awk_value_t *result, struct awk_ext_func *unused)
{
	awk_value_t filename, suffix;

	assert(result != NULL);

	if (nargs != 2)
		fatal(ext_id, _("inplace::end: expects 2 arguments but called with %d"), nargs);

	if (! get_argument(0, AWK_STRING, &filename))
		fatal(ext_id, _("inplace::end: cannot retrieve 1st argument as a string filename"));

	if (! get_argument(1, AWK_STRING, &suffix))
		suffix.str_value.str = NULL;

	if (! state.tname) {
		if (! invalid_filename(&filename.str_value))
			warning(ext_id, _("inplace::end: in-place editing not active"));
		return make_number(0, result);
	}

	fflush(stdout);
	if (dup2(state.default_stdout, STDOUT_FILENO) < 0)
		fatal(ext_id, _("inplace::end: dup2(%d, stdout) failed (%s)"),
			state.default_stdout, strerror(errno));
	if (close(state.default_stdout) < 0)
		fatal(ext_id, _("inplace::end: close(%d) failed (%s)"),
			state.default_stdout, strerror(errno));
	state.default_stdout = -1;
	if (state.posrc == 0 && fsetpos(stdout, &state.pos) < 0)
		fatal(ext_id, _("inplace::end: fsetpos(stdout) failed (%s)"),
			strerror(errno));

	if (suffix.str_value.str && suffix.str_value.str[0]) {
		/* backup requested */
		char *bakname;

		emalloc(bakname, char *, filename.str_value.len+suffix.str_value.len+1,
			"do_inplace_end");
		sprintf(bakname, "%s%s",
			filename.str_value.str, suffix.str_value.str);
		unlink(bakname); /* if backup file exists already, remove it */
		if (link(filename.str_value.str, bakname) < 0)
			fatal(ext_id, _("inplace::end: link(`%s', `%s') failed (%s)"),
				filename.str_value.str, bakname, strerror(errno));
		gawk_free(bakname);
	}

#ifdef __MINGW32__
	unlink(filename.str_value.str);
#endif

	if (rename(state.tname, filename.str_value.str) < 0)
		fatal(ext_id, _("inplace::end: rename(`%s', `%s') failed (%s)"),
			state.tname, filename.str_value.str, strerror(errno));
	gawk_free(state.tname);
	state.tname = NULL;
	return make_number(0, result);
}

static awk_ext_func_t func_table[] = {
	{ "begin", do_inplace_begin, 2, 2, awk_false, NULL },
	{ "end", do_inplace_end, 2, 2, awk_false, NULL },
};

static awk_bool_t init_inplace(void)
{
	awk_atexit(at_exit, NULL);
	return awk_true;
}

static awk_bool_t (*init_func)(void) = init_inplace;

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, inplace, "inplace")
