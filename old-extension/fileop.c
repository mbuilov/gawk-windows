/*
 * fileop.c -- Builtin functions for binary I/O and other interfaces to
 *	the filesystem.
 */

/*
 * Copyright (C) 2012 the Free Software Foundation, Inc.
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

#include "awk.h"

int plugin_is_GPL_compatible;

typedef struct file_struct {
	struct file_struct *next;
	FILE *fp;
	int flags;
	char path[1];
} file_t;

static file_t *files;
static file_t *file_open(const char *builtin_name, int nargs, int do_open);
static int mode2flags(const char *mode);

/* do_fread --- read from file */

static NODE *
do_fread(int nargs)
{
	NODE *arg;
	size_t rlen, count;
	file_t *f;
	char *rbuf;

	f = file_open("fread", nargs, true);

	arg = get_scalar_argument(2, false);
	force_number(arg);
	rlen = get_number_ui(arg);

	emalloc(rbuf, char *, rlen + 1, "do_fread");
	if ((count = fread(rbuf, 1, rlen, f->fp)) < rlen) {
		if (! feof(f->fp))
			update_ERRNO_int(errno);
	}
	return make_str_node(rbuf, count, ALREADY_MALLOCED);
}

/* do_fwrite --- write to file */

static NODE *
do_fwrite(int nargs)
{
	NODE *arg;
	file_t *f;
	size_t count = 0;

	f = file_open("fwrite", nargs, true);

	arg = get_scalar_argument(2, false);
	force_string(arg);
	if (arg->stlen > 0) {
		count = fwrite(arg->stptr, 1, arg->stlen, f->fp);
		if (count < arg->stlen)
			update_ERRNO_int(errno);
	}
	return make_number(count);
}

/* do_fseek --- set the file position indicator */

static NODE *
do_fseek(int nargs)
{
	NODE *arg;
	long offset;
	file_t *f;
	int whence = 0, ret = 0;

	f = file_open("fseek", nargs, true);

	arg = get_scalar_argument(2, false);
	force_number(arg);
	offset = get_number_si(arg);

	arg = get_scalar_argument(3, false);
	force_string(arg);
	if (strcasecmp(arg->stptr, "SEEK_SET") == 0)
		whence = SEEK_SET;
	else if (strcasecmp(arg->stptr, "SEEK_CUR") == 0)
		whence = SEEK_CUR;
	else if (strcasecmp(arg->stptr, "SEEK_END") == 0)
		whence = SEEK_END;
	else
		fatal(_("fseek: `%.*s' is not a valid 4th argument"),
			(int) arg->stlen, arg->stptr);

	if (fseek(f->fp, offset, whence) < 0) {
		update_ERRNO_int(errno);
		ret = -1;
	}
	return make_number(ret);
}

/* do_ftruncate --- truncate the file to a specified length */

static NODE *
do_ftruncate(int nargs)
{
	NODE *arg;
	file_t *f;
	off_t len;
	int ret = 0;

	f = file_open("ftruncate", nargs, true);
	arg = get_scalar_argument(2, false);
	force_number(arg);
	len = (off_t) get_number_si(arg);
	if (ftruncate(fileno(f->fp), len) < 0) {
		update_ERRNO_int(errno);
		ret = -1;
	}
	return make_number(ret);
}

/* do_unlink --- delete the name from the filesystem */

static NODE *
do_unlink(int nargs)
{
	NODE *file;
	int ret = 0;

	file = get_scalar_argument(0, false);
	force_string(file);
	if (file->stlen == 0)
		fatal(_("unlink: filename has empty string value"));
	if (unlink(file->stptr) < 0) {
		update_ERRNO_int(errno);
		ret = -1;
	}
	return make_number(ret);
}

/* do_flush --- flush buffered data to file */

static NODE *
do_flush(int nargs)
{
	file_t *f;
	int status = -1;

	f = file_open("flush", nargs, false);
	if (f != NULL) {
		status = fflush(f->fp);
		if (status != 0)
			update_ERRNO_int(errno);
	}
	return make_number(status);
}

/* do_fclose --- close an open file */

static NODE *
do_fclose(int nargs)
{
	file_t *f;
	int status = -1;

	f = file_open("fclose", nargs, false);
	if (f != NULL) {
		status = fclose(f->fp);
		if (status != 0)
			update_ERRNO_int(errno);
		assert(files == f);
		files = f->next;
		efree(f);
	}
	return make_number(status);
}

/* do_filesize --- return the size of the file */

static NODE *
do_filesize(int nargs)
{
	NODE *file;
	struct stat sbuf;
	AWKNUM d = -1.0;

	file = get_scalar_argument(0, false);
	force_string(file);
	if (file->stlen == 0)
		fatal(_("filesize: filename has empty string value"));

	if (stat(file->stptr, & sbuf) < 0) {
		update_ERRNO_int(errno);
		goto ferror;
	}
	if ((sbuf.st_mode & S_IFMT) != S_IFREG) {
		errno = EINVAL;
		update_ERRNO_int(errno);
		goto ferror;
	}
	d = sbuf.st_size;

ferror:
	return make_number(d);
}

/* do_file_exists --- check if path exists in the filesystem */

static NODE *
do_file_exists(int nargs)
{
	NODE *file;
	struct stat sbuf;
	int ret = 1;

	file = get_scalar_argument(0, false);
	force_string(file);
	if (file->stlen == 0)
		fatal(_("file_exists: filename has empty string value"));

	if (stat(file->stptr, & sbuf) < 0) {
		if (errno != ENOENT)
			update_ERRNO_int(errno);
		ret = 0;
	}
	return make_number(ret);
}


/* file_open --- open a file or find an already opened file */

static file_t *
file_open(const char *builtin_name, int nargs, int do_open)
{
	NODE *file, *mode;
	file_t *f, *prev;
	FILE *fp;
	int flags;
	char *path;

	if (nargs < 2)
		cant_happen();

	file = get_scalar_argument(0, false);
	force_string(file);
	mode = get_scalar_argument(1, true);
	force_string(mode);

	if (file->stlen == 0)
		fatal(_("%s: filename has empty string value"), builtin_name);
	if (mode->stlen == 0)
		fatal(_("%s: mode has empty string value"), builtin_name);

	flags = mode2flags(mode->stptr);
	if (flags < 0)
		fatal(_("%s: invalid mode `%.*s'"), builtin_name,
			(int) mode->stlen, mode->stptr);

	path = file->stptr;
	for (prev = NULL, f = files; f != NULL; prev = f, f = f->next) {
		if (strcmp(f->path, path) == 0 && f->flags == flags) {
			/* Move to the head of the list */
			if (prev != NULL) {
				prev->next = f->next;
				f->next = files;
				files = f;
			}
			return f;
		}
	}

	if (! do_open) {
		if (do_lint)
			lintwarn(_("%s: `%.*s' is not an open file"),
				builtin_name, (int) file->stlen, file->stptr);
		return NULL;
	}

	fp = fopen(path, mode->stptr);
	if (fp == NULL)
		fatal(_("%s: cannot open file `%.*s'"),
			builtin_name, (int) file->stlen, file->stptr);

	os_close_on_exec(fileno(fp), path, "", "");

	emalloc(f, file_t *, sizeof(file_t) + file->stlen + 1, "file_open");
	memcpy(f->path, path, file->stlen + 1);
	f->fp = fp;
	f->flags = flags;
	f->next = files;
	files = f;
	return f;
}


/*
 * mode2flags --- convert a string mode to an integer flag;
 *	modified from str2mode in io.c.
 */

static int
mode2flags(const char *mode)
{
	int ret = -1;
	const char *second;

	if (mode == NULL || mode[0] == '\0')
		return -1;

	second = & mode[1];

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
		ret = -1;
	}
	if (ret != -1 && strchr(mode, 'b') != NULL)
		ret |= O_BINARY;
	return ret;
}


/* dlload --- load new builtins in this library */

NODE *
dlload(NODE *tree, void *dl)
{
	make_old_builtin("fseek", do_fseek, 4);
	make_old_builtin("fread", do_fread, 3);
	make_old_builtin("fwrite", do_fwrite, 3);
	make_old_builtin("flush", do_flush, 2);
	make_old_builtin("filesize", do_filesize, 1);
	make_old_builtin("file_exists", do_file_exists, 1);
	make_old_builtin("fclose", do_fclose, 2);
	make_old_builtin("ftruncate", do_ftruncate, 3);
	make_old_builtin("unlink", do_unlink, 1);
	return make_number((AWKNUM) 0);
}


/* dlunload --- routine called when exiting */

void
dlunload()
{
	file_t *f;
	for (f = files; f != NULL; f = f->next) {
		if (f->fp != NULL) {
			fclose(f->fp);
			f->fp = NULL;
		}
	}
}
