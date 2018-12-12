/*
 * protos.h -- function prototypes for when the headers don't have them.
 */

/*
 * Copyright (C) 1991 - 2002, 2011 the Free Software Foundation, Inc.
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

#ifndef STDC_HEADERS

#define aptr_t void *	/* arbitrary pointer type */
extern aptr_t malloc(size_t);
extern aptr_t realloc(aptr_t, size_t);
extern aptr_t calloc(size_t, size_t);

extern void free(aptr_t);
extern char *getenv(const char *);

#if ! defined(HAVE_STRING_H) && ! defined(HAVE_STRINGS_H)
extern char *strcpy(char *, const char *);
extern char *strcat(char *, const char *);
extern char *strncpy(char *, const char *, size_t);
extern int strcmp(const char *, const char *);
extern int strncmp(const char *, const char *, size_t);
extern char *strchr(const char *, int);
extern char *strrchr(const char *, int);
extern char *strstr(const char *s1, const char *s2);
extern size_t strlen(const char *);
extern long strtol(const char *, char **, int);

extern aptr_t memset(aptr_t, int, size_t);
extern aptr_t memcpy(aptr_t, const aptr_t, size_t);
extern aptr_t memmove(aptr_t, const aptr_t, size_t);
extern aptr_t memchr(const aptr_t, int, size_t);
extern int memcmp(const aptr_t, const aptr_t, size_t);
#endif /* ! defined(HAVE_STRING_H) && ! defined(HAVE_STRINGS_H) */

#ifndef VMS
extern char *strerror(int);
#else
extern char *strerror(int,...);
#endif

#if ! defined(__GNU_LIBRARY__)
extern size_t strftime(char *, size_t, const char *, const struct tm *);
#endif
extern time_t time(time_t *);

extern FILE *fdopen(int, const char *);
extern int fprintf(FILE *, const char *, ...);
#if ! defined(__GNU_LIBRARY__)
extern size_t fwrite(const aptr_t, size_t, size_t, FILE *);
#endif
extern int fputs(const char *, FILE *);
extern int unlink(const char *);
extern int fflush(FILE *);
extern int fclose(FILE *);
extern FILE *popen(const char *, const char *);
extern int pclose(FILE *);
extern void abort();
extern int isatty(int);
extern void exit(int);
extern int system(const char *);
extern int sscanf(const char *, const char *, ...);
#ifndef toupper
extern int toupper(int);
#endif
#ifndef tolower
extern int tolower(int);
#endif

extern double pow(double x, double y);
extern double atof(const char *);
extern double strtod(const char *, char **);
extern int fstat(int, struct stat *);
extern int stat(const char *, struct stat *);
extern off_t lseek(int, off_t, int);
extern int close(int);
extern int creat(const char *, mode_t);
extern int open(const char *, int, ...);
extern int pipe(int *);
extern int dup(int);
extern int dup2(int,int);
extern int fork();
extern int execl(const char *, const char *, ...);
#ifndef HAVE_SYS_WAIT_H
extern int wait(int *);
#endif
extern void _exit(int);

#undef aptr_t

#endif /* STDC_HEADERS */


/* prototypes for missing functions defined in missing_d/ */

#ifndef HAVE_STRNCASECMP
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, register size_t n);
#endif

#ifndef HAVE_STRTOUL
extern unsigned long int strtoul(const char *, char **endptr, int base);
#endif

#ifndef HAVE_TZSET
extern void tzset();
#endif

#ifndef HAVE_MKTIME
extern time_t mktime(struct tm *tp);
#endif

#ifndef HAVE_TIMEGM
extern time_t timegm(struct tm *);
#endif

#ifndef HAVE_SNPRINTF
extern int snprintf(char *restrict buf, size_t len, const char *restrict fmt, ...);
#endif

#ifndef HAVE_USLEEP
extern int usleep(unsigned int);
#endif

#ifndef HAVE_SETENV
extern int setenv(const char *, const char *, int);
extern int unsetenv(const char *);
#endif

#if !defined(HAVE_STRCOLL)
extern int strcoll(const char *, const char *);
#endif

