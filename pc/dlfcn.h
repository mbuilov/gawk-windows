/*
** dlfcn.h -- limited implementation of posix dynamic loading functions
*/

/*
 * Copyright (C) 2003 the Free Software Foundation, Inc.
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

#ifndef _DLFCN_H
#define _DLFCN_H

/* symbols required by susv3. These are not supported here (everything is
 * RTLD_NOW, RTLD_GLOBAL) */
#define RTLD_LAZY   0
#define RTLD_NOW    1
#define RTLD_GLOBAL 0
#define RTLD_LOCAL  2

int dlclose(void *);
char *dlerror(void);
void *dlopen(const char *, int);
void *dlsym(void * /*restrict*/, const char * /*restrict*/);

#endif
