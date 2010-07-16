/*
 * io.c --- routines for dealing with input and output and records
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-2003 the Free Software Foundation, Inc.
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

#ifdef HAVE_SYS_PARAM_H
#undef RE_DUP_MAX	/* avoid spurious conflict w/regex.h */
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifndef O_RDONLY
#include <fcntl.h>
#endif
#ifndef O_ACCMODE
#define O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)
#endif

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#ifdef HAVE_SOCKETS
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#else
#include <socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#else
#include <in.h>
#endif /* HAVE_NETINET_IN_H */
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */
#endif /* HAVE_SOCKETS */

#ifdef __EMX__
#include <process.h>
#endif

#ifndef ENFILE
#define ENFILE EMFILE
#endif

extern int MRL;

#ifdef HAVE_SOCKETS
enum inet_prot { INET_NONE, INET_TCP, INET_UDP, INET_RAW };

#ifndef SHUT_RD
#define SHUT_RD		0
#endif

#ifndef SHUT_WR
#define SHUT_WR		1
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR	2
#endif

#endif /* HAVE_SOCKETS */

#ifdef atarist
#include <stddef.h>
#endif

#if defined(GAWK_AIX)
#undef TANDEM	/* AIX defines this in one of its header files */
#undef MSDOS	/* For good measure, */
#undef WIN32	/* yes, I'm paranoid */
#endif

#if defined(MSDOS) || defined(WIN32) || defined(TANDEM)
#define PIPES_SIMULATED
#endif

typedef enum { CLOSE_ALL, CLOSE_TO, CLOSE_FROM } two_way_close_type;

static IOBUF *nextfile P((int skipping));
static int inrec P((IOBUF *iop));
static int iop_close P((IOBUF *iop));
struct redirect *redirect P((NODE *tree, int *errflg));
static void close_one P((void));
static int close_redir P((struct redirect *rp, int exitwarn, two_way_close_type how));
#ifndef PIPES_SIMULATED
static int wait_any P((int interesting));
#endif
static IOBUF *gawk_popen P((const char *cmd, struct redirect *rp));
static IOBUF *iop_open P((const char *file, const char *how, IOBUF *buf));
static IOBUF *iop_alloc P((int fd, const char *name, IOBUF *buf));
static int gawk_pclose P((struct redirect *rp));
static int do_pathopen P((const char *file));
static int str2mode P((const char *mode));
static void spec_setup P((IOBUF *iop, int len, int allocate));
static int specfdopen P((IOBUF *iop, const char *name, const char *mode));
static int pidopen P((IOBUF *iop, const char *name, const char *mode));
static int useropen P((IOBUF *iop, const char *name, const char *mode));
static int two_way_open P((const char *str, struct redirect *rp));
static int pty_vs_pipe P((const char *command));

static int rs1_get_a_record P((char **out, IOBUF *iop, int rs, Regexp *RSre, int *errcode));
static int rsnull_get_a_record P((char **out, IOBUF *iop, int rs, Regexp *RSre, int *errcode));
static int rsre_get_a_record P((char **out, IOBUF *iop, int rs, Regexp *RSre, int *errcode));

static int (*get_a_record)P((char **out, IOBUF *iop, int rs, Regexp *RSre, int *errcode)) = rs1_get_a_record;

#if defined(HAVE_POPEN_H)
#include "popen.h"
#endif

static struct redirect *red_head = NULL;
static NODE *RS;
static Regexp *RS_re_yes_case;
static Regexp *RS_re_no_case;
static Regexp *RS_regexp;

int RS_is_null;

extern int output_is_tty;
extern NODE *ARGC_node;
extern NODE *ARGV_node;
extern NODE *ARGIND_node;
extern NODE *ERRNO_node;
extern NODE **fields_arr;

static jmp_buf filebuf;		/* for do_nextfile() */

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__EMX__)
static const char *
binmode(const char *mode)
{
	switch (mode[0]) {
	case 'r':
		if ((BINMODE & 1) != 0)
			mode = "rb";
		break;
	case 'w':
	case 'a':
		if ((BINMODE & 2) != 0)
			mode = (mode[0] == 'w' ? "wb" : "ab");
		break;
	}
	return mode;
}
#else
#define binmode(mode)	(mode)
#endif

#ifdef VMS
/* File pointers have an extra level of indirection, and there are cases where
   `stdin' can be null.  That can crash gawk if fileno() is used as-is.  */
static int vmsrtl_fileno P((FILE *));
static int vmsrtl_fileno(fp) FILE *fp; { return fileno(fp); }
#undef fileno
#define fileno(FP) (((FP) && *(FP)) ? vmsrtl_fileno(FP) : -1)
#endif	/* VMS */

/* do_nextfile --- implement gawk "nextfile" extension */

void
do_nextfile()
{
	(void) nextfile(TRUE);
	longjmp(filebuf, 1);
}

/* nextfile --- move to the next input data file */

static IOBUF *
nextfile(int skipping)
{
	static long i = 1;
	static int files = 0;
	NODE *arg;
	static IOBUF *curfile = NULL;
	static IOBUF mybuf;
	const char *fname;

	if (skipping) {
		if (curfile != NULL)
			iop_close(curfile);
		curfile = NULL;
		return NULL;
	}
	if (curfile != NULL) {
		if ((curfile->flag & IOP_AT_EOF) != 0 && curfile->off >= curfile->dataend) {
			(void) iop_close(curfile);
			curfile = NULL;
		} else
			return curfile;
	}
	for (; i < (long) (ARGC_node->lnode->numbr); i++) {
		arg = *assoc_lookup(ARGV_node, tmp_number((AWKNUM) i), FALSE);
		if (arg->stlen == 0)
			continue;
		arg->stptr[arg->stlen] = '\0';
		if (! do_traditional) {
			unref(ARGIND_node->var_value);
			ARGIND_node->var_value = make_number((AWKNUM) i);
		}
		if (! arg_assign(arg->stptr, FALSE)) {
			files++;
			fname = arg->stptr;
			curfile = iop_open(fname, binmode("r"), &mybuf);
			if (curfile == NULL)
				goto give_up;
			curfile->flag |= IOP_NOFREE_OBJ;
			/* This is a kludge.  */
			unref(FILENAME_node->var_value);
			FILENAME_node->var_value = dupnode(arg);
			FNR = 0;
			i++;
			break;
		}
	}
	if (files == 0) {
		files++;
		/* no args. -- use stdin */
		/* FNR is init'ed to 0 */
		FILENAME_node->var_value = make_string("-", 1);
		fname = "-";
		curfile = iop_open(fname, binmode("r"), &mybuf);
		if (curfile == NULL)
			goto give_up;
		curfile->flag |= IOP_NOFREE_OBJ;
	}
	return curfile;

 give_up:
	fatal(_("cannot open file `%s' for reading (%s)"),
		fname, strerror(errno));
	/* NOTREACHED */
	return 0;
}

/* set_FNR --- update internal FNR from awk variable */

void
set_FNR()
{
	FNR = (long) FNR_node->var_value->numbr;
}

/* set_NR --- update internal NR from awk variable */

void
set_NR()
{
	NR = (long) NR_node->var_value->numbr;
}

/* inrec --- This reads in a record from the input file */

static int
inrec(IOBUF *iop)
{
	char *begin;
	register int cnt;
	int retval = 0;

	if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend)
		cnt = EOF;
	else if ((iop->flag & IOP_CLOSED) != 0)
		cnt = EOF;
	else
		cnt = get_a_record(&begin, iop, RS->stptr[0], RS_regexp, NULL);

	if (cnt == EOF) {
		cnt = 0;
		retval = 1;
	} else {
		NR += 1;
		FNR += 1;
		set_record(begin, cnt);
	}

	return retval;
}

/* iop_close --- close an open IOP */

static int
iop_close(IOBUF *iop)
{
	int ret;

	if (iop == NULL)
		return 0;
	errno = 0;

	iop->flag &= ~IOP_AT_EOF;
	iop->flag |= IOP_CLOSED;	/* there may be dangling pointers */
	iop->dataend = NULL;
#ifdef _CRAY
	/* Work around bug in UNICOS popen */
	if (iop->fd < 3)
		ret = 0;
	else
#endif
	/* save these for re-use; don't free the storage */
	if ((iop->flag & IOP_IS_INTERNAL) != 0) {
		iop->off = iop->buf;
		iop->end = iop->buf + strlen(iop->buf);
		iop->count = 0;
		return 0;
	}

	/* Don't close standard files or else crufty code elsewhere will lose */
	if (iop->fd == fileno(stdin)
	    || iop->fd == fileno(stdout)
	    || iop->fd == fileno(stderr))
		ret = 0;
	else
		ret = close(iop->fd);

	if (ret == -1)
		warning(_("close of fd %d (`%s') failed (%s)"), iop->fd,
				iop->name, strerror(errno));
	if ((iop->flag & IOP_NO_FREE) == 0) {
		/*
		 * Be careful -- $0 may still reference the buffer even though
		 * an explicit close is being done; in the future, maybe we
		 * can do this a bit better.
		 */
		if (iop->buf) {
			if ((fields_arr[0]->stptr >= iop->buf)
			    && (fields_arr[0]->stptr < (iop->buf + iop->size))) {
				NODE *t;
	
				t = make_string(fields_arr[0]->stptr,
						fields_arr[0]->stlen);
				unref(fields_arr[0]);
				fields_arr[0] = t;
				/*
				 * 1/27/2003: This used to be here:
				 *
				 * reset_record();
				 *
				 * Don't do that; reset_record() throws away all fields,
				 * saves FS etc.  We just need to make sure memory isn't
				 * corrupted and that references to $0 and fields work.
				 */
			}
			free(iop->buf);
			iop->buf = NULL;
		}
		if ((iop->flag & IOP_NOFREE_OBJ) == 0)
			free((char *) iop);
	}
	return ret == -1 ? 1 : 0;
}

/* do_input --- the main input processing loop */

void
do_input()
{
	IOBUF *iop;
	extern int exiting;
	int rval1, rval2, rval3;

	(void) setjmp(filebuf);	/* for `nextfile' */

	while ((iop = nextfile(FALSE)) != NULL) {
		/*
		 * This was:
		if (inrec(iop) == 0)
			while (interpret(expression_value) && inrec(iop) == 0)
				continue;
		 * Now expand it out for ease of debugging.
		 */
		rval1 = inrec(iop);
		if (rval1 == 0) {
			for (;;) {
				rval2 = rval3 = -1;	/* for debugging */
				rval2 = interpret(expression_value);
				if (rval2 != 0)
					rval3 = inrec(iop);
				if (rval2 == 0 || rval3 != 0)
					break;
			}
		}
		if (exiting)
			break;
	}
}

/* redflags2str --- turn redirection flags into a string, for debugging */

const char *
redflags2str(int flags)
{
	static const struct flagtab redtab[] = {
		{ RED_FILE,	"RED_FILE" },
		{ RED_PIPE,	"RED_PIPE" },
		{ RED_READ,	"RED_READ" },
		{ RED_WRITE,	"RED_WRITE" },
		{ RED_APPEND,	"RED_APPEND" },
		{ RED_NOBUF,	"RED_NOBUF" },
		{ RED_EOF,	"RED_EOF" },
		{ RED_TWOWAY,	"RED_TWOWAY" },
		{ RED_PTY,	"RED_PTY" },
		{ RED_SOCKET,	"RED_SOCKET" },
		{ RED_TCP,	"RED_TCP" },
		{ 0, NULL }
	};

	return genflags2str(flags, redtab);
}

/* redirect --- Redirection for printf and print commands */

struct redirect *
redirect(NODE *tree, int *errflg)
{
	register NODE *tmp;
	register struct redirect *rp;
	register char *str;
	int tflag = 0;
	int outflag = 0;
	const char *direction = "to";
	const char *mode;
	int fd;
	const char *what = NULL;

	switch (tree->type) {
	case Node_redirect_append:
		tflag = RED_APPEND;
		/* FALL THROUGH */
	case Node_redirect_output:
		outflag = (RED_FILE|RED_WRITE);
		tflag |= outflag;
		if (tree->type == Node_redirect_output)
			what = ">";
		else
			what = ">>";
		break;
	case Node_redirect_pipe:
		tflag = (RED_PIPE|RED_WRITE);
		what = "|";
		break;
	case Node_redirect_pipein:
		tflag = (RED_PIPE|RED_READ);
		what = "|";
		break;
	case Node_redirect_input:
		tflag = (RED_FILE|RED_READ);
		what = "<";
		break;
	case Node_redirect_twoway:
		tflag = (RED_READ|RED_WRITE|RED_TWOWAY);
		what = "|&";
		break;
	default:
		fatal(_("invalid tree type %s in redirect()"),
					nodetype2str(tree->type));
		break;
	}
	tmp = tree_eval(tree->subnode);
	if (do_lint && (tmp->flags & STRCUR) == 0)
		lintwarn(_("expression in `%s' redirection only has numeric value"),
			what);
	tmp = force_string(tmp);
	str = tmp->stptr;

	if (str == NULL || *str == '\0')
		fatal(_("expression for `%s' redirection has null string value"),
			what);

	if (do_lint
	    && (STREQN(str, "0", tmp->stlen) || STREQN(str, "1", tmp->stlen)))
		lintwarn(_("filename `%s' for `%s' redirection may be result of logical expression"), str, what);

#ifdef HAVE_SOCKETS
	if (STREQN(str, "/inet/", 6)) {
		tflag |= RED_SOCKET;
		if (STREQN(str + 6, "tcp/", 4))
			tflag |= RED_TCP;	/* use shutdown when closing */
	}
#endif /* HAVE_SOCKETS */

	for (rp = red_head; rp != NULL; rp = rp->next) {
		if (strlen(rp->value) == tmp->stlen
		    && STREQN(rp->value, str, tmp->stlen)
		    && ((rp->flag & ~(RED_NOBUF|RED_EOF|RED_PTY)) == tflag
			|| (outflag != 0
			    && (rp->flag & (RED_FILE|RED_WRITE)) == outflag))) {

			int rpflag = (rp->flag & ~(RED_NOBUF|RED_EOF|RED_PTY));
			int newflag = (tflag & ~(RED_NOBUF|RED_EOF|RED_PTY));

			if (do_lint && rpflag != newflag)
				lintwarn(
		_("unnecessary mixing of `>' and `>>' for file `%.*s'"),
					tmp->stlen, rp->value);

			break;
		}
	}

	if (rp == NULL) {
		emalloc(rp, struct redirect *, sizeof(struct redirect),
			"redirect");
		emalloc(str, char *, tmp->stlen+1, "redirect");
		memcpy(str, tmp->stptr, tmp->stlen);
		str[tmp->stlen] = '\0';
		rp->value = str;
		rp->flag = tflag;
		rp->fp = NULL;
		rp->iop = NULL;
		rp->pid = 0;	/* unlikely that we're worried about init */
		rp->status = 0;
		/* maintain list in most-recently-used first order */
		if (red_head != NULL)
			red_head->prev = rp;
		rp->prev = NULL;
		rp->next = red_head;
		red_head = rp;
	} else
		str = rp->value;	/* get \0 terminated string */

	while (rp->fp == NULL && rp->iop == NULL) {
		if (rp->flag & RED_EOF)
			/*
			 * encountered EOF on file or pipe -- must be cleared
			 * by explicit close() before reading more
			 */
			return rp;
		mode = NULL;
		errno = 0;
		switch (tree->type) {
		case Node_redirect_output:
			mode = binmode("w");
			if ((rp->flag & RED_USED) != 0)
				mode = (rp->mode[1] == 'b') ? "ab" : "a";
			break;
		case Node_redirect_append:
			mode = binmode("a");
			break;
		case Node_redirect_pipe:
			/* synchronize output before new pipe */
			(void) flush_io();

			os_restore_mode(fileno(stdin));
			if ((rp->fp = popen(str, binmode("w"))) == NULL)
				fatal(_("can't open pipe `%s' for output (%s)"),
					str, strerror(errno));
			/* set close-on-exec */
			os_close_on_exec(fileno(rp->fp), str, "pipe", "to");
			rp->flag |= RED_NOBUF;
			break;
		case Node_redirect_pipein:
			direction = "from";
			if (gawk_popen(str, rp) == NULL)
				fatal(_("can't open pipe `%s' for input (%s)"),
					str, strerror(errno));
			break;
		case Node_redirect_input:
			direction = "from";
			rp->iop = iop_open(str, binmode("r"), NULL);
			break;
		case Node_redirect_twoway:
			direction = "to/from";
			if (! two_way_open(str, rp)) {
#ifdef HAVE_SOCKETS
				/* multiple messages make life easier for translators */
				if (STREQN(str, "/inet/", 6))
					fatal(_("can't open two way socket `%s' for input/output (%s)"),
						str, strerror(errno));
				else
#endif
					fatal(_("can't open two way pipe `%s' for input/output (%s)"),
						str, strerror(errno));
			}
			break;
		default:
			cant_happen();
		}
		if (mode != NULL) {
			errno = 0;
			fd = devopen(str, mode);
			if (fd > INVALID_HANDLE) {
				if (fd == fileno(stdin))
					rp->fp = stdin;
				else if (fd == fileno(stdout))
					rp->fp = stdout;
				else if (fd == fileno(stderr))
					rp->fp = stderr;
				else {
#if defined(F_GETFL) && defined(O_APPEND)
					int fd_flags;

					fd_flags = fcntl(fd, F_GETFL);
					if (fd_flags != -1 && (fd_flags & O_APPEND) == O_APPEND)
						rp->fp = fdopen(fd, binmode("a"));
					else
#endif
						rp->fp = fdopen(fd, (const char *) mode);
					rp->mode = (const char *) mode;
					/* don't leak file descriptors */
					if (rp->fp == NULL)
						close(fd);
				}
				if (rp->fp != NULL && isatty(fd))
					rp->flag |= RED_NOBUF;
				/* Move rp to the head of the list. */
				if (red_head != rp) {
					if ((rp->prev->next = rp->next) != NULL)
						rp->next->prev = rp->prev;
					red_head->prev = rp;
					rp->prev = NULL;
					rp->next = red_head;
					red_head = rp;
				}
			}
		}
		if (rp->fp == NULL && rp->iop == NULL) {
			/* too many files open -- close one and try again */
			if (errno == EMFILE || errno == ENFILE)
				close_one();
#if defined __MINGW32__ || defined __sun
			else if (errno == 0)    /* HACK! */
				close_one();
#endif
#ifdef VMS
			/* Alpha/VMS V7.1's C RTL is returning this instead
			   of EMFILE (haven't tried other post-V6.2 systems) */
#define SS$_EXQUOTA 0x001C
			else if (errno == EIO && vaxc$errno == SS$_EXQUOTA)
				close_one();
#endif
			else {
				/*
				 * Some other reason for failure.
				 *
				 * On redirection of input from a file,
				 * just return an error, so e.g. getline
				 * can return -1.  For output to file,
				 * complain. The shell will complain on
				 * a bad command to a pipe.
				 */
				if (errflg != NULL)
					*errflg = errno;
				if (tree->type == Node_redirect_output
				    || tree->type == Node_redirect_append) {
					/* multiple messages make life easier for translators */
					if (*direction == 'f')
						fatal(_("can't redirect from `%s' (%s)"),
					    		str, strerror(errno));
					else
						fatal(_("can't redirect to `%s' (%s)"),
							str, strerror(errno));
				} else {
					free_temp(tmp);
					return NULL;
				}
			}
		}
	}
	free_temp(tmp);
	return rp;
}

/* getredirect --- find the struct redirect for this file or pipe */

struct redirect *
getredirect(const char *str, int len)
{
	struct redirect *rp;

	for (rp = red_head; rp != NULL; rp = rp->next)
		if (strlen(rp->value) == len && STREQN(rp->value, str, len))
			return rp;

	return NULL;
}

/* close_one --- temporarily close an open file to re-use the fd */

static void
close_one()
{
	register struct redirect *rp;
	register struct redirect *rplast = NULL;

	static short warned = FALSE;

	if (do_lint && ! warned) {
		warned = TRUE;
		lintwarn(_("reached system limit for open files: starting to multiplex file descriptors"));
	}

	/* go to end of list first, to pick up least recently used entry */
	for (rp = red_head; rp != NULL; rp = rp->next)
		rplast = rp;
	/* now work back up through the list */
	for (rp = rplast; rp != NULL; rp = rp->prev)
		if (rp->fp != NULL && (rp->flag & RED_FILE) != 0) {
			rp->flag |= RED_USED;
			errno = 0;
			if (/* do_lint && */ fclose(rp->fp) != 0)
				warning(_("close of `%s' failed (%s)."),
					rp->value, strerror(errno));
			rp->fp = NULL;
			break;
		}
	if (rp == NULL)
		/* surely this is the only reason ??? */
		fatal(_("too many pipes or input files open")); 
}

/* do_close --- completely close an open file or pipe */

NODE *
do_close(NODE *tree)
{
	NODE *tmp, *tmp2;
	register struct redirect *rp;
	two_way_close_type how = CLOSE_ALL;	/* default */

	tmp = force_string(tree_eval(tree->lnode));	/* 1st arg: redir to close */

	if (tree->rnode != NULL) {
		/* 2nd arg if present: "to" or "from" for two-way pipe */
		/* DO NOT use _() on the strings here! */
		tmp2 = force_string(tree->rnode->lnode);
		if (strcasecmp(tmp2->stptr, "to") == 0)
			how = CLOSE_TO;
		else if (strcasecmp(tmp2->stptr, "from") == 0)
			how = CLOSE_FROM;
		else
			fatal(_("close: second argument must be `to' or `from'"));
		free_temp(tmp2);
	}

	for (rp = red_head; rp != NULL; rp = rp->next) {
		if (strlen(rp->value) == tmp->stlen
		    && STREQN(rp->value, tmp->stptr, tmp->stlen))
			break;
	}

	if (rp == NULL) {	/* no match, return -1 */
		char *cp;

		if (do_lint)
			lintwarn(_("close: `%.*s' is not an open file, pipe or co-process"),
				tmp->stlen, tmp->stptr);

		/* update ERRNO manually, using errno = ENOENT is a stretch. */
		cp = _("close of redirection that was never opened");
		unref(ERRNO_node->var_value);
		ERRNO_node->var_value = make_string(cp, strlen(cp));

		free_temp(tmp);
		return tmp_number((AWKNUM) -1.0);
	}
	free_temp(tmp);
	fflush(stdout);	/* synchronize regular output */
	tmp = tmp_number((AWKNUM) close_redir(rp, FALSE, how));
	rp = NULL;
	return tmp;
}

/* close_redir --- close an open file or pipe */

static int
close_redir(register struct redirect *rp, int exitwarn, two_way_close_type how)
{
	int status = 0;

	if (rp == NULL)
		return 0;
	if (rp->fp == stdout || rp->fp == stderr)
		return 0;

	if (do_lint && (rp->flag & RED_TWOWAY) == 0 && how != CLOSE_ALL)
		lintwarn(_("close: redirection `%s' not opened with `|&', second argument ignored"),
				rp->value);

	errno = 0;
	if ((rp->flag & RED_TWOWAY) != 0) {	/* two-way pipe */
		/* write end: */
		if ((how == CLOSE_ALL || how == CLOSE_TO) && rp->fp != NULL) {
#ifdef HAVE_SOCKETS
			if ((rp->flag & RED_TCP) != 0)
				(void) shutdown(fileno(rp->fp), SHUT_WR);
#endif /* HAVE_SOCKETS */

			if ((rp->flag & RED_PTY) != 0) {
				fwrite("\004\n", sizeof("\004\n") - 1, 1, rp->fp);
				fflush(rp->fp);
			}
			status = fclose(rp->fp);
			rp->fp = NULL;
		}

		/* read end: */
		if (how == CLOSE_ALL || how == CLOSE_FROM) {
			if ((rp->flag & RED_SOCKET) != 0 && rp->iop != NULL) {
#ifdef HAVE_SOCKETS
				if ((rp->flag & RED_TCP) != 0)
					(void) shutdown(rp->iop->fd, SHUT_RD);
#endif /* HAVE_SOCKETS */
				(void) iop_close(rp->iop);
			} else
				status = gawk_pclose(rp);

			rp->iop = NULL;
		}
	} else if ((rp->flag & (RED_PIPE|RED_WRITE)) == (RED_PIPE|RED_WRITE)) { /* write to pipe */
		status = pclose(rp->fp);
		if ((BINMODE & 1) != 0)
			os_setbinmode(fileno(stdin), O_BINARY);

		rp->fp = NULL;
	} else if (rp->fp != NULL) {	/* write to file */
		status = fclose(rp->fp);
		rp->fp = NULL;
	} else if (rp->iop != NULL) {	/* read from pipe/file */
		if ((rp->flag & RED_PIPE) != 0)		/* read from pipe */
			status = gawk_pclose(rp);
			/* gawk_pclose sets rp->iop to null */
		else {					/* read from file */
			status = iop_close(rp->iop);
			rp->iop = NULL;
		}
	}

	/* SVR4 awk checks and warns about status of close */
	if (status != 0) {
		char *s = strerror(errno);

		/*
		 * Too many people have complained about this.
		 * As of 2.15.6, it is now under lint control.
		 */
		if (do_lint) {
			if ((rp->flag & RED_PIPE) != 0)
				lintwarn(_("failure status (%d) on pipe close of `%s' (%s)"),
					 status, rp->value, s);
			else
				lintwarn(_("failure status (%d) on file close of `%s' (%s)"),
					 status, rp->value, s);
		}

		if (! do_traditional) {
			/* set ERRNO too so that program can get at it */
			update_ERRNO();
		}
	}

	if (exitwarn) {
		/*
		 * Don't use lintwarn() here.  If lint warnings are fatal,
		 * doing so prevents us from closing other open redirections.
		 *
		 * Using multiple full messages instead of string parameters
		 * for the types makes message translation easier.
		 */
		if ((rp->flag & RED_SOCKET) != 0)
			warning(_("no explicit close of socket `%s' provided"),
				rp->value);
		else if ((rp->flag & RED_TWOWAY) != 0)
			warning(_("no explicit close of co-process `%s' provided"),
				rp->value);
		else if ((rp->flag & RED_PIPE) != 0)
			warning(_("no explicit close of pipe `%s' provided"),
				rp->value);
		else
			warning(_("no explicit close of file `%s' provided"),
				rp->value);
	}

	/* remove it from the list if closing both or both ends have been closed */
	if (how == CLOSE_ALL || (rp->iop == NULL && rp->fp == NULL)) {
		if (rp->next != NULL)
			rp->next->prev = rp->prev;
		if (rp->prev != NULL)
			rp->prev->next = rp->next;
		else
			red_head = rp->next;
		free(rp->value);
		free((char *) rp);
	}

	return status;
}

/* flush_io --- flush all open output files */

int
flush_io()
{
	register struct redirect *rp;
	int status = 0;

	errno = 0;
	if (fflush(stdout)) {
		warning(_("error writing standard output (%s)"), strerror(errno));
		status++;
	}
	if (fflush(stderr)) {
		warning(_("error writing standard error (%s)"), strerror(errno));
		status++;
	}
	for (rp = red_head; rp != NULL; rp = rp->next)
		/* flush both files and pipes, what the heck */
		if ((rp->flag & RED_WRITE) && rp->fp != NULL) {
			if (fflush(rp->fp)) {
				if (rp->flag  & RED_PIPE)
					warning(_("pipe flush of `%s' failed (%s)."),
						rp->value, strerror(errno));
				else if (rp->flag & RED_TWOWAY)
					warning(_("co-process flush of pipe to `%s' failed (%s)."),
						rp->value, strerror(errno));
				else
					warning(_("file flush of `%s' failed (%s)."),
						rp->value, strerror(errno));
				status++;
			}
		}
	if (status != 0)
		status = -1;	/* canonicalize it */
	return status;
}

/* close_io --- close all open files, called when exiting */

int
close_io()
{
	register struct redirect *rp;
	register struct redirect *next;
	int status = 0;

	errno = 0;
	for (rp = red_head; rp != NULL; rp = next) {
		next = rp->next;
		/*
		 * close_redir() will print a message if needed
		 * if do_lint, warn about lack of explicit close
		 */
		if (close_redir(rp, do_lint, CLOSE_ALL))
			status++;
		rp = NULL;
	}
	/*
	 * Some of the non-Unix os's have problems doing an fclose
	 * on stdout and stderr.  Since we don't really need to close
	 * them, we just flush them, and do that across the board.
	 */
	if (fflush(stdout)) {
		warning(_("error writing standard output (%s)"), strerror(errno));
		status++;
	}
	if (fflush(stderr)) {
		warning(_("error writing standard error (%s)"), strerror(errno));
		status++;
	}
	return status;
}

/* str2mode --- convert a string mode to an integer mode */

static int
str2mode(const char *mode)
{
	int ret;
	const char *second = & mode[1];

	if (*second == 'b')
		second++;

	switch(mode[0]) {
	case 'r':
		ret = O_RDONLY;
		if (*second == '+' || *second == 'w')
			ret = O_RDWR;
		break;

	case 'w':
		ret = O_WRONLY|O_CREAT|O_TRUNC;
		if (*second == '+' || *second == 'r')
			ret = O_RDWR|O_CREAT|O_TRUNC;
		break;

	case 'a':
		ret = O_WRONLY|O_APPEND|O_CREAT;
		if (*second == '+')
			ret = O_RDWR|O_APPEND|O_CREAT;
		break;

	default:
		ret = 0;		/* lint */
		cant_happen();
	}
	if (strchr(mode, 'b') != NULL)
		ret |= O_BINARY;
	return ret;
}

#ifdef HAVE_SOCKETS
/* socketopen --- open a socket and set it into connected state */

static int
socketopen(enum inet_prot type, int localport, int remoteport, const char *remotehostname)
{
	struct hostent *hp = gethostbyname(remotehostname);
	struct sockaddr_in local_addr, remote_addr;
	int socket_fd;
	int any_remote_host = strcmp(remotehostname, "0");

	socket_fd = INVALID_HANDLE;
	switch (type) {
	case INET_TCP:  
		if (localport != 0 || remoteport != 0) {
			int on = 1;
#ifdef SO_LINGER
			struct linger linger;

			memset(& linger, '\0', sizeof(linger));
#endif
			socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
			setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
				(char *) & on, sizeof(on));
#ifdef SO_LINGER
			linger.l_onoff = 1;
			linger.l_linger = 30;    /* linger for 30/100 second */
			setsockopt(socket_fd, SOL_SOCKET, SO_LINGER,
				(char *) & linger, sizeof(linger));
#endif
		}
		break;
	case INET_UDP:  
		if (localport != 0 || remoteport != 0)
			socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
		break;
	case INET_RAW:  
#ifdef SOCK_RAW
		if (localport == 0 && remoteport == 0)
			socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); 
#endif
		break;
	case INET_NONE:
		/* fall through */
	default:
		cant_happen();
		break;
	}

	if (socket_fd < 0 || socket_fd == INVALID_HANDLE
	    || (hp == NULL && any_remote_host != 0))
		return INVALID_HANDLE;

	local_addr.sin_family = remote_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	remote_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port  = htons(localport);
	remote_addr.sin_port = htons(remoteport);
	if (bind(socket_fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) == 0) {
		if (any_remote_host != 0) { /* not ANY => create a client */
			if (type == INET_TCP || type == INET_UDP) {
				memcpy(&remote_addr.sin_addr, hp->h_addr,
					sizeof(remote_addr.sin_addr));
				if (connect(socket_fd,
						(struct sockaddr *) &remote_addr,
						sizeof(remote_addr)) != 0) {
					close(socket_fd);
					if (localport == 0)
						socket_fd = INVALID_HANDLE;
					else
						socket_fd = socketopen(type, localport, 0, "0");
				}
			} else {
				/* /inet/raw client not ready yet */ 
				fatal(_("/inet/raw client not ready yet, sorry"));
				if (geteuid() != 0)
					fatal(_("only root may use `/inet/raw'."));
			}
		} else { /* remote host is ANY => create a server */
			if (type == INET_TCP) {
				int clientsocket_fd = INVALID_HANDLE;
				int namelen = sizeof(remote_addr);

				if (listen(socket_fd, 1) >= 0
				    && (clientsocket_fd = accept(socket_fd,
						(struct sockaddr *) &remote_addr,
						&namelen)) >= 0) {
					close(socket_fd);
					socket_fd = clientsocket_fd;
				} else {
					close(socket_fd);
					socket_fd = INVALID_HANDLE;
				}
			} else if (type == INET_UDP) {
				char buf[10];
				int readle;

#ifdef MSG_PEEK
				if (recvfrom(socket_fd, buf, 1, MSG_PEEK,
					(struct sockaddr *) & remote_addr,
					& readle) < 1
						|| readle != sizeof(remote_addr)
				    || connect(socket_fd,
					(struct sockaddr *)& remote_addr,
						readle) != 0) {
					close(socket_fd);
					socket_fd = INVALID_HANDLE;
				}
#endif
			} else {
				/* /inet/raw server not ready yet */ 
				fatal(_("/inet/raw server not ready yet, sorry"));
				if (geteuid() != 0)
					fatal(_("only root may use `/inet/raw'."));
			}
		}
	} else {
		close(socket_fd);
		socket_fd = INVALID_HANDLE;
	}

	return socket_fd;
}
#endif /* HAVE_SOCKETS */

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, regular files */

/*
 * This separate version is still needed for output, since file and pipe
 * output is done with stdio. iop_open() handles input with IOBUFs of
 * more "special" files.  Those files are not handled here since it makes
 * no sense to use them for output.
 */

/*
 * Strictly speaking, "name" is not a "const char *" because we temporarily
 * change the string.
 */

int
devopen(const char *name, const char *mode)
{
	int openfd;
	char *cp;
	char *ptr;
	int flag = 0;
	extern double strtod();

	flag = str2mode(mode);

	if (STREQ(name, "-"))
		return fileno(stdin);

	openfd = INVALID_HANDLE;

	if (do_traditional)
		goto strictopen;

	if ((openfd = os_devopen(name, flag)) != INVALID_HANDLE) {
		os_close_on_exec(openfd, name, "file", "");
		return openfd;
	}

	if (STREQN(name, "/dev/", 5)) {
		cp = (char *) name + 5;

		if (STREQ(cp, "stdin") && (flag & O_ACCMODE) == O_RDONLY)
			openfd = fileno(stdin);
		else if (STREQ(cp, "stdout") && (flag & O_ACCMODE) == O_WRONLY)
			openfd = fileno(stdout);
		else if (STREQ(cp, "stderr") && (flag & O_ACCMODE) == O_WRONLY)
			openfd = fileno(stderr);
		else if (STREQN(cp, "fd/", 3)) {
			cp += 3;
			openfd = (int) strtod(cp, &ptr);
			if (openfd <= INVALID_HANDLE || ptr == cp)
				openfd = INVALID_HANDLE;
		}
		/* do not set close-on-exec for inherited fd's */
		if (openfd != INVALID_HANDLE)
			return openfd;
	} else if (STREQN(name, "/inet/", 6)) {
#ifdef HAVE_SOCKETS
		/* /inet/protocol/localport/hostname/remoteport */
		enum inet_prot protocol = INET_NONE;
		int localport, remoteport;
		char *hostname;
		char *hostnameslastcharp;
		char *localpname;
		char proto[4];
		struct servent *service;

		cp = (char *) name + 6;
		/* which protocol? */
		if (STREQN(cp, "tcp/", 4))
			protocol = INET_TCP;
		else if (STREQN(cp, "udp/", 4))
			protocol = INET_UDP;
		else if (STREQN(cp, "raw/", 4))
			protocol = INET_RAW;
		else
			fatal(_("no (known) protocol supplied in special filename `%s'"),
				name);

		proto[0] = cp[0];
		proto[1] = cp[1];   
		proto[2] = cp[2];   
		proto[3] =  '\0';
		cp += 4;

		/* which localport? */
		localpname = cp;
		while (*cp != '/' && *cp != '\0')
			cp++;
		/*                    
		 * Require a port, let them explicitly put 0 if
		 * they don't care.  
		 */
		if (*cp != '/' || cp == localpname)
			fatal(_("special file name `%s' is incomplete"), name);
		/* We change the special file name temporarily because we
		 * need a 0-terminated string here for conversion with atoi().
		 * By using atoi() the use of decimal numbers is enforced.
		 */
		*cp = '\0';

		localport = atoi(localpname);
		if (strcmp(localpname, "0") != 0
		    && (localport <= 0 || localport > 65535)) {
			service = getservbyname(localpname, proto);
			if (service == NULL)
				fatal(_("local port invalid in `%s'"), name);
			else
				localport = ntohs(service->s_port);
		}
		*cp = '/';

		/* which hostname? */
		cp++;
		hostname = cp;
		while (*cp != '/' && *cp != '\0')
			cp++; 
		if (*cp != '/' || cp == hostname)
			fatal(_("must supply a remote hostname to `/inet'"));
		*cp = '\0';
		hostnameslastcharp = cp;

		/* which remoteport? */
		cp++;
		/*
		 * The remote port ends the special file name.
		 * This means there already is a 0 at the end of the string.
		 * Therefore no need to patch any string ending.
		 *
		 * Here too, require a port, let them explicitly put 0 if
		 * they don't care.
		 */
		if (*cp == '\0')
			fatal(_("must supply a remote port to `/inet'"));
		remoteport = atoi(cp);
		if (strcmp(cp, "0") != 0
		    && (remoteport <= 0 || remoteport > 65535)) {
			service = getservbyname(cp, proto);
			if (service == NULL)
				 fatal(_("remote port invalid in `%s'"), name);
			else
				remoteport = ntohs(service->s_port);
		}

		/* Open Sesame! */
		openfd = socketopen(protocol, localport, remoteport, hostname);
		*hostnameslastcharp = '/';

#else /* ! HAVE_SOCKETS */
		fatal(_("TCP/IP communications are not supported"));
#endif /* HAVE_SOCKETS */
	}

strictopen:
	if (openfd == INVALID_HANDLE)
		openfd = open(name, flag, 0666);
	if (openfd != INVALID_HANDLE) {
		if (os_isdir(openfd))
			fatal(_("file `%s' is a directory"), name);

		os_close_on_exec(openfd, name, "file", "");
	}
	return openfd;
}


/* spec_setup --- setup an IOBUF for a special internal file */

static void
spec_setup(IOBUF *iop, int len, int allocate)
{
	char *cp;

	if (allocate) {
		emalloc(cp, char *, len+2, "spec_setup");
		iop->buf = cp;
	} else {
		len = strlen(iop->buf);
		iop->buf[len++] = '\n';	/* get_a_record clobbered it */
		iop->buf[len] = '\0';	/* just in case */
	}
	iop->off = iop->buf;
	iop->count = 0;
	iop->size = len;
	iop->end = iop->buf + len;
	iop->fd = -1;
	iop->flag = IOP_IS_INTERNAL;
}

/* specfdopen --- open an fd special file */

static int
specfdopen(IOBUF *iop, const char *name, const char *mode)
{
	int fd;
	IOBUF *tp;

	fd = devopen(name, mode);
	if (fd == INVALID_HANDLE)
		return INVALID_HANDLE;
	tp = iop_alloc(fd, name, NULL);
	if (tp == NULL) {
		/* don't leak fd's */
		close(fd);
		return INVALID_HANDLE;
	}
	*iop = *tp;
	iop->flag |= IOP_NO_FREE;
	free(tp);
	return 0;
}

#ifdef GETPGRP_VOID
#define getpgrp_arg() /* nothing */
#else
#define getpgrp_arg() getpid()
#endif

/* pidopen --- "open" /dev/pid, /dev/ppid, and /dev/pgrpid */

static int
pidopen(IOBUF *iop, const char *name, const char *mode ATTRIBUTE_UNUSED)
{
	char tbuf[BUFSIZ];
	int i;
	const char *cp = name + 5;

	warning(_("use `PROCINFO[\"%s\"]' instead of `%s'"), cp, name);

	if (name[6] == 'g')
		sprintf(tbuf, "%d\n", (int) getpgrp(getpgrp_arg()));
	else if (name[6] == 'i')
		sprintf(tbuf, "%d\n", (int) getpid());
	else
		sprintf(tbuf, "%d\n", (int) getppid());
	i = strlen(tbuf);
	spec_setup(iop, i, TRUE);
	strcpy(iop->buf, tbuf);
	return 0;
}

/* useropen --- "open" /dev/user */

/*
 * /dev/user creates a record as follows:
 *	$1 = getuid()
 *	$2 = geteuid()
 *	$3 = getgid()
 *	$4 = getegid()
 * If multiple groups are supported, then $5 through $NF are the
 * supplementary group set.
 */

static int
useropen(IOBUF *iop, const char *name ATTRIBUTE_UNUSED, const char *mode ATTRIBUTE_UNUSED)
{
	char tbuf[BUFSIZ], *cp;
	int i;

	warning(_("use `PROCINFO[...]' instead of `/dev/user'"));

	sprintf(tbuf, "%d %d %d %d", (int) getuid(), (int) geteuid(), (int) getgid(), (int) getegid());

	cp = tbuf + strlen(tbuf);
#if defined(NGROUPS_MAX) && NGROUPS_MAX > 0
	for (i = 0; i < ngroups; i++) {
		*cp++ = ' ';
		sprintf(cp, "%d", (int) groupset[i]);
		cp += strlen(cp);
	}
#endif
	*cp++ = '\n';
	*cp++ = '\0';

	i = strlen(tbuf);
	spec_setup(iop, i, TRUE);
	strcpy(iop->buf, tbuf);
	return 0;
}

/* iop_open --- handle special and regular files for input */

static IOBUF *
iop_open(const char *name, const char *mode, IOBUF *iop)
{
	int openfd = INVALID_HANDLE;
	int flag = 0;
	static struct internal {
		const char *name;
		int compare;
		int (*fp) P((IOBUF *, const char *, const char *));
		IOBUF iob;
	} table[] = {
		{ "/dev/fd/",		8,	specfdopen },
		{ "/dev/stdin",		10,	specfdopen },
		{ "/dev/stdout",	11,	specfdopen },
		{ "/dev/stderr",	11,	specfdopen },
		{ "/inet/",		6,	specfdopen },
		{ "/dev/pid",		8,	pidopen },
		{ "/dev/ppid",		9,	pidopen },
		{ "/dev/pgrpid",	11,	pidopen },
		{ "/dev/user",		9,	useropen },
	};
	int devcount = sizeof(table) / sizeof(table[0]);

	flag = str2mode(mode);

	if (STREQ(name, "-"))
		openfd = fileno(stdin);
	else if (do_traditional)
		goto strictopen;
	else if (STREQN(name, "/dev/", 5) || STREQN(name, "/inet/", 6)) {
		int i;

		for (i = 0; i < devcount; i++) {
			if (STREQN(name, table[i].name, table[i].compare)) {
				iop = & table[i].iob;

				if (iop->buf != NULL) {
					spec_setup(iop, 0, FALSE);
					return iop;
				} else if ((*table[i].fp)(iop, name, mode) == 0)
					return iop;
				else {
					warning(_("could not open `%s', mode `%s'"),
						name, mode);
					return NULL;
				}
			}
		}
		/* not in table, fall through to regular code */
	}

strictopen:
	if (openfd == INVALID_HANDLE)
		openfd = open(name, flag, 0666);
	if (openfd != INVALID_HANDLE) {
		if (os_isdir(openfd))
			fatal(_("file `%s' is a directory"), name);

		if (openfd > fileno(stderr))
			os_close_on_exec(openfd, name, "file", "");
	}
	return iop_alloc(openfd, name, iop);
}

/* two_way_open --- open a two way communications channel */

static int
two_way_open(const char *str, struct redirect *rp)
{
	static int no_ptys = FALSE;

#ifdef HAVE_SOCKETS
	/* case 1: socket */
	if (STREQN(str, "/inet/", 6)) {
		int fd, newfd;

		fd = devopen(str, "rw");
		if (fd == INVALID_HANDLE)
			return FALSE;
		rp->fp = fdopen(fd, "w");
		if (rp->fp == NULL) {
			close(fd);
			return FALSE;
		}
		newfd = dup(fd);
		if (newfd < 0) {
			fclose(rp->fp);
			return FALSE;
		}
		os_close_on_exec(newfd, str, "socket", "to/from");
		rp->iop = iop_alloc(newfd, str, NULL);
		if (rp->iop == NULL) {
			fclose(rp->fp);
			return FALSE;
		}
		rp->flag |= RED_SOCKET;
		return TRUE;
	}
#endif /* HAVE_SOCKETS */

#ifdef HAVE_PORTALS
	/* case 1.5: portal */
	if (STREQN(str, "/p/", 3)) {
		int fd, newfd;

		fd = open(str, O_RDWR);
		if (fd == INVALID_HANDLE)
			return FALSE;
		rp->fp = fdopen(fd, "w");
		if (rp->fp == NULL) {
			close(fd);
			return FALSE;
		}
		newfd = dup(fd);
		if (newfd < 0) {
			fclose(rp->fp);
			return FALSE;
		}
		os_close_on_exec(newfd, str, "portal", "to/from");
		rp->iop = iop_alloc(newfd, str, NULL);
		if (rp->iop == NULL) {
			fclose(rp->fp);
			return FALSE;
		}
		rp->flag |= RED_SOCKET;
		return TRUE;
	}
#endif /* HAVE_PORTALS */

#ifdef HAVE_TERMIOS_H
	/* case 2: use ptys for two-way communications to child */
	if (! no_ptys && pty_vs_pipe(str)) {
		static int initialized = FALSE;
		static char first_pty_letter;
#ifdef HAVE_GRANTPT
		static int have_dev_ptmx;
#endif
		char slavenam[32];
		char c;
		int master, dup_master;
		int slave;
		int save_errno; 
		pid_t pid;
		struct stat statb;
		struct termios st;

		if (! initialized) {
			initialized = TRUE;
#ifdef HAVE_GRANTPT
			have_dev_ptmx = (stat("/dev/ptmx", &statb) >= 0);
#endif
			c = 'p';
			do {
				sprintf(slavenam, "/dev/pty%c0", c);
				if (stat(slavenam, &statb) >= 0) {
					first_pty_letter = c;
					break;
				}
				if (++c > 'z')
					c = 'a';
			} while (c != 'p');
		}

#ifdef HAVE_GRANTPT
		if (have_dev_ptmx) {
			master = open("/dev/ptmx", O_RDWR);
			if (master >= 0) {
				char *tem;

				grantpt(master);
				unlockpt(master);
				tem = ptsname(master);
				if (tem != NULL) {
					strcpy(slavenam, tem);
					goto got_the_pty;
				}
				(void) close(master);
			}
		}
#endif

		if (first_pty_letter) {
			/*
			 * Assume /dev/ptyXNN and /dev/ttyXN naming system.
			 * The FIRST_PTY_LETTER gives the first X to try. We try in the 
			 * sequence FIRST_PTY_LETTER, .., 'z', 'a', .., FIRST_PTY_LETTER.
			 * Is this worthwhile, or just over-zealous?
			 */
			c = first_pty_letter;
			do {
				int i;
				for (i = 0; i < 16; i++) {
					sprintf(slavenam, "/dev/pty%c%x", c, i);
					if (stat(slavenam, &statb) < 0) {
						no_ptys = TRUE;	/* bypass all this next time */
						goto use_pipes;
					}

					if ((master = open(slavenam, O_RDWR)) >= 0) {
						slavenam[sizeof("/dev/") - 1] = 't';
						if (access(slavenam, R_OK | W_OK) == 0)
							goto got_the_pty;
						close(master);
					}
				}
				if (++c > 'z')
				c = 'a';
			} while (c != first_pty_letter);
		} else
			no_ptys = TRUE;

		/* Couldn't find a pty. Fall back to using pipes. */
		goto use_pipes;

	got_the_pty:
		if ((slave = open(slavenam, O_RDWR)) < 0) {
			fatal(_("could not open `%s', mode `%s'"),
				slavenam, "r+");
		}

#ifdef I_PUSH
		/*
		 * Push the necessary modules onto the slave to
		 * get terminal semantics.
		 */
		ioctl(slave, I_PUSH, "ptem");
		ioctl(slave, I_PUSH, "ldterm");
#endif

#ifdef TIOCSCTTY
		ioctl(slave, TIOCSCTTY, 0);
#endif
		tcgetattr(slave, &st);
		st.c_iflag &= ~(ISTRIP | IGNCR | INLCR | IXOFF);
		st.c_iflag |= (ICRNL | IGNPAR | BRKINT | IXON);
		st.c_oflag &= ~OPOST;
		st.c_cflag &= ~CSIZE;
		st.c_cflag |= CREAD | CS8 | CLOCAL;
		st.c_lflag &= ~(ECHO | ECHOE | ECHOK | NOFLSH | TOSTOP);
		st.c_lflag |= ISIG;
#if 0
		st.c_cc[VMIN] = 1;
		st.c_cc[VTIME] = 0;
#endif

		/* Set some control codes to default values */
#ifdef VINTR
		st.c_cc[VINTR] = '\003';        /* ^c */
#endif
#ifdef VQUIT
		st.c_cc[VQUIT] = '\034';        /* ^| */
#endif
#ifdef VERASE
		st.c_cc[VERASE] = '\177';       /* ^? */
#endif
#ifdef VKILL
		st.c_cc[VKILL] = '\025';        /* ^u */
#endif
#ifdef VEOF
		st.c_cc[VEOF] = '\004'; /* ^d */
#endif
		tcsetattr(slave, TCSANOW, &st);

		switch (pid = fork ()) {
		case 0:
			/* Child process */
			if (close(master) == -1)
				fatal(_("close of master pty failed (%s)"), strerror(errno));
			if (close(1) == -1)
				fatal(_("close of stdout in child failed (%s)"),
					strerror(errno));
			if (dup(slave) != 1)
				fatal(_("moving slave pty to stdout in child failed (dup: %s)"), strerror(errno));
			if (close(0) == -1)
				fatal(_("close of stdin in child failed (%s)"),
					strerror(errno));
			if (dup(slave) != 0)
				fatal(_("moving slave pty to stdin in child failed (dup: %s)"), strerror(errno));
			if (close(slave))
				fatal(_("close of slave pty failed (%s)"), strerror(errno));

			/* stderr does NOT get dup'ed onto child's stdout */

			signal(SIGPIPE, SIG_DFL);

			execl("/bin/sh", "sh", "-c", str, NULL);
			_exit(127);

		case -1:
			save_errno = errno;
			close(master);
			errno = save_errno;
			return FALSE;

		}

		/* parent */
		if (close(slave))
			fatal(_("close of slave pty failed (%s)"), strerror(errno));

		rp->pid = pid;
		rp->iop = iop_alloc(master, str, NULL);
		if (rp->iop == NULL) {
			(void) close(master);
			(void) kill(pid, SIGKILL);      /* overkill? (pardon pun) */
			return FALSE;
		}

		/*
		 * Force read and write ends of two-way connection to
		 * be different fd's so they can be closed independently.
		 */
		if ((dup_master = dup(master)) < 0
		    || (rp->fp = fdopen(dup_master, "w")) == NULL) {
			iop_close(rp->iop);
			rp->iop = NULL;
			(void) close(master);
			(void) kill(pid, SIGKILL);      /* overkill? (pardon pun) */
			if (dup_master > 0)
				(void) close(dup_master);
			return FALSE;
		}
		rp->flag |= RED_PTY;
		os_close_on_exec(master, str, "pipe", "from");
		os_close_on_exec(dup_master, str, "pipe", "to");
		first_pty_letter = '\0';	/* reset for next command */
		return TRUE;
	}
#endif /* HAVE_TERMIOS_H */

use_pipes:
#ifndef PIPES_SIMULATED		/* real pipes */
	/* case 3: two way pipe to a child process */
    {
	int ptoc[2], ctop[2];
	int pid;
	int save_errno;
#ifdef __EMX__
	int save_stdout, save_stdin;
#endif

	if (pipe(ptoc) < 0)
		return FALSE;	/* errno set, diagnostic from caller */

	if (pipe(ctop) < 0) {
		save_errno = errno;
		close(ptoc[0]);
		close(ptoc[1]);
		errno = save_errno;
		return FALSE;
	}

#ifdef __EMX__
	save_stdin = dup(0);	/* duplicate stdin */
	save_stdout = dup(1);	/* duplicate stdout */
	
	if (save_stdout == -1 || save_stdin == -1) {
		/* if an error occurrs close all open file handles */
		save_errno = errno;
		if (save_stdin != -1)
			close(save_stdin);
		if (save_stdout != -1)
			close(save_stdout);
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		errno = save_errno;
		return FALSE;
	}
	
	/* connect pipes to stdin and stdout */
	close(1);	/* close stdout */
	if (dup(ctop[1]) != 1)	/* connect pipe input to stdout */
		fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));

	close(0);	/* close stdin */
	if (dup(ptoc[0]) != 0)	/* connect pipe output to stdin */
		fatal(_("moving pipe to stdin in child failed (dup: %s)"), strerror(errno));
	
	/* none of these handles must be inherited by the child process */
	(void) close(ptoc[0]);	/* close pipe output, child will use stdin instead */
	(void) close(ctop[1]);	/* close pipe input, child will use stdout instead */
	
	os_close_on_exec(ptoc[1], str, "pipe", "from"); /* pipe input: output of the parent process */
	os_close_on_exec(ctop[0], str, "pipe", "from"); /* pipe output: input of the parent process */
	os_close_on_exec(save_stdin, str, "pipe", "from"); /* saved stdin of the parent process */
	os_close_on_exec(save_stdout, str, "pipe", "from"); /* saved stdout of the parent process */

	/* stderr does NOT get dup'ed onto child's stdout */
	pid = spawnl(P_NOWAIT, "/bin/sh", "sh", "-c", str, NULL);
	
	/* restore stdin and stdout */
	close(1);
	if (dup(save_stdout) != 1)
		fatal(_("restoring stdout in parent process failed\n"));
	close(save_stdout);
	
	close(0);
	if (dup(save_stdin) != 0)
		fatal(_("restoring stdin in parent process failed\n"));
	close(save_stdin);

	if (pid < 0) { /* spawnl() failed */
		save_errno = errno;
		close(ptoc[1]);
		close(ctop[0]);

		errno = save_errno;
		return FALSE;
	}

#else /* NOT __EMX__ */
	if ((pid = fork()) < 0) {
		save_errno = errno;
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		errno = save_errno;
		return FALSE;
	}
	
	if (pid == 0) {	/* child */
		if (close(1) == -1)
			fatal(_("close of stdout in child failed (%s)"),
				strerror(errno));
		if (dup(ctop[1]) != 1)
			fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(0) == -1)
			fatal(_("close of stdin in child failed (%s)"),
				strerror(errno));
		if (dup(ptoc[0]) != 0)
			fatal(_("moving pipe to stdin in child failed (dup: %s)"), strerror(errno));
		if (   close(ptoc[0]) == -1 || close(ptoc[1]) == -1
		    || close(ctop[0]) == -1 || close(ctop[1]) == -1)
			fatal(_("close of pipe failed (%s)"), strerror(errno));
		/* stderr does NOT get dup'ed onto child's stdout */
		execl("/bin/sh", "sh", "-c", str, NULL);
		_exit(errno == ENOENT ? 127 : 126);
	}
#endif /* NOT __EMX__ */

	/* parent */
	rp->pid = pid;
	rp->iop = iop_alloc(ctop[0], str, NULL);
	if (rp->iop == NULL) {
		(void) close(ctop[0]);
		(void) close(ctop[1]);
		(void) close(ptoc[0]);
		(void) close(ptoc[1]);
		(void) kill(pid, SIGKILL);	/* overkill? (pardon pun) */

		return FALSE;
	}
	rp->fp = fdopen(ptoc[1], "w");
	if (rp->fp == NULL) {
		iop_close(rp->iop);
		rp->iop = NULL;
		(void) close(ctop[0]);
		(void) close(ctop[1]);
		(void) close(ptoc[0]);
		(void) close(ptoc[1]);
		(void) kill(pid, SIGKILL);	/* overkill? (pardon pun) */

		return FALSE;
	}

#ifndef __EMX__
	os_close_on_exec(ctop[0], str, "pipe", "from");
	os_close_on_exec(ptoc[1], str, "pipe", "from");

	(void) close(ptoc[0]);
	(void) close(ctop[1]);
#endif

	return TRUE;
    }

#else	/*PIPES_SIMULATED*/

	fatal(_("`|&' not supported"));
	/*NOTREACHED*/
	return FALSE;

#endif
}

#ifndef PIPES_SIMULATED		/* real pipes */

/* wait_any --- wait for a child process, close associated pipe */

static int
wait_any(int interesting)	/* pid of interest, if any */
{
	RETSIGTYPE (*hstat) P((int)), (*istat) P((int)), (*qstat) P((int));
	int pid;
	int status = 0;
	struct redirect *redp;
	extern int errno;

	hstat = signal(SIGHUP, SIG_IGN);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	for (;;) {
#ifdef HAVE_SYS_WAIT_H	/* Posix compatible sys/wait.h */
		pid = wait(&status);
#else
		pid = wait((union wait *)&status);
#endif /* NeXT */
		if (interesting && pid == interesting) {
			break;
		} else if (pid != -1) {
			for (redp = red_head; redp != NULL; redp = redp->next)
				if (pid == redp->pid) {
					redp->pid = -1;
					redp->status = status;
					break;
				}
		}
		if (pid == -1 && errno == ECHILD)
			break;
	}
	signal(SIGHUP, hstat);
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}

/* gawk_popen --- open an IOBUF on a child process */

static IOBUF *
gawk_popen(const char *cmd, struct redirect *rp)
{
	int p[2];
	register int pid;
#ifdef __EMX__
	int save_stdout;
#endif

	/*
	 * used to wait for any children to synchronize input and output,
	 * but this could cause gawk to hang when it is started in a pipeline
	 * and thus has a child process feeding it input (shell dependant)
	 */
	/*(void) wait_any(0);*/	/* wait for outstanding processes */

	if (pipe(p) < 0)
		fatal(_("cannot open pipe `%s' (%s)"), cmd, strerror(errno));

#ifdef __EMX__
	save_stdout = dup(1); /* save stdout */
	rp->iop = NULL;
	if (save_stdout == -1)
		return rp->iop; /* failed */
	
	close(1); /* close stdout */
	if (dup(p[1]) != 1)
		fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
	
	/* none of these handles must be inherited by the child process */
	close(p[1]); /* close pipe input */
	
	os_close_on_exec(p[0], cmd, "pipe", "from"); /* pipe output: input of the parent process */
	os_close_on_exec(save_stdout, cmd, "pipe", "from"); /* saved stdout of the parent process */
	
	pid = spawnl(P_NOWAIT, "/bin/sh", "sh", "-c", cmd, NULL);
	
	/* restore stdout */
	close(1);
	if (dup(save_stdout) != 1)
		fatal(_("restoring stdout in parent process failed\n"));
	close(save_stdout);

#else /* NOT __EMX__ */
	if ((pid = fork()) == 0) {
		if (close(1) == -1)
			fatal(_("close of stdout in child failed (%s)"),
				strerror(errno));
		if (dup(p[1]) != 1)
			fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(p[0]) == -1 || close(p[1]) == -1)
			fatal(_("close of pipe failed (%s)"), strerror(errno));
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		_exit(errno == ENOENT ? 127 : 126);
	}
#endif /* NOT __EMX__ */

	if (pid == -1)
		fatal(_("cannot create child process for `%s' (fork: %s)"), cmd, strerror(errno));
	rp->pid = pid;
#ifndef __EMX__
	if (close(p[1]) == -1)
		fatal(_("close of pipe failed (%s)"), strerror(errno));
#endif
	os_close_on_exec(p[0], cmd, "pipe", "from");
	rp->iop = iop_alloc(p[0], cmd, NULL);
	if (rp->iop == NULL)
		(void) close(p[0]);

	return (rp->iop);
}

/* gawk_pclose --- close an open child pipe */

static int
gawk_pclose(struct redirect *rp)
{
	if (rp->iop != NULL)
		(void) iop_close(rp->iop);
	rp->iop = NULL;

	/* process previously found, return stored status */
	if (rp->pid == -1)
		return rp->status;
	rp->status = wait_any(rp->pid);
	rp->pid = -1;
	return rp->status;
}

#else	/* PIPES_SIMULATED */

/*
 * use temporary file rather than pipe
 * except if popen() provides real pipes too
 */

#if defined(VMS) || defined(OS2) || defined (MSDOS) || defined(WIN32) || defined(TANDEM) || defined(__EMX__)

/* gawk_popen --- open an IOBUF on a child process */

static IOBUF *
gawk_popen(const char *cmd, struct redirect *rp)
{
	FILE *current;

	os_restore_mode(fileno(stdin));
	current = popen(cmd, binmode("r"));
	if ((BINMODE & 1) != 0)
		os_setbinmode(fileno(stdin), O_BINARY);
	if (current == NULL)
		return NULL;
	os_close_on_exec(fileno(current), cmd, "pipe", "from");
	rp->iop = iop_alloc(fileno(current), cmd, NULL);
	if (rp->iop == NULL) {
		(void) pclose(current);
		current = NULL;
	}
	rp->ifp = current;
	return (rp->iop);
}

/* gawk_pclose --- close an open child pipe */

static int
gawk_pclose(struct redirect *rp)
{
	int rval, aval, fd = rp->iop->fd;

	if (rp->iop != NULL) {
		rp->iop->fd = dup(fd);	  /* kludge to allow close() + pclose() */
		rval = iop_close(rp->iop);
	}
	rp->iop = NULL;
	aval = pclose(rp->ifp);
	rp->ifp = NULL;
	return (rval < 0 ? rval : aval);
}
#else	/* not (VMS || OS2 || MSDOS || TANDEM) */

static struct pipeinfo {
	char *command;
	char *name;
} pipes[_NFILE];

/* gawk_popen --- open an IOBUF on a child process */

static IOBUF *
gawk_popen(const char *cmd, struct redirect *rp)
{
	extern char *strdup P((const char *));
	int current;
	char *name;
	static char cmdbuf[256];

	/* get a name to use */
	if ((name = tempnam(".", "pip")) == NULL)
		return NULL;
	sprintf(cmdbuf, "%s > %s", cmd, name);
	system(cmdbuf);
	if ((current = open(name, O_RDONLY)) == INVALID_HANDLE)
		return NULL;
	pipes[current].name = name;
	pipes[current].command = strdup(cmd);
	os_close_on_exec(current, cmd, "pipe", "from");
	rp->iop = iop_alloc(current, name, NULL);
	if (rp->iop == NULL)
		(void) close(current);
	return (rp->iop);
}

/* gawk_pclose --- close an open child pipe */

static int
gawk_pclose(struct redirect *rp)
{
	int cur = rp->iop->fd;
	int rval = 0;

	if (rp->iop != NULL)
		rval = iop_close(rp->iop);
	rp->iop = NULL;

	/* check for an open file  */
	if (pipes[cur].name == NULL)
		return -1;
	unlink(pipes[cur].name);
	free(pipes[cur].name);
	pipes[cur].name = NULL;
	free(pipes[cur].command);
	return rval;
}
#endif	/* not (VMS || OS2 || MSDOS || TANDEM) */

#endif	/* PIPES_SIMULATED */

/* do_getline --- read in a line, into var and with redirection, as needed */

NODE *
do_getline(NODE *tree)
{
	struct redirect *rp = NULL;
	IOBUF *iop;
	int cnt = EOF;
	char *s = NULL;
	int errcode;

	while (cnt == EOF) {
		if (tree->rnode == NULL) {	 /* no redirection */
			iop = nextfile(FALSE);
			if (iop == NULL)		/* end of input */
				return tmp_number((AWKNUM) 0.0);
		} else {
			int redir_error = 0;

			rp = redirect(tree->rnode, &redir_error);
			if (rp == NULL && redir_error) { /* failed redirect */
				if (! do_traditional)
					update_ERRNO();

				return tmp_number((AWKNUM) -1.0);
			}
			iop = rp->iop;
			if (iop == NULL)		/* end of input */
				return tmp_number((AWKNUM) 0.0);
		}
		errcode = 0;
		cnt = get_a_record(&s, iop, RS->stptr[0], RS_regexp, &errcode);
		if (errcode != 0) {
			if (! do_traditional)
				update_ERRNO();

			return tmp_number((AWKNUM) -1.0);
		}
		if (cnt == EOF) {
			if (rp != NULL) {
				/*
				 * Don't do iop_close() here if we are
				 * reading from a pipe; otherwise
				 * gawk_pclose will not be called.
				 */
				if ((rp->flag & (RED_PIPE|RED_TWOWAY)) == 0) {
					(void) iop_close(iop);
					rp->iop = NULL;
				}
				rp->flag |= RED_EOF;	/* sticky EOF */
				return tmp_number((AWKNUM) 0.0);
			} else
				continue;	/* try another file */
		}
		if (rp == NULL) {
			NR++;
			FNR++;
		}
		if (tree->lnode == NULL)	/* no optional var. */
			set_record(s, cnt);
		else {			/* assignment to variable */
			Func_ptr after_assign = NULL;
			NODE **lhs;

			lhs = get_lhs(tree->lnode, &after_assign, FALSE);
			unref(*lhs);
			*lhs = make_string(s, cnt);
			(*lhs)->flags |= MAYBE_NUM;
			/* we may have to regenerate $0 here! */
			if (after_assign != NULL)
				(*after_assign)();
		}
	}
	return tmp_number((AWKNUM) 1.0);
}

/* pathopen --- pathopen with default file extension handling */

int
pathopen(const char *file)
{
	int fd = do_pathopen(file);

#ifdef DEFAULT_FILETYPE
	if (! do_traditional && fd <= INVALID_HANDLE) {
		char *file_awk;
		int save = errno;
#ifdef VMS
		int vms_save = vaxc$errno;
#endif

		/* append ".awk" and try again */
		emalloc(file_awk, char *, strlen(file) +
			sizeof(DEFAULT_FILETYPE) + 1, "pathopen");
		sprintf(file_awk, "%s%s", file, DEFAULT_FILETYPE);
		fd = do_pathopen(file_awk);
		free(file_awk);
		if (fd <= INVALID_HANDLE) {
			errno = save;
#ifdef VMS
			vaxc$errno = vms_save;
#endif
		}
	}
#endif	/*DEFAULT_FILETYPE*/

	return fd;
}

/* do_pathopen --- search $AWKPATH for source file */

static int
do_pathopen(const char *file)
{
	static const char *savepath = NULL;
	static int first = TRUE;
	const char *awkpath;
	char *cp, *trypath;
	int fd;
	int len;

	if (STREQ(file, "-"))
		return (0);

	if (do_traditional)
		return (devopen(file, "r"));

	if (first) {
		first = FALSE;
		if ((awkpath = getenv("AWKPATH")) != NULL && *awkpath)
			savepath = awkpath;	/* used for restarting */
		else
			savepath = defpath;
	}
	awkpath = savepath;

	/* some kind of path name, no search */
	if (ispath(file))
		return (devopen(file, "r"));

	/* no arbitrary limits: */
	len = strlen(awkpath) + strlen(file) + 2;
	emalloc(trypath, char *, len, "do_pathopen");

	do {
		trypath[0] = '\0';

		for (cp = trypath; *awkpath && *awkpath != envsep; )
			*cp++ = *awkpath++;

		if (cp != trypath) {	/* nun-null element in path */
			/* add directory punctuation only if needed */
			if (! isdirpunct(*(cp-1)))
				*cp++ = '/';
			/* append filename */
			strcpy(cp, file);
		} else
			strcpy(trypath, file);
		if ((fd = devopen(trypath, "r")) > INVALID_HANDLE) {
			free(trypath);
			return (fd);
		}

		/* no luck, keep going */
		if(*awkpath == envsep && awkpath[1] != '\0')
			awkpath++;	/* skip colon */
	} while (*awkpath != '\0');
	free(trypath);

	/*
	 * You might have one of the awk paths defined, WITHOUT the current
	 * working directory in it. Therefore try to open the file in the
	 * current directory.
	 */
	return (devopen(file, "r"));
}

#ifdef TEST
int bufsize = 8192;

void
fatal(const char *s)
{
	printf("%s\n", s);
	exit(1);
}
#endif

/* iop_alloc --- allocate an IOBUF structure for an open fd */

static IOBUF *
iop_alloc(int fd, const char *name, IOBUF *iop)
{
        struct stat sbuf;

        if (fd == INVALID_HANDLE)
                return NULL;
        if (iop == NULL)
                emalloc(iop, IOBUF *, sizeof(IOBUF), "iop_alloc");
	memset(iop, '\0', sizeof(IOBUF));
        iop->flag = 0;
        if (isatty(fd))
                iop->flag |= IOP_IS_TTY;
        iop->readsize = iop->size = optimal_bufsize(fd, & sbuf);
        iop->sbuf = sbuf;
        if (do_lint && S_ISREG(sbuf.st_mode) && sbuf.st_size == 0)
                lintwarn(_("data file `%s' is empty"), name);
        errno = 0;
        iop->fd = fd;
        iop->count = iop->total = iop->scanoff = 0;
        iop->name = name;
        emalloc(iop->buf, char *, iop->size += 2, "iop_alloc");
        iop->off = iop->buf;
        iop->dataend = NULL;
        iop->end = iop->buf + iop->size;
	iop->flag = 0;
        return iop;
}

#define set_RT_to_null() \
	(void)(! do_traditional && (unref(RT_node->var_value), \
			   RT_node->var_value = Nnull_string))

#define set_RT(str, len) \
	(void)(! do_traditional && (unref(RT_node->var_value), \
			   RT_node->var_value = make_string(str, len)))

/* grow must increase size of buffer, set end, make sure off and dataend point at */
/* right spot.                                                              */
/*                                                                          */
/*                                                                          */
/* <growbuffer>=                                                            */
/* grow_iop_buffer --- grow the buffer */

static void
grow_iop_buffer(IOBUF *iop)
{
        size_t valid = iop->dataend - iop->off;
        size_t off = iop->off - iop->buf;
	size_t newsize;

	/*
	 * Lop off original extra two bytes, double the size,
	 * add them back.
	 */
        newsize = ((iop->size - 2) * 2) + 2;

	/* Check for overflow */
	if (newsize <= iop->size)
		fatal(_("could not allocate more input memory"));

	/* Make sure there's room for a disk block */
        if (newsize - valid < iop->readsize)
                newsize += iop->readsize + 2;

	/* Check for overflow, again */
	if (newsize <= iop->size)
		fatal(_("could not allocate more input memory"));

	iop->size = newsize;
        erealloc(iop->buf, char *, iop->size, "grow_iop_buffer");
        iop->off = iop->buf + off;
        iop->dataend = iop->off + valid;
        iop->end = iop->buf + iop->size;
}

/* <rs1>=                                                                   */
static int
rs1_get_a_record(char **out,    /* pointer to pointer to data */
        IOBUF *iop,             /* input IOP */
        register int grRS,      /* first char in RS->stptr */
        Regexp *RSre ATTRIBUTE_UNUSED,          /* regexp for RS */
        int *errcode)           /* pointer to error variable */
{
        /* We need a pointer into the buffer.                                       */
        /*                                                                          */
        /* <rs1 local vars>=                                                        */
        register char *bp;
        long retval;
#ifdef MBS_SUPPORT
                size_t mbclen = 0;
                mbstate_t mbs;
#endif


        /* The main code first checks if there was an EOF last                      */
        /* time around, and then sets up the pointers.                              */
        /*                                                                          */
        /*                                                                          */
        /* <rs1 code>=                                                              */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }
        /* First time around, set it to point to start of current record.           */
        /*                                                                          */
        /*                                                                          */
        /* <set initial pointers>=                                                  */
        bp = iop->off;

        /* Most of the work is a for loop that expands the buffer,                  */
        /* and fills it, until the rs character is found.                           */
        /*                                                                          */
        /*                                                                          */
        /* <rs1 code>=                                                              */
        for (;;) {
                /* <common buffer setup code>=                                              */
                if (/* If there's data in the buffer, and we're pointing at the end of it,      */
                    /* grow the buffer.                                                         */
                    /*                                                                          */
                    /*                                                                          */
                    /* <at end of buffer and rs not found yet>=                                 */
                    (iop->flag & IOP_AT_EOF) == 0 &&
                    iop->dataend != NULL && bp >= iop->dataend) {
                        if (iop->off > iop->buf) {
                                /* Moving the data requires remembering how far off the                     */
                                /* dataend pointer was and the bp pointer too.                              */
                                /*                                                                          */
                                /* <move data down>=                                                        */
                                size_t dataend_off = iop->dataend - iop->off;
                                memmove(iop->buf, iop->off, dataend_off);
                                iop->off = iop->buf;
                                bp = iop->dataend = iop->buf + dataend_off;

                                /* <reset pointers>=                                                        */
                                bp = iop->dataend;
                        } else {
                                /* <save position, grow buffer>=                                            */
                                iop->scanoff = bp - iop->off;
                                grow_iop_buffer(iop);
                                bp = iop->off + iop->scanoff;
                        }
                }

                /* no data in buffer or ran out of data */
                if ((iop->flag & IOP_AT_EOF) == 0 && (iop->dataend == NULL || bp >= iop->dataend)) {
                        iop->scanoff = bp - iop->off;
                        if (iop->dataend == NULL) {
                                iop->dataend = iop->buf;        /* first read */
                                if ((iop->flag & IOP_IS_INTERNAL) != 0)
                                        iop->dataend += strlen(iop->buf);
                        }
                        /* Use read to put more data into the buffer. If we've read                 */
                        /* as many characters as in the file, don't try to read more.               */
                        /*                                                                          */
                        /*                                                                          */
                        /* <put more data into the buffer>=                                         */
                        if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                                iop->flag |= IOP_AT_EOF;
                        } else if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                iop->flag |= IOP_AT_EOF;
                        else {
#define min(x, y) (x < y ? x : y)
                                /* subtract one in read count to leave room for sentinel */
                                size_t room_left = iop->end - iop->dataend - 1;
                                size_t amt_to_read = min(iop->readsize, room_left);

                                if (amt_to_read < iop->readsize) {
                                        /* <save position, grow buffer>=                                            */
                                        iop->scanoff = bp - iop->off;
                                        grow_iop_buffer(iop);
                                        bp = iop->off + iop->scanoff;
                                        /* recalculate amt_to_read */
                                        room_left = iop->end - iop->dataend - 1;
                                        amt_to_read = min(iop->readsize, room_left);
                                }
                                while (amt_to_read + iop->readsize < room_left)
                                        amt_to_read += iop->readsize;

                                iop->count = read(iop->fd, iop->dataend, amt_to_read);
                                if (iop->count == -1) {
                                        if (! do_traditional && errcode != NULL) {
                                                *errcode = errno;
                                                iop->flag |= IOP_AT_EOF;
                                                break;
                                        } else
                                                fatal(_("error reading input file `%s': %s"),
                                                        iop->name, strerror(errno));
                                } else if (iop->count == 0) {
                                        /*
                                         * hit EOF before matching RS, so end
                                         * the record and set RT to ""
                                         */
                                        iop->flag |= IOP_AT_EOF;
                                }
                                else {
                                        iop->dataend += iop->count;
                                        iop->total += iop->count;
                                        if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                                iop->flag |= IOP_AT_EOF;
                                        /* reset the sentinel */
                                        /* <set sentinel>=                                                          */
                                        *iop->dataend = grRS;


                                }
                        }

                        bp = iop->off + iop->scanoff;
                }

                /* <set sentinel>=                                                          */
                *iop->dataend = grRS;



                /* search for rs */
#ifdef MBS_SUPPORT
                if (gawk_mb_cur_max > 1) {
                        int len = iop->end - bp + 1;
                        int found = 0;
                        memset(&mbs, 0, sizeof(mbstate_t));
                        do {
                                if (*bp == grRS)
                                        found = 1;
                                mbclen = mbrlen(bp, len, &mbs);
                                if ((mbclen == 1) || (mbclen == (size_t) -1)
                                        || (mbclen == (size_t) -2) || (mbclen == 0)) {
                                        /* We treat it as a singlebyte character.  */
                                        mbclen = 1;
                                }
                                len -= mbclen;
                                bp += mbclen;
                        } while (len > 0 && ! found);
                } else
#endif
                while (*bp++ != grRS)
                        continue;

                /* At the end of the loop, if the rs char is found in the buffer,           */
                /* break.  Otherwise, back the pointer up and then check for EOF.           */
                /* If an internal file, break.                                              */
                /* If EOF, break.                                                           */
                /*                                                                          */
                /*                                                                          */
                /* <end loop logic>=                                                        */
                /* bp is one past newline that marks end of record */
                if (bp <= iop->dataend) /* found it in the buffer, not the sentinel */
                        break;

                if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                        iop->flag |= IOP_AT_EOF;
                        break;
                }

                if ((iop->flag & IOP_AT_EOF) != 0)
                        break;

                /* bp points one past sentinel, back it up for buffer management code */
                bp--;

                /* hit end of data in buffer, continue around to find more */
        }

        /* Once out of the loop, either we hit EOF, or we found the                 */
        /* character.                                                               */
        /*                                                                          */
        /*                                                                          */
        /* <rs1 code>=                                                              */
        /* now out of loop: either we hit EOF, or we found rs */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }

        /* found rs, return the record */
        *out = iop->off;
        retval = bp - iop->off - 1;
        iop->off = bp;  /* set up for next time */

        /* set RT */
        set_RT(bp - 1, 1);

        return retval;
}
/* This next part deals with the case of RS = "".  The goal is to           */
/* 1. skip any leading newlines                                             */
/* 2. scan through for multiple newlines                                    */
/* 3. If hit the end of the buffer, shuffle things down and refill          */
/*    and keep going.                                                       */
/*                                                                          */
/*                                                                          */
/* <rsnull>=                                                                */
static int
rsnull_get_a_record(char **out, /* pointer to pointer to data */
        IOBUF *iop,             /* input IOP */
        register int grRS,      /* first char in RS->stptr */
        Regexp *RSre ATTRIBUTE_UNUSED,          /* regexp for RS */
        int *errcode)           /* pointer to error variable */
{
        /* We will need at least a buffer pointer so that hopefully                 */
        /* some of the litprog code can be reused.                                  */
        /*                                                                          */
        /*                                                                          */
        /* <rsnull local vars>=                                                     */
        register char *bp;
        long retval;
        /* 2. Start scanning the buffer for newlines.  If we hit one, look for another; */
        /* we need at least 2 successive newlines to terminate the record.          */
        /*                                                                          */
        /* If we find two, save the location of the first one so we can set RT      */
        /* correctly. Then continue scanning; if we hit the end of the buffer,      */
        /* we may need to save our state in order to expand the buffer.             */
        /*                                                                          */
        /* If we hit the end of the buffer without seeing any newlines, then        */
        /* we need to save grow the buffer and keep going.                          */
        /*                                                                          */
        /*                                                                          */
        /* <rsnull local vars>=                                                     */
        size_t firstnl = 0;
        int restarting = FALSE;
        char *rt_start = NULL;


        /* ensure real sentinel value */
        grRS = '\n';

        /* Much of the code is similar to the rs1 case:                             */
        /*                                                                          */
        /*                                                                          */
        /* <rsnull code>=                                                           */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }
        /* First time around, set it to point to start of current record.           */
        /*                                                                          */
        /*                                                                          */
        /* <set initial pointers>=                                                  */
        bp = iop->off;

        /* The main loop is similar but not identical.                              */
        /*                                                                          */
        /*                                                                          */
        /* <rsnull code>=                                                           */
        for (;;) {
                /* <common buffer setup code>=                                              */
                if (/* If there's data in the buffer, and we're pointing at the end of it,      */
                    /* grow the buffer.                                                         */
                    /*                                                                          */
                    /*                                                                          */
                    /* <at end of buffer and rs not found yet>=                                 */
                    (iop->flag & IOP_AT_EOF) == 0 &&
                    iop->dataend != NULL && bp >= iop->dataend) {
                        if (iop->off > iop->buf) {
                                /* Moving the data requires remembering how far off the                     */
                                /* dataend pointer was and the bp pointer too.                              */
                                /*                                                                          */
                                /* <move data down>=                                                        */
                                size_t dataend_off = iop->dataend - iop->off;
                                memmove(iop->buf, iop->off, dataend_off);
                                iop->off = iop->buf;
                                bp = iop->dataend = iop->buf + dataend_off;

                                /* <reset pointers>=                                                        */
                                bp = iop->dataend;
                        } else {
                                /* <save position, grow buffer>=                                            */
                                iop->scanoff = bp - iop->off;
                                grow_iop_buffer(iop);
                                bp = iop->off + iop->scanoff;
                        }
                }

                /* no data in buffer or ran out of data */
                if ((iop->flag & IOP_AT_EOF) == 0 && (iop->dataend == NULL || bp >= iop->dataend)) {
                        iop->scanoff = bp - iop->off;
                        if (iop->dataend == NULL) {
                                iop->dataend = iop->buf;        /* first read */
                                if ((iop->flag & IOP_IS_INTERNAL) != 0)
                                        iop->dataend += strlen(iop->buf);
                        }
                        /* Use read to put more data into the buffer. If we've read                 */
                        /* as many characters as in the file, don't try to read more.               */
                        /*                                                                          */
                        /*                                                                          */
                        /* <put more data into the buffer>=                                         */
                        if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                                iop->flag |= IOP_AT_EOF;
                        } else if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                iop->flag |= IOP_AT_EOF;
                        else {
#define min(x, y) (x < y ? x : y)
                                /* subtract one in read count to leave room for sentinel */
                                size_t room_left = iop->end - iop->dataend - 1;
                                size_t amt_to_read = min(iop->readsize, room_left);

                                if (amt_to_read < iop->readsize) {
                                        /* <save position, grow buffer>=                                            */
                                        iop->scanoff = bp - iop->off;
                                        grow_iop_buffer(iop);
                                        bp = iop->off + iop->scanoff;
                                        /* recalculate amt_to_read */
                                        room_left = iop->end - iop->dataend - 1;
                                        amt_to_read = min(iop->readsize, room_left);
                                }
                                while (amt_to_read + iop->readsize < room_left)
                                        amt_to_read += iop->readsize;

                                iop->count = read(iop->fd, iop->dataend, amt_to_read);
                                if (iop->count == -1) {
                                        if (! do_traditional && errcode != NULL) {
                                                *errcode = errno;
                                                iop->flag |= IOP_AT_EOF;
                                                break;
                                        } else
                                                fatal(_("error reading input file `%s': %s"),
                                                        iop->name, strerror(errno));
                                } else if (iop->count == 0) {
                                        /*
                                         * hit EOF before matching RS, so end
                                         * the record and set RT to ""
                                         */
                                        iop->flag |= IOP_AT_EOF;
                                }
                                else {
                                        iop->dataend += iop->count;
                                        iop->total += iop->count;
                                        if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                                iop->flag |= IOP_AT_EOF;
                                        /* reset the sentinel */
                                        /* <set sentinel>=                                                          */
                                        *iop->dataend = grRS;


                                }
                        }

                        bp = iop->off + iop->scanoff;
                }

                /* <set sentinel>=                                                          */
                *iop->dataend = grRS;



                /* The 2.15.6 logic doesn't seem to do the trick, let's do it               */
                /* ourselves.  We need to handle the following things.                      */
                /*                                                                          */
                /* 1. Skip any leading newlines in front of the record, they don't count.   */
                /* NOT leading whitespace, just newlines. See the comment, too.             */
                /*                                                                          */
                /*                                                                          */
                /* <actual rsnull logic>=                                                   */
                /*
                 * skip any newlines at the front of the buffer,
                 * either in front of first record, or after previous
                 * record, e.g. if RS changed in the middle
                 * (see test/nulrsend)
                 */
                if (*bp == '\n') {
                        while (bp < iop->dataend && *bp == '\n')
                                bp++;
                        if (bp == iop->dataend) {
                                if ((iop->flag & IOP_AT_EOF) == 0)
                                        continue;       /* fill buffer, there's LOTS of leading newlines */
                                else {
                                        /* bug out early */
                                        iop->off = iop->dataend;
                                        *out = NULL;
                                        set_RT_to_null();
                                        return EOF;
                                }
                        } else {
                                iop->off = bp;
                        }
                }
                /* <actual rsnull logic>=                                                   */
                /*
                 * This code entered if we previously hit the first
                 * newline at exactly the end of the buffer.
                 */
                if (firstnl && restarting) {
                        restarting = FALSE;
                        /*
                         * back up to just before first newline so following
                         * logic always works.
                         */
                        bp = iop->off + firstnl - 1;
                        firstnl = 0;
                }

                /* we make use of the sentinel, so we don't have to check bp < iop->dataend */
                more:
                while (*bp++ != '\n')
                        continue;

                if (bp >= iop->dataend && (iop->flag & IOP_AT_EOF) == 0)        /* end of buffer */
                        continue;       /* refill, start over */
                else if (bp == (iop->dataend - 1) && (iop->flag & IOP_AT_EOF) == 0) {
                        /* one newline exactly at end, AND not at EOF */
                        firstnl = bp - iop->off;
                        restarting = TRUE;
                        bp++;
                        continue;                       /* refill, earlier logic catches */
                }

                /* found one newline */
                if (*bp != '\n' && bp < iop->dataend)
                        goto more;      /* only one */

                rt_start = bp - 1;      /* prev char was first newline, *bp is second */
                while (bp < iop->dataend && *bp == '\n')
                        bp++;

                if (bp >= iop->dataend && (iop->flag & IOP_AT_EOF) == 0) {
                        firstnl = bp - iop->off;
                        restarting = TRUE;
                        continue;
                } else
                        break;  /* got the record, done */



                /* Here's the logic when the record has been identified.                    */
                /*                                                                          */
                /*                                                                          */
                /* <rsnull end loop logic>=                                                 */

                if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                        iop->flag |= IOP_AT_EOF;
                        break;
                }

                if ((iop->flag & IOP_AT_EOF) != 0)
                        break;


                /* hit end of data in buffer, continue around to find more */
        }

        /* <rsnull code>=                                                           */
        /* now out of loop: either we hit EOF, or we found rs */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }

        set_RT(rt_start, bp - rt_start);

        /* found rs, return the record */
        *out = iop->off;
        retval = rt_start - iop->off;
        iop->off = bp;  /* set up for next time */

        return retval;
}
/* Now the hardest part, regex separated records.                           */
/*                                                                          */
/*                                                                          */
/* <rsre>=                                                                  */
static int
rsre_get_a_record(char **out,   /* pointer to pointer to data */
        IOBUF *iop,             /* input IOP */
        register int grRS,      /* first char in RS->stptr */
        Regexp *RSre,           /* regexp for RS */
        int *errcode)           /* pointer to error variable */
{
        /* Local vars are similar:                                                  */
        /*                                                                          */
        /*                                                                          */
        /* <rsre local vars>=                                                       */
        register char *bp;
        long retval;
        size_t restart = 0, reend = 0;
        int set_res = FALSE;


        /* And the basic code is also similar:                                      */
        /*                                                                          */
        /*                                                                          */
        /* <rsre code>=                                                             */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }
        /* First time around, set it to point to start of current record.           */
        /*                                                                          */
        /*                                                                          */
        /* <set initial pointers>=                                                  */
        bp = iop->off;


        /* Here is the main loop:                                                   */
        /*                                                                          */
        /*                                                                          */
        /* <rsre code>=                                                             */
        for (;;) {
                /* <common buffer setup code>=                                              */
                if (/* If there's data in the buffer, and we're pointing at the end of it,      */
                    /* grow the buffer.                                                         */
                    /*                                                                          */
                    /*                                                                          */
                    /* <at end of buffer and rs not found yet>=                                 */
                    (iop->flag & IOP_AT_EOF) == 0 &&
                    iop->dataend != NULL && bp >= iop->dataend) {
                        if (iop->off > iop->buf) {
                                /* Moving the data requires remembering how far off the                     */
                                /* dataend pointer was and the bp pointer too.                              */
                                /*                                                                          */
                                /* <move data down>=                                                        */
                                size_t dataend_off = iop->dataend - iop->off;
                                memmove(iop->buf, iop->off, dataend_off);
                                iop->off = iop->buf;
                                bp = iop->dataend = iop->buf + dataend_off;

                                /* <reset pointers>=                                                        */
                                bp = iop->dataend;
                        } else {
                                /* <save position, grow buffer>=                                            */
                                iop->scanoff = bp - iop->off;
                                grow_iop_buffer(iop);
                                bp = iop->off + iop->scanoff;
                        }
                }

                /* no data in buffer or ran out of data */
                if ((iop->flag & IOP_AT_EOF) == 0 && (iop->dataend == NULL || bp >= iop->dataend)) {
                        iop->scanoff = bp - iop->off;
                        if (iop->dataend == NULL) {
                                iop->dataend = iop->buf;        /* first read */
                                if ((iop->flag & IOP_IS_INTERNAL) != 0)
                                        iop->dataend += strlen(iop->buf);
                        }
                        /* Use read to put more data into the buffer. If we've read                 */
                        /* as many characters as in the file, don't try to read more.               */
                        /*                                                                          */
                        /*                                                                          */
                        /* <put more data into the buffer>=                                         */
                        if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                                iop->flag |= IOP_AT_EOF;
                        } else if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                iop->flag |= IOP_AT_EOF;
                        else {
#define min(x, y) (x < y ? x : y)
                                /* subtract one in read count to leave room for sentinel */
                                size_t room_left = iop->end - iop->dataend - 1;
                                size_t amt_to_read = min(iop->readsize, room_left);

                                if (amt_to_read < iop->readsize) {
                                        /* <save position, grow buffer>=                                            */
                                        iop->scanoff = bp - iop->off;
                                        grow_iop_buffer(iop);
                                        bp = iop->off + iop->scanoff;
                                        /* recalculate amt_to_read */
                                        room_left = iop->end - iop->dataend - 1;
                                        amt_to_read = min(iop->readsize, room_left);
                                }
                                while (amt_to_read + iop->readsize < room_left)
                                        amt_to_read += iop->readsize;

                                iop->count = read(iop->fd, iop->dataend, amt_to_read);
                                if (iop->count == -1) {
                                        if (! do_traditional && errcode != NULL) {
                                                *errcode = errno;
                                                iop->flag |= IOP_AT_EOF;
                                                break;
                                        } else
                                                fatal(_("error reading input file `%s': %s"),
                                                        iop->name, strerror(errno));
                                } else if (iop->count == 0) {
                                        /*
                                         * hit EOF before matching RS, so end
                                         * the record and set RT to ""
                                         */
                                        iop->flag |= IOP_AT_EOF;
                                }
                                else {
                                        iop->dataend += iop->count;
                                        iop->total += iop->count;
                                        if (S_ISREG(iop->sbuf.st_mode) && iop->total >= iop->sbuf.st_size)
                                                iop->flag |= IOP_AT_EOF;
                                        /* reset the sentinel */
                                        /* <set sentinel>=                                                          */
                                        *iop->dataend = grRS;


                                }
                        }

                        bp = iop->off + iop->scanoff;
                }

                /* not needed for rsre, but doesn't hurt: */
                /* <set sentinel>=                                                          */
                *iop->dataend = grRS;



                /* The hard part is the logic for regex matching.                           */
                /* Start by searching the buffer for a match. Cases:                        */
                /*                                                                          */
                /* 1. No match                                                              */
                /*         if not eof then                                                  */
                /*                 grow buffer, add more data, try again                    */
                /*         else                                                             */
                /*                 set RT to null                                           */
                /*                 return the record                                        */
                /*         endif                                                            */
                /*                                                                          */
                /* <actual rsre logic>=                                                     */
                /* case 1, no match */
                if (research(RSre, iop->off, 0, iop->dataend - iop->off, TRUE) == -1) {
                        if ((iop->flag & IOP_AT_EOF) == 0) {
                                bp = iop->dataend;
                                continue;
                        } else {
                                *out = iop->off;
                                retval = iop->dataend - iop->off;
                                iop->off = iop->dataend;
                                set_RT_to_null();
                                return retval;
                        }
                }
                /* 1a. Save the match info in variables for debugging,                      */
                /* and readability.                                                         */
                /*                                                                          */
                /*                                                                          */
                /* <actual rsre logic>=                                                     */
                restart = RESTART(RSre, iop->off);
                reend = REEND(RSre, iop->off);
                set_res = TRUE;

                /* 2. Match entirely within the bounds of the buffer:                       */
                /*         fill in RT                                                       */
                /*         break from loop to return record                                 */
                /*                                                                          */
                /*                                                                          */
                /* <actual rsre logic>=                                                     */
                /* case 2 is simple, just keep going */
                if (restart == reend) {
                        bp = iop->dataend;
                        continue;
                }
                /* 3. Match exactly at end:                                                 */
                /*         if not eof and re is not a simple string match                   */
                /*                 grow buffer, add more data, try again                    */
                /*         else                                                             */
                /*                 break from loop to set RT and return record              */
                /*         fi                                                               */
                /*                                                                          */
                /*                                                                          */
                /* <actual rsre logic>=                                                     */
                if (iop->off + reend >= iop->dataend) {
                        if ((iop->flag & IOP_AT_EOF) == 0
                            && ! reisstring(RS->stptr, RS->stlen, RSre, iop->off)) {
                                bp = iop->dataend;
                                continue;
                        } else {
                                break;
                        }
                }
                /* 4. Match within xxx bytes of end & maybe islong re:                      */
                /*         if not eof                                                       */
                /*                 grow buffer, add mor data, try again                     */
                /*         else                                                             */
                /*                 fill in RT                                               */
                /*                 break from loop to return record                         */
                /*         fi                                                               */
                /*                                                                          */
                /*                                                                          */
                /* <actual rsre logic>=                                                     */
                /*
                 * case 4, match succeeded, but there may be more in
                 * the next input buffer.
                 *
                 * Consider an RS of   xyz(abc)?   where the
                 * exact end of the buffer is   xyza  and the
                 * next two, unread, characters are bc.
                 *
                 * This matches the "xyz" and ends up putting the
                 * "abc" into the front of the next record. Ooops.
                 *
                 * The remaybelong() function looks to see if the
                 * regex contains one of: + * ? |.  This is a very
                 * simple heuristic, but in combination with the
                 * "end of match within a few bytes of end of buffer"
                 * check, should keep things reasonable.
                 */

                /*
                 * XXX: The reisstring and remaybelong tests should
                 * really be done once when RS is assigned to and
                 * then tested as flags here.  Maybe one day.
                 */

                /* succession of tests is easier to trace in GDB. */
                if ((iop->flag & IOP_AT_EOF) == 0) {
                        if (remaybelong(RS->stptr, RS->stlen)) {
                                char *matchend = iop->off + reend;

                                if (iop->dataend - matchend < RS->stlen) {
                                        bp = iop->dataend;
                                        continue;
                                }
                        }
                }



                /* Here is the end loop logic.                                              */
                /*                                                                          */
                /*                                                                          */
                /* <rsre end loop logic>=                                                   */
                if (bp <= iop->dataend) {
                        break;
                } else
                        bp--;

                if ((iop->flag & IOP_IS_INTERNAL) != 0) {
                        iop->flag |= IOP_AT_EOF;
                        break;
                }

                if ((iop->flag & IOP_AT_EOF) != 0)
                        break;


                /* hit end of data in buffer, continue around to find more */
        }
        /* And the end of function logic:                                           */
        /*                                                                          */
        /*                                                                          */
        /* <rsre code>=                                                             */
        /* now out of loop: either we hit EOF, or we found rs */
        /* Upon EOF, set RT to null, set pointer to null, return EOF.               */
        /* We don't do this unless we've also run out of data.                      */
        /*                                                                          */
        /*                                                                          */
        /* <check for EOF, return right stuff>=                                     */
        if ((iop->flag & IOP_AT_EOF) != 0 && iop->off >= iop->dataend) {
                *out = NULL;
                set_RT_to_null();
                return EOF;
        }

        /* found rs, return the record */

        assert(set_res);
        /* set RT before adjusting pointers in iop. */
        set_RT(iop->off + restart, reend - restart);

        *out = iop->off;
        retval = (iop->off + restart) - iop->off;
        iop->off += reend;

        return retval;
}

/* set_RS --- update things as appropriate when RS is set */

void
set_RS()
{
	static NODE *save_rs = NULL;

	/*
	 * Don't use cmp_nodes(), which pays attention to IGNORECASE.
	 */
	if (save_rs
		&& RS_node->var_value->stlen == save_rs->stlen
		&& STREQN(RS_node->var_value->stptr, save_rs->stptr, save_rs->stlen)) {
		/*
		 * It could be that just IGNORECASE changed.  If so,
		 * update the regex and then do the same for FS.
		 * set_IGNORECASE() relies on this routine to call
		 * set_FS().
		 */
		RS_regexp = (IGNORECASE ? RS_re_no_case : RS_re_yes_case);
		goto set_FS;
	}
	unref(save_rs);
	save_rs = dupnode(RS_node->var_value);
	RS_is_null = FALSE;
	RS = force_string(RS_node->var_value);
	if (RS_regexp != NULL) {
		refree(RS_re_yes_case);
		refree(RS_re_no_case);
		RS_re_yes_case = RS_re_no_case = RS_regexp = NULL;
	}
	if (RS->stlen == 0) {
		RS_is_null = TRUE;
		get_a_record = rsnull_get_a_record;
	} else if (RS->stlen > 1) {
		static int warned = FALSE;

		RS_re_yes_case = make_regexp(RS->stptr, RS->stlen, FALSE);
		RS_re_no_case = make_regexp(RS->stptr, RS->stlen, TRUE);
		RS_regexp = (IGNORECASE ? RS_re_no_case : RS_re_yes_case);

		get_a_record = rsre_get_a_record;

		if (do_lint && ! warned) {
			lintwarn(_("multicharacter value of `RS' is a gawk extension"));
			warned = TRUE;
		}
	} else
		get_a_record = rs1_get_a_record;
set_FS:
	if (! using_fieldwidths())
		set_FS();
}

/* pty_vs_pipe --- return true if should use pty instead of pipes for `|&' */

/*
 * This works by checking if PROCINFO["command", "pty"] exists and is true.
 */

static int
pty_vs_pipe(const char *command)
{
#ifdef HAVE_TERMIOS_H
	char *full_index;
	size_t full_len;
	NODE *val;
	NODE *sub;

	if (PROCINFO_node == NULL)
		return FALSE;

	full_len = strlen(command)
			+ SUBSEP_node->var_value->stlen
			+ 3	/* strlen("pty") */
			+ 1;	/* string terminator */
	emalloc(full_index, char *, full_len, "pty_vs_pipe");
	sprintf(full_index, "%s%.*spty", command,
		(int) SUBSEP_node->var_value->stlen, SUBSEP_node->var_value->stptr);

	sub = tmp_string(full_index, strlen(full_index));
	val = in_array(PROCINFO_node, sub);
	free_temp(sub);
	free(full_index);

	if (val) {
		if (val->flags & MAYBE_NUM)
			(void) force_number(val);
		if (val->flags & NUMBER)
			return (val->numbr != 0.0);
		else
			return (val->stlen != 0);
	}
#endif /* HAVE_TERMIOS_H */
	return FALSE;
}

/* iopflags2str --- make IOP flags printable */

const char *
iopflags2str(int flag)
{
	static struct flagtab values[] = {
		{ IOP_IS_TTY, "IOP_IS_TTY" },
		{ IOP_IS_INTERNAL, "IOP_IS_INTERNAL" },
		{ IOP_NO_FREE, "IOP_NO_FREE" },
		{ IOP_NOFREE_OBJ, "IOP_NOFREE_OBJ" },
		{ IOP_AT_EOF,  "IOP_AT_EOF" },
		{ 0, NULL }
	};

	return genflags2str(flag, values);
}
