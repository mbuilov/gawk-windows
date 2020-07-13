/* dlfcn.h replacement for MS-Windows build.  */
/*
 * Copyright (C) 2010, 2013, the Free Software Foundation, Inc.
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
#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY 1

extern void *dlopen (const char *, int);
extern int   dlclose (void *);
extern void *dlsym (void *, const char *);
extern const char *dlerror_buf (char buf[], unsigned int buf_size);

#endif	/* DLFCN_H */
