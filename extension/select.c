/*
 * select.c - Builtin functions to provide select I/O multiplexing.
 */

/*
 * Copyright (C) 2013 the Free Software Foundation, Inc.
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

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;
static const char *ext_version = "select extension: version 1.0";
static awk_bool_t (*init_func)(void) = NULL;

int plugin_is_GPL_compatible;

#if defined(HAVE_SELECT) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

static const char *const signum2name[] = {
#define init_sig(A, B, C) [A] = B,
#include "siglist.h"
#undef init_sig
};
#define NUMSIG	sizeof(signum2name)/sizeof(signum2name[0])

#define MIN_VALID_SIGNAL	1		/* 0 is not allowed! */
/*
 * We would like to use NSIG, but I think this seems to be a BSD'ism that is not
 * POSIX-compliant.  It is used internally by glibc, but not always
 * available.  We add a buffer to the maximum number in the provided mapping
 * in case the list is not comprehensive:
 */
#define MAX_VALID_SIGNAL	(NUMSIG+100)
#define IS_VALID_SIGNAL(X) \
	(((X) >= MIN_VALID_SIGNAL) && ((X) <= MAX_VALID_SIGNAL))

static int
signame2num(const char *name)
{
	size_t i;

	if (strncasecmp(name, "sig", 3) == 0)
		/* skip "sig" prefix */
		name += 3;
	for (i = MIN_VALID_SIGNAL; i < NUMSIG; i++) {
		if (signum2name[i] && ! strcasecmp(signum2name[i], name))
			return i;
	}
	return -1;
}

static volatile struct {
   int flag;
   sigset_t mask;
} caught;

static void
signal_handler(int signum)
{
	/*
	 * All signals should be blocked, so we do not have to worry about
	 * whether sigaddset is thread-safe.  It is documented to be
	 * async-signal-safe.
	 */
	sigaddset(& caught.mask, signum);
	caught.flag = 1;
}

static int
integer_string(const char *s, long *x)
{
	char *endptr;

	*x = strtol(s, & endptr, 10);
	return ((endptr != s) && (*endptr == '\0')) ? 0 : -1;
}

static int
get_signal_number(awk_value_t signame)
{
	int x;

	switch (signame.val_type) {
	case AWK_NUMBER:
		x = signame.num_value;
		if ((x != signame.num_value) || ! IS_VALID_SIGNAL(x)) {
			update_ERRNO_string(_("select_signal: invalid signal number"));
			return -1;
		}
		return x;
	case AWK_STRING:
		if ((x = signame2num(signame.str_value.str)) >= 0)
			return x;
		{
			long z;
			if ((integer_string(signame.str_value.str, &z) == 0) && IS_VALID_SIGNAL(z))
				return z;
		}
		update_ERRNO_string(_("select_signal: invalid signal name"));
		return -1;
	default:
		update_ERRNO_string(_("select_signal: signal name argument must be string or numeric"));
		return -1;
	}
}

/*  do_signal --- trap signals */

static awk_value_t *
do_signal(int nargs, awk_value_t *result)
{
#ifdef HAVE_SIGACTION
	awk_value_t signame, disposition;
	int signum;
	struct sigaction sa;

	if (! get_argument(0, AWK_UNDEFINED, & signame)) {
		update_ERRNO_string(_("select_signal: missing required signal name argument"));
		return make_number(-1, result);
	}
	if ((signum = get_signal_number(signame)) < 0)
		return make_number(-1, result);
	if (! get_argument(1, AWK_STRING, & disposition)) {
		update_ERRNO_string(_("select_signal: missing required signal disposition argument"));
		return make_number(-1, result);
	}
	if (strcasecmp(disposition.str_value.str, "default") == 0)
		sa.sa_handler = SIG_DFL;
	else if (strcasecmp(disposition.str_value.str, "ignore") == 0)
		sa.sa_handler = SIG_IGN;
	else if (strcasecmp(disposition.str_value.str, "trap") == 0)
		sa.sa_handler = signal_handler;
	else {
		update_ERRNO_string(_("select_signal: invalid disposition argument"));
		return make_number(-1, result);
	}
	sigfillset(& sa.sa_mask);	/* block all signals in handler */
	sa.sa_flags = SA_RESTART;
	if (sigaction(signum, &sa, NULL) < 0) {
		update_ERRNO_int(errno);
		return make_number(-1, result);
	}
	return make_number(0, result);
#else
	update_ERRNO_string(_("select_signal: not supported on this platform"));
	return make_number(-1, result);
#endif
}

/*  do_select --- I/O multiplexing */

static awk_value_t *
do_select(int nargs, awk_value_t *result)
{
	static const char *argname[] = { "read", "write", "except" };
	struct {
		awk_value_t array;
		awk_flat_array_t *flat;
		fd_set bits;
		int *array2fd;
	} fds[3];
	awk_value_t timeout_arg;
	int i;
	struct timeval maxwait;
	struct timeval *timeout;
	int nfds = 0;
	int rc;
	awk_value_t sigarr;
	int dosig = 0;

	if (do_lint && nargs > 5)
		lintwarn(ext_id, _("select: called with too many arguments"));

#define EL	fds[i].flat->elements[j]
	if (nargs == 5) {
		dosig = 1;
		if (! get_argument(4, AWK_ARRAY, &sigarr)) {
			warning(ext_id, _("select: the signal argument must be an array"));
			update_ERRNO_string(_("select: bad signal parameter"));
			return make_number(-1, result);
		}
		clear_array(sigarr.array_cookie);
	}

	for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
		size_t j;

		if (! get_argument(i, AWK_ARRAY, & fds[i].array)) {
			warning(ext_id, _("select: bad array parameter `%s'"), argname[i]);
			update_ERRNO_string(_("select: bad array parameter"));
			return make_number(-1, result);
		}
		/* N.B. flatten_array fails for empty arrays, so that's OK */
		FD_ZERO(&fds[i].bits);
		if (flatten_array(fds[i].array.array_cookie, &fds[i].flat)) {
			emalloc(fds[i].array2fd, int *, fds[i].flat->count*sizeof(int), "select");
			for (j = 0; j < fds[i].flat->count; j++) {
				long x;
				fds[i].array2fd[j] = -1;
				/* note: the index is always delivered as a string */

				if (((EL.value.val_type == AWK_UNDEFINED) || ((EL.value.val_type == AWK_STRING) && ! EL.value.str_value.len)) && (integer_string(EL.index.str_value.str, &x) == 0) && (x >= 0))
					fds[i].array2fd[j] = x;
				else if (EL.value.val_type == AWK_STRING) {
					const awk_input_buf_t *buf;
					if ((buf = get_file(EL.index.str_value.str, EL.index.str_value.len, EL.value.str_value.str, EL.value.str_value.len)) != NULL)
						fds[i].array2fd[j] = buf->fd;
					else
						warning(ext_id, _("select: get_file(`%s', `%s') failed in `%s' array"), EL.index.str_value.str, EL.value.str_value.str, argname[i]);
				}
				else
					warning(ext_id, _("select: command type should be a string for `%s' in `%s' array"), EL.index.str_value.str, argname[i]);
				if (fds[i].array2fd[j] < 0) {
					update_ERRNO_string(_("select: get_file failed"));
					if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
						warning(ext_id, _("select: release_flattened_array failed"));
					free(fds[i].array2fd);
					return make_number(-1, result);
				}
				FD_SET(fds[i].array2fd[j], &fds[i].bits);
				if (nfds <= fds[i].array2fd[j])
					nfds = fds[i].array2fd[j]+1;
			}
		}
		else
			fds[i].flat = NULL;
	}
	if (dosig && caught.flag) {
		/* take a quick poll, but do not block, since signals have been trapped */
		maxwait.tv_sec = maxwait.tv_usec = 0;
		timeout = &maxwait;
	}
	else if (get_argument(3, AWK_NUMBER, &timeout_arg)) {
		double secs = timeout_arg.num_value;
		if (secs < 0) {
			warning(ext_id, _("select: treating negative timeout as zero"));
			secs = 0;
		}
		maxwait.tv_sec = secs;
		maxwait.tv_usec = (secs-(double)maxwait.tv_sec)*1000000.0;
		timeout = &maxwait;
	} else
		timeout = NULL;

	if ((rc = select(nfds, &fds[0].bits, &fds[1].bits, &fds[2].bits, timeout)) < 0)
		update_ERRNO_int(errno);

	if (dosig && caught.flag) {
		int i;
		sigset_t set, oldset, trapped;
		sigfillset(& set);
		sigprocmask(SIG_SETMASK, &set, &oldset);
		trapped = caught.mask;
		sigemptyset(& caught.mask);
		caught.flag = 0;
		sigprocmask(SIG_SETMASK, &oldset, NULL);
		/* populate sigarr with trapped signals */
		/*
		 * XXX this is very inefficient!  Note that get_signal_number
		 * ensures that we trap only signals between MIN_VALID_SIGNAL
		 * and MAX_VALID_SIGNAL.
		 */
		for (i = MIN_VALID_SIGNAL; i <= MAX_VALID_SIGNAL; i++) {
			if (sigismember(& trapped, i) > 0) {
				awk_value_t idx, val;
				if ((i < NUMSIG) && signum2name[i])
					set_array_element(sigarr.array_cookie, make_number(i, &idx), make_const_string(signum2name[i], strlen(signum2name[i]), &val));
				else
					set_array_element(sigarr.array_cookie, make_number(i, &idx), make_null_string(&val));
			}
		}
	}

	if (rc < 0) {
		/* bit masks are undefined, so delete all array entries */
		for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
			if (fds[i].flat) {
				size_t j;
				for (j = 0; j < fds[i].flat->count; j++)
					EL.flags |= AWK_ELEMENT_DELETE;
				if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
					warning(ext_id, _("select: release_flattened_array failed"));
				free(fds[i].array2fd);
			}
		}
		return make_number(rc, result);
	}

	for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
		if (fds[i].flat) {
			size_t j;
			/* remove array elements not set in the bit mask */
			for (j = 0; j < fds[i].flat->count; j++) {
				if (! FD_ISSET(fds[i].array2fd[j], &fds[i].bits))
					EL.flags |= AWK_ELEMENT_DELETE;
			}
			if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
				warning(ext_id, _("select: release_flattened_array failed"));
			free(fds[i].array2fd);
		}
	}
#undef EL

	/* Set the return value */
	return make_number(rc, result);
}

static int
set_non_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL);
	int rc = fcntl(fd, F_SETFL, (flags|O_NONBLOCK));
	if (rc < 0)
		update_ERRNO_int(errno);
	return rc;
}

/*  do_set_non_blocking --- Set a file to be non-blocking */

static awk_value_t *
do_set_non_blocking(int nargs, awk_value_t *result)
{
	awk_value_t cmd, cmdtype;
	int fd;

	if (do_lint && nargs > 2)
		lintwarn(ext_id, _("set_non_blocking: called with too many arguments"));
	if (get_argument(0, AWK_NUMBER, & cmd) &&
		(cmd.num_value == (fd = cmd.num_value)) && 
		! get_argument(1, AWK_STRING, & cmdtype))
		return make_number(set_non_blocking(fd), result);
	else if (get_argument(0, AWK_STRING, & cmd) &&
		get_argument(1, AWK_STRING, & cmdtype)) {
		const awk_input_buf_t *buf;
		if ((buf = get_file(cmd.str_value.str, cmd.str_value.len, cmdtype.str_value.str, cmdtype.str_value.len)) != NULL)
			return make_number(set_non_blocking(buf->fd), result);
		warning(ext_id, _("set_non_blocking: get_file(`%s', `%s') failed"), cmd.str_value.str, cmdtype.str_value.str);
	} else if (do_lint) {
		if (nargs < 2)
			lintwarn(ext_id, _("set_non_blocking: called with too few arguments"));
		else
			lintwarn(ext_id, _("set_non_blocking: called with inappropriate argument(s)"));
	}
	return make_number(-1, result);
}

static awk_ext_func_t func_table[] = {
	{ "select", do_select, 5 },
	{ "select_signal", do_signal, 2 },
	{ "set_non_blocking", do_set_non_blocking, 2 },
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, select, "")
