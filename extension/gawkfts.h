/*	$NetBSD: fts.h,v 1.19 2009/08/16 19:33:38 christos Exp $	*/

/*
 * Copyright (c) 1989, 1993
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
 *
 *	@(#)fts.h	8.3 (Berkeley) 8/14/94
 */

/*
 * Michael M. Builov
 * mbuilov@gmail.com
 * Ported to _MSC_VER/__MINGW32__ 4-5/2020
 */

#ifndef	_FTS_H_
#define	_FTS_H_

#ifndef __THROW
# ifndef __GNUC_PREREQ
#  define __GNUC_PREREQ(maj, min) (0)
# endif
# if defined __cplusplus && __GNUC_PREREQ (2,8)
#  define __THROW	throw ()
# else
#  define __THROW
# endif
#endif

#ifdef __MVS__
#include <limits.h>
#define MAXPATHLEN FILENAME_MAX
#endif

#if !defined(_MSC_VER) && !defined(__MINGW32__)

typedef int		fts_dir_fd_t;
typedef dev_t		fts_dev_t;
typedef ino_t		fts_ino_t;
typedef struct stat	fts_stat_t;

#else /* _MSC_VER || __MINGW32__ */

struct dir_fd_;
typedef struct dir_fd_	*fts_dir_fd_t;
typedef xdev_t		fts_dev_t;
typedef xino_t		fts_ino_t;
typedef struct xstat	fts_stat_t;
#define FTS_DIRNLINK_SUPPORTED 0

#endif /* _MSC_VER || __MINGW32__ */

/* Non-zero if st_nlink field of stat(2) result is the number of
   sub-directories (including "." and "..") for a stat'ed directory.  */
#ifndef FTS_DIRNLINK_SUPPORTED
#define FTS_DIRNLINK_SUPPORTED 1
#endif

typedef struct {
	struct _ftsent *fts_cur;	/* current node */
	struct _ftsent *fts_child;	/* linked list of children */
	struct _ftsent **fts_array;	/* sort array */
	char *fts_path;			/* path for this descent */
	fts_dir_fd_t fts_rfd;		/* fd for root */
	fts_dev_t fts_dev;		/* starting device # */
	unsigned int fts_pathlen;	/* sizeof(path) */
	unsigned int fts_nitems;	/* elements in the sort array */
	int (*fts_compar)		/* compare function */
		(const struct _ftsent **, const struct _ftsent **);

#define	FTS_COMFOLLOW	0x001		/* follow command line symlinks */
#define	FTS_LOGICAL	0x002		/* logical walk */
#define	FTS_NOCHDIR	0x004		/* don't change directories */
#define	FTS_NOSTAT	0x008		/* don't get stat info */
#define	FTS_PHYSICAL	0x010		/* physical walk */
#define	FTS_SEEDOT	0x020		/* return dot and dot-dot */
#define	FTS_XDEV	0x040		/* don't cross devices */
/* #define	FTS_WHITEOUT	0x080 */	/* return whiteout information */
#define	FTS_OPTIONMASK	0x0ff		/* valid user option mask */

#define	FTS_NAMEONLY	0x100		/* (private) child names only */
#define	FTS_STOP	0x200		/* (private) unrecoverable error */
	int fts_options;		/* fts_open options, global flags */
} FTS;

typedef struct _ftsent {
	struct _ftsent *fts_cycle;	/* cycle node */
	struct _ftsent *fts_parent;	/* parent directory */
	struct _ftsent *fts_link;	/* next file in directory */
	void *fts_pointer;	        /* local address value */
	char *fts_accpath;		/* access path */
	char *fts_path;			/* root path */
	long long fts_number;		/* local numeric value */
	fts_dir_fd_t fts_symfd;		/* fd for symlink */
	int fts_errno;			/* errno for this node */
	unsigned int fts_pathlen;	/* strlen(fts_path) */
	unsigned int fts_namelen;	/* strlen(fts_name) */

#define	FTS_ROOTPARENTLEVEL	-1
#define	FTS_ROOTLEVEL		 0
	int fts_level;		/* depth (-1 to N) */

	/* these fields are for FTS_D/FTS_DC only */
#if FTS_DIRNLINK_SUPPORTED
	unsigned int fts_nlink;	/* link count */
#endif
	fts_ino_t fts_ino;		/* inode */
	fts_dev_t fts_dev;		/* device */

#define	FTS_D		 1		/* preorder directory */
#define	FTS_DC		 2		/* directory that causes cycles */
#define	FTS_DEFAULT	 3		/* none of the above */
#define	FTS_DNR		 4		/* unreadable directory */
#define	FTS_DOT		 5		/* dot or dot-dot */
#define	FTS_DP		 6		/* postorder directory */
#define	FTS_ERR		 7		/* error; errno is set */
#define	FTS_F		 8		/* regular file */
#define	FTS_INIT	 9		/* initialized only */
#define	FTS_NS		10		/* stat(2) failed */
#define	FTS_NSOK	11		/* no stat(2) requested */
#define	FTS_SL		12		/* symbolic link */
#define	FTS_SLNONE	13		/* symbolic link without target */
#define	FTS_W		14		/* whiteout object */
	unsigned short fts_info;	/* user flags for FTSENT structure */

#define	FTS_DONTCHDIR	 0x01		/* don't chdir .. to the parent */
#define	FTS_SYMFOLLOW	 0x02		/* followed a symlink to get here */
#define	FTS_ISW		 0x04		/* this is a whiteout object */
	unsigned short fts_flags;	/* private flags for FTSENT structure */

#define	FTS_AGAIN	 1		/* read node again */
#define	FTS_FOLLOW	 2		/* follow symbolic link */
#define	FTS_NOINSTR	 3		/* no instructions */
#define	FTS_SKIP	 4		/* discard node */
	unsigned short fts_instr;	/* fts_set() instructions */

	fts_stat_t *fts_statp;		/* stat(2) information */
	char fts_name[1];		/* file name */
} FTSENT;

/*
 * Due to the wonders of modern linkers, shared libraries,
 * compilers and other deep, dark, black magic voodoo, we
 * redefined the identifiers so our code will use our version
 * of these routines. See README.fts for a little bit more
 * information and a lot more ranting.
 */

#define fts_children gawk_fts_children
#define fts_close gawk_fts_close
#define fts_open gawk_fts_open
#define fts_read gawk_fts_read
#define fts_set gawk_fts_set

FTSENT	*fts_children (FTS *, int);
int	 fts_close (FTS *);
FTS	*fts_open (char * const *, int,
		   int (*)(const FTSENT **, const FTSENT **));
FTSENT	*fts_read (FTS *);
int	 fts_set (FTS *, FTSENT *, int) __THROW;

#endif /* !_FTS_H_ */
