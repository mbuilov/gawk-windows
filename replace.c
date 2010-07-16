/*
 * replace.c -- Get replacement versions of functions.
 */

/* 
 * Copyright (C) 1989, 1991-2010 the Free Software Foundation, Inc.
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

/*
 * Do all necessary includes here, so that we don't have to worry about
 * overlapping includes in the files in missing.d.
 */
#include "config.h"
#include "awk.h"


#ifdef atarist
/*
 * this will work with gcc compiler - for other compilers you may
 * have to replace path separators in this file into backslashes
 */
#include "unsupported/atari/stack.c"
#include "unsupported/atari/tmpnam.c"
#endif /* atarist */

#ifndef HAVE_SYSTEM
#ifdef atarist
#include "unsupported/atari/system.c"
#else
#include "missing_d/system.c"
#endif
#endif /* HAVE_SYSTEM */

#ifndef HAVE_MEMCMP
#include "missing_d/memcmp.c"
#endif	/* HAVE_MEMCMP */

#ifndef HAVE_MEMCPY
#include "missing_d/memcpy.c"
#endif	/* HAVE_MEMCPY */

#ifndef HAVE_MEMSET
#include "missing_d/memset.c"
#endif	/* HAVE_MEMSET */

#ifndef HAVE_MEMMOVE
#include "missing_d/memmove.c"
#endif	/* HAVE_MEMMOVE */

#ifndef HAVE_STRNCASECMP
#include "missing_d/strncasecmp.c"
#endif	/* HAVE_STRCASE */

#ifndef HAVE_STRERROR
#include "missing_d/strerror.c"
#endif	/* HAVE_STRERROR */

#ifndef HAVE_STRFTIME
#include "missing_d/strftime.c"
#endif	/* HAVE_STRFTIME */

#ifndef HAVE_STRCHR
#include "missing_d/strchr.c"
#endif	/* HAVE_STRCHR */

#if !defined(HAVE_STRTOD)
#include "missing_d/strtod.c"
#endif	/* HAVE_STRTOD */

#ifndef HAVE_STRTOUL
#include "missing_d/strtoul.c"
#endif	/* HAVE_STRTOUL */

#ifndef HAVE_TZSET
#include "missing_d/tzset.c"
#endif /* HAVE_TZSET */

#if defined TANDEM
#include "strdupc"
#include "getidc"
#include "strnchkc"
#endif /* TANDEM */

#ifndef HAVE_MKTIME
/* mktime.c defines main() if DEBUG is set */
#undef DEBUG
#include "missing_d/mktime.c"
#endif /* HAVE_MKTIME */

#ifndef HAVE_SNPRINTF
#include "missing_d/snprintf.c"
#endif

#if defined(HAVE_SOCKETS) && ! defined(HAVE_GETADDRINFO)
#include "missing_d/getaddrinfo.c"
#endif

#ifndef HAVE_USLEEP
#include "missing_d/usleep.c"
#endif
