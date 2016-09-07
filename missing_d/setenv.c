/*	$NetBSD: setenv.c,v 1.1.1.1 2009/04/12 15:33:26 christos Exp $	*/

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)setenv.c	8.1 (Berkeley) 6/4/93";
static const char rcsid[] = "Id: setenv.c,v 1.2 2005/04/27 04:56:11 sra Exp";
#endif /* LIBC_SCCS and not lint */

/*
 * Copyright (c) 1987, 1993
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
 * Per the statement at http://opensource.org/licenses/bsd-license.php,
 *
 *	The advertising clause in the license appearing on BSD Unix files was
 *	officially rescinded by the Director of the Office of Technology
 *	Licensing of the University of California on July 22 1999. He states
 *	that clause 3 is "hereby deleted in its entirety."
 *
 * I removed the advertising clause in the above copyright.
 * The above web site points to
 * ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change.
 *
 * Arnold Robbins
 * 7 September 2016
 */

/* #include "port_before.h" */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* #include "port_after.h" */

#define NEED_SETENV
#if !defined(NEED_SETENV)
int __bindcompat_setenv;
#else

/*
 * Gawk changes:
 * 1. Comment out "port_before.h" and "port_after.h" includes.
 * 2. Define NEED_SETENV.
 * 3. Add leading "int" to setenv() for C99 compilers. Which is worse,
 *    that it wasn't there in the first place, or that C99 now makes
 *    such functions void, breaking 40+ years of historical practice?
 * 4. Change return type of unsetenv to "int" for standards
 *    conformance and check for name containing an "=" in
 *    which case it return an error. (See unsetenv(3), at least on Linux.)
 */

extern char **environ;

static char *findenv(const char *name, int *offset);

/*%
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int
setenv(const char *name, const char *value, int rewrite)
{
	extern char **environ;
	static int alloced;			/*%< if allocated space before */
	char *c;
	int l_value, offset;

	if (*value == '=')			/*%< no `=' in value */
		++value;
	l_value = strlen(value);
	if ((c = findenv(name, &offset))) {	/*%< find if already exists */
		if (!rewrite)
			return (0);
		if (strlen(c) >= l_value) {	/*%< old larger; copy over */
			while (*c++ = *value++);
			return (0);
		}
	} else {					/*%< create new slot */
		int cnt;
		char **p;

		for (p = environ, cnt = 0; *p; ++p, ++cnt);
		if (alloced) {			/*%< just increase size */
			environ = (char **)realloc((char *)environ,
			    (size_t)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				return (-1);
		}
		else {				/*%< get new space */
			alloced = 1;		/*%< copy old entries into it */
			p = malloc((size_t)(sizeof(char *) * (cnt + 2)));
			if (!p)
				return (-1);
			memcpy(p, environ, cnt * sizeof(char *));
			environ = p;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (c = (char *)name; *c && *c != '='; ++c);	/*%< no `=' in name */
	if (!(environ[offset] =			/*%< name + `=' + value */
	    malloc((size_t)((int)(c - name) + l_value + 2))))
		return (-1);
	for (c = environ[offset]; (*c = *name++) && *c != '='; ++c);
	for (*c++ = '='; *c++ = *value++;);
	return (0);
}

/*%
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
int
unsetenv(const char *name)
{
	char **p;
	int offset;

	if (strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}

	while (findenv(name, &offset))	/*%< if set multiple times */
		for (p = &environ[offset];; ++p)
			if (!(*p = *(p + 1)))
				break;

	return 0;
}

/*%
 * findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
static char *
findenv(const char *name, int *offset)
{
	const char *np;
	char **p, *c;
	int len;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (c = *p) != NULL; ++p)
		if (strncmp(c, name, len) == 0 && c[len] == '=') {
			*offset = p - environ;
			return (c + len + 1);
		}
	return (NULL);
}
#endif

/*! \file */
