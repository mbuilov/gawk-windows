/*
 * Copyright (C) 2010, the Free Software Foundation, Inc.
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

#ifdef __MINGW32__

#include <process.h>

unsigned int getuid (void)
{
  return (0);                   /* root! */
}

unsigned int geteuid (void)
{
  return (0);
}

unsigned int getgid (void)
{
  return (0);
}

unsigned int getegid (void)
{
  return (0);
}

#endif	/* __MINGW32__ */

int getpgrp(void)
{
  return (0);
}

#if defined(__DJGPP__) || defined(__MINGW32__)
int getppid(void)
{
  return (0);
}
#endif	/* __DJGPP__ || __MINGW32__ */
