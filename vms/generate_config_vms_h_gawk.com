$! File: GENERATE_CONFIG_H_VMS_GAWK.COM
$!
$! Gawk like most open source products uses a variant of a config.h file.
$! Depending on the curl version, this could be config.h or curl_config.h.
$!
$! For GNV based builds, the configure script is run and that produces
$! a [curl_]config.h file.  Configure scripts on VMS generally do not
$! know how to do everything, so there is also a [-.lib]config-vms.h file
$! that has VMS specific code that compensates for bugs in some of the
$! VMS shared images.
$!
$! This generates a []config.h file and also a config_vms.h file,
$! which is used to supplement that file.
$!
$!
$! Copyright (C) 2014, 2016, 2019 the Free Software Foundation, Inc.
$!
$! This file is part of GAWK, the GNU implementation of the
$! AWK Progamming Language.
$!
$! GAWK is free software; you can redistribute it and/or modify
$! it under the terms of the GNU General Public License as published by
$! the Free Software Foundation; either version 3 of the License, or
$! (at your option) any later version.
$!
$! GAWK is distributed in the hope that it will be useful,
$! but WITHOUT ANY WARRANTY; without even the implied warranty of
$! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
$! GNU General Public License for more details.
$!
$! You should have received a copy of the GNU General Public License
$! along with this program; if not, write to the Free Software
$! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
$! USA
$!
$! Per assignment agreement with FSF, similar procedures may be present
$! in other packages under other licensing agreements and copyrights
$!
$!
$! 21-Jan-2014	J. Malmberg
$!
$!=========================================================================
$!
$! Allow arguments to be grouped together with comma or separated by spaces
$! Do no know if we will need more than 8.
$ args = "," + p1 + "," + p2 + "," + p3 + "," + p4 + ","
$ args = args + p5 + "," + p6 + "," + p7 + "," + p8 + ","
$!
$! Provide lower case version to simplify parsing.
$ args_lower = f$edit(args, "LOWERCASE")
$!
$ args_len = f$length(args)
$!
$ if (f$getsyi("HW_MODEL") .lt. 1024)
$ then
$   arch_name = "VAX"
$ else
$   arch_name = ""
$   arch_name = arch_name + f$edit(f$getsyi("ARCH_NAME"), "UPCASE")
$   if (arch_name .eqs. "") then arch_name = "UNK"
$ endif
$!
$!
$ pipe lib/list sys$library:decc$rtldef.tlb | search sys$input: stdint
$ if '$SEVERITY' .ne. 1
$ then
$   create sys$disk:[]stdint.h
$   open/append stdint_h sys$disk:[]stdint.h
$   write stdint_h "/* Fake stdint.h for gnulib */"
$   write stdint_h "#ifndef FAKE_STDINT"
$   write stdint_h "#define FAKE_STDINT"
$   write stdint_h "#include <fake_vms_path/limits.h>"
$   write stdint_h "#define PTRDIFF_MAX (__INT32_MAX)"
$   write stdint_h "#ifndef SIZE_MAX"
$   write stdint_h "#define SIZE_MAX (__UINT32_MAX)"
$   write stdint_h "#endif /* __VAX */"
$   write stdint_h "#endif  /* FAKE_STDINT */"
$   close stdint_h
$ endif
$!
$!
$! Start the configuration file.
$! Need to do a create and then an append to make the file have the
$! typical file attributes of a VMS text file.
$ create sys$disk:[]config_vms.h
$ open/append cvh sys$disk:[]config_vms.h
$!
$! Write the defines to prevent multiple includes.
$! These are probably not needed in this case,
$! but are best practice to put on all header files.
$ write cvh "#ifndef __CONFIG_VMS_H__"
$ write cvh "#define __CONFIG_VMS_H__"
$ write cvh ""
$!
$ write cvh "#if __CRTL_VER >= 70000000"
$ write cvh "#define VMS_V7"
$ write cvh "#else"
$ write cvh "#define HAVE_TZNAME 1 /* (faked in vms/vms_misc.c) */
$ write cvh "#define HAVE_TZSET 1 /* (faked in vms/vms_misc.c) */
$ write cvh "#endif"
$ write cvh "#if __CRTL_VER >= 70200000"
$ write cvh "#define DYNAMIC 1"
$ write cvh "#endif"
$ write cvh ""
$ write cvh "#define PRINTF_HAS_F_FORMAT 1"
$ write cvh ""
$ write cvh "/* The _Noreturn keyword of C11.  */"
$ write cvh "#ifndef _Noreturn"
$ write cvh "# if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__) \"
$ write cvh "      || 0x5110 <= __SUNPRO_C)"
$ write cvh "#  define _Noreturn __attribute__ ((__noreturn__))"
$ write cvh "# elif defined _MSC_VER && 1200 <= _MSC_VER"
$ write cvh "#  define _Noreturn __declspec (noreturn)"
$ write cvh "# else"
$ write cvh "#  define _Noreturn"
$ write cvh "# endif"
$ write cvh "#endif"
$ write cvh ""
$ write cvh "/*"
$ write cvh " * VAXCRTL is pre-ANSI and does some variations of numeric"
$ write cvh " * formatting differently than gawk expects."
$ write cvh " */"
$ write cvh "#if defined(VAX) && !defined(__DECC)"
$ write cvh -
 "/* '0' format modifier for %e,%f,%g gives wrong results in many cases */"
$ write cvh "#define VAXCRTL"
$ write cvh "/* %g format chooses %e format when should use %f */"
$ write cvh "#define GFMT_WORKAROUND	1"
$ write cvh "#endif"
$ write cvh ""
$ write cvh "/*"
$ write cvh " * VAX C"
$ write cvh " *"
$ write cvh -
 " * As of V3.2, VAX C is not yet ANSI-compliant.  But it's close enough"
$ write cvh -
 " * for GAWK's purposes.  Comment this out for VAX C V2.4 and earlier."
$ write cvh -
 " * YYDEBUG definition is needed for combination of VAX C V2.x and Bison."
$ write cvh " */"
$ write cvh "#if defined(VAXC) && !defined(__STDC__)"
$ write cvh "#define __STDC__	0"
$ write cvh "#define NO_TOKEN_PASTING"
$ write cvh "#define signed  /*empty*/"
$ write cvh "#define inline	/*empty*/"
$ write cvh "#ifndef __DECC	/* DEC C does not support #pragma builtins */"
$ write cvh "#define VAXC_BUILTINS"
$ write cvh "#endif"
$ write cvh "/* #define YYDEBUG 0 */"
$ write cvh -
  "#define NO_MBSUPPORT  /* VAX C's preprocessor can't handle mbsupport.h */"
$ write cvh "#endif"
$ write cvh ""
$ write cvh ""
$ write cvh "#if __DECC_VER >= 60400000 && !defined(DEBUG)"
$ write cvh "/* disable ""new feature in C99"" diagnostics (for regex code); "
$ write cvh "   NEWC99 ought to suffice but doesn't (at least in V6.4) */"
$ write cvh "#pragma message disable (NEWC99,DESIGNATORUSE)"
$ write cvh "#ifdef __VAX
$ write cvh "#endif
$ write cvh "#pragma message disable (LONGDOUBLENYI)"
$ write cvh "#endif"
$!
$! This stuff seems needed for VMS 7.3 and earlier, but not VMS 8.2+
$! Need some more data as to which versions these issues are fixed in.
$ write cvh "#if __VMS_VER <= 80200000"
$! mkstemp goes into an infinte loop in gawk in VAX/VMS 7.3
$ write cvh "#ifdef HAVE_MKSTEMP"
$ write cvh "#undef HAVE_MKSTEMP"
$ write cvh "#endif"
$ write cvh "#endif"
$ write cvh ""
$!
$! VMS not legal for ANSI compiler to pre-define
$ write cvh "#ifndef VMS"
$ write cvh "#define VMS 1"
$ write cvh "#endif"
$ write cvh ""
$!
$! Need to temp hide stuff that gawk is replacing or redefining before
$! including the header.
$ write cvh "/* Need to hide some stuff */"
$ write cvh "#define getopt hide_getopt"
$ write cvh "#define optopt hide_optopt"
$ write cvh "#define optind hide_optind"
$ write cvh "#define optarg hide_optarg"
$ write cvh "#define opterr hide_opterr"
$ write cvh "#define getpgrp hide_getpgrp"
$ write cvh "#define unsetenv hide_unsetenv"
$ write cvh "#define read hide_read"
$ write cvh "#define delete hide_delete"
$ write cvh "#define getcwd hide_getcwd"
$ write cvh "#define getgid hide_getgid"
$ write cvh "#define getegid hide_getegid"
$ write cvh "#define setgid hide_setgid"
$ write cvh "#define exit hide_exit"
$ write cvh "#define _exit hide__exit"
$ write cvh "#include <unistd.h>"
$ write cvh "#include <stdlib.h>"
$ write cvh "#include <stdio.h>"
$ write cvh "#include <time.h>"
$ write cvh "#include <stsdef.h>"
$ write cvh "#include <string.h>"
$ write cvh "#undef getopt"
$ write cvh "#undef optopt"
$ write cvh "#undef optind"
$ write cvh "#undef optarg"
$ write cvh "#undef opterr"
$ write cvh "#undef getpgrp"
$ write cvh "#undef getcwd"
$ write cvh "#undef unsetenv"
$ write cvh "#undef read"
$ write cvh "#undef delete"
$ write cvh "#undef getgid"
$ write cvh "#undef getegid"
$ write cvh "#undef setgid"
$ write cvh "#undef exit"
$ write cvh "#undef _exit"
$!
$write cvh "#ifdef HAVE_STRNCASECMP"
$write cvh "#undef HAVE_STRNCASECMP"
$write cvh "#endif"
$!
$ write cvh "#define IN_CONFIG_H"
$ write cvh "#include ""redirect.h"""
$ write cvh "#undef IN_CONFIG_H"
$ write cvh "#define getpgrp gawk_vms_getpgrp"
$ write cvh "#ifdef HAVE_SETENV"
$ write cvh "#undef HAVE_SETENV"
$ write cvh "#endif"
$ write cvh "#ifdef HAVE_UNSETENV"
$ write cvh "#undef HAVE_UNSETENV"
$ write cvh "#endif"
$ write cvh "#ifdef HAVE_STRFTIME"
$ write cvh "#undef HAVE_STRFTIME"
$ write cvh "#define USE_INCLUDED_STRFTIME"
$ write cvh "#endif /* HAVE_STRFTIME */"
$ write cvh ""
$ write cvh "#include <bitypes.h>"
$ write cvh "#define INT32_MAX __INT32_MAX"
$ write cvh "#define INT32_MIN __INT32_MIN"
$ write cvh ""
$ write cvh "/*"
$ write cvh " * DEFPATH"
$ write cvh " *	VMS: ""/AWK_LIBRARY"" => ""AWK_LIBRARY:"""
$ write cvh " * The default search path for the -f option of gawk.  It is"
$ write cvh " * used if the AWKPATH environment variable is undefined."
$ write cvh " *"
$ write cvh " * Note: OK even if no AWK_LIBRARY logical name has been defined."
$ write cvh " */"
$ write cvh ""
$ write cvh "#define DEFPATH	"".,/AWK_LIBRARY"""
$ write cvh "#define DEFLIBPATH "".,/AWK_LIBRARY"""
$ write cvh "#define ENVSEP	','"
$ write cvh ""
$ write cvh "/*"
$ write cvh " * Extended source file access."
$ write cvh " */"
$ write cvh "#define DEFAULT_FILETYPE "".awk"""
$ write cvh ""
$ write cvh "/*"
$ write cvh " * fork/Pipe handling."
$ write cvh " */"
$ write cvh "#define PIPES_SIMULATED	1"
$ write cvh ""
$ write cvh "/* Extension for shared libraries */"
$ write cvh "#define SHLIBEXT	""exe"""
$ write cvh ""
$! GAWK does not want to use ALLOCA
$ write cvh "#define NO_ALLOCA"
$ write cvh "#define STACK_DIRECTION (-1)"
$ write cvh ""
$ write cvh "void decc$exit(int status);"
$ write cvh "#define _exit(foo) vms_exit(foo)"
$ write cvh "#define exit(foo) vms_exit(foo)"
$ write cvh ""
$ write cvh "/* Use POSIX exit codes here */"
$ write cvh "#ifdef EXIT_FAILURE"
$ write cvh "#undef EXIT_FAILURE"
$ write cvh "#endif"
$ write cvh "#define EXIT_FAILURE (1)"
$ write cvh ""
$ write cvh "#define EXIT_FATAL (2)"
$ write cvh ""
$ write cvh "#ifndef C_FACILITY_NO"
$ write cvh "#define C_FACILITY_NO 0x350000"
$ write cvh "#endif"
$ write cvh ""
$ write cvh "/* Build a Posix Exit with VMS severity */
$ write cvh "static void vms_exit(int status) {"
$ write cvh "    int vms_status;"
$ write cvh "    /* Fake the __posix_exit with severity added */"
$ write cvh "    /* Undocumented correct way to do this. */"
$ write cvh "    vms_status = 0;"
$ write cvh "    if (status != 0) {"
$ write cvh "        vms_status = C_FACILITY_NO | 0xA000 | STS$M_INHIB_MSG;"
$ write cvh "        vms_status |= (status << 3);"
$ write cvh "    }"
$ write cvh "    if (status == EXIT_FAILURE) {"
$ write cvh "        vms_status |= STS$K_ERROR;"
$ write cvh "    } else if (status == EXIT_FATAL) {"
$ write cvh "        vms_status |= STS$K_SEVERE;"
$ write cvh "    } else {"
$ write cvh "        vms_status |= STS$K_SUCCESS;"
$ write cvh "    }"
$ write cvh "    decc$exit(vms_status);"
$ write cvh "}"
$ write cvh ""
$ write cvh "#define TIME_T_UNSIGNED 1"
$ write cvh "#include ""custom.h"""
$ write cvh ""
$
$!
$! Close out the file
$!
$ write cvh ""
$ write cvh "#endif /* __CONFIG_VMS_H__ */"
$ close cvh
$!
$all_exit:
$ exit
