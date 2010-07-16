/*
** dlfcn.c -- limited implementation of posix dynamic loading functions
*/

/*
 * Copyright (C) 2003 the Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <errno.h>
#include <windows.h>

/* open the library file. We currently ignore flags. */
void *dlopen(const char * libname, int flags)
{
   HMODULE libH;


   /* if libname is specified, we need to load a library of that name */
   if (libname) {
      libH = LoadLibrary(libname);
   }

   /* otherwise, we're supposed to return a handle to global symbol
    * information, which includes the executable and all libraries loaded
    * with RTLD_GLOBAL. For our purposes, it doesn't really matter, so
    * we simply return the handle to the .exe */
   else {
      libH = GetModuleHandle(NULL);
   }

   return (void *)libH;
}


/* don't need the library any more */
int dlclose(void * libH)
{
   int rc;

   if (FreeLibrary((HMODULE)libH)) {
      rc = 0;
   }
   else {
      rc = -1;
   }

   return rc;
}

/* find the symbol */
void *dlsym(void * /*restrict*/ libH, const char * /*restrict*/ fnName)
{
   return (void *)GetProcAddress((HMODULE)libH, fnName);
}

char *dlerror(void)
{
   static char errbuf[1024];

   FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errbuf, sizeof(errbuf), NULL);

   return errbuf;
}


