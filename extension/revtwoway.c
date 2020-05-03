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

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

/* Include <locale.h> before "gawkapi.h" redefines setlocale().
  "gettext.h" will include <locale.h> anyway */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

GAWK_PLUGIN_GPL_COMPATIBLE
GAWK_PLUGIN("revtwoway extension: version 1.0");

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
	if (--(proc_data->in_use) != 0)
		return;

	free(proc_data->data);
	free(proc_data);
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
	size_t len, data_len;
	two_way_proc_data_t *proc_data;

	(void) unused;

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

	len = data_len = proc_data->len;
	proc_data->len = 0;

	while (len > 0 && proc_data->data[len-1] == '\n')
		len--;

	*rt_len = data_len - len;
	if (data_len != len)
		*rt_start = proc_data->data + len;

	return (int) len;
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
rev2way_fwrite(const void *buf, size_t size, size_t count, awk_output_buf_t *outbuf)
{
	two_way_proc_data_t *proc_data;
	size_t amount;
	const char *src;
	char *dest;

	if (outbuf->opaque == NULL)
		return 0;	/* error */

	proc_data = (two_way_proc_data_t *) outbuf->opaque;
	amount = size * count;

	/* do the dance */
	if (amount > proc_data->size || proc_data->len > 0) {
		if (proc_data->data == NULL)
			emalloc(proc_data->data, char *,  amount, "rev2way_fwrite");
		else
			erealloc(proc_data->data, char *, proc_data->size + amount, "rev2way_fwrite");
		proc_data->size += amount;
	}

	src = (const char *) buf + amount;
	dest = proc_data->data + proc_data->len;
	while (src > (const char *) buf) {
		/* copy in backwards */
		*dest++ = *--src;
	}
	proc_data->len = (size_t) (dest - proc_data->data);

	return amount;
}

/* rev2way_fflush --- do nothing hook for fflush */

static int
rev2way_fflush(awk_output_buf_t *outbuf)
{
	(void) outbuf;
	return 0;
}

/* rev2way_ferror --- do nothing hook for ferror */

static int
rev2way_ferror(awk_output_buf_t *outbuf)
{
	(void) outbuf;
	return 0;
}

/* rev2way_fclose --- close output side of two-way processor */

static int
rev2way_fclose(awk_output_buf_t *outbuf)
{
	two_way_proc_data_t *proc_data;

	if (outbuf->opaque == NULL)
		return EOF;	/* error */

	proc_data = (two_way_proc_data_t *) outbuf->opaque;
	close_two_proc_data(proc_data);

	return 0;
}

/* revtwoway_can_two_way --- return true if we want the file */

static awk_bool_t
revtwoway_can_take_two_way(const char *name)
{
	return (awk_bool_t) (name != NULL && strcmp(name, "/magic/mirror") == 0);
}

/*
 * revtwoway_take_control_of --- set up two way processor
 * We can assume that revtwoway_can_take_two_way just returned true,
 * and no state has changed since then.
 */

static awk_bool_t
revtwoway_take_control_of(const char *name, awk_input_buf_t *inbuf, awk_output_buf_t *outbuf)
{
	static int fake_file;
	two_way_proc_data_t *proc_data;

	(void) name;	/* silence warnings */
	if (inbuf == NULL || outbuf == NULL)
		return awk_false;

	emalloc(proc_data, two_way_proc_data_t *, sizeof(two_way_proc_data_t), "revtwoway_take_control_of");
	proc_data->in_use = 2;
	proc_data->size = 0;
	proc_data->len = 0;
	proc_data->data = NULL;

	/* input side: */
	inbuf->get_record = rev2way_get_record;
	inbuf->close_func = rev2way_close;
	inbuf->fd = INVALID_HANDLE;	/* must be INVALID_HANDLE if not a real handle.  */
	inbuf->opaque = proc_data;

	/* output side: */
	outbuf->file = (FILE *) &fake_file; 	/* must be != NULL.  */
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
init_revtwoway(void)
{
	register_two_way_processor(& two_way_processor);

	return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0, 0, awk_false, NULL }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(init_revtwoway, func_table, revtwoway, "")
