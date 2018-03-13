/*
 * revtwoway.c --- Provide a two-way processor that reverses lines.
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 8/2012
 */

/*
 * Copyright (C) 2012-2014, 2016, 2018 the Free Software Foundation, Inc.
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

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static const char *ext_version = "revtwoway extension: version 1.0";

static awk_bool_t init_revtwoway(void);
static awk_bool_t (*init_func)(void) = init_revtwoway;

int plugin_is_GPL_compatible;

/*
 * Use this variable to provide a value != INVALID_HANDLE in the awk_input_buf_t
 * and != NULL in the awk_output_buf_t.  The idea is to have a value that
 * is greater than the largest allowable file descriptor.
 */
static size_t max_fds;

#ifndef HAVE_GETDTABLESIZE
/* gawk_getdtablesize --- replacement version that should be good enough */

static inline int
gawk_getdtablesize()
{
	/*
	 * Algorithm for the GNULIB folks:
	 *
	 * Set up a bitmap of 2048 elements.
	 * Initialize it to zero.
	 * In a loop, do
	 * 	fd = open("/dev/null", O_RDONLY)
	 * 	set the bit corresponding to fd in the bit map
	 * until it fails.
	 * Get the highest value that succeeded and increment it by one
	 * --> that is how many descriptors we have.
	 * Loop over the bitmap to close all the file descriptors we opened.
	 *
	 * Do all this upon the first call and return static values upon
	 * subsequent calls.
	 */

	/* In the meantime, this is good enough for us: */
	return 1024;
}

#define getdtablesize() gawk_getdtablesize()
#endif

/*
 * IMPORTANT NOTE: This is a NOT a true general purpose
 * extension.  It is intended to demonstrate how to set up
 * all the "plumbing" and to work one record at a time, ONLY.
 *
 * While it would be possible to set up buffering and manage it,
 * that would duplicate a large chunk of the code in gawk's
 * get_a_record() function, and there's no real point in doing that.
 */

/* the data in the opaque pointer */
typedef struct two_way_proc_data {
	size_t size;	/* size of allocated buffer */
	size_t len;	/* how much is actually in use */
	char *data;
	size_t in_use;	/* use count, must hit zero to be freed */
} two_way_proc_data_t;

/* close_two_proc_data --- release the data */

static void
close_two_proc_data(two_way_proc_data_t *proc_data)
{
	if (proc_data->in_use > 1) {
		proc_data->in_use--;
		return;
	}

	gawk_free(proc_data->data);
	gawk_free(proc_data);
}

/*
 * Input side of the two-way processor (input TO gawk)
 */

/* rev2way_get_record --- get one record at a time out of a directory */

static int
rev2way_get_record(char **out, awk_input_buf_t *iobuf, int *errcode,
		char **rt_start, size_t *rt_len,
		const awk_fieldwidth_info_t **unused)
{
	int len = 0;	/* for now */
	two_way_proc_data_t *proc_data;

	/*
	 * The caller sets *errcode to 0, so we should set it only if an
	 * error occurs.
	 */

	(void) errcode;		/* silence warnings */
	if (out == NULL || iobuf == NULL || iobuf->opaque == NULL)
		return EOF;

	proc_data = (two_way_proc_data_t *) iobuf->opaque;
	if (proc_data->len == 0)
		return 0;

	*out = proc_data->data;

	len = proc_data->len;
	proc_data->len = 0;

	*rt_len = 0;	/* default: set RT to "" */
	if (proc_data->data[len-1] == '\n') {
		while (proc_data->data[len-1] == '\n') {
			len--;
			(*rt_len)++;
		}
		*rt_start = proc_data->data + len;
	}

	return len;
}

/* rev2way_close --- close up input side when done */

static void
rev2way_close(awk_input_buf_t *iobuf)
{
	two_way_proc_data_t *proc_data;

	if (iobuf == NULL || iobuf->opaque == NULL)
		return;

	proc_data = (two_way_proc_data_t *) iobuf->opaque;
	close_two_proc_data(proc_data);

	iobuf->fd = INVALID_HANDLE;
}


/*
 * Output side of the two-way processor (output FROM gawk)
 */

/* rev2way_fwrite --- write out characters in reverse order */

static size_t
rev2way_fwrite(const void *buf, size_t size, size_t count, FILE *fp, void *opaque)
{
	two_way_proc_data_t *proc_data;
	size_t amount, char_count;
	char *src, *dest;

	(void) fp;	/* silence warnings */
	if (opaque == NULL)
		return 0;	/* error */

	proc_data = (two_way_proc_data_t *) opaque;
	amount = size * count;

	/* do the dance */
	if (amount > proc_data->size || proc_data->len > 0) {
		if (proc_data->data == NULL)
			emalloc(proc_data->data, char *,  amount, "rev2way_fwrite");
		else
			erealloc(proc_data->data, char *, proc_data->size + amount, "rev2way_fwrite");
		proc_data->size += amount;
	}

	src = (char *) buf + amount -1;
	dest = proc_data->data + proc_data->len;
	for (char_count = amount; char_count > 0; char_count--) {
		/* copy in backwards */
		*dest++ = *src--;
	}
	proc_data->len += amount;

	return amount;
}

/* rev2way_fflush --- do nothing hook for fflush */

static int
rev2way_fflush(FILE *fp, void *opaque)
{
	(void) fp;
	(void) opaque;

	return 0;
}

/* rev2way_ferror --- do nothing hook for ferror */

static int
rev2way_ferror(FILE *fp, void *opaque)
{
	(void) fp;
	(void) opaque;

	return 0;
}

/* rev2way_fclose --- close output side of two-way processor */

static int
rev2way_fclose(FILE *fp, void *opaque)
{
	two_way_proc_data_t *proc_data;

	if (opaque == NULL)
		return EOF;	/* error */

	(void) fp;

	proc_data = (two_way_proc_data_t *) opaque;
	close_two_proc_data(proc_data);

	return 0;
}


/* revtwoway_can_two_way --- return true if we want the file */

static awk_bool_t
revtwoway_can_take_two_way(const char *name)
{
	return (name != NULL && strcmp(name, "/magic/mirror") == 0);
}

/*
 * revtwoway_take_control_of --- set up two way processor
 * We can assume that revtwoway_can_take_two_way just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
revtwoway_take_control_of(const char *name, awk_input_buf_t *inbuf, awk_output_buf_t *outbuf)
{
	two_way_proc_data_t *proc_data;

	(void) name;	/* silence warnings */
	if (inbuf == NULL || outbuf == NULL)
		return awk_false;

	emalloc(proc_data, two_way_proc_data_t *, sizeof(two_way_proc_data_t), "revtwoway_take_control_of");
	proc_data->in_use = 2;
	proc_data->size = 0;
	proc_data->len = 0;
	proc_data->data = NULL;

	if (max_fds + 1 == 0)	/* wrapped. ha! */
		max_fds = getdtablesize();

	/* input side: */
	inbuf->get_record = rev2way_get_record;
	inbuf->close_func = rev2way_close;
	inbuf->fd = max_fds;
	inbuf->opaque = proc_data;

	/* output side: */
	outbuf->fp = (FILE *) max_fds++;
	outbuf->opaque = proc_data;
	outbuf->gawk_fwrite = rev2way_fwrite;
	outbuf->gawk_fflush = rev2way_fflush;
	outbuf->gawk_ferror = rev2way_ferror;
	outbuf->gawk_fclose = rev2way_fclose;
	outbuf->redirected = awk_true;

	return awk_true;
}

static awk_two_way_processor_t two_way_processor = {
	"revtwoway",
	revtwoway_can_take_two_way,
	revtwoway_take_control_of,
	NULL
};

/* init_revtwoway --- set things ups */

static awk_bool_t
init_revtwoway()
{
	register_two_way_processor(& two_way_processor);

	max_fds = getdtablesize();

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0, 0, awk_false, NULL }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, revtwoway, "")
