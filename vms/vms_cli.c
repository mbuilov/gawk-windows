/* vms_cli.c -- interface to CLI$xxx routines for fetching command line components

   Copyright (C) 1991-1993, 2003, 2011, 2014, 2016
   the Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */


/*
 * vms_cli.c - command line interface routines.
 *							Pat Rankin, Nov'89
 *	Routines called from vms_gawk.c for DCL parsing.
 */

#include "config.h"	/* in case we want to suppress 'const' &c */
#include "vms.h"
#ifndef _STRING_H
#include <string.h>
#endif

extern U_Long CLI$PRESENT(const struct dsc$descriptor_s *);
extern U_Long CLI$GET_VALUE(const struct dsc$descriptor_s *,
                            struct dsc$descriptor_s *, short *);
extern U_Long CLI$DCL_PARSE(const struct dsc$descriptor_s *, const void *, ...);
extern U_Long SYS$CLI(void *, ...);
extern U_Long SYS$FILESCAN(const struct dsc$descriptor_s *, void *, long *);
extern void  *LIB$ESTABLISH(U_Long (*handler)(void *, void *));
extern U_Long LIB$SIG_TO_RET(void *, void *);	/* condition handler */

/* Cli_Present() - call CLI$PRESENT to determine whether a parameter or     */
/*		  qualifier is present on the [already parsed] command line */
U_Long
Cli_Present( const char *item )
{
    struct dsc$descriptor_s item_dsc;
    (void)LIB$ESTABLISH(LIB$SIG_TO_RET);

    item_dsc.dsc$w_length = strlen(item_dsc.dsc$a_pointer = (char *)item);
    item_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    item_dsc.dsc$b_class = DSC$K_CLASS_S;
    return CLI$PRESENT(&item_dsc);
}

/* Cli_Get_Value() - call CLI$GET_VALUE to retreive the value of a */
/*		    parameter or qualifier from the command line   */
U_Long
Cli_Get_Value( const char *item, char *result, int size )
{
    struct dsc$descriptor_s item_dsc, res_dsc;
    U_Long sts;
    short len = 0;
    (void)LIB$ESTABLISH(LIB$SIG_TO_RET);

    item_dsc.dsc$w_length = strlen(item_dsc.dsc$a_pointer = (char *)item);
    item_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    item_dsc.dsc$b_class = DSC$K_CLASS_S;
    res_dsc.dsc$w_length = size;
    res_dsc.dsc$a_pointer = result;
    res_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    res_dsc.dsc$b_class = DSC$K_CLASS_S;
    sts = CLI$GET_VALUE(&item_dsc, &res_dsc, &len);
    result[len] = '\0';
    return sts;
}

/* Cli_Parse_Command() - use the $CLI system service (undocumented) to	 */
/*			retreive the actual command line (which might be */
/*			"run prog" or "mcr prog [params]") and then call */
/*			CLI$DCL_PARSE to parse it using specified tables */
U_Long
Cli_Parse_Command( const void *cmd_tables, const char *cmd_verb )
{
    struct { short len, code; void *adr; } fscn[2];
    struct { char rqtype, rqindx, rqflags, rqstat; unsigned :32;
	     struct dsc$descriptor_s rdesc;
             unsigned :32; unsigned :32; unsigned :32; } cmd;
    U_Long sts;
    int    ltmp;
    char   longbuf[8200];
    (void)LIB$ESTABLISH(LIB$SIG_TO_RET);

    memset(&cmd, 0, sizeof cmd);
    cmd.rqtype = CLI$K_GETCMD;		/* command line minus the verb */
    sts = SYS$CLI(&cmd, (void *)0, (void *)0);	/* get actual command line */

    if (vmswork(sts)) {		/* ok => cli available & verb wasn't "RUN" */
	/* invoked via symbol => have command line (which might be empty) */
	/*    [might also be invoked via mcr or dcl; that's ok]		  */
	if (cmd.rqstat == CLI$K_VERB_MCR) {
	    /* need to strip image name from MCR invocation   */
	    memset(fscn, 0, sizeof fscn);
	    fscn[0].code = FSCN$_FILESPEC;	/* full file specification */
	    (void)SYS$FILESCAN(&cmd.rdesc, fscn, (long *)0);
	    cmd.rdesc.dsc$w_length -= fscn[0].len;	/* shrink size */
	    cmd.rdesc.dsc$a_pointer += fscn[0].len;	/* advance ptr */
	}
	/* prepend verb and then parse the command line */
	strcat(strcpy(longbuf, cmd_verb), " "),  ltmp = strlen(longbuf);
	if (cmd.rdesc.dsc$w_length + ltmp > sizeof longbuf)
	    cmd.rdesc.dsc$w_length = sizeof longbuf - ltmp;
	strncpy(&longbuf[ltmp],
                cmd.rdesc.dsc$a_pointer, cmd.rdesc.dsc$w_length);
	cmd.rdesc.dsc$w_length += ltmp,	cmd.rdesc.dsc$a_pointer = longbuf;
	sts = CLI$DCL_PARSE(&cmd.rdesc, cmd_tables);
    }

    return sts;
}
