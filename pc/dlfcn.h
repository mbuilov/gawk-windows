/* dlfcn.h replacement for MS-Windows build.  */
#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY 1

extern void *dlopen (const char *, int);
extern int   dlclose (void *);
extern void *dlsym (void *, const char *);
extern char *dlerror (void);

#endif	/* DLFCN_H */
