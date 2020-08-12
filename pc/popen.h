/*
** popen.h -- prototypes for pipe functions
*/
/*
 * Copyright (C) 2010, 2013, 2014, 2016, the Free Software Foundation, Inc.
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

#if !defined (__DJGPP__) && !defined (__MINGW32__) && !defined (_MSC_VER)
# if defined (popen)
#  undef popen
#  undef pclose
# endif
# define popen(c, m)	os_popen(c, m)
# define pclose(f)	os_pclose(f)
  extern FILE *os_popen( const char *, const char * );
  extern int  os_pclose( FILE * );
#endif	/* !__DJGPP__ && !__MINGW32__ && !_MSC_VER */
