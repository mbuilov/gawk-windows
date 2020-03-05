/* dlfcn.h replacement for MS-Windows build.  */
#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY 1

extern void *dlopen (const char *, int);
extern int   dlclose (void *);
extern void *dlsym (void *, const char *);
extern const char *dlerror_buf (char buf[], unsigned int buf_size);

#endif	/* DLFCN_H */
