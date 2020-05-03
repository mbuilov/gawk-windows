/*	$NetBSD: fts.c,v 1.44 2012/03/14 00:25:19 christos Exp $	*/

/*-
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Michael M. Builov
 * mbuilov@gmail.com
 * Reworked, fixed memory leaks, ported to _MSC_VER 4/2020
 */

#if 0
static char sccsid[] = "@(#)fts.c	8.6 (Berkeley) 8/14/94";
#else
/* __RCSID("$NetBSD: fts.c,v 1.44 2012/03/14 00:25:19 christos Exp $"); */
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _BSD_SOURCE

#include <limits.h>
/* #include "namespace.h" */
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#else
#include <stdio.h>
#endif
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#elif defined _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h> /* _chdir */
#include "xstat.h"
#include "wreaddir.h"
#else
#error Cannot compile the gawkfts extension on this system!
#endif

#include "gawkapi.h"
#include "gawkfts.h"
#include "gawkdirfd.h"

#if ! defined(S_ISREG) && defined(S_IFREG)
#define	S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

/*
#if ! HAVE_NBTOOL_CONFIG_H
#define	HAVE_STRUCT_DIRENT_D_NAMLEN
#endif
*/

#ifndef MAX
static size_t MAX(size_t x, size_t y)
{
	if (x > y)
		return x;

	return y;
}
#endif

static FTSENT	*fts_alloc(FTS *, const char *, size_t);
static FTSENT	*fts_build(FTS *, int);
static void	 fts_free(FTSENT *);
static void	 fts_lfree(FTSENT *);
static void	 fts_load(FTS *, FTSENT *);
static size_t	 fts_maxarglen(char * const *);
static size_t	 fts_pow2(size_t);
static int	 fts_palloc(FTS *, size_t);
static void	 fts_padjust(FTS *, FTSENT *);
static FTSENT	*fts_sort(FTS *, FTSENT *, size_t);
static unsigned short fts_stat(FTS *, FTSENT *, int);
static int	 fts_safe_changedir(const FTS *, const FTSENT *,
    const fts_dir_fd_t, const char *);

/* Bad value of fts_dir_fd_t.  */
#ifndef _MSC_VER
#define BAD_FD -1
#else
#define BAD_FD NULL
#endif

#ifdef FTS_ALIGN_BY
static size_t
align_by(size_t len, size_t by)
{
	/* Assume BY is a power of the two.  */
	return ((len + (by - 1)) & ~(by - 1)) - len;
}

typedef int check_FTS_ALIGN_BY_is_power_of_two[
	1-2*!!(((FTS_ALIGN_BY) - 1) & (FTS_ALIGN_BY))];

#define	FTS_ALLOC_ALIGNED	1
#else
#undef	FTS_ALLOC_ALIGNED
#endif

#ifndef ftsent_namelen_truncate
#define ftsent_namelen_truncate(a)	\
    ((a) > UINT_MAX ? UINT_MAX : (unsigned int)(a))
#endif
#ifndef ftsent_pathlen_truncate
#define ftsent_pathlen_truncate(a) \
    ((a) > UINT_MAX ? UINT_MAX : (unsigned int)(a))
#endif
#ifndef fts_pathsize_truncate
#define fts_pathsize_truncate(a)	\
    ((a) > UINT_MAX ? UINT_MAX : (unsigned int)(a))
#endif
#ifndef fts_nitems_truncate
#define fts_nitems_truncate(a) \
    ((a) > UINT_MAX ? UINT_MAX : (unsigned int)(a))
#endif

#define	ISDOT(a)	(a[0] == '.' && (!a[1] || (a[1] == '.' && !a[2])))
#define	ISWDOT(a)	(a[0] == L'.' && (!a[1] || (a[1] == L'.' && !a[2])))

#define	CLR(opt)	(sp->fts_options &= ~(opt))
#define	ISSET(opt)	(sp->fts_options & (opt))
#define	SET(opt)	(sp->fts_options |= (opt))

#define	CHDIR(sp, path)	(!ISSET(FTS_NOCHDIR) && chdir(path))
#define	FCHDIR(sp, dfd)	(!ISSET(FTS_NOCHDIR) && fchdir(dfd))

#ifndef _MSC_VER

#define xstat(path, buf)	stat(path, buf)
#define xlstat(path, buf)	lstat(path, buf)

#define opencurdfd(flags, dfd)	(*(dfd) = open(".", flags, 0))
#define closedirfd(dfd)		close(dfd)

#ifdef DT_DIR
#define dp_maybe_dir(dp) \
	((dp)->d_type == DT_DIR || (dp)->d_type == DT_UNKNOWN)
#else
#define dp_maybe_dir(dp)	1
#endif

#define dp_release(dp)		((void) 0)

#else /* _MSC_VER */

#define opencurdfd(flags, dfd)	((void)(flags), opencurdirfd(dfd))
#define dp_maybe_dir(dp) \
	((dp)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)

#ifndef FTS_PATHSEP
#define FTS_PATHSEP	'\\'
#endif

#endif /* _MSC_VER */

#ifndef FTS_PATHSEP
#define FTS_PATHSEP	'/'
#endif

/* fts_build flags */
#define	BCHILD		1		/* fts_children */
#define	BNAMES		2		/* fts_children, names only */
#define	BREAD		3		/* fts_read */

#ifndef DTF_HIDEW
#undef FTS_WHITEOUT
#endif

#define _DIAGASSERT(expression)

#ifndef MAXPATHLEN
#define MAXPATHLEN	1024	/* a guess */
#endif

/* Redefine bad names:

  FTS::fts_pathlen      includes terminating '\0', but
  FTSENT::fts_pathlen   do not counts terminating '\0'

  FTS::fts_path         path for this descent
  FTSENT::fts_path      root path
*/

#define fts_pathsize	fts_pathlen
#define fts_rpath	fts_path
#define fts_rpathlen	fts_pathlen

FTS *
fts_open(char * const *argv, int options,
    int (*compar)(const FTSENT **, const FTSENT **))
{
	FTS *sp;
	FTSENT *p, *root;
	size_t nitems;
	FTSENT *parent, **tail = &root;
	size_t len;

	_DIAGASSERT(argv != NULL);

	/* Options check. */
	if (options & ~FTS_OPTIONMASK) {
		errno = EINVAL;
		return (NULL);
	}

	/* Allocate/initialize the stream */
	if ((sp = (FTS *) calloc(1, (unsigned int)sizeof(FTS))) == NULL)
		return (NULL);
	sp->fts_compar = compar;
	sp->fts_options = options;

	/* Logical walks turn on NOCHDIR; symbolic links are too hard. */
	if (ISSET(FTS_LOGICAL))
		SET(FTS_NOCHDIR);

	/*
	 * Start out with 1K of path space, and enough, in any case,
	 * to hold the user's paths.
	 */
	if (fts_palloc(sp, MAX(fts_maxarglen(argv), MAXPATHLEN)))
		goto mem1;

	/* Allocate/initialize root's parent. */
	if ((parent = fts_alloc(sp, "", 0)) == NULL)
		goto mem2;
	parent->fts_level = FTS_ROOTPARENTLEVEL;

	/* Allocate/initialize root(s). */
	for (nitems = 0; *argv; ++argv, ++nitems) {
		/* Don't allow zero-length paths. */
		if ((len = strlen(*argv)) == 0) {
			errno = ENOENT;
			goto mem3;
		}

		if ((p = fts_alloc(sp, *argv, len)) == NULL)
			goto mem3;
		p->fts_level = FTS_ROOTLEVEL;
		p->fts_parent = parent;
		p->fts_accpath = p->fts_name;
		p->fts_info = fts_stat(sp, p, ISSET(FTS_COMFOLLOW));

		/* Command-line "." and ".." are real directories. */
		if (p->fts_info == FTS_DOT)
			p->fts_info = FTS_D;

		/*
		 * If comparison routine supplied, traverse in sorted
		 * order; otherwise traverse in the order specified.
		 */
		*tail = p;
		tail = &p->fts_link;
	}
	*tail = NULL;

	if (compar && nitems > 1)
		root = fts_sort(sp, root, nitems);

	/*
	 * Allocate a dummy pointer and make fts_read think that we've just
	 * finished the node before the root(s); set p->fts_info to FTS_INIT
	 * so that everything about the "current" node is ignored.
	 */
	if ((sp->fts_cur = fts_alloc(sp, "", 0)) == NULL)
		goto mem4;
	sp->fts_cur->fts_link = root;
	sp->fts_cur->fts_info = FTS_INIT;

	/*
	 * If using chdir(2), grab a file descriptor pointing to dot to insure
	 * that we can get back here; this could be avoided for some paths,
	 * but almost certainly not worth the effort.  Slashes, symbolic links,
	 * and ".." are all fairly nasty problems.  Note, if we can't get the
	 * descriptor we run anyway, just more slowly.
	 */
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
	if (!ISSET(FTS_NOCHDIR)) {
		if (opencurdfd(O_RDONLY | O_CLOEXEC, &sp->fts_rfd) == -1)
			SET(FTS_NOCHDIR);
	}

	if (nitems == 0)
		fts_free(parent);

	return (sp);

mem4:	if (sp->fts_array)
		free(sp->fts_array);
mem3:	*tail = NULL;
	fts_lfree(root);
	fts_free(parent);
mem2:	free(sp->fts_path);
mem1:	free(sp);
	return (NULL);
}

static int
is_slash(char c)
{
	if (c == '/')
		return 1;
#ifdef _MSC_VER
	if (c == '\\')
		return 1;
#endif
	return 0;
}

static int
is_alpha(char c)
{
	return
		('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z');
}

static int
is_abs_path(const char *path)
{
#ifndef _MSC_VER
	return *path == '/';
#else
	/* Check for "C:/" */
	if (path[0] && path[1] == ':' && is_slash(path[2]) && is_alpha(path[0]))
		return 1;
	/* Check for "//server/share/" */
	if (is_slash(path[0]) && is_slash(path[1]) && !is_slash(path[2])) {
		const char *p = path + 2;

		do {
			if (*p == '\0')
				return 0; /* "//" or "//abc" */
		} while (!is_slash(*++p));

		while (is_slash(*++p));

		if (*p == '\0')
			return 0; /* "//x/" or "//x//" */

		do {
			if (is_slash(*++p))
				return 1; /* "//x/y/" or "//x//y/" */
		} while (*p != '\0');

		/* "//x/y" or "//x//y" */
	}
	return 0;
#endif
}

static char *
rfind_slash(char *path)
{
	char *cp = strrchr(path, '/');
#ifdef _MSC_VER
	char *cp2 = strrchr(path, '\\');
	if (cp2 != NULL && (cp == NULL || cp < cp2))
		cp = cp2;
#endif
	return cp;
}

static char *
get_name_after_slash(char *path)
{
	char *const cp = rfind_slash(path);
	if (cp == NULL)
		return NULL;
	if (cp[1])
		return cp;

#ifndef _MSC_VER
	/* Exclude root path: "/" */
	if (cp == path)
		return NULL;
#else
	/* Exclude root path: "C:/" */
	if (cp == path + 2 && path[1] == ':' && is_alpha(path[0]))
		return NULL;
	/* Exclude root path: "//x/y/" */
	if (cp >= path + 5 && is_slash(path[0]) && is_slash(path[1])
		&& !is_slash(path[2]))
	{
		const char *p = cp;
		if (!is_slash(*--p)) {
			while (!is_slash(*--p));

			if (p >= path + 3 && !is_slash(*--p)) {
				while (!is_slash(*--p));

				if (p == path + 1)
					return NULL;
			}
		}
	}
#endif
	return cp;
}

static void
fts_load(FTS *sp, FTSENT *p)
{
	size_t len;
	char *cp;

	_DIAGASSERT(sp != NULL);
	_DIAGASSERT(p != NULL);

	/*
	 * Load the stream structure for the next traversal.  Since we don't
	 * actually enter the directory until after the preorder visit, set
	 * the fts_accpath field specially so the chdir gets done to the right
	 * place and the user can access the first node.  From fts_open it's
	 * known that the path will fit.
	 */
	len = p->fts_rpathlen = p->fts_namelen;
	memcpy(sp->fts_path, p->fts_name, len + 1);
	cp = get_name_after_slash(p->fts_name);
	if (cp != NULL) {
		len = strlen(++cp);
		memmove(p->fts_name, cp, len + 1);
		p->fts_namelen = ftsent_namelen_truncate(len);
	}
	p->fts_accpath = p->fts_rpath = sp->fts_path;
	sp->fts_dev = p->fts_dev;
}

int
fts_close(FTS *sp)
{
	FTSENT *freep, *p;
	int saved_errno = 0;

	_DIAGASSERT(sp != NULL);

	/*
	 * This still works if we haven't read anything -- the dummy structure
	 * points to the root list, so we step through to the end of the root
	 * list which has a valid parent pointer.
	 */
	if (sp->fts_cur) {
		if (sp->fts_cur->fts_flags & FTS_SYMFOLLOW)
			(void)closedirfd(sp->fts_cur->fts_symfd);
		for (p = sp->fts_cur; p->fts_level >= FTS_ROOTLEVEL;) {
			freep = p;
			p = p->fts_link ? p->fts_link : p->fts_parent;
			fts_free(freep);
		}
		fts_free(p);
	}

	/* Free up child linked list, sort array, path buffer. */
	if (sp->fts_child)
		fts_lfree(sp->fts_child);
	if (sp->fts_array)
		free(sp->fts_array);
	free(sp->fts_path);

	/* Return to original directory, save errno if necessary. */
	if (!ISSET(FTS_NOCHDIR)) {
		if (fchdir(sp->fts_rfd) == -1)
			saved_errno = errno;
		(void)closedirfd(sp->fts_rfd);
	}

	/* Free up the stream pointer. */
	free(sp);
	if (saved_errno) {
		errno = saved_errno;
		return -1;
	}

	return 0;
}

#ifndef __FTS_COMPAT_TAILINGSLASH

/*
 * Special case of "/" at the end of the path so that slashes aren't
 * appended which would cause paths to be written as "....//foo".
 */
#define	NAPPEND(p)							\
	(is_slash(p->fts_rpath[p->fts_rpathlen - 1])			\
	    ? p->fts_rpathlen - 1 : p->fts_rpathlen)

#else /* __FTS_COMPAT_TAILINGSLASH */

/*
 * compatibility with the old behaviour.
 *
 * Special case a root of "/" so that slashes aren't appended which would
 * cause paths to be written as "//foo".
 */

#define	NAPPEND(p)							\
	(p->fts_level == FTS_ROOTLEVEL && p->fts_rpathlen == 1 &&	\
	    p->fts_rpath[0] == '/' ? 0 : p->fts_rpathlen)

#endif /* __FTS_COMPAT_TAILINGSLASH */

FTSENT *
fts_read(FTS *sp)
{
	FTSENT *p, *tmp;
	int instr;
	char *t;
	int saved_errno;

	_DIAGASSERT(sp != NULL);

	/* If finished or unrecoverable error, return NULL. */
	if (sp->fts_cur == NULL || ISSET(FTS_STOP))
		return (NULL);

	/* Set current node pointer. */
	p = sp->fts_cur;

	/* Save and zero out user instructions. */
	instr = p->fts_instr;
	p->fts_instr = FTS_NOINSTR;

	/* Any type of file may be re-visited; re-stat and re-turn. */
	if (instr == FTS_AGAIN) {
		p->fts_info = fts_stat(sp, p, 0);
		return (p);
	}

	/*
	 * Following a symlink -- SLNONE test allows application to see
	 * SLNONE and recover.  If indirecting through a symlink, have
	 * keep a pointer to current location.  If unable to get that
	 * pointer, follow fails.
	 */
	if (instr == FTS_FOLLOW &&
	    (p->fts_info == FTS_SL || p->fts_info == FTS_SLNONE)) {
		p->fts_info = fts_stat(sp, p, 1);
		if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR)) {
			if (opencurdfd(O_RDONLY | O_CLOEXEC, &p->fts_symfd)
			    == -1) {
				p->fts_errno = errno;
				p->fts_info = FTS_ERR;
			} else
				p->fts_flags |= FTS_SYMFOLLOW;
		}
		return (p);
	}

	/* Directory in pre-order. */
	if (p->fts_info == FTS_D) {
		/* If skipped or crossed mount point, do post-order visit. */
		if (instr == FTS_SKIP ||
		    (ISSET(FTS_XDEV) && p->fts_dev != sp->fts_dev)) {
			if (p->fts_flags & FTS_SYMFOLLOW)
				(void)closedirfd(p->fts_symfd);
			if (sp->fts_child) {
				fts_lfree(sp->fts_child);
				sp->fts_child = NULL;
			}
			p->fts_info = FTS_DP;
			return (p);
		}

		/* Rebuild if only read the names and now traversing. */
		if (sp->fts_child && ISSET(FTS_NAMEONLY)) {
			CLR(FTS_NAMEONLY);
			fts_lfree(sp->fts_child);
			sp->fts_child = NULL;
		}

		/*
		 * Cd to the subdirectory.
		 *
		 * If have already read and now fail to chdir, whack the list
		 * to make the names come out right, and set the parent errno
		 * so the application will eventually get an error condition.
		 * Set the FTS_DONTCHDIR flag so that when we logically change
		 * directories back to the parent we don't do a chdir.
		 *
		 * If haven't read do so.  If the read fails, fts_build sets
		 * FTS_STOP or the fts_info field of the node.
		 */
		if (sp->fts_child) {
			if (fts_safe_changedir(sp, p, BAD_FD, p->fts_accpath)) {
				p->fts_errno = errno;
				p->fts_flags |= FTS_DONTCHDIR;
				for (p = sp->fts_child; p; p = p->fts_link)
					p->fts_accpath =
					    p->fts_parent->fts_accpath;
			}
		} else if ((sp->fts_child = fts_build(sp, BREAD)) == NULL) {
			if (ISSET(FTS_STOP))
				return (NULL);
			return (p);
		}
		p = sp->fts_child;
		sp->fts_child = NULL;
		goto name;
	}

	/* Move to the next node on this level. */
next:	tmp = p;
	if ((p = p->fts_link) != NULL) {
		fts_free(tmp);

		/*
		 * If reached the top, return to the original directory, and
		 * load the paths for the next root.
		 */
		if (p->fts_level == FTS_ROOTLEVEL) {
			if (FCHDIR(sp, sp->fts_rfd)) {
				SET(FTS_STOP);
				return (NULL);
			}
			fts_load(sp, p);
			return (sp->fts_cur = p);
		}

		/*
		 * User may have called fts_set on the node.  If skipped,
		 * ignore.  If followed, get a file descriptor so we can
		 * get back if necessary.
		 */
		if (p->fts_instr == FTS_SKIP)
			goto next;
		if (p->fts_instr == FTS_FOLLOW) {
			p->fts_info = fts_stat(sp, p, 1);
			if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR)) {
				if (opencurdfd(O_RDONLY | O_CLOEXEC,
						 &p->fts_symfd) == -1) {
					p->fts_errno = errno;
					p->fts_info = FTS_ERR;
				} else
					p->fts_flags |= FTS_SYMFOLLOW;
			}
			p->fts_instr = FTS_NOINSTR;
		}

name:		t = sp->fts_path + NAPPEND(p->fts_parent);
		*t++ = FTS_PATHSEP;
		memcpy(t, p->fts_name, (size_t)(p->fts_namelen + 1));
		return (sp->fts_cur = p);
	}

	/* Move up to the parent node. */
	p = tmp->fts_parent;
	fts_free(tmp);

	if (p->fts_level == FTS_ROOTPARENTLEVEL) {
		/*
		 * Done; free everything up and set errno to 0 so the user
		 * can distinguish between error and EOF.
		 */
		fts_free(p);
		errno = 0;
		return (sp->fts_cur = NULL);
	}

	/* Nul terminate the pathname. */
	sp->fts_path[p->fts_rpathlen] = '\0';

	/*
	 * Return to the parent directory.  If at a root node or came through
	 * a symlink, go back through the file descriptor.  Otherwise, cd up
	 * one directory.
	 */
	if (p->fts_level == FTS_ROOTLEVEL) {
		if (FCHDIR(sp, sp->fts_rfd)) {
			SET(FTS_STOP);
			return (NULL);
		}
	} else if (p->fts_flags & FTS_SYMFOLLOW) {
		if (FCHDIR(sp, p->fts_symfd)) {
			saved_errno = errno;
			(void)closedirfd(p->fts_symfd);
			errno = saved_errno;
			SET(FTS_STOP);
			return (NULL);
		}
		(void)closedirfd(p->fts_symfd);
	} else if (!(p->fts_flags & FTS_DONTCHDIR) &&
	    fts_safe_changedir(sp, p->fts_parent, BAD_FD, "..")) {
		SET(FTS_STOP);
		return (NULL);
	}
	p->fts_info = (unsigned short) (p->fts_errno ? FTS_ERR : FTS_DP);
	return (sp->fts_cur = p);
}

/*
 * Fts_set takes the stream as an argument although it's not used in this
 * implementation; it would be necessary if anyone wanted to add global
 * semantics to fts using fts_set.  An error return is allowed for similar
 * reasons.
 */
/* ARGSUSED */
int
fts_set(FTS *sp, FTSENT *p, int instr)
{
	(void) sp;	/* silence warnings */
	_DIAGASSERT(sp != NULL);
	_DIAGASSERT(p != NULL);

	if (instr && instr != FTS_AGAIN && instr != FTS_FOLLOW &&
	    instr != FTS_NOINSTR && instr != FTS_SKIP) {
		errno = EINVAL;
		return (1);
	}
	p->fts_instr = (unsigned short) instr;
	return (0);
}

FTSENT *
fts_children(FTS *sp, int instr)
{
	FTSENT *p;
	fts_dir_fd_t fd;

	_DIAGASSERT(sp != NULL);

	if (instr && instr != FTS_NAMEONLY) {
		errno = EINVAL;
		return (NULL);
	}

	/* Set current node pointer. */
	p = sp->fts_cur;

	/*
	 * Errno set to 0 so user can distinguish empty directory from
	 * an error.
	 */
	errno = 0;

	/* Fatal errors stop here. */
	if (ISSET(FTS_STOP))
		return (NULL);

	/* Return logical hierarchy of user's arguments. */
	if (p->fts_info == FTS_INIT)
		return (p->fts_link);

	/*
	 * If not a directory being visited in pre-order, stop here.  Could
	 * allow FTS_DNR, assuming the user has fixed the problem, but the
	 * same effect is available with FTS_AGAIN.
	 */
	if (p->fts_info != FTS_D /* && p->fts_info != FTS_DNR */)
		return (NULL);

	/* Free up any previous child list. */
	if (sp->fts_child)
		fts_lfree(sp->fts_child);

	if (instr == FTS_NAMEONLY) {
		SET(FTS_NAMEONLY);
		instr = BNAMES;
	} else
		instr = BCHILD;

	/*
	 * If using chdir on a relative path and called BEFORE fts_read does
	 * its chdir to the root of a traversal, we can lose -- we need to
	 * chdir into the subdirectory, and we don't know where the current
	 * directory is, so we can't get back so that the upcoming chdir by
	 * fts_read will work.
	 */
	if (p->fts_level != FTS_ROOTLEVEL || is_abs_path(p->fts_accpath) ||
	    ISSET(FTS_NOCHDIR))
		return (sp->fts_child = fts_build(sp, instr));

	if (opencurdfd(O_RDONLY, &fd) == -1)
		return (sp->fts_child = NULL);
	sp->fts_child = fts_build(sp, instr);
	if (fchdir(fd)) {
		(void)closedirfd(fd);
		return (NULL);
	}
	(void)closedirfd(fd);
	return (sp->fts_child);
}

#ifdef _MSC_VER
static size_t
convert_dname(const wchar_t filename[], char **dname,
	char **dyn_dname, size_t *dname_space)
{
	for (;;) {
		size_t dnamlen = wcstombs(*dname, filename, *dname_space);
		if ((size_t)-1 == dnamlen) {
			errno = EILSEQ;
			return (size_t)-1;
		}

		/* Make sure that conversion wasn't stopped due to small buffer.  */
		/* Also make sure that terminating null byte was written.  */
		/* Note: (*dname_space) > 10.  */
		if (dnamlen < *dname_space - 10/*MB_CUR_MAX*/)
			return dnamlen;

		dnamlen = wcstombs(NULL, filename, 0);
		if ((size_t)-1 == dnamlen) {
			errno = EILSEQ;
			return (size_t)-1;
		}

		if (*dyn_dname != NULL)
			free(*dyn_dname);

		*dyn_dname = (char*) malloc(dnamlen + 1 + 10/*MB_CUR_MAX*/);
		if (*dyn_dname == NULL)
			return (size_t)-1;

		*dname = *dyn_dname;
		*dname_space = dnamlen + 1 + 10/*MB_CUR_MAX*/;
	}
}
#endif /* _MSC_VER */

/*
 * This is the tricky part -- do not casually change *anything* in here.  The
 * idea is to build the linked list of entries that are used by fts_children
 * and fts_read.  There are lots of special cases.
 *
 * The real slowdown in walking the tree is the stat calls.  If FTS_NOSTAT is
 * set and it's a physical walk (so that symbolic links can't be directories),
 * we can do things quickly.  First, if it's a 4.4BSD file system, the type
 * of the file is in the directory entry.  Otherwise, we assume that the number
 * of subdirectories in a node is equal to the number of links to the parent.
 * The former skips all stat calls.  The latter skips stat calls in any leaf
 * directories and for any files after the subdirectories in the directory have
 * been found, cutting the stat calls by about 2/3.
 */
static FTSENT *
fts_build(FTS *sp, const int type)
{
	FTSENT *p, *cur, *head, **tail = &head;
	size_t nitems;
#ifndef _MSC_VER
	struct dirent *dp;
	DIR *dirp;
	char *dname;
# define DP_ISDOT(dp)	ISDOT((dp)->d_name)
# define FREE_DYN_DNAME	(void)0
#else
	WIN32_FIND_DATAW *dp;
	struct win_find *dirp;
	char nambuf[MAX_PATH*4/*MB_CUR_MAX*/ + 10], *dname = nambuf;
	size_t dname_space = sizeof(nambuf);
	char *dyn_dname = NULL;
# define DP_ISDOT(dp)	ISWDOT((dp)->cFileName)
# define FREE_DYN_DNAME	if (dyn_dname != NULL) free(dyn_dname)
#endif
	void *oldaddr;
	int cderrno, descend, level, nlinks, saved_errno, needstat, doadjust;
	size_t len, dnamlen, maxsize; /* maxsize includes space for NUL */
#ifdef FTS_WHITEOUT
	int oflag;
#endif
	char *cp = NULL;	/* pacify gcc */

	_DIAGASSERT(sp != NULL);

	/* Set current node pointer. */
	cur = sp->fts_cur;

	/*
	 * Open the directory for reading.  If this fails, we're done.
	 * If being called from fts_read, set the fts_info field.
	 */
#if 0 /* def FTS_WHITEOUT */
	if (ISSET(FTS_WHITEOUT))
		oflag = DTF_NODUP|DTF_REWIND;
	else
		oflag = DTF_HIDEW|DTF_NODUP|DTF_REWIND;
#else
#define	__opendir2(path, flag) opendir(path)
#endif
	if ((dirp = __opendir2(cur->fts_accpath, oflag)) == NULL) {
		if (type == BREAD) {
			cur->fts_info = FTS_DNR;
			cur->fts_errno = errno;
		}
		return (NULL);
	}

	/*
	 * Nlinks is the number of possible entries of type directory in the
	 * directory if we're cheating on stat calls, 0 if we're not doing
	 * any stat calls at all, -1 if we're doing stats on everything.
	 */
	if (type == BNAMES) {
		/* Don't stat.  */
		nlinks = 0;
		needstat = 0;
	} else if (ISSET(FTS_NOSTAT) && ISSET(FTS_PHYSICAL)) {
#if defined(FTS_DIRNLINK_SUPPORTED) && FTS_DIRNLINK_SUPPORTED
		/* Assume: if the underlying FS does not support st_nlink info
		   for the directories, it sets st_nlink to 0, 1 or some big
		   value, so we may stat everything.  */
		nlinks = cur->fts_nlink <= 1 ? -1 :
			cur->fts_nlink - (ISSET(FTS_SEEDOT) ? 0 : 2);
#else
		/* May stat everything.  */
		nlinks = -1;
#endif
		/* Don't stat if readdir(3) fills d_type so that it will be
		   possible to determine if the entry is a directory or not.  */
		needstat = 0;
	} else {
		/* Do stat everything.  */
		nlinks = -1;
		needstat = 1;
	}

#ifdef notdef
	(void)printf("nlinks == %d (cur: %d)\n", nlinks, cur->fts_nlink);
	(void)printf("NOSTAT %d PHYSICAL %d SEEDOT %d\n",
	    ISSET(FTS_NOSTAT), ISSET(FTS_PHYSICAL), ISSET(FTS_SEEDOT));
#endif
	/*
	 * If we're going to need to stat anything or we want to descend
	 * and stay in the directory, chdir.  If this fails we keep going,
	 * but set a flag so we don't chdir after the post-order visit.
	 * We won't be able to stat anything, but we can still return the
	 * names themselves.  Note, that since fts_read won't be able to
	 * chdir into the directory, it will have to return different path
	 * names than before, i.e. "a/b" instead of "b".  Since the node
	 * has already been visited in pre-order, have to wait until the
	 * post-order visit to return the error.  There is a special case
	 * here, if there was nothing to stat then it's not an error to
	 * not be able to stat.  This is all fairly nasty.  If a program
	 * needed sorted entries or stat information, they had better be
	 * checking FTS_NS on the returned nodes.
	 */
	cderrno = 0;
	if (nlinks || type == BREAD) {
		if (fts_safe_changedir(sp, cur, dirfd(dirp), NULL)) {
			if (nlinks && type == BREAD)
				cur->fts_errno = errno;
			cur->fts_flags |= FTS_DONTCHDIR;
			descend = 0;
			cderrno = errno;
		} else
			descend = 1;
	} else
		descend = 0;

	/*
	 * Figure out the max file name length that can be stored in the
	 * current path -- the inner loop allocates more path as necessary.
	 * We really wouldn't have to do the maxsize calculations here, we
	 * could do them in fts_read before returning the path, but it's a
	 * lot easier here since the length is part of the dirent structure.
	 *
	 * If not changing directories set a pointer so that can just append
	 * each new name into the path.
	 */
	len = NAPPEND(cur);
	if (ISSET(FTS_NOCHDIR)) {
		cp = sp->fts_path + len;
		*cp++ = FTS_PATHSEP;
	}
	len++;
	maxsize = sp->fts_pathsize - len;

#if defined(__FTS_COMPAT_LEVEL)
	if (cur->fts_level == SHRT_MAX) {
		(void)closedir(dirp);
		cur->fts_info = FTS_ERR;
		SET(FTS_STOP);
		errno = ENAMETOOLONG;
		return (NULL);
	}
#endif

	level = cur->fts_level + 1;

	/* Read the directory, attaching each entry to the `link' pointer. */
	doadjust = 0;
	for (nitems = 0; (dp = readdir(dirp)) != NULL; dp_release(dp)) {

		if (!ISSET(FTS_SEEDOT) && DP_ISDOT(dp))
			continue;

#ifndef _MSC_VER
# ifdef HAVE_STRUCT_DIRENT_D_NAMLEN
		dnamlen = dp->d_namlen;
# else
		dnamlen = strlen(dp->d_name);
# endif
		dname = dp->d_name;
#else
		dnamlen = convert_dname(dp->cFileName, &dname,
			&dyn_dname, &dname_space);
		if ((size_t)-1 == dnamlen)
			goto mem1;
#endif

		if ((p = fts_alloc(sp, dname, dnamlen)) == NULL)
			goto mem1;

		/* We walk in directory order so "ls -f" doesn't get upset. */
		*tail = p;
		tail = &p->fts_link;

		if (dnamlen >= maxsize) {	/* include space for NUL */
			oldaddr = sp->fts_path;
			if (fts_palloc(sp, dnamlen + len + 1)) {
				/*
				 * No more memory for path or structures.  Save
				 * errno, free up the current structure and the
				 * structures already allocated.
				 */
mem1:				saved_errno = errno;
				FREE_DYN_DNAME;
				*tail = NULL;
				fts_lfree(head);
				(void)closedir(dirp);
				cur->fts_info = FTS_ERR;
				SET(FTS_STOP);
				errno = saved_errno;
				return (NULL);
			}
			/* Did realloc() change the pointer? */
			if (oldaddr != sp->fts_path) {
				doadjust = 1;
				if (ISSET(FTS_NOCHDIR))
					cp = sp->fts_path + len;
			}
			maxsize = sp->fts_pathsize - len;
		}

#if defined(__FTS_COMPAT_LENGTH)
		if (len + dnamlen >= USHRT_MAX) {
			/*
			 * In an FTSENT, fts_rpathlen is an unsigned short
			 * so it is possible to wraparound here.
			 * If we do, free up the current structure and the
			 * structures already allocated, then error out
			 * with ENAMETOOLONG.
			 */
			FREE_DYN_DNAME;
			*tail = NULL;
			fts_lfree(head);
			(void)closedir(dirp);
			cur->fts_info = FTS_ERR;
			SET(FTS_STOP);
			errno = ENAMETOOLONG;
			return (NULL);
		}
#endif
		p->fts_level = level;
		p->fts_rpathlen = ftsent_pathlen_truncate(len + dnamlen);
		p->fts_parent = sp->fts_cur;

#ifdef FTS_WHITEOUT
		if (dp->d_type == DT_WHT)
			p->fts_flags |= FTS_ISW;
#endif

		if (cderrno) {
			if (nlinks) {
				p->fts_info = FTS_NS;
				p->fts_errno = cderrno;
			} else
				p->fts_info = FTS_NSOK;
			p->fts_accpath = cur->fts_accpath;
		} else if (nlinks == 0 || (!needstat && !dp_maybe_dir(dp))) {
			p->fts_accpath =
			    ISSET(FTS_NOCHDIR) ? p->fts_rpath : p->fts_name;
			p->fts_info = FTS_NSOK;
		} else {
			/* Build a file name for fts_stat to stat. */
			if (ISSET(FTS_NOCHDIR)) {
				p->fts_accpath = p->fts_rpath;
				memcpy(cp, p->fts_name,
				        (size_t)(p->fts_namelen + 1));
			} else
				p->fts_accpath = p->fts_name;
			/* Stat it. */
			p->fts_info = fts_stat(sp, p, 0);

			/* Decrement link count if applicable. */
			if (nlinks > 0 && (p->fts_info == FTS_D ||
			    p->fts_info == FTS_DC || p->fts_info == FTS_DOT))
				--nlinks;
		}
		++nitems;
	}
	(void)closedir(dirp);
	FREE_DYN_DNAME;
	*tail = NULL;

	/*
	 * If had to realloc the path, adjust the addresses for the rest
	 * of the tree.
	 */
	if (doadjust)
		fts_padjust(sp, head);

	/*
	 * If not changing directories, reset the path back to original
	 * state.
	 */
	if (ISSET(FTS_NOCHDIR)) {
		if (len == sp->fts_pathsize || nitems == 0)
			--cp;
		*cp = '\0';
	}

	/*
	 * If descended after called from fts_children or after called from
	 * fts_read and nothing found, get back.  At the root level we use
	 * the saved fd; if one of fts_open()'s arguments is a relative path
	 * to an empty directory, we wind up here with no other way back.  If
	 * can't get back, we're done.
	 */
	if (descend && (type == BCHILD || !nitems) &&
	    (cur->fts_level == FTS_ROOTLEVEL ?
	    FCHDIR(sp, sp->fts_rfd) :
	    fts_safe_changedir(sp, cur->fts_parent, BAD_FD, ".."))) {
		fts_lfree(head);
		cur->fts_info = FTS_ERR;
		SET(FTS_STOP);
		return (NULL);
	}

	/* If didn't find anything, return NULL. */
	if (!nitems) {
		if (type == BREAD)
			cur->fts_info = FTS_DP;
		return (NULL);
	}

	/* Sort the entries. */
	if (sp->fts_compar && nitems > 1)
		head = fts_sort(sp, head, nitems);
	return (head);
#undef FREE_DYN_DNAME
#undef DP_ISDOT
}

static unsigned short
fts_stat(FTS *sp, FTSENT *p, int follow)
{
	FTSENT *t;
	fts_dev_t dev;
	fts_ino_t ino;
	fts_stat_t *sbp, sb;
	int saved_errno;

	_DIAGASSERT(sp != NULL);
	_DIAGASSERT(p != NULL);

	/* If user needs stat info, stat buffer already allocated. */
	sbp = ISSET(FTS_NOSTAT) ? &sb : p->fts_statp;

#ifdef FTS_WHITEOUT
	/* check for whiteout */
	if (p->fts_flags & FTS_ISW) {
		if (sbp != &sb) {
			memset(sbp, '\0', sizeof (*sbp));
			sbp->st_mode = S_IFWHT;
		}
		return (FTS_W);
	}
#endif

	/*
	 * If doing a logical walk, or application requested FTS_FOLLOW, do
	 * a stat(2).  If that fails, check for a non-existent symlink.  If
	 * fail, set the errno from the stat call.
	 */
	if (ISSET(FTS_LOGICAL) || follow) {
		if (xstat(p->fts_accpath, sbp)) {
			saved_errno = errno;
			if (!xlstat(p->fts_accpath, sbp)) {
				errno = 0;
				return (FTS_SLNONE);
			}
			p->fts_errno = saved_errno;
			goto err;
		}
	} else if (xlstat(p->fts_accpath, sbp)) {
		p->fts_errno = errno;
		goto err;
	}

	if (S_ISDIR(sbp->st_mode)) {
		/*
		 * Set the device/inode.  Used to find cycles and check for
		 * crossing mount points.  Also remember the link count, used
		 * in fts_build to limit the number of stat calls.  It is
		 * understood that these fields are only referenced if fts_info
		 * is set to FTS_D.
		 */
		dev = p->fts_dev = sbp->st_dev;
		ino = p->fts_ino = sbp->st_ino;
#if defined(FTS_DIRNLINK_SUPPORTED) && FTS_DIRNLINK_SUPPORTED
		p->fts_nlink = sbp->st_nlink;
#endif

		if (ISDOT(p->fts_name))
			return (FTS_DOT);

		/*
		 * Cycle detection is done by brute force when the directory
		 * is first encountered.  If the tree gets deep enough or the
		 * number of symbolic links to directories is high enough,
		 * something faster might be worthwhile.
		 */
		for (t = p->fts_parent;
		    t->fts_level >= FTS_ROOTLEVEL; t = t->fts_parent)
			if (ino == t->fts_ino && dev == t->fts_dev) {
				p->fts_cycle = t;
				return (FTS_DC);
			}
		return (FTS_D);
	}
	if (S_ISLNK(sbp->st_mode))
		return (FTS_SL);
	if (S_ISREG(sbp->st_mode))
		return (FTS_F);
	return (FTS_DEFAULT);

err:
	memset(sbp, 0, sizeof(*sbp));
	return (FTS_NS);
}

static FTSENT *
fts_sort(FTS *sp, FTSENT *head, size_t nitems)
{
	FTSENT **ap, *p;

	_DIAGASSERT(sp != NULL);
	_DIAGASSERT(head != NULL);

	/*
	 * Construct an array of pointers to the structures and call qsort(3).
	 * Reassemble the array in the order returned by qsort.  If unable to
	 * sort for memory reasons, return the directory entries in their
	 * current order.  Allocate enough space for the current needs plus
	 * 40 so don't realloc one entry at a time.
	 */
	if (nitems > sp->fts_nitems) {
		FTSENT **new_arr;
		if (nitems > (size_t)-1/sizeof(FTSENT *) - 40)
			return (head);

		new_arr = (FTSENT**) realloc(sp->fts_array,
			sizeof(FTSENT *) * (nitems + 40));
		if (new_arr == NULL)
			return (head);
		sp->fts_array = new_arr;
		sp->fts_nitems = fts_nitems_truncate(nitems + 40);
	}
	for (ap = sp->fts_array, p = head; p; p = p->fts_link)
		*ap++ = p;
	qsort((void *)sp->fts_array, nitems, sizeof(FTSENT *),
		(int (*)(const void *, const void *))sp->fts_compar);
	for (head = *(ap = sp->fts_array); --nitems; ++ap)
		ap[0]->fts_link = ap[1];
	ap[0]->fts_link = NULL;
	return (head);
}

static FTSENT *
fts_alloc(FTS *sp, const char *name, size_t namelen)
{
	FTSENT *p;
	size_t len;

	_DIAGASSERT(sp != NULL);
	_DIAGASSERT(name != NULL);

#define FSTENT_HEAD_SIZE \
	((sizeof(FTSENT) - 1) & ~(sizeof(void*) - 1))

	/* check that FSTENT_HEAD_SIZE == offsetof(FTSENT, fts_name) */
	(void) sizeof(int[1-2*(FSTENT_HEAD_SIZE !=
		((char*) &((FTSENT*)NULL)->fts_name - (char*)NULL))]);

	len = FSTENT_HEAD_SIZE + namelen + 1;

#ifdef FTS_ALLOC_ALIGNED
	/*
	 * The file name is a variable length array and no stat structure is
	 * necessary if the user has set the nostat bit.  Allocate the FTSENT
	 * structure, the file name and the stat structure in one chunk, but
	 * be careful that the stat structure is reasonably aligned.
	 */
	if (!ISSET(FTS_NOSTAT))
		len += sizeof(*(p->fts_statp)) + align_by(len, FTS_ALIGN_BY);
#endif

	if ((p = (FTSENT *) malloc(len)) == NULL)
		return (NULL);

#ifdef FTS_ALLOC_ALIGNED
	if (!ISSET(FTS_NOSTAT))
		p->fts_statp = (fts_stat_t *)((char*) p
			+ len - sizeof(*(p->fts_statp)));
#else
	if (!ISSET(FTS_NOSTAT))
		if ((p->fts_statp = (fts_stat_t *) malloc(sizeof(*(p->fts_statp))))
			== NULL)
		{
			free(p);
			return (NULL);
		}
#endif

#undef FSTENT_HEAD_SIZE

	if (ISSET(FTS_NOSTAT))
		p->fts_statp = NULL;

	/* Copy the name plus the trailing NULL. */
	memcpy(p->fts_name, name, namelen + 1);

	p->fts_namelen = ftsent_namelen_truncate(namelen);
	p->fts_rpath = sp->fts_path;
	p->fts_errno = 0;
	p->fts_flags = 0;
	p->fts_instr = FTS_NOINSTR;
	p->fts_number = 0;
	p->fts_pointer = NULL;
	return (p);
}

static void
fts_free(FTSENT *p)
{
#ifndef FTS_ALLOC_ALIGNED
	if (p->fts_statp)
		free(p->fts_statp);
#endif
	free(p);
}

static void
fts_lfree(FTSENT *head)
{
	FTSENT *p;

	/* Free a linked list of structures. */
	while ((p = head) != NULL) {
		head = head->fts_link;
		fts_free(p);
	}
}

/*
 * 0 < x <= 1 -> x = 1,
 * 1 < x <= 2 -> x = 2,
 * 2 < x <= 4 -> x = 4,
 * 4 < x <= 8 -> x = 8,
 * ...
 */
static size_t
fts_pow2(size_t x)
{
	/*
	 * Suppose x has 8 bits "01??????", then
	 * 1) x |= x>>1  -> x = "011?????",
	 * 2) x |= x>>2  -> x = "01111???",
	 * 3) x |= x>>4  -> x = "01111111".
	 */
	unsigned shift = 1;
	for (--x; shift < sizeof(x)*8; shift *= 2)
		x |= x >> shift;
	x++;
	return (x);
}

/*
 * Allow essentially unlimited paths; find, rm, ls should all work on any tree.
 * Most systems will allow creation of paths much longer than MAXPATHLEN, even
 * though the kernel won't resolve them.  Round up the new size to a power of 2,
 * so we don't realloc the path 2 bytes at a time.
 */
static int
fts_palloc(FTS *sp, size_t size)
{
	char *new_path;

	_DIAGASSERT(sp != NULL);

#ifdef __FTS_COMPAT_LENGTH
	/* Protect against fts_pathsize overflow. */
	if (size > USHRT_MAX + 1) {
		errno = ENAMETOOLONG;
		return (1);
	}
#endif
	size = fts_pow2(size);
	new_path = (char *) realloc(sp->fts_path, size);
	if (new_path == NULL)
		return (1);
	sp->fts_path = new_path;
	sp->fts_pathsize = fts_pathsize_truncate(size);
	return (0);
}

/*
 * When the path is realloc'd, have to fix all of the pointers in structures
 * already returned.
 */
static void
fts_padjust(FTS *sp, FTSENT *head)
{
	FTSENT *p;
	char *addr;

	_DIAGASSERT(sp != NULL);

#define	ADJUST(p) do {							\
	if ((p)->fts_accpath != (p)->fts_name)				\
		(p)->fts_accpath =					\
		    addr + ((p)->fts_accpath - (p)->fts_rpath);		\
	(p)->fts_rpath = addr;						\
} while (/*CONSTCOND*/0)

	addr = sp->fts_path;

	/* Adjust the current set of children. */
	for (p = sp->fts_child; p; p = p->fts_link)
		ADJUST(p);

	/* Adjust the rest of the tree, including the current level. */
	for (p = head; p->fts_level >= FTS_ROOTLEVEL;) {
		ADJUST(p);
		p = p->fts_link ? p->fts_link : p->fts_parent;
	}
}

static size_t
fts_maxarglen(char * const *argv)
{
	size_t len, max;

	_DIAGASSERT(argv != NULL);

	for (max = 0; *argv; ++argv)
		if ((len = strlen(*argv)) > max)
			max = len;
	return (max + 1);
}

/*
 * Change to dir specified by fd or p->fts_accpath without getting
 * tricked by someone changing the world out from underneath us.
 * Assumes p->fts_dev and p->fts_ino are filled in.
 */
static int
fts_safe_changedir(const FTS *sp, const FTSENT *p, const fts_dir_fd_t fd,
		   const char *path)
{
	int ret = 0;
	fts_dir_fd_t work_fd = fd;
	fts_stat_t sb;

	if (ISSET(FTS_NOCHDIR))
		return 0;

#ifndef _MSC_VER

	if (work_fd < 0) {
		if (path == NULL) {
			errno = EINVAL;
			return -1;
		}
		if ((work_fd = open(path, O_RDONLY)) == -1)
			return -1;
	}

	if (fstat(work_fd, &sb) == -1)
		goto bail;

	if (sb.st_ino != p->fts_ino || sb.st_dev != p->fts_dev) {
		errno = ENOENT;
		goto bail;
	}

	ret = fchdir(work_fd);
bail:
	if (fd < 0) {
		int save_errno = errno;
		(void)close(work_fd);
		errno = save_errno;
	}

#else /* _MSC_VER */

	(void) work_fd;

	if (fd != BAD_FD) {
		if (xfstat(fd->handle, fd->abs_path, &sb) == -1)
			return -1;
	} else if (path == NULL) {
		errno = EINVAL;
		return -1;
	} else if (xstat(path, &sb) == -1)
		return -1;

	if (sb.st_ino != p->fts_ino || sb.st_dev != p->fts_dev) {
		errno = ENOENT;
		return -1;
	}

	ret = fd != BAD_FD ? fchdir(fd) : _chdir(path);

#endif /* _MSC_VER */

	return ret;
}
