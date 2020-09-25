/*
 * io.c --- routines for dealing with input and output and records
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

/* For OSF/1 to get struct sockaddr_storage */
#if defined(__osf__) && !defined(_OSF_SOURCE)
#define _OSF_SOURCE
#endif

#define NO_LOCALE_RPL /* avoid including "mscrtx/localerpl.h" */
#include "awk.h"

#ifdef HAVE_SYS_PARAM_H
#undef RE_DUP_MAX	/* avoid spurious conflict w/regex.h */
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#ifndef O_ACCMODE
#define O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)
#endif

#if ! defined(S_ISREG) && defined(S_IFREG)
#define	S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
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

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#else /* ! HAVE_NETINET_IN_H */
#include <in.h>
#endif /* HAVE_NETINET_IN_H */

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif	/* HAVE_SYS_SELECT_H */

#ifndef HAVE_GETADDRINFO
#include "missing_d/getaddrinfo.h"
#endif

#ifndef AI_ADDRCONFIG	/* not everyone has this symbol */
#define AI_ADDRCONFIG 0
#endif /* AI_ADDRCONFIG */

#ifndef HAVE_SOCKADDR_STORAGE
#define sockaddr_storage sockaddr	/* for older systems */
#endif /* HAVE_SOCKADDR_STORAGE */

#endif /* HAVE_SOCKETS */

#ifndef AF_UNSPEC
#define AF_UNSPEC 0
#endif
#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef AF_INET6
#define AF_INET6 10
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#if defined(__EMX__) || defined(_MSC_VER)
#include <process.h>
#endif

#if defined(__EMX__) || defined(WINDOWS_NATIVE)
/* looks like only MinGW.org defines _S_IRWXU */
# ifndef _S_IRWXU
#  define _S_IRWXU	(_S_IREAD | _S_IWRITE | _S_IEXEC)
# endif
# ifndef S_IRWXU
#  define S_IRWXU	_S_IRWXU
# endif
# ifndef S_IFDIR
#  define S_IFDIR	_S_IFDIR
# endif
#endif

#if defined(HAVE_POPEN_H)
#include "popen.h"
#endif

#ifndef ENFILE
#define ENFILE EMFILE
#endif

#if defined(VMS)
#include <ssdef.h>
#ifndef SS$_EXBYTLM
#define SS$_EXBYTLM 0x2a14  /* VMS 8.4 seen */
#endif
#include <rmsdef.h>
#endif

#ifdef WINDOWS_NATIVE
#include "mscrtx/is_socket.h"
#include "mscrtx/socket_file.h"
#include "mscrtx/localerpl.h"
#endif

#ifdef WINDOWS_NATIVE
#include "oldnames.h"
#endif

#ifdef HAVE_SOCKETS

#ifndef SHUT_RD
# ifdef SD_RECEIVE
#  define SHUT_RD	SD_RECEIVE
# else
#  define SHUT_RD	0
# endif
#endif

#ifndef SHUT_WR
# ifdef SD_SEND
#  define SHUT_WR	SD_SEND
# else
#  define SHUT_WR	1
# endif
#endif

#ifndef SHUT_RDWR
# ifdef SD_BOTH
#  define SHUT_RDWR	SD_BOTH
# else
#  define SHUT_RDWR	2
# endif
#endif

#ifndef socket_dup
# define socket_dup(oldfd)	dup(oldfd)
#endif

#endif /* HAVE_SOCKETS */

#ifndef closemaybesocket
# define closemaybesocket(fd)	close(fd)
#endif

#ifdef HAVE_TERMIOS_H
#ifndef HAVE_SETSID
#define setsid()	/* nothing */
#endif
#endif

#if defined(_AIX)
#undef TANDEM	/* AIX defines this in one of its header files */
#endif

#ifdef __DJGPP__
#define PIPES_SIMULATED
#endif

#ifdef WINDOWS_NATIVE
# ifndef PIPES_SIMULATED
#  define pipe(fds)	_pipe(fds, 0, O_NOINHERIT)
# endif
#endif

#ifdef HAVE_MPFR
/* increment NR or FNR */
#define INCREMENT_REC(X)	(do_mpfr && X == (LONG_MAX - 1)) ? \
				(mpz_add_ui(M##X, M##X, 1), X = 0) : X++
#else
#define INCREMENT_REC(X)	X++
#endif

/* Several macros to make the code a bit clearer. */
#define at_eof(iop)     (((iop)->flag & IOP_AT_EOF) != 0)
#define has_no_data(iop)        ((iop)->dataend == NULL)
#define no_data_left(iop)	((iop)->off >= (iop)->dataend)
#define buffer_has_all_data(iop) \
	((size_t) ((iop)->dataend - (iop)->off) == (size_t) (iop)->pubc.sbuf.st_size)

/* Bad pid value.  */
#define BAD_PID -1

/*
 * The key point to the design is to split out the code that searches through
 * a buffer looking for the record and the terminator into separate routines,
 * with a higher-level routine doing the reading of data and buffer management.
 * This makes the code easier to manage; the buffering code is the same
 * independent of how we find a record.  Communication is via the return
 * value:
 */

typedef enum recvalues {
        REC_OK,         /* record and terminator found, recmatch struct filled in */
        NOTERM,         /* no terminator found, give me more input data */
        TERMATEND,      /* found terminator at end of buffer */
        TERMNEAREND     /* found terminator close to end of buffer, for when
			   the RE might be match more data further in
			   the file. */
} RECVALUE;

/*
 * Between calls to a scanning routine, the state is stored in
 * an enum scanstate variable.  Not all states apply to all
 * variants, but the higher code doesn't really care.
 */

typedef enum scanstate {
        NOSTATE,        /* scanning not started yet (all) */
        INLEADER,       /* skipping leading data (RS = "") */
        INDATA,         /* in body of record (all) */
        INTERM          /* scanning terminator (RS = "", RS = regexp) */
} SCANSTATE;

/*
 * When a record is seen (REC_OK or TERMATEND), the following
 * structure is filled in.
 */

struct recmatch {
        char *start;    /* record start */
        size_t len;     /* length of record */
        char *rt_start; /* start of terminator */
        size_t rt_len;  /* length of terminator */
};

static int iop_close(IOBUF *iop);
static void close_one(void);
static int close_redir(struct redirect *rp, bool exitwarn, two_way_close_type how);
#ifndef PIPES_SIMULATED
static int wait_any(pid_t interesting);
#endif
static IOBUF *gawk_popen(const char *cmd, struct redirect *rp);
static IOBUF *iop_alloc(fd_t fd, const char *name, int errno_val);
static IOBUF *iop_finish(IOBUF *iop);
static int gawk_pclose(struct redirect *rp);
static int str2mode(const char *mode);
static int two_way_open(const char *str, struct redirect *rp, fd_t extfd);
#ifdef HAVE_TERMIOS_H
static bool pty_vs_pipe(const char *command);
#endif
static void find_input_parser(IOBUF *iop);
static bool find_output_wrapper(awk_output_buf_t *outbuf);
static void init_output_wrapper(awk_output_buf_t *outbuf);
static bool find_two_way_processor(const char *name, struct redirect *rp);

static RECVALUE rs1scan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state);
static RECVALUE rsnullscan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state);
static RECVALUE rsrescan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state);

static RECVALUE (*matchrec)(IOBUF *iop, struct recmatch *recm, SCANSTATE *state) = rs1scan;

static int get_a_record(char **out, IOBUF *iop, int *errcode, const awk_fieldwidth_info_t **field_width);

static void free_rp(struct redirect *rp);

struct inet_socket_info {
	int family;		/* AF_UNSPEC, AF_INET, or AF_INET6 */
	int protocol;		/* SOCK_STREAM or SOCK_DGRAM */
	/*
	 * N.B. If we used 'char *' or 'const char *' pointers to the
	 * substrings, it would trigger compiler warnings about the casts
	 * in either inetfile() or devopen().  So we use offset/len to
	 * avoid that.
	 */
	struct {
		unsigned offset;
		unsigned len;
	} localport, remotehost, remoteport;
};

static bool inetfile(const char *str, size_t len, struct inet_socket_info *isn);

static NODE *in_PROCINFO(const char *pidx1, size_t len1, const char *pidx2, size_t len2, NODE **full_idx);
static awk_ulong_t get_read_timeout(IOBUF *iop);
static ssize_t read_with_timeout(fd_t fd, void *buf, size_t size);

static bool read_can_timeout = false;
static awk_ulong_t read_timeout;
static awk_ulong_t read_default_timeout;

static struct redirect *red_head = NULL;
static NODE *RS = NULL;
static Regexp *RS_re[2];	/* index 0 - don't ignore case, index 1, do */
static Regexp *RS_regexp;

static const char nonfatal[] = "NONFATAL";

bool RS_is_null;

extern NODE *ARGC_node;
extern NODE *ARGV_node;
extern NODE *ARGIND_node;
extern NODE **fields_arr;

static inline NODE *
in_PROCINFO_z(const char *pidx1, const char *pidx2, NODE **full_idx)
{
	return in_PROCINFO(pidx1, pidx1 ? strlen(pidx1) : 0, pidx2, pidx2 ? strlen(pidx2) : 0, full_idx);
}

/* init_io --- set up timeout related variables */

void
init_io(void)
{
	awk_long_t tmout;

	/* Only MinGW has a non-trivial implementation of this.  */
	init_sockets();

	/*
	 * N.B.: all these hacks are to minimize the effect
	 * on programs that do not care about timeout.
	 */

	/* Parse the env. variable only once */
	tmout = getenv_long("GAWK_READ_TIMEOUT");
	if (tmout > 0) {
		read_default_timeout = (awk_ulong_t) tmout;
		read_can_timeout = true;
	}

	/*
	 * PROCINFO entries for timeout are dynamic;
	 * We can't be any more specific than this.
	 */
	if (PROCINFO_node != NULL)
		read_can_timeout = true;
}


#if defined(__DJGPP__) || defined(__EMX__) || defined(__CYGWIN__) || defined(WINDOWS_NATIVE)
/* binmode --- convert BINMODE to string for fopen */

static const char *
binmode(const char *mode)
{
	switch (mode[0]) {
	case 'r':
		if ((BINMODE & BINMODE_INPUT) != 0)
			mode = "rb";
		break;
	case 'w':
	case 'a':
		if ((BINMODE & BINMODE_OUTPUT) != 0)
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
static int vmsrtl_fileno(FILE *);
static int vmsrtl_fileno(fp) FILE *fp; { return fileno(fp); }
#undef fileno
#define fileno(FP) (((FP) && *(FP)) ? vmsrtl_fileno(FP) : INVALID_HANDLE)
#endif	/* VMS */

/* after_beginfile --- reset necessary state after BEGINFILE has run */

void
after_beginfile(IOBUF **cur)
{
	IOBUF *iop;

	iop = *cur;
	assert(iop != NULL);

	/*
	 * Input parsers could have been changed by BEGINFILE,
	 * so delay check until now.
	 */

	find_input_parser(iop);

	if (! iop->valid) {
		const char *fname;
		int errcode;
		bool valid;

		fname = iop->publ.name;
		errcode = iop->errcode;
		valid = iop->valid;
		errno = 0;
		update_ERRNO_int(errcode);
		(void) iop_close(iop);
		*cur = NULL;
		if (! valid && errcode == EISDIR && ! do_traditional) {
			awkwarn(_("command line argument `%s' is a directory: skipped"), fname);
			return;		/* read next file */
		}
		fatal(_("cannot open file `%s' for reading: %s"),
				fname, strerror(errcode));
	}
}

/* nextfile --- move to the next input data file */
/*
 * Return value > 0 ----> run BEGINFILE block
 * *cur = NULL  ----> hit EOF, run ENDFILE block
 */

awk_long_t
nextfile(IOBUF **cur, bool skipping)
{
	static awk_long_t i = 1;
	static bool files = false;
	NODE *arg, *tmp;
	const char *fname;
	fd_t fd = INVALID_HANDLE;
	int errcode;
	IOBUF *iop = *cur;
	awk_long_t argc;

	if (skipping) {			/* for 'nextfile' call */
		errcode = 0;
		if (iop != NULL) {
			errcode = iop->errcode;
			(void) iop_close(iop);
		}
		*cur = NULL;
		return errcode ? 0 : 1;
	}

	if (iop != NULL) {
		if (at_eof(iop)) {
			assert(iop->publ.fd != INVALID_HANDLE);
			(void) iop_close(iop);
			*cur = NULL;
			return 1;	/* run endfile block */
		} else
			return 0;
	}

	argc = get_number_si(ARGC_node->var_value);

	for (; i < argc; i++) {
		tmp = make_number((AWKNUM) i);
		(void) force_string(tmp);
		arg = in_array(ARGV_node, tmp);
		unref(tmp);
		if (arg == NULL || arg->stlen == 0)
			continue;
		arg = force_string(arg);
		if (! do_traditional) {
			unref(ARGIND_node->var_value);
			ARGIND_node->var_value = make_number((AWKNUM) i);
		}

		if (! arg_assign(arg->stptr, false)) {
			files = true;
			fname = arg->stptr;

			/* manage the awk variables: */
			unref(FILENAME_node->var_value);
			FILENAME_node->var_value = dupnode(arg);
#ifdef HAVE_MPFR
			if (is_mpg_number(FNR_node->var_value))
				mpz_set_ui(MFNR, 0);
#endif
			FNR = 0;

			/* IOBUF management: */
			errno = 0;
			fd = devopen(fname, binmode("r"));
			if (fd == INVALID_HANDLE && errno == EMFILE) {
				close_one();
				close_one();
				fd = devopen(fname, binmode("r"));
			}
			errcode = errno;
			if (! do_traditional)
				update_ERRNO_int(errno);
			iop = iop_alloc(fd, fname, errcode);
			*cur = iop_finish(iop);
			if (iop->publ.fd == INVALID_HANDLE)
				iop->errcode = errcode;
			else if (iop->valid)
				iop->errcode = 0;

			if (! do_traditional && iop->errcode != 0)
				update_ERRNO_int(iop->errcode);

			return ++i;	/* run beginfile block */
		}
	}

	if (files == false) {
		files = true;
		/* no args. -- use stdin */
		/* FNR is init'ed to 0 */
		errno = 0;
		if (! do_traditional)
			update_ERRNO_int(errno);

		unref(FILENAME_node->var_value);
		FILENAME_node->var_value = make_string("-", 1);
		FILENAME_node->var_value->flags |= USER_INPUT; /* be pedantic */
		fname = "-";
		iop = iop_alloc(fileno(stdin), fname, 0);
		*cur = iop_finish(iop);

		if (iop->publ.fd == INVALID_HANDLE) {
			errcode = errno;
			errno = 0;
			update_ERRNO_int(errno);
			(void) iop_close(iop);
			*cur = NULL;
			fatal(_("cannot open file `%s' for reading: %s"),
					fname, strerror(errcode));
		}
		return ++i;	/* run beginfile block */
	}

	return -1;	/* end of input, run end block or Op_atexit */
}

/* set_FNR --- update internal FNR from awk variable */

void
set_FNR(void)
{
	NODE *n = FNR_node->var_value;
	(void) force_number(n);
#ifdef HAVE_MPFR
	if (is_mpg_number(n))
		FNR = mpg_set_var(FNR_node);
	else
#endif
	FNR = get_number_fn(n);
}

/* set_NR --- update internal NR from awk variable */

void
set_NR(void)
{
	NODE *n = NR_node->var_value;
	(void) force_number(n);
#ifdef HAVE_MPFR
	if (is_mpg_number(n))
		NR = mpg_set_var(NR_node);
	else
#endif
	NR = get_number_fn(n);
}

/* inrec --- This reads in a record from the input file */

bool
inrec(IOBUF *iop, int *errcode)
{
	char *begin = NULL; /* Silence compiler.  */
	int cnt;
	bool retval = true;
	const awk_fieldwidth_info_t *field_width = NULL;

	if (at_eof(iop) && no_data_left(iop))
		cnt = EOF;
	else if ((iop->flag & IOP_CLOSED) != 0)
		cnt = EOF;
	else
		cnt = get_a_record(& begin, iop, errcode, & field_width);

	/* Note that get_a_record may return -2 when I/O would block */
	if (cnt < 0) {
		retval = false;
	} else {
		INCREMENT_REC(NR);
		INCREMENT_REC(FNR);
		set_record(begin, (size_t) cnt, field_width);
		if (*errcode > 0)
			retval = false;
	}

	return retval;
}

/* remap_std_file --- reopen a standard descriptor on /dev/null */

static int
remap_std_file(fd_t oldfd)
{
	fd_t newfd, fd;
	int ret;

	/*
	 * Give OS-specific routines in gawkmisc.c a chance to interpret
	 * "/dev/null" as appropriate for their platforms.
	 */
	newfd = os_devopen("/dev/null", O_RDWR);
	if (newfd == INVALID_HANDLE)
		newfd = open("/dev/null", O_RDWR);
	if (newfd != INVALID_HANDLE) {
		/* if oldfd is open, dup2() will close oldfd for us first. */
		int save_errno;
		fd = dup2(newfd, oldfd);
		save_errno = errno;
		close(newfd);
		if (fd == INVALID_HANDLE)
			errno = save_errno;
		ret = fd; /* INVALID_HANDLE aka -1 if dup2() failed */
	} else
		ret = 0; /* can't open "/dev/null", ignore an error */

	return ret;
}

/* iop_close --- close an open IOP */

static int
iop_close(IOBUF *iop)
{
	int ret = 0;

	if (iop == NULL)
		return 0;

	errno = 0;

	iop->flag &= ~IOP_AT_EOF;
	iop->flag |= IOP_CLOSED;	/* there may be dangling pointers */
	iop->dataend = NULL;
	/*
	 * Closing standard files can cause crufty code elsewhere to lose.
	 * So we remap the standard file to /dev/null.
	 * Thanks to Jim Meyering for the suggestion.
	 */
	if (iop->publ.close_func != NULL)
		iop->publ.close_func(&iop->publ);

	if (iop->publ.fd != INVALID_HANDLE) {
		if (iop->publ.fd == fileno(stdin)
		    || iop->publ.fd == fileno(stdout)
		    || iop->publ.fd == fileno(stderr))
			ret = remap_std_file(iop->publ.fd);
#if defined(__EMX__) || defined(WINDOWS_NATIVE)
		/* On OS/2 and Windows directory access via open() is
		   not permitted.  So don't close().  */
		else if (S_ISDIR(iop->pubc.sbuf.st_mode))
			ret = 0;
#endif
		else if (S_ISREG(iop->pubc.sbuf.st_mode)
#if !defined(__EMX__) && !defined(WINDOWS_NATIVE)
#ifdef S_ISBLK
		    || S_ISBLK(iop->pubc.sbuf.st_mode)
#endif
#endif
#ifdef S_ISLNK
		    || S_ISLNK(iop->pubc.sbuf.st_mode)
#endif
		    || S_ISDIR(iop->pubc.sbuf.st_mode)
		    || S_ISCHR(iop->pubc.sbuf.st_mode)
		    || S_ISFIFO(iop->pubc.sbuf.st_mode))
			ret = close(iop->publ.fd);
		else
			ret = closemaybesocket(iop->publ.fd);
	}

	if (ret == -1)
		awkwarn(_("close of fd %d (`%s') failed: %s"), iop->publ.fd,
				iop->publ.name, strerror(errno));
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
			 * This used to be here:
			 *
			 * reset_record();
			 *
			 * Don't do that; reset_record() throws away all fields,
			 * saves FS etc.  We just need to make sure memory isn't
			 * corrupted and that references to $0 and fields work.
			 */
		}
		efree(iop->buf);
		iop->buf = NULL;
	}
	efree(iop);
	return ret == -1 ? 1 : 0;
}

/* redflags2str --- turn redirection flags into a string, for debugging */

const char *
redflags2str(redirect_flags_t flags)
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

/* redirect_string --- Redirection for printf and print commands, use string info */

struct redirect *
redirect_string(const char *str, size_t explen, bool not_string,
		enum redirval redirtype, int *errflg, fd_t extfd, bool failure_fatal)
{
	struct redirect *rp;
	redirect_flags_t tflag = 0;
	redirect_flags_t outflag = 0;
	const char *direction = "to";
	const char *mode;
	fd_t fd;
	const char *what = NULL;
	bool new_rp = false;
#ifdef HAVE_SOCKETS
	struct inet_socket_info isi;
#endif
	static struct redirect *save_rp = NULL;	/* hold onto rp that should
	                                         * be freed for reuse
	                                         */

	if (do_sandbox)
		fatal(_("redirection not allowed in sandbox mode"));

	switch (redirtype) {
	case redirect_append:
		tflag = RED_APPEND;
		/* FALL THROUGH */
	case redirect_output:
		outflag = (RED_FILE|RED_WRITE);
		tflag |= outflag;
		if (redirtype == redirect_output)
			what = ">";
		else
			what = ">>";
		break;
	case redirect_pipe:
		tflag = (RED_PIPE|RED_WRITE);
		what = "|";
		break;
	case redirect_pipein:
		tflag = (RED_PIPE|RED_READ);
		what = "|";
		break;
	case redirect_input:
		tflag = (RED_FILE|RED_READ);
		what = "<";
		break;
	case redirect_twoway:
		tflag = (RED_READ|RED_WRITE|RED_TWOWAY);
		what = "|&";
		break;
	default:
		cant_happen();
	}
	if (do_lint && not_string)
		lintwarn(_("expression in `%s' redirection is a number"),
			what);

	if (explen < 1 || str == NULL || *str == '\0')
		fatal(_("expression for `%s' redirection has null string value"),
			what);

	if (do_lint && (strncmp(str, "0", explen) == 0
			|| strncmp(str, "1", explen) == 0))
		lintwarn(_("filename `%.*s' for `%s' redirection may be result of logical expression"),
				TO_PRINTF_WIDTH(explen), str, what);

#ifdef HAVE_SOCKETS
	/*
	 * Use /inet4 to force IPv4, /inet6 to force IPv6, and plain
	 * /inet will be whatever we get back from the system.
	 */
	if (inetfile(str, explen, & isi)) {
		tflag |= RED_SOCKET;
		if (isi.protocol == SOCK_STREAM)
			tflag |= RED_TCP;	/* use shutdown when closing */
	}
#endif /* HAVE_SOCKETS */

	for (rp = red_head; rp != NULL; rp = rp->next) {
#ifndef PIPES_SIMULATED
		/*
		 * This is an efficiency hack.  We want to
		 * recover the process slot for dead children,
		 * if at all possible.  Messing with signal() for
		 * SIGCLD leads to lots of headaches.  However, if
		 * we've gotten EOF from a child input pipeline, it's
		 * a good bet that the child has died. So recover it.
		 */
		if ((rp->flag & RED_EOF) != 0 && redirtype == redirect_pipein) {
			if (rp->pid != BAD_PID)
#ifdef WINDOWS_NATIVE
				/* MinGW cannot wait for any process.  */
				wait_any(rp->pid);
#else
				wait_any(0);
#endif
		}
#endif /* PIPES_SIMULATED */

#define RED_STATE_MASK (RED_NOBUF|RED_EOF|RED_PTY)

		/* now check for a match */
		if (strlen(rp->value) == explen
		    && memcmp(rp->value, str, explen) == 0
		    && ((rp->flag & ~RED_STATE_MASK) == tflag
			|| (outflag != 0
			    && (rp->flag & (RED_FILE|RED_WRITE)) == outflag))) {

			redirect_flags_t rpflag = rp->flag & ~RED_STATE_MASK;
			redirect_flags_t newflag = tflag & ~RED_STATE_MASK;

			if (do_lint && rpflag != newflag)
				lintwarn(
		_("unnecessary mixing of `>' and `>>' for file `%.*s'"),
					TO_PRINTF_WIDTH(explen), rp->value);

			break;
		}
	}

	if (rp == NULL) {
		char *newstr;
		new_rp = true;
		if (save_rp != NULL) {
			rp = save_rp;
			efree(rp->value);
		} else
			emalloc(rp, struct redirect *, sizeof(struct redirect), "redirect");
		emalloc(newstr, char *, explen + 1, "redirect");
		memcpy(newstr, str, explen);
		newstr[explen] = '\0';
		str = newstr;
		rp->value = newstr;
		rp->flag = tflag;
		init_output_wrapper(& rp->output);
		rp->output.name = str;
		rp->iop = NULL;
		rp->pid = BAD_PID;
		rp->status = 0;
	} else
		str = rp->value;	/* get \0 terminated string */
	save_rp = rp;

	while (rp->output.file == NULL && rp->iop == NULL) {
		if (! new_rp && (rp->flag & RED_EOF) != 0) {
			/*
			 * Encountered EOF on file or pipe -- must be cleared
			 * by explicit close() before reading more
			 */
			save_rp = NULL;
			return rp;
		}
		mode = NULL;
		errno = 0;
		switch (redirtype) {
		case redirect_output:
			mode = binmode("w");
			if ((rp->flag & RED_USED) != 0)
				mode = (rp->mode[1] == 'b') ? "ab" : "a";
			break;
		case redirect_append:
			mode = binmode("a");
			break;
		case redirect_pipe:
			if (extfd >= 0) {
				awkwarn(_("get_file cannot create pipe `%s' with fd %d"), str, extfd);
				return NULL;
			}
			/* synchronize output before new pipe */
			(void) flush_io();

			os_restore_mode(fileno(stdin));
			set_sigpipe_to_default();
			/*
			 * Don't check failure_fatal; see input pipe below.
			 * Note that the failure happens upon failure to fork,
			 * using a non-existant program will still succeed the
			 * popen().
			 */
			if ((rp->output.file = popen(str, binmode("w"))) == NULL)
				fatal(_("cannot open pipe `%s' for output: %s"),
						str, strerror(errno));
			ignore_sigpipe();

			/* set close-on-exec */
			os_close_on_exec(fileno((FILE*) rp->output.file), str, "pipe", "to");
			rp->flag |= RED_NOBUF;
			break;
		case redirect_pipein:
			if (extfd >= 0) {
				awkwarn(_("get_file cannot create pipe `%s' with fd %d"), str, extfd);
				return NULL;
			}
			direction = "from";
			if (gawk_popen(str, rp) == NULL)
				fatal(_("cannot open pipe `%s' for input: %s"),
					str, strerror(errno));
			break;
		case redirect_input:
			direction = "from";
			fd = (extfd >= 0) ? extfd : devopen(str, binmode("r"));
			if (fd == INVALID_HANDLE && errno == EISDIR) {
				*errflg = EISDIR;
				/* do not free rp, saving it for reuse (save_rp = rp) */
				return NULL;
			}
			rp->iop = iop_alloc(fd, str, errno);
			find_input_parser(rp->iop);
			iop_finish(rp->iop);
			if (! rp->iop->valid) {
				if (! do_traditional && rp->iop->errcode != 0)
					update_ERRNO_int(rp->iop->errcode);
				(void) iop_close(rp->iop);
				rp->iop = NULL;
			}
			break;
		case redirect_twoway:
#ifndef HAVE_SOCKETS
			if (extfd >= 0) {
				awkwarn(_("get_file socket creation not supported on this platform for `%s' with fd %d"), str, extfd);
				return NULL;
			}
#endif
			direction = "to/from";
			if (! two_way_open(str, rp, extfd)) {
				if (! failure_fatal || is_non_fatal_redirect(str, explen)) {
					*errflg = errno;
					/* do not free rp, saving it for reuse (save_rp = rp) */
					return NULL;
				} else
					fatal(_("cannot open two way pipe `%s' for input/output: %s"),
							str, strerror(errno));
			}
			break;
		default:
			cant_happen();
		}

		if (mode != NULL) {
			errno = 0;
			rp->output.mode = mode;
			fd = (extfd >= 0) ? extfd : devopen(str, mode);

			if (fd > INVALID_HANDLE) {
				if (fd == fileno(stdin))
					rp->output.file = stdin;
				else if (fd == fileno(stdout))
					rp->output.file = stdout;
				else if (fd == fileno(stderr))
					rp->output.file = stderr;
				else {
					const char *omode = mode;
#ifdef WINDOWS_NATIVE
					if (!is_socket(fd))
#endif
					{
#if defined(F_GETFL) && defined(O_APPEND)
						int fd_flags = fcntl(fd, F_GETFL);
						if (fd_flags != -1 &&
						    (fd_flags & O_APPEND) == O_APPEND)
							omode = binmode("a");
#endif
					}

					os_close_on_exec(fd, str, "file", "");
#ifdef WINDOWS_NATIVE
					if (is_socket(fd)) {
						rp->output.file = socket_file_alloc();
						if (rp->output.file != NULL)
							rp->output.socket_fd = fd;
					}
					else
#endif
						rp->output.file = fdopen(fd, omode);
					/* don't leak file descriptors */
					if (rp->output.file == NULL)
						closemaybesocket(fd);
					rp->mode = mode;
				}
				if (rp->output.file != NULL && os_isatty(fd))
					rp->flag |= RED_NOBUF;

				/* Move rp to the head of the list. */
				if (! new_rp && red_head != rp) {
					if ((rp->prev->next = rp->next) != NULL)
						rp->next->prev = rp->prev;
					red_head->prev = rp;
					rp->prev = NULL;
					rp->next = red_head;
					red_head = rp;
				}
			}
			find_output_wrapper(& rp->output);
		}

		if (rp->output.file == NULL && rp->iop == NULL) {
			/* too many files open -- close one and try again */
			if (errno == EMFILE || errno == ENFILE)
				close_one();
#ifdef VMS
			/* Alpha/VMS V7.1+ C RTL is returning these instead
			   of EMFILE (haven't tried other post-V6.2 systems) */
			else if ((errno == EIO || errno == EVMSERR) &&
                                 (vaxc$errno == SS$_EXQUOTA ||
                                  vaxc$errno == SS$_EXBYTLM ||
                                  vaxc$errno == RMS$_ACC ||
				  vaxc$errno == RMS$_SYN)) {
				close_one();
				close_one();
			}
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
				 *
				 * 12/2014: Take nonfatal settings in PROCINFO into account.
				 */
				if (errflg != NULL)
					*errflg = errno;
				if (failure_fatal && ! is_non_fatal_redirect(str, explen) &&
				    (redirtype == redirect_output
				     || redirtype == redirect_append)) {
					/* multiple messages make life easier for translators */
					if (*direction == 'f')
						fatal(_("cannot redirect from `%s': %s"),
					    		str, strerror(errno));
					else
						fatal(_("cannot redirect to `%s': %s"),
							str, strerror(errno));
				} else {
					/* do not free rp, saving it for reuse (save_rp = rp) */
					return NULL;
				}
			}
		}
	}

	if (new_rp) {
		/*
		 * It opened successfully, hook it into the list.
		 * Maintain the list in most-recently-used first order.
		 */
		if (red_head != NULL)
			red_head->prev = rp;
		rp->prev = NULL;
		rp->next = red_head;
		red_head = rp;
	}
	save_rp = NULL;
	return rp;
}

/* redirect --- Redirection for printf and print commands */

struct redirect *
redirect(NODE *redir_exp, enum redirval redirtype, int *errflg, bool failure_fatal)
{
	bool not_string = ((fixtype(redir_exp)->flags & STRING) == 0);

	redir_exp = force_string(redir_exp);
	return redirect_string(redir_exp->stptr, redir_exp->stlen, not_string,
				redirtype, errflg, BAD_PID, failure_fatal);
}

/* getredirect --- find the struct redirect for this file or pipe */

struct redirect *
getredirect(const char *str, size_t len)
{
	struct redirect *rp;

	for (rp = red_head; rp != NULL; rp = rp->next)
		if (strlen(rp->value) == len && memcmp(rp->value, str, len) == 0)
			return rp;

	return NULL;
}

/* is_non_fatal_std --- return true if fp is stdout/stderr and nonfatal */

bool
is_non_fatal_std(FILE *fp)
{
	if (in_PROCINFO_z(nonfatal, NULL, NULL))
		return true;

	/* yucky logic. sigh. */
	if (fp == stdout) {
		return (   in_PROCINFO_z("-", nonfatal, NULL) != NULL
		        || in_PROCINFO_z("/dev/stdout", nonfatal, NULL) != NULL);
	} else if (fp == stderr) {
		return (in_PROCINFO_z("/dev/stderr", nonfatal, NULL) != NULL);
	}

	return false;
}

/* is_non_fatal_redirect --- return true if redirected I/O should be nonfatal */

bool
is_non_fatal_redirect(const char *str, size_t len)
{
	return in_PROCINFO_z(nonfatal, NULL, NULL) != NULL
	       || in_PROCINFO(str, len, nonfatal, strlen(nonfatal), NULL) != NULL;
}

#ifdef WINDOWS_NATIVE
#define awk_output_buf_reset(outbuf) \
		((void) ((outbuf)->file = NULL, (outbuf)->socket_fd = INVALID_HANDLE))
#else
#define awk_output_buf_reset(outbuf)		((outbuf)->file = NULL)
#endif

/* close_one --- temporarily close an open file to re-use the fd */

static void
close_one(void)
{
	struct redirect *rp;
	struct redirect *rplast = NULL;

	static bool warned = false;

	if (do_lint && ! warned) {
		warned = true;
		lintwarn(_("reached system limit for open files: starting to multiplex file descriptors"));
	}

	/* go to end of list first, to pick up least recently used entry */
	for (rp = red_head; rp != NULL; rp = rp->next)
		rplast = rp;
	/* now work back up through the list */
	for (rp = rplast; rp != NULL; rp = rp->prev) {
		if (rp->output.file == NULL)
			continue;

#ifdef WINDOWS_NATIVE
		if (rp->output.socket_fd == INVALID_HANDLE)
#endif
		{
			/* don't close standard files! */
			if (rp->output.file == stderr || rp->output.file == stdout)
				continue;
		}

		if ((rp->flag & (RED_FILE|RED_WRITE)) == (RED_FILE|RED_WRITE)) {
			rp->flag |= RED_USED;
			errno = 0;
			if (rp->output.gawk_fclose(&rp->output) != 0)
				awkwarn(_("close of `%s' failed: %s"),
					rp->value, strerror(errno));
			awk_output_buf_reset(&rp->output);
			break;
		}
	}
	if (rp == NULL)
		/* surely this is the only reason ??? */
		fatal(_("too many pipes or input files open"));
}

/* do_close --- completely close an open file or pipe */

NODE *
do_close(nargs_t nargs)
{
	NODE *tmp, *tmp2;
	int res;
	struct redirect *rp;
	two_way_close_type how = CLOSE_ALL;	/* default */

	if (nargs == 2u) {
		/* 2nd arg if present: "to" or "from" for two-way pipe */
		/* DO NOT use _() on the strings here! */
		char save;

		tmp2 = POP_STRING();
		save = tmp2->stptr[tmp2->stlen];
		tmp2->stptr[tmp2->stlen] = '\0';
		if (cmp_keyword(tmp2->stptr, "to", 3))
			how = CLOSE_TO;
		else if (cmp_keyword(tmp2->stptr, "from", 5))
			how = CLOSE_FROM;
		else {
			DEREF(tmp2);
			fatal(_("close: second argument must be `to' or `from'"));
		}
		tmp2->stptr[tmp2->stlen] = save;
		DEREF(tmp2);
	}

	tmp = POP_STRING(); 	/* 1st arg: redir to close */

	for (rp = red_head; rp != NULL; rp = rp->next) {
		if (strlen(rp->value) == tmp->stlen
		    && memcmp(rp->value, tmp->stptr, tmp->stlen) == 0)
			break;
	}

	if (rp == NULL) {	/* no match, return -1 */
		const char *cp;

		if (do_lint)
			lintwarn(_("close: `%.*s' is not an open file, pipe or co-process"),
				TO_PRINTF_WIDTH(tmp->stlen), tmp->stptr);

		if (! do_traditional) {
			/* update ERRNO manually, using errno = ENOENT is a stretch. */
			cp = _("close of redirection that was never opened");
			update_ERRNO_string(cp);
		}

		DEREF(tmp);
		return make_number((AWKNUM) -1.0);
	}
	DEREF(tmp);
	fflush(stdout);	/* synchronize regular output */
	res = close_redir(rp, false, how);
	/*
	 * POSIX says close() returns 0 on success, non-zero otherwise.
	 * For POSIX, at this point we just return 0.  Otherwise we
	 * return the exit status of the process or of pclose(), depending.
	 * Down in the call tree of close_redir(), we rationalize the
	 * value like we do for system().
	 */
	if (do_posix)
		res = 0;
	return make_number((AWKNUM) res);
}

/* close_rp --- separate function to just do closing */

int
close_rp(struct redirect *rp, two_way_close_type how)
{
	int status = 0;

	errno = 0;
	if ((rp->flag & RED_TWOWAY) != 0) {	/* two-way pipe */
		/* write end: */
		if ((how == CLOSE_ALL || how == CLOSE_TO) && rp->output.file != NULL) {
#ifdef HAVE_SOCKETS
			if ((rp->flag & RED_TCP) != 0) {
				fd_t fd;
#ifdef WINDOWS_NATIVE
				fd = rp->output.socket_fd;
#else
				fd = fileno((FILE*) rp->output.file);
#endif
				(void) shutdown(fd, SHUT_WR);
			}
#endif /* HAVE_SOCKETS */

			if ((rp->flag & RED_PTY) != 0) {
				rp->output.gawk_fwrite("\004\n", sizeof("\004\n") - 1, 1, &rp->output);
				rp->output.gawk_fflush(&rp->output);
			}
			status = rp->output.gawk_fclose(&rp->output);
			awk_output_buf_reset(&rp->output);
		}

		/* read end: */
		if (how == CLOSE_ALL || how == CLOSE_FROM) {
			if ((rp->flag & RED_SOCKET) != 0 && rp->iop != NULL) {
#ifdef HAVE_SOCKETS
				if ((rp->flag & RED_TCP) != 0)
					(void) shutdown(rp->iop->publ.fd, SHUT_RD);
#endif /* HAVE_SOCKETS */
				(void) iop_close(rp->iop);
			} else
				/* status already sanitized */
				status = gawk_pclose(rp);

			rp->iop = NULL;
		}
	} else if ((rp->flag & (RED_PIPE|RED_WRITE)) == (RED_PIPE|RED_WRITE)) {
		/* write to pipe */
		status = sanitize_exit_status(pclose((FILE*) rp->output.file));
		if ((BINMODE & BINMODE_INPUT) != 0)
			os_setbinmode(fileno(stdin), O_BINARY);

		rp->output.file = NULL;
	} else if (rp->output.file != NULL) {	/* write to file */
		status = rp->output.gawk_fclose(&rp->output);
		awk_output_buf_reset(&rp->output);
	} else if (rp->iop != NULL) {	/* read from pipe/file */
		if ((rp->flag & RED_PIPE) != 0)		/* read from pipe */
			status = gawk_pclose(rp);
			/* gawk_pclose sets rp->iop to null */
		else {					/* read from file */
			status = iop_close(rp->iop);
			rp->iop = NULL;
		}
	}

	return status;
}

/* close_redir --- close an open file or pipe */

static int
close_redir(struct redirect *rp, bool exitwarn, two_way_close_type how)
{
	int status = 0;

	if (rp == NULL)
		return 0;
#ifdef WINDOWS_NATIVE
	if (rp->output.socket_fd == INVALID_HANDLE)
#endif
	{
		if (rp->output.file == stdout || rp->output.file == stderr)
			goto checkwarn;		/* bypass closing, remove from list */
	}

	if (do_lint && (rp->flag & RED_TWOWAY) == 0 && how != CLOSE_ALL)
		lintwarn(_("close: redirection `%s' not opened with `|&', second argument ignored"),
				rp->value);

	status = close_rp(rp, how);

	if (status != 0) {
		int save_errno = errno;
		const char *s = strerror(save_errno);

		/*
		 * BWK's awk, as far back as SVR4 (1989) would check
		 * and warn about the status of close.  However, when
		 * we did this we got too many complaints, so we moved
		 * it to be under lint control.
		 */
		if (do_lint) {
			if ((rp->flag & RED_PIPE) != 0)
				lintwarn(_("failure status (%d) on pipe close of `%s': %s"),
					 status, rp->value, s);
			else
				lintwarn(_("failure status (%d) on file close of `%s': %s"),
					 status, rp->value, s);
		}

		if (! do_traditional) {
			/* set ERRNO too so that program can get at it */
			update_ERRNO_int(save_errno);
		}
	}

checkwarn:
	if (exitwarn) {
		/*
		 * Don't use lintwarn() here.  If lint warnings are fatal,
		 * doing so prevents us from closing other open redirections.
		 *
		 * Using multiple full messages instead of string parameters
		 * for the types makes message translation easier.
		 */
		if ((rp->flag & RED_SOCKET) != 0)
			awkwarn(_("no explicit close of socket `%s' provided"),
				rp->value);
		else if ((rp->flag & RED_TWOWAY) != 0)
			awkwarn(_("no explicit close of co-process `%s' provided"),
				rp->value);
		else if ((rp->flag & RED_PIPE) != 0)
			awkwarn(_("no explicit close of pipe `%s' provided"),
				rp->value);
		else
			awkwarn(_("no explicit close of file `%s' provided"),
				rp->value);
	}

	/* remove it from the list if closing both or both ends have been closed */
	if (how == CLOSE_ALL || (rp->iop == NULL && rp->output.file == NULL)) {
		if (rp->next != NULL)
			rp->next->prev = rp->prev;
		if (rp->prev != NULL)
			rp->prev->next = rp->next;
		else
			red_head = rp->next;
		free_rp(rp);
	}

	return status;
}

/* non_fatal_flush_std_file --- flush a standard output file allowing for nonfatal setting */

bool
non_fatal_flush_std_file(FILE *fp)
{
	int status = fflush(fp);

	if (status != 0) {
		bool is_fatal = ! is_non_fatal_std(fp);

		if (is_fatal) {
#ifdef WINDOWS_NATIVE
			if (errno == 0 || errno == EINVAL)
				w32_maybe_set_errno();
#endif
			if (errno == EPIPE)
				die_via_sigpipe();
			else
				fatal(fp == stdout
					? _("fflush: cannot flush standard output: %s")
					: _("fflush: cannot flush standard error: %s"),
						strerror(errno));
		} else {
			update_ERRNO_int(errno);
			awkwarn(fp == stdout
				? _("error writing standard output: %s")
				: _("error writing standard error: %s"),
					strerror(errno));
		}
		return false;
	}

	return true;
}

/* flush_io --- flush all open output files */

int
flush_io(void)
{
	struct redirect *rp;
	int status = 0;

	errno = 0;
	if (! non_fatal_flush_std_file(stdout))	// ERRNO updated
		status++;

	errno = 0;
	if (! non_fatal_flush_std_file(stderr))	// ERRNO updated
		status++;


	// now for all open redirections
	for (rp = red_head; rp != NULL; rp = rp->next) {
		ATTRIBUTE_PRINTF_PTR(mesg, 1, 2) void (*messagefunc)(const char *mesg, ...) = r_fatal;

		/* flush both files and pipes, what the heck */
		if ((rp->flag & RED_WRITE) != 0 && rp->output.file != NULL) {
			if (rp->output.gawk_fflush(&rp->output) != 0) {
				update_ERRNO_int(errno);

				if (is_non_fatal_redirect(rp->value, strlen(rp->value)))
					messagefunc = r_warning;

#if defined _MSC_VER && defined _PREFAST_
#define messagefunc printf
#endif
				if ((rp->flag & RED_PIPE) != 0)
					messagefunc(_("pipe flush of `%s' failed: %s"),
						rp->value, strerror(errno));
				else if ((rp->flag & RED_TWOWAY) != 0)
					messagefunc(_("co-process flush of pipe to `%s' failed: %s"),
						rp->value, strerror(errno));
				else
					messagefunc(_("file flush of `%s' failed: %s"),
						rp->value, strerror(errno));
#if defined _MSC_VER && defined _PREFAST_
#undef messagefunc
#endif
				status++;
			}
		}
	}
	if (status != 0)
		status = -1;	/* canonicalize it */
	return status;
}

/* close_io --- close all open files, called when exiting */

int
close_io(bool *stdio_problem, bool *got_EPIPE)
{
	struct redirect *rp;
	struct redirect *next;
	int status = 0;

	*stdio_problem = *got_EPIPE = false;
	errno = 0;
	for (rp = red_head; rp != NULL; rp = next) {
		next = rp->next;
		/*
		 * close_redir() will print a message if needed.
		 * if do_lint, warn about lack of explicit close
		 */
		if (close_redir(rp, do_lint, CLOSE_ALL))
			status++;
		rp = NULL;
	}
	/*
	 * Some of the non-Unix os's have problems doing an fclose()
	 * on stdout and stderr.  Since we don't really need to close
	 * them, we just flush them, and do that across the board.
	 */
	*stdio_problem = false;
	/* we don't warn about stdout/stderr if EPIPE, but we do error exit */
	if (fflush(stdout) != 0) {
#ifdef WINDOWS_NATIVE
		if (errno == 0 || errno == EINVAL)
			w32_maybe_set_errno();
#endif
		if (errno != EPIPE)
			awkwarn(_("error writing standard output: %s"), strerror(errno));
		else
			*got_EPIPE = true;

		status++;
		*stdio_problem = true;
	}
	if (fflush(stderr) != 0) {
#ifdef WINDOWS_NATIVE
		if (errno == 0 || errno == EINVAL)
			w32_maybe_set_errno();
#endif
		if (errno != EPIPE)
			awkwarn(_("error writing standard error: %s"), strerror(errno));
		else
			*got_EPIPE = true;

		status++;
		*stdio_problem = true;
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

/* Socket descriptor */
typedef int socket_t;

/* socketopen --- open a socket and set it into connected state */

static socket_t
socketopen(int family, int type, const char *localpname,
	const char *remotepname, const char *remotehostname, bool *hard_error)
{
	struct addrinfo *lres, *lres0;
	struct addrinfo lhints;
	struct addrinfo *rres, *rres0;
	struct addrinfo rhints;

	int lerror, rerror;

	int socket_fd;
	bool any_remote_host = (strcmp(remotehostname, "0") == 0);

#ifdef HAVE_GAI_STRERROR_BUF
	char gai_buf[1024];
#define gai_strerror_(e) gai_strerror_buf(e, gai_buf, sizeof(gai_buf))
#else
#define gai_strerror_(e) gai_strerror(e)
#endif

	memset(& lhints, '\0', sizeof (lhints));

	lhints.ai_socktype = type;
	lhints.ai_family = family;

	/*
	 * If only the loopback interface is up and hints.ai_flags has
	 * AI_ADDRCONFIG, getaddrinfo() will succeed and return all wildcard
	 * addresses, but only if hints.ai_family == AF_UNSPEC
	 *
	 * Do return the wildcard address in case the loopback interface
	 * is the only one that is up (and
	 * hints.ai_family == either AF_INET4 or AF_INET6)
	 */
	lhints.ai_flags = AI_PASSIVE;
	if (lhints.ai_family == AF_UNSPEC)
		lhints.ai_flags |= AI_ADDRCONFIG;

	lerror = getaddrinfo(NULL, localpname, & lhints, & lres);
	if (lerror) {
		if (strcmp(localpname, "0") != 0) {
#if defined HAVE_GAI_STRERROR || defined HAVE_GAI_STRERROR_BUF
			awkwarn(_("local port %s invalid in `/inet': %s"), localpname,
					gai_strerror_(lerror));
#else
			awkwarn(_("local port %s invalid in `/inet'"), localpname);
#endif
			*hard_error = true;
			return INVALID_HANDLE;
		}
		lres0 = NULL;
		lres = & lhints;
	} else
		lres0 = lres;

	do {
		memset (& rhints, '\0', sizeof (rhints));
		rhints.ai_flags = lhints.ai_flags;
		rhints.ai_socktype = lhints.ai_socktype;
		rhints.ai_family = lhints.ai_family;
		rhints.ai_protocol = lhints.ai_protocol;

		rerror = getaddrinfo(any_remote_host ? NULL : remotehostname,
				remotepname, & rhints, & rres);
		if (rerror) {
			if (lres0 != NULL)
				freeaddrinfo(lres0);
#if defined HAVE_GAI_STRERROR || defined HAVE_GAI_STRERROR_BUF
			awkwarn(_("remote host and port information (%s, %s) invalid: %s"), remotehostname, remotepname,
					gai_strerror_(rerror));
#else
			awkwarn(_("remote host and port information (%s, %s) invalid"), remotehostname, remotepname);
#endif
			*hard_error = true;
			return INVALID_HANDLE;
		}
		rres0 = rres;
		do {
			socket_fd = socket(rres->ai_family,
				rres->ai_socktype, rres->ai_protocol);
			if (socket_fd < 0 || socket_fd == INVALID_HANDLE)
				goto nextrres;

			if (type == SOCK_STREAM) {
				int on = 1;
#ifdef SO_LINGER
				struct linger linger;
				memset(& linger, '\0', sizeof(linger));
#endif
				setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
					(char *) & on, sizeof(on));
#ifdef SO_LINGER
				linger.l_onoff = 1;
				/* linger for 30/100 second */
				linger.l_linger = 30;
				setsockopt(socket_fd, SOL_SOCKET, SO_LINGER,
					(char *) & linger, sizeof(linger));
#endif
			}
			if (bind(socket_fd, lres->ai_addr, (int)lres->ai_addrlen) != 0)
				goto nextrres;

			if (! any_remote_host) { /* not ANY => create a client */
				if (connect(socket_fd, rres->ai_addr, (int)rres->ai_addrlen) == 0)
					break;
			} else { /* remote host is ANY => create a server */
				struct sockaddr_storage remote_addr;
				socklen_t addrlen = sizeof(remote_addr);
				void *const addr = &remote_addr;

				if (type == SOCK_STREAM) {
					socket_t clientsocket_fd = INVALID_HANDLE;

					if (listen(socket_fd, 1) >= 0
					    && (clientsocket_fd = accept(socket_fd,
						(struct sockaddr *) addr,
						& addrlen)) >= 0) {
						closemaybesocket(socket_fd);
						socket_fd = clientsocket_fd;
						break;
					}
				} else if (type == SOCK_DGRAM) {
#ifdef MSG_PEEK
					char buf[10];
					if (recvfrom(socket_fd, buf, 1, MSG_PEEK,
						(struct sockaddr *) addr,
							& addrlen) >= 0
							&& addrlen
							&& connect(socket_fd,
						(struct sockaddr *) addr,
								addrlen) == 0)
							break;
#endif
				}
			}

nextrres:
			if (socket_fd != INVALID_HANDLE) {
				closemaybesocket(socket_fd);
				socket_fd = INVALID_HANDLE;
			}
		} while ((rres = rres->ai_next) != NULL);
		freeaddrinfo(rres0);
		if (socket_fd != INVALID_HANDLE)
			break;
	} while ((lres = lres->ai_next) != NULL);
	if (lres0 != NULL)
		freeaddrinfo(lres0);

	return socket_fd;
}
#endif /* HAVE_SOCKETS */


/* devopen_simple --- handle "-", /dev/std{in,out,err}, /dev/fd/N */

/*
 * 9/2014: Flow here is a little messy.
 *
 * For do_posix, we don't allow any of the special filenames.
 *
 * For do_traditional, we allow /dev/{stdin,stdout,stderr} since BWK awk
 * (and mawk) support them.  But we don't allow /dev/fd/N or /inet.
 *
 * Note that for POSIX systems os_devopen() is a no-op.
 */

fd_t
devopen_simple(const char *name, const char *mode, bool try_real_open)
{
	fd_t openfd;
	const char *cp;
	char *ptr;
	int flag = 0;

	if (strcmp(name, "-") == 0) {
		if (mode[0] == 'r')
			return fileno(stdin);
		else
			return fileno(stdout);
	}

	flag = str2mode(mode);
	openfd = INVALID_HANDLE;

	if (do_posix)
		goto done;

	if ((openfd = os_devopen(name, flag)) != INVALID_HANDLE) {
		os_close_on_exec(openfd, name, "file", "");
		return openfd;
	}

	if (strncmp(name, "/dev/", 5) == 0) {
		cp = name + 5;

		if (strcmp(cp, "stdin") == 0 && (flag & O_ACCMODE) == O_RDONLY)
			openfd = fileno(stdin);
		else if (strcmp(cp, "stdout") == 0 && (flag & O_ACCMODE) == O_WRONLY)
			openfd = fileno(stdout);
		else if (strcmp(cp, "stderr") == 0 && (flag & O_ACCMODE) == O_WRONLY)
			openfd = fileno(stderr);
		else if (do_traditional)
			goto done;
		else if (strncmp(cp, "fd/", 3) == 0) {
			gawk_stat_t sbuf;

			cp += 3;
			openfd = (fd_t) strtoul(cp, & ptr, 10);
			if (openfd <= INVALID_HANDLE || ptr == cp
			    || os_fstat(openfd, & sbuf) < 0)
				openfd = INVALID_HANDLE;
		}
		/* do not set close-on-exec for inherited fd's */
	}
done:
	if (try_real_open)
		openfd = open(name, flag, 0666);

	return openfd;
}

/* devopen --- handle /dev/std{in,out,err}, /dev/fd/N, /inet, regular files */

/*
 * Strictly speaking, "name" is not a "const char *" because we temporarily
 * change the string.
 */

fd_t
devopen(const char *name, const char *mode)
{
	fd_t openfd;
	int flag;
	struct inet_socket_info isi;
	int save_errno = 0;

	openfd = devopen_simple(name, mode, false);
	if (openfd != INVALID_HANDLE)
		return openfd;

	flag = str2mode(mode);

	if (do_traditional) {
		goto strictopen;
	} else if (inetfile(name, strlen(name), & isi)) {
#ifdef HAVE_SOCKETS
#define DEFAULT_RETRIES 20
		static unsigned long def_retries = DEFAULT_RETRIES;
		static bool first_time = true;
		/* Default is 1 millisecond.  */
		static unsigned long msleep = 1;
		unsigned long retries;
		bool hard_error = false;
		bool non_fatal = is_non_fatal_redirect(name, strlen(name));
		char save;
		char *cp = charp_const_cast(name);

		/* socketopen requires NUL-terminated strings */
		cp[isi.localport.offset+isi.localport.len] = '\0';
		cp[isi.remotehost.offset+isi.remotehost.len] = '\0';
		save = cp[isi.remoteport.offset+isi.remoteport.len];
		cp[isi.remoteport.offset+isi.remoteport.len] = '\0';

		if (first_time) {
			char *end;
			const char *cp2, *ms2;

			first_time = false;
			if ((cp2 = getenv("GAWK_SOCK_RETRIES")) != NULL) {
				const long count = strtol(cp2, & end, 10);
				if (end != cp2 && count > 0)
					def_retries = (unsigned long) count;
			}

			/* Env var is in milliseconds.  */
			if ((ms2 = getenv("GAWK_MSEC_SLEEP")) != NULL) {
				const long m = strtol(ms2, & end, 10);
				if (end != ms2 && m >= 0)
					msleep = (unsigned long) m;
			}
		}
		/*
		 * PROCINFO["NONFATAL"] or PROCINFO[name, "NONFATAL"] overrrides
		 * GAWK_SOCK_RETRIES.  The explicit code in the program carries
		 * a bigger stick than the environment variable does.
		 */
		retries = non_fatal ? 1 : def_retries;

		errno = 0;
		do {
			openfd = socketopen(isi.family, isi.protocol, name+isi.localport.offset,
					name+isi.remoteport.offset, name+isi.remotehost.offset,
					& hard_error);
			retries--;
		} while (openfd == INVALID_HANDLE && ! hard_error && retries > 0 && os_sleep(msleep) == 0);
		save_errno = errno;

		/* restore original name string */
		cp[isi.localport.offset+isi.localport.len] = '/';
		cp[isi.remotehost.offset+isi.remotehost.len] = '/';
		cp[isi.remoteport.offset+isi.remoteport.len] = save;
#else /* ! HAVE_SOCKETS */
		fatal(_("TCP/IP communications are not supported"));
#endif /* HAVE_SOCKETS */
	}

strictopen:
	if (openfd == INVALID_HANDLE) {
		openfd = open(name, flag, 0666);
		/*
		 * ENOENT means there is no such name in the filesystem.
		 * Therefore it's ok to propagate up the error from
		 * getaddrinfo() that's in save_errno.
		 */
		if (openfd == INVALID_HANDLE && errno == ENOENT && save_errno)
			errno = save_errno;
	}
#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	if (openfd == INVALID_HANDLE && errno == EACCES) {
		/* On OS/2 and Windows directory access via open() is
		   not permitted.  */
		gawk_stat_t buf;

		if (! inetfile(name, strlen(name), NULL)
		    && os_stat(name, & buf) == 0 && S_ISDIR(buf.st_mode))
			errno = EISDIR;
	}
#endif
	if (openfd != INVALID_HANDLE) {
		if (openfd > fileno(stderr))
			os_close_on_exec(openfd, name, "file", "");
	}

	return openfd;
}

#if defined(HAVE_TERMIOS_H)
/* push_pty_line_disciplines --- push line disciplines if we work that way */

// Factors out common code for the two versions of fork_and_open_slave_pty().

static void
push_pty_line_disciplines(fd_t slave)
{
#ifdef I_PUSH
	/*
	 * Push the necessary modules onto the slave to
	 * get terminal semantics.  Check that they aren't
	 * already there to avoid hangs on said "limited" systems.
	 */
#ifdef I_FIND
	if (ioctl(slave, I_FIND, "ptem") == 0)
#endif
		ioctl(slave, I_PUSH, "ptem");
#ifdef I_FIND
	if (ioctl(slave, I_FIND, "ldterm") == 0)
#endif
		ioctl(slave, I_PUSH, "ldterm");
#endif
}

/* set_slave_pty_attributes --- set terminal attributes for slave pty */

// Factors out common code for the two versions of fork_and_open_slave_pty().

static void
set_slave_pty_attributes(fd_t slave)
{
	struct termios st;

	tcgetattr(slave, & st);
	st.c_iflag &= ~(ISTRIP | IGNCR | INLCR | IXOFF);
	st.c_iflag |= (ICRNL | IGNPAR | BRKINT | IXON);
	st.c_oflag &= ~OPOST;
	st.c_cflag &= ~CSIZE;
	st.c_cflag |= CREAD | CS8 | CLOCAL;
	st.c_lflag &= ~(ECHO | ECHOE | ECHOK | NOFLSH | TOSTOP);
	st.c_lflag |= ISIG;

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
	tcsetattr(slave, TCSANOW, & st);
}


/* fork_and_open_slave_pty --- handle forking the child and slave pty setup */

/*
 * January, 2018:
 * This is messy. AIX and HP-UX require that the slave pty be opened and
 * set up in the child.  Everything else wants it to be done in the parent,
 * before the fork.  Thus we have two different versions of the routine that
 * do the same thing, but in different orders.  This is not pretty, but it
 * seems to be the simplest thing to do.
 */

static bool
fork_and_open_slave_pty(const char *slavenam, fd_t master, const char *command, pid_t *pid)
{
	fd_t slave;
	int save_errno;

#if defined _AIX || defined __hpux
	/*
	 * We specifically open the slave only in the child. This allows
	 * AIX and HP0UX to work.  The open is specifically without O_NOCTTY
	 * in order to make the slave become the controlling terminal.
	 */

	switch (*pid = fork()) {
	case 0:
		/* Child process */
		setsid();

		if ((slave = open(slavenam, O_RDWR)) < 0) {
			close(master);
			fatal(_("could not open `%s', mode `%s'"),
				slavenam, "r+");
		}

		push_pty_line_disciplines(slave);
		set_slave_pty_attributes(slave);

		if (close(master) == -1)
			fatal(_("close of master pty failed: %s"), strerror(errno));
		if (close(1) == -1)
			fatal(_("close of stdout in child failed: %s"),
				strerror(errno));
		if (dup(slave) != 1)
			fatal(_("moving slave pty to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(0) == -1)
			fatal(_("close of stdin in child failed: %s"),
				strerror(errno));
		if (dup(slave) != 0)
			fatal(_("moving slave pty to stdin in child failed (dup: %s)"), strerror(errno));
		if (close(slave))
			fatal(_("close of slave pty failed: %s"), strerror(errno));

		/* stderr does NOT get dup'ed onto child's stdout */

		set_sigpipe_to_default();

		execl("/bin/sh", "sh", "-c", command, NULL);
		_exit(errno == ENOENT ? 127 : 126);

	case BAD_PID:
		save_errno = errno;
		close(master);
		errno = save_errno;
		return false;

	default:
		return true;
	}

#else

	if ((slave = open(slavenam, O_RDWR)) < 0) {
		close(master);
		fatal(_("could not open `%s', mode `%s'"),
			slavenam, "r+");
	}

	push_pty_line_disciplines(slave);
	set_slave_pty_attributes(slave);

	switch (*pid = fork()) {
	case 0:
		/* Child process */
		setsid();

#ifdef TIOCSCTTY
		ioctl(slave, TIOCSCTTY, 0);
#endif

		if (close(master) == -1)
			fatal(_("close of master pty failed: %s"), strerror(errno));
		if (close(1) == -1)
			fatal(_("close of stdout in child failed: %s"),
				strerror(errno));
		if (dup(slave) != 1)
			fatal(_("moving slave pty to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(0) == -1)
			fatal(_("close of stdin in child failed: %s"),
				strerror(errno));
		if (dup(slave) != 0)
			fatal(_("moving slave pty to stdin in child failed (dup: %s)"), strerror(errno));
		if (close(slave))
			fatal(_("close of slave pty failed: %s"), strerror(errno));

		/* stderr does NOT get dup'ed onto child's stdout */

		signal(SIGPIPE, SIG_DFL);

		execl("/bin/sh", "sh", "-c", command, NULL);
		_exit(errno == ENOENT ? 127 : 126);

	case BAD_PID:
		save_errno = errno;
		close(master);
		close(slave);
		errno = save_errno;
		return false;

	}

	/* parent */
	if (close(slave) != 0) {
		close(master);
		(void) kill(*pid, SIGKILL);
		fatal(_("close of slave pty failed: %s"), strerror(errno));
	}

	return true;
#endif
}

#endif /* defined(HAVE_TERMIOS_H) */

/* spawn_in_parallel --- spawn a command in sub-shell in parallel */

static pid_t
spawn_in_parallel(const char *cmd)
{
	pid_t pid;

#ifdef __EMX__
	pid = spawnl(P_NOWAIT, "/bin/sh", "sh", "-c", cmd, NULL);
#else  /* WINDOWS_NATIVE */
	wchar_t buf[512];
	wchar_t *qcmd = wquote_cmd(cmd, buf, sizeof(buf)/sizeof(buf[0]));
	if (qcmd == NULL)
		return BAD_PID;
	pid = _wspawnl(_P_NOWAIT, _wgetenv(L"ComSpec"), L"cmd.exe", L"/D /S /C", qcmd, NULL);
	if (qcmd != buf)
		free(qcmd);
#endif

	return pid;
}

/* two_way_open --- open a two way communications channel */

static int
two_way_open(const char *str, struct redirect *rp, fd_t extfd)
{
#ifdef HAVE_TERMIOS_H
	static bool no_ptys = false;
#endif

#ifdef HAVE_SOCKETS
	/* case 1: socket */
	if (extfd >= 0 || inetfile(str, strlen(str), NULL)) {
		fd_t fd, newfd;

		fd = (extfd >= 0) ? extfd : devopen(str, "rw");
		if (fd == INVALID_HANDLE)
			return false;
		if ((BINMODE & BINMODE_OUTPUT) != 0)
			os_setbinmode(fd, O_BINARY);
#ifdef WINDOWS_NATIVE
		rp->output.file = socket_file_alloc();
		if (rp->output.file != NULL)
			rp->output.socket_fd = fd;
#else
		rp->output.file = fdopen(fd, binmode("wb"));
#endif
		if (rp->output.file == NULL) {
			closemaybesocket(fd);
			return false;
		}
		newfd = socket_dup(fd);
		if (newfd < 0) {
			rp->output.gawk_fclose(&rp->output);
			return false;
		}
		if ((BINMODE & BINMODE_INPUT) != 0)
			os_setbinmode(newfd, O_BINARY);
		os_close_on_exec(fd, str, "socket", "to/from");
		os_close_on_exec(newfd, str, "socket", "to/from");
		rp->iop = iop_alloc(newfd, str, 0);
		rp->output.name = str;
		find_input_parser(rp->iop);
		iop_finish(rp->iop);
		if (! rp->iop->valid) {
			if (! do_traditional && rp->iop->errcode != 0)
				update_ERRNO_int(rp->iop->errcode);
			(void) iop_close(rp->iop);
			rp->iop = NULL;
			rp->output.gawk_fclose(&rp->output);
			return false;
		}
		rp->flag |= RED_SOCKET;
		return true;
	}
#endif /* HAVE_SOCKETS */

	/* case 2: see if an extension wants it */
	if (find_two_way_processor(str, rp))
		return true;

#ifdef HAVE_TERMIOS_H
	/* case 3: use ptys for two-way communications to child */
	if (! no_ptys && pty_vs_pipe(str)) {
		static bool initialized = false;
		static char first_pty_letter;
#if defined(HAVE_GRANTPT) && ! defined(HAVE_POSIX_OPENPT)
		static int have_dev_ptmx;
#endif
		char slavenam[32];
		char c;
		fd_t master, dup_master;
		pid_t pid;
		gawk_stat_t statb;
		/* Use array of chars to avoid ASCII / EBCDIC issues */
		static const char pty_chars[] = "pqrstuvwxyzabcdefghijklmno";

		if (! initialized) {
			unsigned i;
			initialized = true;
#if defined(HAVE_GRANTPT) && ! defined(HAVE_POSIX_OPENPT)
			have_dev_ptmx = (os_stat("/dev/ptmx", & statb) >= 0);
#endif
			i = 0;
			do {
				c = pty_chars[i++];
				sprintf(slavenam, "/dev/pty%c0", c);
				if (os_stat(slavenam, & statb) >= 0) {
					first_pty_letter = c;
					break;
				}
			} while (pty_chars[i] != '\0');
		}

#ifdef HAVE_GRANTPT
#ifdef HAVE_POSIX_OPENPT
		{
			master = posix_openpt(O_RDWR|O_NOCTTY);
#else
		if (have_dev_ptmx) {
			master = open("/dev/ptmx", O_RDWR);
#endif
			if (master >= 0) {
				const char *tem;

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
			 * The FIRST_PTY_LETTER gives the first X to try.
			 * We try in the sequence FIRST_PTY_LETTER, ..,
			 * 'z', 'a', .., FIRST_PTY_LETTER.
			 * Is this worthwhile, or just over-zealous?
			 */
			c = first_pty_letter;
			do {
				unsigned i;
				const char *cp;

				for (i = 0; i < 16; i++) {
					sprintf(slavenam, "/dev/pty%c%x", c, i);
					if (os_stat(slavenam, & statb) < 0) {
						no_ptys = true;	/* bypass all this next time */
						goto use_pipes;
					}

					if ((master = open(slavenam, O_RDWR)) >= 0) {
						slavenam[sizeof("/dev/") - 1] = 't';
						if (access(slavenam, R_OK | W_OK) == 0)
							goto got_the_pty;
						close(master);
					}
				}
				/* move to next character */
				cp = strchr(pty_chars, c);
				if (cp[1] != '\0')
					cp++;
				else
					cp = pty_chars;
				c = *cp;
			} while (c != first_pty_letter);
		} else
			no_ptys = true;

		/* Couldn't find a pty. Fall back to using pipes. */
		goto use_pipes;

	got_the_pty:

		/* this is the parent */
		if (! fork_and_open_slave_pty(slavenam, master, str, & pid))
			fatal(_("could not create child process or open pty"));

		rp->pid = pid;
		rp->iop = iop_alloc(master, str, 0);
		find_input_parser(rp->iop);
		iop_finish(rp->iop);
		if (! rp->iop->valid) {
			if (! do_traditional && rp->iop->errcode != 0)
				update_ERRNO_int(rp->iop->errcode);
			(void) iop_close(rp->iop);
			rp->iop = NULL;
			(void) kill(pid, SIGKILL);
			return false;
		}

		rp->output.name = str;
		/*
		 * Force read and write ends of two-way connection to
		 * be different fd's so they can be closed independently.
		 */
		rp->output.mode = "w";
		if ((dup_master = dup(master)) < 0
		    || (rp->output.fp = fdopen(dup_master, "w")) == NULL) {
			(void) iop_close(rp->iop);
			rp->iop = NULL;
			(void) close(master);
			(void) kill(pid, SIGKILL);
			if (dup_master >= 0)
				(void) close(dup_master);
			return false;
		} else
			find_output_wrapper(& rp->output);
		rp->flag |= RED_PTY;
		os_close_on_exec(master, str, "pipe", "from");
		os_close_on_exec(dup_master, str, "pipe", "to");
		first_pty_letter = '\0';	/* reset for next command */
		return true;
	}
use_pipes:
#endif /* HAVE_TERMIOS_H */

#ifndef PIPES_SIMULATED		/* real pipes */
	/* case 4: two way pipe to a child process */
    {
	fd_t ptoc[2], ctop[2];
	pid_t pid;
	int save_errno;
#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	fd_t save_stdout, save_stdin;
#endif

	if (pipe(ptoc) < 0)
		return false;	/* errno set, diagnostic from caller */

	if (pipe(ctop) < 0) {
		save_errno = errno;
		close(ptoc[0]);
		close(ptoc[1]);
		errno = save_errno;
		return false;
	}

#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	save_stdin = dup(0);	/* duplicate stdin */
	save_stdout = dup(1);	/* duplicate stdout */

	if (save_stdout == INVALID_HANDLE || save_stdin == INVALID_HANDLE) {
		/* if an error occurs close all open file handles */
		save_errno = errno;
		if (save_stdin != INVALID_HANDLE)
			close(save_stdin);
		if (save_stdout != INVALID_HANDLE)
			close(save_stdout);
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		errno = save_errno;
		return false;
	}

	/* connect pipes to stdin and stdout */
	close(1);	/* close stdout */
	if (dup(ctop[1]) != 1) {	/* connect pipe input to stdout */
		close(save_stdin); close(save_stdout);
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
	}
	close(0);	/* close stdin */
	if (dup(ptoc[0]) != 0) {	/* connect pipe output to stdin */
		close(save_stdin); close(save_stdout);
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		fatal(_("moving pipe to stdin in child failed (dup: %s)"), strerror(errno));
	}

	/* none of these handles must be inherited by the child process */
	(void) close(ptoc[0]);	/* close pipe output, child will use stdin instead */
	(void) close(ctop[1]);	/* close pipe input, child will use stdout instead */

	os_close_on_exec(ptoc[1], str, "pipe", "from"); /* pipe input: output of the parent process */
	os_close_on_exec(ctop[0], str, "pipe", "from"); /* pipe output: input of the parent process */
	os_close_on_exec(save_stdin, str, "pipe", "from"); /* saved stdin of the parent process */
	os_close_on_exec(save_stdout, str, "pipe", "from"); /* saved stdout of the parent process */

	/* stderr does NOT get dup'ed onto child's stdout */
	pid = spawn_in_parallel(str);

	/* restore stdin and stdout */
	close(1);
	if (dup(save_stdout) != 1) {
		close(save_stdin); close(save_stdout);
		close(ptoc[1]); close(ctop[0]);
		fatal(_("restoring stdout in parent process failed"));
	}
	close(save_stdout);

	close(0);
	if (dup(save_stdin) != 0) {
		close(save_stdin);
		close(ptoc[1]);	close(ctop[0]);
		fatal(_("restoring stdin in parent process failed"));
	}
	close(save_stdin);

	if (pid < 0) { /* spawnl() failed */
		save_errno = errno;
		close(ptoc[1]);
		close(ctop[0]);

		errno = save_errno;
		return false;
	}

#else /* NOT __EMX__, NOT WINDOWS_NATIVE */
	if ((pid = fork()) < 0) {
		save_errno = errno;
		close(ptoc[0]); close(ptoc[1]);
		close(ctop[0]); close(ctop[1]);
		errno = save_errno;
		return false;
	}

	if (pid == 0) {	/* child */
		if (close(1) == -1)
			fatal(_("close of stdout in child failed: %s"),
				strerror(errno));
		if (dup(ctop[1]) != 1)
			fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(0) == -1)
			fatal(_("close of stdin in child failed: %s"),
				strerror(errno));
		if (dup(ptoc[0]) != 0)
			fatal(_("moving pipe to stdin in child failed (dup: %s)"), strerror(errno));
		if (   close(ptoc[0]) == -1 || close(ptoc[1]) == -1
		    || close(ctop[0]) == -1 || close(ctop[1]) == -1)
			fatal(_("close of pipe failed: %s"), strerror(errno));
		/* stderr does NOT get dup'ed onto child's stdout */
		set_sigpipe_to_default();
		execl("/bin/sh", "sh", "-c", str, NULL);
		_exit(errno == ENOENT ? 127 : 126);
	}
#endif /* NOT __EMX__, NOT WINDOWS_NATIVE */

	/* parent */
	if ((BINMODE & BINMODE_INPUT) != 0)
		os_setbinmode(ctop[0], O_BINARY);
	if ((BINMODE & BINMODE_OUTPUT) != 0)
		os_setbinmode(ptoc[1], O_BINARY);
	rp->pid = pid;
	rp->iop = iop_alloc(ctop[0], str, 0);
	find_input_parser(rp->iop);
	iop_finish(rp->iop);
	if (! rp->iop->valid) {
		if (! do_traditional && rp->iop->errcode != 0)
			update_ERRNO_int(rp->iop->errcode);
		(void) iop_close(rp->iop);
		rp->iop = NULL;
		(void) close(ctop[1]);
		(void) close(ptoc[0]);
		(void) close(ptoc[1]);
		(void) kill(pid, SIGKILL);

		return false;
	}
	rp->output.file = fdopen(ptoc[1], binmode("w"));
	rp->output.mode = "w";
	rp->output.name = str;
	if (rp->output.file == NULL) {
		(void) iop_close(rp->iop);
		rp->iop = NULL;
		(void) close(ctop[0]);
		(void) close(ctop[1]);
		(void) close(ptoc[0]);
		(void) close(ptoc[1]);
		(void) kill(pid, SIGKILL);

		return false;
	}
	else
		find_output_wrapper(& rp->output);

#if !defined(__EMX__) && !defined(WINDOWS_NATIVE)
	os_close_on_exec(ctop[0], str, "pipe", "from");
	os_close_on_exec(ptoc[1], str, "pipe", "from");

	(void) close(ptoc[0]);
	(void) close(ctop[1]);
#endif

	return true;
    }

#else	/*PIPES_SIMULATED*/

	fatal(_("`|&' not supported"));
	/*NOTREACHED*/
	return false;

#endif
}

#ifndef PIPES_SIMULATED		/* real pipes */

/*
 * wait_any --- if the argument pid is 0, wait for all child processes that
 * have exited.  We loop to make sure to reap all children that have exited to
 * minimize the risk of running out of process slots.  Since we don't process
 * SIGCHLD, we do not immediately reap exited children.  So when we get here,
 * we want to reap any that have piled up.
 *
 * Note: on platforms that do not support waitpid with WNOHANG, when called with
 * a zero argument, this function will hang until all children have exited.
 *
 * AJS, 2013-07-07: I do not see why we need to ignore signals during this
 * function.  This function just waits and updates the pid and status fields.
 * I don't see why that should interfere with any signal handlers.  But I am
 * reluctant to remove this protection.  So I changed to use sigprocmask to
 * block signals instead to avoid interfering with installed signal handlers.
 */

static int
wait_any(pid_t interesting)	/* pid of interest, if any */
{
	pid_t pid;
	int status = 0;
	struct redirect *redp;
#ifdef HAVE_SIGPROCMASK
	sigset_t set, oldset;

	/* I have no idea why we are blocking signals during this function... */
	sigemptyset(& set);
	sigaddset(& set, SIGINT);
	sigaddset(& set, SIGHUP);
	sigaddset(& set, SIGQUIT);
	sigprocmask(SIG_BLOCK, & set, & oldset);
#else
	void (*hstat)(int), (*istat)(int), (*qstat)(int);

	istat = signal(SIGINT, SIG_IGN);
#endif
#ifdef WINDOWS_NATIVE
	if (interesting < 0) {
		status = -1;
		pid = BAD_PID;
	}
	else {
		pid = _cwait(& status, interesting, 0);
	}
	if (pid == interesting && pid > 0) {
		for (redp = red_head; redp != NULL; redp = redp->next)
			if (interesting == redp->pid) {
				redp->pid = BAD_PID;
				redp->status = status;
				break;
			}
	}
	(void)hstat, (void)qstat;
#else /* ! WINDOWS_NATIVE */
#ifndef HAVE_SIGPROCMASK
	hstat = signal(SIGHUP, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
#endif
	for (;;) {
# if defined(HAVE_WAITPID) && defined(WNOHANG)
		/*
		 * N.B. If the caller wants status for a specific child process
		 * (i.e. interesting is non-zero), then we must hang until we
		 * get exit status for that child.
		 */
		if ((pid = waitpid(BAD_PID, & status, (interesting ? 0 : WNOHANG))) == 0)
			/* No children have exited */
			break;
# elif defined(HAVE_SYS_WAIT_H)	/* POSIX compatible sys/wait.h */
		pid = wait(& status);
# else
		pid = wait((union wait *) & status);
# endif
		if (interesting && pid == interesting) {
			break;
		} else if (pid != BAD_PID) {
			for (redp = red_head; redp != NULL; redp = redp->next)
				if (pid == redp->pid) {
					redp->pid = BAD_PID;
					redp->status = status;
					break;
				}
		}
		if (pid == BAD_PID && errno == ECHILD)
			break;
	}
#ifndef HAVE_SIGPROCMASK
	signal(SIGHUP, hstat);
	signal(SIGQUIT, qstat);
#endif
#endif /* ! WINDOWS_NATIVE */
#ifndef HAVE_SIGPROCMASK
	signal(SIGINT, istat);
#else
	sigprocmask(SIG_SETMASK, & oldset, NULL);
#endif
	return status;
}

/* gawk_popen --- open an IOBUF on a child process */

static IOBUF *
gawk_popen(const char *cmd, struct redirect *rp)
{
	fd_t p[2];
	pid_t pid;
#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	fd_t save_stdout;
#endif

	/*
	 * We used to wait for any children to synchronize input and output,
	 * but this could cause gawk to hang when it is started in a pipeline
	 * and thus has a child process feeding it input (shell dependent).
	 *
	 * (void) wait_any(0);	// wait for outstanding processes
	 */

	if (pipe(p) < 0)
		fatal(_("cannot open pipe `%s': %s"), cmd, strerror(errno));

#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	rp->iop = NULL;
	save_stdout = dup(1); /* save stdout */
	if (save_stdout == INVALID_HANDLE) {
		close(p[0]);
		close(p[1]);
		return NULL;	/* failed */
	}

	close(1); /* close stdout */
	if (dup(p[1]) != 1) {
		close(p[0]);
		close(p[1]);
		fatal(_("moving pipe to stdout in child failed (dup: %s)"),
				strerror(errno));
	}

	/* none of these handles must be inherited by the child process */
	close(p[1]); /* close pipe input */

	os_close_on_exec(p[0], cmd, "pipe", "from"); /* pipe output: input of the parent process */
	os_close_on_exec(save_stdout, cmd, "pipe", "from"); /* saved stdout of the parent process */

	pid = spawn_in_parallel(cmd);

	/* restore stdout */
	close(1);
	if (dup(save_stdout) != 1) {
		close(p[0]);
		fatal(_("restoring stdout in parent process failed"));
	}
	close(save_stdout);

#else /* NOT __EMX__, NOT WINDOWS_NATIVE */
	if ((pid = fork()) == 0) {
		if (close(1) == -1)
			fatal(_("close of stdout in child failed: %s"),
				strerror(errno));
		if (dup(p[1]) != 1)
			fatal(_("moving pipe to stdout in child failed (dup: %s)"), strerror(errno));
		if (close(p[0]) == -1 || close(p[1]) == -1)
			fatal(_("close of pipe failed: %s"), strerror(errno));
		set_sigpipe_to_default();
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		_exit(errno == ENOENT ? 127 : 126);
	}
#endif /* NOT __EMX__, NOT WINDOWS_NATIVE */

	if (pid == BAD_PID) {
		close(p[0]); close(p[1]);
		fatal(_("cannot create child process for `%s' (fork: %s)"), cmd, strerror(errno));
	}
	rp->pid = pid;
#if !defined(__EMX__) && !defined(WINDOWS_NATIVE)
	if (close(p[1]) == -1) {
		close(p[0]);
		fatal(_("close of pipe failed: %s"), strerror(errno));
	}
	os_close_on_exec(p[0], cmd, "pipe", "from");
#endif
	if ((BINMODE & BINMODE_INPUT) != 0)
		os_setbinmode(p[0], O_BINARY);
	rp->iop = iop_alloc(p[0], cmd, 0);
	find_input_parser(rp->iop);
	iop_finish(rp->iop);
	if (! rp->iop->valid) {
		if (! do_traditional && rp->iop->errcode != 0)
			update_ERRNO_int(rp->iop->errcode);
		(void) iop_close(rp->iop);
		rp->iop = NULL;
	}

	return rp->iop;
}

/* gawk_pclose --- close an open child pipe */

static int
gawk_pclose(struct redirect *rp)
{
	if (rp->iop != NULL)
		(void) iop_close(rp->iop);
	rp->iop = NULL;

	/* process previously found, return stored status */
	if (rp->pid == BAD_PID)
		return rp->status;
	rp->status = sanitize_exit_status(wait_any(rp->pid));
	rp->pid = BAD_PID;
	return rp->status;
}

#else	/* PIPES_SIMULATED */

/*
 * use temporary file rather than pipe
 * except if popen() provides real pipes too
 */

/* gawk_popen --- open an IOBUF on a child process */

static IOBUF *
gawk_popen(const char *cmd, struct redirect *rp)
{
	FILE *current;

	os_restore_mode(fileno(stdin));
	set_sigpipe_to_default();

	current = popen(cmd, binmode("r"));

	if ((BINMODE & BINMODE_INPUT) != 0)
		os_setbinmode(fileno(stdin), O_BINARY);
	ignore_sigpipe();

	if (current == NULL)
		return NULL;
	os_close_on_exec(fileno(current), cmd, "pipe", "from");
	rp->iop = iop_alloc(fileno(current), cmd, 0);
	find_input_parser(rp->iop);
	iop_finish(rp->iop);
	if (! rp->iop->valid) {
		if (! do_traditional && rp->iop->errcode != 0)
			update_ERRNO_int(rp->iop->errcode);
		(void) pclose(current);
		rp->iop->publ.fd = INVALID_HANDLE;
		(void) iop_close(rp->iop);
		rp->iop = NULL;
		current = NULL;
	}
	rp->ifp = current;
	return rp->iop;
}

/* gawk_pclose --- close an open child pipe */

static int
gawk_pclose(struct redirect *rp)
{
	int rval, aval
	fd_t fd = rp->iop->publ.fd;

	if (rp->iop != NULL) {
		rp->iop->publ.fd = dup(fd);	  /* kludge to allow close() + pclose() */
		rval = iop_close(rp->iop);
	}
	rp->iop = NULL;
	aval = pclose(rp->ifp);
	rp->ifp = NULL;
	return (rval < 0 ? rval : aval);
}

#endif	/* PIPES_SIMULATED */

/* do_getline_redir --- read in a line, into var and with redirection */

NODE *
do_getline_redir(bool into_variable, enum redirval redirtype)
{
	struct redirect *rp = NULL;
	IOBUF *iop;
	int cnt = EOF;
	char *s = NULL;
	int errcode;
	NODE *redir_exp = NULL;
	NODE **lhs = NULL;
	int redir_error = 0;
	const awk_fieldwidth_info_t *field_width = NULL;

	if (into_variable)
		lhs = POP_ADDRESS();

	assert(redirtype != redirect_none);
	redir_exp = TOP();
	rp = redirect(redir_exp, redirtype, & redir_error, false);
	DEREF(redir_exp);
	decr_sp();
	if (rp == NULL) {
		if (redir_error) { /* failed redirect */
			if (! do_traditional)
				update_ERRNO_int(redir_error);
		}
		return make_number((AWKNUM) -1.0);
	} else if ((rp->flag & RED_TWOWAY) != 0 && rp->iop == NULL) {
		if (is_non_fatal_redirect(redir_exp->stptr, redir_exp->stlen)) {
			update_ERRNO_int(EBADF);
			return make_number((AWKNUM) -1.0);
		}
		(void) close_rp(rp, CLOSE_ALL);
		fatal(_("getline: attempt to read from closed read end of two-way pipe"));
	}
	iop = rp->iop;
	if (iop == NULL)		/* end of input */
		return make_number((AWKNUM) 0.0);

	errcode = 0;
	cnt = get_a_record(& s, iop, & errcode, (lhs ? NULL : & field_width));
	if (errcode != 0) {
		if (! do_traditional && (errcode != -1))
			update_ERRNO_int(errcode);
		return make_number((AWKNUM) cnt);
	}

	if (cnt == EOF) {
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
		return make_number((AWKNUM) 0.0);
	}

	if (lhs == NULL)	/* no optional var. */
		set_record(s, (size_t) cnt, field_width);
	else {			/* assignment to variable */
		unref(*lhs);
		*lhs = make_string(s, (size_t) cnt);
		(*lhs)->flags |= USER_INPUT;
	}

	return make_number((AWKNUM) 1.0);
}

/* do_getline --- read in a line, into var and without redirection */

NODE *
do_getline(bool into_variable, IOBUF *iop)
{
	int cnt = EOF;
	char *s = NULL;
	int errcode;
	const awk_fieldwidth_info_t *field_width = NULL;

	if (iop == NULL) {	/* end of input */
		if (into_variable)
			(void) POP_ADDRESS();
		return make_number((AWKNUM) 0.0);
	}

	errcode = 0;
	cnt = get_a_record(& s, iop, & errcode, (into_variable ? NULL : & field_width));
	if (errcode != 0) {
		if (! do_traditional && (errcode != -1))
			update_ERRNO_int(errcode);
		if (into_variable)
			(void) POP_ADDRESS();
		return make_number((AWKNUM) cnt);
	}

	if (cnt == EOF)
		return NULL;	/* try next file */
	INCREMENT_REC(NR);
	INCREMENT_REC(FNR);

	if (! into_variable)	/* no optional var. */
		set_record(s, (size_t) cnt, field_width);
	else {			/* assignment to variable */
		NODE **lhs;
		lhs = POP_ADDRESS();
		unref(*lhs);
		*lhs = make_string(s, (size_t) cnt);
		(*lhs)->flags |= USER_INPUT;
	}
	return make_number((AWKNUM) 1.0);
}

typedef struct {
	const char *envname;
	const char **dfltp;	/* pointer to address of default path */
	char **awkpath;		/* array containing library search paths */
	size_t max_pathlen;	/* length of the longest item in awkpath */
} path_info;

static path_info pi_awkpath = {
	/* envname */	"AWKPATH",
	/* dfltp */	& defpath,
	NULL,
	0
};

static path_info pi_awklibpath = {
	/* envname */	"AWKLIBPATH",
	/* dfltp */	& deflibpath,
	NULL,
	0
};

/* init_awkpath --- split path(=$AWKPATH) into components */

static void
init_awkpath(path_info *pi)
{
	const char *path, *start, *end, *p;
	size_t len;
	unsigned i, max_path;		/* (# of allocated paths)-1 */

	pi->max_pathlen = 0;
	if ((path = getenv(pi->envname)) == NULL || *path == '\0')
		path = pi->dfltp[0];

	/* count number of separators */
	for (max_path = 0, p = path; *p; p++)
		if (*p == envsep)
			max_path++;

	// +3 --> 2 for null entries at front and end of path, 1 for NULL end of list
	ezalloc(pi->awkpath, char **, (max_path + 3) * sizeof(char *), "init_awkpath");

	start = path;
	i = 0;

	if (*path == envsep) {	/* null entry at front of path */
		pi->awkpath[i++] = charp_const_cast(".");
		pi->max_pathlen = 1;
	}

	while (*start) {
		if (*start == envsep) {
			if (start[1] == envsep) {
				pi->awkpath[i++] = charp_const_cast(".");
				if (pi->max_pathlen == 0)
					pi->max_pathlen = 1;
				start++;
			} else if (start[1] == '\0') {
				pi->awkpath[i++] = charp_const_cast(".");
				if (pi->max_pathlen == 0)
					pi->max_pathlen = 1;
				break;
			} else
				start++;
		} else {
			for (end = start; *end && *end != envsep; end++)
				continue;

			len = (size_t) (end - start);
			if (len > 0) {
				char *ap;

				emalloc(ap, char *, len + 2, "init_awkpath");
				memcpy(ap, start, len);

				/* add directory punctuation if necessary */
				if (! isdirpunct(end[-1]))
					ap[len++] = '/';
				ap[len] = '\0';
				pi->awkpath[i++] = ap;
				if (len > pi->max_pathlen)
					pi->max_pathlen = len;

				start = end;
			} else
				start++;
		}
	}

	pi->awkpath[i] = NULL;
}

/* do_find_source --- search $AWKPATH for file, return NULL if not found */

static char *
do_find_source(const char *src, gawk_xstat_t *stb, int *errcode, path_info *pi)
{
	char *path;
	unsigned i;

	assert(errcode != NULL);

	/* some kind of path name, no search */
	if (ispath(src)) {
		emalloc(path, char *, strlen(src) + 1, "do_find_source");
		strcpy(path, src);
		if (os_xstat(path, stb) == 0)
			return path;
		*errcode = errno;
		efree(path);
		return NULL;
	}

	if (pi->awkpath == NULL)
		init_awkpath(pi);

	emalloc(path, char *, pi->max_pathlen + strlen(src) + 1, "do_find_source");
	for (i = 0; pi->awkpath[i] != NULL; i++) {
		if (strcmp(pi->awkpath[i], "./") == 0 || strcmp(pi->awkpath[i], ".") == 0)
			*path = '\0';
		else
			strcpy(path, pi->awkpath[i]);
		strcat(path, src);
		if (os_xstat(path, stb) == 0)
			return path;
	}

	/* not found, give up */
	*errcode = errno;
	efree(path);
	return NULL;
}

/* find_source --- find source file with default file extension handling */

char *
find_source(const char *src, gawk_xstat_t *stb, int *errcode, bool is_extlib)
{
	char *path;
	path_info *pi = (is_extlib ? & pi_awklibpath : & pi_awkpath);

	*errcode = 0;
	if (src == NULL || *src == '\0')
		return NULL;
#ifdef __EMX__
	char os2_src[strlen(src) + 1];

	if (is_extlib)
		src = os2_fixdllname(os2_src, src, sizeof(os2_src));
#endif /* __EMX__ */
	path = do_find_source(src, stb, errcode, pi);

#ifdef WINDOWS_NATIVE
	if (path == NULL && is_extlib && pi->awkpath[0] == NULL) {
		/* environment variable AWKLIBPATH is not set, DEFLIBPATH is empty,
		  assume DLLs are in the same directory as GAWK.EXE */
		wchar_t pathbuf[1024];
		unsigned len = GetModuleFileNameW(NULL, pathbuf, sizeof(pathbuf)/sizeof(pathbuf[0]));
		if (len > 0 && len < sizeof(pathbuf)/sizeof(pathbuf[0])) {
			/* trim "GAWK.EXE" */
			wchar_t *dirend = wcsrchr(pathbuf, L'\\');
			if (dirend != NULL && dirend != pathbuf) {
				size_t n;
				dirend[1/* for trailing '\\' */] = L'\0';
				n = wcstombs(NULL, pathbuf, 0);
				if (n > 0 && n != (size_t)-1) {
					char *dir;
					emalloc(dir, char *, n + 1, "find_source");
					(void) !wcstombs(dir, pathbuf, n + 1);
					assert(!pi->max_pathlen);
					pi->max_pathlen = n;
					assert(pi->awkpath[1] == NULL);
					pi->awkpath[0] = dir;
				}
				/* check if already has the SUFFIX - then try to find DLL now,
				  because below will skip the search if path has the SUFFIX. */
				{
					const char *suffix = strrchr(src, '.');
					if (suffix != NULL && 0 == strcmp(suffix + 1, SHLIBEXT))
						path = do_find_source(src, stb, errcode, pi);
				}
			}
		}
	}
#endif /* WINDOWS_NATIVE */

	if (path == NULL && is_extlib) {
		char *file_ext;
		int save_errno;
		size_t src_len;
		size_t suffix_len;

#define EXTLIB_SUFFIX	"." SHLIBEXT
		src_len = strlen(src);
		suffix_len = strlen(EXTLIB_SUFFIX);

		/* check if already has the SUFFIX */
		if (src_len >= suffix_len && strcmp(& src[src_len - suffix_len], EXTLIB_SUFFIX) == 0)
			return NULL;

		/* append EXTLIB_SUFFIX and try again */
		save_errno = errno;
		emalloc(file_ext, char *, src_len + suffix_len + 1, "find_source");
		sprintf(file_ext, "%s%s", src, EXTLIB_SUFFIX);
		path = do_find_source(file_ext, stb, errcode, pi);
		efree(file_ext);
		if (path == NULL)
			errno = save_errno;
		return path;
#undef EXTLIB_SUFFIX
	}

/*
 * Try searching with .awk appended if the platform headers have not specified
 * another suffix.
 */
#ifndef DEFAULT_FILETYPE
#define DEFAULT_FILETYPE ".awk"
#endif

#ifdef DEFAULT_FILETYPE
	if (! do_traditional && path == NULL) {
		char *file_awk;
		int save_errno = errno;
#ifdef VMS
		int vms_save = vaxc$errno;
#endif

		/* append ".awk" and try again */
		emalloc(file_awk, char *, strlen(src) +
			sizeof(DEFAULT_FILETYPE) + 1, "find_source");
		sprintf(file_awk, "%s%s", src, DEFAULT_FILETYPE);
		path = do_find_source(file_awk, stb, errcode, pi);
		efree(file_awk);
		if (path == NULL) {
			errno = save_errno;
#ifdef VMS
			vaxc$errno = vms_save;
#endif
		}
	}
#endif	/*DEFAULT_FILETYPE*/

	return path;
}


/* srcopen --- open source file */

fd_t
srcopen(SRCFILE *s)
{
	fd_t fd = INVALID_HANDLE;

	if (s->stype == SRC_STDIN)
		fd = fileno(stdin);
	else if (s->stype == SRC_FILE || s->stype == SRC_INC)
		fd = devopen(s->fullpath, "r");

	/* set binary mode so that debugger byte offset calculations will be right */
	if (fd != INVALID_HANDLE)
		os_setbinmode(fd, O_BINARY);

	return fd;
}

/* input parsers, mainly for use by extension functions */

static awk_input_parser_t *ip_head, *ip_tail;

/*
 * register_input_parser --- add an input parser to the list, FIFO.
 * 	The main reason to use FIFO is to provide the diagnostic
 * 	with the correct information: input parser 2 conflicts
 * 	with input parser 1.  Otherwise LIFO would have been easier.
 */

void
register_input_parser(awk_input_parser_t *input_parser)
{
	if (input_parser == NULL)
		fatal(_("register_input_parser: received NULL pointer"));

	input_parser->next = NULL;	/* force it */
	if (ip_head == NULL) {
		ip_head = ip_tail = input_parser;
	} else {
		ip_tail->next = input_parser;
		ip_tail = ip_tail->next;
	}
}

/* find_input_parser --- search the list of input parsers */

static void
find_input_parser(IOBUF *iop)
{
	awk_input_parser_t *ip, *ip2;

	/* if already associated with an input parser, bail out early */
	if (iop->publ.get_record != NULL)
		return;

	ip = ip2 = NULL;
	for (ip2 = ip_head; ip2 != NULL; ip2 = ip2->next) {
		if (ip2->can_take_file(& iop->publ)) {
			if (ip == NULL)
				ip = ip2;	/* found first one */
			else
				fatal(_("input parser `%s' conflicts with previously installed input parser `%s'"),
						ip2->name, ip->name);
		}
	}

	if (ip != NULL) {
		if (! ip->take_control_of(& iop->publ))
			awkwarn(_("input parser `%s' failed to open `%s'"),
					ip->name, iop->publ.name);
		else
			iop->valid = true;
	}
}

/* output wrappers --- for use by extensions */

static awk_output_wrapper_t *op_head, *op_tail;

/*
 * register_output_wrapper --- add an output wrapper to the list.
 * 	Same stuff here as for input parsers.
 */

void
register_output_wrapper(awk_output_wrapper_t *wrapper)
{
	if (wrapper == NULL)
		fatal(_("register_output_wrapper: received NULL pointer"));

	wrapper->next = NULL;	/* force it */
	if (op_head == NULL) {
		op_head = op_tail = wrapper;
	} else {
		op_tail->next = wrapper;
		op_tail = op_tail->next;
	}
}

/* find_output_wrapper --- search the list of output wrappers */

static bool
find_output_wrapper(awk_output_buf_t *outbuf)
{
	awk_output_wrapper_t *op, *op2;

	/* if already associated with an output wrapper, bail out early */
	if (outbuf->redirected)
		return false;

	op = op2 = NULL;
	for (op2 = op_head; op2 != NULL; op2 = op2->next) {
		if (op2->can_take_file(outbuf)) {
			if (op == NULL)
				op = op2;	/* found first one */
			else
				fatal(_("output wrapper `%s' conflicts with previously installed output wrapper `%s'"),
						op2->name, op->name);
		}
	}

	if (op != NULL) {
		if (! op->take_control_of(outbuf)) {
			awkwarn(_("output wrapper `%s' failed to open `%s'"),
					op->name, outbuf->name);
			return false;
		}
		return true;
	}

	return false;
}


/* two way processors --- for use by extensions */

static awk_two_way_processor_t *tw_head, *tw_tail;

/* register_two_way_processor --- register a two-way I/O processor, for extensions */

void
register_two_way_processor(awk_two_way_processor_t *processor)
{
	if (processor == NULL)
		fatal(_("register_output_processor: received NULL pointer"));

	processor->next = NULL;	/* force it */
	if (tw_head == NULL) {
		tw_head = tw_tail = processor;
	} else {
		tw_tail->next = processor;
		tw_tail = tw_tail->next;
	}
}

/* find_two_way_processor --- search the list of two way processors */

static bool
find_two_way_processor(const char *name, struct redirect *rp)
{
	awk_two_way_processor_t *tw, *tw2;

	/* if already associated with i/o, bail out early */
	if (   (rp->iop != NULL && rp->iop->publ.fd != INVALID_HANDLE)
	    || rp->output.file != NULL)
		return false;

	tw = tw2 = NULL;
	for (tw2 = tw_head; tw2 != NULL; tw2 = tw2->next) {
		if (tw2->can_take_two_way(name)) {
			if (tw == NULL)
				tw = tw2;	/* found first one */
			else
				fatal(_("two-way processor `%s' conflicts with previously installed two-way processor `%s'"),
						tw2->name, tw->name);
		}
	}

	if (tw != NULL) {
		if (rp->iop == NULL)
			rp->iop = iop_alloc(INVALID_HANDLE, name, 0);
		if (! tw->take_control_of(name, & rp->iop->publ, & rp->output)) {
			awkwarn(_("two way processor `%s' failed to open `%s'"),
					tw->name, name);
			return false;
		}
		iop_finish(rp->iop);
		return true;
	}

	return false;
}

/*
 * IOBUF management is somewhat complicated.  In particular,
 * it is possible and OK for an IOBUF to be allocated with
 * a file descriptor that is either valid or not usable with
 * read(2), in case an input parser will come along later and
 * make it readable.  Alternatively, an input parser can simply
 * come along and take over reading on a valid readable descriptor.
 *
 * The first stage is simply to allocate the IOBUF.  This is done
 * during nextfile() for command line files and by redirect()
 * and other routines for getline, input pipes, and the input
 * side of a two-way pipe.
 *
 * The second stage is to check for input parsers.  This is done
 * for command line files in after_beginfile() and for the others
 * as part of the full flow.  At this point, either:
 * 	- The fd is valid on a readable file
 * 	- The input parser has taken over a valid fd and made
 * 	  it usable (e.g., directories)
 * 	- Or the input parser has simply hijacked the reading
 * 	  (such as the gawkextlib XML extension)
 * If none of those are true, the fd should be closed, reset
 * to INVALID_HANDLE, and iop->errcode set to indicate the error
 * (EISDIR for directories, EIO for anything else).
 * iop->valid should be set to false in this case.
 *
 * Otherwise, after the second stage, iop->errcode should be
 * zero, iop->valid should be true, and iop->publ.fd should
 * not be INVALID_HANDLE.
 *
 * The third stage is to set up the rest of the IOBUF for
 * use by get_a_record(). In this case, iop->valid must
 * be true already, and iop->publ.fd cannot be INVALID_HANDLE.
 *
 * Checking for input parsers for command line files is delayed
 * to after_beginfile() so that the BEGINFILE rule has an
 * opportunity to look at FILENAME and ERRNO and attempt to
 * recover with a custom input parser. The XML extension, in
 * particular, relies strongly upon this ability.
 */

/* iop_alloc --- allocate an IOBUF structure for an open fd */

static IOBUF *
iop_alloc(fd_t fd, const char *name, int errno_val)
{
	IOBUF *iop;

	ezalloc(iop, IOBUF *, sizeof(IOBUF), "iop_alloc");

	iop->publ.fd = fd;
	iop->publ.name = name;
	iop->publ.read_func = read_wrap;
	iop->valid = false;
	iop->errcode = errno_val;

	if (fd != INVALID_HANDLE)
		os_xfstat(fd, & iop->pubc.sbuf);
#if defined(__EMX__) || defined(WINDOWS_NATIVE)
	else if (errno_val == EISDIR) {
		iop->pubc.sbuf.st_mode = (S_IFDIR | S_IRWXU);
		iop->publ.fd = FAKE_FD_VALUE;
	}
#endif

	return iop;
}

/* iop_finish --- finish setting up an IOBUF */

static IOBUF *
iop_finish(IOBUF *iop)
{
	bool isdir = false;

	if (iop->publ.fd != INVALID_HANDLE) {
		if (os_isreadable(& iop->publ, & isdir))
			iop->valid = true;
		else {
			if (isdir)
				iop->errcode = EISDIR;
			else {
				iop->errcode = EIO;
				/*
				 * Extensions can supply values that are not
				 * INVALID_HANDLE but that are also not real
				 * file descriptors. So check the fd before
				 * trying to close it, which avoids errors
				 * on some operating systems.
				 *
				 * The fcntl call works for Windows, too.
				 */
#if defined(F_GETFL)
				if (fcntl(iop->publ.fd, F_GETFL) >= 0)
#endif
					(void) close(iop->publ.fd);
				iop->publ.fd = INVALID_HANDLE;
			}
			/*
			 * Don't close directories: after_beginfile(),
			 * special cases them.
			 */
		}
	}

	if (! iop->valid || iop->publ.fd == INVALID_HANDLE)
		return iop;

	if (os_isatty(iop->publ.fd))
		iop->flag |= IOP_IS_TTY;

	iop->readsize = iop->size = optimal_bufsize(iop->publ.fd, & iop->pubc.sbuf);
	if (do_lint && S_ISREG(iop->pubc.sbuf.st_mode) && iop->pubc.sbuf.st_size == 0)
		lintwarn(_("data file `%s' is empty"), iop->publ.name);
	iop->errcode = errno = 0;
	iop->count = 0;
	iop->scanoff = 0;
	emalloc(iop->buf, char *, iop->size += 1, "iop_finish");
	iop->off = iop->buf;
	iop->dataend = NULL;
	iop->end = iop->buf + iop->size;
	iop->flag |= IOP_AT_START;

	return iop;
}

#define set_RT_to_null() do { \
	if (do_traditional) break; \
	unref(RT_node->var_value); \
	RT_node->var_value = dupnode(Nnull_string); \
} while ((void)0,0)

#define set_RT(str, len) do { \
	if (do_traditional) break; \
	unref(RT_node->var_value); \
	RT_node->var_value = make_string(str, len); \
} while ((void)0,0)

/*
 * grow_iop_buffer:
 *
 * grow must increase size of buffer, set end, make sure off and dataend
 * point at the right spot.
 */

static void
grow_iop_buffer(IOBUF *iop)
{
	size_t valid = (size_t) (iop->dataend - iop->off);
	size_t off = (size_t) (iop->off - iop->buf);
	size_t newsize;

	/*
	 * Lop off original extra byte, double the size,
	 * add it back.
	 */
	newsize = ((iop->size - 1) * 2) + 1;

	/* Check for overflow */
	if (newsize <= iop->size)
		fatal(_("could not allocate more input memory"));

	/* Make sure there's room for a disk block */
	/* Note: need a space for one extra byte over disk block */
	if (newsize - valid <= iop->readsize)
		newsize = valid + iop->readsize + 1;

	/* Check for overflow, again */
	if (newsize <= iop->size)
		fatal(_("could not allocate more input memory"));

	iop->size = newsize;
	erealloc(iop->buf, char *, iop->size, "grow_iop_buffer");
	iop->off = iop->buf + off;
	iop->dataend = iop->off + valid;
	iop->end = iop->buf + iop->size;
}

/* rs1scan --- scan for a single character record terminator */

static RECVALUE
rs1scan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state)
{
	char *bp;
	char rs;
	size_t mbclen = 0;
	mbstate_t mbs;

	memset(recm, '\0', sizeof(struct recmatch));
	rs = RS->stptr[0];
	*(iop->dataend) = rs;   /* set sentinel */
	recm->start = iop->off; /* beginning of record */

	bp = iop->off;
	if (*state == INDATA)   /* skip over data we've already seen */
		bp += iop->scanoff;

	/*
	 * From: Bruno Haible <bruno@clisp.org>
	 * To: Aharon Robbins <arnold@skeeve.com>, gnits@gnits.org
	 * Subject: Re: multibyte locales: any way to find if a character isn't multibyte?
	 * Date: Mon, 23 Jun 2003 12:20:16 +0200
	 * Cc: isamu@yamato.ibm.com
	 *
	 * Hi,
	 *
	 * > Is there any way to make the following query to the current locale?
	 * >
	 * > 	Given an 8-bit value, can this value ever appear as part of
	 * > 	a multibyte character?
	 *
	 * There is no simple answer here. The easiest solution I see is to
	 * get the current locale's codeset (via locale_charset() which is a
	 * wrapper around nl_langinfo(CODESET)), and then perform a case-by-case
	 * treatment of the known multibyte encodings, from GB2312 to EUC-JISX0213;
	 * for the unibyte encodings, a single btowc() call will tell you.
	 *
	 * > This is particularly critical for me for ASCII newline ('\n').  If I
	 * > can be guaranteed that it never shows up as part of a multibyte character,
	 * > I can speed up gawk considerably in mulitbyte locales.
	 *
	 * This is much simpler to answer!
	 * In all ASCII based multibyte encodings used for locales today (this
	 * excludes EBCDIC based doublebyte encodings from IBM, and also excludes
	 * ISO-2022-JP which is used for email exchange but not as a locale encoding)
	 * ALL bytes in the range 0x00..0x2F occur only as a single character, not
	 * as part of a multibyte character.
	 *
	 * So it's safe to assume, but deserves a comment in the source.
	 *
	 * Bruno
	 ***************************************************************
	 * From: Bruno Haible <bruno@clisp.org>
	 * To: Aharon Robbins <arnold@skeeve.com>
	 * Subject: Re: multibyte locales: any way to find if a character isn't multibyte?
	 * Date: Mon, 23 Jun 2003 14:27:49 +0200
	 *
	 * On Monday 23 June 2003 14:11, you wrote:
	 *
	 * >       if (rs != '\n' && MB_CUR_MAX > 1) {
	 *
	 * If you assume ASCII, you can even write
	 *
	 *         if (rs >= 0x30 && MB_CUR_MAX > 1) {
	 *
	 * (this catches also the space character) but if portability to EBCDIC
	 * systems is desired, your code is fine as is.
	 *
	 * Bruno
	 */
	/* Thus, the check for \n here; big speedup ! */
	if (rs != '\n' && gawk_mb_cur_max > 1) {
		size_t len = (size_t) (iop->dataend - bp);
		bool found = false;

		memset(& mbs, 0, sizeof(mbstate_t));
		do {
			if (*bp == rs)
				found = true;
			if (is_valid_character(*bp))
				mbclen = 1;
			else
				mbclen = mbrlen(bp, len, & mbs);
			if (   mbclen == 1
			    || mbclen == (size_t) -1
			    || mbclen == (size_t) -2
			    || mbclen == 0) {
				/* We treat it as a single-byte character.  */
				mbclen = 1;
			}
			len -= mbclen;
			bp += mbclen;
		} while (len > 0 && ! found);

		/* Check that newline found isn't the sentinel. */
		if (found && (bp - mbclen) < iop->dataend) {
			/*
			 * Set len to what we have so far, in case this is
			 * all there is.
			 */
			recm->len = (size_t) (bp - recm->start) - mbclen;
			recm->rt_start = bp - mbclen;
			recm->rt_len = mbclen;
			*state = NOSTATE;
			return REC_OK;
		} else {
			/* also set len */
			recm->len = (size_t) (bp - recm->start);
			*state = INDATA;
			iop->scanoff = (size_t) (bp - iop->off);
			return NOTERM;
		}
	}

	while (*bp != rs)
		bp++;

	/* set len to what we have so far, in case this is all there is */
	recm->len = (size_t) (bp - recm->start);

	if (bp < iop->dataend) {        /* found it in the buffer */
		recm->rt_start = bp;
		recm->rt_len = 1;
		*state = NOSTATE;
		return REC_OK;
	} else {
		*state = INDATA;
		iop->scanoff = (size_t) (bp - iop->off);
		return NOTERM;
	}
}

/* rsrescan --- search for a regex match in the buffer */

static RECVALUE
rsrescan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state)
{
	char *bp;
	regoff_t restart = 0, reend = 0;
	Regexp *RSre = RS_regexp;
	int regex_flags = RE_NEED_START;

	memset(recm, '\0', sizeof(struct recmatch));
	recm->start = iop->off;

	bp = iop->off;
	if (*state == INDATA)
		bp += iop->scanoff;

	if ((iop->flag & IOP_AT_START) == 0)
		regex_flags |= RE_NO_BOL;
again:
	/* case 1, no match */
	if (research(RSre, bp, 0, (size_t) (iop->dataend - bp), regex_flags) == -1) {
		/* set len, in case this all there is. */
		recm->len = (size_t) (iop->dataend - iop->off);
		return NOTERM;
	}

	/* ok, we matched within the buffer, set start and end */
	restart = RESTART(RSre, iop->off);
	reend = REEND(RSre, iop->off);

	/* case 2, null regex match, grow buffer, try again */
	if (restart == reend) {
		*state = INDATA;
		iop->scanoff = (size_t) reend + 1;
		/*
		 * If still room in buffer, skip over null match
		 * and restart search. Otherwise, return.
		 */
		if (iop->scanoff < (size_t) (iop->dataend - bp)) {
			bp += iop->scanoff;
			goto again;
		}
		recm->len = (size_t) (bp - iop->off) + (size_t) restart;
		return NOTERM;
	}

	/*
	 * At this point, we have a non-empty match.
	 *
	 * First, fill in rest of data. The rest of the cases return
	 * a record and terminator.
	 */
	recm->len = (size_t) restart;
	recm->rt_start = bp + restart;
	recm->rt_len = (size_t) (reend - restart);
	*state = NOSTATE;

	/*
	 * 3. Match exactly at end:
	 *      if re is a simple string match
	 *              found a simple string match at end, return REC_OK
	 *      else
	 *              grow buffer, add more data, try again
	 *      fi
	 */
	if (iop->off + reend >= iop->dataend) {
		if (reisstring(RS->stptr, RS->stlen, RSre, iop->off))
			return REC_OK;
		else
			return TERMATEND;
	}

	/*
	 * 4. Match within xxx bytes of end & maybe islong re:
	 *      return TERMNEAREND
	 */

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
         * The re->maybe_long member is true if the
         * regex contains one of: + * ? |.  This is a very
         * simple heuristic, but in combination with the
         * "end of match within a few bytes of end of buffer"
         * check, should keep things reasonable.
         */

	/* succession of tests is easier to trace in GDB. */
	if (RSre->maybe_long) {
		const char *matchend = iop->off + reend;
		size_t tail = (size_t) (iop->dataend - matchend);

		if (tail < RS->stlen)
			return TERMNEAREND;
	}

	return REC_OK;
}

/* rsnullscan --- handle RS = "" */

static RECVALUE
rsnullscan(IOBUF *iop, struct recmatch *recm, SCANSTATE *state)
{
	char *bp;

	if (*state == NOSTATE || *state == INLEADER)
		memset(recm, '\0', sizeof(struct recmatch));

	recm->start = iop->off;

	bp = iop->off;
	if (*state != NOSTATE)
		bp += iop->scanoff;

	/* set sentinel */
	*(iop->dataend) = '\n';

	if (*state == INTERM)
		goto find_longest_terminator;
	else if (*state == INDATA)
		goto scan_data;
	/* else
		fall into things from beginning,
		either NOSTATE or INLEADER */

/* skip_leading: */
	/* leading newlines are ignored */
	while (*bp == '\n' && bp < iop->dataend)
		bp++;

	if (bp >= iop->dataend) {       /* LOTS of leading newlines, sheesh. */
		*state = INLEADER;
		iop->scanoff = (size_t) (bp - iop->off);
		return NOTERM;
	}

	iop->off = recm->start = bp;    /* real start of record */
scan_data:
	while (*bp++ != '\n')
		continue;

	if (bp >= iop->dataend) {       /* no full terminator */
		iop->scanoff = recm->len = (size_t) (bp - iop->off) - 1;
		if (bp == iop->dataend) {	/* half a terminator */
			recm->rt_start = bp - 1;
			recm->rt_len = 1;
		}
		*state = INDATA;
		return NOTERM;
	}

	/* found one newline before end of buffer, check next char */
	if (*bp != '\n')
		goto scan_data;

	/* we've now seen at least two newlines */
	*state = INTERM;
	recm->len = (size_t) (bp - iop->off) - 1;
	recm->rt_start = bp - 1;

find_longest_terminator:
	/* find as many newlines as we can, to set RT */
	while (*bp == '\n' && bp < iop->dataend)
		bp++;

	recm->rt_len = (size_t) (bp - recm->rt_start);
	iop->scanoff = (size_t) (bp - iop->off);

	if (bp >= iop->dataend)
		return TERMATEND;

	return REC_OK;
}

/* retryable --- return true if PROCINFO[<filename>, "RETRY"] exists */

static inline bool
retryable(IOBUF *iop)
{
	return PROCINFO_node && in_PROCINFO_z(iop->publ.name, "RETRY", NULL);
}

/* errno_io_retry --- Does the I/O error indicate that the operation should be retried later? */

static inline bool
errno_io_retry(void)
{
	switch (errno) {
#ifdef EAGAIN
	case EAGAIN:
#endif
#ifdef EWOULDBLOCK
#if !defined(EAGAIN) || (EWOULDBLOCK != EAGAIN)
	case EWOULDBLOCK:
#endif
#endif
#ifdef EINTR
	case EINTR:
#endif
#ifdef ETIMEDOUT
	case ETIMEDOUT:
#endif
		return true;
	default:
		return false;
	}
}

/*
 * get_a_record --- read a record from IOP into out,
 * return length or EOF, set RT.
 * Note that errcode is never NULL, and the caller initializes *errcode to 0.
 * If I/O would block, return -2.
 */

static int
get_a_record(char **out,        /* pointer to pointer to data */
        IOBUF *iop,             /* input IOP */
        int *errcode,           /* pointer to error variable */
        const awk_fieldwidth_info_t **field_width)
				/* pointer to pointer to field_width info */
{
	struct recmatch recm;
	SCANSTATE state;
	RECVALUE ret;
	int retval;
	NODE *rtval = NULL;
	static RECVALUE (*lastmatchrec)(IOBUF *iop, struct recmatch *recm, SCANSTATE *state) = NULL;

	if (at_eof(iop) && no_data_left(iop))
		return EOF;

	if (read_can_timeout)
		read_timeout = get_read_timeout(iop);

	if (iop->publ.get_record != NULL) {
		char *rt_start;
		size_t rt_len;
		int rc = iop->publ.get_record(out, &iop->publ, errcode,
						&rt_start, &rt_len,
						field_width);
		if (rc == EOF)
			iop->flag |= IOP_AT_EOF;
		else {
			if (rt_len != 0)
				set_RT(rt_start, rt_len);
			else
				set_RT_to_null();
		}
		return rc;
	}

	/* fill initial buffer */
	if (has_no_data(iop) || no_data_left(iop)) {
		iop->count = (*iop->publ.read_func)
			(iop->publ.fd, iop->buf, iop->readsize);
		if (iop->count == 0) {
			iop->flag |= IOP_AT_EOF;
			return EOF;
		} else if (iop->count == -1) {
			*errcode = errno;
			if (errno_io_retry() && retryable(iop))
				return -2;
			iop->flag |= IOP_AT_EOF;
			return EOF;
		} else {
			iop->dataend = iop->buf + iop->count;
			iop->off = iop->buf;
		}
	}

	/* loop through file to find a record */
	state = NOSTATE;
	for (;;) {
		size_t dataend_off;
		size_t room_left;
		size_t amt_to_read;

		ret = (*matchrec)(iop, & recm, & state);
		iop->flag &= ~IOP_AT_START;
		/* found the record, we're done, break the loop */
		if (ret == REC_OK)
			break;

		/*
		 * Likely found the record; if there's no more data
		 * to be had (like from a tiny regular file), break the
		 * loop. Otherwise, see if we can read more.
		 */
		if (ret == TERMNEAREND && buffer_has_all_data(iop))
			break;

		/* need to add more data to buffer */
		/* shift data down in buffer */
		dataend_off = (size_t) (iop->dataend - iop->off);
		memmove(iop->buf, iop->off, dataend_off);
		iop->off = iop->buf;
		iop->dataend = iop->buf + dataend_off;

		/* adjust recm contents */
		recm.start = iop->off;
		if (recm.rt_start != NULL)
			recm.rt_start = iop->off + recm.len;

		/* read more data, break if EOF */
		/* subtract one in read count to leave room for sentinel */
		room_left = (size_t) (iop->end - iop->dataend) - 1;
		amt_to_read = iop->readsize;

		if (room_left < iop->readsize) {
			grow_iop_buffer(iop);
			/* adjust recm contents */
			recm.start = iop->off;
			if (recm.rt_start != NULL)
				recm.rt_start = iop->off + recm.len;
			room_left = (size_t) (iop->end - iop->dataend) - 1;
			assert(room_left >= iop->readsize);
		}
		while (amt_to_read <= room_left - iop->readsize)
			amt_to_read += iop->readsize;

#ifdef SSIZE_MAX
		/*
		 * POSIX limits read to SSIZE_MAX. There are (bizarre)
		 * systems where this amount is small.
		 */
		if (amt_to_read > SSIZE_MAX)
			amt_to_read = SSIZE_MAX;
#endif

		iop->count = (*iop->publ.read_func)
			(iop->publ.fd, iop->dataend, amt_to_read);
		if (iop->count == -1) {
			*errcode = errno;
			if (errno_io_retry() && retryable(iop))
				return -2;
			iop->flag |= IOP_AT_EOF;
			break;
		} else if (iop->count == 0) {
			/*
			 * Hit EOF before being certain that we've matched
			 * the end of the record. If ret is TERMNEAREND,
			 * we need to pull out what we've got in the buffer.
			 * Eventually we'll come back here and see the EOF,
			 * end the record and set RT to "".
			 */
			if (ret != TERMNEAREND)
				iop->flag |= IOP_AT_EOF;
			break;
		} else
			iop->dataend += iop->count;
	}

	/* set record, RT, return right value */

	/*
	 * rtval is not a static pointer to avoid dangling pointer problems
	 * in case awk code assigns to RT.  A remote possibility, to be sure,
	 * but Bitter Experience teaches us not to make ``that'll never
	 * happen'' kinds of assumptions.
	 */
	rtval = RT_node->var_value;

	if (recm.rt_len == 0) {
		set_RT_to_null();
		lastmatchrec = NULL;
	} else {
		assert(recm.rt_start != NULL);
		/*
		 * Optimization. For rs1 case, don't set RT if
		 * character is same as last time.  This knocks a
		 * chunk of time off something simple like
		 *
		 *      gawk '{ print }' /some/big/file
		 *
		 * Similarly, for rsnull case, if length of new RT is
		 * shorter than current RT, just bump length down in RT.
		 *
		 * Make sure that matchrec didn't change since the last
		 * check.  (Ugh, details, details, details.)
		 */
		if (lastmatchrec == NULL || lastmatchrec != matchrec) {
			lastmatchrec = matchrec;
			set_RT(recm.rt_start, recm.rt_len);
		} else if (matchrec == rs1scan) {
			if (rtval->stlen != 1 || rtval->stptr[0] != recm.rt_start[0])
				set_RT(recm.rt_start, recm.rt_len);
			/* else
				leave it alone */
		} else if (matchrec == rsnullscan) {
			if (rtval->stlen >= recm.rt_len) {
				rtval->stlen = recm.rt_len;
				free_wstr(rtval);
			} else
				set_RT(recm.rt_start, recm.rt_len);
		} else
			set_RT(recm.rt_start, recm.rt_len);
	}

	if (recm.len == 0) {
		*out = NULL;
		retval = 0;
	} else {
		assert(recm.start != NULL);
		*out = recm.start;
		retval = (int) recm.len;
	}

	iop->off += recm.len + recm.rt_len;

	if (recm.len == 0 && recm.rt_len == 0 && at_eof(iop))
		return EOF;
	else
		return retval;
}

/* set_RS --- update things as appropriate when RS is set */

void
set_RS(void)
{
	static NODE *save_rs = NULL;

	/*
	 * Don't use cmp_nodes(), which pays attention to IGNORECASE.
	 */
	if (save_rs
		&& RS_node->var_value->stlen == save_rs->stlen
		&& memcmp(RS_node->var_value->stptr, save_rs->stptr, save_rs->stlen) == 0) {
		/*
		 * It could be that just IGNORECASE changed.  If so,
		 * update the regex and then do the same for FS.
		 * set_IGNORECASE() relies on this routine to call
		 * set_FS().
		 */
		RS_regexp = RS_re[IGNORECASE];
		goto set_FS;
	}
	unref(save_rs);
	save_rs = dupnode(RS_node->var_value);
	RS_is_null = false;
	RS = force_string(RS_node->var_value);
	/*
	 * used to be if (RS_regexp != NULL) { refree(..); refree(..); ...; }.
	 * Please do not remerge the if condition; hinders memory deallocation
	 * in case of fatal error in make_regexp.
	 */
	refree(RS_re[0]);	/* NULL argument is ok */
	refree(RS_re[1]);
	RS_re[0] = RS_re[1] = RS_regexp = NULL;

	if (RS->stlen == 0) {
		RS_is_null = true;
		matchrec = rsnullscan;
	} else if ((RS->stlen > 1 || (RS->flags & REGEX) != 0) && ! do_traditional) {
		static bool warned = false;

		RS_re[0] = make_regexp(RS->stptr, RS->stlen, false, true, true);
		RS_re[1] = make_regexp(RS->stptr, RS->stlen, true, true, true);
		RS_regexp = RS_re[IGNORECASE];

		matchrec = rsrescan;

		if (do_lint_extensions && ! warned) {
			lintwarn(_("multicharacter value of `RS' is a gawk extension"));
			warned = true;
		}
	} else
		matchrec = rs1scan;
set_FS:
	if (current_field_sep() == Using_FS)
		set_FS();
}


/* pty_vs_pipe --- return true if should use pty instead of pipes for `|&' */

/*
 * This works by checking if PROCINFO["command", "pty"] exists and is true.
 */

#ifdef HAVE_TERMIOS_H
static bool
pty_vs_pipe(const char *command)
{
	NODE *val;

	/*
	 * N.B. No need to check for NULL PROCINFO_node, since the
	 * in_PROCINFO function now checks that for us.
	 */
	val = in_PROCINFO_z(command, "pty", NULL);
	if (val)
		return boolval(val);
	return false;
}
#endif /* HAVE_TERMIOS_H */

#if 0
/* iopflags2str --- make IOP flags printable */

static const char *
iopflags2str(int flag)
{
	static const struct flagtab values[] = {
		{ IOP_IS_TTY, "IOP_IS_TTY" },
		{ IOP_AT_EOF,  "IOP_AT_EOF" },
		{ IOP_CLOSED, "IOP_CLOSED" },
		{ IOP_AT_START,  "IOP_AT_START" },
		{ 0, NULL }
	};

	return genflags2str(flag, values);
}
#endif

/* free_rp --- release the memory used by rp */

static void
free_rp(struct redirect *rp)
{
	efree(rp->value);
	efree(rp);
}

/* inetfile --- return true for a /inet special file, set other values */

static bool
inetfile(const char *str, size_t len, struct inet_socket_info *isi)
{
#ifndef HAVE_SOCKETS
	return false;
#else
	const char *cp = str;
	const char *cpend = str + len;
	struct inet_socket_info buf;

	/* syntax: /inet/protocol/localport/hostname/remoteport */
	if (len < 5 || memcmp(cp, "/inet", 5) != 0 || len >= (unsigned)-1)
		/* quick exit */
		return false;
	if (! isi)
		isi = & buf;
	cp += 5;
	if (cpend - cp < 2)
		return false;
	switch (*cp) {
	case '/':
		isi->family = AF_UNSPEC;
		break;
	case '4':
		if (*++cp != '/')
			return false;
		isi->family = AF_INET;
		break;
	case '6':
		if (*++cp != '/')
			return false;
		isi->family = AF_INET6;
		break;
	default:
		return false;
	}
	cp++;	/* skip past '/' */

	/* which protocol? */
	if (cpend - cp < 5)
		return false;
	if (memcmp(cp, "tcp/", 4) == 0)
		isi->protocol = SOCK_STREAM;
	else if (memcmp(cp, "udp/", 4) == 0)
		isi->protocol = SOCK_DGRAM;
	else
		return false;
	cp += 4;

	/* which localport? */
	isi->localport.offset = (unsigned)(cp - str);
	while (*cp != '/') {
		if (++cp >= cpend)
			return false;
	}
	/*
	 * Require a port, let them explicitly put 0 if
	 * they don't care.
	 */
	if ((isi->localport.len = (unsigned)(cp - str) - isi->localport.offset) == 0)
		return false;

	/* which hostname? */
	if (cpend - cp < 2)
		return false;
	cp++;
	isi->remotehost.offset = (unsigned)(cp - str);
	while (*cp != '/') {
		if (++cp >= cpend)
			return false;
	}
	if ((isi->remotehost.len = (unsigned)(cp - str) - isi->remotehost.offset) == 0)
		return false;

	/* which remoteport? */
	if (cpend - cp < 2)
		return false;
	cp++;
	/*
	 * The remote port ends the special file name.
	 *
	 * Here too, require a port, let them explicitly put 0 if
	 * they don't care.
	 */
	isi->remoteport.offset = (unsigned)(cp - str);
	while (*cp != '/' && cp < cpend)
		cp++;
	if (cp != cpend || ((isi->remoteport.len = (unsigned)(cp - str) - isi->remoteport.offset) == 0))
		return false;

#ifndef HAVE_GETADDRINFO
	/* final check for IPv6: */
	if (isi->family == AF_INET6)
		fatal(_("IPv6 communication is not supported"));
#endif
	return true;
#endif /* HAVE_SOCKETS */
}

/*
 * in_PROCINFO --- return value for a PROCINFO element with
 *	SUBSEP seperated indices.
 */

static NODE *
in_PROCINFO(const char *pidx1, size_t len1, const char *pidx2, size_t len2, NODE **full_idx)
{
	size_t str_len;
	NODE *r, *sub = NULL;
	NODE *subsep = SUBSEP_node->var_value;

	if (PROCINFO_node == NULL || (pidx1 == NULL && pidx2 == NULL))
		return NULL;

	/* full_idx is in+out parameter */

	if (full_idx)
		sub = *full_idx;

	if (pidx1 != NULL && pidx2 == NULL)
		str_len = len1;
	else if (pidx1 == NULL && pidx2 != NULL)
		str_len = len2;
	else
		str_len = len1 + subsep->stlen + len2;

	if (sub == NULL) {
		char *str;
		emalloc(str, char *, str_len + 1, "in_PROCINFO");
		sub = make_str_node(str, str_len, ALREADY_MALLOCED);
		if (full_idx)
			*full_idx = sub;
	} else if (str_len != sub->stlen) {
		/* *full_idx != NULL */

		assert(sub->valref == 1u);
		erealloc(sub->stptr, char *, str_len + 1, "in_PROCINFO");
		sub->stlen = str_len;
	}

	if (pidx1 != NULL && pidx2 == NULL)
		((char*) memcpy(sub->stptr, pidx1, len1))[len1] = '\0';
	else if (pidx1 == NULL && pidx2 != NULL)
		((char*) memcpy(sub->stptr, pidx2, len2))[len2] = '\0';
	else {
		char *s = sub->stptr;
		s = (char*) memcpy(s, pidx1, len1) + len1;
		s = (char*) memcpy(s, subsep->stptr, subsep->stlen) + subsep->stlen;
		s = (char*) memcpy(s, pidx2, len2) + len2;
		*s = '\0';
	}

	r = in_array(PROCINFO_node, sub);
	if (! full_idx)
		unref(sub);
	return r;
}

/* get_read_timeout --- get timeout in milliseconds for reading */

static awk_ulong_t
get_read_timeout(IOBUF *iop)
{
	awk_ulong_t tmout = 0u;

	if (PROCINFO_node != NULL) {
		const char *name = iop->publ.name;
		NODE *val = NULL;
		static NODE *full_idx = NULL;
		static char *last_name = NULL;

		/*
		 * Do not re-construct the full index when last redirection
		 * string is the same as the current; "efficiency_hack++".
		 */
		if (full_idx == NULL || strcmp(name, last_name) != 0) {
			val = in_PROCINFO_z(name, "READ_TIMEOUT", & full_idx);
			if (last_name != NULL)
				efree(last_name);
			last_name = estrdup(name, strlen(name));
		} else	/* use cached full index */
			val = in_array(PROCINFO_node, full_idx);

		if (val != NULL) {
			awk_long_t t;
			(void) force_number(val);
			t = get_number_si(val);
			if (t > 0)
				tmout = (awk_ulong_t) t;
		}
	} else
		tmout = read_default_timeout;	/* initialized from env. variable in init_io() */

	/* overwrite read routine only if an extension has not done so */
	if ((iop->publ.read_func == read_wrap) && tmout)
		iop->publ.read_func = read_with_timeout;

	return tmout;
}

/*
 * read_with_timeout --- read with a timeout, return failure
 *	if no data is available within the timeout period.
 */

static ssize_t
read_with_timeout(fd_t fd, void *buf, size_t size)
{
#if ! defined(VMS)
	fd_set readfds;
	struct timeval tv;
#ifdef WINDOWS_NATIVE
	/*
	 * Only sockets can be read with a timeout.  Also, the FD_*
	 * macros work on SOCKET type, not on int file descriptors.
	 */
	SOCKET s = valid_socket(fd);

	if (s == INVALID_SOCKET)
		return read_wrap(fd, buf, size);
#else
	fd_t s = fd;
#endif

#ifdef WINDOWS_NATIVE
	(void) sizeof(int[1-2*!(sizeof(tv.tv_sec) == sizeof(long))]);
	(void) sizeof(int[1-2*!(sizeof(tv.tv_usec) == sizeof(long))]);
	tv.tv_sec = (long) (read_timeout / 1000u);
	tv.tv_usec = (long) (1000u * (read_timeout - 1000u * (unsigned long) tv.tv_sec));
#else
	tv.tv_sec = (time_t) (read_timeout / 1000u);
	tv.tv_usec = (suseconds_t) (1000u * (read_timeout - 1000u * tv.tv_sec));
#endif

	FD_ZERO(& readfds);
	FD_SET(s, & readfds);

	errno = 0;
	/*
	 * Note: the 1st arg of 'select' is ignored on MS-Windows, so
	 * it's not a mistake to pass fd+1 there, although we use
	 * sockets, not file descriptors.
	 */
	if (select(fd + 1, & readfds, NULL, NULL, & tv) < 0)
		return -1;

	if (FD_ISSET(s, & readfds))
		return read_wrap(fd, buf, size);
	/* else
		timed out */

	/* Set a meaningful errno */
#ifdef ETIMEDOUT
	errno = ETIMEDOUT;
#else
	errno = EAGAIN;
#endif
	return -1;
#else  /* VMS */
	return read_wrap(fd, buf, size);
#endif	/* VMS */
}

/*
 * read_wrap --- wrapper around OS-specific read() function.
 */

#ifdef WINDOWS_NATIVE
ssize_t
read_wrap(fd_t fd, void *buf, size_t size)
{
	SOCKET s = is_socket(fd) ? valid_socket(fd) : INVALID_SOCKET;
	ssize_t ret = 0;
	while (size > 0) {
		/* Cannot read more than INT_MAX at once.  */
		unsigned n = size <= INT_MAX ? (unsigned) size : INT_MAX;
		int x = s != INVALID_SOCKET ?
			recv(s, (char*) buf, (int) n, 0) :
			read(fd, buf, n);
		if (x < 0)
			return -1;
		ret += x;
		if ((unsigned) x < n)
			break;
		buf = (char*) buf + n;
		size -= n;
	}
	return ret;
}
#endif /* WINDOWS_NATIVE */

/*
 * Dummy pass through functions for default output.
 */

/* gawk_fwrite --- like fwrite */

size_t
gawk_fwrite(const void *buf, size_t size, size_t nmemb, awk_output_buf_t *outbuf)
{
#ifdef WINDOWS_NATIVE
	if (outbuf->socket_fd != INVALID_HANDLE)
		return socket_file_fwrite((socket_file_t*) outbuf->file,
					outbuf->socket_fd, buf, size, nmemb);
#endif
	return fwrite(buf, size, nmemb, (FILE*) outbuf->file);
}

/* gawk_fflush --- like fflush */

int
gawk_fflush(awk_output_buf_t *outbuf)
{
#ifdef WINDOWS_NATIVE
	if (outbuf->socket_fd != INVALID_HANDLE)
		return socket_file_fflush((socket_file_t*) outbuf->file,
					outbuf->socket_fd);
#endif
	return fflush((FILE*) outbuf->file);
}

/* gawk_ferror --- like ferror */

int
gawk_ferror(awk_output_buf_t *outbuf)
{
#ifdef WINDOWS_NATIVE
	if (outbuf->socket_fd != INVALID_HANDLE)
		return socket_file_ferror((socket_file_t*) outbuf->file);
#endif
	return ferror((FILE*) outbuf->file);
}

/* gawk_fclose --- like fclose */

static int
gawk_fclose(awk_output_buf_t *outbuf)
{
#ifdef WINDOWS_NATIVE
	if (outbuf->socket_fd != INVALID_HANDLE) {
		socket_file_free((socket_file_t*) outbuf->file);
		return closemaybesocket(outbuf->socket_fd);
	}
#endif

	return fclose((FILE*) outbuf->file);
}

/* init_output_wrapper --- initialize the output wrapper */

static void
init_output_wrapper(awk_output_buf_t *outbuf)
{
	outbuf->name = NULL;
	outbuf->mode = NULL;
	outbuf->file = NULL;
#ifdef WINDOWS_NATIVE
	outbuf->socket_fd = INVALID_HANDLE;
#endif
	outbuf->redirected = awk_false;
	outbuf->opaque = NULL;
	outbuf->gawk_fwrite = gawk_fwrite;
	outbuf->gawk_fflush = gawk_fflush;
	outbuf->gawk_ferror = gawk_ferror;
	outbuf->gawk_fclose = gawk_fclose;
}
